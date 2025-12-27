#pragma once

#include <lib/types/string/string.h>
#include <lib/collections/vector/vector.h>
#include <lib/collections/unordered_map/unordered_map.h>
#include <lib/collections/unordered_set/unordered_set.h>

namespace NIndex {

using NTypes::TString;
using NCollections::TVector;
using NCollections::TUnorderedMap;
using NCollections::TUnorderedSet;
using NCollections::TStringHash;

using TDocId = size_t;
using TPostingList = TVector<TDocId>;

/**
 * Инвертированный индекс для булева поиска
 */
class TInvertedIndex {
public:
    TInvertedIndex() : NextDocId_(0) {}

    TDocId AddDocument(const TVector<TString>& terms) {
        return AddDocument(terms.begin(), terms.end());
    }

    TDocId AddDocument(const TVector<TString>& terms, const TString& rawContent) {
        return AddDocument(terms.begin(), terms.end(), rawContent);
    }

    template <typename InputIt>
    TDocId AddDocument(InputIt first, InputIt last) {
        TDocId docId = NextDocId_++;

        size_t termCount = 0;
        for (auto it = first; it != last; ++it) {
            TString term = *it;
            AddTermToIndex(term, docId);
            IncrementTermFrequency(docId, term);
            ++termCount;
        }

        DocTermCounts_.Insert(docId, termCount);
        return docId;
    }

    template <typename InputIt>
    TDocId AddDocument(InputIt first, InputIt last, const TString& rawContent) {
        TDocId docId = AddDocument(first, last);
        Documents_.Insert(docId, rawContent);
        return docId;
    }

    const TPostingList& GetPostingList(const TString& term) const {
        static const TPostingList empty;
        auto it = Index_.Find(term);
        if (it != Index_.end()) {
            return it.Value();
        }
        return empty;
    }

    bool ContainsTerm(const TString& term) const {
        return Index_.Contains(term);
    }

    size_t GetDocumentFrequency(const TString& term) const {
        auto it = Index_.Find(term);
        if (it != Index_.end()) {
            return it.Value().Size();
        }
        return 0;
    }

    size_t GetTermFrequency(TDocId docId, const TString& term) const {
        auto docIt = TermFrequencies_.Find(docId);
        if (docIt == TermFrequencies_.end()) return 0;
        
        auto termIt = docIt.Value().Find(term);
        if (termIt == docIt.Value().end()) return 0;
        
        return termIt.Value();
    }

    size_t GetDocumentLength(TDocId docId) const {
        auto it = DocTermCounts_.Find(docId);
        if (it != DocTermCounts_.end()) {
            return it.Value();
        }
        return 0;
    }

    size_t GetDocumentCount() const { return NextDocId_; }
    size_t GetTermCount() const { return Index_.Size(); }

    double GetAverageDocumentLength() const {
        if (NextDocId_ == 0) return 0;
        size_t total = 0;
        for (auto it = DocTermCounts_.begin(); it != DocTermCounts_.end(); ++it) {
            total += it.Value();
        }
        return static_cast<double>(total) / NextDocId_;
    }

    TString GetDocument(TDocId docId) const {
        auto it = Documents_.Find(docId);
        if (it != Documents_.end()) {
            return it.Value();
        }
        return TString();
    }

    TVector<TString> GetAllTerms() const {
        TVector<TString> result;
        for (auto it = Index_.begin(); it != Index_.end(); ++it) {
            result.PushBack(it.Key());
        }
        return result;
    }

    TVector<TDocId> GetAllDocIds() const {
        TVector<TDocId> result;
        for (TDocId i = 0; i < NextDocId_; ++i) {
            result.PushBack(i);
        }
        return result;
    }

    void Clear() {
        Index_.Clear();
        Documents_.Clear();
        TermFrequencies_.Clear();
        DocTermCounts_.Clear();
        NextDocId_ = 0;
    }

private:
    void AddTermToIndex(const TString& term, TDocId docId) {
        auto it = Index_.Find(term);
        if (it != Index_.end()) {
            TPostingList& list = it.Value();
            if (list.Empty() || list.Back() != docId) {
                list.PushBack(docId);
            }
        } else {
            TPostingList list;
            list.PushBack(docId);
            Index_.Insert(term, std::move(list));
        }
    }

    void IncrementTermFrequency(TDocId docId, const TString& term) {
        auto docIt = TermFrequencies_.Find(docId);
        if (docIt == TermFrequencies_.end()) {
            TUnorderedMap<TString, size_t, TStringHash> termMap;
            termMap.Insert(term, 1);
            TermFrequencies_.Insert(docId, std::move(termMap));
        } else {
            auto termIt = docIt.Value().Find(term);
            if (termIt == docIt.Value().end()) {
                docIt.Value().Insert(term, 1);
            } else {
                ++termIt.Value();
            }
        }
    }

    TUnorderedMap<TString, TPostingList, TStringHash> Index_;
    TUnorderedMap<TDocId, TString> Documents_;
    TUnorderedMap<TDocId, TUnorderedMap<TString, size_t, TStringHash>> TermFrequencies_;
    TUnorderedMap<TDocId, size_t> DocTermCounts_;
    TDocId NextDocId_;
};

/**
 * Булев поиск (AND, OR, NOT)
 */
class TBooleanSearch {
public:
    explicit TBooleanSearch(const TInvertedIndex& index) : Index_(index) {}

    TPostingList Search(const TString& term) const {
        const TPostingList& list = Index_.GetPostingList(term);
        return TPostingList(list);
    }

    TPostingList SearchAnd(const TVector<TString>& terms) const {
        return SearchAnd(terms.begin(), terms.end());
    }

    TPostingList SearchOr(const TVector<TString>& terms) const {
        return SearchOr(terms.begin(), terms.end());
    }

    template <typename InputIt>
    TPostingList SearchAnd(InputIt first, InputIt last) const {
        if (first == last) return TPostingList();

        TString firstTerm = *first;
        TPostingList result = Search(firstTerm);
        ++first;
        for (auto it = first; it != last; ++it) {
            TString term = *it;
            result = Intersect(result, Index_.GetPostingList(term));
            if (result.Empty()) break;
        }
        return result;
    }

    template <typename InputIt>
    TPostingList SearchOr(InputIt first, InputIt last) const {
        TPostingList result;
        for (auto it = first; it != last; ++it) {
            TString term = *it;
            result = Union(result, Index_.GetPostingList(term));
        }
        return result;
    }

    TPostingList SearchNot(const TString& term, const TPostingList& universe) const {
        const TPostingList& termDocs = Index_.GetPostingList(term);
        TUnorderedSet<TDocId> termSet;
        for (size_t i = 0; i < termDocs.Size(); ++i) {
            termSet.Insert(termDocs[i]);
        }
        
        TPostingList result;
        for (size_t i = 0; i < universe.Size(); ++i) {
            if (!termSet.Contains(universe[i])) {
                result.PushBack(universe[i]);
            }
        }
        return result;
    }

    TPostingList SearchAndNot(const TVector<TString>& include, const TVector<TString>& exclude) const {
        TPostingList result = SearchAnd(include);
        
        TUnorderedSet<TDocId> excludeSet;
        for (size_t i = 0; i < exclude.Size(); ++i) {
            const TPostingList& excludeDocs = Index_.GetPostingList(exclude[i]);
            for (size_t j = 0; j < excludeDocs.Size(); ++j) {
                excludeSet.Insert(excludeDocs[j]);
            }
        }
        
        TPostingList filtered;
        for (size_t i = 0; i < result.Size(); ++i) {
            if (!excludeSet.Contains(result[i])) {
                filtered.PushBack(result[i]);
            }
        }
        return filtered;
    }

private:
    static TPostingList Intersect(const TPostingList& a, const TPostingList& b) {
        TPostingList result;
        size_t i = 0, j = 0;
        while (i < a.Size() && j < b.Size()) {
            if (a[i] == b[j]) {
                result.PushBack(a[i]);
                ++i; ++j;
            } else if (a[i] < b[j]) {
                ++i;
            } else {
                ++j;
            }
        }
        return result;
    }

    static TPostingList Union(const TPostingList& a, const TPostingList& b) {
        TPostingList result;
        size_t i = 0, j = 0;
        while (i < a.Size() && j < b.Size()) {
            if (a[i] == b[j]) {
                result.PushBack(a[i]);
                ++i; ++j;
            } else if (a[i] < b[j]) {
                result.PushBack(a[i]);
                ++i;
            } else {
                result.PushBack(b[j]);
                ++j;
            }
        }
        while (i < a.Size()) { result.PushBack(a[i]); ++i; }
        while (j < b.Size()) { result.PushBack(b[j]); ++j; }
        return result;
    }

    const TInvertedIndex& Index_;
};

/**
 * TF-IDF ранжирование
 * 
 * TF(t,d) = freq(t,d) / |d|
 * IDF(t) = log(N / df(t))
 * TF-IDF(t,d) = TF(t,d) * IDF(t)
 */
class TTfIdf {
public:
    struct TSearchResult {
        TDocId DocId;
        double Score;
        
        TSearchResult() : DocId(0), Score(0) {}
        TSearchResult(TDocId id, double s) : DocId(id), Score(s) {}
        
        bool operator<(const TSearchResult& other) const {
            return Score > other.Score;
        }
    };

    explicit TTfIdf(const TInvertedIndex& index) : Index_(index) {}

    double ComputeTF(TDocId docId, const TString& term) const {
        size_t tf = Index_.GetTermFrequency(docId, term);
        size_t docLen = Index_.GetDocumentLength(docId);
        if (docLen == 0) return 0;
        return static_cast<double>(tf) / docLen;
    }

    double ComputeRawTF(TDocId docId, const TString& term) const {
        return static_cast<double>(Index_.GetTermFrequency(docId, term));
    }

    double ComputeIDF(const TString& term) const {
        size_t N = Index_.GetDocumentCount();
        size_t df = Index_.GetDocumentFrequency(term);
        if (N == 0 || df == 0) return 0;
        return Log(static_cast<double>(N + 1) / static_cast<double>(df + 1)) + 1.0;
    }

    double ComputeTfIdf(TDocId docId, const TString& term) const {
        return ComputeTF(docId, term) * ComputeIDF(term);
    }

    double ComputeDocumentScore(TDocId docId, const TVector<TString>& queryTerms) const {
        double score = 0;
        for (size_t i = 0; i < queryTerms.Size(); ++i) {
            score += ComputeTfIdf(docId, queryTerms[i]);
        }
        return score;
    }

    template <typename InputIt>
    double ComputeDocumentScore(TDocId docId, InputIt first, InputIt last) const {
        double score = 0;
        for (auto it = first; it != last; ++it) {
            TString term = *it;
            score += ComputeTfIdf(docId, term);
        }
        return score;
    }

    TVector<TSearchResult> Search(const TVector<TString>& queryTerms, size_t topK = 10) const {
        TUnorderedSet<TDocId> candidateDocs;
        for (size_t i = 0; i < queryTerms.Size(); ++i) {
            const TPostingList& docs = Index_.GetPostingList(queryTerms[i]);
            for (size_t j = 0; j < docs.Size(); ++j) {
                candidateDocs.Insert(docs[j]);
            }
        }
        
        TVector<TSearchResult> results;
        for (auto it = candidateDocs.begin(); it != candidateDocs.end(); ++it) {
            TDocId docId = it.Value();
            double score = ComputeDocumentScore(docId, queryTerms);
            if (score > 0) {
                results.PushBack(TSearchResult(docId, score));
            }
        }
        
        SortResults(results);
        
        if (results.Size() > topK) {
            TVector<TSearchResult> topResults;
            for (size_t i = 0; i < topK; ++i) {
                topResults.PushBack(results[i]);
            }
            return topResults;
        }
        
        return results;
    }

    template <typename InputIt>
    TVector<TSearchResult> Search(InputIt first, InputIt last, size_t topK = 10) const {
        TVector<TString> queryTerms;
        for (auto it = first; it != last; ++it) {
            queryTerms.PushBack(TString(*it));
        }
        return Search(queryTerms, topK);
    }

    TVector<double> GetTermWeights(const TVector<TString>& terms) const {
        TVector<double> weights;
        for (size_t i = 0; i < terms.Size(); ++i) {
            weights.PushBack(ComputeIDF(terms[i]));
        }
        return weights;
    }

    template <typename InputIt>
    TVector<double> GetTermWeights(InputIt first, InputIt last) const {
        TVector<double> weights;
        for (auto it = first; it != last; ++it) {
            TString term = *it;
            weights.PushBack(ComputeIDF(term));
        }
        return weights;
    }

private:
    static double Log(double x) {
        if (x <= 0) return 0;
        double result = 0;
        while (x > 2.718281828) { x /= 2.718281828; result += 1; }
        while (x < 0.367879441) { x *= 2.718281828; result -= 1; }
        double y = (x - 1) / (x + 1);
        double y2 = y * y;
        result += 2 * y * (1 + y2/3 + y2*y2/5 + y2*y2*y2/7);
        return result;
    }

    static void SortResults(TVector<TSearchResult>& results) {
        for (size_t i = 0; i < results.Size(); ++i) {
            for (size_t j = i + 1; j < results.Size(); ++j) {
                if (results[j].Score > results[i].Score) {
                    TSearchResult tmp = results[i];
                    results[i] = results[j];
                    results[j] = tmp;
                }
            }
        }
    }

    const TInvertedIndex& Index_;
};

} // namespace NIndex
