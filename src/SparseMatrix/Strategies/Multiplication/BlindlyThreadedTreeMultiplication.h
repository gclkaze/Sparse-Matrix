#ifndef BLINDLY_THREADED_TREE_MULTIPLICATION_H
#define BLINDLY_THREADED_TREE_MULTIPLICATION_H
#include "../../CommonOffset.h"
#include "IMultiplicationStrategy.h"

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

        findThreadedCommonIndices(visitLeft, visitRight, A, B,C);
        return C;
    }

  private:
    void findThreadedCommonIndices(FlatNode &visitLeft, FlatNode &visitRight,
                                   ISparseMatrix *me, ISparseMatrix *other, ISparseMatrix *result) {

        int offsetLeft = visitLeft.childOffset;
        int maxOffsetLeft = visitLeft.numChildren;

        std::vector<int> indicesLeft;
        indicesLeft.reserve(maxOffsetLeft);

        std::vector<int> indicesLeftPos;
        indicesLeftPos.reserve(maxOffsetLeft);

        std::vector<FlatChildEntry> &myflatChildren = me->getFlatChildren();

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
        std::vector<FlatChildEntry> &otherflatChildren =
            other->getFlatChildren();

        for (int i = offsetRight; i < offsetRight + maxOffsetRight; i++) {
            indicesRight.push_back(otherflatChildren[i].tupleIndex);
            indicesRightPos.push_back(i);
        }

        int maxSize =
            maxOffsetLeft < maxOffsetRight ? maxOffsetRight : maxOffsetLeft;

        std::vector<std::thread> workers;

        // lets find common root nodes
        int j = indicesRight[0];
        for (int i = 0; i < maxOffsetLeft; i++) {
            int indexLeft = indicesLeft[i];
            if (j < maxOffsetRight && indexLeft < indicesRight[j]) {
                continue;
            }
            for (; j < maxOffsetRight; j++) {
                int indexRight = indicesRight[j];
                if (indexLeft == indexRight) {
                    workers.emplace_back(BlindlyThreadedTreeMultiplication::
                                             parallelMultiplication,
                                         this, indicesLeftPos[i],
                                         indicesRightPos[j], indexRight, me, other,
                                         result);

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
        indicesLeft.clear();
        indicesRight.clear();

        indicesRight.clear();
        indicesLeftPos.clear();
        return;
    }

    void parallelMultiplication(int indexLeft, int indexRight, int key,
                                ISparseMatrix *me, ISparseMatrix *other,
                                ISparseMatrix *destination) {
        std::vector<int> t;

         const std::vector<FlatNode>& myNodes = me->getNodes();
         const  std::vector<FlatNode>& otherNodes = other->getNodes();

         std::vector<FlatChildEntry>& flatChildren = me->getFlatChildren();
         std::vector<FlatChildEntry>& otherChildren = other->getFlatChildren();

        int left = indexLeft;
        int right = indexRight;

        int leftNode = flatChildren[left].nodeIndex;
        int rightNode = otherChildren[right].nodeIndex;

        FlatNode visitLeft = myNodes[leftNode];
        FlatNode visitRight = otherNodes[rightNode];

        t.push_back(key);
        reduceTree(visitLeft, visitRight, me, other, destination, &t, 1);
        t.pop_back();
    }
};
#endif