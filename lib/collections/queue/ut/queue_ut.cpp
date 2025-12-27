#include <lib/collections/queue/queue.h>
#include <gtest/gtest.h>

using namespace NCollections;

TEST(TQueue, DefaultConstructor) {
    TQueue<int> q;
    EXPECT_TRUE(q.Empty());
    EXPECT_EQ(q.Size(), 0);
}

TEST(TQueue, PushAndPop) {
    TQueue<int> q;
    q.Push(1);
    q.Push(2);
    q.Push(3);
    
    EXPECT_EQ(q.Size(), 3);
    EXPECT_EQ(q.Front(), 1);
    EXPECT_EQ(q.Back(), 3);
    
    q.Pop();
    EXPECT_EQ(q.Front(), 2);
    EXPECT_EQ(q.Size(), 2);
    
    q.Pop();
    EXPECT_EQ(q.Front(), 3);
    EXPECT_EQ(q.Size(), 1);
    
    q.Pop();
    EXPECT_TRUE(q.Empty());
}

TEST(TQueue, MoveSemantics) {
    TQueue<int> q1;
    q1.Push(1);
    q1.Push(2);
    q1.Push(3);
    
    TQueue<int> q2(std::move(q1));
    EXPECT_EQ(q2.Size(), 3);
    EXPECT_EQ(q2.Front(), 1);
    EXPECT_TRUE(q1.Empty());
}

TEST(TQueue, CopyConstructor) {
    TQueue<int> q1;
    q1.Push(1);
    q1.Push(2);
    q1.Push(3);
    
    TQueue<int> q2(q1);
    EXPECT_EQ(q2.Size(), 3);
    EXPECT_EQ(q2.Front(), 1);
    EXPECT_EQ(q1.Size(), 3);
}

TEST(TQueue, Assignment) {
    TQueue<int> q1;
    q1.Push(1);
    q1.Push(2);
    
    TQueue<int> q2;
    q2 = q1;
    EXPECT_EQ(q2.Size(), 2);
    EXPECT_EQ(q2.Front(), 1);
    
    TQueue<int> q3;
    q3.Push(10);
    q3 = std::move(q1);
    EXPECT_EQ(q3.Size(), 2);
    EXPECT_EQ(q3.Front(), 1);
}

TEST(TQueue, Clear) {
    TQueue<int> q;
    q.Push(1);
    q.Push(2);
    q.Push(3);
    
    q.Clear();
    EXPECT_TRUE(q.Empty());
    EXPECT_EQ(q.Size(), 0);
}

TEST(TQueue, CircularBehavior) {
    TQueue<int> q;
    
    // Fill and drain multiple times to test circular behavior
    for (int round = 0; round < 5; ++round) {
        for (int i = 0; i < 100; ++i) {
            q.Push(i);
        }
        EXPECT_EQ(q.Size(), 100);
        
        for (int i = 0; i < 100; ++i) {
            EXPECT_EQ(q.Front(), i);
            q.Pop();
        }
        EXPECT_TRUE(q.Empty());
    }
}

TEST(TQueue, LargeScale) {
    TQueue<int> q;
    
    for (int i = 0; i < 10000; ++i) {
        q.Push(i);
    }
    
    EXPECT_EQ(q.Size(), 10000);
    EXPECT_EQ(q.Front(), 0);
    EXPECT_EQ(q.Back(), 9999);
    
    for (int i = 0; i < 10000; ++i) {
        EXPECT_EQ(q.Front(), i);
        q.Pop();
    }
    
    EXPECT_TRUE(q.Empty());
}

TEST(TQueue, Emplace) {
    TQueue<std::pair<int, int>> q;
    q.Emplace(1, 2);
    q.Emplace(3, 4);
    
    EXPECT_EQ(q.Front().first, 1);
    EXPECT_EQ(q.Front().second, 2);
    EXPECT_EQ(q.Back().first, 3);
    EXPECT_EQ(q.Back().second, 4);
}

TEST(TQueue, Reserve) {
    TQueue<int> q;
    q.Reserve(100);
    EXPECT_GE(q.Capacity(), 100);
    
    for (int i = 0; i < 50; ++i) {
        q.Push(i);
    }
    EXPECT_EQ(q.Size(), 50);
}

TEST(TQueue, Swap) {
    TQueue<int> q1;
    q1.Push(1);
    q1.Push(2);
    
    TQueue<int> q2;
    q2.Push(10);
    q2.Push(20);
    q2.Push(30);
    
    q1.Swap(q2);
    
    EXPECT_EQ(q1.Size(), 3);
    EXPECT_EQ(q1.Front(), 10);
    EXPECT_EQ(q2.Size(), 2);
    EXPECT_EQ(q2.Front(), 1);
}

TEST(TQueue, Equality) {
    TQueue<int> q1;
    TQueue<int> q2;
    
    EXPECT_TRUE(q1 == q2);
    
    q1.Push(1);
    EXPECT_FALSE(q1 == q2);
    
    q2.Push(1);
    EXPECT_TRUE(q1 == q2);
    
    q1.Push(2);
    q2.Push(3);
    EXPECT_FALSE(q1 == q2);
}

TEST(TQueue, WithStrings) {
    TQueue<const char*> q;
    q.Push("hello");
    q.Push("world");
    q.Push("!");
    
    EXPECT_EQ(q.Size(), 3);
    
    const char* expected[] = {"hello", "world", "!"};
    for (int i = 0; i < 3; ++i) {
        EXPECT_STREQ(q.Front(), expected[i]);
        q.Pop();
    }
}

TEST(TQueue, InitialCapacity) {
    TQueue<int> q(100);
    EXPECT_GE(q.Capacity(), 100);
    EXPECT_TRUE(q.Empty());
}

TEST(TQueue, MixedPushPop) {
    TQueue<int> q;
    
    // Push some, pop some, push more
    q.Push(1);
    q.Push(2);
    q.Push(3);
    
    q.Pop();  // removes 1
    
    q.Push(4);
    q.Push(5);
    
    q.Pop();  // removes 2
    q.Pop();  // removes 3
    
    EXPECT_EQ(q.Size(), 2);
    EXPECT_EQ(q.Front(), 4);
    EXPECT_EQ(q.Back(), 5);
}


