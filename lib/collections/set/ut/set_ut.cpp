#include <lib/collections/set/set.h>
#include <lib/types/string/string.h>

#include <gtest/gtest.h>

using namespace NCollections;
using namespace NTypes;

TEST(TSet, DefaultConstructor) {
    TSet<int> s;
    EXPECT_TRUE(s.Empty());
    EXPECT_EQ(s.Size(), 0u);
}

TEST(TSet, InitializerListConstructor) {
    TSet<int> s = {3, 1, 4, 1, 5, 9, 2, 6};
    EXPECT_EQ(s.Size(), 7u);  // No duplicates
    EXPECT_TRUE(s.Contains(1));
    EXPECT_TRUE(s.Contains(9));
}

TEST(TSet, CopyConstructor) {
    TSet<int> s1 = {1, 2, 3};
    TSet<int> s2(s1);
    EXPECT_EQ(s1.Size(), s2.Size());
    EXPECT_TRUE(s2.Contains(1));
    EXPECT_TRUE(s2.Contains(2));
    EXPECT_TRUE(s2.Contains(3));
}

TEST(TSet, MoveConstructor) {
    TSet<int> s1 = {1, 2, 3};
    TSet<int> s2(std::move(s1));
    EXPECT_EQ(s2.Size(), 3u);
    EXPECT_TRUE(s1.Empty());
}

TEST(TSet, CopyAssignment) {
    TSet<int> s1 = {1, 2, 3};
    TSet<int> s2;
    s2 = s1;
    EXPECT_TRUE(s2.Contains(1));
    EXPECT_TRUE(s2.Contains(2));
}

TEST(TSet, MoveAssignment) {
    TSet<int> s1 = {1, 2, 3};
    TSet<int> s2;
    s2 = std::move(s1);
    EXPECT_EQ(s2.Size(), 3u);
    EXPECT_TRUE(s1.Empty());
}

TEST(TSet, Insert) {
    TSet<int> s;
    EXPECT_TRUE(s.Insert(1));
    EXPECT_TRUE(s.Insert(2));
    EXPECT_FALSE(s.Insert(1));  // Duplicate
    EXPECT_EQ(s.Size(), 2u);
}

TEST(TSet, Erase) {
    TSet<int> s = {1, 2, 3};
    EXPECT_TRUE(s.Erase(2));
    EXPECT_FALSE(s.Erase(2));
    EXPECT_EQ(s.Size(), 2u);
    EXPECT_FALSE(s.Contains(2));
}

TEST(TSet, Clear) {
    TSet<int> s = {1, 2, 3};
    s.Clear();
    EXPECT_TRUE(s.Empty());
    EXPECT_EQ(s.Size(), 0u);
}

TEST(TSet, Find) {
    TSet<int> s = {1, 2, 3};
    auto it = s.Find(2);
    EXPECT_NE(it, s.end());
    EXPECT_EQ(*it, 2);
    
    auto notFound = s.Find(999);
    EXPECT_EQ(notFound, s.end());
}

TEST(TSet, Contains) {
    TSet<int> s = {1, 2, 3};
    EXPECT_TRUE(s.Contains(1));
    EXPECT_TRUE(s.Contains(2));
    EXPECT_TRUE(s.Contains(3));
    EXPECT_FALSE(s.Contains(4));
}

TEST(TSet, Count) {
    TSet<int> s = {1, 2, 3};
    EXPECT_EQ(s.Count(1), 1u);
    EXPECT_EQ(s.Count(999), 0u);
}

TEST(TSet, IteratorsInOrder) {
    TSet<int> s = {5, 2, 8, 1, 9, 3};
    
    int prev = 0;
    for (auto it = s.begin(); it != s.end(); ++it) {
        EXPECT_GT(*it, prev);
        prev = *it;
    }
}

TEST(TSet, RangeBasedFor) {
    TSet<int> s = {3, 1, 2};
    int sum = 0;
    for (int x : s) {
        sum += x;
    }
    EXPECT_EQ(sum, 6);
}

TEST(TSet, Swap) {
    TSet<int> s1 = {1, 2};
    TSet<int> s2 = {3, 4, 5};
    s1.Swap(s2);
    EXPECT_EQ(s1.Size(), 3u);
    EXPECT_EQ(s2.Size(), 2u);
    EXPECT_TRUE(s1.Contains(3));
    EXPECT_TRUE(s2.Contains(1));
}

TEST(TSet, LowerBound) {
    TSet<int> s = {1, 3, 5, 7, 9};
    
    auto it = s.LowerBound(3);
    EXPECT_NE(it, s.end());
    EXPECT_EQ(*it, 3);
    
    it = s.LowerBound(4);
    EXPECT_NE(it, s.end());
    EXPECT_EQ(*it, 5);
    
    it = s.LowerBound(10);
    EXPECT_EQ(it, s.end());
}

TEST(TSet, UpperBound) {
    TSet<int> s = {1, 3, 5, 7, 9};
    
    auto it = s.UpperBound(3);
    EXPECT_NE(it, s.end());
    EXPECT_EQ(*it, 5);
    
    it = s.UpperBound(0);
    EXPECT_NE(it, s.end());
    EXPECT_EQ(*it, 1);
    
    it = s.UpperBound(9);
    EXPECT_EQ(it, s.end());
}

TEST(TSet, Union) {
    TSet<int> s1 = {1, 2, 3};
    TSet<int> s2 = {3, 4, 5};
    TSet<int> result = s1.Union(s2);
    
    EXPECT_EQ(result.Size(), 5u);
    EXPECT_TRUE(result.Contains(1));
    EXPECT_TRUE(result.Contains(2));
    EXPECT_TRUE(result.Contains(3));
    EXPECT_TRUE(result.Contains(4));
    EXPECT_TRUE(result.Contains(5));
}

TEST(TSet, Intersection) {
    TSet<int> s1 = {1, 2, 3, 4};
    TSet<int> s2 = {3, 4, 5, 6};
    TSet<int> result = s1.Intersection(s2);
    
    EXPECT_EQ(result.Size(), 2u);
    EXPECT_TRUE(result.Contains(3));
    EXPECT_TRUE(result.Contains(4));
}

TEST(TSet, Difference) {
    TSet<int> s1 = {1, 2, 3, 4};
    TSet<int> s2 = {3, 4, 5, 6};
    TSet<int> result = s1.Difference(s2);
    
    EXPECT_EQ(result.Size(), 2u);
    EXPECT_TRUE(result.Contains(1));
    EXPECT_TRUE(result.Contains(2));
}

TEST(TSet, Equality) {
    TSet<int> s1 = {1, 2, 3};
    TSet<int> s2 = {1, 2, 3};
    TSet<int> s3 = {1, 2, 4};
    
    EXPECT_TRUE(s1 == s2);
    EXPECT_FALSE(s1 == s3);
    EXPECT_FALSE(s1 != s2);
    EXPECT_TRUE(s1 != s3);
}

TEST(TSet, StringValues) {
    TSet<TString> s;
    s.Insert(TString("cherry"));
    s.Insert(TString("apple"));
    s.Insert(TString("banana"));
    
    EXPECT_EQ(s.Size(), 3u);
    EXPECT_TRUE(s.Contains(TString("apple")));
    EXPECT_TRUE(s.Contains(TString("banana")));
    EXPECT_FALSE(s.Contains(TString("missing")));
    
    // Check order
    auto it = s.begin();
    EXPECT_EQ(*it, TString("apple"));
    ++it;
    EXPECT_EQ(*it, TString("banana"));
    ++it;
    EXPECT_EQ(*it, TString("cherry"));
}

TEST(TSet, ManyElements) {
    TSet<int> s;
    for (int i = 0; i < 1000; ++i) {
        s.Insert(i);
    }
    EXPECT_EQ(s.Size(), 1000u);
    
    // Verify order
    int prev = -1;
    for (int x : s) {
        EXPECT_GT(x, prev);
        prev = x;
    }
}

TEST(TSet, NoDuplicates) {
    TSet<int> s;
    for (int i = 0; i < 100; ++i) {
        s.Insert(42);
    }
    EXPECT_EQ(s.Size(), 1u);
    EXPECT_TRUE(s.Contains(42));
}

TEST(TSet, EmptySetOperations) {
    TSet<int> empty;
    TSet<int> s = {1, 2, 3};
    
    EXPECT_EQ(empty.Union(s).Size(), 3u);
    EXPECT_EQ(empty.Intersection(s).Size(), 0u);
    EXPECT_EQ(empty.Difference(s).Size(), 0u);
    EXPECT_EQ(s.Difference(empty).Size(), 3u);
}

