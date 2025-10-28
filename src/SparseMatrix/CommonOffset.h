#ifndef COMMON_OFFSET_H
#define COMMON_OFFSET_H
#include <iostream>
#include <vector>
struct CommonOffset {
    int indexLeft;
    int indexRight;
    int tupleKey;
};

struct CommonOffsets {
    std::vector<CommonOffset> *offsets;
    int maxSize;
    int actualSize;
   // int *commonRoots;

    void dump() {
        for (int i = 0; i < actualSize; i++) {
            CommonOffset offset = (*offsets)[i];

            std::cout << i << " : " << offset.indexLeft << ","
                      << offset.indexRight << " = " << offset.tupleKey
                      << std::endl;
        }
    }
};
#endif