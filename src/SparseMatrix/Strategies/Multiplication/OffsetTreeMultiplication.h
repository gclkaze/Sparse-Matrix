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

        FlatNode visitLeft = A->getNodes()[0];
        FlatNode visitRight = B->getNodes()[0];

        std::unique_ptr<CommonOffsets> common =
            findCommonIndices(visitLeft, visitRight, A, B);
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

            visitLeft = (myNodes)[leftNode];
            visitRight = (otherNodes)[rightNode];

            t.push_back(offset.tupleKey);
            
            if (visitLeft.isLeaf && visitRight.isLeaf) {
                // do the operation
                m_Multi++;
                double result = visitLeft.value * visitRight.value;
                C->insert(t, result);
                t.pop_back();
                continue;
            }

            reduceTree(visitLeft, visitRight, A, B,C, &t, 1);
            t.pop_back();
        }

        common.get()->offsets.clear();
        common.reset();
        return C;
    }

    std::unique_ptr<CommonOffsets> findCommonIndices(FlatNode &visitLeft,
                                         FlatNode &visitRight,
                                          ISparseMatrix *me,
                                          ISparseMatrix *other) {

        // lets find the root nodes for each
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
        std::vector<CommonOffset> offsets;

        int j = indicesRight[0];
        for (int i = 0; i < maxOffsetLeft; i++) {
            int indexLeft = indicesLeft[i];
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

        auto ptr = std::unique_ptr<CommonOffsets>{
            new CommonOffsets{offsets, maxSize, (int)offsets.size()}};
        return ptr;
    }
};

#endif