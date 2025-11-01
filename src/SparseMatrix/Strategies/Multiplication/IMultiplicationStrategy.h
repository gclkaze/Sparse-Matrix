#ifndef IMULTIPLICATION_STRATEGY_H
#define IMULTIPLICATION_STRATEGY_H
#include "../../FlatIndex.h"
#include "../../ISparseMatrix.h"
#include <assert.h>
#include <memory>
#include <vector>

class IMultiplicationStrategy {
    friend class SparseMatrix;

  protected:
    int m_Multi = 0;

  public:
    virtual ISparseMatrix *multiply(ISparseMatrix *A, ISparseMatrix *B,
                                    ISparseMatrix *C) = 0;
    virtual ~IMultiplicationStrategy() {}

    void reduceTree(FlatNode &visitLeft, FlatNode &visitRight,
                    ISparseMatrix *me, ISparseMatrix *other,
                    ISparseMatrix *destination, std::vector<int> *currentTuple,
                    int tupleMaxSize) {

        auto info = collectIndexInformation(visitLeft, visitRight, me, other);

        std::vector<CommonOffset> offsets;
        offsets.reserve(info->maxSize);

        // lets find common root nodes
        int j = 0; // indicesRight[0];
        for (int i = 0; i < info->maxOffsetLeft; i++) {
            int indexLeft = info->leftIndices[i];
            if (j < info->maxOffsetRight && indexLeft < info->rightIndices[j]) {
                continue;
            }
            for (; j < info->maxOffsetRight; j++) {
                int indexRight = info->rightIndices[j];
                if (indexLeft == indexRight) {
                    offsets.push_back({info->leftIndexPos[i],
                                       info->rightIndexPos[j], indexRight});
                    j++;
                    break;
                }
                if (indexLeft < indexRight) {
                    break;
                }
            }
        }
        info->leftIndices.clear();
        info->rightIndices.clear();
        info->leftIndexPos.clear();
        info->rightIndexPos.clear();

        const std::vector<FlatChildEntry> &myflatChildren =
            me->getFlatChildren();
        const std::vector<FlatChildEntry> &flatChildren =
            other->getFlatChildren();

        const std::vector<FlatNode> &myNodes = me->getNodes();
        const std::vector<FlatNode> &otherNodes = other->getNodes();

        for (const CommonOffset &offset : offsets) {
            int left = offset.indexLeft;
            int right = offset.indexRight;

            int leftNode = myflatChildren[left].nodeIndex;
            int rightNode = flatChildren[right].nodeIndex;

            visitLeft = (myNodes)[leftNode];
            visitRight = (otherNodes)[rightNode];

            if (visitLeft.isLeaf && visitRight.isLeaf) {
                // do the operation
                m_Multi++;
                double result = visitLeft.value * visitRight.value;
                currentTuple->push_back(offset.tupleKey);
                destination->insert(&(*currentTuple)[0], tupleMaxSize + 1,
                                    result);
                currentTuple->pop_back();
                continue;
            }

            assert(!visitLeft.isLeaf && !visitRight.isLeaf);
            currentTuple->push_back(offset.tupleKey);
            reduceTree(visitLeft, visitRight, me, other, destination,
                       currentTuple, tupleMaxSize + 1);
            currentTuple->pop_back();
        }

        offsets.clear();
        info.release();
        return;
    }

    std::unique_ptr<FlatIndex>
    collectIndexInformation(const FlatNode &visitLeft,
                            const FlatNode &visitRight, ISparseMatrix *me,
                            ISparseMatrix *other) {
        auto ptr = std::make_unique<FlatIndex>();

        int offsetLeft = visitLeft.childOffset;
        int maxOffsetLeft = visitLeft.numChildren;

        ptr->leftIndices.reserve(maxOffsetLeft);
        ptr->leftIndexPos.reserve(maxOffsetLeft);

        const std::vector<FlatChildEntry> &myflatChildren =
            me->getFlatChildren();

        int max = offsetLeft + maxOffsetLeft;

        for (int i = offsetLeft; i < max; i++) {
            ptr->leftIndices.push_back(myflatChildren[i].tupleIndex);
            ptr->leftIndexPos.push_back(i);
        }

        int offsetRight = visitRight.childOffset;
        int maxOffsetRight = visitRight.numChildren;

        ptr->rightIndices.reserve(maxOffsetRight);
        ptr->rightIndexPos.reserve(maxOffsetRight);

        const std::vector<FlatChildEntry> &flatChildren =
            other->getFlatChildren();
        max = offsetRight + maxOffsetRight;

        for (int i = offsetRight; i < max; i++) {
            ptr->rightIndices.push_back(flatChildren[i].tupleIndex);
            ptr->rightIndexPos.push_back(i);
        }

        int maxSize =
            maxOffsetLeft < maxOffsetRight ? maxOffsetRight : maxOffsetLeft;

        ptr->maxSize = maxSize;
        ptr->maxOffsetLeft = maxOffsetLeft;
        ptr->maxOffsetRight = maxOffsetRight;

        return (ptr);
    }
};
#endif