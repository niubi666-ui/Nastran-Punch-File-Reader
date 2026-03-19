#include "dtknastrangraphpchmapping.h"

ElementLayout PchMapping::getLayout(int elementType, const std::string& cat,bool isComplex,bool isMagPhase)
{
    ElementLayout layout;
    if (cat == "STRAIN" || cat == "STRESS")
    {
        if (elementType == 82)
        { // QUADR
            layout.repeatCount = 5; // CEN + 4 Corners
            layout.wordsPerPoint = 19;
            layout.wordToInfo[5] = { Component::SX, LocationType::Z1 };
            layout.wordToInfo[13] = { Component::SX, LocationType::Z2 };
            // ... 继续添加其他 Word 映射
        }
        else if (elementType == 34)
        { // BAR
            layout.repeatCount = 1;
            layout.wordsPerPoint = 10;
            layout.wordToInfo[3] = { Component::SX, LocationType::SINGLE };
        }
        else if (elementType == 39)
        {
            layout.repeatCount = 5;
            layout.wordsPerPoint = 21;
            layout.dataStartWord = 5;
            layout.wordGridID = 5;

            // Word 5 是 GRID ID，由解析器在 parseSinglePoint 中通过 getField(line, 2) 提取
            layout.wordToInfo[6]  = { Component::SX,    LocationType::SINGLE }; // Normal-X
            layout.wordToInfo[7]  = { Component::SXY,   LocationType::SINGLE }; // Shear-XY
            layout.wordToInfo[8]  = { Component::P1,    LocationType::SINGLE }; // First Principal

            layout.wordToInfo[9]  = { Component::P1X,   LocationType::SINGLE }; // P1 X-Cos
            layout.wordToInfo[10] = { Component::P1Y,   LocationType::SINGLE }; // P1 Y-Cos
            layout.wordToInfo[11] = { Component::P1Z,   LocationType::SINGLE }; // P1 Z-Cos

            layout.wordToInfo[12] = { Component::P_AVG, LocationType::SINGLE }; // Mean Pressure
            layout.wordToInfo[13] = { Component::OCT,   LocationType::SINGLE }; // Octahedral
            layout.wordToInfo[14] = { Component::SY,    LocationType::SINGLE }; // Normal-Y

            layout.wordToInfo[15] = { Component::SYZ,   LocationType::SINGLE }; // Shear-YZ
            layout.wordToInfo[16] = { Component::P2,    LocationType::SINGLE }; // Second Principal
            layout.wordToInfo[17] = { Component::P2X,   LocationType::SINGLE }; // P2 X-Cos

            layout.wordToInfo[18] = { Component::P2Y,   LocationType::SINGLE }; // P2 Y-Cos
            layout.wordToInfo[19] = { Component::P2Z,   LocationType::SINGLE }; // P2 Z-Cos
            layout.wordToInfo[20] = { Component::SZ,    LocationType::SINGLE }; // Normal-Z

            layout.wordToInfo[21] = { Component::SZX,   LocationType::SINGLE }; // Shear-ZX
            layout.wordToInfo[22] = { Component::P3,    LocationType::SINGLE }; // Third Principal
            layout.wordToInfo[23] = { Component::P3X,   LocationType::SINGLE }; // P3 X-Cos

            layout.wordToInfo[24] = { Component::P3Y,   LocationType::SINGLE }; // P3 Y-Cos
            layout.wordToInfo[25] = { Component::P3Z,   LocationType::SINGLE }; // P3 Z-Cos
        }
    }
    else if (cat == "ENERGY")
    {
        layout.repeatCount = 1;
        layout.wordsPerPoint = 2;
        layout.wordToInfo[2] = { Component::STRAIN_ENERGY, LocationType::SINGLE };
        layout.wordToInfo[3] = { Component::ENERGY_PERCENT, LocationType::SINGLE };
    }
    if (cat == "DISPLACEMENT" || cat == "VELOCITY" || cat == "ACCELERATION")
    {
        layout.repeatCount = 1;
        layout.dataStartWord = 3;

        if (!isComplex)
        {
            // --- 模式 1: 纯实数 (如 SOL 101) ---
            layout.wordsPerPoint = 6;
            layout.wordToInfo[3] = { Component::T1, LocationType::SINGLE };
            layout.wordToInfo[4] = { Component::T2, LocationType::SINGLE };
            layout.wordToInfo[5] = { Component::T3, LocationType::SINGLE };
            layout.wordToInfo[6] = { Component::R1, LocationType::SINGLE };
            layout.wordToInfo[7] = { Component::R2, LocationType::SINGLE };
            layout.wordToInfo[8] = { Component::R3, LocationType::SINGLE };
        }
        else if (isMagPhase)
        {
            // --- 模式 2: 幅值 & 相位
            layout.wordsPerPoint = 12;

            layout.wordToInfo[3] = { Component::T1_MAG,   LocationType::SINGLE };
            layout.wordToInfo[4] = { Component::T2_MAG,   LocationType::SINGLE };
            layout.wordToInfo[5] = { Component::T3_MAG,   LocationType::SINGLE };
            layout.wordToInfo[6] = { Component::R1_MAG,   LocationType::SINGLE };
            layout.wordToInfo[7] = { Component::R2_MAG,   LocationType::SINGLE };
            layout.wordToInfo[8] = { Component::R3_MAG,   LocationType::SINGLE };
            layout.wordToInfo[9] = { Component::T1_PHASE, LocationType::SINGLE };
            layout.wordToInfo[10] = { Component::T2_PHASE, LocationType::SINGLE };
            layout.wordToInfo[11] = { Component::T3_PHASE, LocationType::SINGLE };
            layout.wordToInfo[12] = { Component::R1_PHASE, LocationType::SINGLE };
            layout.wordToInfo[13] = { Component::R2_PHASE, LocationType::SINGLE };
            layout.wordToInfo[14] = { Component::R3_PHASE, LocationType::SINGLE };
        }
        else
        {
            // --- 模式 3: 实部 & 虚部 (REAL-IMAGINARY) ---
            layout.wordsPerPoint = 12;

            layout.wordToInfo[3] = { Component::T1_REAL,   LocationType::SINGLE };
            layout.wordToInfo[4] = { Component::T2_REAL,   LocationType::SINGLE };
            layout.wordToInfo[5] = { Component::T3_REAL,   LocationType::SINGLE };
            layout.wordToInfo[6] = { Component::R1_REAL,   LocationType::SINGLE };
            layout.wordToInfo[7] = { Component::R2_REAL,   LocationType::SINGLE };
            layout.wordToInfo[8] = { Component::R3_REAL,   LocationType::SINGLE };
            layout.wordToInfo[9] = { Component::T1_IMAG,   LocationType::SINGLE };
            layout.wordToInfo[10] = { Component::T2_IMAG,   LocationType::SINGLE };
            layout.wordToInfo[11] = { Component::T3_IMAG,   LocationType::SINGLE };
            layout.wordToInfo[12] = { Component::R1_IMAG,   LocationType::SINGLE };
            layout.wordToInfo[13] = { Component::R2_IMAG,   LocationType::SINGLE };
            layout.wordToInfo[14] = { Component::R3_IMAG,   LocationType::SINGLE };
        }
    }
    return layout;
}
