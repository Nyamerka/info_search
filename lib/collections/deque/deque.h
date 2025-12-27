#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <initializer_list>

namespace NCollections {

/**
 * Высокопроизводительная двусторонняя очередь (deque)
 * 
 * Блочный массив: O(1) amortized push/pop с обеих сторон, O(1) random access
 */
template <typename T>
class TDeque {
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;

    static constexpr size_type BLOCK_SIZE = 512 / sizeof(T) > 0 ? 512 / sizeof(T) : 1;
    static constexpr size_type MIN_MAP_SIZE = 8;
    static constexpr size_type GROWTH_FACTOR = 2;

private:
    T** Map_;
    size_type MapSize_;
    size_type FrontBlock_;
    size_type BackBlock_;
    size_type FrontOffset_;
    size_type BackOffset_;
    size_type Size_;

public:
    class TIterator {
    public:
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        TIterator() : Deque_(nullptr), Index_(0) {}
        TIterator(TDeque* deque, size_type index) : Deque_(deque), Index_(index) {}

        reference operator*() { return (*Deque_)[Index_]; }
        pointer operator->() { return &(*Deque_)[Index_]; }
        TIterator& operator++() { ++Index_; return *this; }
        TIterator operator++(int) { TIterator tmp = *this; ++Index_; return tmp; }
        TIterator& operator--() { --Index_; return *this; }
        TIterator operator--(int) { TIterator tmp = *this; --Index_; return tmp; }
        TIterator& operator+=(difference_type n) { Index_ += n; return *this; }
        TIterator& operator-=(difference_type n) { Index_ -= n; return *this; }
        TIterator operator+(difference_type n) const { return TIterator(Deque_, Index_ + n); }
        TIterator operator-(difference_type n) const { return TIterator(Deque_, Index_ - n); }
        difference_type operator-(const TIterator& other) const { return Index_ - other.Index_; }
        reference operator[](difference_type n) { return (*Deque_)[Index_ + n]; }
        bool operator==(const TIterator& other) const { return Index_ == other.Index_; }
        bool operator!=(const TIterator& other) const { return Index_ != other.Index_; }
        bool operator<(const TIterator& other) const { return Index_ < other.Index_; }
        bool operator<=(const TIterator& other) const { return Index_ <= other.Index_; }
        bool operator>(const TIterator& other) const { return Index_ > other.Index_; }
        bool operator>=(const TIterator& other) const { return Index_ >= other.Index_; }

    private:
        TDeque* Deque_;
        size_type Index_;
    };

    class TConstIterator {
    public:
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        TConstIterator() : Deque_(nullptr), Index_(0) {}
        TConstIterator(const TDeque* deque, size_type index) : Deque_(deque), Index_(index) {}

        reference operator*() const { return (*Deque_)[Index_]; }
        pointer operator->() const { return &(*Deque_)[Index_]; }
        TConstIterator& operator++() { ++Index_; return *this; }
        TConstIterator operator++(int) { TConstIterator tmp = *this; ++Index_; return tmp; }
        TConstIterator& operator--() { --Index_; return *this; }
        TConstIterator operator--(int) { TConstIterator tmp = *this; --Index_; return tmp; }
        TConstIterator& operator+=(difference_type n) { Index_ += n; return *this; }
        TConstIterator& operator-=(difference_type n) { Index_ -= n; return *this; }
        TConstIterator operator+(difference_type n) const { return TConstIterator(Deque_, Index_ + n); }
        TConstIterator operator-(difference_type n) const { return TConstIterator(Deque_, Index_ - n); }
        difference_type operator-(const TConstIterator& other) const { return Index_ - other.Index_; }
        reference operator[](difference_type n) const { return (*Deque_)[Index_ + n]; }
        bool operator==(const TConstIterator& other) const { return Index_ == other.Index_; }
        bool operator!=(const TConstIterator& other) const { return Index_ != other.Index_; }
        bool operator<(const TConstIterator& other) const { return Index_ < other.Index_; }
        bool operator<=(const TConstIterator& other) const { return Index_ <= other.Index_; }
        bool operator>(const TConstIterator& other) const { return Index_ > other.Index_; }
        bool operator>=(const TConstIterator& other) const { return Index_ >= other.Index_; }

    private:
        const TDeque* Deque_;
        size_type Index_;
    };

    using iterator = TIterator;
    using const_iterator = TConstIterator;

public:
    TDeque() : Map_(nullptr), MapSize_(0), FrontBlock_(0), BackBlock_(0), FrontOffset_(0), BackOffset_(0), Size_(0) {}
    explicit TDeque(size_type count) : Map_(nullptr), MapSize_(0), FrontBlock_(0), BackBlock_(0), FrontOffset_(0), BackOffset_(0), Size_(0) { Resize(count); }
    TDeque(size_type count, const T& value) : Map_(nullptr), MapSize_(0), FrontBlock_(0), BackBlock_(0), FrontOffset_(0), BackOffset_(0), Size_(0) { Resize(count, value); }

    TDeque(std::initializer_list<T> init) : Map_(nullptr), MapSize_(0), FrontBlock_(0), BackBlock_(0), FrontOffset_(0), BackOffset_(0), Size_(0) {
        for (const auto& item : init) PushBack(item);
    }

    TDeque(const TDeque& other) : Map_(nullptr), MapSize_(0), FrontBlock_(0), BackBlock_(0), FrontOffset_(0), BackOffset_(0), Size_(0) {
        for (size_type i = 0; i < other.Size_; ++i) PushBack(other[i]);
    }

    TDeque(TDeque&& other) noexcept
        : Map_(other.Map_), MapSize_(other.MapSize_), FrontBlock_(other.FrontBlock_), BackBlock_(other.BackBlock_),
          FrontOffset_(other.FrontOffset_), BackOffset_(other.BackOffset_), Size_(other.Size_) {
        other.Map_ = nullptr;
        other.MapSize_ = 0;
        other.FrontBlock_ = 0;
        other.BackBlock_ = 0;
        other.FrontOffset_ = 0;
        other.BackOffset_ = 0;
        other.Size_ = 0;
    }

    ~TDeque() { Clear(); DeallocateMap(); }

    TDeque& operator=(const TDeque& other) {
        if (this != &other) { TDeque tmp(other); Swap(tmp); }
        return *this;
    }

    TDeque& operator=(TDeque&& other) noexcept {
        if (this != &other) {
            Clear();
            DeallocateMap();
            Map_ = other.Map_;
            MapSize_ = other.MapSize_;
            FrontBlock_ = other.FrontBlock_;
            BackBlock_ = other.BackBlock_;
            FrontOffset_ = other.FrontOffset_;
            BackOffset_ = other.BackOffset_;
            Size_ = other.Size_;
            other.Map_ = nullptr;
            other.MapSize_ = 0;
            other.FrontBlock_ = 0;
            other.BackBlock_ = 0;
            other.FrontOffset_ = 0;
            other.BackOffset_ = 0;
            other.Size_ = 0;
        }
        return *this;
    }

    TDeque& operator=(std::initializer_list<T> init) { TDeque tmp(init); Swap(tmp); return *this; }

    reference At(size_type pos) {
        if (pos >= Size_) throw "Index out of range";
        return (*this)[pos];
    }

    const_reference At(size_type pos) const {
        if (pos >= Size_) throw "Index out of range";
        return (*this)[pos];
    }

    reference operator[](size_type pos) {
        size_type absolutePos = FrontOffset_ + pos;
        size_type blockIdx = FrontBlock_ + absolutePos / BLOCK_SIZE;
        size_type offset = absolutePos % BLOCK_SIZE;
        return Map_[blockIdx][offset];
    }

    const_reference operator[](size_type pos) const {
        size_type absolutePos = FrontOffset_ + pos;
        size_type blockIdx = FrontBlock_ + absolutePos / BLOCK_SIZE;
        size_type offset = absolutePos % BLOCK_SIZE;
        return Map_[blockIdx][offset];
    }

    reference Front() { return Map_[FrontBlock_][FrontOffset_]; }
    const_reference Front() const { return Map_[FrontBlock_][FrontOffset_]; }
    reference Back() { return BackOffset_ == 0 ? Map_[BackBlock_ - 1][BLOCK_SIZE - 1] : Map_[BackBlock_][BackOffset_ - 1]; }
    const_reference Back() const { return BackOffset_ == 0 ? Map_[BackBlock_ - 1][BLOCK_SIZE - 1] : Map_[BackBlock_][BackOffset_ - 1]; }

    iterator begin() noexcept { return iterator(this, 0); }
    const_iterator begin() const noexcept { return const_iterator(this, 0); }
    iterator end() noexcept { return iterator(this, Size_); }
    const_iterator end() const noexcept { return const_iterator(this, Size_); }

    bool Empty() const noexcept { return Size_ == 0; }
    size_type Size() const noexcept { return Size_; }
    void ShrinkToFit() {}

    void Clear() noexcept { while (Size_ > 0) PopBack(); }

    void PushBack(const T& value) {
        EnsureBackCapacity();
        new (Map_[BackBlock_] + BackOffset_) T(value);
        AdvanceBack();
        ++Size_;
    }

    void PushBack(T&& value) {
        EnsureBackCapacity();
        new (Map_[BackBlock_] + BackOffset_) T(std::move(value));
        AdvanceBack();
        ++Size_;
    }

    template <typename... Args>
    reference EmplaceBack(Args&&... args) {
        EnsureBackCapacity();
        T* ptr = new (Map_[BackBlock_] + BackOffset_) T(std::forward<Args>(args)...);
        AdvanceBack();
        ++Size_;
        return *ptr;
    }

    void PopBack() {
        if (Size_ > 0) {
            RetreatBack();
            Map_[BackBlock_][BackOffset_].~T();
            --Size_;
        }
    }

    void PushFront(const T& value) {
        EnsureFrontCapacity();
        RetreatFront();
        new (Map_[FrontBlock_] + FrontOffset_) T(value);
        ++Size_;
    }

    void PushFront(T&& value) {
        EnsureFrontCapacity();
        RetreatFront();
        new (Map_[FrontBlock_] + FrontOffset_) T(std::move(value));
        ++Size_;
    }

    template <typename... Args>
    reference EmplaceFront(Args&&... args) {
        EnsureFrontCapacity();
        RetreatFront();
        T* ptr = new (Map_[FrontBlock_] + FrontOffset_) T(std::forward<Args>(args)...);
        ++Size_;
        return *ptr;
    }

    void PopFront() {
        if (Size_ > 0) {
            Map_[FrontBlock_][FrontOffset_].~T();
            AdvanceFront();
            --Size_;
        }
    }

    void Resize(size_type count) {
        while (Size_ > count) PopBack();
        while (Size_ < count) {
            EnsureBackCapacity();
            new (Map_[BackBlock_] + BackOffset_) T();
            AdvanceBack();
            ++Size_;
        }
    }

    void Resize(size_type count, const T& value) {
        while (Size_ > count) PopBack();
        while (Size_ < count) PushBack(value);
    }

    void Swap(TDeque& other) noexcept {
        T** tmpMap = Map_; Map_ = other.Map_; other.Map_ = tmpMap;
        size_type tmp = MapSize_; MapSize_ = other.MapSize_; other.MapSize_ = tmp;
        tmp = FrontBlock_; FrontBlock_ = other.FrontBlock_; other.FrontBlock_ = tmp;
        tmp = BackBlock_; BackBlock_ = other.BackBlock_; other.BackBlock_ = tmp;
        tmp = FrontOffset_; FrontOffset_ = other.FrontOffset_; other.FrontOffset_ = tmp;
        tmp = BackOffset_; BackOffset_ = other.BackOffset_; other.BackOffset_ = tmp;
        tmp = Size_; Size_ = other.Size_; other.Size_ = tmp;
    }

    bool operator==(const TDeque& other) const {
        if (Size_ != other.Size_) return false;
        for (size_type i = 0; i < Size_; ++i) {
            if (!((*this)[i] == other[i])) return false;
        }
        return true;
    }

    bool operator!=(const TDeque& other) const { return !(*this == other); }

    bool operator<(const TDeque& other) const {
        size_type minSize = Size_ < other.Size_ ? Size_ : other.Size_;
        for (size_type i = 0; i < minSize; ++i) {
            if ((*this)[i] < other[i]) return true;
            if (other[i] < (*this)[i]) return false;
        }
        return Size_ < other.Size_;
    }

    bool operator<=(const TDeque& other) const { return !(other < *this); }
    bool operator>(const TDeque& other) const { return other < *this; }
    bool operator>=(const TDeque& other) const { return !(*this < other); }

private:
    static T* AllocateBlock() { return static_cast<T*>(::operator new(BLOCK_SIZE * sizeof(T))); }
    static void DeallocateBlock(T* block) { ::operator delete(block); }

    void AllocateMap(size_type mapSize) {
        Map_ = static_cast<T**>(::operator new(mapSize * sizeof(T*)));
        for (size_type i = 0; i < mapSize; ++i) Map_[i] = nullptr;
        MapSize_ = mapSize;
    }

    void DeallocateMap() {
        if (Map_) {
            for (size_type i = 0; i < MapSize_; ++i) {
                if (Map_[i]) DeallocateBlock(Map_[i]);
            }
            ::operator delete(Map_);
            Map_ = nullptr;
            MapSize_ = 0;
        }
    }

    void InitializeEmpty() {
        if (!Map_) {
            AllocateMap(MIN_MAP_SIZE);
            FrontBlock_ = MapSize_ / 2;
            BackBlock_ = FrontBlock_;
            FrontOffset_ = BLOCK_SIZE / 2;
            BackOffset_ = FrontOffset_;
            Map_[FrontBlock_] = AllocateBlock();
        }
    }

    void EnsureBackCapacity() {
        InitializeEmpty();
        if (BackOffset_ == BLOCK_SIZE) {
            if (BackBlock_ + 1 >= MapSize_) GrowMap(false);
            ++BackBlock_;
            BackOffset_ = 0;
            if (!Map_[BackBlock_]) Map_[BackBlock_] = AllocateBlock();
        }
    }

    void EnsureFrontCapacity() {
        InitializeEmpty();
        if (FrontOffset_ == 0) {
            if (FrontBlock_ == 0) GrowMap(true);
            --FrontBlock_;
            FrontOffset_ = BLOCK_SIZE;
            if (!Map_[FrontBlock_]) Map_[FrontBlock_] = AllocateBlock();
        }
    }

    void GrowMap(bool growAtFront) {
        size_type newMapSize = MapSize_ * GROWTH_FACTOR;
        T** newMap = static_cast<T**>(::operator new(newMapSize * sizeof(T*)));
        for (size_type i = 0; i < newMapSize; ++i) newMap[i] = nullptr;
        size_type offset = growAtFront ? (newMapSize - MapSize_) : 0;
        for (size_type i = 0; i < MapSize_; ++i) newMap[offset + i] = Map_[i];
        if (growAtFront) {
            FrontBlock_ += offset;
            BackBlock_ += offset;
        }
        ::operator delete(Map_);
        Map_ = newMap;
        MapSize_ = newMapSize;
    }

    void AdvanceBack() { ++BackOffset_; }
    void RetreatBack() { if (BackOffset_ == 0) { --BackBlock_; BackOffset_ = BLOCK_SIZE; } --BackOffset_; }
    void AdvanceFront() { ++FrontOffset_; if (FrontOffset_ == BLOCK_SIZE) { ++FrontBlock_; FrontOffset_ = 0; } }
    void RetreatFront() { if (FrontOffset_ == 0) { --FrontBlock_; FrontOffset_ = BLOCK_SIZE; } --FrontOffset_; }
};

} // namespace NCollections

