#include "dtknastrangraphpchmapping.h"

ElementLayout PchMapping::getLayout(int elementType, const std::string& cat,bool isComplex,bool isMagPhase) {
    ElementLayout layout;
    if (cat == "STRAIN" || cat == "STRESS") {
        if (elementType == 82) { // QUADR
            layout.repeatCount = 5; // CEN + 4 Corners
            layout.wordsPerPoint = 19;
            layout.wordToInfo[5] = { Component::SX, LocationType::Z1 };
            layout.wordToInfo[13] = { Component::SX, LocationType::Z2 };
            // ... 继续添加其他 Word 映射
        }
        else if (elementType == 34) { // BAR
            layout.repeatCount = 1;
            layout.wordsPerPoint = 10;
            layout.wordToInfo[3] = { Component::SX, LocationType::SINGLE };
        }
    }
    else if (cat == "ENERGY") { // 应变能
        layout.repeatCount = 1;
        layout.wordsPerPoint = 4;
        layout.wordToInfo[2] = { Component::STRAIN_ENERGY, LocationType::SINGLE };
        layout.wordToInfo[3] = { Component::ENERGY_PERCENT, LocationType::SINGLE };
    }
    if (cat == "DISPLACEMENT" && isComplex) {
        layout.wordsPerPoint = 14; // ID + Type + 6个分量*2 = 14
        if (isMagPhase) {
            layout.wordToInfo[3] = { Component::T1_MAG, LocationType::SINGLE };
            layout.wordToInfo[4] = { Component::T1_PHASE, LocationType::SINGLE };
            // ... T2_MAG, T2_PHASE ...
        }
        else {
            layout.wordToInfo[3] = { Component::T1_REAL, LocationType::SINGLE };
            layout.wordToInfo[4] = { Component::T1_IMAG, LocationType::SINGLE };
        }
    }
    return layout;
}