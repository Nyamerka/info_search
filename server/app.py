"""
Streamlit UI для поисковой системы по поэзии.
"""

import os
import logging
import streamlit as st
import pandas as pd
import plotly.graph_objects as go
from typing import List, Optional
from dataclasses import dataclass
from pymongo import MongoClient, UpdateOne


# Настройка логирования
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler('search_app.log')
    ]
)


# Попытка импорта C++ моста (может не работать без библиотеки)
try:
    from search_bridge import SearchEngine, SearchResult
    SEARCH_ENGINE_AVAILABLE = True
except Exception as e:
    SEARCH_ENGINE_AVAILABLE = False
    print(f"Warning: C++ search engine not available: {e}")


# Импорт модулей оценки
try:
    from evaluation import SearchEvaluator
    EVALUATION_AVAILABLE = True
except Exception as e:
    EVALUATION_AVAILABLE = False
    print(f"Warning: Evaluation module not available: {e}")


@dataclass
class DisplayResult:
    """Результат поиска для отображения."""
    doc_id: int
    score: float
    title: str
    text: str
    author: str
    year: str


class SearchApp:
    """Streamlit приложение для поиска."""
    
    def __init__(self, progress_callback=None):
        self.mongo_uri = os.getenv("MONGO_URI", "mongodb://mongodb:27017")
        self.db_name = os.getenv("DB_NAME", "poetry_search")
        self.lib_path = os.getenv("LIB_PATH", None)
        self.progress_callback = progress_callback
        self.logger = logging.getLogger(__name__)
        
        self._init_mongo()
        self._init_search_engine()
    
    def _init_mongo(self):
        """Инициализация подключения к MongoDB."""
        try:
            self.mongo_client = MongoClient(self.mongo_uri, serverSelectionTimeoutMS=5000)
            self.mongo_client.admin.command('ping')
            self.db = self.mongo_client[self.db_name]
            self.collection = self.db["poems"]
            self.mongo_available = True
            self.logger.info("MongoDB connected successfully")
        except Exception as e:
            self.logger.error(f"MongoDB not available: {e}")
            self.mongo_available = False
            self.collection = None
    
    def _init_search_engine(self):
        """Инициализация C++ поисковой системы."""
        if SEARCH_ENGINE_AVAILABLE:
            try:
                self.search_engine = SearchEngine(lib_path=self.lib_path)
                self.engine_available = True
                self.logger.info("C++ search engine initialized")
                
                # Если MongoDB доступна, загружаем документы в индекс
                if self.mongo_available and self.search_engine.get_document_count() == 0:
                    self._load_index_from_mongo()
            except Exception as e:
                self.logger.error(f"Search engine error: {e}")
                self.search_engine = None
                self.engine_available = False
        else:
            self.search_engine = None
            self.engine_available = False
    
    def _load_index_from_mongo(self, limit: int = 50000):
        """Загружает документы из MongoDB в C++ индекс."""
        if self.collection is None or not self.search_engine:
            return
        
        # Очищаем старые cpp_doc_id перед переиндексацией
        self.collection.update_many({}, {"$unset": {"cpp_doc_id": ""}})
        
        self.logger.info(f"Starting to index documents (limit: {limit})...")
        # Сортируем по _id для детерминированного порядка
        cursor = self.collection.find().sort("_id", 1).limit(limit)
        
        # Получаем общее количество для прогресс-бара
        total = min(limit, self.collection.count_documents({}))
        
        # Batch операции для MongoDB
        batch_size = 500
        bulk_operations = []
        
        for idx, doc in enumerate(cursor, 1):
            content = doc.get("text", "")
            title = doc.get("title", "")
            
            if content:
                # Добавляем в C++ индекс
                cpp_id = self.search_engine.add_document(content, title)
                
                # Накапливаем bulk операции
                bulk_operations.append(
                    UpdateOne(
                        {"_id": doc["_id"]},
                        {"$set": {"cpp_doc_id": cpp_id}}
                    )
                )
                
                # Выполняем batch обновление
                if len(bulk_operations) >= batch_size:
                    self.collection.bulk_write(bulk_operations, ordered=False)
                    bulk_operations = []
                
                # Обновляем прогресс
                if idx % 1000 == 0:
                    progress = idx / total
                    self.logger.info(f"Indexed {idx}/{total} documents ({progress*100:.1f}%)")
                    if self.progress_callback:
                        self.progress_callback(progress, f"Индексировано {idx}/{total} документов")
        
        # Дозаписываем оставшиеся операции
        if bulk_operations:
            self.collection.bulk_write(bulk_operations, ordered=False)
        
        indexed_count = self.search_engine.get_document_count()
        self.logger.info(f"Indexing complete! Total indexed: {indexed_count}")
        if self.progress_callback:
            self.progress_callback(1.0, f"Готово! Индексировано {indexed_count} документов")
    
    def search_tfidf(self, query: str, top_k: int = 10) -> List[DisplayResult]:
        """TF-IDF поиск."""
        results = []
        
        if self.engine_available and self.search_engine:
            search_results = self.search_engine.search_tfidf(query, top_k)
            
            for sr in search_results:
                doc = self._get_doc_by_cpp_id(sr.doc_id)
                if doc:
                    results.append(DisplayResult(
                        doc_id=sr.doc_id,
                        score=sr.score,
                        title=doc.get("title", "Без названия"),
                        text=doc.get("text", "")[:500] + "...",
                        author=doc.get("author", "Неизвестен"),
                        year=doc.get("year", ""),
                    ))
        
        return results
    
    def search_boolean(self, query: str, top_k: int = 10) -> List[DisplayResult]:
        """Булев поиск с TF-IDF ранжированием."""
        self.logger.info(f"Boolean search: query='{query}', top_k={top_k}")
        results = []
    
        if self.engine_available and self.search_engine:
            # 1. Получаем булевые результаты (фильтрация по AND/OR/NOT)
            doc_ids = self.search_engine.boolean_query(query)
            self.logger.info(f"Boolean search: C++ returned {len(doc_ids)} document IDs")
        
            if not doc_ids:
                self.logger.warning(f"Boolean search: No results found for query '{query}'")
                return []
        
            # 2. Извлекаем термины запроса (без операторов)
            query_terms = self._extract_query_terms(query)
        
            if not query_terms:
                # Если нет терминов, возвращаем первые top_k результатов
                for doc_id in doc_ids[:top_k]:
                    doc = self._get_doc_by_cpp_id(doc_id)
                    if doc:
                        results.append(DisplayResult(
                            doc_id=doc_id,
                            score=0.0,
                            title=doc.get("title", "Без названия"),
                            text=doc.get("text", "")[:500] + "...",
                            author=doc.get("author", "Неизвестен"),
                            year=doc.get("year", ""),
                        ))
                return results
        
            # 3. Получаем TF-IDF для всего индекса
            clean_query = ' '.join(query_terms)
            # Запрашиваем достаточно результатов, чтобы покрыть булевый набор
            # Но не больше разумного лимита
            search_limit = min(len(doc_ids), 100)
            tfidf_results = self.search_engine.search_tfidf(clean_query, top_k=search_limit)
        
            # 4. Создаём set из булевых результатов для быстрой проверки
            boolean_set = set(doc_ids)
        
            # 5. Фильтруем TF-IDF результаты, оставляя только булевые
            for tfidf_res in tfidf_results:
                if tfidf_res.doc_id in boolean_set:
                    doc = self._get_doc_by_cpp_id(tfidf_res.doc_id)
                    if doc:
                        results.append(DisplayResult(
                            doc_id=tfidf_res.doc_id,
                            score=tfidf_res.score,
                            title=doc.get("title", "Без названия"),
                            text=doc.get("text", "")[:500] + "...",
                            author=doc.get("author", "Неизвестен"),
                            year=doc.get("year", ""),
                        ))
                    
                        # Как только получили top_k, можем остановиться
                        if len(results) >= top_k:
                            break
        
            # 6. Если не хватило результатов, добавляем оставшиеся без score
            if len(results) < top_k:
                scored_ids = {r.doc_id for r in results}
                for doc_id in doc_ids:
                    if doc_id not in scored_ids:
                        doc = self._get_doc_by_cpp_id(doc_id)
                        if doc:
                            results.append(DisplayResult(
                                doc_id=doc_id,
                                score=0.0,
                                title=doc.get("title", "Без названия"),
                                text=doc.get("text", "")[:500] + "...",
                                author=doc.get("author", "Неизвестен"),
                                year=doc.get("year", ""),
                            ))
                            if len(results) >= top_k:
                                break
        
            self.logger.info(f"Boolean search: Returning {len(results)} ranked results")
    
        return results

    
    def _extract_query_terms(self, query: str) -> List[str]:
        """Извлекает термины запроса, исключая операторы AND/OR/NOT и скобки."""
        operators = {'and', 'or', 'not', '(', ')'}
        
        # Разбиваем запрос на токены
        tokens = query.lower().replace('(', ' ( ').replace(')', ' ) ').split()
        
        # Фильтруем операторы
        terms = [t for t in tokens if t not in operators and t.strip()]
        
        self.logger.debug(f"Extracted query terms: {terms} from query: '{query}'")
        return terms
    
    def _get_doc_by_cpp_id(self, cpp_id: int) -> Optional[dict]:
        """Получает документ из MongoDB по C++ ID."""
        if self.collection is not None:
            return self.collection.find_one({"cpp_doc_id": cpp_id})
        return None
    
    def get_stats(self) -> dict:
        """Статистика системы."""
        stats = {
            "mongo_available": self.mongo_available,
            "engine_available": self.engine_available,
            "mongo_docs": 0,
            "indexed_docs": 0,
        }
        
        if self.mongo_available and self.collection is not None:
            stats["mongo_docs"] = self.collection.count_documents({})
        
        if self.engine_available and self.search_engine:
            stats["indexed_docs"] = self.search_engine.get_document_count()
        
        return stats


def render_search_tab(app: SearchApp):
    """Вкладка поиска."""
    search_mode = st.radio(
        "Режим поиска:",
        ["TF-IDF (релевантность)", "Булев (AND/OR/NOT)"],
        horizontal=True,
    )
    
    query = st.text_input(
        "Введите поисковый запрос:",
        placeholder="love AND heart" if "Булев" in search_mode else "eternal love",
    )
    
    col1, col2 = st.columns([1, 4])
    with col1:
        top_k = st.number_input("Результатов:", min_value=1, max_value=100, value=10)
    
    if st.button("🔍 Искать", type="primary"):
        if query:
            with st.spinner("Поиск..."):
                if "TF-IDF" in search_mode:
                    results = app.search_tfidf(query, top_k)
                else:
                    results = app.search_boolean(query, top_k)
            
            if results:
                st.success(f"Найдено результатов: {len(results)}")
                
                for i, res in enumerate(results, 1):
                    with st.expander(f"{i}. {res.title} (score: {res.score:.4f})"):
                        st.markdown(f"**Автор:** {res.author}")
                        if res.year:
                            st.markdown(f"**Год:** {res.year}")
                        st.markdown("---")
                        st.text(res.text)
            else:
                st.warning("Ничего не найдено. Попробуйте другой запрос.")
        else:
            st.info("Введите запрос для поиска.")


def render_metrics_tab(app: SearchApp):
    """Вкладка бенчмарков и метрик."""
    st.header("📊 Оценка качества поиска")
    
    if not EVALUATION_AVAILABLE:
        st.error("❌ Модуль evaluation.py недоступен. Убедитесь, что файл существует.")
        return
    
    if not app.engine_available:
        st.warning("⚠️ Поисковая система недоступна. Бенчмарки не могут быть запущены.")
        return
    
    st.markdown("""
    Этот раздел позволяет оценить качество поиска с помощью метрик:
    - **Precision@k** - доля релевантных документов в топ-k
    - **DCG@k** - учитывает позицию документов
    - **NDCG@k** - нормализованный DCG
    - **ERR@k** - вероятностная метрика ожидаемого ранга
    """)
    
    # Настройки бенчмарка
    st.subheader("⚙️ Настройки бенчмарка")
    
    col1, col2 = st.columns(2)
    
    with col1:
        search_mode = st.selectbox(
            "Режим поиска:",
            ["tfidf", "boolean"],
            index=0
        )
        
        n_synthetic = st.number_input(
            "Количество синтетических запросов:",
            min_value=5,
            max_value=100,
            value=20,
            help="Автоматически генерируемые запросы для оценки"
        )
    
    with col2:
        top_k = st.number_input(
            "Top-K для оценки:",
            min_value=5,
            max_value=50,
            value=10
        )
        
        use_synthetic = st.checkbox(
            "Использовать синтетические запросы",
            value=True,
            help="Генерировать случайные запросы из корпуса"
        )
    
    # Кнопка запуска
    if st.button("🚀 Запустить бенчмарк", type="primary"):
        with st.spinner("Выполняется оценка качества поиска..."):
            try:
                # Создаём evaluator
                evaluator = SearchEvaluator(app)
                
                # Запускаем оценку
                results = evaluator.evaluate_all(
                    search_mode=search_mode,
                    top_k=top_k,
                    use_synthetic=use_synthetic,
                    n_synthetic=n_synthetic
                )
                
                # Сохраняем результаты в session_state
                st.session_state.benchmark_results = results
                
            except Exception as e:
                st.error(f"Ошибка при выполнении бенчмарка: {e}")
                st.exception(e)
    
    # Отображение результатов
    if "benchmark_results" in st.session_state:
        results = st.session_state.benchmark_results
        
        if results:
            st.success(f"✅ Бенчмарк выполнен! Оценено запросов: {results.get('n_queries', 0)}")
            
            # Усредненные метрики
            st.subheader("📈 Усредненные метрики")
            
            avg_metrics = results.get('avg_metrics', {})
            
            if avg_metrics:
                # Создаём DataFrame для таблицы
                k_values = sorted(list(avg_metrics['P'].keys()))
                
                metrics_data = {
                    'Метрика': ['P', 'DCG', 'NDCG', 'ERR']
                }
                
                for k in k_values:
                    metrics_data[f'@{k}'] = [
                        avg_metrics['P'][k],
                        avg_metrics['DCG'][k],
                        avg_metrics['NDCG'][k],
                        avg_metrics['ERR'][k]
                    ]
                
                df_metrics = pd.DataFrame(metrics_data)
                
                # Отображаем таблицу
                st.dataframe(
                    df_metrics.style.format({col: "{:.4f}" for col in df_metrics.columns if col != 'Метрика'}),
                    use_container_width=True
                )
                
                # Графики метрик
                st.subheader("📊 Визуализация метрик")
                
                # Создаём интерактивный график с Plotly
                fig = go.Figure()
                
                colors = {
                    'P': '#1f77b4',
                    'DCG': '#ff7f0e',
                    'NDCG': '#2ca02c',
                    'ERR': '#d62728'
                }
                
                for metric_name in ['P', 'NDCG', 'ERR']:
                    values = [avg_metrics[metric_name][k] for k in k_values]
                    
                    fig.add_trace(go.Scatter(
                        x=[f'@{k}' for k in k_values],
                        y=values,
                        mode='lines+markers',
                        name=metric_name,
                        line=dict(color=colors[metric_name], width=2),
                        marker=dict(size=8)
                    ))
                
                fig.update_layout(
                    title='Метрики качества поиска',
                    xaxis_title='Top-K',
                    yaxis_title='Значение метрики',
                    hovermode='x unified',
                    legend=dict(
                        orientation="h",
                        yanchor="bottom",
                        y=1.02,
                        xanchor="right",
                        x=1
                    )
                )
                
                st.plotly_chart(fig, use_container_width=True)
                
                # Отдельный график для DCG (может иметь другой масштаб)
                fig_dcg = go.Figure()
                
                dcg_values = [avg_metrics['DCG'][k] for k in k_values]
                
                fig_dcg.add_trace(go.Scatter(
                    x=[f'@{k}' for k in k_values],
                    y=dcg_values,
                    mode='lines+markers',
                    name='DCG',
                    line=dict(color=colors['DCG'], width=2),
                    marker=dict(size=8)
                ))
                
                fig_dcg.update_layout(
                    title='DCG (без нормализации)',
                    xaxis_title='Top-K',
                    yaxis_title='DCG',
                    hovermode='x unified'
                )
                
                st.plotly_chart(fig_dcg, use_container_width=True)
                
                # Детальные результаты по запросам (опционально)
                with st.expander("🔍 Детальные результаты по запросам"):
                    per_query = results.get('per_query_metrics', [])
                    
                    if per_query:
                        for i, query_result in enumerate(per_query[:10], 1):
                            st.markdown(f"**{i}. {query_result['query']}**")
                            
                            query_metrics = query_result.get('metrics', {})
                            
                            # Мини-таблица для каждого запроса
                            mini_data = {'Метрика': ['NDCG@10', 'P@10', 'ERR@10']}
                            mini_data['Значение'] = [
                                query_metrics.get('NDCG', {}).get(10, 0.0),
                                query_metrics.get('P', {}).get(10, 0.0),
                                query_metrics.get('ERR', {}).get(10, 0.0)
                            ]
                            
                            st.dataframe(
                                pd.DataFrame(mini_data).style.format({'Значение': "{:.4f}"}),
                                hide_index=True
                            )
                            
                            if i >= 10:
                                st.info(f"Показаны первые 10 из {len(per_query)} запросов")
                                break


def main():
    st.set_page_config(
        page_title="Poetry Search Engine",
        page_icon="📜",
        layout="wide",
    )
    
    st.title("📜 Poetry Search Engine")
    st.markdown("*Поисковая система по корпусу поэзии (C++ backend + Streamlit UI)*")
    
    # Инициализация приложения
    if "app" not in st.session_state:
        # Создаём progress bar
        progress_bar = st.progress(0)
        status_text = st.empty()
        
        def update_progress(progress: float, text: str):
            progress_bar.progress(progress)
            status_text.text(text)
        
        status_text.text("Подключение к базе данных...")
        st.session_state.app = SearchApp(progress_callback=update_progress)
        
        # Очищаем UI после инициализации
        progress_bar.empty()
        status_text.empty()
    
    app = st.session_state.app
    
    # Боковая панель со статистикой
    with st.sidebar:
        st.header("📊 Статистика")
        stats = app.get_stats()
        
        st.metric("MongoDB", "✅ Доступна" if stats["mongo_available"] else "❌ Недоступна")
        st.metric("C++ Engine", "✅ Активен" if stats["engine_available"] else "❌ Недоступен")
        st.metric("Документов в MongoDB", stats["mongo_docs"])
        st.metric("Проиндексировано", stats["indexed_docs"])
        
        st.markdown("---")
        st.markdown("""
        **Возможности:**
        - TF-IDF ранжирование
        - Булев поиск (AND, OR, NOT)
        - LZW-сжатие документов
        - Porter Stemmer
        - Метрики качества (P, NDCG, ERR)
        """)
    
    # Вкладки
    tab1, tab2 = st.tabs(["🔍 Поиск", "📊 Метрики"])
    
    with tab1:
        render_search_tab(app)
    
    with tab2:
        render_metrics_tab(app)
    
    # Демо режим если движок недоступен
    if not app.engine_available:
        st.warning("""
        ⚠️ **C++ поисковая система недоступна.**
        
        Убедитесь, что:
        1. Библиотека `libsearch_engine.so` скомпилирована
        2. Путь к библиотеке указан в переменной `LIB_PATH`
        """)


if __name__ == "__main__":
    main()