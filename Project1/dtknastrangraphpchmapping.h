#ifndef DTKNASTRANGRAPHPCHMAPPING_H
#define DTKNASTRANGRAPHPCHMAPPING_H
#include "dtknastranpchdefinitions.h"
#include <map>

/*
* 以下面这个pch文件数据为例
    2.000000E+01 G      8.842153E-01      3.250077E-01      3.285045E-01
-CONT-                  0.000000E+00      0.000000E+00      0.000000E+00
-CONT-                  4.432753E+00      1.814928E+02      3.527903E+02
-CONT-                  0.000000E+00      0.000000E+00      0.000000E+00
    2.100000E+01 G      7.956031E-01      3.151495E-01      3.824907E-01
-CONT-                  0.000000E+00      0.000000E+00      0.000000E+00
-CONT-                  6.034775E+00      1.832411E+02      3.525269E+02
-CONT-                  0.000000E+00      0.000000E+00      0.000000E+00
    ...
* 在 Nastran 的 pch 格式中，每一行被划分为 5 个字段（每个字段 16 字符宽）。Word 就是这些字段的编号。
比如第一行2.000000E+01     G      8.842153E-01      3.250077E-01      3.285045E-01 分别是五个字段
而第二行在   -CONT-  的后面是一个占16字符宽的大长空格，也视作一个字段，也是五个字段

* repeatCount (重复次数)：
通常用于单元结果。一个单元（如 CQUAD4）不仅输出中心点（CEN）的结果，
还可能输出 4 个角点的结果。repeatCount = 5 就意味着在同一个数据块里，这个单元会重复出现 5 次类似的数据结构。
上面文件中repeatCount = 1，它不是单元结果，单元结果详见其它类型的pch文件

* wordsPerPoint (每个点占用的总字数)：
代表一个完整的物理实体（如一个单元、一个节点）在文件中占用的“格子”总数。这决定了我们要连续读取多少个 16 字符宽的字段。
上面的数据中wordPerPoint = 12，因为
Word 1: Frequency (2.0000E+01)

Word 2: 标识符 (G)

Word 3-5: T1, T2, T3 的幅值

Word 6-8: R1, R2, R3 的幅值

Word 9-11: T1, T2, T3 的相位

Word 12-14: R1, R2, R3 的相位

但是我们只求实际的物理数据，所以在解析时我们会跳过 Word 2（标识符 G）和 Word 1（频率或-CONT-或节点ID），直接从 Word 3 开始处理物理数据。
频率在读取数据之前存入
*/

struct ElementLayout {
    int repeatCount = 0;
    int wordsPerPoint = 0;
    int dataStartWord = 0;
    int wordGridID = 0;//只有GridID和数据在一起的时候才用填写这个值，一般是有ParentID和GridID的单元应力应变才会有
    struct Info { Component comp; LocationType loc; };
    std::map<int, Info> wordToInfo;//当前读到的第i个word，它的物理意义是什么？比如读到第3个word，它代表了tz的幅值
};

class PchMapping {
public:
    ElementLayout getLayout(int elementType, ResultCategory category,bool isComplex,bool isMagPhase);
};
#endif
