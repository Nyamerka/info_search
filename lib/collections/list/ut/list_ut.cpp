#include <lib/collections/list/list.h>

#include <gtest/gtest.h>

using namespace NCollections;

TEST(TList, DefaultConstructor) {
    TList<int> l;
    EXPECT_TRUE(l.Empty());
    EXPECT_EQ(l.Size(), 0u);
}

TEST(TList, InitializerListConstructor) {
    TList<int> l = {1, 2, 3, 4, 5};
    EXPECT_EQ(l.Size(), 5u);
    EXPECT_EQ(l.Front(), 1);
    EXPECT_EQ(l.Back(), 5);
}

TEST(TList, CopyConstructor) {
    TList<int> l1 = {1, 2, 3};
    TList<int> l2(l1);
    EXPECT_EQ(l1.Size(), l2.Size());
    auto it1 = l1.begin();
    auto it2 = l2.begin();
    while (it1 != l1.end() && it2 != l2.end()) {
        EXPECT_EQ(*it1, *it2);
        ++it1;
        ++it2;
    }
    l2.Front() = 100;
    EXPECT_NE(l1.Front(), l2.Front());
}

TEST(TList, MoveConstructor) {
    TList<int> l1 = {1, 2, 3};
    TList<int> l2(std::move(l1));
    EXPECT_EQ(l2.Size(), 3u);
    EXPECT_TRUE(l1.Empty());
}

TEST(TList, CopyAssignment) {
    TList<int> l1 = {1, 2, 3};
    TList<int> l2;
    l2 = l1;
    EXPECT_EQ(l1.Size(), l2.Size());
}

TEST(TList, MoveAssignment) {
    TList<int> l1 = {1, 2, 3};
    TList<int> l2;
    l2 = std::move(l1);
    EXPECT_EQ(l2.Size(), 3u);
    EXPECT_TRUE(l1.Empty());
}

TEST(TList, FrontBack) {
    TList<int> l = {1, 2, 3};
    EXPECT_EQ(l.Front(), 1);
    EXPECT_EQ(l.Back(), 3);
    l.Front() = 10;
    l.Back() = 30;
    EXPECT_EQ(l.Front(), 10);
    EXPECT_EQ(l.Back(), 30);
}

TEST(TList, Iterators) {
    TList<int> l = {1, 2, 3};
    int sum = 0;
    for (auto it = l.begin(); it != l.end(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(sum, 6);
}

TEST(TList, RangeBasedFor) {
    TList<int> l = {1, 2, 3};
    int sum = 0;
    for (int x : l) {
        sum += x;
    }
    EXPECT_EQ(sum, 6);
}

TEST(TList, Clear) {
    TList<int> l = {1, 2, 3};
    l.Clear();
    EXPECT_TRUE(l.Empty());
    EXPECT_EQ(l.Size(), 0u);
}

TEST(TList, PushFront) {
    TList<int> l;
    l.PushFront(3);
    l.PushFront(2);
    l.PushFront(1);
    EXPECT_EQ(l.Size(), 3u);
    EXPECT_EQ(l.Front(), 1);
    EXPECT_EQ(l.Back(), 3);
}

TEST(TList, PushBack) {
    TList<int> l;
    l.PushBack(1);
    l.PushBack(2);
    l.PushBack(3);
    EXPECT_EQ(l.Size(), 3u);
    EXPECT_EQ(l.Front(), 1);
    EXPECT_EQ(l.Back(), 3);
}

TEST(TList, EmplaceFront) {
    TList<TList<int>> l;
    l.EmplaceFront(std::initializer_list<int>{1, 2, 3});
    EXPECT_EQ(l.Size(), 1u);
    EXPECT_EQ(l.Front().Size(), 3u);
}

TEST(TList, EmplaceBack) {
    TList<TList<int>> l;
    l.EmplaceBack(std::initializer_list<int>{1, 2, 3});
    EXPECT_EQ(l.Size(), 1u);
    EXPECT_EQ(l.Back().Size(), 3u);
}

TEST(TList, PopFront) {
    TList<int> l = {1, 2, 3};
    l.PopFront();
    EXPECT_EQ(l.Size(), 2u);
    EXPECT_EQ(l.Front(), 2);
}

TEST(TList, PopBack) {
    TList<int> l = {1, 2, 3};
    l.PopBack();
    EXPECT_EQ(l.Size(), 2u);
    EXPECT_EQ(l.Back(), 2);
}

TEST(TList, Insert) {
    TList<int> l = {1, 3};
    auto it = l.begin();
    ++it;
    l.Insert(it, 2);
    EXPECT_EQ(l.Size(), 3u);
    
    auto checkIt = l.begin();
    EXPECT_EQ(*checkIt, 1);
    ++checkIt;
    EXPECT_EQ(*checkIt, 2);
    ++checkIt;
    EXPECT_EQ(*checkIt, 3);
}

TEST(TList, InsertAtBegin) {
    TList<int> l = {2, 3};
    l.Insert(l.begin(), 1);
    EXPECT_EQ(l.Front(), 1);
    EXPECT_EQ(l.Size(), 3u);
}

TEST(TList, InsertAtEnd) {
    TList<int> l = {1, 2};
    l.Insert(l.end(), 3);
    EXPECT_EQ(l.Back(), 3);
    EXPECT_EQ(l.Size(), 3u);
}

TEST(TList, Erase) {
    TList<int> l = {1, 2, 3};
    auto it = l.begin();
    ++it;
    l.Erase(it);
    EXPECT_EQ(l.Size(), 2u);
    
    auto checkIt = l.begin();
    EXPECT_EQ(*checkIt, 1);
    ++checkIt;
    EXPECT_EQ(*checkIt, 3);
}

TEST(TList, EraseFirst) {
    TList<int> l = {1, 2, 3};
    l.Erase(l.begin());
    EXPECT_EQ(l.Front(), 2);
    EXPECT_EQ(l.Size(), 2u);
}

TEST(TList, Swap) {
    TList<int> l1 = {1, 2, 3};
    TList<int> l2 = {4, 5};
    l1.Swap(l2);
    EXPECT_EQ(l1.Size(), 2u);
    EXPECT_EQ(l2.Size(), 3u);
    EXPECT_EQ(l1.Front(), 4);
    EXPECT_EQ(l2.Front(), 1);
}

TEST(TList, Reverse) {
    TList<int> l = {1, 2, 3, 4, 5};
    l.Reverse();
    
    auto it = l.begin();
    EXPECT_EQ(*it++, 5);
    EXPECT_EQ(*it++, 4);
    EXPECT_EQ(*it++, 3);
    EXPECT_EQ(*it++, 2);
    EXPECT_EQ(*it++, 1);
}

TEST(TList, Remove) {
    TList<int> l = {1, 2, 3, 2, 4, 2};
    l.Remove(2);
    EXPECT_EQ(l.Size(), 3u);
    
    auto it = l.begin();
    EXPECT_EQ(*it++, 1);
    EXPECT_EQ(*it++, 3);
    EXPECT_EQ(*it++, 4);
}

TEST(TList, Equality) {
    TList<int> l1 = {1, 2, 3};
    TList<int> l2 = {1, 2, 3};
    TList<int> l3 = {1, 2, 4};
    EXPECT_TRUE(l1 == l2);
    EXPECT_FALSE(l1 == l3);
    EXPECT_FALSE(l1 != l2);
    EXPECT_TRUE(l1 != l3);
}

TEST(TList, SingleElement) {
    TList<int> l;
    l.PushBack(42);
    EXPECT_EQ(l.Front(), 42);
    EXPECT_EQ(l.Back(), 42);
    EXPECT_EQ(l.Size(), 1u);
    l.PopBack();
    EXPECT_TRUE(l.Empty());
}

TEST(TList, ManyElements) {
    TList<int> l;
    for (int i = 0; i < 10000; ++i) {
        l.PushBack(i);
    }
    EXPECT_EQ(l.Size(), 10000u);
    
    int expected = 0;
    for (int x : l) {
        EXPECT_EQ(x, expected++);
    }
}

