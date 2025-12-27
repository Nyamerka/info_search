#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <initializer_list>

namespace NCollections {

/**
 * Высокопроизводительная хеш-функция с Fibonacci hashing
 */
template <typename T>
struct THash {
    size_t operator()(const T& value) const {
        size_t h = static_cast<size_t>(value);
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccdULL;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53ULL;
        h ^= h >> 33;
        return h;
    }
};

template <typename T>
struct THash<T*> {
    size_t operator()(T* ptr) const {
        size_t h = reinterpret_cast<size_t>(ptr);
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccdULL;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53ULL;
        h ^= h >> 33;
        return h;
    }
};

template <typename T>
struct TEqual {
    bool operator()(const T& a, const T& b) const { return a == b; }
};

/**
 * Специализация для TStringHash - работает с любым типом имеющим Hash() метод
 */
struct TStringHash {
    template <typename StringType>
    size_t operator()(const StringType& str) const {
        return str.Hash();
    }
};

struct TStringEqual {
    template <typename StringType>
    bool operator()(const StringType& a, const StringType& b) const {
        return a == b;
    }
};

/**
 * Оптимизированная хеш-таблица с Robin Hood hashing
 * 
 * Open addressing для лучшей cache locality, power-of-2 bucket count
 */
template <typename K, typename V, typename Hash = THash<K>, typename Equal = TEqual<K>>
class TUnorderedMap {
private:
    enum class EState : unsigned char { Empty = 0, Occupied = 1, Deleted = 2 };

    struct TSlot {
        alignas(K) char KeyStorage[sizeof(K)];
        alignas(V) char ValueStorage[sizeof(V)];
        EState State;
        unsigned char ProbeDistance;

        TSlot() : State(EState::Empty), ProbeDistance(0) {}
        K& Key() { return *reinterpret_cast<K*>(KeyStorage); }
        const K& Key() const { return *reinterpret_cast<const K*>(KeyStorage); }
        V& Value() { return *reinterpret_cast<V*>(ValueStorage); }
        const V& Value() const { return *reinterpret_cast<const V*>(ValueStorage); }
        bool IsEmpty() const { return State == EState::Empty; }
        bool IsOccupied() const { return State == EState::Occupied; }
        bool IsDeleted() const { return State == EState::Deleted; }

        void Destroy() {
            if (State == EState::Occupied) {
                Key().~K();
                Value().~V();
            }
            State = EState::Empty;
            ProbeDistance = 0;
        }
    };

public:
    class TIterator {
    public:
        TIterator() : Slots_(nullptr), Current_(0), End_(0) {}
        TIterator(TSlot* slots, size_t current, size_t end) : Slots_(slots), Current_(current), End_(end) { AdvanceToOccupied(); }

        K& Key() { return Slots_[Current_].Key(); }
        const K& Key() const { return Slots_[Current_].Key(); }
        V& Value() { return Slots_[Current_].Value(); }
        const V& Value() const { return Slots_[Current_].Value(); }

        TIterator& operator++() { ++Current_; AdvanceToOccupied(); return *this; }
        TIterator operator++(int) { TIterator tmp = *this; ++(*this); return tmp; }
        bool operator==(const TIterator& other) const { return Current_ == other.Current_; }
        bool operator!=(const TIterator& other) const { return Current_ != other.Current_; }

    private:
        void AdvanceToOccupied() { while (Current_ < End_ && !Slots_[Current_].IsOccupied()) ++Current_; }
        TSlot* Slots_;
        size_t Current_;
        size_t End_;
    };

    class TConstIterator {
    public:
        TConstIterator() : Slots_(nullptr), Current_(0), End_(0) {}
        TConstIterator(const TSlot* slots, size_t current, size_t end) : Slots_(slots), Current_(current), End_(end) { AdvanceToOccupied(); }

        const K& Key() const { return Slots_[Current_].Key(); }
        const V& Value() const { return Slots_[Current_].Value(); }

        TConstIterator& operator++() { ++Current_; AdvanceToOccupied(); return *this; }
        TConstIterator operator++(int) { TConstIterator tmp = *this; ++(*this); return tmp; }
        bool operator==(const TConstIterator& other) const { return Current_ == other.Current_; }
        bool operator!=(const TConstIterator& other) const { return Current_ != other.Current_; }

    private:
        void AdvanceToOccupied() { while (Current_ < End_ && !Slots_[Current_].IsOccupied()) ++Current_; }
        const TSlot* Slots_;
        size_t Current_;
        size_t End_;
    };

    using key_type = K;
    using mapped_type = V;
    using size_type = size_t;
    using iterator = TIterator;
    using const_iterator = TConstIterator;

    static constexpr size_type DEFAULT_CAPACITY = 16;
    static constexpr float MAX_LOAD_FACTOR = 0.75f;
    static constexpr size_type MAX_PROBE_DISTANCE = 128;

public:
    TUnorderedMap() : Slots_(nullptr), Capacity_(0), Size_(0), Mask_(0), Hash_(), Equal_() { InitSlots(DEFAULT_CAPACITY); }
    explicit TUnorderedMap(size_type initialCapacity) : Slots_(nullptr), Capacity_(0), Size_(0), Mask_(0), Hash_(), Equal_() {
        size_type capacity = RoundUpToPowerOf2(initialCapacity > 0 ? initialCapacity : DEFAULT_CAPACITY);
        InitSlots(capacity);
    }

    TUnorderedMap(std::initializer_list<std::pair<K, V>> init) : Slots_(nullptr), Capacity_(0), Size_(0), Mask_(0), Hash_(), Equal_() {
        size_type capacity = RoundUpToPowerOf2(init.size() > 0 ? init.size() * 2 : DEFAULT_CAPACITY);
        InitSlots(capacity);
        for (const auto& pair : init) Insert(pair.first, pair.second);
    }

    TUnorderedMap(const TUnorderedMap& other) : Slots_(nullptr), Capacity_(0), Size_(0), Mask_(0), Hash_(other.Hash_), Equal_(other.Equal_) {
        InitSlots(other.Capacity_);
        for (size_type i = 0; i < other.Capacity_; ++i) {
            if (other.Slots_[i].IsOccupied()) InsertInternal(other.Slots_[i].Key(), other.Slots_[i].Value());
        }
    }

    TUnorderedMap(TUnorderedMap&& other) noexcept
        : Slots_(other.Slots_), Capacity_(other.Capacity_), Size_(other.Size_), Mask_(other.Mask_), Hash_(std::move(other.Hash_)), Equal_(std::move(other.Equal_)) {
        other.Slots_ = nullptr;
        other.Capacity_ = 0;
        other.Size_ = 0;
        other.Mask_ = 0;
    }

    ~TUnorderedMap() { Clear(); DeallocateSlots(Slots_, Capacity_); }

    TUnorderedMap& operator=(const TUnorderedMap& other) {
        if (this != &other) { TUnorderedMap tmp(other); Swap(tmp); }
        return *this;
    }

    TUnorderedMap& operator=(TUnorderedMap&& other) noexcept {
        if (this != &other) {
            Clear();
            DeallocateSlots(Slots_, Capacity_);
            Slots_ = other.Slots_;
            Capacity_ = other.Capacity_;
            Size_ = other.Size_;
            Mask_ = other.Mask_;
            Hash_ = std::move(other.Hash_);
            Equal_ = std::move(other.Equal_);
            other.Slots_ = nullptr;
            other.Capacity_ = 0;
            other.Size_ = 0;
            other.Mask_ = 0;
        }
        return *this;
    }

    V& At(const K& key) {
        size_type idx = FindIndex(key);
        if (idx == Capacity_) throw "Key not found";
        return Slots_[idx].Value();
    }

    const V& At(const K& key) const {
        size_type idx = FindIndex(key);
        if (idx == Capacity_) throw "Key not found";
        return Slots_[idx].Value();
    }

    V& operator[](const K& key) {
        size_type idx = FindIndex(key);
        if (idx != Capacity_) return Slots_[idx].Value();
        Insert(key, V());
        return At(key);
    }

    V& operator[](K&& key) {
        size_type idx = FindIndex(key);
        if (idx != Capacity_) return Slots_[idx].Value();
        K keyCopy = std::move(key);
        Insert(keyCopy, V());
        return At(keyCopy);
    }

    iterator begin() noexcept { return iterator(Slots_, 0, Capacity_); }
    const_iterator begin() const noexcept { return const_iterator(Slots_, 0, Capacity_); }
    iterator end() noexcept { return iterator(Slots_, Capacity_, Capacity_); }
    const_iterator end() const noexcept { return const_iterator(Slots_, Capacity_, Capacity_); }

    bool Empty() const noexcept { return Size_ == 0; }
    size_type Size() const noexcept { return Size_; }
    size_type BucketCount() const noexcept { return Capacity_; }
    float LoadFactor() const noexcept { return Capacity_ > 0 ? static_cast<float>(Size_) / Capacity_ : 0.0f; }

    void Clear() noexcept {
        for (size_type i = 0; i < Capacity_; ++i) Slots_[i].Destroy();
        Size_ = 0;
    }

    bool Insert(const K& key, const V& value) {
        if (ShouldRehash()) Rehash(Capacity_ * 2);
        return InsertInternal(key, value);
    }

    bool Insert(K&& key, V&& value) {
        if (ShouldRehash()) Rehash(Capacity_ * 2);
        return InsertInternalMove(std::move(key), std::move(value));
    }

    template <typename... Args>
    bool Emplace(const K& key, Args&&... args) {
        if (ShouldRehash()) Rehash(Capacity_ * 2);
        return EmplaceInternal(key, std::forward<Args>(args)...);
    }

    bool Erase(const K& key) {
        size_type idx = FindIndex(key);
        if (idx == Capacity_) return false;
        
        Slots_[idx].Destroy();
        Slots_[idx].State = EState::Deleted;
        --Size_;
        
        size_type next = (idx + 1) & Mask_;
        while (Slots_[next].IsOccupied() && Slots_[next].ProbeDistance > 0) {
            new (&Slots_[idx].Key()) K(std::move(Slots_[next].Key()));
            new (&Slots_[idx].Value()) V(std::move(Slots_[next].Value()));
            Slots_[idx].State = EState::Occupied;
            Slots_[idx].ProbeDistance = Slots_[next].ProbeDistance - 1;
            Slots_[next].Destroy();
            Slots_[next].State = EState::Deleted;
            idx = next;
            next = (next + 1) & Mask_;
        }
        Slots_[idx].State = EState::Empty;
        return true;
    }

    void Swap(TUnorderedMap& other) noexcept {
        TSlot* tmpSlots = Slots_; Slots_ = other.Slots_; other.Slots_ = tmpSlots;
        size_type tmp = Capacity_; Capacity_ = other.Capacity_; other.Capacity_ = tmp;
        tmp = Size_; Size_ = other.Size_; other.Size_ = tmp;
        tmp = Mask_; Mask_ = other.Mask_; other.Mask_ = tmp;
        Hash tmpHash = Hash_; Hash_ = other.Hash_; other.Hash_ = tmpHash;
        Equal tmpEqual = Equal_; Equal_ = other.Equal_; other.Equal_ = tmpEqual;
    }

    void Rehash(size_type newCapacity) {
        newCapacity = RoundUpToPowerOf2(newCapacity);
        if (newCapacity <= Capacity_ && LoadFactor() < MAX_LOAD_FACTOR) return;

        TSlot* oldSlots = Slots_;
        size_type oldCapacity = Capacity_;

        Slots_ = AllocateSlots(newCapacity);
        Capacity_ = newCapacity;
        Mask_ = newCapacity - 1;
        Size_ = 0;

        for (size_type i = 0; i < oldCapacity; ++i) {
            if (oldSlots[i].IsOccupied()) {
                InsertInternalMove(std::move(oldSlots[i].Key()), std::move(oldSlots[i].Value()));
                oldSlots[i].Destroy();
            }
        }
        DeallocateSlots(oldSlots, oldCapacity);
    }

    iterator Find(const K& key) {
        size_type idx = FindIndex(key);
        return idx == Capacity_ ? end() : iterator(Slots_, idx, Capacity_);
    }

    const_iterator Find(const K& key) const {
        size_type idx = FindIndex(key);
        return idx == Capacity_ ? end() : const_iterator(Slots_, idx, Capacity_);
    }

    bool Contains(const K& key) const { return FindIndex(key) != Capacity_; }
    size_type Count(const K& key) const { return Contains(key) ? 1 : 0; }

private:
    static size_type RoundUpToPowerOf2(size_type n) {
        if (n == 0) return 1;
        --n;
        n |= n >> 1; n |= n >> 2; n |= n >> 4; n |= n >> 8; n |= n >> 16; n |= n >> 32;
        return n + 1;
    }

    static TSlot* AllocateSlots(size_type n) {
        TSlot* slots = static_cast<TSlot*>(::operator new(n * sizeof(TSlot)));
        for (size_type i = 0; i < n; ++i) new (slots + i) TSlot();
        return slots;
    }

    static void DeallocateSlots(TSlot* slots, size_type n) {
        if (slots) {
            for (size_type i = 0; i < n; ++i) slots[i].~TSlot();
            ::operator delete(slots);
        }
    }

    void InitSlots(size_type capacity) {
        Slots_ = AllocateSlots(capacity);
        Capacity_ = capacity;
        Mask_ = capacity - 1;
    }

    bool ShouldRehash() const { return static_cast<float>(Size_ + 1) / Capacity_ > MAX_LOAD_FACTOR; }

    size_type FindIndex(const K& key) const {
        if (Capacity_ == 0) return Capacity_;
        size_type hash = Hash_(key);
        size_type idx = hash & Mask_;
        unsigned char probeDistance = 0;
        
        while (probeDistance <= MAX_PROBE_DISTANCE) {
            if (Slots_[idx].IsEmpty()) return Capacity_;
            if (Slots_[idx].IsOccupied() && Equal_(Slots_[idx].Key(), key)) return idx;
            if (Slots_[idx].IsOccupied() && probeDistance > Slots_[idx].ProbeDistance) return Capacity_;
            ++probeDistance;
            idx = (idx + 1) & Mask_;
        }
        return Capacity_;
    }

    bool InsertInternal(const K& key, const V& value) {
        size_type hash = Hash_(key);
        size_type idx = hash & Mask_;
        unsigned char probeDistance = 0;
        K currentKey = key;
        V currentValue = value;
        
        while (probeDistance <= MAX_PROBE_DISTANCE) {
            if (Slots_[idx].IsEmpty() || Slots_[idx].IsDeleted()) {
                new (&Slots_[idx].Key()) K(std::move(currentKey));
                new (&Slots_[idx].Value()) V(std::move(currentValue));
                Slots_[idx].State = EState::Occupied;
                Slots_[idx].ProbeDistance = probeDistance;
                ++Size_;
                return true;
            }
            if (Slots_[idx].IsOccupied() && Equal_(Slots_[idx].Key(), currentKey)) {
                Slots_[idx].Value() = std::move(currentValue);
                return false;
            }
            if (Slots_[idx].IsOccupied() && probeDistance > Slots_[idx].ProbeDistance) {
                K tmpKey = std::move(Slots_[idx].Key());
                V tmpValue = std::move(Slots_[idx].Value());
                unsigned char tmpProbe = Slots_[idx].ProbeDistance;
                Slots_[idx].Key() = std::move(currentKey);
                Slots_[idx].Value() = std::move(currentValue);
                Slots_[idx].ProbeDistance = probeDistance;
                currentKey = std::move(tmpKey);
                currentValue = std::move(tmpValue);
                probeDistance = tmpProbe;
            }
            ++probeDistance;
            idx = (idx + 1) & Mask_;
        }
        Rehash(Capacity_ * 2);
        return InsertInternal(currentKey, currentValue);
    }

    bool InsertInternalMove(K&& key, V&& value) {
        size_type hash = Hash_(key);
        size_type idx = hash & Mask_;
        unsigned char probeDistance = 0;
        K currentKey = std::move(key);
        V currentValue = std::move(value);
        
        while (probeDistance <= MAX_PROBE_DISTANCE) {
            if (Slots_[idx].IsEmpty() || Slots_[idx].IsDeleted()) {
                new (&Slots_[idx].Key()) K(std::move(currentKey));
                new (&Slots_[idx].Value()) V(std::move(currentValue));
                Slots_[idx].State = EState::Occupied;
                Slots_[idx].ProbeDistance = probeDistance;
                ++Size_;
                return true;
            }
            if (Slots_[idx].IsOccupied() && Equal_(Slots_[idx].Key(), currentKey)) {
                Slots_[idx].Value() = std::move(currentValue);
                return false;
            }
            if (Slots_[idx].IsOccupied() && probeDistance > Slots_[idx].ProbeDistance) {
                K tmpKey = std::move(Slots_[idx].Key());
                V tmpValue = std::move(Slots_[idx].Value());
                unsigned char tmpProbe = Slots_[idx].ProbeDistance;
                Slots_[idx].Key() = std::move(currentKey);
                Slots_[idx].Value() = std::move(currentValue);
                Slots_[idx].ProbeDistance = probeDistance;
                currentKey = std::move(tmpKey);
                currentValue = std::move(tmpValue);
                probeDistance = tmpProbe;
            }
            ++probeDistance;
            idx = (idx + 1) & Mask_;
        }
        Rehash(Capacity_ * 2);
        return InsertInternalMove(std::move(currentKey), std::move(currentValue));
    }

    template <typename... Args>
    bool EmplaceInternal(const K& key, Args&&... args) {
        size_type idx = FindIndex(key);
        if (idx != Capacity_) return false;
        V value(std::forward<Args>(args)...);
        return InsertInternalMove(K(key), std::move(value));
    }

private:
    TSlot* Slots_;
    size_type Capacity_;
    size_type Size_;
    size_type Mask_;
    Hash Hash_;
    Equal Equal_;
};

} // namespace NCollections

