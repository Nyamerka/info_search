#pragma once


namespace NStemmer {


struct TSuffixRule {
    const char* From;
    const char* To;
    size_t Len;
};


namespace NStep1 {
    static const TSuffixRule SUFFIXES[] = {
        {"sses", "ss", 4},
        {"ies", "i", 3},
        {"ss", "ss", 2},
        {"s", "", 1},
        {"eed", "ee", 3},
        {"ed", "", 2},
        {"ing", "", 3},
        {nullptr, nullptr, 0}
    };
}


namespace NStep2 {
    static const TSuffixRule SUFFIXES[] = {
        {"ational", "ate", 7},
        {"tional", "tion", 6},
        {"enci", "ence", 4},
        {"anci", "ance", 4},
        {"izer", "ize", 4},
        {"abli", "able", 4},
        {"alli", "al", 4},
        {"entli", "ent", 5},
        {"eli", "e", 3},
        {"ousli", "ous", 5},
        {"ization", "ize", 7},
        {"ation", "ate", 5},
        {"ator", "ate", 4},
        {"alism", "al", 5},
        {"iveness", "ive", 7},
        {"fulness", "ful", 7},
        {"ousness", "ous", 7},
        {"aliti", "al", 5},
        {"iviti", "ive", 5},
        {"biliti", "ble", 6},
        {"logi", "log", 4},
        {"fulli", "ful", 5},
        {"lessli", "less", 6},
        {nullptr, nullptr, 0}
    };
}


namespace NStep3 {
    static const TSuffixRule SUFFIXES[] = {
        {"icate", "ic", 5},
        {"ative", "", 5},
        {"alize", "al", 5},
        {"iciti", "ic", 5},
        {"ical", "ic", 4},
        {"ful", "", 3},
        {"ness", "", 4},
        {nullptr, nullptr, 0}
    };
}


namespace NStep4 {
    static const char* SUFFIXES[] = {
        "ement",
        "ance",
        "ence",
        "able",
        "ible",
        "ment",
        "ant",
        "ent",
        "ion",
        "ism",
        "ate",
        "iti",
        "ous",
        "ive",
        "ize",
        "al",
        "er",
        "ic",
        "ou",
        nullptr
    };
}


namespace NPrefixes {
    static const char* PREFIXES[] = {
        "un",
        "re",
        "in",
        "im",
        "dis",
        "en",
        "non",
        "over",
        "mis",
        "sub",
        "pre",
        "inter",
        "fore",
        "de",
        "trans",
        "super",
        "semi",
        "anti",
        "mid",
        "under",
        nullptr
    };
}


namespace NIrregularVerbs {
    struct TIrregularWord {
        const char* Word;
        const char* Lemma;
    };


    static const TIrregularWord BE_FORMS[] = {
        {"am", "be"},
        {"are", "be"},
        {"is", "be"},
        {"was", "be"},
        {"were", "be"},
        {"been", "be"},
        {"being", "be"},
        {nullptr, nullptr}
    };


    static const TIrregularWord HAVE_FORMS[] = {
        {"have", "have"},
        {"has", "have"},
        {"had", "have"},
        {"having", "have"},
        {nullptr, nullptr}
    };


    static const TIrregularWord DO_FORMS[] = {
        {"do", "do"},
        {"does", "do"},
        {"did", "do"},
        {"doing", "do"},
        {"done", "do"},
        {nullptr, nullptr}
    };


    static const TIrregularWord GO_FORMS[] = {
        {"go", "go"},
        {"goes", "go"},
        {"went", "go"},
        {"going", "go"},
        {"gone", "go"},
        {"undergo", "undergo"},
        {"underwent", "undergo"},
        {"undergone", "undergo"},
        {"forgo", "forgo"},
        {"forwent", "forgo"},
        {"forgone", "forgo"},
        {nullptr, nullptr}
    };


    static const TIrregularWord RUN_FORMS[] = {
        {"ran", "run"},
        {"running", "run"},
        {"runs", "run"},
        {"overran", "overrun"},
        {"overrun", "overrun"},
        {"reran", "rerun"},
        {"rerun", "rerun"},
        {nullptr, nullptr}
    };


    static const TIrregularWord TAKE_FORMS[] = {
        {"took", "take"},
        {"taken", "take"},
        {"taking", "take"},
        {"takes", "take"},
        {"undertook", "undertake"},
        {"undertaken", "undertake"},
        {"mistook", "mistake"},
        {"mistaken", "mistake"},
        {"overtook", "overtake"},
        {"overtaken", "overtake"},
        {"retook", "retake"},
        {"retaken", "retake"},
        {nullptr, nullptr}
    };


    static const TIrregularWord GIVE_FORMS[] = {
        {"gave", "give"},
        {"given", "give"},
        {"giving", "give"},
        {"gives", "give"},
        {"forgave", "forgive"},
        {"forgiven", "forgive"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SEE_FORMS[] = {
        {"saw", "see"},
        {"seen", "see"},
        {"seeing", "see"},
        {"sees", "see"},
        {"foresee", "foresee"},
        {"foresaw", "foresee"},
        {"foreseen", "foresee"},
        {"oversee", "oversee"},
        {"oversaw", "oversee"},
        {"overseen", "oversee"},
        {nullptr, nullptr}
    };


    static const TIrregularWord COME_FORMS[] = {
        {"came", "come"},
        {"coming", "come"},
        {"comes", "come"},
        {"become", "become"},
        {"became", "become"},
        {"overcome", "overcome"},
        {"overcame", "overcome"},
        {nullptr, nullptr}
    };


    static const TIrregularWord KNOW_FORMS[] = {
        {"knew", "know"},
        {"known", "know"},
        {"knowing", "know"},
        {"knows", "know"},
        {nullptr, nullptr}
    };


    static const TIrregularWord MAKE_FORMS[] = {
        {"made", "make"},
        {"making", "make"},
        {"makes", "make"},
        {"remake", "remake"},
        {"remade", "remake"},
        {"unmake", "unmake"},
        {"unmade", "unmake"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SAY_FORMS[] = {
        {"said", "say"},
        {"saying", "say"},
        {"says", "say"},
        {nullptr, nullptr}
    };


    static const TIrregularWord TELL_FORMS[] = {
        {"told", "tell"},
        {"telling", "tell"},
        {"tells", "tell"},
        {"retell", "retell"},
        {"retold", "retell"},
        {"foretell", "foretell"},
        {"foretold", "foretell"},
        {nullptr, nullptr}
    };


    static const TIrregularWord THINK_FORMS[] = {
        {"thought", "think"},
        {"thinking", "think"},
        {"thinks", "think"},
        {"rethought", "rethink"},
        {nullptr, nullptr}
    };


    static const TIrregularWord FIND_FORMS[] = {
        {"found", "find"},
        {"finding", "find"},
        {"finds", "find"},
        {nullptr, nullptr}
    };


    static const TIrregularWord GET_FORMS[] = {
        {"got", "get"},
        {"gotten", "get"},
        {"getting", "get"},
        {"gets", "get"},
        {"forget", "forget"},
        {"forgot", "forget"},
        {"forgotten", "forget"},
        {"beget", "beget"},
        {"begot", "beget"},
        {"begotten", "beget"},
        {nullptr, nullptr}
    };


    static const TIrregularWord LEAVE_FORMS[] = {
        {"left", "leave"},
        {"leaving", "leave"},
        {"leaves", "leave"},
        {nullptr, nullptr}
    };


    static const TIrregularWord FEEL_FORMS[] = {
        {"felt", "feel"},
        {"feeling", "feel"},
        {"feels", "feel"},
        {nullptr, nullptr}
    };


    static const TIrregularWord BRING_FORMS[] = {
        {"brought", "bring"},
        {"bringing", "bring"},
        {"brings", "bring"},
        {nullptr, nullptr}
    };


    static const TIrregularWord BUY_FORMS[] = {
        {"bought", "buy"},
        {"buying", "buy"},
        {"buys", "buy"},
        {nullptr, nullptr}
    };


    static const TIrregularWord CATCH_FORMS[] = {
        {"caught", "catch"},
        {"catching", "catch"},
        {"catches", "catch"},
        {nullptr, nullptr}
    };


    static const TIrregularWord TEACH_FORMS[] = {
        {"taught", "teach"},
        {"teaching", "teach"},
        {"teaches", "teach"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SEEK_FORMS[] = {
        {"sought", "seek"},
        {"seeking", "seek"},
        {"seeks", "seek"},
        {nullptr, nullptr}
    };


    static const TIrregularWord WRITE_FORMS[] = {
        {"wrote", "write"},
        {"written", "write"},
        {"writing", "write"},
        {"writes", "write"},
        {"rewrite", "rewrite"},
        {"rewrote", "rewrite"},
        {"rewritten", "rewrite"},
        {"overwrite", "overwrite"},
        {"overwrote", "overwrite"},
        {"overwritten", "overwrite"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SPEAK_FORMS[] = {
        {"spoke", "speak"},
        {"spoken", "speak"},
        {"speaking", "speak"},
        {"speaks", "speak"},
        {nullptr, nullptr}
    };


    static const TIrregularWord BREAK_FORMS[] = {
        {"broke", "break"},
        {"broken", "break"},
        {"breaking", "break"},
        {"breaks", "break"},
        {"outbreak", "outbreak"},
        {"outbroke", "outbreak"},
        {"outbroken", "outbreak"},
        {nullptr, nullptr}
    };


    static const TIrregularWord CHOOSE_FORMS[] = {
        {"chose", "choose"},
        {"chosen", "choose"},
        {"choosing", "choose"},
        {"chooses", "choose"},
        {nullptr, nullptr}
    };


    static const TIrregularWord DRIVE_FORMS[] = {
        {"drove", "drive"},
        {"driven", "drive"},
        {"driving", "drive"},
        {"drives", "drive"},
        {"overdrive", "overdrive"},
        {"overdrove", "overdrive"},
        {"overdriven", "overdrive"},
        {nullptr, nullptr}
    };


    static const TIrregularWord RIDE_FORMS[] = {
        {"rode", "ride"},
        {"ridden", "ride"},
        {"riding", "ride"},
        {"rides", "ride"},
        {"override", "override"},
        {"overrode", "override"},
        {"overridden", "override"},
        {nullptr, nullptr}
    };


    static const TIrregularWord RISE_FORMS[] = {
        {"rose", "rise"},
        {"risen", "rise"},
        {"rising", "rise"},
        {"rises", "rise"},
        {"arise", "arise"},
        {"arose", "arise"},
        {"arisen", "arise"},
        {nullptr, nullptr}
    };


    static const TIrregularWord FLY_FORMS[] = {
        {"flew", "fly"},
        {"flown", "fly"},
        {"flying", "fly"},
        {"flies", "fly"},
        {"overfly", "overfly"},
        {"overflew", "overfly"},
        {"overflown", "overfly"},
        {nullptr, nullptr}
    };


    static const TIrregularWord GROW_FORMS[] = {
        {"grew", "grow"},
        {"grown", "grow"},
        {"growing", "grow"},
        {"grows", "grow"},
        {"outgrow", "outgrow"},
        {"outgrew", "outgrow"},
        {"outgrown", "outgrow"},
        {nullptr, nullptr}
    };


    static const TIrregularWord THROW_FORMS[] = {
        {"threw", "throw"},
        {"thrown", "throw"},
        {"throwing", "throw"},
        {"throws", "throw"},
        {"overthrow", "overthrow"},
        {"overthrew", "overthrow"},
        {"overthrown", "overthrow"},
        {nullptr, nullptr}
    };


    static const TIrregularWord DRAW_FORMS[] = {
        {"drew", "draw"},
        {"drawn", "draw"},
        {"drawing", "draw"},
        {"draws", "draw"},
        {"withdraw", "withdraw"},
        {"withdrew", "withdraw"},
        {"withdrawn", "withdraw"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SING_FORMS[] = {
        {"sang", "sing"},
        {"sung", "sing"},
        {"singing", "sing"},
        {"sings", "sing"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SWIM_FORMS[] = {
        {"swam", "swim"},
        {"swum", "swim"},
        {"swimming", "swim"},
        {"swims", "swim"},
        {nullptr, nullptr}
    };


    static const TIrregularWord BEGIN_FORMS[] = {
        {"began", "begin"},
        {"begun", "begin"},
        {"beginning", "begin"},
        {"begins", "begin"},
        {nullptr, nullptr}
    };


    static const TIrregularWord DRINK_FORMS[] = {
        {"drank", "drink"},
        {"drunk", "drink"},
        {"drinking", "drink"},
        {"drinks", "drink"},
        {nullptr, nullptr}
    };


    static const TIrregularWord RING_FORMS[] = {
        {"rang", "ring"},
        {"rung", "ring"},
        {"ringing", "ring"},
        {"rings", "ring"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SIT_FORMS[] = {
        {"sat", "sit"},
        {"sitting", "sit"},
        {"sits", "sit"},
        {"babysit", "babysit"},
        {"babysat", "babysit"},
        {nullptr, nullptr}
    };


    static const TIrregularWord STAND_FORMS[] = {
        {"stood", "stand"},
        {"standing", "stand"},
        {"stands", "stand"},
        {"understand", "understand"},
        {"understood", "understand"},
        {"withstand", "withstand"},
        {"withstood", "withstand"},
        {nullptr, nullptr}
    };


    static const TIrregularWord HOLD_FORMS[] = {
        {"held", "hold"},
        {"holding", "hold"},
        {"holds", "hold"},
        {"behold", "behold"},
        {"beheld", "behold"},
        {"withhold", "withhold"},
        {"withheld", "withhold"},
        {"uphold", "uphold"},
        {"upheld", "uphold"},
        {nullptr, nullptr}
    };


    static const TIrregularWord READ_FORMS[] = {
        {"read", "read"},
        {"reading", "read"},
        {"reads", "read"},
        {nullptr, nullptr}
    };


    static const TIrregularWord LEAD_FORMS[] = {
        {"led", "lead"},
        {"leading", "lead"},
        {"leads", "lead"},
        {"mislead", "mislead"},
        {"misled", "mislead"},
        {nullptr, nullptr}
    };


    static const TIrregularWord MEET_FORMS[] = {
        {"met", "meet"},
        {"meeting", "meet"},
        {"meets", "meet"},
        {nullptr, nullptr}
    };


    static const TIrregularWord PAY_FORMS[] = {
        {"paid", "pay"},
        {"paying", "pay"},
        {"pays", "pay"},
        {"repay", "repay"},
        {"repaid", "repay"},
        {"overpay", "overpay"},
        {"overpaid", "overpay"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SEND_FORMS[] = {
        {"sent", "send"},
        {"sending", "send"},
        {"sends", "send"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SPEND_FORMS[] = {
        {"spent", "spend"},
        {"spending", "spend"},
        {"spends", "spend"},
        {"overspend", "overspend"},
        {"overspent", "overspend"},
        {nullptr, nullptr}
    };


    static const TIrregularWord BUILD_FORMS[] = {
        {"built", "build"},
        {"building", "build"},
        {"builds", "build"},
        {"rebuild", "rebuild"},
        {"rebuilt", "rebuild"},
        {nullptr, nullptr}
    };


    static const TIrregularWord LOSE_FORMS[] = {
        {"lost", "lose"},
        {"losing", "lose"},
        {"loses", "lose"},
        {nullptr, nullptr}
    };


    static const TIrregularWord KEEP_FORMS[] = {
        {"kept", "keep"},
        {"keeping", "keep"},
        {"keeps", "keep"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SLEEP_FORMS[] = {
        {"slept", "sleep"},
        {"sleeping", "sleep"},
        {"sleeps", "sleep"},
        {"oversleep", "oversleep"},
        {"overslept", "oversleep"},
        {nullptr, nullptr}
    };


    static const TIrregularWord WIN_FORMS[] = {
        {"won", "win"},
        {"winning", "win"},
        {"wins", "win"},
        {nullptr, nullptr}
    };


    static const TIrregularWord WEAR_FORMS[] = {
        {"wore", "wear"},
        {"worn", "wear"},
        {"wearing", "wear"},
        {"wears", "wear"},
        {nullptr, nullptr}
    };


    static const TIrregularWord BEAT_FORMS[] = {
        {"beat", "beat"},
        {"beaten", "beat"},
        {"beating", "beat"},
        {"beats", "beat"},
        {nullptr, nullptr}
    };


    static const TIrregularWord BITE_FORMS[] = {
        {"bit", "bite"},
        {"bitten", "bite"},
        {"biting", "bite"},
        {"bites", "bite"},
        {nullptr, nullptr}
    };


    static const TIrregularWord BIND_FORMS[] = {
        {"bound", "bind"},
        {"binding", "bind"},
        {"binds", "bind"},
        {"unbind", "unbind"},
        {"unbound", "unbind"},
        {"rebind", "rebind"},
        {"rebound", "rebind"},
        {nullptr, nullptr}
    };


    static const TIrregularWord BLEED_FORMS[] = {
        {"bled", "bleed"},
        {"bleeding", "bleed"},
        {"bleeds", "bleed"},
        {nullptr, nullptr}
    };


    static const TIrregularWord BLOW_FORMS[] = {
        {"blew", "blow"},
        {"blown", "blow"},
        {"blowing", "blow"},
        {"blows", "blow"},
        {"overblow", "overblow"},
        {"overblew", "overblow"},
        {"overblown", "overblow"},
        {nullptr, nullptr}
    };


    static const TIrregularWord BEAR_FORMS[] = {
        {"bore", "bear"},
        {"born", "bear"},
        {"borne", "bear"},
        {"bearing", "bear"},
        {"bears", "bear"},
        {nullptr, nullptr}
    };


    static const TIrregularWord EAT_FORMS[] = {
        {"ate", "eat"},
        {"eaten", "eat"},
        {"eating", "eat"},
        {"eats", "eat"},
        {"overeat", "overeat"},
        {"overate", "overeat"},
        {"overeaten", "overeat"},
        {nullptr, nullptr}
    };


    static const TIrregularWord FALL_FORMS[] = {
        {"fell", "fall"},
        {"fallen", "fall"},
        {"falling", "fall"},
        {"falls", "fall"},
        {"befall", "befall"},
        {"befell", "befall"},
        {"befallen", "befall"},
        {nullptr, nullptr}
    };


    static const TIrregularWord HIDE_FORMS[] = {
        {"hid", "hide"},
        {"hidden", "hide"},
        {"hiding", "hide"},
        {"hides", "hide"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SHAKE_FORMS[] = {
        {"shook", "shake"},
        {"shaken", "shake"},
        {"shaking", "shake"},
        {"shakes", "shake"},
        {nullptr, nullptr}
    };


    static const TIrregularWord FREEZE_FORMS[] = {
        {"froze", "freeze"},
        {"frozen", "freeze"},
        {"freezing", "freeze"},
        {"freezes", "freeze"},
        {nullptr, nullptr}
    };


    static const TIrregularWord STEAL_FORMS[] = {
        {"stole", "steal"},
        {"stolen", "steal"},
        {"stealing", "steal"},
        {"steals", "steal"},
        {nullptr, nullptr}
    };


    static const TIrregularWord TEAR_FORMS[] = {
        {"tore", "tear"},
        {"torn", "tear"},
        {"tearing", "tear"},
        {"tears", "tear"},
        {nullptr, nullptr}
    };


    static const TIrregularWord WEAVE_FORMS[] = {
        {"wove", "weave"},
        {"woven", "weave"},
        {"weaving", "weave"},
        {"weaves", "weave"},
        {nullptr, nullptr}
    };


    static const TIrregularWord FORBID_FORMS[] = {
        {"forbade", "forbid"},
        {"forbidden", "forbid"},
        {"forbidding", "forbid"},
        {"forbids", "forbid"},
        {nullptr, nullptr}
    };


    static const TIrregularWord FORGIVE_FORMS[] = {
        {"forgave", "forgive"},
        {"forgiven", "forgive"},
        {"forgiving", "forgive"},
        {"forgives", "forgive"},
        {nullptr, nullptr}
    };


    static const TIrregularWord LIE_FORMS[] = {
        {"lay", "lie"},
        {"lain", "lie"},
        {"lying", "lie"},
        {"lies", "lie"},
        {nullptr, nullptr}
    };


    static const TIrregularWord LAY_FORMS[] = {
        {"laid", "lay"},
        {"laying", "lay"},
        {"lays", "lay"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SHINE_FORMS[] = {
        {"shone", "shine"},
        {"shined", "shine"},
        {"shining", "shine"},
        {"shines", "shine"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SHOOT_FORMS[] = {
        {"shot", "shoot"},
        {"shooting", "shoot"},
        {"shoots", "shoot"},
        {"overshoot", "overshoot"},
        {"overshot", "overshoot"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SHOW_FORMS[] = {
        {"showed", "show"},
        {"shown", "show"},
        {"showing", "show"},
        {"shows", "show"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SHRINK_FORMS[] = {
        {"shrank", "shrink"},
        {"shrunk", "shrink"},
        {"shrinking", "shrink"},
        {"shrinks", "shrink"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SHUT_FORMS[] = {
        {"shut", "shut"},
        {"shutting", "shut"},
        {"shuts", "shut"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SLAY_FORMS[] = {
        {"slew", "slay"},
        {"slain", "slay"},
        {"slaying", "slay"},
        {"slays", "slay"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SLIDE_FORMS[] = {
        {"slid", "slide"},
        {"sliding", "slide"},
        {"slides", "slide"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SLING_FORMS[] = {
        {"slung", "sling"},
        {"slinging", "sling"},
        {"slings", "sling"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SLIT_FORMS[] = {
        {"slit", "slit"},
        {"slitting", "slit"},
        {"slits", "slit"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SMITE_FORMS[] = {
        {"smote", "smite"},
        {"smitten", "smite"},
        {"smiting", "smite"},
        {"smites", "smite"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SOW_FORMS[] = {
        {"sowed", "sow"},
        {"sown", "sow"},
        {"sowing", "sow"},
        {"sows", "sow"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SPIN_FORMS[] = {
        {"spun", "spin"},
        {"spinning", "spin"},
        {"spins", "spin"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SPIT_FORMS[] = {
        {"spat", "spit"},
        {"spit", "spit"},
        {"spitting", "spit"},
        {"spits", "spit"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SPLIT_FORMS[] = {
        {"split", "split"},
        {"splitting", "split"},
        {"splits", "split"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SPREAD_FORMS[] = {
        {"spread", "spread"},
        {"spreading", "spread"},
        {"spreads", "spread"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SPRING_FORMS[] = {
        {"sprang", "spring"},
        {"sprung", "spring"},
        {"springing", "spring"},
        {"springs", "spring"},
        {nullptr, nullptr}
    };


    static const TIrregularWord STICK_FORMS[] = {
        {"stuck", "stick"},
        {"sticking", "stick"},
        {"sticks", "stick"},
        {nullptr, nullptr}
    };


    static const TIrregularWord STING_FORMS[] = {
        {"stung", "sting"},
        {"stinging", "sting"},
        {"stings", "sting"},
        {nullptr, nullptr}
    };


    static const TIrregularWord STINK_FORMS[] = {
        {"stank", "stink"},
        {"stunk", "stink"},
        {"stinking", "stink"},
        {"stinks", "stink"},
        {nullptr, nullptr}
    };


    static const TIrregularWord STRIDE_FORMS[] = {
        {"strode", "stride"},
        {"stridden", "stride"},
        {"striding", "stride"},
        {"strides", "stride"},
        {nullptr, nullptr}
    };


    static const TIrregularWord STRIKE_FORMS[] = {
        {"struck", "strike"},
        {"stricken", "strike"},
        {"striking", "strike"},
        {"strikes", "strike"},
        {nullptr, nullptr}
    };


    static const TIrregularWord STRING_FORMS[] = {
        {"strung", "string"},
        {"stringing", "string"},
        {"strings", "string"},
        {nullptr, nullptr}
    };


    static const TIrregularWord STRIVE_FORMS[] = {
        {"strove", "strive"},
        {"striven", "strive"},
        {"striving", "strive"},
        {"strives", "strive"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SWEAR_FORMS[] = {
        {"swore", "swear"},
        {"sworn", "swear"},
        {"swearing", "swear"},
        {"swears", "swear"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SWEEP_FORMS[] = {
        {"swept", "sweep"},
        {"sweeping", "sweep"},
        {"sweeps", "sweep"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SWELL_FORMS[] = {
        {"swelled", "swell"},
        {"swollen", "swell"},
        {"swelling", "swell"},
        {"swells", "swell"},
        {nullptr, nullptr}
    };


    static const TIrregularWord SWING_FORMS[] = {
        {"swung", "swing"},
        {"swinging", "swing"},
        {"swings", "swing"},
        {nullptr, nullptr}
    };


    static const TIrregularWord TREAD_FORMS[] = {
        {"trod", "tread"},
        {"trodden", "tread"},
        {"treading", "tread"},
        {"treads", "tread"},
        {nullptr, nullptr}
    };


    static const TIrregularWord WAKE_FORMS[] = {
        {"woke", "wake"},
        {"woken", "wake"},
        {"waking", "wake"},
        {"wakes", "wake"},
        {"awake", "awake"},
        {"awoke", "awake"},
        {"awoken", "awake"},
        {nullptr, nullptr}
    };


    static const TIrregularWord WIND_FORMS[] = {
        {"wound", "wind"},
        {"winding", "wind"},
        {"winds", "wind"},
        {"unwind", "unwind"},
        {"unwound", "unwind"},
        {"rewind", "rewind"},
        {"rewound", "rewind"},
        {nullptr, nullptr}
    };


    static const TIrregularWord WRING_FORMS[] = {
        {"wrung", "wring"},
        {"wringing", "wring"},
        {"wrings", "wring"},
        {nullptr, nullptr}
    };

    static const TIrregularWord LIGHT_FORMS[] = {
        {"lit", "light"},
        {"lighted", "light"},
        {"lighting", "light"},
        {"lights", "light"},
        {nullptr, nullptr}
    };

    static const TIrregularWord QUIT_FORMS[] = {
        {"quit", "quit"},
        {"quitting", "quit"},
        {"quits", "quit"},
        {nullptr, nullptr}
    };

    static const TIrregularWord SET_FORMS[] = {
        {"set", "set"},
        {"setting", "set"},
        {"sets", "set"},
        {"upset", "upset"},
        {"reset", "reset"},
        {"offset", "offset"},
        {nullptr, nullptr}
    };

    static const TIrregularWord CUT_FORMS[] = {
        {"cut", "cut"},
        {"cutting", "cut"},
        {"cuts", "cut"},
        {"undercut", "undercut"},
        {nullptr, nullptr}
    };

    static const TIrregularWord HIT_FORMS[] = {
        {"hit", "hit"},
        {"hitting", "hit"},
        {"hits", "hit"},
        {nullptr, nullptr}
    };

    static const TIrregularWord PUT_FORMS[] = {
        {"put", "put"},
        {"putting", "put"},
        {"puts", "put"},
        {"input", "input"},
        {"output", "output"},
        {nullptr, nullptr}
    };

    static const TIrregularWord LET_FORMS[] = {
        {"let", "let"},
        {"letting", "let"},
        {"lets", "let"},
        {nullptr, nullptr}
    };

    static const TIrregularWord COST_FORMS[] = {
        {"cost", "cost"},
        {"costing", "cost"},
        {"costs", "cost"},
        {nullptr, nullptr}
    };

    static const TIrregularWord CAST_FORMS[] = {
        {"cast", "cast"},
        {"casting", "cast"},
        {"casts", "cast"},
        {"broadcast", "broadcast"},
        {"forecast", "forecast"},
        {"overcast", "overcast"},
        {nullptr, nullptr}
    };

    static const TIrregularWord BURST_FORMS[] = {
        {"burst", "burst"},
        {"bursting", "burst"},
        {"bursts", "burst"},
        {nullptr, nullptr}
    };

    static const TIrregularWord HURT_FORMS[] = {
        {"hurt", "hurt"},
        {"hurting", "hurt"},
        {"hurts", "hurt"},
        {nullptr, nullptr}
    };

    static const TIrregularWord BET_FORMS[] = {
        {"bet", "bet"},
        {"betting", "bet"},
        {"bets", "bet"},
        {nullptr, nullptr}
    };

    static const TIrregularWord BEND_FORMS[] = {
        {"bent", "bend"},
        {"bending", "bend"},
        {"bends", "bend"},
        {nullptr, nullptr}
    };

    static const TIrregularWord LEND_FORMS[] = {
        {"lent", "lend"},
        {"lending", "lend"},
        {"lends", "lend"},
        {nullptr, nullptr}
    };

    static const TIrregularWord FEED_FORMS[] = {
        {"fed", "feed"},
        {"feeding", "feed"},
        {"feeds", "feed"},
        {"overfeed", "overfeed"},
        {"overfed", "overfeed"},
        {nullptr, nullptr}
    };

    static const TIrregularWord BREED_FORMS[] = {
        {"bred", "breed"},
        {"breeding", "breed"},
        {"breeds", "breed"},
        {"crossbreed", "crossbreed"},
        {"crossbred", "crossbreed"},
        {nullptr, nullptr}
    };

    static const TIrregularWord SPEED_FORMS[] = {
        {"sped", "speed"},
        {"speeding", "speed"},
        {"speeds", "speed"},
        {nullptr, nullptr}
    };

    static const TIrregularWord FLEE_FORMS[] = {
        {"fled", "flee"},
        {"fleeing", "flee"},
        {"flees", "flee"},
        {nullptr, nullptr}
    };

    static const TIrregularWord DEAL_FORMS[] = {
        {"dealt", "deal"},
        {"dealing", "deal"},
        {"deals", "deal"},
        {nullptr, nullptr}
    };

    static const TIrregularWord MEAN_FORMS[] = {
        {"meant", "mean"},
        {"meaning", "mean"},
        {"means", "mean"},
        {nullptr, nullptr}
    };

    static const TIrregularWord LEAN_FORMS[] = {
        {"leant", "lean"},
        {"leaned", "lean"},
        {"leaning", "lean"},
        {"leans", "lean"},
        {nullptr, nullptr}
    };

    static const TIrregularWord LEAP_FORMS[] = {
        {"leapt", "leap"},
        {"leaped", "leap"},
        {"leaping", "leap"},
        {"leaps", "leap"},
        {"overleap", "overleap"},
        {"overleapt", "overleap"},
        {nullptr, nullptr}
    };

    static const TIrregularWord LEARN_FORMS[] = {
        {"learnt", "learn"},
        {"learned", "learn"},
        {"learning", "learn"},
        {"learns", "learn"},
        {nullptr, nullptr}
    };

    static const TIrregularWord BURN_FORMS[] = {
        {"burnt", "burn"},
        {"burned", "burn"},
        {"burning", "burn"},
        {"burns", "burn"},
        {nullptr, nullptr}
    };

    static const TIrregularWord SMELL_FORMS[] = {
        {"smelt", "smell"},
        {"smelled", "smell"},
        {"smelling", "smell"},
        {"smells", "smell"},
        {nullptr, nullptr}
    };

    static const TIrregularWord SPELL_FORMS[] = {
        {"spelt", "spell"},
        {"spelled", "spell"},
        {"spelling", "spell"},
        {"spells", "spell"},
        {"misspell", "misspell"},
        {"misspelt", "misspell"},
        {nullptr, nullptr}
    };

    static const TIrregularWord SPILL_FORMS[] = {
        {"spilt", "spill"},
        {"spilled", "spill"},
        {"spilling", "spill"},
        {"spills", "spill"},
        {nullptr, nullptr}
    };

    static const TIrregularWord SPOIL_FORMS[] = {
        {"spoilt", "spoil"},
        {"spoiled", "spoil"},
        {"spoiling", "spoil"},
        {"spoils", "spoil"},
        {nullptr, nullptr}
    };

    static const TIrregularWord DREAM_FORMS[] = {
        {"dreamt", "dream"},
        {"dreamed", "dream"},
        {"dreaming", "dream"},
        {"dreams", "dream"},
        {nullptr, nullptr}
    };

    static const TIrregularWord DWELL_FORMS[] = {
        {"dwelt", "dwell"},
        {"dwelled", "dwell"},
        {"dwelling", "dwell"},
        {"dwells", "dwell"},
        {nullptr, nullptr}
    };

    static const TIrregularWord HANG_FORMS[] = {
        {"hung", "hang"},
        {"hanged", "hang"},
        {"hanging", "hang"},
        {"hangs", "hang"},
        {"overhang", "overhang"},
        {"overhung", "overhang"},
        {nullptr, nullptr}
    };

    static const TIrregularWord DIG_FORMS[] = {
        {"dug", "dig"},
        {"digging", "dig"},
        {"digs", "dig"},
        {nullptr, nullptr}
    };

    static const TIrregularWord CLING_FORMS[] = {
        {"clung", "cling"},
        {"clinging", "cling"},
        {"clings", "cling"},
        {nullptr, nullptr}
    };

    static const TIrregularWord FLING_FORMS[] = {
        {"flung", "fling"},
        {"flinging", "fling"},
        {"flings", "fling"},
        {nullptr, nullptr}
    };

    static const TIrregularWord WRING_FORMS_EXT[] = {
        {"wrung", "wring"},
        {"wringing", "wring"},
        {"wrings", "wring"},
        {nullptr, nullptr}
    };
}


namespace NIrregularNouns {
    struct TIrregularNoun {
        const char* Plural;
        const char* Singular;
    };


    static const TIrregularNoun NOUNS[] = {
        {"children", "child"},
        {"men", "man"},
        {"women", "woman"},
        {"feet", "foot"},
        {"teeth", "tooth"},
        {"mice", "mouse"},
        {"geese", "goose"},
        {"people", "person"},
        {"lice", "louse"},
        {"oxen", "ox"},
        {"deer", "deer"},
        {"sheep", "sheep"},
        {"fish", "fish"},
        {"moose", "moose"},
        {"series", "series"},
        {"species", "species"},
        {"aircraft", "aircraft"},
        {"spacecraft", "spacecraft"},
        {"salmon", "salmon"},
        {"trout", "trout"},
        {"swine", "swine"},
        {"bison", "bison"},
        {"buffalo", "buffalo"},
        {"shrimp", "shrimp"},
        {"cod", "cod"},
        {"squid", "squid"},
        {"cacti", "cactus"},
        {"cactuses", "cactus"},
        {"fungi", "fungus"},
        {"funguses", "fungus"},
        {"nuclei", "nucleus"},
        {"syllabi", "syllabus"},
        {"syllabuses", "syllabus"},
        {"alumni", "alumnus"},
        {"foci", "focus"},
        {"focuses", "focus"},
        {"radii", "radius"},
        {"stimuli", "stimulus"},
        {"termini", "terminus"},
        {"terminuses", "terminus"},
        {"cacti", "cactus"},
        {"analyses", "analysis"},
        {"axes", "axis"},
        {"bases", "basis"},
        {"crises", "crisis"},
        {"diagnoses", "diagnosis"},
        {"ellipses", "ellipsis"},
        {"hypotheses", "hypothesis"},
        {"oases", "oasis"},
        {"parentheses", "parenthesis"},
        {"synopses", "synopsis"},
        {"syntheses", "synthesis"},
        {"theses", "thesis"},
        {"phenomena", "phenomenon"},
        {"criteria", "criterion"},
        {"data", "datum"},
        {"errata", "erratum"},
        {"strata", "stratum"},
        {"addenda", "addendum"},
        {"bacteria", "bacterium"},
        {"curricula", "curriculum"},
        {"memoranda", "memorandum"},
        {"media", "medium"},
        {"millennia", "millennium"},
        {"ova", "ovum"},
        {"spectra", "spectrum"},
        {"symposia", "symposium"},
        {"algae", "alga"},
        {"antennae", "antenna"},
        {"antennas", "antenna"},
        {"formulae", "formula"},
        {"formulas", "formula"},
        {"larvae", "larva"},
        {"nebulae", "nebula"},
        {"vertebrae", "vertebra"},
        {"vitae", "vita"},
        {"appendices", "appendix"},
        {"appendixes", "appendix"},
        {"codices", "codex"},
        {"indices", "index"},
        {"indexes", "index"},
        {"matrices", "matrix"},
        {"matrixes", "matrix"},
        {"vertices", "vertex"},
        {"vortices", "vortex"},
        {"vortexes", "vortex"},
        {"apices", "apex"},
        {"apexes", "apex"},
        {"cortices", "cortex"},
        {"helices", "helix"},
        {"loci", "locus"},
        {"fungi", "fungus"},
        {"octopi", "octopus"},
        {"octopuses", "octopus"},
        {"platypuses", "platypus"},
        {"platypi", "platypus"},
        {"cacti", "cactus"},
        {"genii", "genius"},
        {"geniuses", "genius"},
        {"styli", "stylus"},
        {"styluses", "stylus"},
        {"abscissae", "abscissa"},
        {"amoebae", "amoeba"},
        {"amoebas", "amoeba"},
        {"antitheses", "antithesis"},
        {"aphides", "aphis"},
        {"apices", "apex"},
        {"automata", "automaton"},
        {"automatons", "automaton"},
        {"cervices", "cervix"},
        {"crania", "cranium"},
        {"equilibria", "equilibrium"},
        {"ganglia", "ganglion"},
        {"genera", "genus"},
        {"gymnasia", "gymnasium"},
        {"loci", "locus"},
        {"penumbrae", "penumbra"},
        {"phyla", "phylum"},
        {"quanta", "quantum"},
        {"rostra", "rostrum"},
        {"septa", "septum"},
        {"solaria", "solarium"},
        {"stamina", "stamen"},
        {"thoraces", "thorax"},
        {"ultimata", "ultimatum"},
        {"umbrae", "umbra"},
        {"uteri", "uterus"},
        {"viscera", "viscus"},
        {"aquaria", "aquarium"},
        {"aquariums", "aquarium"},
        {"consortia", "consortium"},
        {"crania", "cranium"},
        {"craniums", "cranium"},
        {"emporium", "emporium"},
        {"emporia", "emporium"},
        {"equilibria", "equilibrium"},
        {"equilibriums", "equilibrium"},
        {"ganglia", "ganglion"},
        {"ganglions", "ganglion"},
        {"gymnasia", "gymnasium"},
        {"gymnasiums", "gymnasium"},
        {"honoraria", "honorarium"},
        {"honorariums", "honorarium"},
        {"mausolea", "mausoleum"},
        {"mausoleums", "mausoleum"},
        {"moratorium", "moratorium"},
        {"moratoria", "moratorium"},
        {"planetaria", "planetarium"},
        {"planetariums", "planetarium"},
        {"podiums", "podium"},
        {"podia", "podium"},
        {"referenda", "referendum"},
        {"referendums", "referendum"},
        {"rostra", "rostrum"},
        {"rostrums", "rostrum"},
        {"sanatoriums", "sanatorium"},
        {"sanatoria", "sanatorium"},
        {"stadiums", "stadium"},
        {"stadia", "stadium"},
        {"symposiums", "symposium"},
        {"symposia", "symposium"},
        {"terrariums", "terrarium"},
        {"terraria", "terrarium"},
        {"ultimatums", "ultimatum"},
        {"ultimata", "ultimatum"},
        {"vivariums", "vivarium"},
        {"vivaria", "vivarium"},
        
        {"atria", "atrium"},
        {"bacilli", "bacillus"},
        {"bronchi", "bronchus"},
        {"cilia", "cilium"},
        {"flagella", "flagellum"},
        {"ganglia", "ganglion"},
        {"mitochondria", "mitochondrion"},
        {"mycelia", "mycelium"},
        {"ova", "ovum"},
        {"protozoa", "protozoan"},
        {"septa", "septum"},
        {"spermatozoa", "spermatozoon"},
        {"venae", "vena"},
        
        {"abscissae", "abscissa"},
        {"abscissas", "abscissa"},
        {"apices", "apex"},
        {"asymptotes", "asymptote"},
        {"axes", "axis"},
        {"binomials", "binomial"},
        {"corollaries", "corollary"},
        {"loci", "locus"},
        {"maxima", "maximum"},
        {"maximums", "maximum"},
        {"minima", "minimum"},
        {"minimums", "minimum"},
        {"optima", "optimum"},
        {"optimums", "optimum"},
        {"polyhedra", "polyhedron"},
        {"polyhedrons", "polyhedron"},
        {"quanta", "quantum"},
        {"radices", "radix"},
        {"simplices", "simplex"},
        {"vertices", "vertex"},
        
        {"corpora", "corpus"},
        {"genera", "genus"},
        {"lemmas", "lemma"},
        {"lemmata", "lemma"},
        {"lexica", "lexicon"},
        {"lexicons", "lexicon"},
        {"parentheses", "parenthesis"},
        {"schemata", "schema"},
        {"schemas", "schema"},
        
        {"amoebae", "amoeba"},
        {"antennae", "antenna"},
        {"larvae", "larva"},
        {"pupae", "pupa"},
        {"chrysalises", "chrysalis"},
        {"chrysalides", "chrysalis"},
        
        {"addenda", "addendum"},
        {"addendums", "addendum"},
        {"agenda", "agendum"},
        {"algae", "alga"},
        {"alumni", "alumnus"},
        {"alumnae", "alumna"},
        {"automata", "automaton"},
        {"automatons", "automaton"},
        {"candelabra", "candelabrum"},
        {"corrigenda", "corrigendum"},
        {"desiderata", "desideratum"},
        {"dicta", "dictum"},
        {"effluvia", "effluvium"},
        {"errata", "erratum"},
        {"insignia", "insigne"},
        {"memoranda", "memorandum"},
        {"millennia", "millennium"},
        {"millenniums", "millennium"},
        {"minima", "minimum"},
        {"phyla", "phylum"},
        {"quanta", "quantum"},
        {"spectra", "spectrum"},
        {"spectrums", "spectrum"},
        {"strata", "stratum"},
        {"symposia", "symposium"},
        {"vaccinia", "vaccinium"},

        {nullptr, nullptr}

    };
}


namespace NIrregularAdverbs {
    struct TIrregularAdverb {
        const char* Form;
        const char* Base;
    };

    static const TIrregularAdverb ADVERBS[] = {
        {"worse", "badly"},
        {"worst", "badly"},
        {"better", "well"},
        {"best", "well"},
        {"more", "much"},
        {"most", "much"},
        {"less", "little"},
        {"least", "little"},
        {"farther", "far"},
        {"farthest", "far"},
        {"further", "far"},
        {"furthest", "far"},
        {nullptr, nullptr}
    };
}


namespace NIrregularAdjectives {
    struct TIrregularAdjective {
        const char* Form;
        const char* Base;
    };


    static const TIrregularAdjective ADJECTIVES[] = {
        {"better", "good"},
        {"best", "good"},
        {"worse", "bad"},
        {"worst", "bad"},
        {"more", "much"},
        {"most", "much"},
        {"less", "little"},
        {"least", "little"},
        {"farther", "far"},
        {"farthest", "far"},
        {"further", "far"},
        {"furthest", "far"},
        {"older", "old"},
        {"oldest", "old"},
        {"elder", "old"},
        {"eldest", "old"},
        {nullptr, nullptr}
    };
}


} // namespace NStemmer
