#ifndef FLAT_INDEX_H
#define FLAT_INDEX_H
#include <vector>
struct FlatIndex {
    std::vector<int> leftIndices;
    std::vector<int> leftIndexPos;

    std::vector<int> rightIndices;
    std::vector<int> rightIndexPos;

    int maxOffsetLeft;
    int maxOffsetRight;
    int maxSize;
};
#endif