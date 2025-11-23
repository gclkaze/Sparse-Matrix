#ifndef INDEX_INTERSECTION_FINDER_H
#define INDEX_INTERSECTION_FINDER_H
#include "../../FlatIndex.h"
#include "../../CommonOffset.h"

class IndexIntersectionFinder {
   public:
    IndexIntersectionFinder() {}
    ~IndexIntersectionFinder() {}

    std::vector<CommonOffset> find( FlatIndex& info) {
        std::vector<CommonOffset> offsets;
        int j = info.leftIndexPos[0];
        for (int i = 0; i < info.maxOffsetLeft; i++) {
            int indexLeft = info.leftIndices[i];
            if (j < info.maxOffsetRight && indexLeft < info.rightIndices[j]) {
                continue;
            }
            for (; j < info.maxOffsetRight; j++) {
                int indexRight = info.rightIndices[j];
                if (indexLeft == indexRight) {
                    offsets.push_back({info.leftIndexPos[i], info.rightIndexPos[j], indexRight});

                    j++;
                    break;
                }
                if (indexLeft < indexRight) {
                    break;
                }
            }
        }
        return offsets;
    }
};
#endif