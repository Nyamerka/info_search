#pragma once

#include <lib/types/string/string.h>
#include <lib/collections/vector/vector.h>

namespace NTokenizer {

using NTypes::TString;
using NCollections::TVector;

/**
 * Токен - единица текста с метаданными
 */
struct TToken {
    TString Text;
    size_t Position;
    size_t Length;
    
    TToken() : Position(0), Length(0) {}
    TToken(const TString& text, size_t pos, size_t len) : Text(text), Position(pos), Length(len) {}
    TToken(TString&& text, size_t pos, size_t len) : Text(std::move(text)), Position(pos), Length(len) {}
    
    bool operator==(const TToken& other) const {
        return Text == other.Text && Position == other.Position && Length == other.Length;
    }
};

/**
 * Токенизатор текста
 * 
 * Разбивает текст на токены (слова, числа, знаки препинания)
 */
class TTokenizer {
public:
    enum ETokenType {
        Word,
        Number,
        Punctuation,
        Whitespace,
        All
    };

    struct TOptions {
        bool LowerCase = true;
        bool SkipWhitespace = true;
        bool SkipPunctuation = true;
        bool SkipNumbers = true;
        size_t MinTokenLength = 1;
        size_t MaxTokenLength = 1000;
    };

public:
    TTokenizer() : Options_() {}
    explicit TTokenizer(const TOptions& options) : Options_(options) {}

    TVector<TToken> Tokenize(const TString& text) const {
        TVector<TToken> tokens;
        size_t pos = 0;
        size_t len = text.Size();
        
        while (pos < len) {
            while (pos < len && IsWhitespace(text[pos])) {
                if (!Options_.SkipWhitespace) {
                    size_t start = pos;
                    while (pos < len && IsWhitespace(text[pos])) ++pos;
                    tokens.PushBack(TToken(text.SubStr(start, pos - start), start, pos - start));
                } else {
                    ++pos;
                }
            }
            
            if (pos >= len) break;
            
            size_t start = pos;
            ETokenType type = GetCharType(text[pos]);
            
            if (type == Word) {
                while (pos < len && (IsAlpha(text[pos]) || IsDigit(text[pos]) || text[pos] == '_' || text[pos] == '-')) {
                    ++pos;
                }
                TString tokenText = text.SubStr(start, pos - start);
                if (Options_.LowerCase) {
                    tokenText = ToLower(tokenText);
                }
                if (tokenText.Size() >= Options_.MinTokenLength && tokenText.Size() <= Options_.MaxTokenLength) {
                    tokens.PushBack(TToken(std::move(tokenText), start, pos - start));
                }
            } else if (type == Number) {
                while (pos < len && (IsDigit(text[pos]) || text[pos] == '.' || text[pos] == ',')) {
                    ++pos;
                }
                if (!Options_.SkipNumbers) {
                    TString tokenText = text.SubStr(start, pos - start);
                    tokens.PushBack(TToken(std::move(tokenText), start, pos - start));
                }
            } else if (type == Punctuation) {
                ++pos;
                if (!Options_.SkipPunctuation) {
                    TString tokenText = text.SubStr(start, 1);
                    tokens.PushBack(TToken(std::move(tokenText), start, 1));
                }
            } else {
                ++pos;
            }
        }
        
        return tokens;
    }

    TVector<TString> TokenizeToStrings(const TString& text) const {
        TVector<TToken> tokens = Tokenize(text);
        TVector<TString> result;
        result.Reserve(tokens.Size());
        for (size_t i = 0; i < tokens.Size(); ++i) {
            result.PushBack(std::move(tokens[i].Text));
        }
        return result;
    }

    void SetOptions(const TOptions& options) { Options_ = options; }
    const TOptions& GetOptions() const { return Options_; }

    static TString ToLower(const TString& str) {
        TString result;
        result.Reserve(str.Size());
        for (size_t i = 0; i < str.Size(); ++i) {
            result.PushBack(ToLowerChar(str[i]));
        }
        return result;
    }

    static TString ToUpper(const TString& str) {
        TString result;
        result.Reserve(str.Size());
        for (size_t i = 0; i < str.Size(); ++i) {
            result.PushBack(ToUpperChar(str[i]));
        }
        return result;
    }

    static TString Normalize(const TString& str) {
        TString result;
        result.Reserve(str.Size());
        for (size_t i = 0; i < str.Size(); ++i) {
            char c = str[i];
            if (IsAlpha(c) || IsDigit(c)) {
                result.PushBack(ToLowerChar(c));
            }
        }
        return result;
    }

    static TString RemovePunctuation(const TString& str) {
        TString result;
        result.Reserve(str.Size());
        for (size_t i = 0; i < str.Size(); ++i) {
            if (!IsPunctuation(str[i])) {
                result.PushBack(str[i]);
            }
        }
        return result;
    }

    static TString Trim(const TString& str) {
        if (str.Empty()) return str;
        size_t start = 0;
        size_t end = str.Size();
        while (start < end && IsWhitespace(str[start])) ++start;
        while (end > start && IsWhitespace(str[end - 1])) --end;
        return str.SubStr(start, end - start);
    }

    static TVector<TString> Split(const TString& str, char delimiter) {
        TVector<TString> result;
        size_t start = 0;
        for (size_t i = 0; i < str.Size(); ++i) {
            if (str[i] == delimiter) {
                if (i > start) {
                    result.PushBack(str.SubStr(start, i - start));
                }
                start = i + 1;
            }
        }
        if (start < str.Size()) {
            result.PushBack(str.SubStr(start, str.Size() - start));
        }
        return result;
    }

    static TString Join(const TVector<TString>& parts, const TString& delimiter) {
        if (parts.Empty()) return TString();
        TString result = parts[0];
        for (size_t i = 1; i < parts.Size(); ++i) {
            result.Append(delimiter);
            result.Append(parts[i]);
        }
        return result;
    }

private:
    static bool IsAlpha(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    static bool IsDigit(char c) {
        return c >= '0' && c <= '9';
    }

    static bool IsWhitespace(char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    static bool IsPunctuation(char c) {
        return c == '.' || c == ',' || c == '!' || c == '?' || c == ';' || c == ':' ||
               c == '"' || c == '\'' || c == '(' || c == ')' || c == '[' || c == ']' ||
               c == '{' || c == '}' || c == '-' || c == '/' || c == '\\';
    }

    static char ToLowerChar(char c) {
        if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
        return c;
    }

    static char ToUpperChar(char c) {
        if (c >= 'a' && c <= 'z') return c - ('a' - 'A');
        return c;
    }

    static ETokenType GetCharType(char c) {
        if (IsAlpha(c)) return Word;
        if (IsDigit(c)) return Number;
        if (IsWhitespace(c)) return Whitespace;
        return Punctuation;
    }

    TOptions Options_;
};

} // namespace NTokenizer


