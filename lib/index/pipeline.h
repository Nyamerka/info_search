#pragma once

#include <lib/types/string/string.h>
#include <lib/collections/vector/vector.h>
#include <lib/tokenizer/tokenizer.h>
#include <lib/stemmer/stemmer.h>
#include <lib/index/boolean_index.h>

namespace NIndex {

using NTypes::TString;
using NCollections::TVector;
using NTokenizer::TTokenizer;
using NTokenizer::TToken;
using NStemmer::TPorterStemmer;
using NStemmer::TLemmatizer;

/**
 * Конвейер обработки текста: токенизация -> нормализация -> стемминг/лемматизация
 */
class TTextPipeline {
public:
    struct TOptions {
        bool LowerCase = true;
        bool UseStemming = true;
        bool UseLemmatization = false;
        bool SkipPunctuation = true;
        bool SkipNumbers = true;
        size_t MinTokenLength = 2;
        size_t MaxTokenLength = 100;
    };

    TTextPipeline() : Options_() {}
    explicit TTextPipeline(const TOptions& options) : Options_(options) {}

    TVector<TString> Process(const TString& text) const {
        TTokenizer::TOptions tokOpts;
        tokOpts.LowerCase = Options_.LowerCase;
        tokOpts.SkipPunctuation = Options_.SkipPunctuation;
        tokOpts.SkipNumbers = Options_.SkipNumbers;
        tokOpts.MinTokenLength = Options_.MinTokenLength;
        tokOpts.MaxTokenLength = Options_.MaxTokenLength;
        
        TTokenizer tokenizer(tokOpts);
        TVector<TString> tokens = tokenizer.TokenizeToStrings(text);
        
        if (Options_.UseLemmatization) {
            TLemmatizer lemmatizer;
            return lemmatizer.LemmatizeAll(tokens);
        }
        
        if (Options_.UseStemming) {
            TPorterStemmer stemmer;
            return stemmer.StemAll(tokens);
        }
        
        return tokens;
    }

    TVector<TToken> Tokenize(const TString& text) const {
        TTokenizer::TOptions tokOpts;
        tokOpts.LowerCase = Options_.LowerCase;
        tokOpts.SkipPunctuation = Options_.SkipPunctuation;
        tokOpts.SkipNumbers = Options_.SkipNumbers;
        tokOpts.MinTokenLength = Options_.MinTokenLength;
        tokOpts.MaxTokenLength = Options_.MaxTokenLength;
        
        TTokenizer tokenizer(tokOpts);
        return tokenizer.Tokenize(text);
    }

    TString NormalizeTerm(const TString& term) const {
        TString result = term;
        
        if (Options_.LowerCase) {
            result = TTokenizer::ToLower(result);
        }
        
        if (Options_.UseLemmatization) {
            TLemmatizer lemmatizer;
            return lemmatizer.Lemmatize(result);
        }
        
        if (Options_.UseStemming) {
            TPorterStemmer stemmer;
            return stemmer.Stem(result);
        }
        
        return result;
    }

    TVector<TString> NormalizeTerms(const TVector<TString>& terms) const {
        TVector<TString> result;
        result.Reserve(terms.Size());
        for (size_t i = 0; i < terms.Size(); ++i) {
            result.PushBack(NormalizeTerm(terms[i]));
        }
        return result;
    }

    const TOptions& GetOptions() const { return Options_; }
    void SetOptions(const TOptions& options) { Options_ = options; }

private:
    TOptions Options_;
};

/**
 * Поисковый движок: объединяет Pipeline + Index + Search
 */
class TSearchEngine {
public:
    struct TOptions {
        TTextPipeline::TOptions PipelineOptions;
    };

    TSearchEngine() : Pipeline_(), Index_(), TfIdf_(Index_), BooleanSearch_(Index_) {}
    explicit TSearchEngine(const TOptions& options) 
        : Pipeline_(options.PipelineOptions), Index_(), TfIdf_(Index_), BooleanSearch_(Index_) {}

    TDocId AddDocument(const TString& content) {
        TVector<TString> terms = Pipeline_.Process(content);
        return Index_.AddDocument(terms, content);
    }

    TDocId AddDocument(const TString& content, const TString& title) {
        TDocId docId = AddDocument(content);
        Titles_.Insert(docId, title);
        return docId;
    }

    TDocId AddDocumentTerms(const TVector<TString>& terms) {
        return Index_.AddDocument(terms);
    }

    template <typename InputIt>
    TDocId AddDocumentTerms(InputIt first, InputIt last) {
        return Index_.AddDocument(first, last);
    }

    TVector<TTfIdf::TSearchResult> Search(const TString& query, size_t topK = 10) const {
        TVector<TString> queryTerms = Pipeline_.Process(query);
        return TfIdf_.Search(queryTerms, topK);
    }

    TVector<TTfIdf::TSearchResult> SearchTerms(const TVector<TString>& queryTerms, size_t topK = 10) const {
        return TfIdf_.Search(queryTerms, topK);
    }

    template <typename InputIt>
    TVector<TTfIdf::TSearchResult> SearchTerms(InputIt first, InputIt last, size_t topK = 10) const {
        return TfIdf_.Search(first, last, topK);
    }

    TPostingList BooleanAnd(const TVector<TString>& terms) const {
        TVector<TString> normalizedTerms = Pipeline_.NormalizeTerms(terms);
        return BooleanSearch_.SearchAnd(normalizedTerms);
    }

    TPostingList BooleanOr(const TVector<TString>& terms) const {
        TVector<TString> normalizedTerms = Pipeline_.NormalizeTerms(terms);
        return BooleanSearch_.SearchOr(normalizedTerms);
    }

    template <typename InputIt>
    TPostingList BooleanAnd(InputIt first, InputIt last) const {
        TVector<TString> normalizedTerms;
        for (auto it = first; it != last; ++it) {
            normalizedTerms.PushBack(Pipeline_.NormalizeTerm(TString(*it)));
        }
        return BooleanSearch_.SearchAnd(normalizedTerms.begin(), normalizedTerms.end());
    }

    template <typename InputIt>
    TPostingList BooleanOr(InputIt first, InputIt last) const {
        TVector<TString> normalizedTerms;
        for (auto it = first; it != last; ++it) {
            normalizedTerms.PushBack(Pipeline_.NormalizeTerm(TString(*it)));
        }
        return BooleanSearch_.SearchOr(normalizedTerms.begin(), normalizedTerms.end());
    }

    TPostingList BooleanAndNot(const TVector<TString>& include, const TVector<TString>& exclude) const {
        TVector<TString> normInclude = Pipeline_.NormalizeTerms(include);
        TVector<TString> normExclude = Pipeline_.NormalizeTerms(exclude);
        return BooleanSearch_.SearchAndNot(normInclude, normExclude);
    }

    TString GetDocument(TDocId docId) const {
        return Index_.GetDocument(docId);
    }

    TString GetTitle(TDocId docId) const {
        auto it = Titles_.Find(docId);
        if (it != Titles_.end()) {
            return it.Value();
        }
        return TString();
    }

    size_t GetDocumentCount() const { return Index_.GetDocumentCount(); }
    size_t GetTermCount() const { return Index_.GetTermCount(); }

    const TInvertedIndex& GetIndex() const { return Index_; }
    const TTextPipeline& GetPipeline() const { return Pipeline_; }
    const TTfIdf& GetTfIdf() const { return TfIdf_; }
    const TBooleanSearch& GetBooleanSearch() const { return BooleanSearch_; }

    void Clear() {
        Index_.Clear();
        Titles_.Clear();
    }

private:
    TTextPipeline Pipeline_;
    TInvertedIndex Index_;
    TTfIdf TfIdf_;
    TBooleanSearch BooleanSearch_;
    TUnorderedMap<TDocId, TString> Titles_;
};

} // namespace NIndex

