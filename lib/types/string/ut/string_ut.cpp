#include <lib/types/string/string.h>

#include <gtest/gtest.h>

using namespace NTypes;

TEST(TString, DefaultConstructor) {
    TString s;
    EXPECT_TRUE(s.Empty());
    EXPECT_EQ(s.Size(), 0u);
    EXPECT_STREQ(s.CStr(), "");
}

TEST(TString, CStringConstructor) {
    TString s("hello");
    EXPECT_EQ(s.Size(), 5u);
    EXPECT_STREQ(s.CStr(), "hello");
}

TEST(TString, CStringWithLengthConstructor) {
    TString s("hello world", 5);
    EXPECT_EQ(s.Size(), 5u);
    EXPECT_STREQ(s.CStr(), "hello");
}

TEST(TString, CharCountConstructor) {
    TString s(5, 'x');
    EXPECT_EQ(s.Size(), 5u);
    EXPECT_STREQ(s.CStr(), "xxxxx");
}

TEST(TString, CopyConstructor) {
    TString s1("hello");
    TString s2(s1);
    EXPECT_EQ(s1.Size(), s2.Size());
    EXPECT_STREQ(s1.CStr(), s2.CStr());
    s2[0] = 'H';
    EXPECT_NE(s1[0], s2[0]);
}

TEST(TString, MoveConstructor) {
    TString s1("hello");
    TString s2(std::move(s1));
    EXPECT_EQ(s2.Size(), 5u);
    EXPECT_STREQ(s2.CStr(), "hello");
    EXPECT_TRUE(s1.Empty());
}

TEST(TString, CopyAssignment) {
    TString s1("hello");
    TString s2;
    s2 = s1;
    EXPECT_STREQ(s1.CStr(), s2.CStr());
}

TEST(TString, MoveAssignment) {
    TString s1("hello");
    TString s2;
    s2 = std::move(s1);
    EXPECT_STREQ(s2.CStr(), "hello");
    EXPECT_TRUE(s1.Empty());
}

TEST(TString, CStringAssignment) {
    TString s;
    s = "hello";
    EXPECT_STREQ(s.CStr(), "hello");
}

TEST(TString, At) {
    TString s("hello");
    EXPECT_EQ(s.At(0), 'h');
    EXPECT_EQ(s.At(4), 'o');
    EXPECT_THROW(s.At(5), const char*);
}

TEST(TString, SubscriptOperator) {
    TString s("hello");
    EXPECT_EQ(s[0], 'h');
    s[0] = 'H';
    EXPECT_EQ(s[0], 'H');
}

TEST(TString, FrontBack) {
    TString s("hello");
    EXPECT_EQ(s.Front(), 'h');
    EXPECT_EQ(s.Back(), 'o');
}

TEST(TString, Iterators) {
    TString s("hello");
    TString result;
    for (auto it = s.begin(); it != s.end(); ++it) {
        result.PushBack(*it);
    }
    EXPECT_STREQ(result.CStr(), "hello");
}

TEST(TString, RangeBasedFor) {
    TString s("hello");
    TString result;
    for (char c : s) {
        result.PushBack(c);
    }
    EXPECT_STREQ(result.CStr(), "hello");
}

TEST(TString, Reserve) {
    TString s;
    s.Reserve(100);
    EXPECT_GE(s.Capacity(), 100u);
    EXPECT_EQ(s.Size(), 0u);
}

TEST(TString, Clear) {
    TString s("hello");
    s.Clear();
    EXPECT_TRUE(s.Empty());
    EXPECT_EQ(s.Size(), 0u);
}

TEST(TString, PushBack) {
    TString s;
    s.PushBack('h');
    s.PushBack('i');
    EXPECT_STREQ(s.CStr(), "hi");
}

TEST(TString, PopBack) {
    TString s("hello");
    s.PopBack();
    EXPECT_STREQ(s.CStr(), "hell");
}

TEST(TString, Append) {
    TString s("hello");
    s.Append(" world");
    EXPECT_STREQ(s.CStr(), "hello world");
}

TEST(TString, AppendString) {
    TString s1("hello");
    TString s2(" world");
    s1.Append(s2);
    EXPECT_STREQ(s1.CStr(), "hello world");
}

TEST(TString, AppendChar) {
    TString s("hello");
    s.Append(3, '!');
    EXPECT_STREQ(s.CStr(), "hello!!!");
}

TEST(TString, PlusEqual) {
    TString s("hello");
    s += " world";
    EXPECT_STREQ(s.CStr(), "hello world");
    s += '!';
    EXPECT_STREQ(s.CStr(), "hello world!");
}

TEST(TString, Concatenation) {
    TString s1("hello");
    TString s2(" world");
    TString s3 = s1 + s2;
    EXPECT_STREQ(s3.CStr(), "hello world");
}

TEST(TString, ConcatenationWithCString) {
    TString s1("hello");
    TString s2 = s1 + " world";
    TString s3 = "say " + s1;
    EXPECT_STREQ(s2.CStr(), "hello world");
    EXPECT_STREQ(s3.CStr(), "say hello");
}

TEST(TString, Find) {
    TString s("hello world");
    EXPECT_EQ(s.Find("world"), 6u);
    EXPECT_EQ(s.Find("xyz"), TString::npos);
    EXPECT_EQ(s.Find('o'), 4u);
    EXPECT_EQ(s.Find('o', 5), 7u);
}

TEST(TString, RFind) {
    TString s("hello world");
    EXPECT_EQ(s.RFind('o'), 7u);
    EXPECT_EQ(s.RFind('o', 6), 4u);
}

TEST(TString, SubStr) {
    TString s("hello world");
    TString sub = s.SubStr(6);
    EXPECT_STREQ(sub.CStr(), "world");
    
    TString sub2 = s.SubStr(0, 5);
    EXPECT_STREQ(sub2.CStr(), "hello");
}

TEST(TString, Compare) {
    TString s1("abc");
    TString s2("abd");
    TString s3("abc");
    EXPECT_LT(s1.Compare(s2), 0);
    EXPECT_GT(s2.Compare(s1), 0);
    EXPECT_EQ(s1.Compare(s3), 0);
}

TEST(TString, StartsWith) {
    TString s("hello world");
    EXPECT_TRUE(s.StartsWith("hello"));
    EXPECT_TRUE(s.StartsWith(TString("hello")));
    EXPECT_FALSE(s.StartsWith("world"));
}

TEST(TString, EndsWith) {
    TString s("hello world");
    EXPECT_TRUE(s.EndsWith("world"));
    EXPECT_TRUE(s.EndsWith(TString("world")));
    EXPECT_FALSE(s.EndsWith("hello"));
}

TEST(TString, ComparisonOperators) {
    TString s1("abc");
    TString s2("abd");
    TString s3("abc");
    
    EXPECT_TRUE(s1 == s3);
    EXPECT_TRUE(s1 != s2);
    EXPECT_TRUE(s1 < s2);
    EXPECT_TRUE(s1 <= s2);
    EXPECT_TRUE(s1 <= s3);
    EXPECT_TRUE(s2 > s1);
    EXPECT_TRUE(s2 >= s1);
    EXPECT_TRUE(s1 >= s3);
}

TEST(TString, ComparisonWithCString) {
    TString s("hello");
    EXPECT_TRUE(s == "hello");
    EXPECT_FALSE(s == "world");
    EXPECT_TRUE(s != "world");
}

TEST(TString, Swap) {
    TString s1("hello");
    TString s2("world");
    s1.Swap(s2);
    EXPECT_STREQ(s1.CStr(), "world");
    EXPECT_STREQ(s2.CStr(), "hello");
}

TEST(TString, Hash) {
    TString s1("hello");
    TString s2("hello");
    TString s3("world");
    EXPECT_EQ(s1.Hash(), s2.Hash());
    EXPECT_NE(s1.Hash(), s3.Hash());
}

TEST(TString, LongString) {
    TString s;
    for (int i = 0; i < 10000; ++i) {
        s.PushBack('a' + (i % 26));
    }
    EXPECT_EQ(s.Size(), 10000u);
}

TEST(TString, EmptyString) {
    TString s1("");
    TString s2;
    EXPECT_TRUE(s1.Empty());
    EXPECT_TRUE(s2.Empty());
    EXPECT_STREQ(s1.CStr(), "");
    EXPECT_STREQ(s2.CStr(), "");
}

