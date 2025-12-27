#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <initializer_list>

namespace NCollections {

template <typename T>
struct TLess {
    bool operator()(const T& a, const T& b) const {
        return a < b;
    }
};

template <typename T>
struct TGreater {
    bool operator()(const T& a, const T& b) const {
        return a > b;
    }
};

/**
 * Бинарная куча (Binary Heap)
 * 
 * По умолчанию — max-heap (максимальный элемент на вершине).
 * Для min-heap используйте THeap<T, TGreater<T>>.
 * 
 * Операции:
 * - Push: O(log n)
 * - Pop: O(log n)  
 * - Top: O(1)
 * - Heapify: O(n)
 */
template <typename T, typename Compare = TLess<T>>
class THeap {
public:
    using value_type = T;
    using size_type = size_t;
    using reference = T&;
    using const_reference = const T&;

    static constexpr size_type MIN_CAPACITY = 8;
    static constexpr size_type GROWTH_NUMERATOR = 3;
    static constexpr size_type GROWTH_DENOMINATOR = 2;

public:
    THeap()
        : Data_(nullptr)
        , Size_(0)
        , Capacity_(0)
        , Compare_()
    {}

    explicit THeap(const Compare& comp)
        : Data_(nullptr)
        , Size_(0)
        , Capacity_(0)
        , Compare_(comp)
    {}

    THeap(std::initializer_list<T> init)
        : Data_(nullptr)
        , Size_(0)
        , Capacity_(0)
        , Compare_()
    {
        if (init.size() > 0) {
            Reserve(init.size());
            for (const auto& item : init) {
                new (Data_ + Size_) T(item);
                ++Size_;
            }
            Heapify();
        }
    }

    template <typename InputIt>
    THeap(InputIt first, InputIt last)
        : Data_(nullptr)
        , Size_(0)
        , Capacity_(0)
        , Compare_()
    {
        for (InputIt it = first; it != last; ++it) {
            PushBack(*it);
        }
        Heapify();
    }

    THeap(const THeap& other)
        : Data_(nullptr)
        , Size_(0)
        , Capacity_(0)
        , Compare_(other.Compare_)
    {
        if (other.Size_ > 0) {
            Data_ = Allocate(other.Size_);
            Capacity_ = other.Size_;
            for (size_type i = 0; i < other.Size_; ++i) {
                new (Data_ + i) T(other.Data_[i]);
            }
            Size_ = other.Size_;
        }
    }

    THeap(THeap&& other) noexcept
        : Data_(other.Data_)
        , Size_(other.Size_)
        , Capacity_(other.Capacity_)
        , Compare_(std::move(other.Compare_))
    {
        other.Data_ = nullptr;
        other.Size_ = 0;
        other.Capacity_ = 0;
    }

    ~THeap() {
        Clear();
        Deallocate(Data_);
    }

    THeap& operator=(const THeap& other) {
        if (this != &other) {
            THeap tmp(other);
            Swap(tmp);
        }
        return *this;
    }

    THeap& operator=(THeap&& other) noexcept {
        if (this != &other) {
            Clear();
            Deallocate(Data_);
            Data_ = other.Data_;
            Size_ = other.Size_;
            Capacity_ = other.Capacity_;
            Compare_ = std::move(other.Compare_);
            other.Data_ = nullptr;
            other.Size_ = 0;
            other.Capacity_ = 0;
        }
        return *this;
    }

    const_reference Top() const {
        return Data_[0];
    }

    bool Empty() const noexcept {
        return Size_ == 0;
    }

    size_type Size() const noexcept {
        return Size_;
    }

    size_type Capacity() const noexcept {
        return Capacity_;
    }

    void Reserve(size_type newCapacity) {
        if (newCapacity > Capacity_) {
            Grow(newCapacity);
        }
    }

    void Push(const T& value) {
        if (Size_ == Capacity_) {
            GrowForInsert();
        }
        new (Data_ + Size_) T(value);
        ++Size_;
        SiftUp(Size_ - 1);
    }

    void Push(T&& value) {
        if (Size_ == Capacity_) {
            GrowForInsert();
        }
        new (Data_ + Size_) T(std::move(value));
        ++Size_;
        SiftUp(Size_ - 1);
    }

    template <typename... Args>
    void Emplace(Args&&... args) {
        if (Size_ == Capacity_) {
            GrowForInsert();
        }
        new (Data_ + Size_) T(std::forward<Args>(args)...);
        ++Size_;
        SiftUp(Size_ - 1);
    }

    void Pop() {
        if (Size_ > 0) {
            Data_[0].~T();
            --Size_;
            if (Size_ > 0) {
                new (Data_) T(std::move(Data_[Size_]));
                Data_[Size_].~T();
                SiftDown(0);
            }
        }
    }

    T ExtractTop() {
        T result = std::move(Data_[0]);
        Pop();
        return result;
    }

    void Clear() noexcept {
        for (size_type i = 0; i < Size_; ++i) {
            Data_[i].~T();
        }
        Size_ = 0;
    }

    void Swap(THeap& other) noexcept {
        T* tmpData = Data_;
        size_type tmpSize = Size_;
        size_type tmpCapacity = Capacity_;

        Data_ = other.Data_;
        Size_ = other.Size_;
        Capacity_ = other.Capacity_;

        other.Data_ = tmpData;
        other.Size_ = tmpSize;
        other.Capacity_ = tmpCapacity;

        Compare tmpComp = Compare_;
        Compare_ = other.Compare_;
        other.Compare_ = tmpComp;
    }

    bool IsHeap() const {
        for (size_type i = 1; i < Size_; ++i) {
            size_type parent = (i - 1) / 2;
            if (Compare_(Data_[parent], Data_[i])) {
                return false;
            }
        }
        return true;
    }

    bool operator==(const THeap& other) const {
        if (Size_ != other.Size_) {
            return false;
        }
        for (size_type i = 0; i < Size_; ++i) {
            if (!(Data_[i] == other.Data_[i])) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const THeap& other) const {
        return !(*this == other);
    }

private:
    static T* Allocate(size_type n) {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }

    static void Deallocate(T* ptr) {
        ::operator delete(ptr);
    }

    void Grow(size_type minCapacity) {
        size_type newCapacity = Capacity_;
        if (newCapacity == 0) {
            newCapacity = MIN_CAPACITY;
        }
        while (newCapacity < minCapacity) {
            newCapacity = newCapacity * GROWTH_NUMERATOR / GROWTH_DENOMINATOR;
        }

        T* newData = Allocate(newCapacity);
        for (size_type i = 0; i < Size_; ++i) {
            new (newData + i) T(std::move(Data_[i]));
            Data_[i].~T();
        }
        Deallocate(Data_);
        Data_ = newData;
        Capacity_ = newCapacity;
    }

    void GrowForInsert() {
        Grow(Size_ + 1);
    }

    void PushBack(const T& value) {
        if (Size_ == Capacity_) {
            GrowForInsert();
        }
        new (Data_ + Size_) T(value);
        ++Size_;
    }

    void SiftUp(size_type idx) {
        while (idx > 0) {
            size_type parent = (idx - 1) / 2;
            if (Compare_(Data_[parent], Data_[idx])) {
                SwapElements(parent, idx);
                idx = parent;
            } else {
                break;
            }
        }
    }

    void SiftDown(size_type idx) {
        while (true) {
            size_type largest = idx;
            size_type left = 2 * idx + 1;
            size_type right = 2 * idx + 2;

            if (left < Size_ && Compare_(Data_[largest], Data_[left])) {
                largest = left;
            }
            if (right < Size_ && Compare_(Data_[largest], Data_[right])) {
                largest = right;
            }

            if (largest != idx) {
                SwapElements(idx, largest);
                idx = largest;
            } else {
                break;
            }
        }
    }

    void SwapElements(size_type i, size_type j) {
        T tmp = std::move(Data_[i]);
        Data_[i] = std::move(Data_[j]);
        Data_[j] = std::move(tmp);
    }

    void Heapify() {
        if (Size_ <= 1) {
            return;
        }
        for (size_type i = Size_ / 2; i > 0; --i) {
            SiftDown(i - 1);
        }
    }

private:
    T* Data_;
    size_type Size_;
    size_type Capacity_;
    Compare Compare_;
};

template <typename T>
using TMaxHeap = THeap<T, TLess<T>>;

template <typename T>
using TMinHeap = THeap<T, TGreater<T>>;

} // namespace NCollections


