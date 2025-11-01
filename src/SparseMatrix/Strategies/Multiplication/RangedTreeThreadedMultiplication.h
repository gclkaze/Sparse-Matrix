#ifndef RANGED_TREE_THREADED_MULTIPLICATION_H
#define RANGED_TREE_THREADED_MULTIPLICATION_H

#include <thread>
#include <vector>

#include "../../CommonOffset.h"
#include "../../FlatNode.h"
#include "IMultiplicationStrategy.h"

class RangedTreeThreadedMultiplication : public IMultiplicationStrategy {
   private:
    size_t m_RangeElementsPerThread = 20;

   public:
    ~RangedTreeThreadedMultiplication() {}

    ISparseMatrix* multiply(ISparseMatrix* A, ISparseMatrix* B, ISparseMatrix* C) {
        m_Multi = 0;
        if (A->size() == 0 || B->size() == 0) {
            return C;
        }

        findRangedThreadedCommonIndices(A, B, C);

        return C;
    }

   private:
    void findRangedThreadedCommonIndices(ISparseMatrix* me, ISparseMatrix* other,
                                         ISparseMatrix* result) {
        FlatNode visitLeft = me->getNodes()[0];
        FlatNode visitRight = other->getNodes()[0];

        auto info = collectIndexInformation(visitLeft, visitRight, me, other);
        std::vector<std::thread> workers;
        // each thread, it handles m_RangeElementsPerThread indices
        std::vector<CommonOffset> offsets;
        offsets.reserve(m_RangeElementsPerThread);

        int j = info->leftIndexPos[0];
        for (int i = 0; i < info->maxOffsetLeft; i++) {
            int indexLeft = info->leftIndices[i];
            if (j < info->maxOffsetRight && indexLeft < info->rightIndices[j]) {
                continue;
            }
            for (; j < info->maxOffsetRight; j++) {
                int indexRight = info->rightIndices[j];
                if (indexLeft == indexRight) {
                    offsets.push_back({info->leftIndexPos[i], info->rightIndexPos[j], indexRight});

                    if (offsets.size() == m_RangeElementsPerThread) {
                        workers.emplace_back(
                            RangedTreeThreadedMultiplication::parallelRangedMultiplication, this,
                            offsets, me, other, result);

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

        for (auto& t : workers) {
            if (t.joinable()) {
                t.join();
            }
        }
        info.release();
        return;
    }

    void parallelRangedMultiplication(std::vector<CommonOffset> offsets, ISparseMatrix* me,
                                      ISparseMatrix* other, ISparseMatrix* destination) {
        const std::vector<FlatNode>& myNodes = me->getNodes();
        const std::vector<FlatNode>& otherNodes = other->getNodes();

        std::vector<FlatChildEntry>& flatChildren = me->getFlatChildren();
        std::vector<FlatChildEntry>& otherChildren = other->getFlatChildren();

        for (const CommonOffset& offset : offsets) {
            std::vector<int> t;

            int left = offset.indexLeft;
            int right = offset.indexRight;

            int leftNode = flatChildren[left].nodeIndex;
            int rightNode = otherChildren[right].nodeIndex;

            FlatNode visitLeft = myNodes[leftNode];
            FlatNode visitRight = otherNodes[rightNode];

            t.push_back(offset.tupleKey);

            if (visitLeft.isLeaf && visitRight.isLeaf) {
                // do the operation
                m_Multi++;
                double result = visitLeft.value * visitRight.value;
                destination->insert(t, result);
                t.pop_back();
                continue;
            }

            reduceTree(visitLeft, visitRight, me, other, destination, &t, 1);
            t.pop_back();
        }
    }
};
#endif