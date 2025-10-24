#ifndef SPARSE_MATRIX_ITERATOR_H
#define SPARSE_MATRIX_ITERATOR_H
#include "FlatNode.h"
#include <vector>
#include "SparseMatrixTuple.h"

class SparseMatrixIterator
{
private:
    std::vector<FlatNode> *m_Nodes;
    std::vector<FlatChildEntry> *m_Children;
    int m_CurrentNode = 0;
    int m_CurrentFlatChild = 0;
    int m_Size = 0;
    std::vector<int> m_State;
    std::vector<int> m_ChildrenLimits;
    SparseMatrixTuple m_Tuple;
    std::vector<int> m_Stack;

public:
    SparseMatrixIterator(std::vector<FlatNode> *nodes, std::vector<FlatChildEntry> *children, int size) : m_Nodes(nodes), m_Children(children), m_Size(size)
    {
    }
    SparseMatrixIterator(std::vector<FlatNode> *nodes, std::vector<FlatChildEntry> *children) : m_Nodes(nodes), m_Children(children)
    {
    }
    SparseMatrixIterator begin()
    {
        return SparseMatrixIterator(m_Nodes, m_Children,m_Size);
    }

    SparseMatrixIterator end()
    {
        return SparseMatrixIterator(m_Nodes, m_Children);
    }

    SparseMatrixTuple operator*()
    {

        // SparseMatrixTuple result;
        //  start from root
        FlatNode *currentNode = &(*m_Nodes)[0];
        FlatChildEntry *visited = nullptr;

        int onNewLoop = m_ChildrenLimits.size() == 0;
        int currentFlatChild = 0;
        int currentIndex = 0;

        if (m_Stack.empty())
        {
            currentIndex = currentNode->childOffset;

            m_Stack.push_back(currentIndex);
            visited = &(*m_Children)[currentIndex];
            m_ChildrenLimits.push_back(currentNode->numChildren);
            m_State.push_back(0);
        }
        else
        {
            int lastState = m_State.size() - 1;
            int lastChildrenLimit = m_ChildrenLimits.size() - 1;

            if (m_State[lastState] < m_ChildrenLimits[lastChildrenLimit])
            {
                //std::cout << m_State[lastState] << " < " << m_ChildrenLimits[lastChildrenLimit] << std::endl;
                int lastElem = m_Stack.size() - 1;
                // int lastStack = m_Stack[lastElem];
                assert(m_Stack.size() == m_State.size());
                int entryIndex = m_Stack[lastElem];
                int increment = m_State[lastElem];
             //   std::cout << "visited : "<< entryIndex + increment << std::endl;
            //    std::cout << "entry : "<< entryIndex << " incr + :" << increment << std::endl;

                visited = &(*m_Children)[entryIndex + increment];
              //  m_Stack[lastElem] = entryIndex + increment;
            }
            else
            {
                // we pop both stack and state
                // new tree to be explored
                int lastElem = m_Stack.size() - 1;

                while (m_State[lastElem] + 1 > m_ChildrenLimits[lastElem])
                {
                    m_Stack.pop_back();
                    m_State.pop_back();
                    m_ChildrenLimits.pop_back();

                    lastElem--;
                    if (lastElem == -1)
                    {
                        break;
                    }
                    m_State[lastElem]++;
                }

                if (lastElem == -1)
                {
                    std::cout << "recalibratio" << std::endl;
                }
                else
                {
                    lastElem = m_Stack.size() - 1;
                    int entryIndex = m_Stack[lastElem];
                    visited = &(*m_Children)[entryIndex + 1];
                    m_Stack[lastElem] = entryIndex + 1;
                }
            }
        }

        m_Tuple.tuple.clear();
        m_Tuple.value = 0;

        while (true)
        {
            // stack saves the visited FlatNodeEntry index
            currentNode = &(*m_Nodes)[visited->nodeIndex];
            if (currentNode->isLeaf)
            {
                int stackSize = m_Stack.size();
                for (int i = 0; i < stackSize - 1; i++)
                {
                    int childIndex = m_Stack[i];//+m_State[i];
                    m_Tuple.tuple.push_back((*m_Children)[childIndex].tupleIndex);
                }
                int childIndex = m_Stack[stackSize - 1] +m_State[stackSize - 1];
                m_Tuple.tuple.push_back((*m_Children)[childIndex].tupleIndex);

                m_Tuple.value = currentNode->value;
                m_Size--;
                int last = m_State.size() - 1;
                m_State[last]++;
                return m_Tuple;
            }

            visited = &(*m_Children)[currentNode->childOffset];
            m_Stack.push_back(currentNode->childOffset);
            m_ChildrenLimits.push_back(currentNode->numChildren);
            m_State.push_back(0);
        }
    }

    const SparseMatrixTuple *operator->() { return &m_Tuple; }

    friend bool operator==(const SparseMatrixIterator &a, const SparseMatrixIterator &b)
    {
        return a.m_Size == b.m_Size;
    }

    friend bool operator!=(const SparseMatrixIterator &a, const SparseMatrixIterator &b)
    {
        return a.m_Size != b.m_Size;
    }

    SparseMatrixIterator &operator++()
    {
        // ++m_Matrix;
        return *this;
    }

    // Postfix increment
    SparseMatrixIterator operator++(int)
    {
        SparseMatrixIterator tmp = *this;
        ++(*this);
        return tmp;
    }
};
#endif