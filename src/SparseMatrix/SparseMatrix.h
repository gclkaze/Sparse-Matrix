#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H
#include "FlatNode.h"
#include "SparseMatrixIterator.h"
#include <assert.h>
#include <iostream>
#include <vector>

class SparseMatrix {
    friend class SparseMatrixIterator;

    typedef enum SearchStatus { none, smaller, equal, larger } SearchStatus;

    std::vector<FlatNode> m_Nodes;
    std::vector<FlatChildEntry> m_FlatChildren;
    int m_Size = 0;

  public:
    void reset() {
        m_Nodes.clear();
        m_FlatChildren.clear();
        m_Size = 0;
    }

    SparseMatrixIterator iterator() {
        return SparseMatrixIterator(&m_Nodes, &m_FlatChildren, m_Size);
    }
    bool insert(const std::vector<int> &tuple, double value) {

        if (value == 0.0) {
            return erase(tuple);
        }

        assert(tuple.size() > 0);

        if (m_Nodes.size() == 0) {
            // root is empty
            m_Nodes.push_back({0, false, 0, 0, 0});
            put(tuple, value);
            return true;
        }
        put(tuple, value);
        return true;
    }

    const int size() { return m_Size; }

    void clear() {
        if (m_Size == 0) {
            return;
        }

        m_FlatChildren.clear();
        m_Nodes.clear();
    }

    bool erase(const std::vector<int> &tuple) {
        // we need to check if the tuple exists.
        int tupleSize = static_cast<int>(tuple.size());
        if (tupleSize == 0) {
            return false;
        }

        if (m_FlatChildren.size() == 0) {
            return false;
        }
        FlatNode *current = &m_Nodes[0];
        SearchStatus status = none;
        int flatChildrenPos = 0;
        int node = -1;

        std::vector<int> entries;
        std::vector<int> visitedNodes;

        entries.reserve(tupleSize);
        visitedNodes.reserve(tupleSize + 1);

        visitedNodes.push_back(0);

        for (int i = 0; i < tupleSize; i++) {
            flatChildrenPos = this->findTheTuple(tuple[i], current->numChildren,
                                                 current->childOffset, &status);
            if (status != equal) {
                // not a match
                return false;
            }
            node = m_FlatChildren[flatChildrenPos].nodeIndex;
            current = &m_Nodes[node];
            entries.push_back(flatChildrenPos);
            visitedNodes.push_back(
                this->m_FlatChildren[flatChildrenPos].nodeIndex);
        }

        deleteTuple(entries, visitedNodes);
        return true;
    }

    SparseMatrix operator*(SparseMatrix &other) {
        SparseMatrixIterator it = iterator();
        SparseMatrixIterator otherIt = other.iterator();

        SparseMatrix result;
        SparseMatrixTuple myTuple;
        SparseMatrixTuple otherTuple;

        bool myEnd = false;
        bool otherEnd = false;
        bool inserted = false;

        myTuple = *it;
        otherTuple = *otherIt;

        while (true) {
            if (myTuple == otherTuple) {
                result.insert(myTuple.tuple, myTuple.value * otherTuple.value);
                inserted = true;
            }

            myEnd = it.ended();
            otherEnd = otherIt.ended();

            if (!myEnd || !otherEnd) {
                if (!inserted) {
                    if (!myEnd && !otherEnd) {
                        if (myTuple < otherTuple) {
                            myTuple = *it;
                        } else {
                            otherTuple = *otherIt;
                        }
                    } else if (myEnd && !otherEnd) {
                        otherTuple = *otherIt;
                    } else if (!myEnd && otherEnd) {
                        myTuple = *it;
                    }
                } else {
                    myTuple = *it;
                    otherTuple = *otherIt;
                    inserted = false;
                }
            } else {
                break;
            }
        }
        return result;
    }

    SparseMatrix oldMultiplication(SparseMatrix &other) {
        SparseMatrixIterator it = iterator();
        SparseMatrixIterator otherIt = other.iterator();

        SparseMatrix result;
        SparseMatrixTuple myTuple = *it;
        SparseMatrixTuple otherTuple = *otherIt;

        bool myEnd = it.ended();
        bool otherEnd = otherIt.ended();

        while (true) {

            if (myEnd && otherEnd) {
                break;
            } else if (!myEnd && otherEnd) {
                myTuple = *it;
            } else if (myEnd && !otherEnd) {
                otherTuple = *otherIt;
            }

            if (myTuple == otherTuple) {
                result.insert(myTuple.tuple, myTuple.value * otherTuple.value);
                myTuple = *it;
                otherTuple = *otherIt;
            } else if (myTuple < otherTuple) {
                myTuple = *it;
            } else {
                otherTuple = *otherIt;
            }

            myEnd = it.ended();
            otherEnd = otherIt.ended();
        }

        return result;
    }

  private:
    void put(const std::vector<int> &tuple, const double value) {

        int tupleSize = static_cast<int>(tuple.size());
        if (tupleSize == 0) {
            return;
        }
        FlatNode *current = &m_Nodes[0];
        SearchStatus status = none;
        int flatChildrenPos = 0;
        bool notAMatch = false;
        int node = -1;
        int nodesAmount = -1;
        bool insertion = false;

        for (int i = 0; i < tupleSize; i++) {
            if (!notAMatch) {
                flatChildrenPos =
                    this->findTheTuple(tuple[i], current->numChildren,
                                       current->childOffset, &status);
            }

            switch (status) {
            case larger:
            case smaller: {
                current->numChildren++;
                nodesAmount = m_Nodes.size();
                m_FlatChildren.insert(m_FlatChildren.begin() + flatChildrenPos,
                                      {tuple[i], nodesAmount});
                updateChildOffsets(current, nodesAmount);
                current = saveNode(i + 1 < tupleSize, nodesAmount, value,
                                   current, false);
                status = none;
                notAMatch = true;
                insertion = true;
                break;
            }
            case equal: {
                // it is a match, we need to go further to the next index
                node = m_FlatChildren[flatChildrenPos].nodeIndex;
                nodesAmount = m_Nodes.size();
                assert(node < nodesAmount);
                current = &m_Nodes[node];
                break;
            }

            case none: {
                // the path does not exist at all
                current->numChildren++;

                // we did our stuff with the parent node, lets continue to the
                // new nodes,
                nodesAmount = m_Nodes.size();
                current = saveNode(i + 1 < tupleSize, nodesAmount, value,
                                   current, true);
                m_FlatChildren.push_back({tuple[i], nodesAmount});
                insertion = true;
                break;
            }
            default: {
                assert(false);
                break;
            }
            }
        }

        if (!insertion) {
            current = updateNotZeroValue(current, value);
        }
    }

    int findMinIndex(const std::vector<bool> &zeroChildrenRemaining,
                     const std::vector<int> nodePositions) {
        int min = m_Nodes.size();
        for (int i = 0; i < (int)zeroChildrenRemaining.size(); i++) {
            if (zeroChildrenRemaining[i]) {
                int currentMin = nodePositions[i];
                if (currentMin < min) {
                    min = currentMin;
                }
            }
        }
        return min;
    }

    int findMaxIndex(const std::vector<bool> &zeroChildrenRemaining,
                     const std::vector<int> nodePositions) {
        int max = 0;
        for (int i = 0; i < (int)zeroChildrenRemaining.size(); i++) {
            if (zeroChildrenRemaining[i]) {
                int currentMax = nodePositions[i];
                if (currentMax > max) {
                    max = currentMax;
                }
            }
        }
        return max;
    }

    int findRemovedFlatChildrenBefore(const std::vector<int> &removed,
                                      int flatChildrenIndex) {
        int ded = 0;
        for (const int &line : removed) {
            if (flatChildrenIndex > line) {
                ded++;
            }
        }
        return ded;
    }

    void reduceNodeIndices(const std::vector<bool> &zeroChildrenRemaining,
                           const std::vector<int> &visitedNodes,
                           int zeroChildrenAppears) {
        int min = findMinIndex(zeroChildrenRemaining, visitedNodes);
        int deduction = zeroChildrenRemaining.size() - zeroChildrenAppears;
        int flatChildrenSize = (int)m_FlatChildren.size();

        for (int i = 0; i < flatChildrenSize; i++) {
            if (m_FlatChildren[i].nodeIndex > min) {
                m_FlatChildren[i].nodeIndex -= deduction;
            }
        }
    }

    void reduceChildOffsets(const std::vector<bool> &zeroChildrenRemaining,
                            const std::vector<int> &visitedNodes,
                            const int zeroChildrenAppears,
                            const std::vector<int> &entries) {
        int maxD = findMaxIndex(zeroChildrenRemaining, visitedNodes);
        int deduction = zeroChildrenRemaining.size() - zeroChildrenAppears;
        int nodesSize = (int)m_Nodes.size();

        for (int i = 1; i < nodesSize; i++) {
            if (m_Nodes[i].childOffset > maxD) {
                m_Nodes[i].childOffset -= deduction;
            } else if (m_Nodes[i].childOffset <= maxD) {
                int ded = findRemovedFlatChildrenBefore(entries,
                                                        m_Nodes[i].childOffset);
                m_Nodes[i].childOffset -= ded;
            }
        }
    }

    void deleteTuple(std::vector<int> &entries,
                     std::vector<int> &visitedNodes) {
        int i = 0;
        int max = entries.size();
        FlatNode *current = &m_Nodes[visitedNodes[i]];
        std::vector<bool> zeroChildrenRemaining;
        zeroChildrenRemaining.reserve(max);

        int zeroChildrenAppears = -1;
        FlatChildEntry *child = nullptr;

        while (i < max) {
            current->numChildren--;
            child = &m_FlatChildren[entries[i]];

            if (current->numChildren == 0) {
                if (zeroChildrenAppears == -1) {
                    zeroChildrenAppears = i;
                }
                // need to delete the subtree
                zeroChildrenRemaining.push_back(true);
            } else {
                // the subtree childrenOffsets need to be adjusted, there are
                // more children
                zeroChildrenRemaining.push_back(false);
            }

            i++;
            current = &m_Nodes[child->nodeIndex];
            assert(current != nullptr);
        }

        // handle flat children
        zeroChildrenRemaining.push_back(true);
        reduceNodeIndices(zeroChildrenRemaining, visitedNodes,
                          zeroChildrenAppears);

        // handle node child offsets
        reduceChildOffsets(zeroChildrenRemaining, visitedNodes,
                           zeroChildrenAppears, entries);
        assert(current->isLeaf);
        cleanNodesAndChildren(entries, visitedNodes);
        return;
    }

    void cleanNodesAndChildren(const std::vector<int> &entries,
                               const std::vector<int> &visitedNodes) {
        // clean flatChildren
        int entriesRemoved = 0;
        for (const int &pos : entries) {
            m_FlatChildren.erase(m_FlatChildren.begin() +
                                 (pos - entriesRemoved));
            entriesRemoved++;
        }

        // clean nodes
        entriesRemoved = 0;
        for (const int &pos : visitedNodes) {
            if (pos == 0) {
                continue;
            }
            if (m_Nodes[(pos - entriesRemoved)].isLeaf) {
                m_Size--;
            }
            m_Nodes.erase(m_Nodes.begin() + (pos - entriesRemoved));
            entriesRemoved++;
        }
    }

    FlatNode *updateNotZeroValue(FlatNode *current, const double value) {
        assert(current->isLeaf);
        if (value != 0 && value != current->value) {
            current->value = value;
        }
        return current;
    }
    FlatNode *saveNode(const bool isNotLeaf, const int nodesAmount,
                       const double value, FlatNode *current,
                       const bool childInsertedAtTheEnd) {
        FlatNode node;
        node.id = nodesAmount;

        if (isNotLeaf) {
            node.childOffset = (static_cast<int>(m_FlatChildren.size())) +
                               (childInsertedAtTheEnd ? 1 : 0);
            node.numChildren = 1;
            node.isLeaf = false;
            node.value = 0.0f;
        } else {
            node.childOffset = -1;
            node.numChildren = 0;
            node.isLeaf = true;
            node.value = value;
            m_Size++;
        }

        m_Nodes.emplace_back(node);
        current = &node;
        return current;
    }

    void updateChildOffsets(FlatNode *parent, int index) {
        index--;
        FlatNode *start = &m_Nodes[index];

        while (start != parent) {
            assert(index >= 0);
            if (start->isLeaf) {
                index--;
                start = &m_Nodes[index];
                continue;
            }
            m_Nodes[index].childOffset++;
            index--;
            start = &m_Nodes[index];
        }
    }

    int findTheTuple(int tupleKey, int numChildren, int childOffset,
                     SearchStatus *status) {

        int i = childOffset;
        int flatChildrenAmount = m_FlatChildren.size();
        int max = childOffset + numChildren;

        *status = none;

        if (flatChildrenAmount == 0) {
            return -1;
        }

        if (i >= flatChildrenAmount) {
            return -1;
        }
        for (; i < max; i++) {
            int storedTuple = m_FlatChildren[i].tupleIndex;
            if (tupleKey > storedTuple) {
                *status = larger;
                continue; // break;
            } else if (tupleKey < storedTuple) {
                *status = smaller;
                break;
            } else if (tupleKey == storedTuple) {
                *status = equal;
                break;
            }
        }
        return i;
    }

  public:
    double getValue(const std::vector<int> &tuple) {
        FlatNode *current = &m_Nodes[0];
        SearchStatus status = none;
        int nodeIndex = -1;
        int foundChild = -1;
        int tupleSize = static_cast<int>(tuple.size());

        for (int i = 0; i < tupleSize; i++) {
            foundChild = findTuple(tuple[i], current->numChildren,
                                   current->childOffset, &status);
            if (status != equal) {
                return 0.0f;
            }
            status = none;
            nodeIndex = m_FlatChildren[foundChild].nodeIndex;
            current = &m_Nodes[nodeIndex];

            if (current->isLeaf && i == tupleSize - 1) {
                // we found it
                return current->value;
            }
        }
        return 0.0;
    }

  private:
    int findTuple(const int tupleKey, const int numChildren,
                  const int entryIndex, SearchStatus *status) {

        int startingIndex = entryIndex;
        int i = startingIndex;
        *status = none;

        if (m_FlatChildren.size() == 0) {
            return -1;
        }

        for (; i < startingIndex + numChildren; i++) {
            int childIndex = m_FlatChildren[i].tupleIndex;
            if (tupleKey > childIndex) {
                *status = larger;
            } else if (tupleKey < childIndex) {
                *status = smaller;
                break;
            } else if (tupleKey == childIndex) {
                *status = equal;
                break;
            }
        }
        return i;
    }

  public:
    bool
    assertFlatChildrenValues(const std::vector<std::vector<int>> &children) {
        /*    std::cout << children.size() << std::endl;
            std::cout << m_FlatChildren.size() << std::endl;*/

        size_t providedChildrenSize = static_cast<size_t>(children.size());
        size_t flatChildrenSize = static_cast<size_t>(m_FlatChildren.size());

        assert(providedChildrenSize == flatChildrenSize);

        for (size_t i = 0; i < flatChildrenSize; i++) {
            const FlatChildEntry &child = m_FlatChildren[i];
            const std::vector<int> &groundTruth = children[i];
            assert(child.tupleIndex == groundTruth[0]);
            assert(child.nodeIndex == groundTruth[1]);
        }
        return true;
    }

    bool assertFlatNodeValues(const std::vector<std::vector<int>> &nodes) {
        size_t providedNodeSize = nodes.size();
        size_t nodeSize = m_Nodes.size();

        assert(providedNodeSize == nodeSize);

        for (size_t i = 0; i < nodeSize; i++) {
            FlatNode child = m_Nodes[i];
            std::vector<int> groundTruth = nodes[i];
            if (child.childOffset != groundTruth[0]) {
                std::cout << i << ". Child " << child.childOffset
                          << " == " << "Ground " << groundTruth[0] << std::endl;
            }

            assert(child.childOffset == groundTruth[0]);
            assert(child.numChildren == groundTruth[1]);
            assert(child.isLeaf == groundTruth[2]);
            assert(child.value == groundTruth[3]);
        }
        return true;
    }
};
#endif