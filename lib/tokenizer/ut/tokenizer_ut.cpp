#include <lib/tokenizer/tokenizer.h>
#include <gtest/gtest.h>

using namespace NTokenizer;
using NTypes::TString;
using NCollections::TVector;

TEST(TTokenizer, BasicTokenization) {
    TTokenizer tokenizer;
    TVector<TToken> tokens = tokenizer.Tokenize(TString("Hello world"));
    
    ASSERT_EQ(tokens.Size(), 2);
    EXPECT_EQ(tokens[0].Text, TString("hello"));
    EXPECT_EQ(tokens[1].Text, TString("world"));
}

TEST(TTokenizer, PreservesPosition) {
    TTokenizer tokenizer;
    TVector<TToken> tokens = tokenizer.Tokenize(TString("Hello world"));
    
    ASSERT_EQ(tokens.Size(), 2);
    EXPECT_EQ(tokens[0].Position, 0);
    EXPECT_EQ(tokens[0].Length, 5);
    EXPECT_EQ(tokens[1].Position, 6);
    EXPECT_EQ(tokens[1].Length, 5);
}

TEST(TTokenizer, HandlesNumbers) {
    TTokenizer::TOptions opts;
    opts.SkipNumbers = false;
    TTokenizer tokenizer(opts);
    
    TVector<TToken> tokens = tokenizer.Tokenize(TString("test 123 abc"));
    
    ASSERT_EQ(tokens.Size(), 3);
    EXPECT_EQ(tokens[0].Text, TString("test"));
    EXPECT_EQ(tokens[1].Text, TString("123"));
    EXPECT_EQ(tokens[2].Text, TString("abc"));
}

TEST(TTokenizer, SkipsNumbers) {
    TTokenizer tokenizer;
    TVector<TToken> tokens = tokenizer.Tokenize(TString("test 123 abc"));
    
    ASSERT_EQ(tokens.Size(), 2);
    EXPECT_EQ(tokens[0].Text, TString("test"));
    EXPECT_EQ(tokens[1].Text, TString("abc"));
}

TEST(TTokenizer, HandlesPunctuation) {
    TTokenizer::TOptions opts;
    opts.SkipPunctuation = false;
    TTokenizer tokenizer(opts);
    
    TVector<TToken> tokens = tokenizer.Tokenize(TString("Hello, world!"));
    
    ASSERT_EQ(tokens.Size(), 4);
    EXPECT_EQ(tokens[0].Text, TString("hello"));
    EXPECT_EQ(tokens[1].Text, TString(","));
    EXPECT_EQ(tokens[2].Text, TString("world"));
    EXPECT_EQ(tokens[3].Text, TString("!"));
}

TEST(TTokenizer, SkipsPunctuation) {
    TTokenizer tokenizer;
    TVector<TToken> tokens = tokenizer.Tokenize(TString("Hello, world!"));
    
    ASSERT_EQ(tokens.Size(), 2);
    EXPECT_EQ(tokens[0].Text, TString("hello"));
    EXPECT_EQ(tokens[1].Text, TString("world"));
}

TEST(TTokenizer, LowerCase) {
    TTokenizer tokenizer;
    TVector<TToken> tokens = tokenizer.Tokenize(TString("HELLO World"));
    
    ASSERT_EQ(tokens.Size(), 2);
    EXPECT_EQ(tokens[0].Text, TString("hello"));
    EXPECT_EQ(tokens[1].Text, TString("world"));
}

TEST(TTokenizer, PreserveCase) {
    TTokenizer::TOptions opts;
    opts.LowerCase = false;
    TTokenizer tokenizer(opts);
    
    TVector<TToken> tokens = tokenizer.Tokenize(TString("HELLO World"));
    
    ASSERT_EQ(tokens.Size(), 2);
    EXPECT_EQ(tokens[0].Text, TString("HELLO"));
    EXPECT_EQ(tokens[1].Text, TString("World"));
}

TEST(TTokenizer, MinTokenLength) {
    TTokenizer::TOptions opts;
    opts.MinTokenLength = 3;
    TTokenizer tokenizer(opts);
    
    TVector<TToken> tokens = tokenizer.Tokenize(TString("a ab abc abcd"));
    
    ASSERT_EQ(tokens.Size(), 2);
    EXPECT_EQ(tokens[0].Text, TString("abc"));
    EXPECT_EQ(tokens[1].Text, TString("abcd"));
}

TEST(TTokenizer, EmptyInput) {
    TTokenizer tokenizer;
    TVector<TToken> tokens = tokenizer.Tokenize(TString(""));
    EXPECT_TRUE(tokens.Empty());
}

TEST(TTokenizer, WhitespaceOnly) {
    TTokenizer tokenizer;
    TVector<TToken> tokens = tokenizer.Tokenize(TString("   \t\n  "));
    EXPECT_TRUE(tokens.Empty());
}

TEST(TTokenizer, TokenizeToStrings) {
    TTokenizer tokenizer;
    TVector<TString> tokens = tokenizer.TokenizeToStrings(TString("Hello beautiful world"));
    
    ASSERT_EQ(tokens.Size(), 3);
    EXPECT_EQ(tokens[0], TString("hello"));
    EXPECT_EQ(tokens[1], TString("beautiful"));
    EXPECT_EQ(tokens[2], TString("world"));
}

TEST(TTokenizer, ToLower) {
    EXPECT_EQ(TTokenizer::ToLower(TString("HELLO")), TString("hello"));
    EXPECT_EQ(TTokenizer::ToLower(TString("Hello")), TString("hello"));
    EXPECT_EQ(TTokenizer::ToLower(TString("hello")), TString("hello"));
}

TEST(TTokenizer, ToUpper) {
    EXPECT_EQ(TTokenizer::ToUpper(TString("hello")), TString("HELLO"));
    EXPECT_EQ(TTokenizer::ToUpper(TString("Hello")), TString("HELLO"));
    EXPECT_EQ(TTokenizer::ToUpper(TString("HELLO")), TString("HELLO"));
}

TEST(TTokenizer, Normalize) {
    EXPECT_EQ(TTokenizer::Normalize(TString("Hello, World!")), TString("helloworld"));
    EXPECT_EQ(TTokenizer::Normalize(TString("Test123")), TString("test123"));
}

TEST(TTokenizer, Trim) {
    EXPECT_EQ(TTokenizer::Trim(TString("  hello  ")), TString("hello"));
    EXPECT_EQ(TTokenizer::Trim(TString("hello")), TString("hello"));
    EXPECT_EQ(TTokenizer::Trim(TString("  ")), TString(""));
}

TEST(TTokenizer, Split) {
    TVector<TString> parts = TTokenizer::Split(TString("a,b,c"), ',');
    
    ASSERT_EQ(parts.Size(), 3);
    EXPECT_EQ(parts[0], TString("a"));
    EXPECT_EQ(parts[1], TString("b"));
    EXPECT_EQ(parts[2], TString("c"));
}

TEST(TTokenizer, Join) {
    TVector<TString> parts;
    parts.PushBack(TString("a"));
    parts.PushBack(TString("b"));
    parts.PushBack(TString("c"));
    
    EXPECT_EQ(TTokenizer::Join(parts, TString(", ")), TString("a, b, c"));
}

TEST(TTokenizer, ComplexText) {
    TTokenizer tokenizer;
    TVector<TToken> tokens = tokenizer.Tokenize(
        TString("The quick brown fox jumps over the lazy dog.")
    );
    
    EXPECT_EQ(tokens.Size(), 9);
}

TEST(TTokenizer, HyphenatedWords) {
    TTokenizer tokenizer;
    TVector<TToken> tokens = tokenizer.Tokenize(TString("self-driving car"));
    
    ASSERT_EQ(tokens.Size(), 2);
    EXPECT_EQ(tokens[0].Text, TString("self-driving"));
    EXPECT_EQ(tokens[1].Text, TString("car"));
}
