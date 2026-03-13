#include "dtknastrangraphpchparser.h"
#include <fstream>

PchParser::PchParser(PchDataStore& store) : m_store(store) {}

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
        if (line.find("$SUBCASE ID =") != std::string::npos) {
            m_currentSubcase = std::stoi(line.substr(line.find("=") + 1));
        }

        if (line.find("$REAL OUTPUT") != std::string::npos) {
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
        // --- 3. 多路径触发解析 (关键修改) ---

        // 路径 A: 单元类结果 (需检测 Element Type)
        if (line.find("ELEMENT TYPE =") != std::string::npos) {
            m_currentElementType = std::stoi(line.substr(line.find("=") + 1));
            processBlock(file); // 进入解析循环
        }

        // 路径 B: 点类结果 (检测到 POINT ID 即可进入)
        // 对应你图片 image_9aadd8.png 中的情况
        else if (line.find("$POINT ID =") != std::string::npos) {
            m_currentElementType = 0; // 约定 0 代表节点结果
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
    std::string currentLine = firstLine;
    int parentID = (int)m_utils.safeStod(m_utils.getField(currentLine, 0));

    for (int r = 0; r < layout.repeatCount; ++r) {
        if (r > 0) {
            if (!std::getline(file, currentLine)) break;
        }

        int currentGridID = 0;
        std::string gridStr = m_utils.getField(currentLine, 1);
        if (gridStr.find("CEN") == std::string::npos) {
            currentGridID = (int)m_utils.safeStod(gridStr);
        }

        int wordCounter = 1;
        while (wordCounter < layout.wordsPerPoint) {
            // C++11 兼容写法
            bool isCont = (currentLine.compare(0, 6, "-CONT-") == 0);
            int fieldsInLine = isCont ? 4 : 3;
            int startIdx = isCont ? 1 : 2;

            for (int f = 0; f < fieldsInLine; ++f) {
                wordCounter++;

                if (layout.wordToInfo.count(wordCounter)) {
                    auto& info = layout.wordToInfo.at(wordCounter);
                    double val = m_utils.safeStod(m_utils.getField(currentLine, startIdx + f));

                    PchEntry entry;
                    entry.subcase = m_currentSubcase;
                    entry.parentID = parentID;
                    entry.gridID = currentGridID;
                    entry.loc = info.loc;
                    entry.comp = info.comp;
                    entry.xVal = (float)m_currentXVal;
                    entry.yVal = (float)val;

                    m_store.addEntry(entry);
                }

                if (wordCounter >= layout.wordsPerPoint) break;
            }

            if (wordCounter < layout.wordsPerPoint) {
                if (!std::getline(file, currentLine)) break;
            }
        }
    }
}