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
    if (cat == "DISPLACEMENT" || cat == "VELOCITY" || cat == "ACCELERATION") {
        layout.repeatCount = 1;

        if (!isComplex) {
            // --- 模式 1: 纯实数 (如 SOL 101) ---
            layout.wordsPerPoint = 8;
            layout.wordToInfo[3] = { Component::T1, LocationType::SINGLE };
            layout.wordToInfo[4] = { Component::T2, LocationType::SINGLE };
            layout.wordToInfo[5] = { Component::T3, LocationType::SINGLE };
            layout.wordToInfo[6] = { Component::R1, LocationType::SINGLE };
            layout.wordToInfo[7] = { Component::R2, LocationType::SINGLE };
            layout.wordToInfo[8] = { Component::R3, LocationType::SINGLE };
        }
        else if (isMagPhase) {
            // --- 模式 2: 幅值 & 相位
            // 此时 Words 总数翻倍，因为 T1 占 2 个 Word，T2 占 2 个...
            layout.wordsPerPoint = 14;

            // T1
            layout.wordToInfo[3] = { Component::T1_MAG,   LocationType::SINGLE };
            layout.wordToInfo[4] = { Component::T1_PHASE, LocationType::SINGLE };
            // T2
            layout.wordToInfo[5] = { Component::T2_MAG,   LocationType::SINGLE };
            layout.wordToInfo[6] = { Component::T2_PHASE, LocationType::SINGLE };
            // T3
            layout.wordToInfo[7] = { Component::T3_MAG,   LocationType::SINGLE };
            layout.wordToInfo[8] = { Component::T3_PHASE, LocationType::SINGLE };
            // R1
            layout.wordToInfo[9] = { Component::R1_MAG,   LocationType::SINGLE };
            layout.wordToInfo[10] = { Component::R1_PHASE, LocationType::SINGLE };
            // R2
            layout.wordToInfo[11] = { Component::R2_MAG,   LocationType::SINGLE };
            layout.wordToInfo[12] = { Component::R2_PHASE, LocationType::SINGLE };
            // R3
            layout.wordToInfo[13] = { Component::R3_MAG,   LocationType::SINGLE };
            layout.wordToInfo[14] = { Component::R3_PHASE, LocationType::SINGLE };
        }
        else {
            // --- 模式 3: 实部 & 虚部 (REAL-IMAGINARY) ---
            layout.wordsPerPoint = 14;
            // T1
            layout.wordToInfo[3] = { Component::T1_REAL,   LocationType::SINGLE };
            layout.wordToInfo[4] = { Component::T1_IMAG, LocationType::SINGLE };
            // T2
            layout.wordToInfo[5] = { Component::T2_REAL,   LocationType::SINGLE };
            layout.wordToInfo[6] = { Component::T2_IMAG, LocationType::SINGLE };
            // T3
            layout.wordToInfo[7] = { Component::T3_REAL,   LocationType::SINGLE };
            layout.wordToInfo[8] = { Component::T3_IMAG, LocationType::SINGLE };
            // R1
            layout.wordToInfo[9] = { Component::R1_REAL,   LocationType::SINGLE };
            layout.wordToInfo[10] = { Component::R1_IMAG, LocationType::SINGLE };
            // R2
            layout.wordToInfo[9] = { Component::R2_REAL,   LocationType::SINGLE };
            layout.wordToInfo[10] = { Component::R2_IMAG, LocationType::SINGLE };
            // R3
            layout.wordToInfo[9] = { Component::R3_REAL,   LocationType::SINGLE };
            layout.wordToInfo[10] = { Component::R3_IMAG, LocationType::SINGLE };
        }
    }
    return layout;
}