#pragma once

#include <lib/types/string/string.h>
#include <lib/collections/vector/vector.h>
#include <lib/collections/unordered_map/unordered_map.h>
#include <lib/index/pipeline.h>
#include <lib/lzw/lzw.h>

namespace NSearchSystem {

using NTypes::TString;
using NCollections::TVector;
using NCollections::TUnorderedMap;

using NIndex::TDocId;
using NIndex::TPostingList;
using NIndex::TTfIdf;

/**
 * База документов и поисковый интерфейс: добавление документов, булев поиск, TF-IDF ранжирование.
 */
class TSearchDatabase {
public:
    struct TOptions {
        NIndex::TTextPipeline::TOptions Pipeline;
        bool StoreDocuments = true;
        bool CompressDocuments = true;
        bool StoreTitles = true;
    };

    TSearchDatabase() : TSearchDatabase(TOptions()) {}

    explicit TSearchDatabase(const TOptions& options)
        : Options_(options)
        , Engine_(MakeEngineOptions(options))
        , Lzw_()
    {}

    TDocId AddDocument(const TString& content) {
        return AddDocument(content, TString());
    }

    TDocId AddDocument(const TString& content, const TString& title) {
        TVector<TString> terms = Engine_.GetPipeline().Process(content);
        TDocId docId = Engine_.AddDocumentTerms(terms.begin(), terms.end());

        if (Options_.StoreDocuments) {
            StoreDoc(docId, content);
        }
        if (Options_.StoreTitles && !title.Empty()) {
            Titles_.Insert(docId, title);
        }
        return docId;
    }

    template <typename TermIt>
    TDocId AddDocumentTerms(TermIt first, TermIt last) {
        TDocId docId = Engine_.AddDocumentTerms(first, last);
        return docId;
    }

    template <typename TermIt>
    TDocId AddDocumentTerms(TermIt first, TermIt last, const TString& content) {
        TDocId docId = Engine_.AddDocumentTerms(first, last);
        if (Options_.StoreDocuments) {
            StoreDoc(docId, content);
        }
        return docId;
    }

    TVector<TTfIdf::TSearchResult> Search(const TString& query, size_t topK = 10) const {
        return Engine_.Search(query, topK);
    }

    template <typename TermIt>
    TVector<TTfIdf::TSearchResult> SearchTerms(TermIt first, TermIt last, size_t topK = 10) const {
        return Engine_.SearchTerms(first, last, topK);
    }

    TPostingList BooleanAnd(const TVector<TString>& terms) const {
        return Engine_.BooleanAnd(terms);
    }

    TPostingList BooleanOr(const TVector<TString>& terms) const {
        return Engine_.BooleanOr(terms);
    }

    template <typename TermIt>
    TPostingList BooleanAnd(TermIt first, TermIt last) const {
        return Engine_.BooleanAnd(first, last);
    }

    template <typename TermIt>
    TPostingList BooleanOr(TermIt first, TermIt last) const {
        return Engine_.BooleanOr(first, last);
    }

    TPostingList BooleanQuery(const TString& query) const {
        TVector<TString> tokens = TokenizeBooleanQuery(query);
        TVector<TString> rpn = ToRpn(tokens);
        return EvalRpn(rpn);
    }

    TString GetDocument(TDocId docId) const {
        if (!Options_.StoreDocuments) {
            return TString();
        }
        if (Options_.CompressDocuments) {
            auto it = CompressedDocs_.Find(docId);
            if (it == CompressedDocs_.end()) {
                return TString();
            }
            return Lzw_.Decompress(it.Value());
        }
        auto it = RawDocs_.Find(docId);
        if (it == RawDocs_.end()) {
            return TString();
        }
        return it.Value();
    }

    TString GetTitle(TDocId docId) const {
        if (!Options_.StoreTitles) {
            return TString();
        }
        auto it = Titles_.Find(docId);
        if (it == Titles_.end()) {
            return TString();
        }
        return it.Value();
    }

    size_t GetDocumentCount() const { return Engine_.GetDocumentCount(); }
    size_t GetTermCount() const { return Engine_.GetTermCount(); }

    void Clear() {
        Engine_.Clear();
        RawDocs_.Clear();
        CompressedDocs_.Clear();
        Titles_.Clear();
    }

    const NIndex::TSearchEngine& GetEngine() const { return Engine_; }

private:
    static NIndex::TSearchEngine::TOptions MakeEngineOptions(const TOptions& options) {
        NIndex::TSearchEngine::TOptions e;
        e.PipelineOptions = options.Pipeline;
        return e;
    }

    void StoreDoc(TDocId docId, const TString& content) {
        if (Options_.CompressDocuments) {
            CompressedDocs_.Insert(docId, Lzw_.Compress(content));
        } else {
            RawDocs_.Insert(docId, content);
        }
    }

    static bool IsWs(char c) {
        return c == ' ' || c == '\t' || c == '\n' || c == '\r';
    }

    static bool IsParen(char c) {
        return c == '(' || c == ')';
    }

    TVector<TString> TokenizeBooleanQuery(const TString& query) const {
        TVector<TString> tokens;
        TString cur;
        for (size_t i = 0; i < query.Size(); ++i) {
            char c = query[i];
            if (IsWs(c)) {
                if (!cur.Empty()) {
                    tokens.PushBack(cur);
                    cur.Clear();
                }
                continue;
            }
            if (IsParen(c)) {
                if (!cur.Empty()) {
                    tokens.PushBack(cur);
                    cur.Clear();
                }
                tokens.PushBack(TString(1, c));
                continue;
            }
            cur.PushBack(c);
        }
        if (!cur.Empty()) {
            tokens.PushBack(cur);
        }
        return tokens;
    }

    static bool IsOp(const TString& t) {
        return t == "and" || t == "or" || t == "not" || t == "AND" || t == "OR" || t == "NOT";
    }

    static int Prec(const TString& t) {
        if (t == "not" || t == "NOT") return 3;
        if (t == "and" || t == "AND") return 2;
        if (t == "or" || t == "OR") return 1;
        return 0;
    }

    static bool IsLeftAssoc(const TString& t) {
        return !(t == "not" || t == "NOT");
    }

    TVector<TString> ToRpn(const TVector<TString>& tokens) const {
        TVector<TString> out;
        TVector<TString> ops;

        for (size_t i = 0; i < tokens.Size(); ++i) {
            TString tok = tokens[i];

            if (tok == "(") {
                ops.PushBack(tok);
                continue;
            }
            if (tok == ")") {
                while (!ops.Empty() && ops.Back() != "(") {
                    out.PushBack(ops.Back());
                    ops.PopBack();
                }
                if (!ops.Empty() && ops.Back() == "(") {
                    ops.PopBack();
                }
                continue;
            }

            if (IsOp(tok)) {
                while (!ops.Empty() && IsOp(ops.Back())) {
                    int p1 = Prec(tok);
                    int p2 = Prec(ops.Back());
                    if ((p2 > p1) || (p2 == p1 && IsLeftAssoc(tok))) {
                        out.PushBack(ops.Back());
                        ops.PopBack();
                    } else {
                        break;
                    }
                }
                ops.PushBack(tok);
                continue;
            }

            out.PushBack(Engine_.GetPipeline().NormalizeTerm(tok));
        }

        while (!ops.Empty()) {
            if (ops.Back() != "(" && ops.Back() != ")") {
                out.PushBack(ops.Back());
            }
            ops.PopBack();
        }

        return out;
    }

    static TPostingList Intersect(const TPostingList& a, const TPostingList& b) {
        TPostingList r;
        size_t i = 0;
        size_t j = 0;
        while (i < a.Size() && j < b.Size()) {
            if (a[i] == b[j]) {
                r.PushBack(a[i]);
                ++i;
                ++j;
            } else if (a[i] < b[j]) {
                ++i;
            } else {
                ++j;
            }
        }
        return r;
    }

    static TPostingList Union(const TPostingList& a, const TPostingList& b) {
        TPostingList r;
        size_t i = 0;
        size_t j = 0;
        while (i < a.Size() && j < b.Size()) {
            if (a[i] == b[j]) {
                r.PushBack(a[i]);
                ++i;
                ++j;
            } else if (a[i] < b[j]) {
                r.PushBack(a[i]);
                ++i;
            } else {
                r.PushBack(b[j]);
                ++j;
            }
        }
        while (i < a.Size()) {
            r.PushBack(a[i]);
            ++i;
        }
        while (j < b.Size()) {
            r.PushBack(b[j]);
            ++j;
        }
        return r;
    }

    TPostingList NotList(const TPostingList& a) const {
        TPostingList r;
        size_t n = Engine_.GetDocumentCount();
        size_t i = 0;
        for (size_t doc = 0; doc < n; ++doc) {
            if (i < a.Size() && a[i] == doc) {
                ++i;
            } else {
                r.PushBack(doc);
            }
        }
        return r;
    }

    TPostingList EvalRpn(const TVector<TString>& rpn) const {
        TVector<TPostingList> st;
        for (size_t i = 0; i < rpn.Size(); ++i) {
            const TString& tok = rpn[i];
            if (IsOp(tok)) {
                if (tok == "not" || tok == "NOT") {
                    if (st.Empty()) return TPostingList();
                    TPostingList a = st.Back();
                    st.PopBack();
                    st.PushBack(NotList(a));
                    continue;
                }
                if (st.Size() < 2) return TPostingList();
                TPostingList b = st.Back();
                st.PopBack();
                TPostingList a = st.Back();
                st.PopBack();
                if (tok == "and" || tok == "AND") {
                    st.PushBack(Intersect(a, b));
                } else {
                    st.PushBack(Union(a, b));
                }
                continue;
            }
            const TPostingList& pl = Engine_.GetIndex().GetPostingList(tok);
            st.PushBack(TPostingList(pl));
        }
        if (st.Empty()) return TPostingList();
        return st.Back();
    }

private:
    TOptions Options_;
    NIndex::TSearchEngine Engine_;
    NLzw::TLzw Lzw_;

    TUnorderedMap<TDocId, TString> RawDocs_;
    TUnorderedMap<TDocId, NLzw::TLzw::TBytes> CompressedDocs_;
    TUnorderedMap<TDocId, TString> Titles_;
};

} // namespace NSearchSystem


