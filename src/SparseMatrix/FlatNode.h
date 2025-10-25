#ifndef FLAT_NODE_H
#define FLAT_NODE_H
struct FlatChildEntry {
    int tupleIndex;
    int nodeIndex;
};

struct FlatNode {
    int childOffset;
    int numChildren;
    bool isLeaf;
    double value;
    int id;
};

#endif