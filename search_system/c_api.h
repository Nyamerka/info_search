#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * C API для поисковой системы (для вызова из Python/Go через FFI)
 */

typedef void* SearchDBHandle;

typedef struct {
    size_t doc_id;
    double score;
} SearchResult;

typedef struct {
    SearchResult* results;
    size_t count;
} SearchResultList;

typedef struct {
    size_t* doc_ids;
    size_t count;
} DocIdList;

SearchDBHandle search_db_create(int use_stemming, int use_compression);
void search_db_destroy(SearchDBHandle handle);

size_t search_db_add_document(SearchDBHandle handle, const char* content, const char* title);
const char* search_db_get_document(SearchDBHandle handle, size_t doc_id);
const char* search_db_get_title(SearchDBHandle handle, size_t doc_id);
size_t search_db_get_document_count(SearchDBHandle handle);

SearchResultList* search_db_search_tfidf(SearchDBHandle handle, const char* query, size_t top_k);
void search_result_list_free(SearchResultList* list);

DocIdList* search_db_boolean_query(SearchDBHandle handle, const char* query);
void doc_id_list_free(DocIdList* list);

const char* search_db_compress_text(const char* text);
const char* search_db_decompress_text(const char* compressed);
void search_db_free_string(const char* str);

#ifdef __cplusplus
}
#endif

