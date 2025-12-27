#include <lib/collections/unordered_set/unordered_set.h>
#include <lib/types/string/string.h>

#include <gtest/gtest.h>

using namespace NCollections;
using namespace NTypes;

// Hash specialization for TString
namespace NCollections {
    template <>
    struct THash<TString> {
        size_t operator()(const TString& s) const {
            return s.Hash();
        }
    };
}

TEST(TUnorderedSet, DefaultConstructor) {
    TUnorderedSet<int> s;
    EXPECT_TRUE(s.Empty());
    EXPECT_EQ(s.Size(), 0u);
}

TEST(TUnorderedSet, BucketCountConstructor) {
    TUnorderedSet<int> s(32);
    EXPECT_TRUE(s.Empty());
}

TEST(TUnorderedSet, InitializerListConstructor) {
    TUnorderedSet<int> s = {1, 2, 3, 4, 5};
    EXPECT_EQ(s.Size(), 5u);
    EXPECT_TRUE(s.Contains(1));
    EXPECT_TRUE(s.Contains(5));
}

TEST(TUnorderedSet, CopyConstructor) {
    TUnorderedSet<int> s1 = {1, 2, 3};
    TUnorderedSet<int> s2(s1);
    EXPECT_EQ(s1.Size(), s2.Size());
    EXPECT_TRUE(s2.Contains(1));
    EXPECT_TRUE(s2.Contains(2));
    EXPECT_TRUE(s2.Contains(3));
}

TEST(TUnorderedSet, MoveConstructor) {
    TUnorderedSet<int> s1 = {1, 2, 3};
    TUnorderedSet<int> s2(std::move(s1));
    EXPECT_EQ(s2.Size(), 3u);
    EXPECT_TRUE(s1.Empty());
}

TEST(TUnorderedSet, CopyAssignment) {
    TUnorderedSet<int> s1 = {1, 2, 3};
    TUnorderedSet<int> s2;
    s2 = s1;
    EXPECT_TRUE(s2.Contains(1));
    EXPECT_TRUE(s2.Contains(2));
}

TEST(TUnorderedSet, MoveAssignment) {
    TUnorderedSet<int> s1 = {1, 2, 3};
    TUnorderedSet<int> s2;
    s2 = std::move(s1);
    EXPECT_EQ(s2.Size(), 3u);
    EXPECT_TRUE(s1.Empty());
}

TEST(TUnorderedSet, Insert) {
    TUnorderedSet<int> s;
    EXPECT_TRUE(s.Insert(1));
    EXPECT_TRUE(s.Insert(2));
    EXPECT_FALSE(s.Insert(1));
    EXPECT_EQ(s.Size(), 2u);
}

TEST(TUnorderedSet, Erase) {
    TUnorderedSet<int> s = {1, 2, 3};
    EXPECT_TRUE(s.Erase(2));
    EXPECT_FALSE(s.Erase(2));
    EXPECT_EQ(s.Size(), 2u);
    EXPECT_FALSE(s.Contains(2));
}

TEST(TUnorderedSet, Clear) {
    TUnorderedSet<int> s = {1, 2, 3};
    s.Clear();
    EXPECT_TRUE(s.Empty());
    EXPECT_EQ(s.Size(), 0u);
}

TEST(TUnorderedSet, Find) {
    TUnorderedSet<int> s = {1, 2, 3};
    auto it = s.Find(2);
    EXPECT_NE(it, s.end());
    EXPECT_EQ(*it, 2);
    
    auto notFound = s.Find(999);
    EXPECT_EQ(notFound, s.end());
}

TEST(TUnorderedSet, Contains) {
    TUnorderedSet<int> s = {1, 2, 3};
    EXPECT_TRUE(s.Contains(1));
    EXPECT_TRUE(s.Contains(2));
    EXPECT_TRUE(s.Contains(3));
    EXPECT_FALSE(s.Contains(4));
}

TEST(TUnorderedSet, Count) {
    TUnorderedSet<int> s = {1, 2, 3};
    EXPECT_EQ(s.Count(1), 1u);
    EXPECT_EQ(s.Count(999), 0u);
}

TEST(TUnorderedSet, Iterators) {
    TUnorderedSet<int> s = {1, 2, 3};
    int sum = 0;
    for (auto it = s.begin(); it != s.end(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(sum, 6);
}

TEST(TUnorderedSet, RangeBasedFor) {
    TUnorderedSet<int> s = {1, 2, 3};
    int sum = 0;
    for (int x : s) {
        sum += x;
    }
    EXPECT_EQ(sum, 6);
}

TEST(TUnorderedSet, Swap) {
    TUnorderedSet<int> s1 = {1, 2};
    TUnorderedSet<int> s2 = {3, 4, 5};
    s1.Swap(s2);
    EXPECT_EQ(s1.Size(), 3u);
    EXPECT_EQ(s2.Size(), 2u);
    EXPECT_TRUE(s1.Contains(3));
    EXPECT_TRUE(s2.Contains(1));
}

TEST(TUnorderedSet, Union) {
    TUnorderedSet<int> s1 = {1, 2, 3};
    TUnorderedSet<int> s2 = {3, 4, 5};
    TUnorderedSet<int> result = s1.Union(s2);
    
    EXPECT_EQ(result.Size(), 5u);
    EXPECT_TRUE(result.Contains(1));
    EXPECT_TRUE(result.Contains(2));
    EXPECT_TRUE(result.Contains(3));
    EXPECT_TRUE(result.Contains(4));
    EXPECT_TRUE(result.Contains(5));
}

TEST(TUnorderedSet, Intersection) {
    TUnorderedSet<int> s1 = {1, 2, 3, 4};
    TUnorderedSet<int> s2 = {3, 4, 5, 6};
    TUnorderedSet<int> result = s1.Intersection(s2);
    
    EXPECT_EQ(result.Size(), 2u);
    EXPECT_TRUE(result.Contains(3));
    EXPECT_TRUE(result.Contains(4));
}

TEST(TUnorderedSet, Difference) {
    TUnorderedSet<int> s1 = {1, 2, 3, 4};
    TUnorderedSet<int> s2 = {3, 4, 5, 6};
    TUnorderedSet<int> result = s1.Difference(s2);
    
    EXPECT_EQ(result.Size(), 2u);
    EXPECT_TRUE(result.Contains(1));
    EXPECT_TRUE(result.Contains(2));
}

TEST(TUnorderedSet, Equality) {
    TUnorderedSet<int> s1 = {1, 2, 3};
    TUnorderedSet<int> s2 = {1, 2, 3};
    TUnorderedSet<int> s3 = {1, 2, 4};
    
    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1 == s3);
    EXPECT_FALSE(s1 != s2);
    EXPECT_TRUE(s1 != s3);
}

TEST(TUnorderedSet, StringValues) {
    TUnorderedSet<TString> s;
    s.Insert(TString("hello"));
    s.Insert(TString("world"));
    s.Insert(TString("test"));
    
    EXPECT_EQ(s.Size(), 3u);
    EXPECT_TRUE(s.Contains(TString("hello")));
    EXPECT_TRUE(s.Contains(TString("world")));
    EXPECT_FALSE(s.Contains(TString("missing")));
}

TEST(TUnorderedSet, ManyElements) {
    TUnorderedSet<int> s;
    for (int i = 0; i < 10000; ++i) {
        s.Insert(i);
    }
    EXPECT_EQ(s.Size(), 10000u);
    for (int i = 0; i < 10000; ++i) {
        EXPECT_TRUE(s.Contains(i));
    }
    EXPECT_FALSE(s.Contains(10000));
}

TEST(TUnorderedSet, NoDuplicates) {
    TUnorderedSet<int> s;
    for (int i = 0; i < 100; ++i) {
        s.Insert(42);
    }
    EXPECT_EQ(s.Size(), 1u);
    EXPECT_TRUE(s.Contains(42));
}

TEST(TUnorderedSet, EmptySetOperations) {
    TUnorderedSet<int> empty;
    TUnorderedSet<int> s = {1, 2, 3};
    
    EXPECT_EQ(empty.Union(s).Size(), 3u);
    EXPECT_EQ(empty.Intersection(s).Size(), 0u);
    EXPECT_EQ(empty.Difference(s).Size(), 0u);
    EXPECT_EQ(s.Difference(empty).Size(), 3u);
}

