"""
Метрики качества информационного поиска.

Реализованы метрики:
- Precision@k (P@k)
- Discounted Cumulative Gain (DCG)
- Normalized DCG (NDCG)
- Expected Reciprocal Rank (ERR)

Используются для оценки качества ранжирования результатов поиска.
"""
import math
from typing import List, Dict, Tuple
import logging


logger = logging.getLogger(__name__)


def precision_at_k(relevance: List[int], k: int) -> float:
    """
    Precision@k - доля релевантных документов в топ-k выдаче.
    
    Args:
        relevance: Список оценок релевантности (0 - нерелевантен, >= 1 - релевантен)
        k: Количество рассматриваемых документов
    
    Returns:
        P@k в диапазоне [0, 1]
    """
    if k <= 0 or not relevance:
        return 0.0
    
    k = min(k, len(relevance))
    relevant_count = sum(1 for rel in relevance[:k] if rel > 0)
    
    return relevant_count / k


def dcg_at_k(relevance: List[int], k: int) -> float:
    """
    Discounted Cumulative Gain@k - учитывает позицию документов.
    
    DCG@k = sum(rel_i / log2(i + 1)) for i in [0, k-1]
    
    Args:
        relevance: Список оценок релевантности (неотрицательные числа)
        k: Количество рассматриваемых документов
    
    Returns:
        DCG@k значение
    """
    if k <= 0 or not relevance:
        return 0.0
    
    k = min(k, len(relevance))
    dcg = 0.0
    
    for i in range(k):
        
        dcg += relevance[i] / math.log2(i + 2)
    
    return dcg


def ndcg_at_k(relevance: List[int], k: int) -> float:
    """
    Normalized Discounted Cumulative Gain@k - DCG нормализованный к идеальной выдаче.
    
    NDCG@k = DCG@k / IDCG@k
    
    Args:
        relevance: Список оценок релевантности
        k: Количество рассматриваемых документов
    
    Returns:
        NDCG@k в диапазоне [0, 1]
    """
    if k <= 0 or not relevance:
        return 0.0
    
    dcg = dcg_at_k(relevance, k)
    
    
    ideal_relevance = sorted(relevance, reverse=True)
    idcg = dcg_at_k(ideal_relevance, k)
    
    if idcg == 0.0:
        return 0.0
    
    return dcg / idcg


def err_at_k(relevance: List[int], k: int, max_grade: int = 4) -> float:
    """
    Expected Reciprocal Rank@k - вероятностная метрика.
    
    Моделирует вероятность того, что пользователь найдет релевантный документ
    на каждой позиции, учитывая, что он мог удовлетвориться предыдущими.
    
    ERR@k = sum(1/(i+1) * P(user stops at position i))
    
    Args:
        relevance: Список оценок релевантности (0 до max_grade)
        k: Количество рассматриваемых документов
        max_grade: Максимальная оценка релевантности (для нормализации)
    
    Returns:
        ERR@k значение
    """
    if k <= 0 or not relevance or max_grade == 0:
        return 0.0
    
    k = min(k, len(relevance))
    err = 0.0
    p = 1.0  
    
    for i in range(k):
        
        
        r_i = (2 ** relevance[i] - 1) / (2 ** max_grade)
        
        
        err += p * r_i / (i + 1)
        
        
        p *= (1 - r_i)
    
    return err


def calculate_all_metrics(
    relevance: List[int],
    k_values: List[int] = [1, 3, 5, 10],
    max_grade: int = 4
) -> Dict[str, Dict[int, float]]:
    """
    Вычисляет все метрики для разных значений k.
    
    Args:
        relevance: Список оценок релевантности для результатов поиска
        k_values: Список значений k для вычисления метрик
        max_grade: Максимальная оценка релевантности (для ERR)
    
    Returns:
        Словарь {метрика: {k: значение}}
    """
    metrics = {
        'P': {},
        'DCG': {},
        'NDCG': {},
        'ERR': {}
    }
    
    for k in k_values:
        metrics['P'][k] = precision_at_k(relevance, k)
        metrics['DCG'][k] = dcg_at_k(relevance, k)
        metrics['NDCG'][k] = ndcg_at_k(relevance, k)
        metrics['ERR'][k] = err_at_k(relevance, k, max_grade)
    
    return metrics


def average_metrics(
    all_queries_relevance: List[List[int]],
    k_values: List[int] = [1, 3, 5, 10],
    max_grade: int = 4
) -> Dict[str, Dict[int, float]]:
    """
    Усредняет метрики по множеству запросов.
    
    Args:
        all_queries_relevance: Список релевантностей для разных запросов
        k_values: Список значений k
        max_grade: Максимальная оценка релевантности
    
    Returns:
        Усредненные метрики
    """
    if not all_queries_relevance:
        return {}
    
    
    sum_metrics = {
        'P': {k: 0.0 for k in k_values},
        'DCG': {k: 0.0 for k in k_values},
        'NDCG': {k: 0.0 for k in k_values},
        'ERR': {k: 0.0 for k in k_values}
    }
    
    
    for relevance in all_queries_relevance:
        query_metrics = calculate_all_metrics(relevance, k_values, max_grade)
        
        for metric_name in sum_metrics:
            for k in k_values:
                sum_metrics[metric_name][k] += query_metrics[metric_name][k]
    
    
    num_queries = len(all_queries_relevance)
    avg_metrics = {
        metric_name: {
            k: value / num_queries
            for k, value in k_values_dict.items()
        }
        for metric_name, k_values_dict in sum_metrics.items()
    }
    
    return avg_metrics


def format_metrics_table(metrics: Dict[str, Dict[int, float]]) -> str:
    """
    Форматирует метрики в виде текстовой таблицы.
    
    Args:
        metrics: Словарь метрик
    
    Returns:
        Форматированная строка таблицы
    """
    if not metrics:
        return "No metrics available"
    
    
    k_values = sorted(list(metrics[list(metrics.keys())[0]].keys()))
    
    
    header = "Metric\t" + "\t".join(f"@{k}" for k in k_values)
    lines = [header, "-" * len(header)]
    
    
    for metric_name in ['P', 'DCG', 'NDCG', 'ERR']:
        if metric_name in metrics:
            values = [f"{metrics[metric_name][k]:.4f}" for k in k_values]
            lines.append(f"{metric_name}\t" + "\t".join(values))
    
    return "\n".join(lines)