"""
Python обёртка над C++ поисковой системой через ctypes.
"""
import ctypes
import os
from dataclasses import dataclass
from typing import List, Optional


@dataclass
class SearchResult:
    doc_id: int
    score: float


class SearchResultStruct(ctypes.Structure):
    _fields_ = [
        ("doc_id", ctypes.c_size_t),
        ("score", ctypes.c_double),
    ]


class SearchResultListStruct(ctypes.Structure):
    _fields_ = [
        ("results", ctypes.POINTER(SearchResultStruct)),
        ("count", ctypes.c_size_t),
    ]


class DocIdListStruct(ctypes.Structure):
    _fields_ = [
        ("doc_ids", ctypes.POINTER(ctypes.c_size_t)),
        ("count", ctypes.c_size_t),
    ]


class SearchEngine:
    """Обёртка над C++ TSearchDatabase."""

    def __init__(
        self,
        lib_path: Optional[str] = None,
        use_stemming: bool = True,
        use_compression: bool = True,
    ):
        if lib_path is None:
            lib_path = self._find_library()

        self._lib = ctypes.CDLL(lib_path)
        self._setup_functions()
        self._handle = self._lib.search_db_create(
            ctypes.c_int(1 if use_stemming else 0),
            ctypes.c_int(1 if use_compression else 0),
        )

    def _find_library(self) -> str:
        """Ищет скомпилированную библиотеку."""
        possible_paths = [
            "/app/lib/libsearch_engine.so",
            "/app/build/search_system/libsearch_engine.so",
            "/app/build/search_system/libsearch_engine.dylib",
            os.path.join(
                os.path.dirname(__file__),
                "..",
                "build",
                "search_system",
                "libsearch_engine.so",
            ),
            os.path.join(
                os.path.dirname(__file__),
                "..",
                "build",
                "search_system",
                "libsearch_engine.dylib",
            ),
        ]
        for path in possible_paths:
            if os.path.exists(path):
                return path
        raise FileNotFoundError(
            f"Cannot find libsearch_engine. Tried: {possible_paths}"
        )

    def _setup_functions(self):
        """Настройка сигнатур функций."""
        self._lib.search_db_create.argtypes = [ctypes.c_int, ctypes.c_int]
        self._lib.search_db_create.restype = ctypes.c_void_p

        self._lib.search_db_destroy.argtypes = [ctypes.c_void_p]
        self._lib.search_db_destroy.restype = None

        self._lib.search_db_add_document.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_char_p,
        ]
        self._lib.search_db_add_document.restype = ctypes.c_size_t

        self._lib.search_db_get_document.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
        self._lib.search_db_get_document.restype = ctypes.c_char_p

        self._lib.search_db_get_title.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
        self._lib.search_db_get_title.restype = ctypes.c_char_p

        self._lib.search_db_get_document_count.argtypes = [ctypes.c_void_p]
        self._lib.search_db_get_document_count.restype = ctypes.c_size_t

        self._lib.search_db_search_tfidf.argtypes = [
            ctypes.c_void_p,
            ctypes.c_char_p,
            ctypes.c_size_t,
        ]
        self._lib.search_db_search_tfidf.restype = ctypes.POINTER(SearchResultListStruct)

        self._lib.search_result_list_free.argtypes = [
            ctypes.POINTER(SearchResultListStruct)
        ]
        self._lib.search_result_list_free.restype = None

        self._lib.search_db_boolean_query.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        self._lib.search_db_boolean_query.restype = ctypes.POINTER(DocIdListStruct)

        self._lib.doc_id_list_free.argtypes = [ctypes.POINTER(DocIdListStruct)]
        self._lib.doc_id_list_free.restype = None

        self._lib.search_db_free_string.argtypes = [ctypes.c_char_p]
        self._lib.search_db_free_string.restype = None

    def __del__(self):
        if hasattr(self, "_handle") and self._handle:
            self._lib.search_db_destroy(self._handle)

    def add_document(self, content: str, title: str = "") -> int:
        """Добавить документ в индекс."""
        return self._lib.search_db_add_document(
            self._handle,
            content.encode("utf-8"),
            title.encode("utf-8") if title else None,
        )

    def get_document(self, doc_id: int) -> str:
        """Получить содержимое документа по ID."""
        result = self._lib.search_db_get_document(self._handle, ctypes.c_size_t(doc_id))
        if result:
            text = result.decode("utf-8")
            self._lib.search_db_free_string(result)
            return text
        return ""

    def get_title(self, doc_id: int) -> str:
        """Получить заголовок документа по ID."""
        result = self._lib.search_db_get_title(self._handle, ctypes.c_size_t(doc_id))
        if result:
            text = result.decode("utf-8")
            self._lib.search_db_free_string(result)
            return text
        return ""

    def get_document_count(self) -> int:
        """Получить количество документов в индексе."""
        return self._lib.search_db_get_document_count(self._handle)

    def search_tfidf(self, query: str, top_k: int = 10) -> List[SearchResult]:
        """TF-IDF поиск."""
        result_list = self._lib.search_db_search_tfidf(
            self._handle,
            query.encode("utf-8"),
            ctypes.c_size_t(top_k),
        )

        results = []
        if result_list and result_list.contents:
            for i in range(result_list.contents.count):
                r = result_list.contents.results[i]
                results.append(SearchResult(doc_id=r.doc_id, score=r.score))
            self._lib.search_result_list_free(result_list)

        return results

    def boolean_query(self, query: str) -> List[int]:
        """Булев поиск (AND, OR, NOT, скобки)."""
        result_list = self._lib.search_db_boolean_query(
            self._handle,
            query.encode("utf-8"),
        )

        doc_ids = []
        if result_list and result_list.contents:
            for i in range(result_list.contents.count):
                doc_ids.append(result_list.contents.doc_ids[i])
            self._lib.doc_id_list_free(result_list)

        return doc_ids

