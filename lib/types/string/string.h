#pragma once

#include <cstddef>
#include <cstring>
#include <new>
#include <utility>

namespace NTypes {

/**
 * Оптимизированная строка с Small String Optimization (SSO)
 * 
 * SSO позволяет хранить короткие строки (до 22 байт на 64-bit) прямо в объекте,
 * избегая дорогих аллокаций памяти.
 */
class TString {
public:
    using size_type = size_t;
    using value_type = char;
    using reference = char&;
    using const_reference = const char&;
    using pointer = char*;
    using const_pointer = const char*;
    using iterator = char*;
    using const_iterator = const char*;

    static constexpr size_type npos = static_cast<size_type>(-1);
    static constexpr size_type SSO_CAPACITY = 22;

private:
    struct TLong {
        char* Data;
        size_type Size;
        size_type Capacity;
    };
    
    struct TShort {
        char Data[SSO_CAPACITY + 1];
        unsigned char Size;
    };
    
    union {
        TLong Long_;
        TShort Short_;
    };
    
    bool IsLong_;

    bool IsLong() const noexcept {
        return IsLong_;
    }

    void SetShort(size_type size) noexcept {
        IsLong_ = false;
        Short_.Size = static_cast<unsigned char>(size);
        Short_.Data[size] = '\0';
    }

    void SetLong() noexcept {
        IsLong_ = true;
    }

    size_type GetShortSize() const noexcept {
        return Short_.Size;
    }

public:
    TString() noexcept : IsLong_(false) {
        Short_.Data[0] = '\0';
        Short_.Size = 0;
    }

    TString(const char* str) : IsLong_(false) {
        if (str) {
            size_type len = StrLen(str);
            if (len <= SSO_CAPACITY) {
                MemCopy(Short_.Data, str, len);
                SetShort(len);
            } else {
                Long_.Capacity = len + 1;
                Long_.Data = Allocate(Long_.Capacity);
                MemCopy(Long_.Data, str, len);
                Long_.Data[len] = '\0';
                Long_.Size = len;
                SetLong();
            }
        } else {
            Short_.Data[0] = '\0';
            Short_.Size = 0;
        }
    }

    TString(const char* str, size_type count) : IsLong_(false) {
        if (str && count > 0) {
            if (count <= SSO_CAPACITY) {
                MemCopy(Short_.Data, str, count);
                SetShort(count);
            } else {
                Long_.Capacity = count + 1;
                Long_.Data = Allocate(Long_.Capacity);
                MemCopy(Long_.Data, str, count);
                Long_.Data[count] = '\0';
                Long_.Size = count;
                SetLong();
            }
        } else {
            Short_.Data[0] = '\0';
            Short_.Size = 0;
        }
    }

    TString(size_type count, char ch) : IsLong_(false) {
        if (count > 0) {
            if (count <= SSO_CAPACITY) {
                for (size_type i = 0; i < count; ++i) {
                    Short_.Data[i] = ch;
                }
                SetShort(count);
            } else {
                Long_.Capacity = count + 1;
                Long_.Data = Allocate(Long_.Capacity);
                for (size_type i = 0; i < count; ++i) {
                    Long_.Data[i] = ch;
                }
                Long_.Data[count] = '\0';
                Long_.Size = count;
                SetLong();
            }
        } else {
            Short_.Data[0] = '\0';
            Short_.Size = 0;
        }
    }

    TString(const TString& other) : IsLong_(false) {
        if (other.IsLong()) {
            Long_.Capacity = other.Long_.Size + 1;
            Long_.Data = Allocate(Long_.Capacity);
            MemCopy(Long_.Data, other.Long_.Data, other.Long_.Size);
            Long_.Data[other.Long_.Size] = '\0';
            Long_.Size = other.Long_.Size;
            SetLong();
        } else {
            size_type size = other.GetShortSize();
            MemCopy(Short_.Data, other.Short_.Data, size);
            SetShort(size);
        }
    }

    TString(TString&& other) noexcept : IsLong_(false) {
        if (other.IsLong()) {
            Long_.Data = other.Long_.Data;
            Long_.Size = other.Long_.Size;
            Long_.Capacity = other.Long_.Capacity;
            SetLong();
            other.Short_.Data[0] = '\0';
            other.SetShort(0);
        } else {
            size_type size = other.GetShortSize();
            MemCopy(Short_.Data, other.Short_.Data, size);
            SetShort(size);
            other.Short_.Data[0] = '\0';
            other.SetShort(0);
        }
    }

    ~TString() {
        if (IsLong()) {
            Deallocate(Long_.Data);
        }
    }

    TString& operator=(const TString& other) {
        if (this != &other) {
            TString tmp(other);
            Swap(tmp);
        }
        return *this;
    }

    TString& operator=(TString&& other) noexcept {
        if (this != &other) {
            if (IsLong()) {
                Deallocate(Long_.Data);
            }
            if (other.IsLong()) {
                Long_.Data = other.Long_.Data;
                Long_.Size = other.Long_.Size;
                Long_.Capacity = other.Long_.Capacity;
                SetLong();
                other.Short_.Data[0] = '\0';
                other.SetShort(0);
            } else {
                size_type size = other.GetShortSize();
                MemCopy(Short_.Data, other.Short_.Data, size);
                SetShort(size);
                other.Short_.Data[0] = '\0';
                other.SetShort(0);
            }
        }
        return *this;
    }

    TString& operator=(const char* str) {
        TString tmp(str);
        Swap(tmp);
        return *this;
    }

    reference At(size_type pos) {
        if (pos >= Size()) {
            throw "Index out of range";
        }
        return Data()[pos];
    }

    const_reference At(size_type pos) const {
        if (pos >= Size()) {
            throw "Index out of range";
        }
        return Data()[pos];
    }

    reference operator[](size_type pos) { return Data()[pos]; }
    const_reference operator[](size_type pos) const { return Data()[pos]; }
    reference Front() { return Data()[0]; }
    const_reference Front() const { return Data()[0]; }
    reference Back() { return Data()[Size() - 1]; }
    const_reference Back() const { return Data()[Size() - 1]; }
    const char* CStr() const noexcept { return Data(); }
    pointer Data() noexcept { return IsLong() ? Long_.Data : Short_.Data; }
    const_pointer Data() const noexcept { return IsLong() ? Long_.Data : Short_.Data; }

    iterator begin() noexcept { return Data(); }
    const_iterator begin() const noexcept { return Data(); }
    iterator end() noexcept { return Data() + Size(); }
    const_iterator end() const noexcept { return Data() + Size(); }

    bool Empty() const noexcept { return Size() == 0; }
    size_type Size() const noexcept { return IsLong() ? Long_.Size : GetShortSize(); }
    size_type Length() const noexcept { return Size(); }
    size_type Capacity() const noexcept { return IsLong() ? Long_.Capacity - 1 : SSO_CAPACITY; }
    static constexpr size_type MaxSize() noexcept { return static_cast<size_type>(-1) / 2; }

    void Reserve(size_type newCapacity) {
        if (newCapacity <= Capacity()) {
            return;
        }
        
        size_type currentSize = Size();
        const char* currentData = Data();
        
        size_type actualCapacity = newCapacity;
        if (actualCapacity < Capacity() * 3 / 2) {
            actualCapacity = Capacity() * 3 / 2;
        }
        
        char* newData = Allocate(actualCapacity + 1);
        MemCopy(newData, currentData, currentSize);
        newData[currentSize] = '\0';
        
        if (IsLong()) {
            Deallocate(Long_.Data);
        }
        
        Long_.Data = newData;
        Long_.Size = currentSize;
        Long_.Capacity = actualCapacity + 1;
        SetLong();
    }

    void ShrinkToFit() {
        if (!IsLong()) {
            return;
        }
        
        if (Long_.Size <= SSO_CAPACITY) {
            char* oldData = Long_.Data;
            size_type oldSize = Long_.Size;
            MemCopy(Short_.Data, oldData, oldSize);
            SetShort(oldSize);
            Deallocate(oldData);
        } else if (Long_.Capacity > Long_.Size + 1) {
            char* newData = Allocate(Long_.Size + 1);
            MemCopy(newData, Long_.Data, Long_.Size);
            newData[Long_.Size] = '\0';
            Deallocate(Long_.Data);
            Long_.Data = newData;
            Long_.Capacity = Long_.Size + 1;
        }
    }

    void Clear() noexcept {
        if (IsLong()) {
            Long_.Data[0] = '\0';
            Long_.Size = 0;
        } else {
            Short_.Data[0] = '\0';
            SetShort(0);
        }
    }

    void PushBack(char ch) {
        size_type currentSize = Size();
        if (currentSize + 1 > Capacity()) {
            Reserve(currentSize + 1);
        }
        
        char* data = Data();
        data[currentSize] = ch;
        data[currentSize + 1] = '\0';
        
        if (IsLong()) {
            ++Long_.Size;
        } else {
            SetShort(currentSize + 1);
        }
    }

    void PopBack() {
        size_type currentSize = Size();
        if (currentSize > 0) {
            if (IsLong()) {
                --Long_.Size;
                Long_.Data[Long_.Size] = '\0';
            } else {
                SetShort(currentSize - 1);
            }
        }
    }

    TString& Append(const TString& str) { return Append(str.Data(), str.Size()); }
    TString& Append(const char* str) { return str ? Append(str, StrLen(str)) : *this; }

    TString& Append(const char* str, size_type count) {
        if (str && count > 0) {
            size_type currentSize = Size();
            size_type newSize = currentSize + count;
            
            if (newSize > Capacity()) {
                Reserve(newSize);
            }
            
            char* data = Data();
            MemCopy(data + currentSize, str, count);
            data[newSize] = '\0';
            
            if (IsLong()) {
                Long_.Size = newSize;
            } else {
                SetShort(newSize);
            }
        }
        return *this;
    }

    TString& Append(size_type count, char ch) {
        if (count > 0) {
            size_type currentSize = Size();
            size_type newSize = currentSize + count;
            
            if (newSize > Capacity()) {
                Reserve(newSize);
            }
            
            char* data = Data();
            for (size_type i = 0; i < count; ++i) {
                data[currentSize + i] = ch;
            }
            data[newSize] = '\0';
            
            if (IsLong()) {
                Long_.Size = newSize;
            } else {
                SetShort(newSize);
            }
        }
        return *this;
    }

    TString& operator+=(const TString& str) { return Append(str); }
    TString& operator+=(const char* str) { return Append(str); }
    TString& operator+=(char ch) { PushBack(ch); return *this; }

    void Swap(TString& other) noexcept {
        char tmp[sizeof(TString)];
        MemCopy(tmp, reinterpret_cast<char*>(this), sizeof(TString));
        MemCopy(reinterpret_cast<char*>(this), reinterpret_cast<char*>(&other), sizeof(TString));
        MemCopy(reinterpret_cast<char*>(&other), tmp, sizeof(TString));
    }

    size_type Find(const TString& str, size_type pos = 0) const { return Find(str.Data(), pos, str.Size()); }
    size_type Find(const char* str, size_type pos = 0) const { return Find(str, pos, StrLen(str)); }

    size_type Find(const char* str, size_type pos, size_type count) const {
        size_type size = Size();
        if (pos > size || !str) return npos;
        if (count == 0) return pos;
        if (count > size - pos) return npos;
        
        const char* data = Data();
        for (size_type i = pos; i <= size - count; ++i) {
            bool found = true;
            for (size_type j = 0; j < count; ++j) {
                if (data[i + j] != str[j]) {
                    found = false;
                    break;
                }
            }
            if (found) return i;
        }
        return npos;
    }

    size_type Find(char ch, size_type pos = 0) const {
        const char* data = Data();
        size_type size = Size();
        for (size_type i = pos; i < size; ++i) {
            if (data[i] == ch) return i;
        }
        return npos;
    }

    size_type RFind(char ch, size_type pos = npos) const {
        size_type size = Size();
        if (size == 0) return npos;
        const char* data = Data();
        size_type start = (pos >= size) ? size - 1 : pos;
        for (size_type i = start + 1; i > 0; --i) {
            if (data[i - 1] == ch) return i - 1;
        }
        return npos;
    }

    TString SubStr(size_type pos = 0, size_type count = npos) const {
        size_type size = Size();
        if (pos >= size) return TString();
        size_type len = (count == npos || pos + count > size) ? size - pos : count;
        return TString(Data() + pos, len);
    }

    int Compare(const TString& other) const {
        const char* data1 = Data();
        const char* data2 = other.Data();
        size_type size1 = Size();
        size_type size2 = other.Size();
        size_type minLen = size1 < size2 ? size1 : size2;
        
        for (size_type i = 0; i < minLen; ++i) {
            if (data1[i] < data2[i]) return -1;
            if (data1[i] > data2[i]) return 1;
        }
        if (size1 < size2) return -1;
        if (size1 > size2) return 1;
        return 0;
    }

    bool StartsWith(const TString& prefix) const {
        if (prefix.Size() > Size()) return false;
        const char* data = Data();
        const char* prefixData = prefix.Data();
        for (size_type i = 0; i < prefix.Size(); ++i) {
            if (data[i] != prefixData[i]) return false;
        }
        return true;
    }

    bool StartsWith(const char* prefix) const {
        if (!prefix) return true;
        size_type prefixLen = StrLen(prefix);
        if (prefixLen > Size()) return false;
        const char* data = Data();
        for (size_type i = 0; i < prefixLen; ++i) {
            if (data[i] != prefix[i]) return false;
        }
        return true;
    }

    bool EndsWith(const TString& suffix) const {
        if (suffix.Size() > Size()) return false;
        const char* data = Data();
        const char* suffixData = suffix.Data();
        size_type offset = Size() - suffix.Size();
        for (size_type i = 0; i < suffix.Size(); ++i) {
            if (data[offset + i] != suffixData[i]) return false;
        }
        return true;
    }

    bool EndsWith(const char* suffix) const {
        if (!suffix) return true;
        size_type suffixLen = StrLen(suffix);
        if (suffixLen > Size()) return false;
        const char* data = Data();
        size_type offset = Size() - suffixLen;
        for (size_type i = 0; i < suffixLen; ++i) {
            if (data[offset + i] != suffix[i]) return false;
        }
        return true;
    }

    bool operator==(const TString& other) const { return Size() == other.Size() && Compare(other) == 0; }
    bool operator!=(const TString& other) const { return !(*this == other); }
    bool operator<(const TString& other) const { return Compare(other) < 0; }
    bool operator<=(const TString& other) const { return Compare(other) <= 0; }
    bool operator>(const TString& other) const { return Compare(other) > 0; }
    bool operator>=(const TString& other) const { return Compare(other) >= 0; }

    bool operator==(const char* str) const {
        if (!str) return Size() == 0;
        const char* data = Data();
        size_type size = Size();
        for (size_type i = 0; i < size; ++i) {
            if (str[i] == '\0' || data[i] != str[i]) return false;
        }
        return str[size] == '\0';
    }

    bool operator!=(const char* str) const { return !(*this == str); }

    size_type Hash() const noexcept {
        const char* data = Data();
        size_type size = Size();
        size_type hash = 14695981039346656037ULL;
        for (size_type i = 0; i < size; ++i) {
            hash ^= static_cast<unsigned char>(data[i]);
            hash *= 1099511628211ULL;
        }
        return hash;
    }

private:
    static char* Allocate(size_type n) { return static_cast<char*>(::operator new(n)); }
    static void Deallocate(char* ptr) { ::operator delete(ptr); }

    static size_type StrLen(const char* str) {
        size_type len = 0;
        while (str[len] != '\0') ++len;
        return len;
    }

    static void MemCopy(char* dest, const char* src, size_type count) {
        for (size_type i = 0; i < count; ++i) dest[i] = src[i];
    }
};

inline TString operator+(const TString& lhs, const TString& rhs) {
    TString result(lhs);
    result.Append(rhs);
    return result;
}

inline TString operator+(const TString& lhs, const char* rhs) {
    TString result(lhs);
    result.Append(rhs);
    return result;
}

inline TString operator+(const char* lhs, const TString& rhs) {
    TString result(lhs);
    result.Append(rhs);
    return result;
}

} // namespace NTypes

