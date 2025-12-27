#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <initializer_list>

#include <lib/collections/map/map.h>

namespace NCollections {

template <typename T, typename Compare = TLess<T>>
class TSet {
private:
    struct TDummy {};
    using TStorage = TMap<T, TDummy, Compare>;

public:
    class TIterator {
    public:
        using value_type = T;
        using pointer = const T*;
        using reference = const T&;

        TIterator() = default;
        explicit TIterator(typename TStorage::iterator it) : It_(it) {}

        reference operator*() const { return It_.Key(); }
        pointer operator->() const { return &It_.Key(); }

        TIterator& operator++() {
            ++It_;
            return *this;
        }

        TIterator operator++(int) {
            TIterator tmp = *this;
            ++It_;
            return tmp;
        }

        TIterator& operator--() {
            --It_;
            return *this;
        }

        TIterator operator--(int) {
            TIterator tmp = *this;
            --It_;
            return tmp;
        }

        bool operator==(const TIterator& other) const {
            return It_ == other.It_;
        }

        bool operator!=(const TIterator& other) const {
            return It_ != other.It_;
        }

        typename TStorage::iterator GetIt() const { return It_; }

    private:
        typename TStorage::iterator It_;
    };

    class TConstIterator {
    public:
        using value_type = T;
        using pointer = const T*;
        using reference = const T&;

        TConstIterator() = default;
        explicit TConstIterator(typename TStorage::const_iterator it) : It_(it) {}
        TConstIterator(const TIterator& it) : It_(it.GetIt()) {}

        reference operator*() const { return It_.Key(); }
        pointer operator->() const { return &It_.Key(); }

        TConstIterator& operator++() {
            ++It_;
            return *this;
        }

        TConstIterator operator++(int) {
            TConstIterator tmp = *this;
            ++It_;
            return tmp;
        }

        bool operator==(const TConstIterator& other) const {
            return It_ == other.It_;
        }

        bool operator!=(const TConstIterator& other) const {
            return It_ != other.It_;
        }

    private:
        typename TStorage::const_iterator It_;
    };

    using key_type = T;
    using value_type = T;
    using size_type = size_t;
    using iterator = TIterator;
    using const_iterator = TConstIterator;

public:
    TSet() = default;

    TSet(std::initializer_list<T> init) {
        for (const auto& item : init) {
            Insert(item);
        }
    }

    TSet(const TSet& other) = default;
    TSet(TSet&& other) noexcept = default;

    TSet& operator=(const TSet& other) = default;
    TSet& operator=(TSet&& other) noexcept = default;

    // Iterators
    iterator begin() noexcept {
        return iterator(Storage_.begin());
    }

    const_iterator begin() const noexcept {
        return const_iterator(Storage_.begin());
    }

    iterator end() noexcept {
        return iterator(Storage_.end());
    }

    const_iterator end() const noexcept {
        return const_iterator(Storage_.end());
    }

    // Capacity
    bool Empty() const noexcept {
        return Storage_.Empty();
    }

    size_type Size() const noexcept {
        return Storage_.Size();
    }

    // Modifiers
    void Clear() noexcept {
        Storage_.Clear();
    }

    bool Insert(const T& value) {
        return Storage_.Insert(value, TDummy{});
    }

    bool Insert(T&& value) {
        return Storage_.Insert(std::move(value), TDummy{});
    }

    template <typename... Args>
    bool Emplace(Args&&... args) {
        T value(std::forward<Args>(args)...);
        return Storage_.Insert(std::move(value), TDummy{});
    }

    bool Erase(const T& value) {
        return Storage_.Erase(value);
    }

    void Swap(TSet& other) noexcept {
        Storage_.Swap(other.Storage_);
    }

    // Lookup
    iterator Find(const T& value) {
        return iterator(Storage_.Find(value));
    }

    const_iterator Find(const T& value) const {
        return const_iterator(Storage_.Find(value));
    }

    bool Contains(const T& value) const {
        return Storage_.Contains(value);
    }

    size_type Count(const T& value) const {
        return Storage_.Count(value);
    }

    iterator LowerBound(const T& value) {
        return iterator(Storage_.LowerBound(value));
    }

    iterator UpperBound(const T& value) {
        return iterator(Storage_.UpperBound(value));
    }

    // Set operations
    TSet Union(const TSet& other) const {
        TSet result(*this);
        for (const auto& item : other) {
            result.Insert(item);
        }
        return result;
    }

    TSet Intersection(const TSet& other) const {
        TSet result;
        for (const auto& item : *this) {
            if (other.Contains(item)) {
                result.Insert(item);
            }
        }
        return result;
    }

    TSet Difference(const TSet& other) const {
        TSet result;
        for (const auto& item : *this) {
            if (!other.Contains(item)) {
                result.Insert(item);
            }
        }
        return result;
    }

    // Comparison
    bool operator==(const TSet& other) const {
        if (Size() != other.Size()) {
            return false;
        }
        for (const auto& item : *this) {
            if (!other.Contains(item)) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const TSet& other) const {
        return !(*this == other);
    }

private:
    TStorage Storage_;
};

} // namespace NCollections

