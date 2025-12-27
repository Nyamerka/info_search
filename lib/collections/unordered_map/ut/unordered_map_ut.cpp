#include <lib/collections/unordered_map/unordered_map.h>
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

TEST(TUnorderedMap, DefaultConstructor) {
    TUnorderedMap<int, int> m;
    EXPECT_TRUE(m.Empty());
    EXPECT_EQ(m.Size(), 0u);
}

TEST(TUnorderedMap, BucketCountConstructor) {
    TUnorderedMap<int, int> m(32);
    EXPECT_GE(m.BucketCount(), 32u);
}

TEST(TUnorderedMap, InitializerListConstructor) {
    TUnorderedMap<int, int> m = {{1, 10}, {2, 20}, {3, 30}};
    EXPECT_EQ(m.Size(), 3u);
    EXPECT_EQ(m[1], 10);
    EXPECT_EQ(m[2], 20);
    EXPECT_EQ(m[3], 30);
}

TEST(TUnorderedMap, CopyConstructor) {
    TUnorderedMap<int, int> m1 = {{1, 10}, {2, 20}};
    TUnorderedMap<int, int> m2(m1);
    EXPECT_EQ(m1.Size(), m2.Size());
    EXPECT_EQ(m2[1], 10);
    EXPECT_EQ(m2[2], 20);
}

TEST(TUnorderedMap, MoveConstructor) {
    TUnorderedMap<int, int> m1 = {{1, 10}, {2, 20}};
    TUnorderedMap<int, int> m2(std::move(m1));
    EXPECT_EQ(m2.Size(), 2u);
    EXPECT_TRUE(m1.Empty());
}

TEST(TUnorderedMap, CopyAssignment) {
    TUnorderedMap<int, int> m1 = {{1, 10}, {2, 20}};
    TUnorderedMap<int, int> m2;
    m2 = m1;
    EXPECT_EQ(m2[1], 10);
    EXPECT_EQ(m2[2], 20);
}

TEST(TUnorderedMap, MoveAssignment) {
    TUnorderedMap<int, int> m1 = {{1, 10}, {2, 20}};
    TUnorderedMap<int, int> m2;
    m2 = std::move(m1);
    EXPECT_EQ(m2.Size(), 2u);
    EXPECT_TRUE(m1.Empty());
}

TEST(TUnorderedMap, At) {
    TUnorderedMap<int, int> m = {{1, 10}, {2, 20}};
    EXPECT_EQ(m.At(1), 10);
    EXPECT_EQ(m.At(2), 20);
    EXPECT_THROW(m.At(3), const char*);
}

TEST(TUnorderedMap, SubscriptOperator) {
    TUnorderedMap<int, int> m;
    m[1] = 10;
    m[2] = 20;
    EXPECT_EQ(m[1], 10);
    EXPECT_EQ(m[2], 20);
    
    int& val = m[3];
    EXPECT_EQ(val, 0);
    val = 30;
    EXPECT_EQ(m[3], 30);
}

TEST(TUnorderedMap, Insert) {
    TUnorderedMap<int, int> m;
    EXPECT_TRUE(m.Insert(1, 10));
    EXPECT_TRUE(m.Insert(2, 20));
    EXPECT_FALSE(m.Insert(1, 100));
    
    EXPECT_EQ(m.Size(), 2u);
    EXPECT_EQ(m[1], 100);
    EXPECT_EQ(m[2], 20);
}

TEST(TUnorderedMap, Erase) {
    TUnorderedMap<int, int> m = {{1, 10}, {2, 20}, {3, 30}};
    EXPECT_TRUE(m.Erase(2));
    EXPECT_FALSE(m.Erase(2));
    EXPECT_EQ(m.Size(), 2u);
    EXPECT_FALSE(m.Contains(2));
}

TEST(TUnorderedMap, Clear) {
    TUnorderedMap<int, int> m = {{1, 10}, {2, 20}};
    m.Clear();
    EXPECT_TRUE(m.Empty());
    EXPECT_EQ(m.Size(), 0u);
}

TEST(TUnorderedMap, Find) {
    TUnorderedMap<int, int> m = {{1, 10}, {2, 20}};
    auto it = m.Find(1);
    EXPECT_NE(it, m.end());
    EXPECT_EQ(it.Key(), 1);
    EXPECT_EQ(it.Value(), 10);
    
    auto notFound = m.Find(999);
    EXPECT_EQ(notFound, m.end());
}

TEST(TUnorderedMap, Contains) {
    TUnorderedMap<int, int> m = {{1, 10}, {2, 20}};
    EXPECT_TRUE(m.Contains(1));
    EXPECT_TRUE(m.Contains(2));
    EXPECT_FALSE(m.Contains(3));
}

TEST(TUnorderedMap, Count) {
    TUnorderedMap<int, int> m = {{1, 10}, {2, 20}};
    EXPECT_EQ(m.Count(1), 1u);
    EXPECT_EQ(m.Count(999), 0u);
}

TEST(TUnorderedMap, Iterators) {
    TUnorderedMap<int, int> m = {{1, 10}, {2, 20}, {3, 30}};
    int sum = 0;
    int keySum = 0;
    for (auto it = m.begin(); it != m.end(); ++it) {
        keySum += it.Key();
        sum += it.Value();
    }
    EXPECT_EQ(keySum, 6);
    EXPECT_EQ(sum, 60);
}

TEST(TUnorderedMap, Swap) {
    TUnorderedMap<int, int> m1 = {{1, 10}};
    TUnorderedMap<int, int> m2 = {{2, 20}, {3, 30}};
    m1.Swap(m2);
    EXPECT_EQ(m1.Size(), 2u);
    EXPECT_EQ(m2.Size(), 1u);
    EXPECT_TRUE(m1.Contains(2));
    EXPECT_TRUE(m2.Contains(1));
}

TEST(TUnorderedMap, LoadFactor) {
    TUnorderedMap<int, int> m(16);
    EXPECT_FLOAT_EQ(m.LoadFactor(), 0.0f);
    m[1] = 10;
    EXPECT_GT(m.LoadFactor(), 0.0f);
}

TEST(TUnorderedMap, Rehash) {
    TUnorderedMap<int, int> m(4);
    for (int i = 0; i < 100; ++i) {
        m[i] = i * 10;
    }
    EXPECT_EQ(m.Size(), 100u);
    for (int i = 0; i < 100; ++i) {
        EXPECT_TRUE(m.Contains(i));
        EXPECT_EQ(m[i], i * 10);
    }
}

TEST(TUnorderedMap, StringKeys) {
    TUnorderedMap<TString, int> m;
    m.Insert(TString("one"), 1);
    m.Insert(TString("two"), 2);
    m.Insert(TString("three"), 3);
    
    EXPECT_EQ(m.Size(), 3u);
    EXPECT_EQ(m.At(TString("one")), 1);
    EXPECT_EQ(m.At(TString("two")), 2);
    EXPECT_EQ(m.At(TString("three")), 3);
}

TEST(TUnorderedMap, CollisionHandling) {
    TUnorderedMap<int, int> m(2);
    for (int i = 0; i < 50; ++i) {
        m[i] = i;
    }
    EXPECT_EQ(m.Size(), 50u);
    for (int i = 0; i < 50; ++i) {
        EXPECT_EQ(m[i], i);
    }
}

TEST(TUnorderedMap, ManyElements) {
    TUnorderedMap<int, int> m;
    for (int i = 0; i < 10000; ++i) {
        m[i] = i * 2;
    }
    EXPECT_EQ(m.Size(), 10000u);
    for (int i = 0; i < 10000; ++i) {
        EXPECT_TRUE(m.Contains(i));
        EXPECT_EQ(m[i], i * 2);
    }
}

