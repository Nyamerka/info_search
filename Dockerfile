# Multi-stage build: сначала компилируем C++, потом запускаем Python

# Stage 1: Build C++ shared library
FROM gcc:13 AS cpp-builder

RUN apt-get update && apt-get install -y cmake

WORKDIR /build

COPY lib/ /build/lib/
COPY search_system/ /build/search_system/
COPY CMakeLists.txt /build/

RUN mkdir -p build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    cmake --build . --target search_engine -j$(nproc)

# Stage 2: Python runtime with Streamlit
FROM python:3.11-slim

WORKDIR /app

# Копируем скомпилированную библиотеку
COPY --from=cpp-builder /build/build/search_system/libsearch_engine.so /app/lib/

# Устанавливаем Python зависимости
COPY server/requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Копируем Python код
COPY server/ /app/server/

# Создаём папку для данных
RUN mkdir -p /app/data

ENV LIB_PATH=/app/lib/libsearch_engine.so
ENV PYTHONPATH=/app/server
ENV MONGO_URI=mongodb://mongodb:27017
ENV DB_NAME=poetry_search

EXPOSE 8501

CMD ["streamlit", "run", "server/app.py", "--server.port=8501", "--server.address=0.0.0.0"]

