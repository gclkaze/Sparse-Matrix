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
#ifdef NODE_DEBUG
    int id;
#endif
};

struct TupleNode {
    int tupleSize;
    int tuple[64];
	double value;
};

#endif