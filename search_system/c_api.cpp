#include "c_api.h"
#include "search_system.h"
#include <lib/lzw/lzw.h>
#include <cstring>
#include <cstdlib>
#include <memory>

using namespace NSearchSystem;
using namespace NTypes;
using namespace NCollections;
using namespace NLzw;

struct SearchDBWrapper {
    std::unique_ptr<TSearchDatabase> db;
    
    SearchDBWrapper(bool useStemming, bool useCompression) {
        TSearchDatabase::TOptions opts;
        opts.Pipeline.UseStemming = useStemming;
        opts.CompressDocuments = useCompression;
        db = std::make_unique<TSearchDatabase>(opts);
    }
};

static char* allocate_cstring(const TString& str) {
    char* result = static_cast<char*>(malloc(str.Size() + 1));
    if (result) {
        for (size_t i = 0; i < str.Size(); ++i) {
            result[i] = str[i];
        }
        result[str.Size()] = '\0';
    }
    return result;
}

extern "C" {

SearchDBHandle search_db_create(int use_stemming, int use_compression) {
    return new SearchDBWrapper(use_stemming != 0, use_compression != 0);
}

void search_db_destroy(SearchDBHandle handle) {
    delete static_cast<SearchDBWrapper*>(handle);
}

size_t search_db_add_document(SearchDBHandle handle, const char* content, const char* title) {
    auto* wrapper = static_cast<SearchDBWrapper*>(handle);
    TString contentStr(content ? content : "");
    TString titleStr(title ? title : "");
    return wrapper->db->AddDocument(contentStr, titleStr);
}

const char* search_db_get_document(SearchDBHandle handle, size_t doc_id) {
    auto* wrapper = static_cast<SearchDBWrapper*>(handle);
    TString doc = wrapper->db->GetDocument(doc_id);
    return allocate_cstring(doc);
}

const char* search_db_get_title(SearchDBHandle handle, size_t doc_id) {
    auto* wrapper = static_cast<SearchDBWrapper*>(handle);
    TString title = wrapper->db->GetTitle(doc_id);
    return allocate_cstring(title);
}

size_t search_db_get_document_count(SearchDBHandle handle) {
    auto* wrapper = static_cast<SearchDBWrapper*>(handle);
    return wrapper->db->GetDocumentCount();
}

SearchResultList* search_db_search_tfidf(SearchDBHandle handle, const char* query, size_t top_k) {
    auto* wrapper = static_cast<SearchDBWrapper*>(handle);
    TString queryStr(query ? query : "");
    
    auto results = wrapper->db->Search(queryStr, top_k);
    
    SearchResultList* list = static_cast<SearchResultList*>(malloc(sizeof(SearchResultList)));
    list->count = results.Size();
    list->results = static_cast<SearchResult*>(malloc(sizeof(SearchResult) * (results.Size() > 0 ? results.Size() : 1)));
    
    for (size_t i = 0; i < results.Size(); ++i) {
        list->results[i].doc_id = results[i].DocId;
        list->results[i].score = results[i].Score;
    }
    
    return list;
}

void search_result_list_free(SearchResultList* list) {
    if (list) {
        free(list->results);
        free(list);
    }
}

DocIdList* search_db_boolean_query(SearchDBHandle handle, const char* query) {
    auto* wrapper = static_cast<SearchDBWrapper*>(handle);
    TString queryStr(query ? query : "");
    
    auto docIds = wrapper->db->BooleanQuery(queryStr);
    
    DocIdList* list = static_cast<DocIdList*>(malloc(sizeof(DocIdList)));
    list->count = docIds.Size();
    list->doc_ids = static_cast<size_t*>(malloc(sizeof(size_t) * (docIds.Size() > 0 ? docIds.Size() : 1)));
    
    for (size_t i = 0; i < docIds.Size(); ++i) {
        list->doc_ids[i] = docIds[i];
    }
    
    return list;
}

void doc_id_list_free(DocIdList* list) {
    if (list) {
        free(list->doc_ids);
        free(list);
    }
}

const char* search_db_compress_text(const char* text) {
    if (!text) return nullptr;
    TString input(text);
    TLzw lzw;
    TLzw::TBytes compressed = lzw.Compress(input);
    
    TString hex;
    hex.Reserve(compressed.Size() * 2);
    static const char hexChars[] = "0123456789abcdef";
    for (size_t i = 0; i < compressed.Size(); ++i) {
        hex.PushBack(hexChars[(compressed[i] >> 4) & 0xF]);
        hex.PushBack(hexChars[compressed[i] & 0xF]);
    }
    return allocate_cstring(hex);
}

const char* search_db_decompress_text(const char* compressed) {
    if (!compressed) return nullptr;
    
    size_t len = 0;
    while (compressed[len]) ++len;
    if (len % 2 != 0) return nullptr;
    
    TLzw::TBytes bytes;
    bytes.Reserve(len / 2);
    for (size_t i = 0; i < len; i += 2) {
        unsigned char hi = 0, lo = 0;
        char c1 = compressed[i], c2 = compressed[i + 1];
        if (c1 >= '0' && c1 <= '9') hi = c1 - '0';
        else if (c1 >= 'a' && c1 <= 'f') hi = c1 - 'a' + 10;
        else if (c1 >= 'A' && c1 <= 'F') hi = c1 - 'A' + 10;
        if (c2 >= '0' && c2 <= '9') lo = c2 - '0';
        else if (c2 >= 'a' && c2 <= 'f') lo = c2 - 'a' + 10;
        else if (c2 >= 'A' && c2 <= 'F') lo = c2 - 'A' + 10;
        bytes.PushBack((hi << 4) | lo);
    }
    
    TLzw lzw;
    TString decompressed = lzw.Decompress(bytes);
    return allocate_cstring(decompressed);
}

void search_db_free_string(const char* str) {
    free(const_cast<char*>(str));
}

} // extern "C"
