# Poetry Search Engine

Поисковая система по корпусу поэзии для курса "Информационный поиск" МАИ.

## Архитектура

```
┌─────────────────────────────────────────────────────────────┐
│                    Streamlit UI (Python)                    │
│                        app.py                               │
└──────────────────────────┬──────────────────────────────────┘
                           │ ctypes FFI
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                  libsearch_engine.so (C++)                  │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────────┐    │
│  │ TSearchDB   │  │ TInvertedIdx │  │ TBooleanSearch │    │
│  └─────────────┘  └──────────────┘  └─────────────────┘    │
│  ┌─────────────┐  ┌──────────────┐  ┌─────────────────┐    │
│  │ TTfIdf      │  │ TPorterStem  │  │ TLzw (сжатие)  │    │
│  └─────────────┘  └──────────────┘  └─────────────────┘    │
└─────────────────────────────────────────────────────────────┘
                           │
                           ▼
┌─────────────────────────────────────────────────────────────┐
│                      MongoDB (документы)                     │
└─────────────────────────────────────────────────────────────┘
```

## Реализованные компоненты

### C++ (lib/)

| Компонент | Описание |
|-----------|----------|
| `TVector`, `TList`, `TDeque`, `TQueue`, `THeap` | STL-подобные контейнеры без STL |
| `TUnorderedMap`, `TUnorderedSet` | Хеш-таблицы (Robin Hood) |
| `TMap`, `TSet` | Красно-чёрные деревья |
| `TString` | Строка с SSO и FNV-1a хешем |
| `TTokenizer` | Токенизация текста |
| `TPorterStemmer`, `TLemmatizer` | Стемминг / лемматизация |
| `TInvertedIndex` | Инвертированный индекс |
| `TBooleanSearch` | Булев поиск (AND/OR/NOT) |
| `TTfIdf` | TF-IDF ранжирование |
| `TZipfAnalyzer` | Анализ по закону Ципфа |
| `TLzw` | LZW-сжатие |
| `TSearchDatabase` | Высокоуровневая БД документов |

### Python (server/)

- **app.py** — Streamlit веб-интерфейс
- **cli.py** — CLI утилита (stdin → stdout)
- **search_bridge.py** — ctypes обёртка над C++
- **data_loader.py** — загрузка JSONL в MongoDB и индексацию

## Сборка

```bash
# Сборка C++
mkdir build && cd build
cmake ..
cmake --build .

# Запуск тестов
ctest -j4 --output-on-failure
```

## Запуск с Docker

```bash
# Положить датасет
cp ~/Desktop/poetry50k.dedup.jsonl data/

# Запустить MongoDB + Streamlit
docker-compose up -d

# Загрузить данные (один раз)
docker-compose run --rm data_loader

# Открыть http://localhost:8501
```

## CLI использование

```bash
# TF-IDF поиск
echo "eternal love" | python server/cli.py --mode tfidf --top-k 10

# Булев поиск
echo "love AND heart" | python server/cli.py --mode boolean

# Интерактивный режим
python server/cli.py --interactive
```

## Требования лабораторной

- [x] 🔴 Токенизация
- [x] 🔴 Стемминг
- [x] 🟢 Лемматизация
- [x] 🔴 Закон Ципфа
- [x] 🔴 Булев индекс
- [x] 🔴 Булев поиск
- [x] 🟡 TF-IDF
- [x] 🟡 Сжатие (LZW)
- [x] 🟡 Автотесты (288 юнит-тестов)

## Датасет

Poetry 50K: https://ciir.cs.umass.edu/downloads/poetry/

570,930 уникальных стихотворений.

