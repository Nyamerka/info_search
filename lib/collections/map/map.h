#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <initializer_list>

namespace NCollections {

// Default comparator
template <typename T>
struct TLess {
    bool operator()(const T& a, const T& b) const {
        return a < b;
    }
};

template <typename K, typename V, typename Compare = TLess<K>>
class TMap {
private:
    enum class EColor : bool {
        Red = false,
        Black = true
    };

    struct TNode {
        K Key;
        V Value;
        TNode* Parent;
        TNode* Left;
        TNode* Right;
        EColor Color;

        TNode(const K& key, const V& value)
            : Key(key)
            , Value(value)
            , Parent(nullptr)
            , Left(nullptr)
            , Right(nullptr)
            , Color(EColor::Red)
        {}

        TNode(K&& key, V&& value)
            : Key(std::move(key))
            , Value(std::move(value))
            , Parent(nullptr)
            , Left(nullptr)
            , Right(nullptr)
            , Color(EColor::Red)
        {}
    };

public:
    class TIterator {
    public:
        using value_type = TNode;
        using pointer = TNode*;
        using reference = TNode&;

        TIterator() : Node_(nullptr), Root_(nullptr) {}
        TIterator(TNode* node, TNode* root) : Node_(node), Root_(root) {}

        K& Key() { return Node_->Key; }
        const K& Key() const { return Node_->Key; }
        V& Value() { return Node_->Value; }
        const V& Value() const { return Node_->Value; }

        TIterator& operator++() {
            if (Node_->Right) {
                Node_ = Node_->Right;
                while (Node_->Left) {
                    Node_ = Node_->Left;
                }
            } else {
                TNode* parent = Node_->Parent;
                while (parent && Node_ == parent->Right) {
                    Node_ = parent;
                    parent = parent->Parent;
                }
                Node_ = parent;
            }
            return *this;
        }

        TIterator operator++(int) {
            TIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        TIterator& operator--() {
            if (!Node_) {
                // end() case - go to maximum
                Node_ = Root_;
                if (Node_) {
                    while (Node_->Right) {
                        Node_ = Node_->Right;
                    }
                }
            } else if (Node_->Left) {
                Node_ = Node_->Left;
                while (Node_->Right) {
                    Node_ = Node_->Right;
                }
            } else {
                TNode* parent = Node_->Parent;
                while (parent && Node_ == parent->Left) {
                    Node_ = parent;
                    parent = parent->Parent;
                }
                Node_ = parent;
            }
            return *this;
        }

        TIterator operator--(int) {
            TIterator tmp = *this;
            --(*this);
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
        TNode* Root_;
    };

    class TConstIterator {
    public:
        using value_type = const TNode;
        using pointer = const TNode*;
        using reference = const TNode&;

        TConstIterator() : Node_(nullptr), Root_(nullptr) {}
        TConstIterator(const TNode* node, const TNode* root) : Node_(node), Root_(root) {}
        TConstIterator(const TIterator& it) : Node_(it.GetNode()), Root_(nullptr) {}

        const K& Key() const { return Node_->Key; }
        const V& Value() const { return Node_->Value; }

        TConstIterator& operator++() {
            if (Node_->Right) {
                Node_ = Node_->Right;
                while (Node_->Left) {
                    Node_ = Node_->Left;
                }
            } else {
                const TNode* parent = Node_->Parent;
                while (parent && Node_ == parent->Right) {
                    Node_ = parent;
                    parent = parent->Parent;
                }
                Node_ = parent;
            }
            return *this;
        }

        TConstIterator operator++(int) {
            TConstIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const TConstIterator& other) const {
            return Node_ == other.Node_;
        }

        bool operator!=(const TConstIterator& other) const {
            return Node_ != other.Node_;
        }

    private:
        const TNode* Node_;
        const TNode* Root_;
    };

    using key_type = K;
    using mapped_type = V;
    using size_type = size_t;
    using iterator = TIterator;
    using const_iterator = TConstIterator;

public:
    TMap()
        : Root_(nullptr)
        , Size_(0)
        , Compare_()
    {}

    TMap(std::initializer_list<std::pair<K, V>> init)
        : Root_(nullptr)
        , Size_(0)
        , Compare_()
    {
        for (const auto& pair : init) {
            Insert(pair.first, pair.second);
        }
    }

    TMap(const TMap& other)
        : Root_(nullptr)
        , Size_(0)
        , Compare_(other.Compare_)
    {
        CopyTree(other.Root_);
    }

    TMap(TMap&& other) noexcept
        : Root_(other.Root_)
        , Size_(other.Size_)
        , Compare_(std::move(other.Compare_))
    {
        other.Root_ = nullptr;
        other.Size_ = 0;
    }

    ~TMap() {
        Clear();
    }

    TMap& operator=(const TMap& other) {
        if (this != &other) {
            TMap tmp(other);
            Swap(tmp);
        }
        return *this;
    }

    TMap& operator=(TMap&& other) noexcept {
        if (this != &other) {
            Clear();
            Root_ = other.Root_;
            Size_ = other.Size_;
            Compare_ = std::move(other.Compare_);
            other.Root_ = nullptr;
            other.Size_ = 0;
        }
        return *this;
    }

    // Element access
    V& At(const K& key) {
        TNode* node = FindNode(key);
        if (!node) {
            throw "Key not found";
        }
        return node->Value;
    }

    const V& At(const K& key) const {
        const TNode* node = FindNode(key);
        if (!node) {
            throw "Key not found";
        }
        return node->Value;
    }

    V& operator[](const K& key) {
        TNode* node = FindNode(key);
        if (node) {
            return node->Value;
        }
        Insert(key, V());
        return FindNode(key)->Value;
    }

    // Iterators
    iterator begin() noexcept {
        if (!Root_) {
            return end();
        }
        TNode* node = Root_;
        while (node->Left) {
            node = node->Left;
        }
        return iterator(node, Root_);
    }

    const_iterator begin() const noexcept {
        if (!Root_) {
            return end();
        }
        const TNode* node = Root_;
        while (node->Left) {
            node = node->Left;
        }
        return const_iterator(node, Root_);
    }

    iterator end() noexcept {
        return iterator(nullptr, Root_);
    }

    const_iterator end() const noexcept {
        return const_iterator(nullptr, Root_);
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
        DestroyTree(Root_);
        Root_ = nullptr;
        Size_ = 0;
    }

    bool Insert(const K& key, const V& value) {
        if (!Root_) {
            Root_ = new TNode(key, value);
            Root_->Color = EColor::Black;
            ++Size_;
            return true;
        }

        TNode* parent = nullptr;
        TNode* current = Root_;

        while (current) {
            parent = current;
            if (Compare_(key, current->Key)) {
                current = current->Left;
            } else if (Compare_(current->Key, key)) {
                current = current->Right;
            } else {
                current->Value = value;
                return false;
            }
        }

        TNode* newNode = new TNode(key, value);
        newNode->Parent = parent;

        if (Compare_(key, parent->Key)) {
            parent->Left = newNode;
        } else {
            parent->Right = newNode;
        }

        InsertFixup(newNode);
        ++Size_;
        return true;
    }

    bool Erase(const K& key) {
        TNode* node = FindNode(key);
        if (!node) {
            return false;
        }

        DeleteNode(node);
        --Size_;
        return true;
    }

    void Swap(TMap& other) noexcept {
        TNode* tmpRoot = Root_;
        size_type tmpSize = Size_;
        Compare tmpCompare = Compare_;

        Root_ = other.Root_;
        Size_ = other.Size_;
        Compare_ = other.Compare_;

        other.Root_ = tmpRoot;
        other.Size_ = tmpSize;
        other.Compare_ = tmpCompare;
    }

    // Lookup
    iterator Find(const K& key) {
        TNode* node = FindNode(key);
        return iterator(node, Root_);
    }

    const_iterator Find(const K& key) const {
        const TNode* node = FindNode(key);
        return const_iterator(node, Root_);
    }

    bool Contains(const K& key) const {
        return FindNode(key) != nullptr;
    }

    size_type Count(const K& key) const {
        return Contains(key) ? 1 : 0;
    }

    iterator LowerBound(const K& key) {
        TNode* result = nullptr;
        TNode* current = Root_;
        while (current) {
            if (!Compare_(current->Key, key)) {
                result = current;
                current = current->Left;
            } else {
                current = current->Right;
            }
        }
        return iterator(result, Root_);
    }

    iterator UpperBound(const K& key) {
        TNode* result = nullptr;
        TNode* current = Root_;
        while (current) {
            if (Compare_(key, current->Key)) {
                result = current;
                current = current->Left;
            } else {
                current = current->Right;
            }
        }
        return iterator(result, Root_);
    }

private:
    TNode* FindNode(const K& key) const {
        TNode* current = Root_;
        while (current) {
            if (Compare_(key, current->Key)) {
                current = current->Left;
            } else if (Compare_(current->Key, key)) {
                current = current->Right;
            } else {
                return current;
            }
        }
        return nullptr;
    }

    void CopyTree(TNode* node) {
        if (node) {
            Insert(node->Key, node->Value);
            CopyTree(node->Left);
            CopyTree(node->Right);
        }
    }

    void DestroyTree(TNode* node) {
        if (node) {
            DestroyTree(node->Left);
            DestroyTree(node->Right);
            delete node;
        }
    }

    void RotateLeft(TNode* x) {
        TNode* y = x->Right;
        x->Right = y->Left;
        if (y->Left) {
            y->Left->Parent = x;
        }
        y->Parent = x->Parent;
        if (!x->Parent) {
            Root_ = y;
        } else if (x == x->Parent->Left) {
            x->Parent->Left = y;
        } else {
            x->Parent->Right = y;
        }
        y->Left = x;
        x->Parent = y;
    }

    void RotateRight(TNode* x) {
        TNode* y = x->Left;
        x->Left = y->Right;
        if (y->Right) {
            y->Right->Parent = x;
        }
        y->Parent = x->Parent;
        if (!x->Parent) {
            Root_ = y;
        } else if (x == x->Parent->Right) {
            x->Parent->Right = y;
        } else {
            x->Parent->Left = y;
        }
        y->Right = x;
        x->Parent = y;
    }

    void InsertFixup(TNode* z) {
        while (z->Parent && z->Parent->Color == EColor::Red) {
            if (z->Parent == z->Parent->Parent->Left) {
                TNode* y = z->Parent->Parent->Right;
                if (y && y->Color == EColor::Red) {
                    z->Parent->Color = EColor::Black;
                    y->Color = EColor::Black;
                    z->Parent->Parent->Color = EColor::Red;
                    z = z->Parent->Parent;
                } else {
                    if (z == z->Parent->Right) {
                        z = z->Parent;
                        RotateLeft(z);
                    }
                    z->Parent->Color = EColor::Black;
                    z->Parent->Parent->Color = EColor::Red;
                    RotateRight(z->Parent->Parent);
                }
            } else {
                TNode* y = z->Parent->Parent->Left;
                if (y && y->Color == EColor::Red) {
                    z->Parent->Color = EColor::Black;
                    y->Color = EColor::Black;
                    z->Parent->Parent->Color = EColor::Red;
                    z = z->Parent->Parent;
                } else {
                    if (z == z->Parent->Left) {
                        z = z->Parent;
                        RotateRight(z);
                    }
                    z->Parent->Color = EColor::Black;
                    z->Parent->Parent->Color = EColor::Red;
                    RotateLeft(z->Parent->Parent);
                }
            }
        }
        Root_->Color = EColor::Black;
    }

    void Transplant(TNode* u, TNode* v) {
        if (!u->Parent) {
            Root_ = v;
        } else if (u == u->Parent->Left) {
            u->Parent->Left = v;
        } else {
            u->Parent->Right = v;
        }
        if (v) {
            v->Parent = u->Parent;
        }
    }

    TNode* Minimum(TNode* node) {
        while (node->Left) {
            node = node->Left;
        }
        return node;
    }

    void DeleteNode(TNode* z) {
        TNode* y = z;
        TNode* x = nullptr;
        TNode* xParent = nullptr;
        EColor yOriginalColor = y->Color;

        if (!z->Left) {
            x = z->Right;
            xParent = z->Parent;
            Transplant(z, z->Right);
        } else if (!z->Right) {
            x = z->Left;
            xParent = z->Parent;
            Transplant(z, z->Left);
        } else {
            y = Minimum(z->Right);
            yOriginalColor = y->Color;
            x = y->Right;
            if (y->Parent == z) {
                xParent = y;
            } else {
                xParent = y->Parent;
                Transplant(y, y->Right);
                y->Right = z->Right;
                y->Right->Parent = y;
            }
            Transplant(z, y);
            y->Left = z->Left;
            y->Left->Parent = y;
            y->Color = z->Color;
        }

        delete z;

        if (yOriginalColor == EColor::Black) {
            DeleteFixup(x, xParent);
        }
    }

    void DeleteFixup(TNode* x, TNode* xParent) {
        while (x != Root_ && (!x || x->Color == EColor::Black)) {
            if (x == (xParent ? xParent->Left : nullptr)) {
                TNode* w = xParent ? xParent->Right : nullptr;
                if (w && w->Color == EColor::Red) {
                    w->Color = EColor::Black;
                    xParent->Color = EColor::Red;
                    RotateLeft(xParent);
                    w = xParent->Right;
                }
                if (w && (!w->Left || w->Left->Color == EColor::Black) &&
                    (!w->Right || w->Right->Color == EColor::Black)) {
                    w->Color = EColor::Red;
                    x = xParent;
                    xParent = x ? x->Parent : nullptr;
                } else if (w) {
                    if (!w->Right || w->Right->Color == EColor::Black) {
                        if (w->Left) w->Left->Color = EColor::Black;
                        w->Color = EColor::Red;
                        RotateRight(w);
                        w = xParent->Right;
                    }
                    if (w) {
                        w->Color = xParent->Color;
                        xParent->Color = EColor::Black;
                        if (w->Right) w->Right->Color = EColor::Black;
                        RotateLeft(xParent);
                    }
                    x = Root_;
                    break;
                } else {
                    break;
                }
            } else {
                TNode* w = xParent ? xParent->Left : nullptr;
                if (w && w->Color == EColor::Red) {
                    w->Color = EColor::Black;
                    xParent->Color = EColor::Red;
                    RotateRight(xParent);
                    w = xParent->Left;
                }
                if (w && (!w->Right || w->Right->Color == EColor::Black) &&
                    (!w->Left || w->Left->Color == EColor::Black)) {
                    w->Color = EColor::Red;
                    x = xParent;
                    xParent = x ? x->Parent : nullptr;
                } else if (w) {
                    if (!w->Left || w->Left->Color == EColor::Black) {
                        if (w->Right) w->Right->Color = EColor::Black;
                        w->Color = EColor::Red;
                        RotateLeft(w);
                        w = xParent->Left;
                    }
                    if (w) {
                        w->Color = xParent->Color;
                        xParent->Color = EColor::Black;
                        if (w->Left) w->Left->Color = EColor::Black;
                        RotateRight(xParent);
                    }
                    x = Root_;
                    break;
                } else {
                    break;
                }
            }
        }
        if (x) {
            x->Color = EColor::Black;
        }
    }

private:
    TNode* Root_;
    size_type Size_;
    Compare Compare_;
};

} // namespace NCollections

