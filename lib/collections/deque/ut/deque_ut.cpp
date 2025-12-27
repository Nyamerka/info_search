#include <lib/collections/deque/deque.h>
#include <gtest/gtest.h>

using namespace NCollections;

TEST(TDeque, DefaultConstructor) {
    TDeque<int> d;
    EXPECT_TRUE(d.Empty());
    EXPECT_EQ(d.Size(), 0);
}

TEST(TDeque, PushBackAndPopBack) {
    TDeque<int> d;
    d.PushBack(1);
    d.PushBack(2);
    d.PushBack(3);
    
    EXPECT_EQ(d.Size(), 3);
    EXPECT_EQ(d.Front(), 1);
    EXPECT_EQ(d.Back(), 3);
    
    d.PopBack();
    EXPECT_EQ(d.Back(), 2);
    EXPECT_EQ(d.Size(), 2);
}

TEST(TDeque, PushFrontAndPopFront) {
    TDeque<int> d;
    d.PushFront(1);
    d.PushFront(2);
    d.PushFront(3);
    
    EXPECT_EQ(d.Size(), 3);
    EXPECT_EQ(d.Front(), 3);
    EXPECT_EQ(d.Back(), 1);
    
    d.PopFront();
    EXPECT_EQ(d.Front(), 2);
    EXPECT_EQ(d.Size(), 2);
}

TEST(TDeque, MixedPushPop) {
    TDeque<int> d;
    d.PushBack(1);
    d.PushFront(0);
    d.PushBack(2);
    d.PushFront(-1);
    
    EXPECT_EQ(d.Size(), 4);
    EXPECT_EQ(d.Front(), -1);
    EXPECT_EQ(d.Back(), 2);
    
    // Check all elements
    EXPECT_EQ(d[0], -1);
    EXPECT_EQ(d[1], 0);
    EXPECT_EQ(d[2], 1);
    EXPECT_EQ(d[3], 2);
}

TEST(TDeque, RandomAccess) {
    TDeque<int> d;
    for (int i = 0; i < 10; ++i) {
        d.PushBack(i);
    }
    
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(d[i], i);
        EXPECT_EQ(d.At(i), i);
    }
}

TEST(TDeque, InitializerList) {
    TDeque<int> d = {1, 2, 3, 4, 5};
    
    EXPECT_EQ(d.Size(), 5);
    EXPECT_EQ(d[0], 1);
    EXPECT_EQ(d[4], 5);
}

TEST(TDeque, CopyConstructor) {
    TDeque<int> d1;
    d1.PushBack(1);
    d1.PushBack(2);
    d1.PushBack(3);
    
    TDeque<int> d2(d1);
    EXPECT_EQ(d2.Size(), 3);
    EXPECT_EQ(d2[0], 1);
    EXPECT_EQ(d2[2], 3);
    
    // Original should be unchanged
    EXPECT_EQ(d1.Size(), 3);
}

TEST(TDeque, MoveConstructor) {
    TDeque<int> d1;
    d1.PushBack(1);
    d1.PushBack(2);
    d1.PushBack(3);
    
    TDeque<int> d2(std::move(d1));
    EXPECT_EQ(d2.Size(), 3);
    EXPECT_EQ(d2[0], 1);
    EXPECT_TRUE(d1.Empty());
}

TEST(TDeque, Assignment) {
    TDeque<int> d1;
    d1.PushBack(1);
    d1.PushBack(2);
    
    TDeque<int> d2;
    d2 = d1;
    EXPECT_EQ(d2.Size(), 2);
    EXPECT_EQ(d2[0], 1);
    
    TDeque<int> d3;
    d3.PushBack(10);
    d3 = std::move(d1);
    EXPECT_EQ(d3.Size(), 2);
    EXPECT_EQ(d3[0], 1);
}

TEST(TDeque, Clear) {
    TDeque<int> d;
    d.PushBack(1);
    d.PushBack(2);
    d.PushBack(3);
    
    d.Clear();
    EXPECT_TRUE(d.Empty());
    EXPECT_EQ(d.Size(), 0);
}

TEST(TDeque, Resize) {
    TDeque<int> d;
    d.Resize(5);
    EXPECT_EQ(d.Size(), 5);
    
    d.Resize(3);
    EXPECT_EQ(d.Size(), 3);
    
    d.Resize(10, 42);
    EXPECT_EQ(d.Size(), 10);
    EXPECT_EQ(d[5], 42);
    EXPECT_EQ(d[9], 42);
}

TEST(TDeque, LargeScale) {
    TDeque<int> d;
    
    // Add many elements from back
    for (int i = 0; i < 1000; ++i) {
        d.PushBack(i);
    }
    EXPECT_EQ(d.Size(), 1000);
    
    // Add many elements from front
    for (int i = 0; i < 1000; ++i) {
        d.PushFront(-i - 1);
    }
    EXPECT_EQ(d.Size(), 2000);
    
    // Verify order
    EXPECT_EQ(d.Front(), -1000);
    EXPECT_EQ(d.Back(), 999);
    
    // Check random access
    EXPECT_EQ(d[0], -1000);
    EXPECT_EQ(d[999], -1);
    EXPECT_EQ(d[1000], 0);
    EXPECT_EQ(d[1999], 999);
}

TEST(TDeque, Emplace) {
    TDeque<std::pair<int, int>> d;
    d.EmplaceBack(1, 2);
    d.EmplaceFront(3, 4);
    
    EXPECT_EQ(d[0].first, 3);
    EXPECT_EQ(d[0].second, 4);
    EXPECT_EQ(d[1].first, 1);
    EXPECT_EQ(d[1].second, 2);
}

TEST(TDeque, Swap) {
    TDeque<int> d1 = {1, 2, 3};
    TDeque<int> d2 = {4, 5};
    
    d1.Swap(d2);
    
    EXPECT_EQ(d1.Size(), 2);
    EXPECT_EQ(d1[0], 4);
    EXPECT_EQ(d2.Size(), 3);
    EXPECT_EQ(d2[0], 1);
}

TEST(TDeque, Equality) {
    TDeque<int> d1 = {1, 2, 3};
    TDeque<int> d2 = {1, 2, 3};
    TDeque<int> d3 = {1, 2, 4};
    
    EXPECT_TRUE(d1 == d2);
    EXPECT_FALSE(d1 == d3);
    EXPECT_TRUE(d1 != d3);
}

TEST(TDeque, Comparison) {
    TDeque<int> d1 = {1, 2, 3};
    TDeque<int> d2 = {1, 2, 4};
    TDeque<int> d3 = {1, 2};
    
    EXPECT_TRUE(d1 < d2);
    EXPECT_TRUE(d3 < d1);
    EXPECT_TRUE(d1 <= d2);
    EXPECT_TRUE(d2 > d1);
    EXPECT_TRUE(d2 >= d1);
}

TEST(TDeque, Iterator) {
    TDeque<int> d = {1, 2, 3, 4, 5};
    
    int sum = 0;
    for (auto it = d.begin(); it != d.end(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(sum, 15);
    
    // Range-based for
    sum = 0;
    for (int x : d) {
        sum += x;
    }
    EXPECT_EQ(sum, 15);
}

TEST(TDeque, IteratorArithmetic) {
    TDeque<int> d = {0, 1, 2, 3, 4};
    
    auto it = d.begin();
    EXPECT_EQ(*it, 0);
    
    it += 2;
    EXPECT_EQ(*it, 2);
    
    it -= 1;
    EXPECT_EQ(*it, 1);
    
    auto it2 = it + 3;
    EXPECT_EQ(*it2, 4);
    
    EXPECT_EQ(it2 - it, 3);
}

TEST(TDeque, At) {
    TDeque<int> d = {1, 2, 3};
    
    EXPECT_EQ(d.At(0), 1);
    EXPECT_EQ(d.At(2), 3);
    
    bool threw = false;
    try {
        d.At(10);
    } catch (...) {
        threw = true;
    }
    EXPECT_TRUE(threw);
}

TEST(TDeque, CountConstructor) {
    TDeque<int> d1(5);
    EXPECT_EQ(d1.Size(), 5);
    
    TDeque<int> d2(5, 42);
    EXPECT_EQ(d2.Size(), 5);
    EXPECT_EQ(d2[0], 42);
    EXPECT_EQ(d2[4], 42);
}

TEST(TDeque, AlternatingOperations) {
    TDeque<int> d;
    
    // Alternate push front and back
    for (int i = 0; i < 100; ++i) {
        if (i % 2 == 0) {
            d.PushBack(i);
        } else {
            d.PushFront(i);
        }
    }
    
    EXPECT_EQ(d.Size(), 100);
    
    // Pop alternately
    for (int i = 0; i < 50; ++i) {
        d.PopFront();
        d.PopBack();
    }
    
    EXPECT_TRUE(d.Empty());
}

TEST(TDeque, ModifyThroughIterator) {
    TDeque<int> d = {1, 2, 3, 4, 5};
    
    for (auto it = d.begin(); it != d.end(); ++it) {
        *it *= 2;
    }
    
    EXPECT_EQ(d[0], 2);
    EXPECT_EQ(d[2], 6);
    EXPECT_EQ(d[4], 10);
}


