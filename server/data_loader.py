"""
Скрипт для загрузки данных из JSONL в MongoDB и индексации в C++ поисковой системе.
"""
import json
import os
from typing import Iterator, Dict, Any, Optional
from dataclasses import dataclass
from pymongo import MongoClient
from pymongo.collection import Collection

from search_bridge import SearchEngine


@dataclass
class PoemDocument:
    """Документ из датасета poetry50k."""
    id: str
    text: str
    title: str = ""
    author: str = ""
    year: str = ""


def parse_jsonl(file_path: str, limit: Optional[int] = None) -> Iterator[Dict[str, Any]]:
    """Парсит JSONL файл и возвращает итератор документов."""
    with open(file_path, "r", encoding="utf-8") as f:
        for i, line in enumerate(f):
            if limit and i >= limit:
                break
            if line.strip():
                try:
                    yield json.loads(line)
                except json.JSONDecodeError:
                    continue


def extract_poem(raw: Dict[str, Any]) -> PoemDocument:
    """Извлекает структурированный документ из сырых данных."""
    text = raw.get("poem", raw.get("text", raw.get("content", "")))
    title = raw.get("title", raw.get("name", ""))
    author = raw.get("author", raw.get("poet", ""))
    year = str(raw.get("year", raw.get("date", "")))
    doc_id = raw.get("id", raw.get("_id", ""))
    
    if not doc_id:
        doc_id = str(hash(text[:100] if text else ""))
    
    return PoemDocument(
        id=str(doc_id),
        text=text,
        title=title,
        author=author,
        year=year,
    )


class DataLoader:
    """Загрузчик данных в MongoDB и C++ индекс."""
    
    def __init__(
        self,
        mongo_uri: str = "mongodb://mongodb:27017",
        db_name: str = "poetry_search",
        collection_name: str = "poems",
    ):
        self.mongo_client = MongoClient(mongo_uri)
        self.db = self.mongo_client[db_name]
        self.collection: Collection = self.db[collection_name]
        self.search_engine: Optional[SearchEngine] = None
    
    def init_search_engine(self, lib_path: Optional[str] = None):
        """Инициализирует C++ поисковую систему."""
        self.search_engine = SearchEngine(lib_path=lib_path)
    
    def load_jsonl_to_mongo(
        self,
        jsonl_path: str,
        batch_size: int = 1000,
        limit: Optional[int] = None,
    ) -> int:
        """Загружает данные из JSONL в MongoDB."""
        self.collection.drop()
        self.collection.create_index("external_id")
        
        batch = []
        total = 0
        
        for raw in parse_jsonl(jsonl_path, limit=limit):
            poem = extract_poem(raw)
            doc = {
                "external_id": poem.id,
                "title": poem.title,
                "text": poem.text,
                "author": poem.author,
                "year": poem.year,
            }
            batch.append(doc)
            
            if len(batch) >= batch_size:
                self.collection.insert_many(batch)
                total += len(batch)
                print(f"Loaded {total} documents to MongoDB...")
                batch = []
        
        if batch:
            self.collection.insert_many(batch)
            total += len(batch)
        
        print(f"Total loaded to MongoDB: {total}")
        return total
    
    def index_from_mongo(self, limit: Optional[int] = None) -> int:
        """Индексирует документы из MongoDB в C++ поисковую систему."""
        if not self.search_engine:
            raise RuntimeError("Search engine not initialized. Call init_search_engine first.")
        
        cursor = self.collection.find()
        if limit:
            cursor = cursor.limit(limit)
        
        total = 0
        for doc in cursor:
            content = doc.get("text", "")
            title = doc.get("title", "")
            
            if content:
                cpp_doc_id = self.search_engine.add_document(content, title)
                self.collection.update_one(
                    {"_id": doc["_id"]},
                    {"$set": {"cpp_doc_id": cpp_doc_id}}
                )
                total += 1
                
                if total % 1000 == 0:
                    print(f"Indexed {total} documents in C++...")
        
        print(f"Total indexed in C++: {total}")
        return total
    
    def get_mongo_doc_by_cpp_id(self, cpp_doc_id: int) -> Optional[Dict[str, Any]]:
        """Получает документ из MongoDB по C++ ID."""
        return self.collection.find_one({"cpp_doc_id": cpp_doc_id})
    
    def close(self):
        """Закрывает соединение с MongoDB."""
        self.mongo_client.close()


def main():
    """Основная функция для загрузки данных."""
    import argparse
    
    parser = argparse.ArgumentParser(description="Load poetry data into search system")
    parser.add_argument("--jsonl", default="/app/data/poetry50k.dedup.jsonl",
                       help="Path to JSONL file")
    parser.add_argument("--mongo-uri", default="mongodb://mongodb:27017",
                       help="MongoDB URI")
    parser.add_argument("--limit", type=int, default=None,
                       help="Limit number of documents to load")
    parser.add_argument("--lib-path", default=None,
                       help="Path to libsearch_engine.so")
    parser.add_argument("--skip-mongo", action="store_true",
                       help="Skip MongoDB loading, only index")
    args = parser.parse_args()
    
    loader = DataLoader(mongo_uri=args.mongo_uri)
    
    try:
        if not args.skip_mongo:
            if os.path.exists(args.jsonl):
                print(f"Loading data from {args.jsonl}...")
                loader.load_jsonl_to_mongo(args.jsonl, limit=args.limit)
            else:
                print(f"JSONL file not found: {args.jsonl}")
                return
        
        print("Initializing C++ search engine...")
        loader.init_search_engine(lib_path=args.lib_path)
        
        print("Indexing documents in C++...")
        loader.index_from_mongo(limit=args.limit)
        
        print("Done!")
    finally:
        loader.close()


if __name__ == "__main__":
    main()

