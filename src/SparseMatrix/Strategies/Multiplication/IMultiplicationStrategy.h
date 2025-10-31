#ifndef IMULTIPLICATION_STRATEGY_H
#define IMULTIPLICATION_STRATEGY_H
#include "../../ISparseMatrix.h"
#include <vector>
#include <assert.h>

class IMultiplicationStrategy {
    friend class SparseMatrix;
    protected :
       int m_Multi = 0;
    public:
    virtual ISparseMatrix* multiply( ISparseMatrix *A,   ISparseMatrix *B, ISparseMatrix *C) = 0;
    virtual ~IMultiplicationStrategy() {}

    void reduceTree(  FlatNode &visitLeft,    FlatNode &visitRight,  ISparseMatrix *me,
                    ISparseMatrix *other, ISparseMatrix *destination,
                    std::vector<int> *currentTuple, int tupleMaxSize) {
        int offsetLeft = visitLeft.childOffset;
        int maxOffsetLeft = visitLeft.numChildren;

        std::vector<int> indicesLeft;
        indicesLeft.reserve(maxOffsetLeft);

        std::vector<int> indicesLeftPos;
        indicesLeftPos.reserve(maxOffsetLeft);

         std::vector<FlatChildEntry>& myflatChildren = me->getFlatChildren();

        for (int i = offsetLeft; i < offsetLeft + maxOffsetLeft; i++) {
            indicesLeft.push_back(myflatChildren[i].tupleIndex);
            indicesLeftPos.push_back(i);
        }

        int offsetRight = visitRight.childOffset;
        int maxOffsetRight = visitRight.numChildren;

        std::vector<int> indicesRight;
        indicesRight.reserve(maxOffsetRight);

        std::vector<int> indicesRightPos;
        indicesRightPos.reserve(maxOffsetRight);

        std::vector<FlatChildEntry>& flatChildren = other->getFlatChildren();

        for (int i = offsetRight; i < offsetRight + maxOffsetRight; i++) {
            indicesRight.push_back(flatChildren[i].tupleIndex);
            indicesRightPos.push_back(i);
        }

        int maxSize =
            maxOffsetLeft < maxOffsetRight ? maxOffsetRight : maxOffsetLeft;

        std::vector<CommonOffset> offsets;
        offsets.reserve(maxSize);

        // lets find common root nodes
        int j = 0;//indicesRight[0];
        for (int i = 0; i < maxOffsetLeft; i++) {
            int indexLeft = indicesLeft[i];
            if (j < maxOffsetRight && indexLeft < indicesRight[j]) {
                continue;
            }
            for (; j < maxOffsetRight; j++) {
                int indexRight = indicesRight[j];
                if (indexLeft == indexRight) {
                    offsets.push_back(
                        {indicesLeftPos[i], indicesRightPos[j], indexRight});
                    j++;
                    break;
                }
                if (indexLeft < indexRight) {
                    break;
                }
            }
        }
        indicesLeft.clear();
        indicesRight.clear();
        indicesLeftPos.clear();
        indicesRightPos.clear();

        const std::vector<FlatNode>& myNodes = me->getNodes();
        const std::vector<FlatNode>& otherNodes = other->getNodes();

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
            reduceTree(visitLeft, visitRight, me, other, destination, currentTuple,
                       tupleMaxSize + 1);
            currentTuple->pop_back();
        }

        offsets.clear();
        return;
    }
};
#endif