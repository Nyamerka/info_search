#pragma once

#include <lib/types/string/string.h>
#include <lib/collections/vector/vector.h>
#include <lib/collections/unordered_map/unordered_map.h>
#include <lib/collections/heap/heap.h>
#include <lib/tokenizer/tokenizer.h>

namespace NZipf {

using NTypes::TString;
using NCollections::TVector;
using NCollections::TUnorderedMap;
using NCollections::THeap;
using NTokenizer::TTokenizer;

/**
 * Статистика частотности слов и проверка закона Ципфа
 * 
 * Закон Ципфа: частота слова обратно пропорциональна его рангу
 * f(r) ≈ C / r^α, где α ≈ 1
 */
class TZipfAnalyzer {
public:
    struct TWordFrequency {
        TString Word;
        size_t Frequency;
        size_t Rank;
        double ExpectedFrequency;
        double Ratio;
        
        TWordFrequency() : Frequency(0), Rank(0), ExpectedFrequency(0), Ratio(0) {}
        TWordFrequency(const TString& word, size_t freq, size_t rank)
            : Word(word), Frequency(freq), Rank(rank), ExpectedFrequency(0), Ratio(0) {}
        
        bool operator<(const TWordFrequency& other) const {
            return Frequency < other.Frequency;
        }
    };

    struct TZipfStats {
        size_t TotalWords;
        size_t UniqueWords;
        double ZipfConstant;
        double ZipfExponent;
        double CorrelationCoefficient;
        TVector<TWordFrequency> TopWords;
    };

    TZipfAnalyzer() : TotalWords_(0) {}

    void AddText(const TString& text) {
        TTokenizer::TOptions opts;
        opts.LowerCase = true;
        opts.SkipPunctuation = true;
        opts.MinTokenLength = 2;
        
        TTokenizer tokenizer(opts);
        TVector<TString> tokens = tokenizer.TokenizeToStrings(text);
        
        for (size_t i = 0; i < tokens.Size(); ++i) {
            AddWord(tokens[i]);
        }
    }

    void AddWord(const TString& word) {
        size_t h = word.Hash();
        auto it = Frequencies_.Find(h);
        if (it != Frequencies_.end()) {
            ++it.Value();
        } else {
            Frequencies_.Insert(h, 1);
            Words_.Insert(h, word);
        }
        ++TotalWords_;
    }

    TZipfStats Analyze(size_t topN = 50) const {
        TZipfStats stats;
        stats.TotalWords = TotalWords_;
        stats.UniqueWords = Frequencies_.Size();
        
        TVector<TWordFrequency> freqs = GetSortedFrequencies();
        
        if (freqs.Empty()) {
            stats.ZipfConstant = 0;
            stats.ZipfExponent = 0;
            stats.CorrelationCoefficient = 0;
            return stats;
        }
        
        stats.ZipfConstant = static_cast<double>(freqs[0].Frequency);
        stats.ZipfExponent = EstimateExponent(freqs);
        stats.CorrelationCoefficient = CalculateCorrelation(freqs, stats.ZipfConstant, stats.ZipfExponent);
        
        size_t n = topN < freqs.Size() ? topN : freqs.Size();
        for (size_t i = 0; i < n; ++i) {
            TWordFrequency wf = freqs[i];
            wf.ExpectedFrequency = stats.ZipfConstant / Power(static_cast<double>(wf.Rank), stats.ZipfExponent);
            wf.Ratio = static_cast<double>(wf.Frequency) / wf.ExpectedFrequency;
            stats.TopWords.PushBack(wf);
        }
        
        return stats;
    }

    TVector<TWordFrequency> GetSortedFrequencies() const {
        THeap<TWordFrequency> heap;
        
        for (auto it = Frequencies_.begin(); it != Frequencies_.end(); ++it) {
            auto wordIt = Words_.Find(it.Key());
            if (wordIt != Words_.end()) {
                heap.Push(TWordFrequency(wordIt.Value(), it.Value(), 0));
            }
        }
        
        TVector<TWordFrequency> result;
        result.Reserve(heap.Size());
        
        size_t rank = 1;
        while (!heap.Empty()) {
            TWordFrequency wf = heap.ExtractTop();
            wf.Rank = rank++;
            result.PushBack(wf);
        }
        
        return result;
    }

    size_t GetFrequency(const TString& word) const {
        auto it = Frequencies_.Find(word.Hash());
        if (it != Frequencies_.end()) {
            return it.Value();
        }
        return 0;
    }

    size_t GetTotalWords() const { return TotalWords_; }
    size_t GetUniqueWords() const { return Frequencies_.Size(); }

    double GetTypeTokenRatio() const {
        if (TotalWords_ == 0) return 0;
        return static_cast<double>(Frequencies_.Size()) / TotalWords_;
    }

    void Clear() {
        Frequencies_.Clear();
        Words_.Clear();
        TotalWords_ = 0;
    }

    static bool VerifyZipfLaw(const TVector<TWordFrequency>& freqs, double tolerance = 0.3) {
        if (freqs.Size() < 10) return false;
        
        size_t validCount = 0;
        for (size_t i = 0; i < freqs.Size() && i < 20; ++i) {
            double ratio = freqs[i].Ratio;
            if (ratio > 1 - tolerance && ratio < 1 + tolerance) {
                ++validCount;
            }
        }
        
        return validCount >= freqs.Size() / 2;
    }

    static TString FormatStats(const TZipfStats& stats) {
        TString result;
        result.Append("=== Zipf Analysis ===\n");
        result.Append("Total words: ");
        result.Append(NumberToString(stats.TotalWords));
        result.Append("\nUnique words: ");
        result.Append(NumberToString(stats.UniqueWords));
        result.Append("\nZipf constant (C): ");
        result.Append(DoubleToString(stats.ZipfConstant, 2));
        result.Append("\nZipf exponent (α): ");
        result.Append(DoubleToString(stats.ZipfExponent, 4));
        result.Append("\nCorrelation: ");
        result.Append(DoubleToString(stats.CorrelationCoefficient, 4));
        result.Append("\n\nTop words:\n");
        result.Append("Rank\tFreq\tExpected\tRatio\tWord\n");
        
        for (size_t i = 0; i < stats.TopWords.Size() && i < 20; ++i) {
            const TWordFrequency& wf = stats.TopWords[i];
            result.Append(NumberToString(wf.Rank));
            result.Append("\t");
            result.Append(NumberToString(wf.Frequency));
            result.Append("\t");
            result.Append(DoubleToString(wf.ExpectedFrequency, 1));
            result.Append("\t\t");
            result.Append(DoubleToString(wf.Ratio, 3));
            result.Append("\t");
            result.Append(wf.Word);
            result.Append("\n");
        }
        
        return result;
    }

private:
    static double Power(double base, double exp) {
        if (exp == 0) return 1;
        if (exp == 1) return base;
        
        double result = 1;
        bool negative = exp < 0;
        if (negative) exp = -exp;
        
        int intPart = static_cast<int>(exp);
        double fracPart = exp - intPart;
        
        for (int i = 0; i < intPart; ++i) {
            result *= base;
        }
        
        if (fracPart > 0.001) {
            result *= ExpApprox(fracPart * LogApprox(base));
        }
        
        return negative ? 1.0 / result : result;
    }

    static double LogApprox(double x) {
        if (x <= 0) return 0;
        double result = 0;
        while (x > 2) { x /= 2.718281828; result += 1; }
        while (x < 0.5) { x *= 2.718281828; result -= 1; }
        double y = (x - 1) / (x + 1);
        double y2 = y * y;
        result += 2 * y * (1 + y2/3 + y2*y2/5 + y2*y2*y2/7);
        return result;
    }

    static double ExpApprox(double x) {
        double result = 1 + x;
        double term = x;
        for (int i = 2; i < 15; ++i) {
            term *= x / i;
            result += term;
        }
        return result;
    }

    static double EstimateExponent(const TVector<TWordFrequency>& freqs) {
        if (freqs.Size() < 2) return 1.0;
        
        double sumXY = 0, sumX = 0, sumY = 0, sumX2 = 0;
        size_t n = freqs.Size() < 100 ? freqs.Size() : 100;
        
        for (size_t i = 0; i < n; ++i) {
            double x = LogApprox(static_cast<double>(freqs[i].Rank));
            double y = LogApprox(static_cast<double>(freqs[i].Frequency));
            sumXY += x * y;
            sumX += x;
            sumY += y;
            sumX2 += x * x;
        }
        
        double slope = (n * sumXY - sumX * sumY) / (n * sumX2 - sumX * sumX);
        return -slope;
    }

    static double CalculateCorrelation(const TVector<TWordFrequency>& freqs, double C, double alpha) {
        if (freqs.Size() < 2) return 0;
        
        double sumXY = 0, sumX = 0, sumY = 0, sumX2 = 0, sumY2 = 0;
        size_t n = freqs.Size() < 100 ? freqs.Size() : 100;
        
        for (size_t i = 0; i < n; ++i) {
            double x = LogApprox(static_cast<double>(freqs[i].Frequency));
            double expected = C / Power(static_cast<double>(freqs[i].Rank), alpha);
            double y = LogApprox(expected);
            sumXY += x * y;
            sumX += x;
            sumY += y;
            sumX2 += x * x;
            sumY2 += y * y;
        }
        
        double num = n * sumXY - sumX * sumY;
        double den = (n * sumX2 - sumX * sumX) * (n * sumY2 - sumY * sumY);
        if (den <= 0) return 0;
        
        double sqrtDen = Power(den, 0.5);
        return num / sqrtDen;
    }

    static TString NumberToString(size_t n) {
        if (n == 0) return TString("0");
        TString result;
        while (n > 0) {
            result.PushBack('0' + (n % 10));
            n /= 10;
        }
        TString reversed;
        for (size_t i = result.Size(); i > 0; --i) {
            reversed.PushBack(result[i - 1]);
        }
        return reversed;
    }

    static TString DoubleToString(double d, int precision) {
        TString result;
        if (d < 0) {
            result.PushBack('-');
            d = -d;
        }
        
        size_t intPart = static_cast<size_t>(d);
        result.Append(NumberToString(intPart));
        
        if (precision > 0) {
            result.PushBack('.');
            double frac = d - intPart;
            for (int i = 0; i < precision; ++i) {
                frac *= 10;
                int digit = static_cast<int>(frac) % 10;
                result.PushBack('0' + digit);
            }
        }
        
        return result;
    }

    TUnorderedMap<size_t, size_t> Frequencies_;
    TUnorderedMap<size_t, TString> Words_;
    size_t TotalWords_;
};

} // namespace NZipf


