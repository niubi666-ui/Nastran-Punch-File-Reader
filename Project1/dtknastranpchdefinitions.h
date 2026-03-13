#ifndef DTKNASTRANPCHDEFINITIONS_H
#define DTKNASTRANPCHDEFINITIONS_H

#include <string>

enum class Component {
    NONE = 0,
    // 基础位移/加速度分量
    T1, T2, T3, R1, R2, R3,
    //幅值相位
    T1_MAG, T1_PHASE, T1_REAL, T1_IMAG,
    T2_MAG, T2_PHASE, T2_REAL, T2_IMAG,
    T3_MAG, T3_PHASE, T3_REAL, T3_IMAG,
    R1_MAG, R1_PHASE, R1_REAL, R1_IMAG,
    R2_MAG, R2_PHASE, R2_REAL, R2_IMAG,
    R3_MAG, R3_PHASE, R3_REAL, R3_IMAG,
    // 应力/应变分量 (SX, SY, SZ, TXY, TYZ, TZX)
    SX, SY, SZ, TXY, TYZ, TZX,
    P1, P2, P3, VON_MISES,
    // 应变能分量
    STRAIN_ENERGY, ENERGY_PERCENT, ENERGY_DENSITY
};

enum class LocationType {
    SINGLE = 0,
    CENTER,     // CEN
    CORNER,     // 节点处
    Z1,         // Top
    Z2          // Bottom
};

// 原子数据条目
struct PchEntry {
    int subcase;
    int parentID;   // Element 或 Node ID
    int gridID;     // 关联的 Grid ID (如果没有则为0)
    LocationType loc;
    Component comp;
    float xVal;     // 频率、时间或 Subcase ID
    float yVal;     // 物理数值
};

// UI 导航树的节点关键字
struct ResultModule {
    std::string category; // "STRESS", "STRAIN", "DISPLACEMENT", "ENERGY"
    int elementType;      // Nastran ID (如 34, 82, 39)，标量结果为 -1

    bool operator<(const ResultModule& other) const {
        if (category != other.category) return category < other.category;
        return elementType < other.elementType;
    }
};

#endif