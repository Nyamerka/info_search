#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <initializer_list>

namespace NCollections {

template <typename T>
class TList {
private:
    struct TNode {
        T Value;
        TNode* Prev;
        TNode* Next;

        TNode()
            : Value()
            , Prev(nullptr)
            , Next(nullptr)
        {}

        explicit TNode(const T& value)
            : Value(value)
            , Prev(nullptr)
            , Next(nullptr)
        {}

        explicit TNode(T&& value)
            : Value(std::move(value))
            , Prev(nullptr)
            , Next(nullptr)
        {}

        template <typename... Args>
        explicit TNode(Args&&... args)
            : Value(std::forward<Args>(args)...)
            , Prev(nullptr)
            , Next(nullptr)
        {}
    };

public:
    class TIterator {
    public:
        using iterator_category = void;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        TIterator() : Node_(nullptr) {}
        explicit TIterator(TNode* node) : Node_(node) {}

        reference operator*() const { return Node_->Value; }
        pointer operator->() const { return &Node_->Value; }

        TIterator& operator++() {
            Node_ = Node_->Next;
            return *this;
        }

        TIterator operator++(int) {
            TIterator tmp = *this;
            Node_ = Node_->Next;
            return tmp;
        }

        TIterator& operator--() {
            Node_ = Node_->Prev;
            return *this;
        }

        TIterator operator--(int) {
            TIterator tmp = *this;
            Node_ = Node_->Prev;
            return tmp;
        }

        bool operator==(const TIterator& other) const {
            return Node_ == other.Node_;
        }

        bool operator!=(const TIterator& other) const {
            return Node_ != other.Node_;
        }

        TNode* GetNode() const { return Node_; }

    private:
        TNode* Node_;
    };

    class TConstIterator {
    public:
        using iterator_category = void;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;

        TConstIterator() : Node_(nullptr) {}
        explicit TConstIterator(const TNode* node) : Node_(node) {}
        TConstIterator(const TIterator& it) : Node_(it.GetNode()) {}

        reference operator*() const { return Node_->Value; }
        pointer operator->() const { return &Node_->Value; }

        TConstIterator& operator++() {
            Node_ = Node_->Next;
            return *this;
        }

        TConstIterator operator++(int) {
            TConstIterator tmp = *this;
            Node_ = Node_->Next;
            return tmp;
        }

        TConstIterator& operator--() {
            Node_ = Node_->Prev;
            return *this;
        }

        TConstIterator operator--(int) {
            TConstIterator tmp = *this;
            Node_ = Node_->Prev;
            return tmp;
        }

        bool operator==(const TConstIterator& other) const {
            return Node_ == other.Node_;
        }

        bool operator!=(const TConstIterator& other) const {
            return Node_ != other.Node_;
        }

        const TNode* GetNode() const { return Node_; }

    private:
        const TNode* Node_;
    };

    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using iterator = TIterator;
    using const_iterator = TConstIterator;

public:
    TList()
        : Head_(nullptr)
        , Tail_(nullptr)
        , Size_(0)
    {}

    TList(std::initializer_list<T> init)
        : Head_(nullptr)
        , Tail_(nullptr)
        , Size_(0)
    {
        for (const auto& item : init) {
            PushBack(item);
        }
    }

    TList(const TList& other)
        : Head_(nullptr)
        , Tail_(nullptr)
        , Size_(0)
    {
        for (const auto& item : other) {
            PushBack(item);
        }
    }

    TList(TList&& other) noexcept
        : Head_(other.Head_)
        , Tail_(other.Tail_)
        , Size_(other.Size_)
    {
        other.Head_ = nullptr;
        other.Tail_ = nullptr;
        other.Size_ = 0;
    }

    ~TList() {
        Clear();
    }

    TList& operator=(const TList& other) {
        if (this != &other) {
            TList tmp(other);
            Swap(tmp);
        }
        return *this;
    }

    TList& operator=(TList&& other) noexcept {
        if (this != &other) {
            Clear();
            Head_ = other.Head_;
            Tail_ = other.Tail_;
            Size_ = other.Size_;
            other.Head_ = nullptr;
            other.Tail_ = nullptr;
            other.Size_ = 0;
        }
        return *this;
    }

    // Element access
    reference Front() {
        return Head_->Value;
    }

    const_reference Front() const {
        return Head_->Value;
    }

    reference Back() {
        return Tail_->Value;
    }

    const_reference Back() const {
        return Tail_->Value;
    }

    // Iterators
    iterator begin() noexcept {
        return iterator(Head_);
    }

    const_iterator begin() const noexcept {
        return const_iterator(Head_);
    }

    iterator end() noexcept {
        return iterator(nullptr);
    }

    const_iterator end() const noexcept {
        return const_iterator(nullptr);
    }

    // Capacity
    bool Empty() const noexcept {
        return Size_ == 0;
    }

    size_type Size() const noexcept {
        return Size_;
    }

    // Modifiers
    void Clear() noexcept {
        TNode* current = Head_;
        while (current) {
            TNode* next = current->Next;
            delete current;
            current = next;
        }
        Head_ = nullptr;
        Tail_ = nullptr;
        Size_ = 0;
    }

    void PushFront(const T& value) {
        TNode* node = new TNode(value);
        node->Next = Head_;
        if (Head_) {
            Head_->Prev = node;
        } else {
            Tail_ = node;
        }
        Head_ = node;
        ++Size_;
    }

    void PushFront(T&& value) {
        TNode* node = new TNode(std::move(value));
        node->Next = Head_;
        if (Head_) {
            Head_->Prev = node;
        } else {
            Tail_ = node;
        }
        Head_ = node;
        ++Size_;
    }

    template <typename... Args>
    reference EmplaceFront(Args&&... args) {
        TNode* node = new TNode(std::forward<Args>(args)...);
        node->Next = Head_;
        if (Head_) {
            Head_->Prev = node;
        } else {
            Tail_ = node;
        }
        Head_ = node;
        ++Size_;
        return node->Value;
    }

    void PushBack(const T& value) {
        TNode* node = new TNode(value);
        node->Prev = Tail_;
        if (Tail_) {
            Tail_->Next = node;
        } else {
            Head_ = node;
        }
        Tail_ = node;
        ++Size_;
    }

    void PushBack(T&& value) {
        TNode* node = new TNode(std::move(value));
        node->Prev = Tail_;
        if (Tail_) {
            Tail_->Next = node;
        } else {
            Head_ = node;
        }
        Tail_ = node;
        ++Size_;
    }

    template <typename... Args>
    reference EmplaceBack(Args&&... args) {
        TNode* node = new TNode(std::forward<Args>(args)...);
        node->Prev = Tail_;
        if (Tail_) {
            Tail_->Next = node;
        } else {
            Head_ = node;
        }
        Tail_ = node;
        ++Size_;
        return node->Value;
    }

    void PopFront() {
        if (Head_) {
            TNode* toDelete = Head_;
            Head_ = Head_->Next;
            if (Head_) {
                Head_->Prev = nullptr;
            } else {
                Tail_ = nullptr;
            }
            delete toDelete;
            --Size_;
        }
    }

    void PopBack() {
        if (Tail_) {
            TNode* toDelete = Tail_;
            Tail_ = Tail_->Prev;
            if (Tail_) {
                Tail_->Next = nullptr;
            } else {
                Head_ = nullptr;
            }
            delete toDelete;
            --Size_;
        }
    }

    iterator Insert(const_iterator pos, const T& value) {
        if (pos == begin()) {
            PushFront(value);
            return begin();
        }
        if (pos == end()) {
            PushBack(value);
            return iterator(Tail_);
        }

        TNode* current = const_cast<TNode*>(pos.GetNode());
        TNode* node = new TNode(value);
        node->Next = current;
        node->Prev = current->Prev;
        current->Prev->Next = node;
        current->Prev = node;
        ++Size_;
        return iterator(node);
    }

    iterator Insert(const_iterator pos, T&& value) {
        if (pos == begin()) {
            PushFront(std::move(value));
            return begin();
        }
        if (pos == end()) {
            PushBack(std::move(value));
            return iterator(Tail_);
        }

        TNode* current = const_cast<TNode*>(pos.GetNode());
        TNode* node = new TNode(std::move(value));
        node->Next = current;
        node->Prev = current->Prev;
        current->Prev->Next = node;
        current->Prev = node;
        ++Size_;
        return iterator(node);
    }

    iterator Erase(const_iterator pos) {
        TNode* node = const_cast<TNode*>(pos.GetNode());
        if (!node) {
            return end();
        }

        TNode* nextNode = node->Next;

        if (node->Prev) {
            node->Prev->Next = node->Next;
        } else {
            Head_ = node->Next;
        }

        if (node->Next) {
            node->Next->Prev = node->Prev;
        } else {
            Tail_ = node->Prev;
        }

        delete node;
        --Size_;
        return iterator(nextNode);
    }

    void Swap(TList& other) noexcept {
        TNode* tmpHead = Head_;
        TNode* tmpTail = Tail_;
        size_type tmpSize = Size_;

        Head_ = other.Head_;
        Tail_ = other.Tail_;
        Size_ = other.Size_;

        other.Head_ = tmpHead;
        other.Tail_ = tmpTail;
        other.Size_ = tmpSize;
    }

    // Operations
    void Reverse() noexcept {
        TNode* current = Head_;
        while (current) {
            TNode* tmp = current->Prev;
            current->Prev = current->Next;
            current->Next = tmp;
            current = current->Prev;
        }
        TNode* tmp = Head_;
        Head_ = Tail_;
        Tail_ = tmp;
    }

    void Remove(const T& value) {
        TNode* current = Head_;
        while (current) {
            TNode* next = current->Next;
            if (current->Value == value) {
                if (current->Prev) {
                    current->Prev->Next = current->Next;
                } else {
                    Head_ = current->Next;
                }
                if (current->Next) {
                    current->Next->Prev = current->Prev;
                } else {
                    Tail_ = current->Prev;
                }
                delete current;
                --Size_;
            }
            current = next;
        }
    }

    // Comparison
    bool operator==(const TList& other) const {
        if (Size_ != other.Size_) {
            return false;
        }
        TNode* a = Head_;
        TNode* b = other.Head_;
        while (a && b) {
            if (!(a->Value == b->Value)) {
                return false;
            }
            a = a->Next;
            b = b->Next;
        }
        return true;
    }

    bool operator!=(const TList& other) const {
        return !(*this == other);
    }

private:
    TNode* Head_;
    TNode* Tail_;
    size_type Size_;
};

} // namespace NCollections

