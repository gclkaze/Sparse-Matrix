#pragma once

__device__ int copyAndMultiplySubtree(const FlatNode* leftNodes, const FlatChildEntry* leftChildren, const FlatNode* rightNodes, const FlatChildEntry* rightChildren, FlatNode* outNodes, FlatChildEntry* outChildren, int leftIndex, int rightIndex, int* outNodeCounter, int* outChildCounter, int* multiplications, FlatChildEntry* newChildren, FlatNode* newNodes, int* newChildrenAmount, int* newNodesAmount)
{
	const FlatNode& leftNode = leftNodes[leftIndex];
	const FlatNode& rightNode = rightNodes[rightIndex];
	// Leaf case: only create node if both are leaves
	if (leftNode.isLeaf && rightNode.isLeaf) {
		//printf("print at start\n");

		FlatNode newNode;
		newNode.isLeaf = true;
		newNode.numChildren = 0;
		newNode.childOffset = -1;
		newNode.value = leftNode.value * rightNode.value;
		int myIndex = atomicAdd(outNodeCounter, 1);
		outNodes[myIndex] = newNode;
		atomicAdd(multiplications, 1);
		return myIndex;
	}

	// Internal node: check children
	int childOffset = atomicAdd(outChildCounter, leftNode.numChildren);
	int numChildren = 0;
	bool hasChildren = false;

	FlatNode newNode;
	newNode.isLeaf = false;
	newNode.childOffset = childOffset;
	newNode.value = 0.0;

	for (int i = 0; i < leftNode.numChildren; ++i) {
		FlatChildEntry leftChild = leftChildren[leftNode.childOffset + i];
		int matchingRightIndex = findMatchingChild(
			rightChildren, rightNode.childOffset, rightNode.numChildren, leftChild.tupleIndex
		);

		if (matchingRightIndex != -1) {

			int childNodeIndex = copyAndMultiplySubtree(
				leftNodes, leftChildren,
				rightNodes, rightChildren,
				outNodes, outChildren,
				leftChild.nodeIndex, matchingRightIndex,
				outNodeCounter, outChildCounter,
				multiplications, newChildren, newNodes, newChildrenAmount, newNodesAmount
			);

			if (childNodeIndex == -1) {
				return -1;
			}
			//we found it, we need to dive further
			newChildren[(*newChildrenAmount)++] = { leftChild.tupleIndex ,matchingRightIndex };


		}
		else {
			return -1;
		}
	}

	return -1; // No node created
}
