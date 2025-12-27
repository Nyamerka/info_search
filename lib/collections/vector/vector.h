#pragma once

#include <cstddef>
#include <cstring>
#include <new>
#include <utility>
#include <initializer_list>

namespace NCollections {

/**
 * Оптимизированный динамический массив с адаптивным growth factor
 * 
 * Особенности:
 * - Growth factor 1.5x для лучшего использования памяти
 * - Минимальная начальная capacity 8 элементов
 */
template <typename T>
class TVector {
public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = T*;
    using const_iterator = const T*;

    static constexpr size_type MIN_CAPACITY = 8;
    static constexpr size_type GROWTH_NUMERATOR = 3;
    static constexpr size_type GROWTH_DENOMINATOR = 2;

public:
    TVector() noexcept : Data_(nullptr), Size_(0), Capacity_(0) {}

    explicit TVector(size_type count) : Data_(nullptr), Size_(0), Capacity_(0) {
        if (count > 0) {
            Data_ = Allocate(count);
            Capacity_ = count;
            for (size_type i = 0; i < count; ++i) {
                new (Data_ + i) T();
            }
            Size_ = count;
        }
    }

    TVector(size_type count, const T& value) : Data_(nullptr), Size_(0), Capacity_(0) {
        if (count > 0) {
            Data_ = Allocate(count);
            Capacity_ = count;
            for (size_type i = 0; i < count; ++i) {
                new (Data_ + i) T(value);
            }
            Size_ = count;
        }
    }

    TVector(std::initializer_list<T> init) : Data_(nullptr), Size_(0), Capacity_(0) {
        if (init.size() > 0) {
            Data_ = Allocate(init.size());
            Capacity_ = init.size();
            size_type i = 0;
            for (const auto& item : init) {
                new (Data_ + i) T(item);
                ++i;
            }
            Size_ = init.size();
        }
    }

    TVector(const T* first, const T* last) : Data_(nullptr), Size_(0), Capacity_(0) {
        for (const T* it = first; it != last; ++it) {
            PushBack(*it);
        }
    }

    TVector(const TVector& other) : Data_(nullptr), Size_(0), Capacity_(0) {
        if (other.Size_ > 0) {
            Data_ = Allocate(other.Size_);
            Capacity_ = other.Size_;
            for (size_type i = 0; i < other.Size_; ++i) {
                new (Data_ + i) T(other.Data_[i]);
            }
            Size_ = other.Size_;
        }
    }

    TVector(TVector&& other) noexcept
        : Data_(other.Data_), Size_(other.Size_), Capacity_(other.Capacity_) {
        other.Data_ = nullptr;
        other.Size_ = 0;
        other.Capacity_ = 0;
    }

    ~TVector() {
        Clear();
        Deallocate(Data_);
    }

    TVector& operator=(const TVector& other) {
        if (this != &other) {
            TVector tmp(other);
            Swap(tmp);
        }
        return *this;
    }

    TVector& operator=(TVector&& other) noexcept {
        if (this != &other) {
            Clear();
            Deallocate(Data_);
            Data_ = other.Data_;
            Size_ = other.Size_;
            Capacity_ = other.Capacity_;
            other.Data_ = nullptr;
            other.Size_ = 0;
            other.Capacity_ = 0;
        }
        return *this;
    }

    TVector& operator=(std::initializer_list<T> init) {
        TVector tmp(init);
        Swap(tmp);
        return *this;
    }

    reference At(size_type pos) {
        if (pos >= Size_) throw "Index out of range";
        return Data_[pos];
    }

    const_reference At(size_type pos) const {
        if (pos >= Size_) throw "Index out of range";
        return Data_[pos];
    }

    reference operator[](size_type pos) { return Data_[pos]; }
    const_reference operator[](size_type pos) const { return Data_[pos]; }
    reference Front() { return Data_[0]; }
    const_reference Front() const { return Data_[0]; }
    reference Back() { return Data_[Size_ - 1]; }
    const_reference Back() const { return Data_[Size_ - 1]; }
    pointer Data() noexcept { return Data_; }
    const_pointer Data() const noexcept { return Data_; }

    iterator begin() noexcept { return Data_; }
    const_iterator begin() const noexcept { return Data_; }
    const_iterator cbegin() const noexcept { return Data_; }
    iterator end() noexcept { return Data_ + Size_; }
    const_iterator end() const noexcept { return Data_ + Size_; }
    const_iterator cend() const noexcept { return Data_ + Size_; }

    bool Empty() const noexcept { return Size_ == 0; }
    size_type Size() const noexcept { return Size_; }
    size_type Capacity() const noexcept { return Capacity_; }
    static constexpr size_type MaxSize() noexcept { return static_cast<size_type>(-1) / sizeof(T); }

    void Reserve(size_type newCapacity) {
        if (newCapacity > Capacity_) Grow(newCapacity);
    }

    void ShrinkToFit() {
        if (Size_ < Capacity_) {
            if (Size_ == 0) {
                Deallocate(Data_);
                Data_ = nullptr;
                Capacity_ = 0;
            } else {
                T* newData = Allocate(Size_);
                for (size_type i = 0; i < Size_; ++i) {
                    new (newData + i) T(std::move(Data_[i]));
                    Data_[i].~T();
                }
                Deallocate(Data_);
                Data_ = newData;
                Capacity_ = Size_;
            }
        }
    }

    void Clear() noexcept {
        for (size_type i = 0; i < Size_; ++i) Data_[i].~T();
        Size_ = 0;
    }

    void PushBack(const T& value) {
        if (Size_ == Capacity_) GrowForInsert();
        new (Data_ + Size_) T(value);
        ++Size_;
    }

    void PushBack(T&& value) {
        if (Size_ == Capacity_) GrowForInsert();
        new (Data_ + Size_) T(std::move(value));
        ++Size_;
    }

    template <typename... Args>
    reference EmplaceBack(Args&&... args) {
        if (Size_ == Capacity_) GrowForInsert();
        new (Data_ + Size_) T(std::forward<Args>(args)...);
        return Data_[Size_++];
    }

    void PopBack() {
        if (Size_ > 0) {
            --Size_;
            Data_[Size_].~T();
        }
    }

    void Resize(size_type count) {
        if (count > Size_) {
            if (count > Capacity_) Grow(count);
            for (size_type i = Size_; i < count; ++i) new (Data_ + i) T();
        } else if (count < Size_) {
            for (size_type i = count; i < Size_; ++i) Data_[i].~T();
        }
        Size_ = count;
    }

    void Resize(size_type count, const T& value) {
        if (count > Size_) {
            if (count > Capacity_) Grow(count);
            for (size_type i = Size_; i < count; ++i) new (Data_ + i) T(value);
        } else if (count < Size_) {
            for (size_type i = count; i < Size_; ++i) Data_[i].~T();
        }
        Size_ = count;
    }

    void Swap(TVector& other) noexcept {
        T* tmpData = Data_;
        size_type tmpSize = Size_;
        size_type tmpCapacity = Capacity_;
        Data_ = other.Data_;
        Size_ = other.Size_;
        Capacity_ = other.Capacity_;
        other.Data_ = tmpData;
        other.Size_ = tmpSize;
        other.Capacity_ = tmpCapacity;
    }

    iterator Erase(const_iterator pos) {
        size_type index = pos - Data_;
        Data_[index].~T();
        for (size_type i = index; i < Size_ - 1; ++i) {
            new (Data_ + i) T(std::move(Data_[i + 1]));
            Data_[i + 1].~T();
        }
        --Size_;
        return Data_ + index;
    }

    iterator Erase(const_iterator first, const_iterator last) {
        size_type firstIdx = first - Data_;
        size_type lastIdx = last - Data_;
        size_type count = lastIdx - firstIdx;
        if (count == 0) return Data_ + firstIdx;
        
        for (size_type i = firstIdx; i < lastIdx; ++i) Data_[i].~T();
        for (size_type i = lastIdx; i < Size_; ++i) {
            new (Data_ + i - count) T(std::move(Data_[i]));
            Data_[i].~T();
        }
        Size_ -= count;
        return Data_ + firstIdx;
    }

    iterator Insert(const_iterator pos, const T& value) {
        size_type index = pos - Data_;
        if (Size_ == Capacity_) GrowForInsert();
        for (size_type i = Size_; i > index; --i) {
            new (Data_ + i) T(std::move(Data_[i - 1]));
            Data_[i - 1].~T();
        }
        new (Data_ + index) T(value);
        ++Size_;
        return Data_ + index;
    }

    iterator Insert(const_iterator pos, T&& value) {
        size_type index = pos - Data_;
        if (Size_ == Capacity_) GrowForInsert();
        for (size_type i = Size_; i > index; --i) {
            new (Data_ + i) T(std::move(Data_[i - 1]));
            Data_[i - 1].~T();
        }
        new (Data_ + index) T(std::move(value));
        ++Size_;
        return Data_ + index;
    }

    iterator Insert(const_iterator pos, size_type count, const T& value) {
        if (count == 0) return const_cast<iterator>(pos);
        size_type index = pos - Data_;
        if (Size_ + count > Capacity_) Grow(Size_ + count);
        for (size_type i = Size_; i > index; --i) {
            new (Data_ + i + count - 1) T(std::move(Data_[i - 1]));
            Data_[i - 1].~T();
        }
        for (size_type i = 0; i < count; ++i) new (Data_ + index + i) T(value);
        Size_ += count;
        return Data_ + index;
    }

    void Assign(size_type count, const T& value) {
        Clear();
        if (count > Capacity_) {
            Deallocate(Data_);
            Data_ = Allocate(count);
            Capacity_ = count;
        }
        for (size_type i = 0; i < count; ++i) new (Data_ + i) T(value);
        Size_ = count;
    }

    void Assign(const T* first, const T* last) {
        Clear();
        for (const T* it = first; it != last; ++it) PushBack(*it);
    }

    bool operator==(const TVector& other) const {
        if (Size_ != other.Size_) return false;
        for (size_type i = 0; i < Size_; ++i) {
            if (!(Data_[i] == other.Data_[i])) return false;
        }
        return true;
    }

    bool operator!=(const TVector& other) const { return !(*this == other); }

    bool operator<(const TVector& other) const {
        size_type minSize = Size_ < other.Size_ ? Size_ : other.Size_;
        for (size_type i = 0; i < minSize; ++i) {
            if (Data_[i] < other.Data_[i]) return true;
            if (other.Data_[i] < Data_[i]) return false;
        }
        return Size_ < other.Size_;
    }

    bool operator<=(const TVector& other) const { return !(other < *this); }
    bool operator>(const TVector& other) const { return other < *this; }
    bool operator>=(const TVector& other) const { return !(*this < other); }

private:
    static T* Allocate(size_type n) { return static_cast<T*>(::operator new(n * sizeof(T))); }
    static void Deallocate(T* ptr) { ::operator delete(ptr); }

    size_type CalculateGrowth(size_type minCapacity) const {
        size_type newCapacity = Capacity_;
        if (newCapacity == 0) {
            newCapacity = MIN_CAPACITY;
        }
        while (newCapacity < minCapacity) {
            size_type prev = newCapacity;
            size_type grown = prev * GROWTH_NUMERATOR / GROWTH_DENOMINATOR;
            if (grown <= prev) {
                grown = prev + 1;
            }
            if (grown < prev) {
                newCapacity = minCapacity;
                break;
            }
            newCapacity = grown;
        }
        return newCapacity;
    }

    void Grow(size_type minCapacity) {
        size_type newCapacity = CalculateGrowth(minCapacity);
        T* newData = Allocate(newCapacity);
        for (size_type i = 0; i < Size_; ++i) {
            new (newData + i) T(std::move(Data_[i]));
            Data_[i].~T();
        }
        Deallocate(Data_);
        Data_ = newData;
        Capacity_ = newCapacity;
    }

    void GrowForInsert() { Grow(Size_ + 1); }

private:
    T* Data_;
    size_type Size_;
    size_type Capacity_;
};

} // namespace NCollections

