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

bool PchParser::parse(const std::string& filePath)
{
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // --- 1. 识别类别 ---
        if (line.find("$DISPLACEMENTS") != std::string::npos) m_currentCategory = ResultCategory::DISPLACEMENT;
        else if (line.find("$VELOCITY") != std::string::npos) m_currentCategory = ResultCategory::VELOCITY;
        else if (line.find("$ACCELERATION") != std::string::npos) m_currentCategory = ResultCategory::ACCELERATION;
        else if (line.find("$ELEMENT STRAINS") != std::string::npos) m_currentCategory = ResultCategory::STRAIN;
        else if (line.find("$ELEMENT STRESSES") != std::string::npos) m_currentCategory = ResultCategory::STRESS;
        else if (line.find("$SPCF") != std::string::npos) m_currentCategory = ResultCategory::SPCF;

        // --- 2. 识别 Subcase 和 输出模式 ---
        else if (line.find("$SUBCASE ID =") != std::string::npos) {
            m_currentSubcase = std::stoi(line.substr(line.find("=") + 1));
        }
        else if (line.find("$REAL OUTPUT") != std::string::npos) {
            m_isComplex = false; m_isMagPhase = false;
        }
        else if (line.find("$COMPLEX OUTPUT") != std::string::npos) {
            m_isComplex = true;
        }
        else if (line.find("$MAGNITUDE-PHASE") != std::string::npos) {
            m_isComplex = true; m_isMagPhase = true;
        }
        else if (line.find("$REAL-IMAGINARY") != std::string::npos) {
            m_isComplex = true; m_isMagPhase = false;
        }

        // --- 3. 识别 Sort 模式与元数据 ---
        else if (line.find("$FREQUENCY =") != std::string::npos || line.find("$TIME =") != std::string::npos) {
            m_currentSortMode = SortMode::SORT_1;
            m_currentXVal = m_utils.safeStod(line.substr(line.find("=") + 1));
        }
        else if (line.find("$POINT ID =") != std::string::npos) {
            m_currentSortMode = SortMode::SORT_2;
            m_currentParentID = std::stoi(line.substr(line.find("=") + 1));
            m_currentElementType = 0;
        }
        else if (line.find("ELEMENT TYPE =") != std::string::npos) {
            m_currentElementType = std::stoi(line.substr(line.find("=") + 1));
        }
        else if (line.find("$") != std::string::npos) {
            continue;
        }
        if (file.peek() != EOF && file.peek() != '$')
        {
            processBlock(file);
        }
    }
    m_store.finalize();
    return true;
}

void PchParser::processBlock(std::ifstream& file)
{
    ElementLayout layout = m_mapping.getLayout(m_currentElementType, m_currentCategory, m_isComplex, m_isMagPhase);

    while (file.good())
    {
        if (file.peek() == '$')
        {
            // 遇见标题行，说明当前 Block 彻底结束，交还给 parse()
            break;
        }

        for (int i = 0; i < layout.repeatCount; ++i)
        {
            std::string line;
            if (!std::getline(file, line) || line.empty())
            {
                break;
            }

            // 提取 Word 1
            std::string w1;
            if (i == 0)
            {
                w1 = m_utils.getField(line, 0);
            }
            else
            {
                w1 = m_utils.getField(line, 2);
            }

            if (m_currentSortMode == SortMode::SORT_1 && i == 0)
            {
                // SORT1: Word 1 是 ID，X 已经在标题行读过了
                if (!w1.empty())
                {
                    m_currentParentID = std::stoi(w1);
                }
                m_currentGridID = 0; // 节点默认为 0
            }
            else // SortMode::SORT_2
            {
                // SORT2: Word 1 是频率 (X)，ParentID 已经在标题行读过了
                if (!w1.empty())
                {
                    m_currentXVal = m_utils.safeStod(w1);
                }
                m_currentGridID = 0;
            }

            // 针对单元结果的特殊逻辑：如果 i=0 且是单元，可能需要处理 ParentID 更新和换行
            if (m_currentElementType != 0 && i == 0)
            {
                if (m_currentSortMode == SortMode::SORT_1)
                {
                    // 单元 SORT1 下，Word 1 是 ElementID
                    m_currentParentID = std::stoi(w1);

                    // 如果数据在下一行开始，则换行
                    if (layout.dataStartWord >= 5)
                    {
                        if (!std::getline(file, line)) break;
                    }
                }
            }

            parseElementData(file, line, layout);
        }
    }
}

void PchParser::parseElementData(std::ifstream& file, const std::string& firstLine, const ElementLayout& layout)
{
    std::string currentLine = firstLine;
    int wordsHandled = 0;

    while (wordsHandled < layout.wordsPerPoint)
    {
        for (int f = 0; f < 3 && wordsHandled < layout.wordsPerPoint; ++f)
        {
            // 1. 计算当前槽位对应的逻辑 Word 编号
            int currentWordIdx = layout.dataStartWord + wordsHandled;

            // 2. 物理提取数据 (getField 2, 3, 4 对应 Word 3, 4, 5)
            std::string valStr = m_utils.getField(currentLine, f + 2);

            // 3. 判定是否为 Grid ID 位
            if (currentWordIdx == layout.wordGridID)
            {
                if (!valStr.empty())
                {
                    m_currentGridID = std::stoi(valStr);
                }
            }
            // 4. 判定是否为物理分量位
            else if (layout.wordToInfo.count(currentWordIdx))
            {
                PchEntry entry;
                entry.subcase = m_currentSubcase;
                entry.category = m_currentCategory;
                entry.eType = m_currentElementType;
                entry.parentID = m_currentParentID;
                entry.gridID = m_currentGridID;
                entry.xVal = (double)m_currentXVal;
                entry.yVal = (double)m_utils.safeStod(valStr);

                const auto& info = layout.wordToInfo.at(currentWordIdx);
                entry.comp = info.comp;
                entry.loc = (m_currentGridID == 0) ? LocationType::CENTER : LocationType::CORNER;

                m_store.addEntry(entry);
            }

            wordsHandled++;
        }

        // 5. 换行逻辑：如果没处理完，读下一行
        if (wordsHandled < layout.wordsPerPoint)
        {
            if (!std::getline(file, currentLine))
            {
                break;
            }
        }
    }
}
