#pragma once

#include <cstddef>

#include <lib/types/string/string.h>
#include <lib/collections/vector/vector.h>
#include <lib/collections/unordered_map/unordered_map.h>

namespace NLzw {

using NTypes::TString;
using NCollections::TVector;
using NCollections::TUnorderedMap;
using NCollections::TStringHash;

/**
 * LZW (Lempel–Ziv–Welch) компрессор для байтовой строки
 */
class TLzw {
public:
    using TByte = unsigned char;
    using TBytes = TVector<TByte>;

    struct TOptions {
        unsigned short MaxCode = 4095;
        unsigned short EndCode = 4095;
        unsigned short FirstFreeCode = 256;
        unsigned short CodeBits = 12;
    };

    TLzw() : Options_() {}
    explicit TLzw(const TOptions& options) : Options_(options) {}

    TBytes Compress(const TString& input) const {
        return Compress(input.begin(), input.end());
    }

    template <typename InputIt>
    TBytes Compress(InputIt first, InputIt last) const {
        TUnorderedMap<TString, unsigned short, TStringHash> dict;
        dict.Rehash(4096);

        for (unsigned int i = 0; i < 256; ++i) {
            TString s(1, static_cast<char>(static_cast<unsigned char>(i)));
            dict.Insert(std::move(s), static_cast<unsigned short>(i));
        }

        unsigned short nextCode = Options_.FirstFreeCode;
        TString w;
        TVector<unsigned short> codes;

        for (auto it = first; it != last; ++it) {
            char c = static_cast<char>(*it);
            if (w.Empty()) {
                w = TString(1, c);
                continue;
            }

            TString wc(w);
            wc.PushBack(c);

            auto found = dict.Find(wc);
            if (found != dict.end()) {
                w = std::move(wc);
                continue;
            }

            auto wIt = dict.Find(w);
            if (wIt != dict.end()) {
                codes.PushBack(wIt.Value());
            }

            if (nextCode < Options_.EndCode) {
                dict.Insert(std::move(wc), nextCode);
                ++nextCode;
            }

            w = TString(1, c);
        }

        if (!w.Empty()) {
            auto wIt = dict.Find(w);
            if (wIt != dict.end()) {
                codes.PushBack(wIt.Value());
            }
        }

        codes.PushBack(Options_.EndCode);
        return PackCodes(codes);
    }

    TString Decompress(const TBytes& data) const {
        return Decompress(data.begin(), data.end());
    }

    template <typename ByteIt>
    TString Decompress(ByteIt first, ByteIt last) const {
        TVector<unsigned short> codes = UnpackCodes(first, last);
        if (codes.Empty()) {
            return TString();
        }

        TVector<TString> dict;
        dict.Reserve(4096);
        for (unsigned int i = 0; i < 256; ++i) {
            dict.PushBack(TString(1, static_cast<char>(static_cast<unsigned char>(i))));
        }

        unsigned short nextCode = Options_.FirstFreeCode;

        size_t idx = 0;
        unsigned short firstCode = codes[idx++];
        if (firstCode == Options_.EndCode) {
            return TString();
        }

        if (firstCode >= dict.Size()) {
            return TString();
        }

        TString w = dict[firstCode];
        TString out = w;

        while (idx < codes.Size()) {
            unsigned short k = codes[idx++];
            if (k == Options_.EndCode) {
                break;
            }

            TString entry;
            if (k < dict.Size()) {
                entry = dict[k];
            } else if (k == nextCode && !w.Empty()) {
                entry = w;
                entry.PushBack(w[0]);
            } else {
                return TString();
            }

            out.Append(entry);

            if (nextCode < Options_.EndCode && !w.Empty() && !entry.Empty()) {
                TString newEntry = w;
                newEntry.PushBack(entry[0]);
                dict.PushBack(std::move(newEntry));
                ++nextCode;
            }

            w = std::move(entry);
        }

        return out;
    }

private:
    TBytes PackCodes(const TVector<unsigned short>& codes) const {
        TBytes out;
        unsigned int buffer = 0;
        unsigned int bits = 0;

        for (size_t i = 0; i < codes.Size(); ++i) {
            unsigned int code = static_cast<unsigned int>(codes[i]) & ((1u << Options_.CodeBits) - 1u);
            buffer |= (code << bits);
            bits += Options_.CodeBits;
            while (bits >= 8) {
                out.PushBack(static_cast<TByte>(buffer & 0xFFu));
                buffer >>= 8;
                bits -= 8;
            }
        }

        if (bits > 0) {
            out.PushBack(static_cast<TByte>(buffer & 0xFFu));
        }

        return out;
    }

    template <typename ByteIt>
    TVector<unsigned short> UnpackCodes(ByteIt first, ByteIt last) const {
        TVector<unsigned short> codes;
        unsigned int buffer = 0;
        unsigned int bits = 0;

        for (auto it = first; it != last; ++it) {
            unsigned int byte = static_cast<unsigned int>(static_cast<unsigned char>(*it));
            buffer |= (byte << bits);
            bits += 8;

            while (bits >= Options_.CodeBits) {
                unsigned int code = buffer & ((1u << Options_.CodeBits) - 1u);
                codes.PushBack(static_cast<unsigned short>(code));
                buffer >>= Options_.CodeBits;
                bits -= Options_.CodeBits;
            }
        }

        return codes;
    }

private:
    TOptions Options_;
};

} // namespace NLzw


