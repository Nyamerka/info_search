#pragma once

#include <cstddef>
#include <new>
#include <utility>

namespace NCollections {

/**
 * Высокопроизводительная очередь на основе circular buffer
 * 
 * O(1) для push и pop операций, отличная cache locality
 */
template <typename T>
class TQueue {
public:
    using value_type = T;
    using size_type = size_t;
    using reference = T&;
    using const_reference = const T&;

    static constexpr size_type MIN_CAPACITY = 8;
    static constexpr size_type GROWTH_NUMERATOR = 3;
    static constexpr size_type GROWTH_DENOMINATOR = 2;

public:
    TQueue() : Data_(nullptr), Head_(0), Tail_(0), Size_(0), Capacity_(0) {}

    explicit TQueue(size_type initialCapacity) : Data_(nullptr), Head_(0), Tail_(0), Size_(0), Capacity_(0) {
        if (initialCapacity > 0) {
            Capacity_ = RoundUpToPowerOf2(initialCapacity);
            Data_ = Allocate(Capacity_);
        }
    }

    TQueue(const TQueue& other) : Data_(nullptr), Head_(0), Tail_(0), Size_(0), Capacity_(0) {
        if (other.Size_ > 0) {
            Capacity_ = RoundUpToPowerOf2(other.Size_);
            Data_ = Allocate(Capacity_);
            size_type idx = other.Head_;
            for (size_type i = 0; i < other.Size_; ++i) {
                new (Data_ + i) T(other.Data_[idx]);
                idx = (idx + 1) & (other.Capacity_ - 1);
            }
            Head_ = 0;
            Tail_ = other.Size_;
            Size_ = other.Size_;
        }
    }

    TQueue(TQueue&& other) noexcept
        : Data_(other.Data_), Head_(other.Head_), Tail_(other.Tail_), Size_(other.Size_), Capacity_(other.Capacity_) {
        other.Data_ = nullptr;
        other.Head_ = 0;
        other.Tail_ = 0;
        other.Size_ = 0;
        other.Capacity_ = 0;
    }

    ~TQueue() { Clear(); Deallocate(Data_); }

    TQueue& operator=(const TQueue& other) {
        if (this != &other) { TQueue tmp(other); Swap(tmp); }
        return *this;
    }

    TQueue& operator=(TQueue&& other) noexcept {
        if (this != &other) {
            Clear();
            Deallocate(Data_);
            Data_ = other.Data_;
            Head_ = other.Head_;
            Tail_ = other.Tail_;
            Size_ = other.Size_;
            Capacity_ = other.Capacity_;
            other.Data_ = nullptr;
            other.Head_ = 0;
            other.Tail_ = 0;
            other.Size_ = 0;
            other.Capacity_ = 0;
        }
        return *this;
    }

    reference Front() { return Data_[Head_]; }
    const_reference Front() const { return Data_[Head_]; }
    reference Back() { size_type backIdx = (Tail_ == 0) ? Capacity_ - 1 : Tail_ - 1; return Data_[backIdx]; }
    const_reference Back() const { size_type backIdx = (Tail_ == 0) ? Capacity_ - 1 : Tail_ - 1; return Data_[backIdx]; }

    bool Empty() const noexcept { return Size_ == 0; }
    size_type Size() const noexcept { return Size_; }
    size_type Capacity() const noexcept { return Capacity_; }

    void Reserve(size_type newCapacity) {
        if (newCapacity > Capacity_) Grow(RoundUpToPowerOf2(newCapacity));
    }

    void Push(const T& value) {
        if (Size_ == Capacity_) GrowForInsert();
        new (Data_ + Tail_) T(value);
        Tail_ = (Tail_ + 1) & (Capacity_ - 1);
        ++Size_;
    }

    void Push(T&& value) {
        if (Size_ == Capacity_) GrowForInsert();
        new (Data_ + Tail_) T(std::move(value));
        Tail_ = (Tail_ + 1) & (Capacity_ - 1);
        ++Size_;
    }

    template <typename... Args>
    reference Emplace(Args&&... args) {
        if (Size_ == Capacity_) GrowForInsert();
        new (Data_ + Tail_) T(std::forward<Args>(args)...);
        size_type idx = Tail_;
        Tail_ = (Tail_ + 1) & (Capacity_ - 1);
        ++Size_;
        return Data_[idx];
    }

    void Pop() {
        if (Size_ > 0) {
            Data_[Head_].~T();
            Head_ = (Head_ + 1) & (Capacity_ - 1);
            --Size_;
        }
    }

    void Clear() noexcept {
        while (Size_ > 0) {
            Data_[Head_].~T();
            Head_ = (Head_ + 1) & (Capacity_ - 1);
            --Size_;
        }
        Head_ = 0;
        Tail_ = 0;
    }

    void Swap(TQueue& other) noexcept {
        T* tmpData = Data_; Data_ = other.Data_; other.Data_ = tmpData;
        size_type tmp = Head_; Head_ = other.Head_; other.Head_ = tmp;
        tmp = Tail_; Tail_ = other.Tail_; other.Tail_ = tmp;
        tmp = Size_; Size_ = other.Size_; other.Size_ = tmp;
        tmp = Capacity_; Capacity_ = other.Capacity_; other.Capacity_ = tmp;
    }

    bool operator==(const TQueue& other) const {
        if (Size_ != other.Size_) return false;
        size_type idx1 = Head_, idx2 = other.Head_;
        for (size_type i = 0; i < Size_; ++i) {
            if (!(Data_[idx1] == other.Data_[idx2])) return false;
            idx1 = (idx1 + 1) & (Capacity_ - 1);
            idx2 = (idx2 + 1) & (other.Capacity_ - 1);
        }
        return true;
    }

    bool operator!=(const TQueue& other) const { return !(*this == other); }

private:
    static T* Allocate(size_type n) { return static_cast<T*>(::operator new(n * sizeof(T))); }
    static void Deallocate(T* ptr) { ::operator delete(ptr); }

    static size_type RoundUpToPowerOf2(size_type n) {
        if (n == 0 || n < MIN_CAPACITY) return MIN_CAPACITY;
        --n;
        n |= n >> 1; n |= n >> 2; n |= n >> 4; n |= n >> 8; n |= n >> 16; n |= n >> 32;
        return n + 1;
    }

    void Grow(size_type newCapacity) {
        T* newData = Allocate(newCapacity);
        size_type idx = Head_;
        for (size_type i = 0; i < Size_; ++i) {
            new (newData + i) T(std::move(Data_[idx]));
            Data_[idx].~T();
            idx = (idx + 1) & (Capacity_ - 1);
        }
        Deallocate(Data_);
        Data_ = newData;
        Head_ = 0;
        Tail_ = Size_;
        Capacity_ = newCapacity;
    }

    void GrowForInsert() {
        size_type newCapacity = Capacity_ == 0 ? MIN_CAPACITY : Capacity_ * GROWTH_NUMERATOR / GROWTH_DENOMINATOR;
        newCapacity = RoundUpToPowerOf2(newCapacity);
        Grow(newCapacity);
    }

private:
    T* Data_;
    size_type Head_;
    size_type Tail_;
    size_type Size_;
    size_type Capacity_;
};

} // namespace NCollections

