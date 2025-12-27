#include <lib/collections/heap/heap.h>
#include <gtest/gtest.h>

using namespace NCollections;

TEST(THeap, DefaultConstructor) {
    THeap<int> h;
    EXPECT_TRUE(h.Empty());
    EXPECT_EQ(h.Size(), 0);
}

TEST(THeap, PushAndTop) {
    THeap<int> h;
    h.Push(3);
    EXPECT_EQ(h.Top(), 3);
    
    h.Push(5);
    EXPECT_EQ(h.Top(), 5);
    
    h.Push(1);
    EXPECT_EQ(h.Top(), 5);
    
    h.Push(10);
    EXPECT_EQ(h.Top(), 10);
}

TEST(THeap, Pop) {
    THeap<int> h;
    h.Push(3);
    h.Push(5);
    h.Push(1);
    h.Push(10);
    h.Push(7);
    
    EXPECT_EQ(h.Top(), 10);
    h.Pop();
    EXPECT_EQ(h.Top(), 7);
    h.Pop();
    EXPECT_EQ(h.Top(), 5);
    h.Pop();
    EXPECT_EQ(h.Top(), 3);
    h.Pop();
    EXPECT_EQ(h.Top(), 1);
    h.Pop();
    EXPECT_TRUE(h.Empty());
}

TEST(THeap, ExtractTop) {
    THeap<int> h;
    h.Push(3);
    h.Push(5);
    h.Push(1);
    
    EXPECT_EQ(h.ExtractTop(), 5);
    EXPECT_EQ(h.ExtractTop(), 3);
    EXPECT_EQ(h.ExtractTop(), 1);
    EXPECT_TRUE(h.Empty());
}

TEST(THeap, InitializerList) {
    THeap<int> h = {3, 1, 4, 1, 5, 9, 2, 6};
    
    EXPECT_EQ(h.Size(), 8);
    EXPECT_EQ(h.Top(), 9);
    EXPECT_TRUE(h.IsHeap());
}

TEST(THeap, CopyConstructor) {
    THeap<int> h1;
    h1.Push(5);
    h1.Push(3);
    h1.Push(7);
    
    THeap<int> h2(h1);
    EXPECT_EQ(h2.Size(), 3);
    EXPECT_EQ(h2.Top(), 7);
    EXPECT_TRUE(h2.IsHeap());
}

TEST(THeap, MoveConstructor) {
    THeap<int> h1;
    h1.Push(5);
    h1.Push(3);
    h1.Push(7);
    
    THeap<int> h2(std::move(h1));
    EXPECT_EQ(h2.Size(), 3);
    EXPECT_EQ(h2.Top(), 7);
    EXPECT_TRUE(h1.Empty());
}

TEST(THeap, Assignment) {
    THeap<int> h1;
    h1.Push(5);
    h1.Push(3);
    
    THeap<int> h2;
    h2 = h1;
    EXPECT_EQ(h2.Size(), 2);
    EXPECT_EQ(h2.Top(), 5);
    
    THeap<int> h3;
    h3.Push(10);
    h3 = std::move(h1);
    EXPECT_EQ(h3.Size(), 2);
    EXPECT_EQ(h3.Top(), 5);
}

TEST(THeap, Clear) {
    THeap<int> h = {1, 2, 3, 4, 5};
    h.Clear();
    EXPECT_TRUE(h.Empty());
    EXPECT_EQ(h.Size(), 0);
}

TEST(THeap, MinHeap) {
    TMinHeap<int> h;
    h.Push(5);
    h.Push(3);
    h.Push(7);
    h.Push(1);
    
    EXPECT_EQ(h.Top(), 1);
    h.Pop();
    EXPECT_EQ(h.Top(), 3);
    h.Pop();
    EXPECT_EQ(h.Top(), 5);
    h.Pop();
    EXPECT_EQ(h.Top(), 7);
}

TEST(THeap, MaxHeap) {
    TMaxHeap<int> h;
    h.Push(5);
    h.Push(3);
    h.Push(7);
    h.Push(1);
    
    EXPECT_EQ(h.Top(), 7);
    h.Pop();
    EXPECT_EQ(h.Top(), 5);
    h.Pop();
    EXPECT_EQ(h.Top(), 3);
    h.Pop();
    EXPECT_EQ(h.Top(), 1);
}

TEST(THeap, IsHeap) {
    THeap<int> h;
    EXPECT_TRUE(h.IsHeap());
    
    h.Push(10);
    EXPECT_TRUE(h.IsHeap());
    
    h.Push(5);
    h.Push(8);
    h.Push(3);
    h.Push(4);
    EXPECT_TRUE(h.IsHeap());
}

TEST(THeap, LargeScale) {
    THeap<int> h;
    
    for (int i = 0; i < 1000; ++i) {
        h.Push(i);
    }
    
    EXPECT_EQ(h.Size(), 1000);
    EXPECT_EQ(h.Top(), 999);
    EXPECT_TRUE(h.IsHeap());
    
    for (int i = 999; i >= 0; --i) {
        EXPECT_EQ(h.Top(), i);
        h.Pop();
    }
    
    EXPECT_TRUE(h.Empty());
}

TEST(THeap, Emplace) {
    THeap<std::pair<int, int>> h;
    h.Emplace(1, 2);
    h.Emplace(3, 4);
    h.Emplace(2, 1);
    
    EXPECT_EQ(h.Top().first, 3);
    EXPECT_EQ(h.Top().second, 4);
}

TEST(THeap, Swap) {
    THeap<int> h1 = {1, 2, 3};
    THeap<int> h2 = {4, 5};
    
    h1.Swap(h2);
    
    EXPECT_EQ(h1.Size(), 2);
    EXPECT_EQ(h1.Top(), 5);
    EXPECT_EQ(h2.Size(), 3);
    EXPECT_EQ(h2.Top(), 3);
}

TEST(THeap, Reserve) {
    THeap<int> h;
    h.Reserve(100);
    EXPECT_GE(h.Capacity(), 100);
    EXPECT_TRUE(h.Empty());
}

TEST(THeap, HeapSort) {
    int arr[] = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    int n = sizeof(arr) / sizeof(arr[0]);
    
    THeap<int> h;
    for (int i = 0; i < n; ++i) {
        h.Push(arr[i]);
    }
    
    int sorted[11];
    for (int i = n - 1; i >= 0; --i) {
        sorted[i] = h.ExtractTop();
    }
    
    for (int i = 0; i < n - 1; ++i) {
        EXPECT_LE(sorted[i], sorted[i + 1]);
    }
}

TEST(THeap, DuplicateValues) {
    THeap<int> h;
    h.Push(5);
    h.Push(5);
    h.Push(5);
    h.Push(3);
    h.Push(3);
    
    EXPECT_EQ(h.Size(), 5);
    EXPECT_EQ(h.Top(), 5);
    EXPECT_TRUE(h.IsHeap());
    
    h.Pop();
    EXPECT_EQ(h.Top(), 5);
    h.Pop();
    EXPECT_EQ(h.Top(), 5);
    h.Pop();
    EXPECT_EQ(h.Top(), 3);
}

TEST(THeap, SingleElement) {
    THeap<int> h;
    h.Push(42);
    
    EXPECT_EQ(h.Size(), 1);
    EXPECT_EQ(h.Top(), 42);
    EXPECT_TRUE(h.IsHeap());
    
    h.Pop();
    EXPECT_TRUE(h.Empty());
}

TEST(THeap, Equality) {
    THeap<int> h1 = {1, 2, 3};
    THeap<int> h2 = {1, 2, 3};
    THeap<int> h3 = {1, 2, 4};
    
    EXPECT_TRUE(h1 == h2);
    EXPECT_FALSE(h1 == h3);
    EXPECT_TRUE(h1 != h3);
}

TEST(THeap, MinHeapSort) {
    TMinHeap<int> h = {5, 2, 8, 1, 9, 3};
    
    int prev = h.ExtractTop();
    while (!h.Empty()) {
        int curr = h.ExtractTop();
        EXPECT_GE(curr, prev);
        prev = curr;
    }
}


