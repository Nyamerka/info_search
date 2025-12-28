"""
Streamlit UI для поисковой системы по поэзии.
"""
import os
import logging
import streamlit as st
import pandas as pd
import plotly.graph_objects as go
from typing import List, Optional, Dict
from dataclasses import dataclass
from pymongo import MongoClient, UpdateOne, ASCENDING


logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.StreamHandler(),
        logging.FileHandler('search_app.log')
    ]
)


try:
    from search_bridge import SearchEngine, SearchResult
    SEARCH_ENGINE_AVAILABLE = True
except Exception as e:
    SEARCH_ENGINE_AVAILABLE = False
    print(f"Warning: C++ search engine not available: {e}")




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
        self._doc_cache: Dict[int, dict] = {}
        
        self._init_mongo()
        self._init_search_engine()
    
    def _init_mongo(self):
        """Инициализация подключения к MongoDB."""
        try:
            self.mongo_client = MongoClient(self.mongo_uri, serverSelectionTimeoutMS=5000)
            self.mongo_client.admin.command('ping')
            self.db = self.mongo_client[self.db_name]
            self.collection = self.db["poems"]
            
            self.collection.create_index([("cpp_doc_id", ASCENDING)], unique=False, background=True)
            self.collection.create_index([("_id", ASCENDING), ("cpp_doc_id", ASCENDING)], background=True)
            self.logger.info("MongoDB indexes ensured")
            
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
        
        self.collection.update_many({}, {"$unset": {"cpp_doc_id": ""}})
        
        self.logger.info(f"Starting to index documents (limit: {limit})...")
        
        cursor = self.collection.find().sort("_id", 1).limit(limit)
        
        total = min(limit, self.collection.count_documents({}))
        
        batch_size = 500
        bulk_operations = []
        
        for idx, doc in enumerate(cursor, 1):
            content = doc.get("text", "")
            title = doc.get("title", "")
            
            if content:
                cpp_id = self.search_engine.add_document(content, title)
                
                bulk_operations.append(
                    UpdateOne(
                        {"_id": doc["_id"]},
                        {"$set": {"cpp_doc_id": cpp_id}}
                    )
                )
                
                if len(bulk_operations) >= batch_size:
                    self.collection.bulk_write(bulk_operations, ordered=False)
                    bulk_operations = []
                
                if idx % 1000 == 0:
                    progress = idx / total
                    self.logger.info(f"Indexed {idx}/{total} documents ({progress*100:.1f}%)")
                    if self.progress_callback:
                        self.progress_callback(progress, f"Индексировано {idx}/{total} документов")
        
        if bulk_operations:
            self.collection.bulk_write(bulk_operations, ordered=False)
        
        indexed_count = self.search_engine.get_document_count()
        self.logger.info(f"Indexing complete! Total indexed: {indexed_count}")
        if self.progress_callback:
            self.progress_callback(1.0, f"Готово! Индексировано {indexed_count} документов")
    
    def _get_docs_batch(self, cpp_ids: List[int]) -> Dict[int, dict]:
        """Получает документы из MongoDB пакетом по C++ ID."""
        if self.collection is None or not cpp_ids:
            return {}
        
        cached_ids = set(cpp_ids) & set(self._doc_cache.keys())
        missing_ids = set(cpp_ids) - cached_ids
        
        result = {cpp_id: self._doc_cache[cpp_id] for cpp_id in cached_ids}
        
        if missing_ids:
            docs = self.collection.find(
                {"cpp_doc_id": {"$in": list(missing_ids)}},
                {"cpp_doc_id": 1, "title": 1, "text": 1, "author": 1, "year": 1}
            )
            
            for doc in docs:
                cpp_id = doc.get("cpp_doc_id")
                if cpp_id is not None:
                    self._doc_cache[cpp_id] = doc
                    result[cpp_id] = doc
        
        return result
    
    def search_tfidf(self, query: str, top_k: int = 10) -> List[DisplayResult]:
        """TF-IDF поиск."""
        results = []
        
        if self.engine_available and self.search_engine:
            search_results = self.search_engine.search_tfidf(query, top_k)
            
            cpp_ids = [sr.doc_id for sr in search_results]
            docs_map = self._get_docs_batch(cpp_ids)
            
            for sr in search_results:
                doc = docs_map.get(sr.doc_id)
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
            doc_ids = self.search_engine.boolean_query(query)
            self.logger.info(f"Boolean search: C++ returned {len(doc_ids)} document IDs")
        
            if not doc_ids:
                self.logger.warning(f"Boolean search: No results found for query '{query}'")
                return []
        
            query_terms = self._extract_query_terms(query)
        
            if not query_terms:
                cpp_ids = doc_ids[:top_k]
                docs_map = self._get_docs_batch(cpp_ids)
                
                for doc_id in cpp_ids:
                    doc = docs_map.get(doc_id)
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
        
            clean_query = ' '.join(query_terms)
            
            tfidf_results = self.search_engine.search_tfidf(clean_query, top_k=doc_ids)
        
            boolean_set = set(doc_ids)
            
            matched_results = []
            for tfidf_res in tfidf_results:
                if tfidf_res.doc_id in boolean_set:
                    matched_results.append(tfidf_res)
                    if len(matched_results) >= top_k:
                        break
            
            cpp_ids = [r.doc_id for r in matched_results]
            docs_map = self._get_docs_batch(cpp_ids)
        
            for tfidf_res in matched_results:
                doc = docs_map.get(tfidf_res.doc_id)
                if doc:
                    results.append(DisplayResult(
                        doc_id=tfidf_res.doc_id,
                        score=tfidf_res.score,
                        title=doc.get("title", "Без названия"),
                        text=doc.get("text", "")[:500] + "...",
                        author=doc.get("author", "Неизвестен"),
                        year=doc.get("year", ""),
                    ))
        
            if len(results) < top_k:
                scored_ids = {r.doc_id for r in results}
                remaining_ids = [did for did in doc_ids if did not in scored_ids][:top_k - len(results)]
                
                if remaining_ids:
                    remaining_docs = self._get_docs_batch(remaining_ids)
                    for doc_id in remaining_ids:
                        doc = remaining_docs.get(doc_id)
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
        
        tokens = query.lower().replace('(', ' ( ').replace(')', ' ) ').split()
        
        terms = [t for t in tokens if t not in operators and t.strip()]
        
        self.logger.debug(f"Extracted query terms: {terms} from query: '{query}'")
        return terms
    
    def _get_doc_by_cpp_id(self, cpp_id: int) -> Optional[dict]:
        """Получает документ из MongoDB по C++ ID."""
        if cpp_id in self._doc_cache:
            return self._doc_cache[cpp_id]
        
        if self.collection is not None:
            doc = self.collection.find_one({"cpp_doc_id": cpp_id})
            if doc:
                self._doc_cache[cpp_id] = doc
            return doc
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
        key="search_mode_radio"
    )
    
    query = st.text_input(
        "Введите поисковый запрос:",
        placeholder="love AND heart" if "Булев" in search_mode else "eternal love",
        key="search_query_input"
    )
    
    col1, col2 = st.columns([1, 4])
    with col1:
        top_k = st.number_input("Результатов:", min_value=1, max_value=100, value=10, key="search_top_k")
    
    if st.button("🔍 Искать", type="primary", key="search_button"):
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
    - **Precision@k (P@k)** - доля релевантных документов в топ-k
    - **DCG@k** - учитывает позицию документов (чем выше в списке, тем важнее)
    - **NDCG@k** - нормализованный DCG к идеальной выдаче (значение от 0 до 1)
    - **ERR@k** - вероятностная метрика ожидаемого обратного ранга
    """)
    
    st.subheader("⚙️ Настройки бенчмарка")
    
    col1, col2 = st.columns(2)
    
    with col1:
        search_mode = st.selectbox(
            "Режим поиска:",
            ["tfidf", "boolean"],
            index=0,
            key="metrics_search_mode"
        )
        
        n_synthetic = st.number_input(
            "Количество синтетических запросов:",
            min_value=5,
            max_value=100,
            value=50,
            help="Автоматически генерируемые запросы для оценки",
            key="metrics_n_synthetic"
        )
    
    with col2:
        evaluate_all = st.checkbox(
            "Оценить все результаты (без ограничения Top-K)",
            value=False,
            help="Построить графики по всем найденным документам, без лимита Top-K",
            key="metrics_evaluate_all"
        )
        
        if evaluate_all:
            st.info("📊 Режим: оценка всех результатов")
            top_k = 50000
            display_top_k = "Все"
        else:
            top_k = st.number_input(
                "Top-K для оценки:",
                min_value=5,
                max_value=200,
                value=50,
                help="Количество результатов для оценки по каждому запросу",
                key="metrics_top_k"
            )
            display_top_k = str(top_k)
        
        use_synthetic = st.checkbox(
            "Использовать синтетические запросы",
            value=True,
            help="Генерировать случайные запросы из корпуса",
            key="metrics_use_synthetic"
        )
    
    progress_placeholder = st.empty()
    status_placeholder = st.empty()
    
    if st.button("🚀 Запустить бенчмарк", type="primary", key="run_benchmark_button"):
        progress_bar = progress_placeholder.progress(0)
        
        def update_progress(progress: float, message: str):
            progress_bar.progress(min(progress, 1.0))
            status_placeholder.text(message)
        
        try:
            evaluator = SearchEvaluator(app, progress_callback=update_progress)
            
            update_progress(0.05, "Инициализация бенчмарка...")
            
            results = evaluator.evaluate_all(
                search_mode=search_mode,
                top_k=top_k,
                use_synthetic=use_synthetic,
                n_synthetic=n_synthetic
            )
            
            progress_bar.progress(1.0)
            status_placeholder.success("✅ Бенчмарк завершён!")
            
            st.session_state.benchmark_results = results
            st.session_state.evaluate_all_mode = evaluate_all
            st.session_state.display_top_k = display_top_k
            
        except Exception as e:
            status_placeholder.error(f"Ошибка при выполнении бенчмарка: {e}")
            st.exception(e)
    
    if "benchmark_results" in st.session_state:
        results = st.session_state.benchmark_results
        evaluate_all_mode = st.session_state.get('evaluate_all_mode', False)
        display_top_k = st.session_state.get('display_top_k', 'N/A')
        
        if results:
            mode_text = "по всем результатам" if evaluate_all_mode else f"с Top-K={display_top_k}"
            st.success(f"✅ Бенчмарк выполнен {mode_text}! Оценено запросов: {results.get('n_queries', 0)}")
            
            st.subheader("📈 Усредненные метрики")
            
            avg_metrics = results.get('avg_metrics', {})
            k_values = results.get('k_values', sorted(list(avg_metrics['P'].keys())) if avg_metrics else [])
            
            if avg_metrics and k_values:
                st.info(f"📍 Метрики рассчитаны для следующих значений K: {', '.join(map(str, k_values))}")
                
                metrics_data = {
                    'Метрика': ['P', 'DCG', 'NDCG', 'ERR']
                }
                
                for k in k_values:
                    metrics_data[f'@{k}'] = [
                        avg_metrics['P'].get(k, 0.0),
                        avg_metrics['DCG'].get(k, 0.0),
                        avg_metrics['NDCG'].get(k, 0.0),
                        avg_metrics['ERR'].get(k, 0.0)
                    ]
                
                df_metrics = pd.DataFrame(metrics_data)
                
                st.dataframe(
                    df_metrics.style.format({col: "{:.4f}" for col in df_metrics.columns if col != 'Метрика'}),
                    use_container_width=True
                )
                
                st.subheader("📊 Визуализация усредненных метрик")
                
                fig = go.Figure()
                
                colors = {
                    'P': '#1f77b4',
                    'DCG': '#ff7f0e',
                    'NDCG': '#2ca02c',
                    'ERR': '#d62728'
                }
                
                for metric_name in ['P', 'NDCG', 'ERR']:
                    values = [avg_metrics[metric_name].get(k, 0.0) for k in k_values]
                    
                    fig.add_trace(go.Scatter(
                        x=[f'@{k}' for k in k_values],
                        y=values,
                        mode='lines+markers',
                        name=metric_name,
                        line=dict(color=colors[metric_name], width=2),
                        marker=dict(size=8)
                    ))
                
                title_suffix = " (все результаты)" if evaluate_all_mode else f" (Top-{display_top_k})"
                fig.update_layout(
                    title=f'Метрики качества поиска (усредненные){title_suffix}',
                    xaxis_title='Top-K',
                    yaxis_title='Значение метрики',
                    hovermode='x unified',
                    legend=dict(
                        orientation="h",
                        yanchor="bottom",
                        y=1.02,
                        xanchor="right",
                        x=1
                    ),
                    height=500
                )
                
                st.plotly_chart(fig, use_container_width=True)
                
                fig_dcg = go.Figure()
                
                dcg_values = [avg_metrics['DCG'].get(k, 0.0) for k in k_values]
                
                fig_dcg.add_trace(go.Scatter(
                    x=[f'@{k}' for k in k_values],
                    y=dcg_values,
                    mode='lines+markers',
                    name='DCG',
                    line=dict(color=colors['DCG'], width=2),
                    marker=dict(size=8)
                ))
                
                fig_dcg.update_layout(
                    title=f'DCG (без нормализации){title_suffix}',
                    xaxis_title='Top-K',
                    yaxis_title='DCG',
                    hovermode='x unified',
                    height=400
                )
                
                st.plotly_chart(fig_dcg, use_container_width=True)
                
                st.subheader("📉 Распределение метрик по запросам")
                
                per_query = results.get('per_query_metrics', [])
                
                if per_query and len(per_query) > 0:
                    default_k = k_values[len(k_values) // 2] if len(k_values) > 0 else k_values[0] if k_values else 10
                    
                    metric_options = []
                    for metric_name in ['NDCG', 'P', 'ERR', 'DCG']:
                        for k in k_values:
                            metric_options.append(f"{metric_name}@{k}")
                    
                    default_metric = f"NDCG@{default_k}"
                    default_index = metric_options.index(default_metric) if default_metric in metric_options else 0
                    
                    metric_to_plot = st.selectbox(
                        "Выберите метрику для детального просмотра:",
                        metric_options,
                        index=default_index,
                        key="metric_to_plot_select"
                    )
                    
                    metric_name, k_str = metric_to_plot.split('@')
                    k_value = int(k_str)
                    
                    query_numbers = []
                    metric_values = []
                    query_texts = []
                    
                    for i, query_result in enumerate(per_query, 1):
                        query_metrics = query_result.get('metrics', {})
                        value = query_metrics.get(metric_name, {}).get(k_value, 0.0)
                        
                        query_numbers.append(i)
                        metric_values.append(value)
                        query_texts.append(query_result['query'][:50] + "...")
                    
                    fig_per_query = go.Figure()
                    
                    fig_per_query.add_trace(go.Scatter(
                        x=query_numbers,
                        y=metric_values,
                        mode='markers+lines',
                        name=metric_to_plot,
                        marker=dict(
                            size=6,
                            color=metric_values,
                            colorscale='Viridis',
                            showscale=True,
                            colorbar=dict(title=metric_to_plot)
                        ),
                        line=dict(width=1, color='rgba(100,100,100,0.3)'),
                        text=query_texts,
                        hovertemplate='<b>Запрос %{x}</b><br>' +
                                      '%{text}<br>' +
                                      f'{metric_to_plot}: %{{y:.4f}}<br>' +
                                      '<extra></extra>'
                    ))
                    
                    avg_value = sum(metric_values) / len(metric_values) if metric_values else 0
                    fig_per_query.add_hline(
                        y=avg_value,
                        line_dash="dash",
                        line_color="red",
                        annotation_text=f"Среднее: {avg_value:.4f}",
                        annotation_position="right"
                    )
                    
                    n_queries = len(query_numbers)
                    if n_queries <= 20:
                        tick_step = 1
                    elif n_queries <= 50:
                        tick_step = 5
                    else:
                        tick_step = max(1, n_queries // 10)
                    
                    tickvals = list(range(1, n_queries + 1, tick_step))
                    if n_queries not in tickvals:
                        tickvals.append(n_queries)
                    
                    fig_per_query.update_layout(
                        title=f'{metric_to_plot} для каждого запроса{title_suffix}',
                        xaxis_title='Номер запроса',
                        yaxis_title=metric_to_plot,
                        hovermode='closest',
                        height=500,
                        showlegend=True,
                        xaxis=dict(
                            tickmode='array',
                            tickvals=tickvals,
                            ticktext=[str(v) for v in tickvals],
                            range=[0, n_queries + 1]
                        )
                    )
                    
                    st.plotly_chart(fig_per_query, use_container_width=True)
                    
                    col1, col2, col3, col4 = st.columns(4)
                    with col1:
                        st.metric("Среднее", f"{avg_value:.4f}")
                    with col2:
                        st.metric("Минимум", f"{min(metric_values):.4f}")
                    with col3:
                        st.metric("Максимум", f"{max(metric_values):.4f}")
                    with col4:
                        import statistics
                        std_dev = statistics.stdev(metric_values) if len(metric_values) > 1 else 0
                        st.metric("Ст. откл.", f"{std_dev:.4f}")
                
                with st.expander("🔍 Детальные результаты по запросам"):
                    per_query = results.get('per_query_metrics', [])
                    
                    if per_query:
                        st.info(f"Всего оценено запросов: {len(per_query)}")
                        
                        num_to_show = st.slider(
                            "Показать запросов:",
                            min_value=5,
                            max_value=min(len(per_query), 100),
                            value=min(20, len(per_query)),
                            step=5,
                            key="num_queries_slider"
                        )
                        
                        display_k = st.selectbox(
                            "Отображать метрики для K:",
                            k_values,
                            index=min(2, len(k_values) - 1),
                            key="display_k_select"
                        )
                        
                        for i, query_result in enumerate(per_query[:num_to_show], 1):
                            st.markdown(f"**{i}. {query_result['query']}**")
                            
                            query_metrics = query_result.get('metrics', {})
                            
                            mini_data = {'Метрика': [f'P@{display_k}', f'DCG@{display_k}', f'NDCG@{display_k}', f'ERR@{display_k}']}
                            mini_data['Значение'] = [
                                query_metrics.get('P', {}).get(display_k, 0.0),
                                query_metrics.get('DCG', {}).get(display_k, 0.0),
                                query_metrics.get('NDCG', {}).get(display_k, 0.0),
                                query_metrics.get('ERR', {}).get(display_k, 0.0)
                            ]
                            
                            st.dataframe(
                                pd.DataFrame(mini_data).style.format({'Значение': "{:.4f}"}),
                                hide_index=True
                            )
                        
                        if num_to_show < len(per_query):
                            st.info(f"Отображено {num_to_show} из {len(per_query)} запросов. Увеличьте слайдер для просмотра большего количества.")



def main():
    st.set_page_config(
        page_title="Poetry Search Engine",
        page_icon="📜",
        layout="wide",
    )
    
    st.title("📜 Poetry Search Engine")
    st.markdown("*Поисковая система по корпусу поэзии (C++ backend + Streamlit UI)*")
    
    if "app" not in st.session_state:
        init_container = st.empty()
        
        with init_container.container():
            progress_bar = st.progress(0)
            status_text = st.empty()
            
            def update_progress(progress: float, text: str):
                progress_bar.progress(progress)
                status_text.text(text)
            
            status_text.text("Подключение к базе данных...")
            st.session_state.app = SearchApp(progress_callback=update_progress)
        
        init_container.empty()
    
    app = st.session_state.app
    
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
        - Метрики качества (P, DCG, NDCG, ERR)
        """)
    
    tab1, tab2 = st.tabs(["🔍 Поиск", "📊 Метрики"])
    
    with tab1:
        render_search_tab(app)
    
    with tab2:
        render_metrics_tab(app)
    
    if not app.engine_available:
        st.warning("""
        ⚠️ **C++ поисковая система недоступна.**
        
        Убедитесь, что:
        1. Библиотека `libsearch_engine.so` скомпилирована
        2. Путь к библиотеке указан в переменной `LIB_PATH`
        """)



if __name__ == "__main__":
    main()
