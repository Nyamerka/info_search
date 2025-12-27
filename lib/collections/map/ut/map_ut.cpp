#include <lib/collections/map/map.h>
#include <lib/types/string/string.h>

#include <gtest/gtest.h>

using namespace NCollections;
using namespace NTypes;

TEST(TMap, DefaultConstructor) {
    TMap<int, int> m;
    EXPECT_TRUE(m.Empty());
    EXPECT_EQ(m.Size(), 0u);
}

TEST(TMap, InitializerListConstructor) {
    TMap<int, int> m = {{1, 10}, {2, 20}, {3, 30}};
    EXPECT_EQ(m.Size(), 3u);
    EXPECT_EQ(m[1], 10);
    EXPECT_EQ(m[2], 20);
    EXPECT_EQ(m[3], 30);
}

TEST(TMap, CopyConstructor) {
    TMap<int, int> m1 = {{1, 10}, {2, 20}};
    TMap<int, int> m2(m1);
    EXPECT_EQ(m1.Size(), m2.Size());
    EXPECT_EQ(m2[1], 10);
    EXPECT_EQ(m2[2], 20);
}

TEST(TMap, MoveConstructor) {
    TMap<int, int> m1 = {{1, 10}, {2, 20}};
    TMap<int, int> m2(std::move(m1));
    EXPECT_EQ(m2.Size(), 2u);
    EXPECT_TRUE(m1.Empty());
}

TEST(TMap, CopyAssignment) {
    TMap<int, int> m1 = {{1, 10}, {2, 20}};
    TMap<int, int> m2;
    m2 = m1;
    EXPECT_EQ(m2[1], 10);
    EXPECT_EQ(m2[2], 20);
}

TEST(TMap, MoveAssignment) {
    TMap<int, int> m1 = {{1, 10}, {2, 20}};
    TMap<int, int> m2;
    m2 = std::move(m1);
    EXPECT_EQ(m2.Size(), 2u);
    EXPECT_TRUE(m1.Empty());
}

TEST(TMap, At) {
    TMap<int, int> m = {{1, 10}, {2, 20}};
    EXPECT_EQ(m.At(1), 10);
    EXPECT_EQ(m.At(2), 20);
    EXPECT_THROW(m.At(3), const char*);
}

TEST(TMap, SubscriptOperator) {
    TMap<int, int> m;
    m[1] = 10;
    m[2] = 20;
    EXPECT_EQ(m[1], 10);
    EXPECT_EQ(m[2], 20);
    
    int& val = m[3];
    EXPECT_EQ(val, 0);
    val = 30;
    EXPECT_EQ(m[3], 30);
}

TEST(TMap, Insert) {
    TMap<int, int> m;
    EXPECT_TRUE(m.Insert(1, 10));
    EXPECT_TRUE(m.Insert(2, 20));
    EXPECT_FALSE(m.Insert(1, 100));
    
    EXPECT_EQ(m.Size(), 2u);
    EXPECT_EQ(m[1], 100);
    EXPECT_EQ(m[2], 20);
}

TEST(TMap, Erase) {
    TMap<int, int> m = {{1, 10}, {2, 20}, {3, 30}};
    EXPECT_TRUE(m.Erase(2));
    EXPECT_FALSE(m.Erase(2));
    EXPECT_EQ(m.Size(), 2u);
    EXPECT_FALSE(m.Contains(2));
}

TEST(TMap, Clear) {
    TMap<int, int> m = {{1, 10}, {2, 20}};
    m.Clear();
    EXPECT_TRUE(m.Empty());
    EXPECT_EQ(m.Size(), 0u);
}

TEST(TMap, Find) {
    TMap<int, int> m = {{1, 10}, {2, 20}};
    auto it = m.Find(1);
    EXPECT_NE(it, m.end());
    EXPECT_EQ(it.Key(), 1);
    EXPECT_EQ(it.Value(), 10);
    
    auto notFound = m.Find(999);
    EXPECT_EQ(notFound, m.end());
}

TEST(TMap, Contains) {
    TMap<int, int> m = {{1, 10}, {2, 20}};
    EXPECT_TRUE(m.Contains(1));
    EXPECT_TRUE(m.Contains(2));
    EXPECT_FALSE(m.Contains(3));
}

TEST(TMap, Count) {
    TMap<int, int> m = {{1, 10}, {2, 20}};
    EXPECT_EQ(m.Count(1), 1u);
    EXPECT_EQ(m.Count(999), 0u);
}

TEST(TMap, IteratorsInOrder) {
    TMap<int, int> m = {{3, 30}, {1, 10}, {4, 40}, {1, 10}, {5, 50}, {2, 20}};
    
    int prev = 0;
    for (auto it = m.begin(); it != m.end(); ++it) {
        EXPECT_GT(it.Key(), prev);
        prev = it.Key();
    }
}

TEST(TMap, Swap) {
    TMap<int, int> m1 = {{1, 10}};
    TMap<int, int> m2 = {{2, 20}, {3, 30}};
    m1.Swap(m2);
    EXPECT_EQ(m1.Size(), 2u);
    EXPECT_EQ(m2.Size(), 1u);
    EXPECT_TRUE(m1.Contains(2));
    EXPECT_TRUE(m2.Contains(1));
}

TEST(TMap, LowerBound) {
    TMap<int, int> m = {{1, 10}, {3, 30}, {5, 50}};
    
    auto it = m.LowerBound(3);
    EXPECT_NE(it, m.end());
    EXPECT_EQ(it.Key(), 3);
    
    it = m.LowerBound(2);
    EXPECT_NE(it, m.end());
    EXPECT_EQ(it.Key(), 3);
    
    it = m.LowerBound(6);
    EXPECT_EQ(it, m.end());
}

TEST(TMap, UpperBound) {
    TMap<int, int> m = {{1, 10}, {3, 30}, {5, 50}};
    
    auto it = m.UpperBound(3);
    EXPECT_NE(it, m.end());
    EXPECT_EQ(it.Key(), 5);
    
    it = m.UpperBound(0);
    EXPECT_NE(it, m.end());
    EXPECT_EQ(it.Key(), 1);
    
    it = m.UpperBound(5);
    EXPECT_EQ(it, m.end());
}

TEST(TMap, StringKeys) {
    TMap<TString, int> m;
    m.Insert(TString("apple"), 1);
    m.Insert(TString("banana"), 2);
    m.Insert(TString("cherry"), 3);
    
    EXPECT_EQ(m.Size(), 3u);
    EXPECT_EQ(m.At(TString("apple")), 1);
    EXPECT_EQ(m.At(TString("banana")), 2);
    EXPECT_EQ(m.At(TString("cherry")), 3);
}

TEST(TMap, ManyElements) {
    TMap<int, int> m;
    for (int i = 0; i < 1000; ++i) {
        m[i] = i * 2;
    }
    EXPECT_EQ(m.Size(), 1000u);
    
    // Verify order
    int prev = -1;
    for (auto it = m.begin(); it != m.end(); ++it) {
        EXPECT_GT(it.Key(), prev);
        EXPECT_EQ(it.Value(), it.Key() * 2);
        prev = it.Key();
    }
}

TEST(TMap, EraseMany) {
    TMap<int, int> m;
    for (int i = 0; i < 100; ++i) {
        m[i] = i;
    }
    
    for (int i = 0; i < 100; i += 2) {
        m.Erase(i);
    }
    
    EXPECT_EQ(m.Size(), 50u);
    for (int i = 0; i < 100; ++i) {
        if (i % 2 == 0) {
            EXPECT_FALSE(m.Contains(i));
        } else {
            EXPECT_TRUE(m.Contains(i));
        }
    }
}

TEST(TMap, RandomInsertDelete) {
    TMap<int, int> m;
    
    // Insert in random order
    int values[] = {50, 25, 75, 10, 30, 60, 90, 5, 15, 27, 35};
    for (int v : values) {
        m[v] = v * 10;
    }
    
    EXPECT_EQ(m.Size(), 11u);
    
    // Delete some values
    m.Erase(25);
    m.Erase(75);
    m.Erase(50);
    
    EXPECT_EQ(m.Size(), 8u);
    
    // Verify remaining are in order
    int prev = -1;
    for (auto it = m.begin(); it != m.end(); ++it) {
        EXPECT_GT(it.Key(), prev);
        prev = it.Key();
    }
}

