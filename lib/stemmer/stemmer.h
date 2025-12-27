#pragma once

#include <lib/types/string/string.h>
#include <lib/collections/vector/vector.h>
#include <lib/collections/unordered_map/unordered_map.h>
#include <lib/stemmer/stemmer_const.h>

namespace NStemmer {

using NTypes::TString;
using NCollections::TVector;
using NCollections::TUnorderedMap;

/**
 * Porter Stemmer для английского языка
 */
class TPorterStemmer {
public:
    TString Stem(const TString& word) const {
        if (word.Size() < 3) return word;
        
        TString result = ToLower(word);
        
        result = Step1a(result);
        result = Step1b(result);
        result = Step1c(result);
        result = Step2(result);
        result = Step3(result);
        result = Step4(result);
        result = Step5a(result);
        result = Step5b(result);
        
        return result;
    }

    TVector<TString> StemAll(const TVector<TString>& words) const {
        TVector<TString> result;
        result.Reserve(words.Size());
        for (size_t i = 0; i < words.Size(); ++i) {
            result.PushBack(Stem(words[i]));
        }
        return result;
    }

private:
    static TString ToLower(const TString& str) {
        TString result;
        result.Reserve(str.Size());
        for (size_t i = 0; i < str.Size(); ++i) {
            char c = str[i];
            if (c >= 'A' && c <= 'Z') c = c + ('a' - 'A');
            result.PushBack(c);
        }
        return result;
    }

    static bool IsConsonant(const TString& str, size_t i) {
        char c = str[i];
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') return false;
        if (c == 'y') {
            if (i == 0) return true;
            return !IsConsonant(str, i - 1);
        }
        return true;
    }

    static size_t MeasureM(const TString& str) {
        size_t n = 0;
        size_t i = 0;
        size_t len = str.Size();
        
        while (i < len && !IsConsonant(str, i)) ++i;
        while (i < len) {
            while (i < len && IsConsonant(str, i)) ++i;
            if (i >= len) break;
            ++n;
            while (i < len && !IsConsonant(str, i)) ++i;
        }
        return n;
    }

    static bool HasVowel(const TString& str) {
        for (size_t i = 0; i < str.Size(); ++i) {
            if (!IsConsonant(str, i)) return true;
        }
        return false;
    }

    static bool EndsWithDoubleConsonant(const TString& str) {
        size_t len = str.Size();
        if (len < 2) return false;
        if (str[len - 1] != str[len - 2]) return false;
        return IsConsonant(str, len - 1);
    }

    static bool EndsCVC(const TString& str) {
        size_t len = str.Size();
        if (len < 3) return false;
        if (!IsConsonant(str, len - 1)) return false;
        if (IsConsonant(str, len - 2)) return false;
        if (!IsConsonant(str, len - 3)) return false;
        char c = str[len - 1];
        if (c == 'w' || c == 'x' || c == 'y') return false;
        return true;
    }

    static bool EndsWith(const TString& str, const char* suffix) {
        size_t slen = 0;
        while (suffix[slen]) ++slen;
        if (str.Size() < slen) return false;
        for (size_t i = 0; i < slen; ++i) {
            if (str[str.Size() - slen + i] != suffix[i]) return false;
        }
        return true;
    }

    static TString RemoveSuffix(const TString& str, size_t len) {
        if (str.Size() <= len) return TString();
        return str.SubStr(0, str.Size() - len);
    }

    static TString ReplaceSuffix(const TString& str, size_t removeLen, const char* add) {
        TString result = RemoveSuffix(str, removeLen);
        while (*add) {
            result.PushBack(*add);
            ++add;
        }
        return result;
    }

    TString Step1a(const TString& str) const {
        if (EndsWith(str, "sses")) return ReplaceSuffix(str, 4, "ss");
        if (EndsWith(str, "ies")) return ReplaceSuffix(str, 3, "i");
        if (EndsWith(str, "ss")) return str;
        if (EndsWith(str, "s")) return RemoveSuffix(str, 1);
        return str;
    }

    TString Step1b(const TString& str) const {
        if (EndsWith(str, "eed")) {
            TString stem = RemoveSuffix(str, 3);
            if (MeasureM(stem) > 0) return ReplaceSuffix(str, 3, "ee");
            return str;
        }
        
        TString result = str;
        bool flag = false;
        
        if (EndsWith(str, "ed")) {
            TString stem = RemoveSuffix(str, 2);
            if (HasVowel(stem)) {
                result = stem;
                flag = true;
            }
        } else if (EndsWith(str, "ing")) {
            TString stem = RemoveSuffix(str, 3);
            if (HasVowel(stem)) {
                result = stem;
                flag = true;
            }
        }
        
        if (flag) {
            if (EndsWith(result, "at")) return ReplaceSuffix(result, 2, "ate");
            if (EndsWith(result, "bl")) return ReplaceSuffix(result, 2, "ble");
            if (EndsWith(result, "iz")) return ReplaceSuffix(result, 2, "ize");
            if (EndsWithDoubleConsonant(result)) {
                char c = result[result.Size() - 1];
                if (c != 'l' && c != 's' && c != 'z') {
                    return RemoveSuffix(result, 1);
                }
            }
            if (MeasureM(result) == 1 && EndsCVC(result)) {
                result.PushBack('e');
            }
        }
        
        return result;
    }

    TString Step1c(const TString& str) const {
        if (EndsWith(str, "y")) {
            TString stem = RemoveSuffix(str, 1);
            if (HasVowel(stem)) return ReplaceSuffix(str, 1, "i");
        }
        return str;
    }

    TString Step2(const TString& str) const {
        for (size_t i = 0; NStep2::SUFFIXES[i].From; ++i) {
            if (EndsWith(str, NStep2::SUFFIXES[i].From)) {
                TString stem = RemoveSuffix(str, NStep2::SUFFIXES[i].Len);
                if (MeasureM(stem) > 0) {
                    return ReplaceSuffix(str, NStep2::SUFFIXES[i].Len, NStep2::SUFFIXES[i].To);
                }
                return str;
            }
        }
        return str;
    }

    TString Step3(const TString& str) const {
        for (size_t i = 0; NStep3::SUFFIXES[i].From; ++i) {
            if (EndsWith(str, NStep3::SUFFIXES[i].From)) {
                TString stem = RemoveSuffix(str, NStep3::SUFFIXES[i].Len);
                if (MeasureM(stem) > 0) {
                    return ReplaceSuffix(str, NStep3::SUFFIXES[i].Len, NStep3::SUFFIXES[i].To);
                }
                return str;
            }
        }
        return str;
    }

    TString Step4(const TString& str) const {
        for (size_t i = 0; NStep4::SUFFIXES[i]; ++i) {
            const char* suffix = NStep4::SUFFIXES[i];
            
            if (EndsWith(str, suffix)) {
                size_t slen = 0;
                while (suffix[slen]) ++slen;
                
                TString stem = RemoveSuffix(str, slen);
                
                if (suffix[0] == 'i' && suffix[1] == 'o' && suffix[2] == 'n' && suffix[3] == '\0') {
                    if (stem.Size() > 0) {
                        char c = stem[stem.Size() - 1];
                        if ((c == 's' || c == 't') && MeasureM(stem) > 1) {
                            return stem;
                        }
                    }
                } else {
                    if (MeasureM(stem) > 1) {
                        return stem;
                    }
                }
            }
        }
        return str;
    }
    

    TString Step5a(const TString& str) const {
        if (EndsWith(str, "e")) {
            TString stem = RemoveSuffix(str, 1);
            if (MeasureM(stem) > 1) return stem;
            if (MeasureM(stem) == 1 && !EndsCVC(stem)) return stem;
        }
        return str;
    }

    TString Step5b(const TString& str) const {
        if (MeasureM(str) > 1 && EndsWithDoubleConsonant(str) && EndsWith(str, "l")) {
            return RemoveSuffix(str, 1);
        }
        return str;
    }
};

/**
 * Лемматизатор на основе словаря неправильных форм + стемминга
 */
class TLemmatizer {
public:
    TLemmatizer() {
        InitDictionary();
    }

    TString Lemmatize(const TString& word) const {
        TString lower = ToLower(word);
        
        auto it = Dictionary_.Find(Hash(lower));
        if (it != Dictionary_.end() && it.Key() == Hash(lower)) {
            return GetLemma(lower);
        }
        
        return Stemmer_.Stem(lower);
    }

    TVector<TString> LemmatizeAll(const TVector<TString>& words) const {
        TVector<TString> result;
        result.Reserve(words.Size());
        for (size_t i = 0; i < words.Size(); ++i) {
            result.PushBack(Lemmatize(words[i]));
        }
        return result;
    }

    void AddWord(const TString& word, const TString& lemma) {
        Dictionary_.Insert(Hash(ToLower(word)), lemma);
    }

private:
    static TString ToLower(const TString& str) {
        TString result;
        result.Reserve(str.Size());
        for (size_t i = 0; i < str.Size(); ++i) {
            char c = str[i];
            if (c >= 'A' && c <= 'Z') c = c + ('a' - 'A');
            result.PushBack(c);
        }
        return result;
    }

    static size_t Hash(const TString& str) {
        return str.Hash();
    }

    TString GetLemma(const TString& word) const {
        auto it = Dictionary_.Find(Hash(word));
        if (it != Dictionary_.end()) {
            return it.Value();
        }
        return word;
    }

    void InitDictionary() {
        // Be, Have, Do, Go, Run
        for (size_t i = 0; NIrregularVerbs::BE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BE_FORMS[i].Word), TString(NIrregularVerbs::BE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::HAVE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::HAVE_FORMS[i].Word), TString(NIrregularVerbs::HAVE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::DO_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::DO_FORMS[i].Word), TString(NIrregularVerbs::DO_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::GO_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::GO_FORMS[i].Word), TString(NIrregularVerbs::GO_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::RUN_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::RUN_FORMS[i].Word), TString(NIrregularVerbs::RUN_FORMS[i].Lemma));
        }
        
        // Take, Give, See, Come group
        for (size_t i = 0; NIrregularVerbs::TAKE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::TAKE_FORMS[i].Word), TString(NIrregularVerbs::TAKE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::GIVE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::GIVE_FORMS[i].Word), TString(NIrregularVerbs::GIVE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SEE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SEE_FORMS[i].Word), TString(NIrregularVerbs::SEE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::COME_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::COME_FORMS[i].Word), TString(NIrregularVerbs::COME_FORMS[i].Lemma));
        }
        
        // Know, Make, Say, Tell, Think group
        for (size_t i = 0; NIrregularVerbs::KNOW_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::KNOW_FORMS[i].Word), TString(NIrregularVerbs::KNOW_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::MAKE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::MAKE_FORMS[i].Word), TString(NIrregularVerbs::MAKE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SAY_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SAY_FORMS[i].Word), TString(NIrregularVerbs::SAY_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::TELL_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::TELL_FORMS[i].Word), TString(NIrregularVerbs::TELL_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::THINK_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::THINK_FORMS[i].Word), TString(NIrregularVerbs::THINK_FORMS[i].Lemma));
        }
        
        // Find, Get, Leave, Feel group
        for (size_t i = 0; NIrregularVerbs::FIND_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::FIND_FORMS[i].Word), TString(NIrregularVerbs::FIND_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::GET_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::GET_FORMS[i].Word), TString(NIrregularVerbs::GET_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::LEAVE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::LEAVE_FORMS[i].Word), TString(NIrregularVerbs::LEAVE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::FEEL_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::FEEL_FORMS[i].Word), TString(NIrregularVerbs::FEEL_FORMS[i].Lemma));
        }
        
        // Bring, Buy, Catch, Teach, Seek group
        for (size_t i = 0; NIrregularVerbs::BRING_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BRING_FORMS[i].Word), TString(NIrregularVerbs::BRING_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BUY_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BUY_FORMS[i].Word), TString(NIrregularVerbs::BUY_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::CATCH_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::CATCH_FORMS[i].Word), TString(NIrregularVerbs::CATCH_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::TEACH_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::TEACH_FORMS[i].Word), TString(NIrregularVerbs::TEACH_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SEEK_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SEEK_FORMS[i].Word), TString(NIrregularVerbs::SEEK_FORMS[i].Lemma));
        }
        
        // Write, Speak, Break, Choose group
        for (size_t i = 0; NIrregularVerbs::WRITE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::WRITE_FORMS[i].Word), TString(NIrregularVerbs::WRITE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SPEAK_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SPEAK_FORMS[i].Word), TString(NIrregularVerbs::SPEAK_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BREAK_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BREAK_FORMS[i].Word), TString(NIrregularVerbs::BREAK_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::CHOOSE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::CHOOSE_FORMS[i].Word), TString(NIrregularVerbs::CHOOSE_FORMS[i].Lemma));
        }
        
        // Drive, Ride, Rise, Fly group
        for (size_t i = 0; NIrregularVerbs::DRIVE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::DRIVE_FORMS[i].Word), TString(NIrregularVerbs::DRIVE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::RIDE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::RIDE_FORMS[i].Word), TString(NIrregularVerbs::RIDE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::RISE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::RISE_FORMS[i].Word), TString(NIrregularVerbs::RISE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::FLY_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::FLY_FORMS[i].Word), TString(NIrregularVerbs::FLY_FORMS[i].Lemma));
        }
        
        // Grow, Throw, Draw group
        for (size_t i = 0; NIrregularVerbs::GROW_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::GROW_FORMS[i].Word), TString(NIrregularVerbs::GROW_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::THROW_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::THROW_FORMS[i].Word), TString(NIrregularVerbs::THROW_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::DRAW_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::DRAW_FORMS[i].Word), TString(NIrregularVerbs::DRAW_FORMS[i].Lemma));
        }
        
        // Sing, Swim, Begin, Drink, Ring group
        for (size_t i = 0; NIrregularVerbs::SING_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SING_FORMS[i].Word), TString(NIrregularVerbs::SING_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SWIM_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SWIM_FORMS[i].Word), TString(NIrregularVerbs::SWIM_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BEGIN_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BEGIN_FORMS[i].Word), TString(NIrregularVerbs::BEGIN_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::DRINK_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::DRINK_FORMS[i].Word), TString(NIrregularVerbs::DRINK_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::RING_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::RING_FORMS[i].Word), TString(NIrregularVerbs::RING_FORMS[i].Lemma));
        }
        
        // Sit, Stand, Hold, Read, Lead group
        for (size_t i = 0; NIrregularVerbs::SIT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SIT_FORMS[i].Word), TString(NIrregularVerbs::SIT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::STAND_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::STAND_FORMS[i].Word), TString(NIrregularVerbs::STAND_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::HOLD_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::HOLD_FORMS[i].Word), TString(NIrregularVerbs::HOLD_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::READ_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::READ_FORMS[i].Word), TString(NIrregularVerbs::READ_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::LEAD_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::LEAD_FORMS[i].Word), TString(NIrregularVerbs::LEAD_FORMS[i].Lemma));
        }
        
        // Meet, Pay, Send, Spend group
        for (size_t i = 0; NIrregularVerbs::MEET_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::MEET_FORMS[i].Word), TString(NIrregularVerbs::MEET_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::PAY_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::PAY_FORMS[i].Word), TString(NIrregularVerbs::PAY_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SEND_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SEND_FORMS[i].Word), TString(NIrregularVerbs::SEND_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SPEND_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SPEND_FORMS[i].Word), TString(NIrregularVerbs::SPEND_FORMS[i].Lemma));
        }
        
        // Build, Lose, Keep, Sleep, Win, Wear group
        for (size_t i = 0; NIrregularVerbs::BUILD_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BUILD_FORMS[i].Word), TString(NIrregularVerbs::BUILD_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::LOSE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::LOSE_FORMS[i].Word), TString(NIrregularVerbs::LOSE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::KEEP_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::KEEP_FORMS[i].Word), TString(NIrregularVerbs::KEEP_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SLEEP_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SLEEP_FORMS[i].Word), TString(NIrregularVerbs::SLEEP_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::WIN_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::WIN_FORMS[i].Word), TString(NIrregularVerbs::WIN_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::WEAR_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::WEAR_FORMS[i].Word), TString(NIrregularVerbs::WEAR_FORMS[i].Lemma));
        }
        
        // Beat, Bite, Bind, Bleed, Blow, Bear group
        for (size_t i = 0; NIrregularVerbs::BEAT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BEAT_FORMS[i].Word), TString(NIrregularVerbs::BEAT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BITE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BITE_FORMS[i].Word), TString(NIrregularVerbs::BITE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BIND_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BIND_FORMS[i].Word), TString(NIrregularVerbs::BIND_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BLEED_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BLEED_FORMS[i].Word), TString(NIrregularVerbs::BLEED_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BLOW_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BLOW_FORMS[i].Word), TString(NIrregularVerbs::BLOW_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BEAR_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BEAR_FORMS[i].Word), TString(NIrregularVerbs::BEAR_FORMS[i].Lemma));
        }
        
        // Eat, Fall, Hide, Shake, Freeze group
        for (size_t i = 0; NIrregularVerbs::EAT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::EAT_FORMS[i].Word), TString(NIrregularVerbs::EAT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::FALL_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::FALL_FORMS[i].Word), TString(NIrregularVerbs::FALL_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::HIDE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::HIDE_FORMS[i].Word), TString(NIrregularVerbs::HIDE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SHAKE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SHAKE_FORMS[i].Word), TString(NIrregularVerbs::SHAKE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::FREEZE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::FREEZE_FORMS[i].Word), TString(NIrregularVerbs::FREEZE_FORMS[i].Lemma));
        }
        
        // Steal, Tear, Weave, Forbid, Forgive group
        for (size_t i = 0; NIrregularVerbs::STEAL_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::STEAL_FORMS[i].Word), TString(NIrregularVerbs::STEAL_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::TEAR_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::TEAR_FORMS[i].Word), TString(NIrregularVerbs::TEAR_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::WEAVE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::WEAVE_FORMS[i].Word), TString(NIrregularVerbs::WEAVE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::FORBID_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::FORBID_FORMS[i].Word), TString(NIrregularVerbs::FORBID_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::FORGIVE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::FORGIVE_FORMS[i].Word), TString(NIrregularVerbs::FORGIVE_FORMS[i].Lemma));
        }
        
        // Lie, Lay, Shine, Shoot, Show group
        for (size_t i = 0; NIrregularVerbs::LIE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::LIE_FORMS[i].Word), TString(NIrregularVerbs::LIE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::LAY_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::LAY_FORMS[i].Word), TString(NIrregularVerbs::LAY_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SHINE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SHINE_FORMS[i].Word), TString(NIrregularVerbs::SHINE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SHOOT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SHOOT_FORMS[i].Word), TString(NIrregularVerbs::SHOOT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SHOW_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SHOW_FORMS[i].Word), TString(NIrregularVerbs::SHOW_FORMS[i].Lemma));
        }
        
        // Shrink, Shut, Slay, Slide group
        for (size_t i = 0; NIrregularVerbs::SHRINK_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SHRINK_FORMS[i].Word), TString(NIrregularVerbs::SHRINK_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SHUT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SHUT_FORMS[i].Word), TString(NIrregularVerbs::SHUT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SLAY_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SLAY_FORMS[i].Word), TString(NIrregularVerbs::SLAY_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SLIDE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SLIDE_FORMS[i].Word), TString(NIrregularVerbs::SLIDE_FORMS[i].Lemma));
        }
        
        // Sling, Slit, Smite, Sow group
        for (size_t i = 0; NIrregularVerbs::SLING_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SLING_FORMS[i].Word), TString(NIrregularVerbs::SLING_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SLIT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SLIT_FORMS[i].Word), TString(NIrregularVerbs::SLIT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SMITE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SMITE_FORMS[i].Word), TString(NIrregularVerbs::SMITE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SOW_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SOW_FORMS[i].Word), TString(NIrregularVerbs::SOW_FORMS[i].Lemma));
        }
        
        // Spin, Spit, Split, Spread group
        for (size_t i = 0; NIrregularVerbs::SPIN_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SPIN_FORMS[i].Word), TString(NIrregularVerbs::SPIN_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SPIT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SPIT_FORMS[i].Word), TString(NIrregularVerbs::SPIT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SPLIT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SPLIT_FORMS[i].Word), TString(NIrregularVerbs::SPLIT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SPREAD_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SPREAD_FORMS[i].Word), TString(NIrregularVerbs::SPREAD_FORMS[i].Lemma));
        }
        
        // Spring, Stick, Sting, Stink group
        for (size_t i = 0; NIrregularVerbs::SPRING_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SPRING_FORMS[i].Word), TString(NIrregularVerbs::SPRING_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::STICK_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::STICK_FORMS[i].Word), TString(NIrregularVerbs::STICK_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::STING_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::STING_FORMS[i].Word), TString(NIrregularVerbs::STING_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::STINK_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::STINK_FORMS[i].Word), TString(NIrregularVerbs::STINK_FORMS[i].Lemma));
        }
        
        // Stride, Strike, String, Strive group
        for (size_t i = 0; NIrregularVerbs::STRIDE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::STRIDE_FORMS[i].Word), TString(NIrregularVerbs::STRIDE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::STRIKE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::STRIKE_FORMS[i].Word), TString(NIrregularVerbs::STRIKE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::STRING_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::STRING_FORMS[i].Word), TString(NIrregularVerbs::STRING_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::STRIVE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::STRIVE_FORMS[i].Word), TString(NIrregularVerbs::STRIVE_FORMS[i].Lemma));
        }
        
        // Swear, Sweep, Swell, Swing group
        for (size_t i = 0; NIrregularVerbs::SWEAR_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SWEAR_FORMS[i].Word), TString(NIrregularVerbs::SWEAR_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SWEEP_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SWEEP_FORMS[i].Word), TString(NIrregularVerbs::SWEEP_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SWELL_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SWELL_FORMS[i].Word), TString(NIrregularVerbs::SWELL_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SWING_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SWING_FORMS[i].Word), TString(NIrregularVerbs::SWING_FORMS[i].Lemma));
        }
        
        // Tread, Wake, Wind, Wring group
        for (size_t i = 0; NIrregularVerbs::TREAD_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::TREAD_FORMS[i].Word), TString(NIrregularVerbs::TREAD_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::WAKE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::WAKE_FORMS[i].Word), TString(NIrregularVerbs::WAKE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::WIND_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::WIND_FORMS[i].Word), TString(NIrregularVerbs::WIND_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::WRING_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::WRING_FORMS[i].Word), TString(NIrregularVerbs::WRING_FORMS[i].Lemma));
        }
        
        for (size_t i = 0; NIrregularVerbs::LIGHT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::LIGHT_FORMS[i].Word), TString(NIrregularVerbs::LIGHT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::QUIT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::QUIT_FORMS[i].Word), TString(NIrregularVerbs::QUIT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SET_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SET_FORMS[i].Word), TString(NIrregularVerbs::SET_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::CUT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::CUT_FORMS[i].Word), TString(NIrregularVerbs::CUT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::HIT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::HIT_FORMS[i].Word), TString(NIrregularVerbs::HIT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::PUT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::PUT_FORMS[i].Word), TString(NIrregularVerbs::PUT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::LET_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::LET_FORMS[i].Word), TString(NIrregularVerbs::LET_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::COST_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::COST_FORMS[i].Word), TString(NIrregularVerbs::COST_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::CAST_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::CAST_FORMS[i].Word), TString(NIrregularVerbs::CAST_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BURST_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BURST_FORMS[i].Word), TString(NIrregularVerbs::BURST_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::HURT_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::HURT_FORMS[i].Word), TString(NIrregularVerbs::HURT_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BET_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BET_FORMS[i].Word), TString(NIrregularVerbs::BET_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BEND_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BEND_FORMS[i].Word), TString(NIrregularVerbs::BEND_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::LEND_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::LEND_FORMS[i].Word), TString(NIrregularVerbs::LEND_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::FEED_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::FEED_FORMS[i].Word), TString(NIrregularVerbs::FEED_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BREED_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BREED_FORMS[i].Word), TString(NIrregularVerbs::BREED_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SPEED_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SPEED_FORMS[i].Word), TString(NIrregularVerbs::SPEED_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::FLEE_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::FLEE_FORMS[i].Word), TString(NIrregularVerbs::FLEE_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::DEAL_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::DEAL_FORMS[i].Word), TString(NIrregularVerbs::DEAL_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::MEAN_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::MEAN_FORMS[i].Word), TString(NIrregularVerbs::MEAN_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::LEAN_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::LEAN_FORMS[i].Word), TString(NIrregularVerbs::LEAN_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::LEAP_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::LEAP_FORMS[i].Word), TString(NIrregularVerbs::LEAP_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::LEARN_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::LEARN_FORMS[i].Word), TString(NIrregularVerbs::LEARN_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::BURN_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::BURN_FORMS[i].Word), TString(NIrregularVerbs::BURN_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SMELL_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SMELL_FORMS[i].Word), TString(NIrregularVerbs::SMELL_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SPELL_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SPELL_FORMS[i].Word), TString(NIrregularVerbs::SPELL_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SPILL_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SPILL_FORMS[i].Word), TString(NIrregularVerbs::SPILL_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::SPOIL_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::SPOIL_FORMS[i].Word), TString(NIrregularVerbs::SPOIL_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::DREAM_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::DREAM_FORMS[i].Word), TString(NIrregularVerbs::DREAM_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::DWELL_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::DWELL_FORMS[i].Word), TString(NIrregularVerbs::DWELL_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::HANG_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::HANG_FORMS[i].Word), TString(NIrregularVerbs::HANG_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::DIG_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::DIG_FORMS[i].Word), TString(NIrregularVerbs::DIG_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::CLING_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::CLING_FORMS[i].Word), TString(NIrregularVerbs::CLING_FORMS[i].Lemma));
        }
        for (size_t i = 0; NIrregularVerbs::FLING_FORMS[i].Word; ++i) {
            AddWord(TString(NIrregularVerbs::FLING_FORMS[i].Word), TString(NIrregularVerbs::FLING_FORMS[i].Lemma));
        }
        
        // Adverbs
        for (size_t i = 0; NIrregularAdverbs::ADVERBS[i].Form; ++i) {
            AddWord(TString(NIrregularAdverbs::ADVERBS[i].Form), TString(NIrregularAdverbs::ADVERBS[i].Base));
        }
        
        // Nouns
        for (size_t i = 0; NIrregularNouns::NOUNS[i].Plural; ++i) {
            AddWord(TString(NIrregularNouns::NOUNS[i].Plural), TString(NIrregularNouns::NOUNS[i].Singular));
        }
        
        // Adjectives
        for (size_t i = 0; NIrregularAdjectives::ADJECTIVES[i].Form; ++i) {
            AddWord(TString(NIrregularAdjectives::ADJECTIVES[i].Form), TString(NIrregularAdjectives::ADJECTIVES[i].Base));
        }
    }


    TUnorderedMap<size_t, TString> Dictionary_;
    TPorterStemmer Stemmer_;
};

} // namespace NStemmer

