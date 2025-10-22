#ifndef DATA_NODE_H
#define DATA_NODE_H
#include "FlatNode.h"
#include <assert.h>
#include <vector>

class DataNode
{
  typedef enum SearchStatus
  {
    none,
    smaller,
    equal,
    larger
  } SearchStatus;

  std::vector<FlatNode> m_Nodes;
  std::vector<FlatChildEntry> m_FlatChildren;
  int m_Size = 0;

public:
  void insert(const std::vector<int> &tuple, double value)
  {

    assert(value != 0.0);
    assert(tuple.size() > 0);

    if (m_Nodes.size() == 0)
    {
      // root is empty
      FlatNode root = {0, false, 0, 0, 0};

      m_Nodes.push_back(root);
      insertNode(tuple, 0, value);
      return;
    }
    insertNode(tuple, 0, value);
  }

  int size()
  {
    return m_Size;
  }

  void insertNode(const std::vector<int> &tuple, int index, const double value)
  {
    int tupleSize = static_cast<int>(tuple.size());
    if (index == tupleSize)
    {
      return;
    }

    FlatNode *current = &m_Nodes[0];
    SearchStatus status = none;
    int flatChildrenPos = 0;
    bool notAMatch = false;
    int node = -1;
    int nodesAmount = -1;
    FlatNode *nodePtr = nullptr;
    bool insertion = false;
    for (int i = 0; i < tupleSize; i++)
    {
      if (!notAMatch)
      {
        flatChildrenPos = this->findTheTuple(tuple[i], current->numChildren, current->childOffset, &status);
      }

      switch (status)
      {
      case larger:
      case smaller:
      {
        current->numChildren++;
        nodesAmount = m_Nodes.size();
        m_FlatChildren.insert(m_FlatChildren.begin() + flatChildrenPos, {tuple[i], nodesAmount});
        updateChildOffsets(current, nodesAmount);
        current = saveNode(i + 1 < tupleSize, nodesAmount, value, current, false);
        status = none;
        notAMatch = true;
        insertion = true;
        break;
      }
      case equal:
      {
        // it is a match, we need to go further to the next index
        node = m_FlatChildren[flatChildrenPos].nodeIndex;
        nodesAmount = m_Nodes.size();
        assert(node < nodesAmount);
        current = &m_Nodes[node];
        break;
      }

      case none:
      {
        // the path does not exist at all
        current->numChildren++;

        // we did our stuff with the parent node, lets continue to the new nodes,
        nodesAmount = m_Nodes.size();
        current = saveNode(i + 1 < tupleSize, nodesAmount, value, current, true);
        m_FlatChildren.push_back({tuple[i], nodesAmount});
        insertion = true;
        break;
      }
      default:
      {
        assert(false);
        break;
      }
      }
    }

    if (!insertion)
    {
      current = updateNotZeroValue(current, value);
    }
  }

private:
  FlatNode *updateNotZeroValue(FlatNode *current, const double value)
  {
    assert(current->isLeaf);
    if (value != 0 && value != current->value)
    {
      current->value = value;
    }
    return current;
  }
  FlatNode *saveNode(const bool isNotLeaf, const int nodesAmount, const double value, FlatNode *current, const bool childInsertedAtTheEnd)
  {
    FlatNode node;
    node.id = nodesAmount;

    if (isNotLeaf)
    {
      node.childOffset = (static_cast<int>(m_FlatChildren.size())) + (childInsertedAtTheEnd ? 1 : 0);
      node.numChildren = 1;
      node.isLeaf = false;
      node.value = 0.0f;
    }
    else
    {
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
  void updateChildOffsets(FlatNode *parent, int index)
  {
    index--;
    FlatNode *start = &m_Nodes[index];

    while (start != parent)
    {
      assert(index >= 0);
      if (start->isLeaf)
      {
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
                   SearchStatus *status)
  {

    int i = childOffset;
    int flatChildrenAmount = m_FlatChildren.size();
    int max = childOffset + numChildren;

    *status = none;

    if (flatChildrenAmount == 0)
    {
      return -1;
    }

    if (i >= flatChildrenAmount)
    {
      return -1;
    }
    for (; i < max; i++)
    {
      int storedTuple = m_FlatChildren[i].tupleIndex;
      if (tupleKey > storedTuple)
      {
        *status = larger;
        continue; // break;
      }
      else if (tupleKey < storedTuple)
      {
        *status = smaller;
        break;
      }
      else if (tupleKey == storedTuple)
      {
        *status = equal;
        break;
      }
    }
    return i;
  }

public:
  double getValue(const std::vector<int> &tuple)
  {
    FlatNode *current = &m_Nodes[0];
    SearchStatus status = none;
    int nodeIndex = -1;
    int foundChild = -1;
    int tupleSize = static_cast<int>(tuple.size());

    for (int i = 0; i < tupleSize; i++)
    {
      foundChild = findTuple(tuple[i], current->numChildren, current->childOffset, &status);
      if (status != equal)
      {
        return 0.0f;
      }
      status = none;
      nodeIndex = m_FlatChildren[foundChild].nodeIndex;
      current = &m_Nodes[nodeIndex];

      if (current->isLeaf && i == tupleSize - 1)
      {
        // we found it
        return current->value;
      }
    }
    return 0.0;
  }

private:
  int findTuple(const int tupleKey, const int numChildren, const int entryIndex,
                SearchStatus *status)
  {

    int startingIndex = entryIndex;
    int i = startingIndex;
    *status = none;

    if (m_FlatChildren.size() == 0)
    {
      return -1;
    }

    for (; i < startingIndex + numChildren; i++)
    {
      int childIndex = m_FlatChildren[i].tupleIndex;
      if (tupleKey > childIndex)
      {
        *status = larger;
      }
      else if (tupleKey < childIndex)
      {
        *status = smaller;
        break;
      }
      else if (tupleKey == childIndex)
      {
        *status = equal;
        break;
      }
    }
    return i;
  }

public:
  bool assertFlatChildrenValues(const std::vector<std::vector<int>> &children)
  {
    /*    std::cout << children.size() << std::endl;
        std::cout << m_FlatChildren.size() << std::endl;*/

    size_t providedChildrenSize = static_cast<size_t>(children.size());
    size_t flatChildrenSize = static_cast<size_t>(m_FlatChildren.size());

    assert(providedChildrenSize == flatChildrenSize);

    for (size_t i = 0; i < flatChildrenSize; i++)
    {
      const FlatChildEntry &child = m_FlatChildren[i];
      const std::vector<int> &groundTruth = children[i];
      assert(child.tupleIndex == groundTruth[0]);
      assert(child.nodeIndex == groundTruth[1]);
    }
    return true;
  }

  bool assertFlatNodeValues(const std::vector<std::vector<int>> &nodes)
  {
    size_t providedNodeSize = nodes.size();
    size_t nodeSize = m_Nodes.size();

    assert(providedNodeSize == nodeSize);

    for (size_t i = 0; i < nodeSize; i++)
    {
      FlatNode child = m_Nodes[i];
      std::vector<int> groundTruth = nodes[i];
      if (child.childOffset != groundTruth[0])
      {
        std::cout << i << std::endl;
        std::cout << "Child " << child.childOffset << " == " << "Ground "
                  << groundTruth[0] << std::endl;
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