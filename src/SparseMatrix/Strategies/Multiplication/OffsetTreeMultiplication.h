#ifndef OFFSET_TREE_MULTIPLICATION_H
#define OFFSET_TREE_MULTIPLICATION_H
#include "../../CommonOffset.h"
#include "IMultiplicationStrategy.h"
#include <memory>
class OFfsetTreeMultiplication : public IMultiplicationStrategy {
  private:
    size_t m_RangeElementsPerThread = 20;

  public:
    ~OFfsetTreeMultiplication() {}

    ISparseMatrix *multiply(ISparseMatrix *A, ISparseMatrix *B,
                            ISparseMatrix *C) {

        m_Multi = 0;
        if (A->size() == 0 || B->size() == 0) {
            return C;
        }

        std::unique_ptr<CommonOffsets> common = findCommonIndices(A, B);
        assert(common);

        if (!common->actualSize) {
            common.get()->offsets.clear();
            common.reset();

            return C;
        }

        std::vector<int> t;

        const std::vector<FlatNode> &myNodes = A->getNodes();
        const std::vector<FlatNode> &otherNodes = B->getNodes();

        std::vector<FlatChildEntry> &flatChildren = A->getFlatChildren();
        std::vector<FlatChildEntry> &otherChildren = B->getFlatChildren();

      
        for (const CommonOffset &offset : common.get()->offsets) {

            int left = offset.indexLeft;
            int right = offset.indexRight;

            int leftNode = flatChildren[left].nodeIndex;
            int rightNode = otherChildren[right].nodeIndex;

            FlatNode visitLeft = (myNodes)[leftNode];
            FlatNode visitRight = (otherNodes)[rightNode];

            t.push_back(offset.tupleKey);

            if (visitLeft.isLeaf && visitRight.isLeaf) {
                // do the operation
                m_Multi++;
                double result = visitLeft.value * visitRight.value;
                C->insert(t, result);
                t.pop_back();
                continue;
            }

            reduceTree(visitLeft, visitRight, A, B, C, &t, 1);
            t.pop_back();
        }

        common.get()->offsets.clear();
        common.reset();
        return C;
    }

    std::unique_ptr<CommonOffsets> findCommonIndices(
                                                     ISparseMatrix *me,
                                                     ISparseMatrix *other) {

        FlatNode visitLeft = me->getNodes()[0];
        FlatNode visitRight = other->getNodes()[0];
        // lets find the root nodes for each
        std::vector<CommonOffset> offsets;
        auto info = collectIndexInformation(visitLeft, visitRight, me, other);
        int j = info->rightIndices[0];

        for (int i = 0; i < info->maxOffsetLeft; i++) {
            int indexLeft = info->leftIndices[i];
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

        auto ptr = std::unique_ptr<CommonOffsets>{
            new CommonOffsets{offsets, info->maxSize, (int)offsets.size()}};
        info.release();
        return ptr;
    }
};

#endif