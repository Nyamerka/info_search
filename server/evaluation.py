"""
Оценка качества поиска на тестовых запросах.

Функционал:
- Загрузка тестовых запросов с оценками релевантности
- Автоматическая генерация синтетических тестовых запросов
- Вычисление метрик для поисковой системы
- Визуализация результатов
"""
import json
import random
import logging
from typing import List, Dict, Tuple, Optional
from dataclasses import dataclass
from pymongo import MongoClient
from metrics import calculate_all_metrics, average_metrics


logger = logging.getLogger(__name__)


@dataclass
class TestQuery:
    """Тестовый запрос с известной релевантностью результатов."""
    query: str
    doc_relevance: Dict[int, int]  # {doc_id: relevance_grade}
    description: str = ""


class SearchEvaluator:
    """Класс для оценки качества поисковой системы."""
    
    def __init__(self, search_app, test_queries: Optional[List[TestQuery]] = None):
        self.search_app = search_app
        self.test_queries = test_queries or []
        self.logger = logging.getLogger(__name__)
    
    def load_test_queries_from_file(self, filepath: str):
        """
        Загружает тестовые запросы из JSON файла.
        
        Формат файла:
        [
            {
                "query": "love and heart",
                "description": "poems about love",
                "relevance": {
                    "doc_id_1": 2,  # 0-4 scale
                    "doc_id_2": 1
                }
            }
        ]
        """
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                data = json.load(f)
            
            for item in data:
                # Конвертируем строковые ключи в int
                doc_relevance = {
                    int(doc_id): grade
                    for doc_id, grade in item['relevance'].items()
                }
                
                test_query = TestQuery(
                    query=item['query'],
                    doc_relevance=doc_relevance,
                    description=item.get('description', '')
                )
                self.test_queries.append(test_query)
            
            self.logger.info(f"Loaded {len(data)} test queries from {filepath}")
        
        except FileNotFoundError:
            self.logger.warning(f"Test queries file not found: {filepath}")
        except json.JSONDecodeError as e:
            self.logger.error(f"Invalid JSON in {filepath}: {e}")
        except Exception as e:
            self.logger.error(f"Error loading test queries: {e}")
    
    def generate_synthetic_queries(self, n_queries: int = 10, top_k: int = 10) -> List[TestQuery]:
        """
        Генерирует синтетические тестовые запросы.
        
        Стратегия:
        1. Выбираем случайные слова из корпуса
        2. Делаем запрос
        3. Назначаем релевантность на основе TF-IDF скоров
        """
        synthetic_queries = []
        
        # Популярные слова для запросов
        common_words = [
            'love', 'heart', 'soul', 'death', 'life', 'dream', 'night', 'day',
            'time', 'world', 'beauty', 'nature', 'god', 'man', 'woman', 'child',
            'hope', 'fear', 'joy', 'pain', 'tears', 'light', 'dark', 'wind',
            'sea', 'sky', 'earth', 'sun', 'moon', 'star', 'flower', 'tree'
        ]
        
        for i in range(n_queries):
            # Генерируем запрос из 1-3 слов
            query_length = random.randint(1, 3)
            query_words = random.sample(common_words, query_length)
            query = ' '.join(query_words)
            
            # Выполняем поиск
            results = self.search_app.search_tfidf(query, top_k=top_k)
            
            if not results:
                continue
            
            # Назначаем релевантность на основе скоров
            # Верхние 20% - релевантность 2, следующие 30% - 1, остальные - 0
            doc_relevance = {}
            
            scores = [r.score for r in results]
            if max(scores) > 0:
                for r in results:
                    normalized_score = r.score / max(scores)
                    
                    if normalized_score >= 0.8:
                        relevance = 2
                    elif normalized_score >= 0.5:
                        relevance = 1
                    else:
                        relevance = 0
                    
                    doc_relevance[r.doc_id] = relevance
            
            test_query = TestQuery(
                query=query,
                doc_relevance=doc_relevance,
                description=f"Synthetic query #{i+1}"
            )
            synthetic_queries.append(test_query)
        
        self.logger.info(f"Generated {len(synthetic_queries)} synthetic queries")
        return synthetic_queries
    
    def evaluate_query(
        self,
        test_query: TestQuery,
        search_mode: str = 'tfidf',
        top_k: int = 10
    ) -> Tuple[List[int], Dict]:
        """
        Оценивает качество поиска для одного запроса.
        
        Returns:
            (relevance_list, metrics_dict)
        """
        # Выполняем поиск
        if search_mode == 'tfidf':
            results = self.search_app.search_tfidf(test_query.query, top_k=top_k)
        else:
            results = self.search_app.search_boolean(test_query.query, top_k=top_k)
        
        # Формируем список релевантностей в порядке результатов
        relevance_list = []
        for result in results:
            relevance = test_query.doc_relevance.get(result.doc_id, 0)
            relevance_list.append(relevance)
        
        # Вычисляем метрики
        metrics = calculate_all_metrics(relevance_list, k_values=[1, 3, 5, 10], max_grade=2)
        
        return relevance_list, metrics
    
    def evaluate_all(
        self,
        search_mode: str = 'tfidf',
        top_k: int = 10,
        use_synthetic: bool = True,
        n_synthetic: int = 20
    ) -> Dict:
        """
        Оценивает качество поиска на всех тестовых запросах.
        
        Returns:
            {
                'avg_metrics': Dict[str, Dict[int, float]],
                'per_query_metrics': List[Dict],
                'queries': List[str]
            }
        """
        # Объединяем реальные и синтетические запросы
        queries_to_test = self.test_queries.copy()
        
        if use_synthetic and n_synthetic > 0:
            synthetic = self.generate_synthetic_queries(n_queries=n_synthetic, top_k=top_k)
            queries_to_test.extend(synthetic)
        
        if not queries_to_test:
            self.logger.warning("No test queries available")
            return {}
        
        # Оцениваем каждый запрос
        all_relevance_lists = []
        per_query_results = []
        
        for i, test_query in enumerate(queries_to_test):
            self.logger.info(f"Evaluating query {i+1}/{len(queries_to_test)}: {test_query.query}")
            
            try:
                relevance_list, metrics = self.evaluate_query(
                    test_query,
                    search_mode=search_mode,
                    top_k=top_k
                )
                
                all_relevance_lists.append(relevance_list)
                per_query_results.append({
                    'query': test_query.query,
                    'description': test_query.description,
                    'metrics': metrics,
                    'relevance': relevance_list
                })
            
            except Exception as e:
                self.logger.error(f"Error evaluating query '{test_query.query}': {e}")
                continue
        
        # Усредняем метрики
        avg_metrics = average_metrics(all_relevance_lists, k_values=[1, 3, 5, 10], max_grade=2)
        
        return {
            'avg_metrics': avg_metrics,
            'per_query_metrics': per_query_results,
            'n_queries': len(queries_to_test),
            'search_mode': search_mode
        }