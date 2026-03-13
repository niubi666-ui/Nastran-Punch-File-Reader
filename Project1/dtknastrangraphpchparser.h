#ifndef DTKNASTRANGRAPHPCHPARSER_H
#define DTKNASTRANGRAPHPCHPARSER_H
#include "dtknastrangraphpchdatastore.h"
#include "dtknastrangraphpchutils.h"
#include "dtknastrangraphpchmapping.h"

class PchParser {
public:
    PchParser(PchDataStore& store);
    bool parse(const std::string& filePath);

private:
    void processBlock(std::ifstream& file);
    void parseElementData(std::ifstream& file, const std::string& firstLine, const ElementLayout& layout);

    PchDataStore& m_store;
    PchUtils m_utils;
    PchMapping m_mapping;
    int m_currentSubcase = 0;
    int m_currentElementType = 0;
    std::string m_currentCategory;
    double m_currentXVal = 0.0;

    bool m_isComplex = false;    // 当前块是否为复数 (REAL 为 false, COMPLEX 为 true)
    bool m_isMagPhase = false;   // 若为复数，是 Magnitude/Phase (true) 还是 Real/Imaginary (false)
};
#endif