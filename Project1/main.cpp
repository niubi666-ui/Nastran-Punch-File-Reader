#include <iostream>
#include <vector>
#include <string>
#include "dtknastrangraphpchdatastore.h"
#include "dtknastrangraphpchparser.h"

// 简单的翻译函数，模拟 UI 层名称转换
std::string translateComp(Component comp) {
    switch (comp) {
    case Component::SX: return "SX (Normal Stress/Strain)";
    case Component::T1: return "T1 (Translation X)";
    case Component::STRAIN_ENERGY: return "Strain Energy";
    case Component::VON_MISES: return "Von Mises";
    default: return "Other Component";
    }
}

std::string translateLoc(LocationType loc) {
    switch (loc) {
    case LocationType::Z1: return "Z1 (Top)";
    case LocationType::Z2: return "Z2 (Bottom)";
    case LocationType::CENTER: return "Center";
    default: return "Single";
    }
}

int main()
{
    // 1. 初始化数据池和解析器
    PchDataStore store;
    PchParser parser(store);

    // 2. 指定 PCH 文件路径进行解析
    std::string pchPath = "K11_TB_mode_20180123.pch";

    if (!parser.parse(pchPath)) {
        std::cerr << "Failed to open or parse PCH file!" << std::endl;
        return -1;
    }

    // 3. 模拟 UI 树状结构的遍历展示
    std::cout << "\n--- UI Navigation Tree Structure ---" << std::endl;
    for (auto const& subcasePair : store.m_uiNavigationTree)
    {
        int subID = subcasePair.first;
        std::cout << "Subcase: " << subID << std::endl;

        for (auto const& module : subcasePair.second)
        {
            std::cout << "  |- Category: " << module.category
                << " | Element Type: " << module.elementType << std::endl;
        }
    }

    // 4. 模拟用户点击：假设用户选择了 Subcase 1, Category "STRAIN", ElementType 82
    // 并想提取 Element ID 为 24 的 SX 分量 (Z1层)
    std::cout << "\n--- Fetching Curve Data ---" << std::endl;

    std::vector<double> xCoords;
    std::vector<double> yCoords;

    // 参数：Subcase, ElementType, ParentID, GridID, Location, Component
    store.getCurveData(2, 0, 7012, 0, LocationType::SINGLE, Component::T1_PHASE, xCoords, yCoords);

    if (xCoords.empty())
    {
        std::cout << "No data found for the specified criteria." << std::endl;
    }
    else
    {
        std::cout << "Found " << xCoords.size() << " points for Element 24 SX (Z1):" << std::endl;
        for (size_t i = 0; i < xCoords.size(); ++i)
        {
            std::cout << "  X: " << xCoords[i] << " \t Y: " << yCoords[i] << std::endl;
        }
    }

    std::cout << "\n--- Test Completed ---" << std::endl;
    return 0;
}