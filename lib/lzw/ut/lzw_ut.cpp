#include <lib/lzw/lzw.h>

#include <gtest/gtest.h>

using NLzw::TLzw;
using NTypes::TString;
using NCollections::TVector;

TEST(TLzw, EmptyString) {
    TLzw lzw;
    TString s;
    auto compressed = lzw.Compress(s);
    TString restored = lzw.Decompress(compressed);
    EXPECT_EQ(restored, s);
}

TEST(TLzw, RoundTripSimple) {
    TLzw lzw;
    TString s("hello world");
    auto compressed = lzw.Compress(s);
    TString restored = lzw.Decompress(compressed);
    EXPECT_EQ(restored, s);
}

TEST(TLzw, RoundTripLong) {
    TLzw lzw;
    TString s;
    for (int i = 0; i < 5000; ++i) {
        s.Append("the quick brown fox jumps over the lazy dog ");
    }
    auto compressed = lzw.Compress(s);
    TString restored = lzw.Decompress(compressed);
    EXPECT_EQ(restored, s);
}

TEST(TLzw, CompressionHelpsOnRepetition) {
    TLzw lzw;
    TString s;
    for (int i = 0; i < 20000; ++i) {
        s.PushBack('a');
    }
    auto compressed = lzw.Compress(s);
    EXPECT_LT(compressed.Size(), s.Size());
    TString restored = lzw.Decompress(compressed);
    EXPECT_EQ(restored, s);
}

TEST(TLzw, IteratorOverload) {
    TLzw lzw;
    TString s("compress me please");
    auto compressed = lzw.Compress(s.begin(), s.end());
    TString restored = lzw.Decompress(compressed.begin(), compressed.end());
    EXPECT_EQ(restored, s);
}

TEST(TLzw, BinaryLikeData) {
    TLzw lzw;
    TString s;
    for (int i = 0; i < 256; ++i) {
        s.PushBack(static_cast<char>(static_cast<unsigned char>(i)));
    }
    for (int i = 0; i < 256; ++i) {
        s.PushBack(static_cast<char>(static_cast<unsigned char>(i)));
    }
    auto compressed = lzw.Compress(s);
    TString restored = lzw.Decompress(compressed);
    EXPECT_EQ(restored, s);
}


