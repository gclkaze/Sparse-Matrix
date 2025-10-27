#ifndef COMMON_OFFSET_H
#define COMMON_OFFSET_H
#include <iostream>
struct CommonOffset {
    int indexLeft;
    int indexRight;
    int tupleKey;
};

struct CommonOffsets {
    CommonOffset *offsets;
    int maxSize;
    int actualSize;
   // int *commonRoots;

    void dump() {
        for (int i = 0; i < actualSize; i++) {
            std::cout << i << " : " << offsets[i].indexLeft << ","
                      << offsets[i].indexRight << " = " << offsets[i].tupleKey
                      << std::endl;
        }
    }
};
#endif