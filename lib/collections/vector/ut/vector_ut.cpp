#include <lib/collections/vector/vector.h>

#include <gtest/gtest.h>

using namespace NCollections;

TEST(TVector, DefaultConstructor) {
    TVector<int> v;
    EXPECT_TRUE(v.Empty());
    EXPECT_EQ(v.Size(), 0u);
    EXPECT_EQ(v.Capacity(), 0u);
}

TEST(TVector, SizeConstructor) {
    TVector<int> v(5);
    EXPECT_FALSE(v.Empty());
    EXPECT_EQ(v.Size(), 5u);
    EXPECT_GE(v.Capacity(), 5u);
    for (size_t i = 0; i < v.Size(); ++i) {
        EXPECT_EQ(v[i], 0);
    }
}

TEST(TVector, SizeValueConstructor) {
    TVector<int> v(5, 42);
    EXPECT_EQ(v.Size(), 5u);
    for (size_t i = 0; i < v.Size(); ++i) {
        EXPECT_EQ(v[i], 42);
    }
}

TEST(TVector, InitializerListConstructor) {
    TVector<int> v = {1, 2, 3, 4, 5};
    EXPECT_EQ(v.Size(), 5u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
    EXPECT_EQ(v[4], 5);
}

TEST(TVector, CopyConstructor) {
    TVector<int> v1 = {1, 2, 3};
    TVector<int> v2(v1);
    EXPECT_EQ(v1.Size(), v2.Size());
    for (size_t i = 0; i < v1.Size(); ++i) {
        EXPECT_EQ(v1[i], v2[i]);
    }
    v2[0] = 100;
    EXPECT_EQ(v1[0], 1);
    EXPECT_EQ(v2[0], 100);
}

TEST(TVector, MoveConstructor) {
    TVector<int> v1 = {1, 2, 3};
    TVector<int> v2(std::move(v1));
    EXPECT_EQ(v2.Size(), 3u);
    EXPECT_EQ(v2[0], 1);
    EXPECT_EQ(v2[1], 2);
    EXPECT_EQ(v2[2], 3);
    EXPECT_TRUE(v1.Empty());
}

TEST(TVector, CopyAssignment) {
    TVector<int> v1 = {1, 2, 3};
    TVector<int> v2;
    v2 = v1;
    EXPECT_EQ(v1.Size(), v2.Size());
    for (size_t i = 0; i < v1.Size(); ++i) {
        EXPECT_EQ(v1[i], v2[i]);
    }
}

TEST(TVector, MoveAssignment) {
    TVector<int> v1 = {1, 2, 3};
    TVector<int> v2;
    v2 = std::move(v1);
    EXPECT_EQ(v2.Size(), 3u);
    EXPECT_TRUE(v1.Empty());
}

TEST(TVector, At) {
    TVector<int> v = {1, 2, 3};
    EXPECT_EQ(v.At(0), 1);
    EXPECT_EQ(v.At(1), 2);
    EXPECT_EQ(v.At(2), 3);
    EXPECT_THROW(v.At(3), const char*);
}

TEST(TVector, FrontBack) {
    TVector<int> v = {1, 2, 3};
    EXPECT_EQ(v.Front(), 1);
    EXPECT_EQ(v.Back(), 3);
    v.Front() = 10;
    v.Back() = 30;
    EXPECT_EQ(v.Front(), 10);
    EXPECT_EQ(v.Back(), 30);
}

TEST(TVector, Data) {
    TVector<int> v = {1, 2, 3};
    int* data = v.Data();
    EXPECT_EQ(data[0], 1);
    EXPECT_EQ(data[1], 2);
    EXPECT_EQ(data[2], 3);
}

TEST(TVector, Iterators) {
    TVector<int> v = {1, 2, 3};
    int sum = 0;
    for (auto it = v.begin(); it != v.end(); ++it) {
        sum += *it;
    }
    EXPECT_EQ(sum, 6);
}

TEST(TVector, RangeBasedFor) {
    TVector<int> v = {1, 2, 3};
    int sum = 0;
    for (int x : v) {
        sum += x;
    }
    EXPECT_EQ(sum, 6);
}

TEST(TVector, Reserve) {
    TVector<int> v;
    v.Reserve(100);
    EXPECT_GE(v.Capacity(), 100u);
    EXPECT_EQ(v.Size(), 0u);
}

TEST(TVector, ShrinkToFit) {
    TVector<int> v = {1, 2, 3};
    v.Reserve(100);
    v.ShrinkToFit();
    EXPECT_EQ(v.Capacity(), 3u);
}

TEST(TVector, Clear) {
    TVector<int> v = {1, 2, 3};
    v.Clear();
    EXPECT_TRUE(v.Empty());
    EXPECT_EQ(v.Size(), 0u);
}

TEST(TVector, PushBack) {
    TVector<int> v;
    v.PushBack(1);
    v.PushBack(2);
    v.PushBack(3);
    EXPECT_EQ(v.Size(), 3u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
}

TEST(TVector, PushBackMove) {
    TVector<TVector<int>> v;
    TVector<int> inner = {1, 2, 3};
    v.PushBack(std::move(inner));
    EXPECT_EQ(v.Size(), 1u);
    EXPECT_EQ(v[0].Size(), 3u);
}

TEST(TVector, EmplaceBack) {
    TVector<TVector<int>> v;
    v.EmplaceBack(3, 42);
    EXPECT_EQ(v.Size(), 1u);
    EXPECT_EQ(v[0].Size(), 3u);
    EXPECT_EQ(v[0][0], 42);
}

TEST(TVector, PopBack) {
    TVector<int> v = {1, 2, 3};
    v.PopBack();
    EXPECT_EQ(v.Size(), 2u);
    EXPECT_EQ(v.Back(), 2);
}

TEST(TVector, Resize) {
    TVector<int> v = {1, 2, 3};
    v.Resize(5);
    EXPECT_EQ(v.Size(), 5u);
    EXPECT_EQ(v[3], 0);
    EXPECT_EQ(v[4], 0);
    
    v.Resize(2);
    EXPECT_EQ(v.Size(), 2u);
}

TEST(TVector, ResizeWithValue) {
    TVector<int> v = {1, 2, 3};
    v.Resize(5, 42);
    EXPECT_EQ(v.Size(), 5u);
    EXPECT_EQ(v[3], 42);
    EXPECT_EQ(v[4], 42);
}

TEST(TVector, Swap) {
    TVector<int> v1 = {1, 2, 3};
    TVector<int> v2 = {4, 5};
    v1.Swap(v2);
    EXPECT_EQ(v1.Size(), 2u);
    EXPECT_EQ(v2.Size(), 3u);
    EXPECT_EQ(v1[0], 4);
    EXPECT_EQ(v2[0], 1);
}

TEST(TVector, Erase) {
    TVector<int> v = {1, 2, 3, 4, 5};
    v.Erase(v.begin() + 2);
    EXPECT_EQ(v.Size(), 4u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 4);
    EXPECT_EQ(v[3], 5);
}

TEST(TVector, Insert) {
    TVector<int> v = {1, 2, 4, 5};
    v.Insert(v.begin() + 2, 3);
    EXPECT_EQ(v.Size(), 5u);
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);
    EXPECT_EQ(v[3], 4);
    EXPECT_EQ(v[4], 5);
}

TEST(TVector, Equality) {
    TVector<int> v1 = {1, 2, 3};
    TVector<int> v2 = {1, 2, 3};
    TVector<int> v3 = {1, 2, 4};
    EXPECT_TRUE(v1 == v2);
    EXPECT_FALSE(v1 == v3);
    EXPECT_FALSE(v1 != v2);
    EXPECT_TRUE(v1 != v3);
}

TEST(TVector, ManyElements) {
    TVector<int> v;
    for (int i = 0; i < 10000; ++i) {
        v.PushBack(i);
    }
    EXPECT_EQ(v.Size(), 10000u);
    for (int i = 0; i < 10000; ++i) {
        EXPECT_EQ(v[i], i);
    }
}

