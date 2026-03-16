#include "dtknastrangraphpchparser.h"
#include <fstream>

PchParser::PchParser(PchDataStore& store)
    : m_store(store),
    m_currentSubcase(0),
    m_currentParentID(0),
    m_currentElementType(0),
    m_currentSortMode(SortMode::SORT_1)
{
}

bool PchParser::parse(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // --- 1. 识别类别 (Category) ---
        if (line.find("$DISPLACEMENTS") != std::string::npos) m_currentCategory = "DISPLACEMENT";
        else if (line.find("$VELOCITY") != std::string::npos) m_currentCategory = "VELOCITY";
        else if (line.find("$ACCELERATION") != std::string::npos) m_currentCategory = "ACCELERATION";
        else if (line.find("$ELEMENT STRAINS") != std::string::npos) m_currentCategory = "STRAIN";

        // --- 2. 识别 Subcase 和 X 轴数值 ---
        else if (line.find("$SUBCASE ID =") != std::string::npos) {
            m_currentSubcase = std::stoi(line.substr(line.find("=") + 1));
        }

        else if (line.find("$REAL OUTPUT") != std::string::npos) {
            m_isComplex = false;
            m_isMagPhase = false;
        }
        else if (line.find("$COMPLEX OUTPUT") != std::string::npos) {
            m_isComplex = true;
        }
        else if (line.find("$MAGNITUDE-PHASE") != std::string::npos) {
            m_isComplex = true;
            m_isMagPhase = true; // 此时数据对是：幅值, 相位
        }
        else if (line.find("$REAL-IMAGINARY") != std::string::npos) {
            m_isComplex = true;
            m_isMagPhase = false; // 此时数据对是：实部, 虚部
        }
        else if (line.find("$FREQUENCY =") != std::string::npos ||
            line.find("$TIME STEP =") != std::string::npos) {
            m_currentSortMode = SortMode::SORT_1;
            m_currentXVal = m_utils.safeStod(line.substr(line.find("=") + 1));
            continue;
        }

        // 点类结果
        else if (line.find("$POINT ID =") != std::string::npos) {
            m_currentSortMode = SortMode::SORT_2;
            m_currentElementType = 0; // 约定 0 代表节点结果
            processBlock(file);
        }

        // 单元类结果
        else if (line.find("ELEMENT TYPE =") != std::string::npos) {
            m_currentElementType = std::stoi(line.substr(line.find("=") + 1));
            processBlock(file);
        }
    }
    m_store.finalize();
    return true;
}

void PchParser::processBlock(std::ifstream& file) {
    m_store.registerMetadata(m_currentSubcase, m_currentCategory, m_currentElementType);
    ElementLayout layout = m_mapping.getLayout(m_currentElementType, m_currentCategory,m_isComplex,m_isMagPhase);
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (line[0] == '$') break;
        if (line.compare(0, 6, "-CONT-") != 0) parseElementData(file, line, layout);
    }
}

void PchParser::parseElementData(std::ifstream& file, const std::string& firstLine, const ElementLayout& layout) {
    int wordCounter = 1;
    std::string currentLine = firstLine;

    while (wordCounter <= layout.wordsPerPoint) {
        bool isCont = (currentLine.compare(0, 6, "-CONT-") == 0);
        int startIdx = isCont ? 1 : 0;
        int fieldsInLine = isCont ? 4 : 3;

        for (int f = 0; f < fieldsInLine && wordCounter <= layout.wordsPerPoint; ++f) {
            std::string field = m_utils.getField(currentLine, startIdx + f);
            
            // --- 处理 Word 1 (ID 或 Frequency) ---
            if (wordCounter == 1) {
                if (m_currentSortMode == SortMode::SORT_1) {
                    // Sort 1 模式：第一列是 ID
                    m_currentParentID = std::stoi(field);
                } else {
                    // Sort 2 模式：第一列是频率 (X轴)
                    m_currentXVal = m_utils.safeStod(field);
                }
            } 
            // --- 处理后续数据 Word (3, 4, 5...) ---
            else if (layout.wordToInfo.count(wordCounter)) {
                auto info = layout.wordToInfo.at(wordCounter);
                PchEntry entry;
                entry.subcase = m_currentSubcase;
                entry.parentID = m_currentParentID; // 如果是 Sort 2，这个值来自 $POINT ID 行
                entry.gridID = 0; // 如果需要 GridID，逻辑需进一步扩展
                entry.xVal = (float)m_currentXVal; // 如果是 Sort 2，这个值来自 Word 1
                entry.yVal = (float)m_utils.safeStod(field);
                entry.comp = info.comp;
                entry.loc = info.loc;
                m_store.addEntry(entry);
            }
            wordCounter++;
        }
        
        // 如果 Word 还没读够，换下一行
        if (wordCounter <= layout.wordsPerPoint) {
            if (!std::getline(file, currentLine)) break;
        }
    }
}