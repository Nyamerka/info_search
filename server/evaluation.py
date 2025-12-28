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
from typing import List, Dict, Tuple, Optional, Callable
from dataclasses import dataclass
from pymongo import MongoClient
from metrics import calculate_all_metrics, average_metrics


logger = logging.getLogger(__name__)


@dataclass
class TestQuery:
    """Тестовый запрос с известной релевантностью результатов."""
    query: str
    doc_relevance: Dict[int, int]
    description: str = ""


def generate_k_values(top_k: int) -> List[int]:
    """
    Генерирует список значений k для метрик на основе top_k.
    """
    if top_k <= 10:
        return [k for k in [1, 3, 5, 10] if k <= top_k]
    
    k_values = [1]
    
    step = max(1, top_k // 10)
    
    for k in range(step, top_k, step):
        if k not in k_values:
            k_values.append(k)
    
    if top_k not in k_values:
        k_values.append(top_k)
    
    return sorted(k_values)


class SearchEvaluator:
    """Класс для оценки качества поисковой системы."""
    
    def __init__(
        self,
        search_app,
        test_queries: Optional[List[TestQuery]] = None,
        progress_callback: Optional[Callable[[float, str], None]] = None
    ):
        self.search_app = search_app
        self.test_queries = test_queries or []
        self.logger = logging.getLogger(__name__)
        self.progress_callback = progress_callback
    
    def _report_progress(self, progress: float, message: str):
        """Сообщает о прогрессе через callback."""
        self.logger.info(f"[{progress*100:.1f}%] {message}")
        if self.progress_callback:
            self.progress_callback(progress, message)
    
    def load_test_queries_from_file(self, filepath: str):
        """
        Загружает тестовые запросы из JSON файла.
        
        Формат файла:
        [
            {
                "query": "love and heart",
                "description": "poems about love",
                "relevance": {
                    "doc_id_1": 2,
                    "doc_id_2": 1
                }
            }
        ]
        """
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                data = json.load(f)
            
            for item in data:
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
        
        common_words = [
            'love', 'heart', 'soul', 'death', 'life', 'dream', 'night', 'day',
            'time', 'world', 'beauty', 'nature', 'god', 'man', 'woman', 'child',
            'hope', 'fear', 'joy', 'pain', 'tears', 'light', 'dark', 'wind',
            'sea', 'sky', 'earth', 'sun', 'moon', 'star', 'flower', 'tree'
        ]
        
        self._report_progress(0.0, f"Генерация {n_queries} синтетических запросов...")
        
        for i in range(n_queries):
            query_length = random.randint(1, 3)
            query_words = random.sample(common_words, query_length)
            query = ' '.join(query_words)
            
            results = self.search_app.search_tfidf(query, top_k=top_k)
            
            if not results:
                continue
            
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
            
            if (i + 1) % 10 == 0 or i == n_queries - 1:
                progress = (i + 1) / n_queries * 0.3
                self._report_progress(progress, f"Сгенерировано запросов: {i + 1}/{n_queries}")
        
        self.logger.info(f"Generated {len(synthetic_queries)} synthetic queries")
        return synthetic_queries
    
    def evaluate_query(
        self,
        test_query: TestQuery,
        search_mode: str = 'tfidf',
        top_k: int = 10,
        k_values: Optional[List[int]] = None
    ) -> Tuple[List[int], Dict]:
        """
        Оценивает качество поиска для одного запроса.
        
        Returns:
            (relevance_list, metrics_dict)
        """
        if search_mode == 'tfidf':
            results = self.search_app.search_tfidf(test_query.query, top_k=top_k)
        else:
            results = self.search_app.search_boolean(test_query.query, top_k=top_k)
        
        relevance_list = []
        for result in results:
            relevance = test_query.doc_relevance.get(result.doc_id, 0)
            relevance_list.append(relevance)
        
        if k_values is None:
            k_values = generate_k_values(top_k)
        
        metrics = calculate_all_metrics(relevance_list, k_values=k_values, max_grade=2)
        
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
        queries_to_test = self.test_queries.copy()
        
        if use_synthetic and n_synthetic > 0:
            synthetic = self.generate_synthetic_queries(n_queries=n_synthetic, top_k=top_k)
            queries_to_test.extend(synthetic)
        
        if not queries_to_test:
            self.logger.warning("No test queries available")
            return {}
        
        k_values = generate_k_values(top_k)
        self.logger.info(f"Using k_values: {k_values} for top_k={top_k}")
        
        all_relevance_lists = []
        per_query_results = []
        
        total_queries = len(queries_to_test)
        
        for i, test_query in enumerate(queries_to_test):
            progress = 0.3 + (i / total_queries) * 0.7
            self._report_progress(progress, f"Оценка запроса {i+1}/{total_queries}: {test_query.query[:30]}...")
            
            try:
                relevance_list, metrics = self.evaluate_query(
                    test_query,
                    search_mode=search_mode,
                    top_k=top_k,
                    k_values=k_values
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
        
        avg_metrics = average_metrics(all_relevance_lists, k_values=k_values, max_grade=2)
        
        self._report_progress(1.0, f"Готово! Оценено {len(per_query_results)} запросов")
        
        return {
            'avg_metrics': avg_metrics,
            'per_query_metrics': per_query_results,
            'n_queries': len(queries_to_test),
            'search_mode': search_mode,
            'k_values': k_values
        }
