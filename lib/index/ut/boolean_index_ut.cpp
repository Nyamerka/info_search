#include <lib/index/boolean_index.h>
#include <lib/index/pipeline.h>
#include <gtest/gtest.h>

using namespace NIndex;
using NTypes::TString;
using NCollections::TVector;

TEST(TInvertedIndex, AddDocument) {
    TInvertedIndex index;
    TVector<TString> terms;
    terms.PushBack(TString("hello"));
    terms.PushBack(TString("world"));
    
    TDocId id = index.AddDocument(terms);
    
    EXPECT_EQ(id, 0);
    EXPECT_EQ(index.GetDocumentCount(), 1);
    EXPECT_EQ(index.GetTermCount(), 2);
}

TEST(TInvertedIndex, AddDocumentIterators) {
    TInvertedIndex index;
    TVector<TString> terms;
    terms.PushBack(TString("hello"));
    terms.PushBack(TString("world"));

    TDocId id = index.AddDocument(terms.begin(), terms.end());

    EXPECT_EQ(id, 0);
    EXPECT_EQ(index.GetDocumentCount(), 1);
    EXPECT_EQ(index.GetDocumentFrequency(TString("hello")), 1);
    EXPECT_EQ(index.GetDocumentFrequency(TString("world")), 1);
}

TEST(TInvertedIndex, GetPostingList) {
    TInvertedIndex index;
    
    TVector<TString> doc1;
    doc1.PushBack(TString("hello"));
    doc1.PushBack(TString("world"));
    index.AddDocument(doc1);
    
    TVector<TString> doc2;
    doc2.PushBack(TString("hello"));
    doc2.PushBack(TString("universe"));
    index.AddDocument(doc2);
    
    const TPostingList& helloList = index.GetPostingList(TString("hello"));
    EXPECT_EQ(helloList.Size(), 2);
    
    const TPostingList& worldList = index.GetPostingList(TString("world"));
    EXPECT_EQ(worldList.Size(), 1);
}

TEST(TInvertedIndex, TermFrequency) {
    TInvertedIndex index;
    
    TVector<TString> terms;
    terms.PushBack(TString("hello"));
    terms.PushBack(TString("hello"));
    terms.PushBack(TString("world"));
    
    TDocId docId = index.AddDocument(terms);
    
    EXPECT_EQ(index.GetTermFrequency(docId, TString("hello")), 2);
    EXPECT_EQ(index.GetTermFrequency(docId, TString("world")), 1);
    EXPECT_EQ(index.GetDocumentLength(docId), 3);
}

TEST(TBooleanSearch, SingleTerm) {
    TInvertedIndex index;
    
    TVector<TString> doc1;
    doc1.PushBack(TString("cat"));
    doc1.PushBack(TString("dog"));
    index.AddDocument(doc1);
    
    TVector<TString> doc2;
    doc2.PushBack(TString("cat"));
    doc2.PushBack(TString("bird"));
    index.AddDocument(doc2);
    
    TBooleanSearch search(index);
    TPostingList result = search.Search(TString("cat"));
    
    EXPECT_EQ(result.Size(), 2);
}

TEST(TBooleanSearch, AndSearch) {
    TInvertedIndex index;
    
    TVector<TString> doc1;
    doc1.PushBack(TString("cat"));
    doc1.PushBack(TString("dog"));
    index.AddDocument(doc1);
    
    TVector<TString> doc2;
    doc2.PushBack(TString("cat"));
    doc2.PushBack(TString("bird"));
    index.AddDocument(doc2);
    
    TVector<TString> doc3;
    doc3.PushBack(TString("fish"));
    index.AddDocument(doc3);
    
    TBooleanSearch search(index);
    
    TVector<TString> query;
    query.PushBack(TString("cat"));
    query.PushBack(TString("dog"));
    
    TPostingList result = search.SearchAnd(query);
    
    EXPECT_EQ(result.Size(), 1);
    EXPECT_EQ(result[0], 0);
}

TEST(TBooleanSearch, AndSearchIterators) {
    TInvertedIndex index;

    TVector<TString> doc1;
    doc1.PushBack(TString("cat"));
    doc1.PushBack(TString("dog"));
    index.AddDocument(doc1);

    TVector<TString> doc2;
    doc2.PushBack(TString("cat"));
    doc2.PushBack(TString("bird"));
    index.AddDocument(doc2);

    TBooleanSearch search(index);

    TVector<TString> query;
    query.PushBack(TString("cat"));
    query.PushBack(TString("dog"));

    TPostingList result = search.SearchAnd(query.begin(), query.end());

    ASSERT_EQ(result.Size(), 1);
    EXPECT_EQ(result[0], 0);
}

TEST(TBooleanSearch, OrSearch) {
    TInvertedIndex index;
    
    TVector<TString> doc1;
    doc1.PushBack(TString("cat"));
    index.AddDocument(doc1);
    
    TVector<TString> doc2;
    doc2.PushBack(TString("dog"));
    index.AddDocument(doc2);
    
    TVector<TString> doc3;
    doc3.PushBack(TString("fish"));
    index.AddDocument(doc3);
    
    TBooleanSearch search(index);
    
    TVector<TString> query;
    query.PushBack(TString("cat"));
    query.PushBack(TString("dog"));
    
    TPostingList result = search.SearchOr(query);
    
    EXPECT_EQ(result.Size(), 2);
}

TEST(TBooleanSearch, AndNotSearch) {
    TInvertedIndex index;
    
    TVector<TString> doc1;
    doc1.PushBack(TString("cat"));
    doc1.PushBack(TString("black"));
    index.AddDocument(doc1);
    
    TVector<TString> doc2;
    doc2.PushBack(TString("cat"));
    doc2.PushBack(TString("white"));
    index.AddDocument(doc2);
    
    TBooleanSearch search(index);
    
    TVector<TString> include;
    include.PushBack(TString("cat"));
    
    TVector<TString> exclude;
    exclude.PushBack(TString("black"));
    
    TPostingList result = search.SearchAndNot(include, exclude);
    
    EXPECT_EQ(result.Size(), 1);
    EXPECT_EQ(result[0], 1);
}

TEST(TTfIdf, ComputeTF) {
    TInvertedIndex index;
    
    TVector<TString> terms;
    terms.PushBack(TString("hello"));
    terms.PushBack(TString("hello"));
    terms.PushBack(TString("world"));
    
    TDocId docId = index.AddDocument(terms);
    
    TTfIdf tfidf(index);
    
    double tf = tfidf.ComputeTF(docId, TString("hello"));
    EXPECT_NEAR(tf, 2.0 / 3.0, 0.001);
}

TEST(TTfIdf, ComputeIDF) {
    TInvertedIndex index;
    
    TVector<TString> doc1;
    doc1.PushBack(TString("cat"));
    doc1.PushBack(TString("dog"));
    index.AddDocument(doc1);
    
    TVector<TString> doc2;
    doc2.PushBack(TString("cat"));
    index.AddDocument(doc2);
    
    TVector<TString> doc3;
    doc3.PushBack(TString("bird"));
    index.AddDocument(doc3);
    
    TTfIdf tfidf(index);
    
    double idfCat = tfidf.ComputeIDF(TString("cat"));
    double idfDog = tfidf.ComputeIDF(TString("dog"));
    
    EXPECT_LT(idfCat, idfDog);
}

TEST(TTfIdf, Search) {
    TInvertedIndex index;
    
    TVector<TString> doc1;
    doc1.PushBack(TString("machine"));
    doc1.PushBack(TString("learning"));
    doc1.PushBack(TString("python"));
    index.AddDocument(doc1);
    
    TVector<TString> doc2;
    doc2.PushBack(TString("deep"));
    doc2.PushBack(TString("learning"));
    doc2.PushBack(TString("neural"));
    index.AddDocument(doc2);
    
    TVector<TString> doc3;
    doc3.PushBack(TString("cooking"));
    doc3.PushBack(TString("recipes"));
    index.AddDocument(doc3);
    
    TTfIdf tfidf(index);
    
    TVector<TString> query;
    query.PushBack(TString("learning"));
    
    TVector<TTfIdf::TSearchResult> results = tfidf.Search(query, 10);
    
    ASSERT_EQ(results.Size(), 2);
    EXPECT_GT(results[0].Score, 0);
}

TEST(TTfIdf, SearchRanking) {
    TInvertedIndex index;
    
    TVector<TString> doc1;
    doc1.PushBack(TString("python"));
    doc1.PushBack(TString("python"));
    doc1.PushBack(TString("python"));
    index.AddDocument(doc1);
    
    TVector<TString> doc2;
    doc2.PushBack(TString("python"));
    doc2.PushBack(TString("java"));
    doc2.PushBack(TString("cpp"));
    index.AddDocument(doc2);
    
    TTfIdf tfidf(index);
    
    TVector<TString> query;
    query.PushBack(TString("python"));
    
    TVector<TTfIdf::TSearchResult> results = tfidf.Search(query, 10);
    
    ASSERT_EQ(results.Size(), 2);
    EXPECT_EQ(results[0].DocId, 0);
}

TEST(TTfIdf, SearchIteratorOverload) {
    TInvertedIndex index;

    TVector<TString> doc1;
    doc1.PushBack(TString("python"));
    doc1.PushBack(TString("python"));
    doc1.PushBack(TString("python"));
    index.AddDocument(doc1);

    TVector<TString> doc2;
    doc2.PushBack(TString("python"));
    doc2.PushBack(TString("java"));
    doc2.PushBack(TString("cpp"));
    index.AddDocument(doc2);

    TTfIdf tfidf(index);

    TVector<TString> query;
    query.PushBack(TString("python"));

    TVector<TTfIdf::TSearchResult> results = tfidf.Search(query.begin(), query.end(), 10);

    ASSERT_EQ(results.Size(), 2);
    EXPECT_EQ(results[0].DocId, 0);
}

TEST(TTextPipeline, BasicProcess) {
    TTextPipeline pipeline;
    TVector<TString> terms = pipeline.Process(TString("Hello World"));
    
    EXPECT_EQ(terms.Size(), 2);
    EXPECT_EQ(terms[0], TString("hello"));
    EXPECT_EQ(terms[1], TString("world"));
}

TEST(TTextPipeline, WithStemming) {
    TTextPipeline::TOptions opts;
    opts.UseStemming = true;
    TTextPipeline pipeline(opts);
    
    TVector<TString> terms = pipeline.Process(TString("running faster"));
    
    EXPECT_EQ(terms.Size(), 2);
    EXPECT_EQ(terms[0], TString("run"));
    EXPECT_EQ(terms[1], TString("faster"));
}

TEST(TTextPipeline, NormalizeTerm) {
    TTextPipeline pipeline;
    
    TString normalized = pipeline.NormalizeTerm(TString("Running"));
    EXPECT_EQ(normalized, TString("run"));
}

TEST(TSearchEngine, AddAndSearch) {
    TSearchEngine engine;
    
    engine.AddDocument(TString("machine learning with python"));
    engine.AddDocument(TString("deep learning neural networks"));
    engine.AddDocument(TString("cooking italian recipes"));
    
    TVector<TTfIdf::TSearchResult> results = engine.Search(TString("learning"), 10);
    
    EXPECT_EQ(results.Size(), 2);
}

TEST(TSearchEngine, BooleanSearch) {
    TSearchEngine engine;
    
    engine.AddDocument(TString("cat and dog"));
    engine.AddDocument(TString("cat and bird"));
    engine.AddDocument(TString("fish only"));
    
    TVector<TString> query;
    query.PushBack(TString("cat"));
    query.PushBack(TString("dog"));
    
    TPostingList result = engine.BooleanAnd(query);
    
    EXPECT_EQ(result.Size(), 1);
    EXPECT_EQ(result[0], 0);
}

TEST(TSearchEngine, WithTitle) {
    TSearchEngine engine;
    
    TDocId docId = engine.AddDocument(TString("content here"), TString("My Title"));
    
    EXPECT_EQ(engine.GetTitle(docId), TString("My Title"));
    EXPECT_EQ(engine.GetDocument(docId), TString("content here"));
}

TEST(TSearchEngine, Clear) {
    TSearchEngine engine;
    
    engine.AddDocument(TString("hello world"));
    engine.AddDocument(TString("hello universe"));
    
    EXPECT_EQ(engine.GetDocumentCount(), 2);
    
    engine.Clear();
    
    EXPECT_EQ(engine.GetDocumentCount(), 0);
    EXPECT_EQ(engine.GetTermCount(), 0);
}
