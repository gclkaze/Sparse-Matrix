#ifndef RANGED_TREE_THREADED_MULTIPLICATION_H
#define RANGED_TREE_THREADED_MULTIPLICATION_H

#include "../../CommonOffset.h"
#include "../../FlatNode.h"
#include "IMultiplicationStrategy.h"
#include <vector>
#include <thread>

class RangedTreeThreadedMultiplication : public IMultiplicationStrategy {
  private:
    size_t m_RangeElementsPerThread = 20;

  public:
    ~RangedTreeThreadedMultiplication() {}

    ISparseMatrix* multiply( ISparseMatrix *A,  ISparseMatrix *B,ISparseMatrix *C) {

        m_Multi = 0;
        if (A->size() == 0 || B->size() == 0) {
            return C;
        }

        FlatNode visitLeft =  A->getNodes()[0];
        FlatNode visitRight = B->getNodes()[0];

        findRangedThreadedCommonIndices(visitLeft, visitRight, A, B, C);

        return C;
    }

  private:
    void findRangedThreadedCommonIndices(FlatNode &visitLeft,
                                         FlatNode &visitRight,
                                          ISparseMatrix *me,
                                          ISparseMatrix *other,
                                         ISparseMatrix *result) {

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

        std::vector<FlatChildEntry>& otherflatChildren = other->getFlatChildren();

        for (int i = offsetRight; i < offsetRight + maxOffsetRight; i++) {
            indicesRight.push_back(otherflatChildren[i].tupleIndex);
            indicesRightPos.push_back(i);
        }

        int maxSize =
            maxOffsetLeft < maxOffsetRight ? maxOffsetRight : maxOffsetLeft;

        std::vector<std::thread> workers;

        // each thread, it handles m_RangeElementsPerThread indices
        std::vector<CommonOffset> offsets;
        offsets.reserve(m_RangeElementsPerThread);

        int j = indicesRightPos[0];
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

                    if (offsets.size() == m_RangeElementsPerThread) {
                         workers.emplace_back(
                           RangedTreeThreadedMultiplication::parallelRangedMultiplication, this,
                            offsets,  me, other, result);

                        offsets.clear();
                    }
                    j++;
                    break;
                }
                if (indexLeft < indexRight) {
                    break;
                }
            }
        }

        if (!offsets.empty()) {
         workers.emplace_back(RangedTreeThreadedMultiplication::parallelRangedMultiplication,
                                 this, offsets, me, other, result);
        }

        for (auto &t : workers) {
            if (t.joinable()) {
                t.join();
            }
        }
        return;
    }

    void parallelRangedMultiplication(std::vector<CommonOffset> offsets,
                                       ISparseMatrix *me,
                                       ISparseMatrix *other,
                                      ISparseMatrix *destination) {

         const std::vector<FlatNode>& myNodes = me->getNodes();
         const  std::vector<FlatNode>& otherNodes = other->getNodes();

         std::vector<FlatChildEntry>& flatChildren = me->getFlatChildren();
         std::vector<FlatChildEntry>& otherChildren = other->getFlatChildren();

        for (const CommonOffset &offset : offsets) {
            std::vector<int> t;

            int left = offset.indexLeft;
            int right = offset.indexRight;

            int leftNode = flatChildren[left].nodeIndex;
            int rightNode = otherChildren[right].nodeIndex;

            FlatNode visitLeft = myNodes[leftNode];
            FlatNode visitRight = otherNodes[rightNode];

            t.push_back(offset.tupleKey);
            reduceTree(visitLeft, visitRight,me, other, destination, &t, 1);
            t.pop_back();
        }
    }
};
#endif