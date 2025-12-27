#include <lib/stemmer/stemmer.h>
#include <gtest/gtest.h>

using namespace NStemmer;
using NTypes::TString;
using NCollections::TVector;

TEST(TPorterStemmer, BasicStemming) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("running")), TString("run"));
    EXPECT_EQ(stemmer.Stem(TString("jumps")), TString("jump"));
    EXPECT_EQ(stemmer.Stem(TString("easily")), TString("easili"));
}

TEST(TPorterStemmer, PluralForms) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("cats")), TString("cat"));
    EXPECT_EQ(stemmer.Stem(TString("dogs")), TString("dog"));
    EXPECT_EQ(stemmer.Stem(TString("houses")), TString("hous"));
    EXPECT_EQ(stemmer.Stem(TString("ponies")), TString("poni"));
    EXPECT_EQ(stemmer.Stem(TString("caresses")), TString("caress"));
}

TEST(TPorterStemmer, PastTense) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("walked")), TString("walk"));
    EXPECT_EQ(stemmer.Stem(TString("computed")), TString("comput"));
}

TEST(TPorterStemmer, IngForm) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("walking")), TString("walk"));
    EXPECT_EQ(stemmer.Stem(TString("singing")), TString("sing"));
}

TEST(TPorterStemmer, ShortWords) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("a")), TString("a"));
    EXPECT_EQ(stemmer.Stem(TString("an")), TString("an"));
    EXPECT_EQ(stemmer.Stem(TString("the")), TString("the"));
}

TEST(TPorterStemmer, Step2Suffixes) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("relational")), TString("relat"));
    EXPECT_EQ(stemmer.Stem(TString("conditional")), TString("condit"));
    EXPECT_EQ(stemmer.Stem(TString("rational")), TString("ration"));
}

TEST(TPorterStemmer, Step3Suffixes) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("triplicate")), TString("triplic"));
    EXPECT_EQ(stemmer.Stem(TString("formative")), TString("form"));
    EXPECT_EQ(stemmer.Stem(TString("formalize")), TString("formal"));
}

TEST(TPorterStemmer, StemAll) {
    TPorterStemmer stemmer;
    
    TVector<TString> words;
    words.PushBack(TString("running"));
    words.PushBack(TString("jumping"));
    words.PushBack(TString("swimming"));
    
    TVector<TString> stems = stemmer.StemAll(words);
    
    ASSERT_EQ(stems.Size(), 3);
    EXPECT_EQ(stems[0], TString("run"));
    EXPECT_EQ(stems[1], TString("jump"));
    EXPECT_EQ(stems[2], TString("swim"));
}

TEST(TPorterStemmer, CaseInsensitive) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("RUNNING")), TString("run"));
    EXPECT_EQ(stemmer.Stem(TString("Running")), TString("run"));
    EXPECT_EQ(stemmer.Stem(TString("running")), TString("run"));
}

TEST(TLemmatizer, IrregularVerbs) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("am")), TString("be"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("is")), TString("be"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("are")), TString("be"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("was")), TString("be"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("were")), TString("be"));
}

TEST(TLemmatizer, IrregularNouns) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("children")), TString("child"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("men")), TString("man"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("women")), TString("woman"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("feet")), TString("foot"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("teeth")), TString("tooth"));
}

TEST(TLemmatizer, IrregularAdjectives) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("better")), TString("good"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("best")), TString("good"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("worse")), TString("bad"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("worst")), TString("bad"));
}

TEST(TLemmatizer, RegularWords) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("running")), TString("run"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("cats")), TString("cat"));
}

TEST(TLemmatizer, LemmatizeAll) {
    TLemmatizer lemmatizer;
    
    TVector<TString> words;
    words.PushBack(TString("children"));
    words.PushBack(TString("are"));
    words.PushBack(TString("running"));
    
    TVector<TString> lemmas = lemmatizer.LemmatizeAll(words);
    
    ASSERT_EQ(lemmas.Size(), 3);
    EXPECT_EQ(lemmas[0], TString("child"));
    EXPECT_EQ(lemmas[1], TString("be"));
    EXPECT_EQ(lemmas[2], TString("run"));
}

TEST(TLemmatizer, CaseInsensitive) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("AM")), TString("be"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("Children")), TString("child"));
}

TEST(TLemmatizer, CustomDictionary) {
    TLemmatizer lemmatizer;
    
    lemmatizer.AddWord("customword", "custom");
    EXPECT_EQ(lemmatizer.Lemmatize(TString("customword")), TString("custom"));
}

TEST(TPorterStemmer, DoubleConsonants) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("hopping")), TString("hop"));
    EXPECT_EQ(stemmer.Stem(TString("hopped")), TString("hop"));
    EXPECT_EQ(stemmer.Stem(TString("hoping")), TString("hope"));
}

TEST(TPorterStemmer, ComplexSuffixes) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("complexity")), TString("complex"));
    EXPECT_EQ(stemmer.Stem(TString("electrical")), TString("electric"));
    EXPECT_EQ(stemmer.Stem(TString("hopeful")), TString("hope"));
    EXPECT_EQ(stemmer.Stem(TString("goodness")), TString("good"));
}

TEST(TPorterStemmer, Step4Removal) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("revival")), TString("reviv"));
    EXPECT_EQ(stemmer.Stem(TString("allowance")), TString("allowanc"));
    EXPECT_EQ(stemmer.Stem(TString("inference")), TString("inferenc"));
    EXPECT_EQ(stemmer.Stem(TString("airliner")), TString("airliner"));
    EXPECT_EQ(stemmer.Stem(TString("gyroscopic")), TString("gyroscop"));
}


TEST(TPorterStemmer, Step5Processing) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("probate")), TString("probat"));
    EXPECT_EQ(stemmer.Stem(TString("rate")), TString("rate"));
    EXPECT_EQ(stemmer.Stem(TString("cease")), TString("ceas"));
}

TEST(TPorterStemmer, YEnding) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("happy")), TString("happi"));
    EXPECT_EQ(stemmer.Stem(TString("sky")), TString("sky"));
}

TEST(TPorterStemmer, EdgeCases) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("agree")), TString("agre"));
    EXPECT_EQ(stemmer.Stem(TString("agreed")), TString("agreed"));
    EXPECT_EQ(stemmer.Stem(TString("disabled")), TString("disabl"));
    EXPECT_EQ(stemmer.Stem(TString("sized")), TString("size"));
}

TEST(TPorterStemmer, LongWords) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("internationalization")), TString("internation"));
    EXPECT_EQ(stemmer.Stem(TString("responsibilities")), TString("respons"));
    EXPECT_EQ(stemmer.Stem(TString("characterization")), TString("character"));
}

TEST(TLemmatizer, ModalVerbs) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("went")), TString("go"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("took")), TString("take"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("gave")), TString("give"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("saw")), TString("see"));
}

TEST(TLemmatizer, PastParticiples) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("taken")), TString("take"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("given")), TString("give"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("seen")), TString("see"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("written")), TString("write"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("spoken")), TString("speak"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("broken")), TString("break"));
}

TEST(TLemmatizer, LatinPluralNouns) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("data")), TString("datum"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("criteria")), TString("criterion"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("phenomena")), TString("phenomenon"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("bacteria")), TString("bacterium"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("curricula")), TString("curriculum"));
}

TEST(TLemmatizer, GreekPluralNouns) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("analyses")), TString("analysis"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("hypotheses")), TString("hypothesis"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("theses")), TString("thesis"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("parentheses")), TString("parenthesis"));
}

TEST(TLemmatizer, IrregularPluralAnimals) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("mice")), TString("mouse"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("geese")), TString("goose"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("oxen")), TString("ox"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("lice")), TString("louse"));
}

TEST(TLemmatizer, UnchangedPlurals) {
    TLemmatizer lemmatizer;
    
    TString sheep = TString("sheep");
    TString deer = TString("deer");
    TString fish = TString("fish");
    
    EXPECT_TRUE(lemmatizer.Lemmatize(sheep) == sheep || 
                lemmatizer.Lemmatize(sheep) == TString("sheep"));
}

TEST(TLemmatizer, CompoundVerbs) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("understood")), TString("understand"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("overthrown")), TString("overthrow"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("withdrawn")), TString("withdraw"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("undertaken")), TString("undertake"));
}

TEST(TLemmatizer, StrongVerbs) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("sang")), TString("sing"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("swam")), TString("swim"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("rang")), TString("ring"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("drank")), TString("drink"));
}

TEST(TLemmatizer, WeakVerbs) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("kept")), TString("keep"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("slept")), TString("sleep"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("meant")), TString("mean"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("dealt")), TString("deal"));
}

TEST(TLemmatizer, MixedForms) {
    TLemmatizer lemmatizer;
    
    TVector<TString> words;
    words.PushBack(TString("children"));
    words.PushBack(TString("were"));
    words.PushBack(TString("swimming"));
    words.PushBack(TString("faster"));
    words.PushBack(TString("analyses"));
    
    TVector<TString> lemmas = lemmatizer.LemmatizeAll(words);
    
    ASSERT_EQ(lemmas.Size(), 5);
    EXPECT_EQ(lemmas[0], TString("child"));
    EXPECT_EQ(lemmas[1], TString("be"));
}

TEST(TLemmatizer, CapitalizationPreservation) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("CHILDREN")), TString("child"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("Children")), TString("child"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("ChIlDrEn")), TString("child"));
}

TEST(TLemmatizer, EmptyAndShortInputs) {
    TLemmatizer lemmatizer;
    
    EXPECT_EQ(lemmatizer.Lemmatize(TString("a")), TString("a"));
    EXPECT_EQ(lemmatizer.Lemmatize(TString("I")), TString("i"));
}

TEST(TPorterStemmer, SpecificPorterExamples) {
    TPorterStemmer stemmer;
    
    EXPECT_EQ(stemmer.Stem(TString("consign")), TString("consign"));
    EXPECT_EQ(stemmer.Stem(TString("consigned")), TString("consign"));
    EXPECT_EQ(stemmer.Stem(TString("consigning")), TString("consign"));
    EXPECT_EQ(stemmer.Stem(TString("consignment")), TString("consign"));
}

TEST(TLemmatizer, MultipleWordProcessing) {
    TLemmatizer lemmatizer;
    
    TVector<TString> words;
    for (int i = 0; i < 100; ++i) {
        words.PushBack(TString("running"));
    }
    
    TVector<TString> lemmas = lemmatizer.LemmatizeAll(words);
    EXPECT_EQ(lemmas.Size(), 100);
    
    for (size_t i = 0; i < lemmas.Size(); ++i) {
        EXPECT_EQ(lemmas[i], TString("run"));
    }
}

