#include <search_system/search_system.h>

#include <gtest/gtest.h>

using NSearchSystem::TSearchDatabase;
using NTypes::TString;
using NCollections::TVector;

TEST(TSearchDatabase, AddAndGetDocumentCompressed) {
    TSearchDatabase::TOptions opts;
    opts.StoreDocuments = true;
    opts.CompressDocuments = true;

    TSearchDatabase db(opts);
    auto id = db.AddDocument(TString("hello world"), TString("t"));

    EXPECT_EQ(db.GetDocument(id), TString("hello world"));
    EXPECT_EQ(db.GetTitle(id), TString("t"));
}

TEST(TSearchDatabase, TfIdfSearch) {
    TSearchDatabase db;
    db.AddDocument(TString("machine learning with python"));
    db.AddDocument(TString("deep learning neural networks"));
    db.AddDocument(TString("cooking italian recipes"));

    auto r = db.Search(TString("learning"), 10);
    ASSERT_EQ(r.Size(), 2);
}

TEST(TSearchDatabase, BooleanQueryAnd) {
    TSearchDatabase db;
    db.AddDocument(TString("cat dog"));
    db.AddDocument(TString("cat bird"));
    db.AddDocument(TString("fish"));

    auto r = db.BooleanQuery(TString("cat AND dog"));
    ASSERT_EQ(r.Size(), 1);
    EXPECT_EQ(r[0], 0);
}

TEST(TSearchDatabase, BooleanQueryOrNot) {
    TSearchDatabase db;
    db.AddDocument(TString("red apple"));
    db.AddDocument(TString("green apple"));
    db.AddDocument(TString("red banana"));

    auto r = db.BooleanQuery(TString("(red OR green) AND NOT banana"));
    ASSERT_EQ(r.Size(), 2);
    EXPECT_EQ(r[0], 0);
    EXPECT_EQ(r[1], 1);
}

TEST(TSearchDatabase, AddDocumentTermsIterator) {
    TSearchDatabase db;
    TVector<TString> terms;
    terms.PushBack(TString("hello"));
    terms.PushBack(TString("world"));
    auto id = db.AddDocumentTerms(terms.begin(), terms.end(), TString("hello world"));
    EXPECT_EQ(db.GetDocument(id), TString("hello world"));
}


