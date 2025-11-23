#ifndef SPARSE_MATRIX_ITERATOR_H
#define SPARSE_MATRIX_ITERATOR_H
#include <assert.h>

#include <vector>

#include "FlatNode.h"
#include "SparseMatrixTuple.h"


class SparseMatrixIterator {
   private:
    std::vector<FlatNode>* m_Nodes = nullptr;
    std::vector<FlatChildEntry>* m_Children = nullptr;
    int m_Size = 0;
    std::vector<int> m_State;
    std::vector<int> m_ChildrenLimits;
    SparseMatrixTuple m_Tuple;
    std::vector<int> m_Stack;

   public:
    SparseMatrixIterator(std::vector<FlatNode>* nodes, std::vector<FlatChildEntry>* children,
                         int size)
        : m_Nodes(nodes), m_Children(children), m_Size(size) {}
    SparseMatrixIterator(std::vector<FlatNode>* nodes, std::vector<FlatChildEntry>* children)
        : m_Nodes(nodes), m_Children(children) {}
    SparseMatrixIterator begin() { return SparseMatrixIterator(m_Nodes, m_Children, m_Size); }

    bool ended() const { return m_Size == 0; }

    SparseMatrixIterator end() { return SparseMatrixIterator(m_Nodes, m_Children); }

    SparseMatrixTuple& getTuple() { return m_Tuple; }

    SparseMatrixTuple operator*() {
        if (m_Size == 0) {
            return m_Tuple;
        }
        // start from root
        FlatNode* currentNode = &(*m_Nodes)[0];
        FlatChildEntry* visited = calibrateVisit(currentNode);

        m_Tuple.tuple.clear();
        m_Tuple.value = 0;

        while (true) {
            currentNode = &(*m_Nodes)[visited->nodeIndex];
            if (currentNode->isLeaf) {
                int stackSize = (int)m_Stack.size();
                int childIndex = 0;
                for (int i = 0; i < stackSize - 1; i++) {
                    childIndex = m_Stack[i];
                    m_Tuple.tuple.push_back((*m_Children)[childIndex].tupleIndex);
                }
                // add the branches value by adding the stored child offset to
                // the current stacked child entry (that is the first child of
                // the branch)
                childIndex = m_Stack[stackSize - 1] + m_State[stackSize - 1];
                m_Tuple.tuple.push_back((*m_Children)[childIndex].tupleIndex);

                m_Tuple.value = currentNode->value;
                m_Size--;
                m_State[stackSize - 1]++;
                return m_Tuple;
            }

            visited = &(*m_Children)[currentNode->childOffset];
            // stack saves the freshly visited FlatNodeEntry index
            m_Stack.push_back(currentNode->childOffset);
            m_ChildrenLimits.push_back(currentNode->numChildren);
            m_State.push_back(0);
        }
    }

    const SparseMatrixTuple* operator->() { return &m_Tuple; }

    friend bool operator==(const SparseMatrixIterator& a, const SparseMatrixIterator& b) {
        return a.m_Size == b.m_Size;
    }

    friend bool operator!=(const SparseMatrixIterator& a, const SparseMatrixIterator& b) {
        return a.m_Size != b.m_Size;
    }

    SparseMatrixIterator& operator++() { return *this; }

    // Postfix increment
    SparseMatrixIterator operator++(int) {
        SparseMatrixIterator tmp = *this;
        ++(*this);
        return tmp;
    }

   private:
    FlatChildEntry* calibrateVisit(FlatNode* currentNode) {
        if (!m_Stack.empty()) {
            int lastElem = (int)m_State.size() - 1;
            assert(m_State.size() == m_Stack.size() &&
                   lastElem + 1 == (int)m_ChildrenLimits.size());
            if (m_State[lastElem] < m_ChildrenLimits[lastElem]) {
                int entryIndex = m_Stack[lastElem];
                int increment = m_State[lastElem];
                return &(*m_Children)[entryIndex + increment];
            } else {
                // we pop both stack and state
                // new tree to be explored, we need to clear our state from the
                // Finished Branches a.k.a the Ones that reached their loop
                // Limit
                while (m_State[lastElem] + 1 > m_ChildrenLimits[lastElem]) {
                    m_Stack.pop_back();
                    m_State.pop_back();
                    m_ChildrenLimits.pop_back();

                    lastElem--;
                    if (lastElem == -1) {
                        break;
                    }
                    m_State[lastElem]++;
                }

                if (lastElem != -1) {
                    lastElem = (int) m_Stack.size() - 1;
                    int entryIndex = m_Stack[lastElem];
                    m_Stack[lastElem] = entryIndex + 1;
                    return &(*m_Children)[entryIndex + 1];
                } else {
                    assert(false);
                }
            }
        } else {
            int currentIndex = currentNode->childOffset;
            m_Stack.push_back(currentIndex);
            m_ChildrenLimits.push_back(currentNode->numChildren);
            m_State.push_back(0);
            return &(*m_Children)[currentIndex];
        }
        assert(false);
        return nullptr;
    }
};
#endif