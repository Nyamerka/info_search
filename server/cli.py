#!/usr/bin/env python3
"""
CLI утилита командной строки для поисковой системы.
Получает запросы со стандартного ввода, выдаёт результат в стандартный вывод.

Использование:
    echo "love AND heart" | python cli.py --mode boolean
    echo "eternal love" | python cli.py --mode tfidf --top-k 10
    python cli.py --interactive
"""
import sys
import os
import argparse
import json
from typing import Optional

from search_bridge import SearchEngine
from data_loader import DataLoader


def format_result(doc_id: int, score: float, title: str, text: str, fmt: str = "text") -> str:
    """Форматирует результат поиска."""
    if fmt == "json":
        return json.dumps({
            "doc_id": doc_id,
            "score": score,
            "title": title,
            "text": text[:200],
        }, ensure_ascii=False)
    else:
        return f"[{doc_id}] (score: {score:.4f}) {title}\n{text[:200]}...\n"


def run_search(
    engine: SearchEngine,
    loader: DataLoader,
    query: str,
    mode: str = "tfidf",
    top_k: int = 10,
    output_format: str = "text",
) -> None:
    """Выполняет поиск и выводит результаты."""
    if mode == "boolean":
        doc_ids = engine.boolean_query(query)
        for doc_id in doc_ids[:top_k]:
            mongo_doc = loader.get_mongo_doc_by_cpp_id(doc_id)
            if mongo_doc:
                print(format_result(
                    doc_id=doc_id,
                    score=0,
                    title=mongo_doc.get("title", ""),
                    text=mongo_doc.get("text", ""),
                    fmt=output_format,
                ))
    else:
        results = engine.search_tfidf(query, top_k)
        for r in results:
            mongo_doc = loader.get_mongo_doc_by_cpp_id(r.doc_id)
            if mongo_doc:
                print(format_result(
                    doc_id=r.doc_id,
                    score=r.score,
                    title=mongo_doc.get("title", ""),
                    text=mongo_doc.get("text", ""),
                    fmt=output_format,
                ))


def main():
    parser = argparse.ArgumentParser(
        description="Poetry Search CLI - поисковая система по поэзии"
    )
    parser.add_argument(
        "--mode", "-m",
        choices=["tfidf", "boolean"],
        default="tfidf",
        help="Режим поиска: tfidf (TF-IDF ранжирование) или boolean (AND/OR/NOT)"
    )
    parser.add_argument(
        "--top-k", "-k",
        type=int,
        default=10,
        help="Количество результатов (по умолчанию: 10)"
    )
    parser.add_argument(
        "--format", "-f",
        choices=["text", "json"],
        default="text",
        help="Формат вывода: text или json"
    )
    parser.add_argument(
        "--interactive", "-i",
        action="store_true",
        help="Интерактивный режим"
    )
    parser.add_argument(
        "--mongo-uri",
        default=os.getenv("MONGO_URI", "mongodb://mongodb:27017"),
        help="MongoDB URI"
    )
    parser.add_argument(
        "--lib-path",
        default=os.getenv("LIB_PATH"),
        help="Путь к libsearch_engine.so"
    )
    parser.add_argument(
        "--query", "-q",
        help="Запрос (если не указан, читает из stdin)"
    )
    
    args = parser.parse_args()
    
    # Инициализация
    try:
        loader = DataLoader(mongo_uri=args.mongo_uri)
        loader.init_search_engine(lib_path=args.lib_path)
        engine = loader.search_engine
        
        # Загружаем индекс из MongoDB если он пуст
        if engine.get_document_count() == 0:
            print("Загрузка индекса из MongoDB...", file=sys.stderr)
            loader.index_from_mongo(limit=50000)
            print(f"Загружено {engine.get_document_count()} документов", file=sys.stderr)
    except Exception as e:
        print(f"Ошибка инициализации: {e}", file=sys.stderr)
        sys.exit(1)
    
    if args.interactive:
        print("Poetry Search CLI (введите 'exit' для выхода)", file=sys.stderr)
        print(f"Режим: {args.mode}, Top-K: {args.top_k}", file=sys.stderr)
        
        while True:
            try:
                query = input(">>> ")
                if query.lower() in ("exit", "quit", "q"):
                    break
                if query.strip():
                    run_search(engine, loader, query, args.mode, args.top_k, args.format)
                    print()
            except EOFError:
                break
            except KeyboardInterrupt:
                print("\nВыход.", file=sys.stderr)
                break
    else:
        if args.query:
            query = args.query
        else:
            query = sys.stdin.read().strip()
        
        if query:
            run_search(engine, loader, query, args.mode, args.top_k, args.format)
    
    loader.close()


if __name__ == "__main__":
    main()

