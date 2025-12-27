"""
Streamlit UI для поисковой системы по поэзии.
"""
import os
import streamlit as st
from typing import List, Optional
from dataclasses import dataclass
from pymongo import MongoClient

# Попытка импорта C++ моста (может не работать без библиотеки)
try:
    from search_bridge import SearchEngine, SearchResult
    SEARCH_ENGINE_AVAILABLE = True
except Exception as e:
    SEARCH_ENGINE_AVAILABLE = False
    print(f"Warning: C++ search engine not available: {e}")


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
    
    def __init__(self):
        self.mongo_uri = os.getenv("MONGO_URI", "mongodb://mongodb:27017")
        self.db_name = os.getenv("DB_NAME", "poetry_search")
        self.lib_path = os.getenv("LIB_PATH", None)
        
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
        except Exception as e:
            print(f"MongoDB not available: {e}")
            self.mongo_available = False
            self.collection = None
    
    def _init_search_engine(self):
        """Инициализация C++ поисковой системы."""
        if SEARCH_ENGINE_AVAILABLE:
            try:
                self.search_engine = SearchEngine(lib_path=self.lib_path)
                self.engine_available = True
                
                # Если MongoDB доступна, загружаем документы в индекс
                if self.mongo_available and self.search_engine.get_document_count() == 0:
                    self._load_index_from_mongo()
            except Exception as e:
                print(f"Search engine error: {e}")
                self.search_engine = None
                self.engine_available = False
        else:
            self.search_engine = None
            self.engine_available = False
    
    def _load_index_from_mongo(self, limit: int = 10000):
        """Загружает документы из MongoDB в C++ индекс."""
        if not self.collection or not self.search_engine:
            return
        
        cursor = self.collection.find().limit(limit)
        for doc in cursor:
            content = doc.get("text", "")
            title = doc.get("title", "")
            if content:
                cpp_id = self.search_engine.add_document(content, title)
                self.collection.update_one(
                    {"_id": doc["_id"]},
                    {"$set": {"cpp_doc_id": cpp_id}}
                )
    
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
    
    def search_boolean(self, query: str) -> List[DisplayResult]:
        """Булев поиск."""
        results = []
        
        if self.engine_available and self.search_engine:
            doc_ids = self.search_engine.boolean_query(query)
            
            for doc_id in doc_ids[:50]:  # Ограничиваем вывод
                doc = self._get_doc_by_cpp_id(doc_id)
                if doc:
                    results.append(DisplayResult(
                        doc_id=doc_id,
                        score=0,
                        title=doc.get("title", "Без названия"),
                        text=doc.get("text", "")[:500] + "...",
                        author=doc.get("author", "Неизвестен"),
                        year=doc.get("year", ""),
                    ))
        
        return results
    
    def _get_doc_by_cpp_id(self, cpp_id: int) -> Optional[dict]:
        """Получает документ из MongoDB по C++ ID."""
        if self.collection:
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
        
        if self.mongo_available and self.collection:
            stats["mongo_docs"] = self.collection.count_documents({})
        
        if self.engine_available and self.search_engine:
            stats["indexed_docs"] = self.search_engine.get_document_count()
        
        return stats


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
        with st.spinner("Инициализация поисковой системы..."):
            st.session_state.app = SearchApp()
    
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
        """)
    
    # Основной интерфейс поиска
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
                    results = app.search_boolean(query)
            
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

