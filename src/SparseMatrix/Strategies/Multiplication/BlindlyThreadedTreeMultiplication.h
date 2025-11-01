#ifndef BLINDLY_THREADED_TREE_MULTIPLICATION_H
#define BLINDLY_THREADED_TREE_MULTIPLICATION_H
#include "../../CommonOffset.h"
#include "IMultiplicationStrategy.h"
#include <thread>
class BlindlyThreadedTreeMultiplication : public IMultiplicationStrategy {

  public:
    ~BlindlyThreadedTreeMultiplication() {}

    ISparseMatrix *multiply(ISparseMatrix *A, ISparseMatrix *B,
                            ISparseMatrix *C) {
        m_Multi = 0;
        if (A->size() == 0 || B->size() == 0) {
            return C;
        }

        FlatNode visitLeft = A->getNodes()[0];
        FlatNode visitRight = B->getNodes()[0];

        findThreadedCommonIndices(visitLeft, visitRight, A, B, C);
        return C;
    }

  private:
    void findThreadedCommonIndices(FlatNode &visitLeft, FlatNode &visitRight,
                                   ISparseMatrix *me, ISparseMatrix *other,
                                   ISparseMatrix *result) {

        auto info = collectIndexInformation(visitLeft, visitRight, me, other);
        std::vector<std::thread> workers;

        // lets find common root nodes
        int j = info->rightIndices[0];
        for (int i = 0; i < info->maxOffsetLeft; i++) {
            int indexLeft = info->leftIndices[i];
            if (j < info->maxOffsetRight && indexLeft < info->rightIndices[j]) {
                continue;
            }
            for (; j < info->maxOffsetRight; j++) {
                int indexRight = info->rightIndices[j];
                if (indexLeft == indexRight) {
                    workers.emplace_back(BlindlyThreadedTreeMultiplication::
                                             parallelMultiplication,
                                         this, info->leftIndexPos[i],
                                         info->rightIndexPos[j], indexRight, me,
                                         other, result);

                    j++;
                    break;
                }
                if (indexLeft < indexRight) {
                    break;
                }
            }
        }
        for (auto &t : workers) {
            if (t.joinable()) {
                t.join();
            }
        }

        info.release();
        return;
    }

    void parallelMultiplication(int indexLeft, int indexRight, int key,
                                ISparseMatrix *me, ISparseMatrix *other,
                                ISparseMatrix *destination) {
        std::vector<int> t;

        const std::vector<FlatNode> &myNodes = me->getNodes();
        const std::vector<FlatNode> &otherNodes = other->getNodes();

        std::vector<FlatChildEntry> &flatChildren = me->getFlatChildren();
        std::vector<FlatChildEntry> &otherChildren = other->getFlatChildren();

        int left = indexLeft;
        int right = indexRight;

        int leftNode = flatChildren[left].nodeIndex;
        int rightNode = otherChildren[right].nodeIndex;

        FlatNode visitLeft = myNodes[leftNode];
        FlatNode visitRight = otherNodes[rightNode];

        t.push_back(key);
        if (visitLeft.isLeaf && visitRight.isLeaf) {
            // do the operation
            m_Multi++;
            double result = visitLeft.value * visitRight.value;
            destination->insert(t, result);
            t.pop_back();
            return;
        }
        reduceTree(visitLeft, visitRight, me, other, destination, &t, 1);
        t.pop_back();
    }
};
#endif