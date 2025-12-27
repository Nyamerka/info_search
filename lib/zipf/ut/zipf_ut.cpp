#include <lib/zipf/zipf.h>
#include <gtest/gtest.h>

using namespace NZipf;
using NTypes::TString;
using NCollections::TVector;

TEST(TZipfAnalyzer, BasicUsage) {
    TZipfAnalyzer analyzer;
    analyzer.AddText(TString("the cat sat on the mat"));
    
    EXPECT_EQ(analyzer.GetTotalWords(), 6);
    EXPECT_EQ(analyzer.GetUniqueWords(), 5);
}

TEST(TZipfAnalyzer, WordFrequency) {
    TZipfAnalyzer analyzer;
    analyzer.AddText(TString("the cat sat on the mat the dog"));
    
    EXPECT_EQ(analyzer.GetFrequency(TString("the")), 3);
    EXPECT_EQ(analyzer.GetFrequency(TString("cat")), 1);
    EXPECT_EQ(analyzer.GetFrequency(TString("nonexistent")), 0);
}

TEST(TZipfAnalyzer, TypeTokenRatio) {
    TZipfAnalyzer analyzer;
    analyzer.AddText(TString("the the the cat"));
    
    double ttr = analyzer.GetTypeTokenRatio();
    EXPECT_GT(ttr, 0);
    EXPECT_LT(ttr, 1);
}

TEST(TZipfAnalyzer, GetSortedFrequencies) {
    TZipfAnalyzer analyzer;
    analyzer.AddText(TString("one two two three three three"));
    
    TVector<TZipfAnalyzer::TWordFrequency> freqs = analyzer.GetSortedFrequencies();
    
    ASSERT_GE(freqs.Size(), 3);
    EXPECT_EQ(freqs[0].Frequency, 3);
    EXPECT_EQ(freqs[1].Frequency, 2);
    EXPECT_EQ(freqs[2].Frequency, 1);
    EXPECT_EQ(freqs[0].Rank, 1);
    EXPECT_EQ(freqs[1].Rank, 2);
    EXPECT_EQ(freqs[2].Rank, 3);
}

TEST(TZipfAnalyzer, Analyze) {
    TZipfAnalyzer analyzer;
    
    TString text("the quick brown fox jumps over the lazy dog "
                 "the cat sat on the mat the dog ran away "
                 "the bird flew over the tree the sun shone bright");
    analyzer.AddText(text);
    
    TZipfAnalyzer::TZipfStats stats = analyzer.Analyze(10);
    
    EXPECT_GT(stats.TotalWords, 0);
    EXPECT_GT(stats.UniqueWords, 0);
    EXPECT_GT(stats.ZipfConstant, 0);
    EXPECT_FALSE(stats.TopWords.Empty());
}

TEST(TZipfAnalyzer, ZipfExponent) {
    TZipfAnalyzer analyzer;
    
    for (int i = 0; i < 100; ++i) {
        analyzer.AddText(TString("the"));
    }
    for (int i = 0; i < 50; ++i) {
        analyzer.AddText(TString("of"));
    }
    for (int i = 0; i < 33; ++i) {
        analyzer.AddText(TString("and"));
    }
    for (int i = 0; i < 25; ++i) {
        analyzer.AddText(TString("to"));
    }
    
    TZipfAnalyzer::TZipfStats stats = analyzer.Analyze();
    
    EXPECT_GT(stats.ZipfExponent, 0.5);
    EXPECT_LT(stats.ZipfExponent, 2.0);
}

TEST(TZipfAnalyzer, Clear) {
    TZipfAnalyzer analyzer;
    analyzer.AddText(TString("hello world"));
    
    EXPECT_GT(analyzer.GetTotalWords(), 0);
    
    analyzer.Clear();
    
    EXPECT_EQ(analyzer.GetTotalWords(), 0);
    EXPECT_EQ(analyzer.GetUniqueWords(), 0);
}

TEST(TZipfAnalyzer, EmptyText) {
    TZipfAnalyzer analyzer;
    
    EXPECT_EQ(analyzer.GetTotalWords(), 0);
    EXPECT_EQ(analyzer.GetUniqueWords(), 0);
    EXPECT_EQ(analyzer.GetTypeTokenRatio(), 0);
}

TEST(TZipfAnalyzer, MultipleTexts) {
    TZipfAnalyzer analyzer;
    analyzer.AddText(TString("hello world"));
    analyzer.AddText(TString("hello universe"));
    analyzer.AddText(TString("goodbye world"));
    
    EXPECT_EQ(analyzer.GetFrequency(TString("hello")), 2);
    EXPECT_EQ(analyzer.GetFrequency(TString("world")), 2);
}

TEST(TZipfAnalyzer, LargeText) {
    TZipfAnalyzer analyzer;
    
    TString sentence("the quick brown fox jumps over the lazy dog ");
    for (int i = 0; i < 100; ++i) {
        analyzer.AddText(sentence);
    }
    
    TZipfAnalyzer::TZipfStats stats = analyzer.Analyze();
    
    EXPECT_GT(stats.TotalWords, 500);
    EXPECT_GT(stats.ZipfConstant, 0);
}

TEST(TZipfAnalyzer, TopWordsOrdering) {
    TZipfAnalyzer analyzer;
    analyzer.AddWord(TString("rare"));
    for (int i = 0; i < 5; ++i) analyzer.AddWord(TString("common"));
    for (int i = 0; i < 10; ++i) analyzer.AddWord(TString("frequent"));
    
    TZipfAnalyzer::TZipfStats stats = analyzer.Analyze();
    
    ASSERT_GE(stats.TopWords.Size(), 3);
    EXPECT_EQ(stats.TopWords[0].Word, TString("frequent"));
    EXPECT_EQ(stats.TopWords[1].Word, TString("common"));
    EXPECT_EQ(stats.TopWords[2].Word, TString("rare"));
}

TEST(TZipfAnalyzer, FormatStats) {
    TZipfAnalyzer analyzer;
    analyzer.AddText(TString("hello world hello universe"));
    
    TZipfAnalyzer::TZipfStats stats = analyzer.Analyze();
    TString formatted = TZipfAnalyzer::FormatStats(stats);
    
    EXPECT_TRUE(formatted.Size() > 0);
}

TEST(TZipfAnalyzer, ExpectedFrequency) {
    TZipfAnalyzer analyzer;
    
    for (int i = 0; i < 100; ++i) analyzer.AddWord(TString("first"));
    for (int i = 0; i < 50; ++i) analyzer.AddWord(TString("second"));
    
    TZipfAnalyzer::TZipfStats stats = analyzer.Analyze();
    
    ASSERT_GE(stats.TopWords.Size(), 2);
    EXPECT_GT(stats.TopWords[0].ExpectedFrequency, 0);
    EXPECT_GT(stats.TopWords[1].ExpectedFrequency, 0);
}


