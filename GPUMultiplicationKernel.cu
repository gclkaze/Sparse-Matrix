#include <cuda.h>
#include <cuda_runtime_api.h>
#include <stdio.h>

#include <memory>

#include "../../CommonOffset.h"
#include "../../FlatIndex.h"
#include "../../ISparseMatrix.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <iostream>

// Find matching child in sorted array (early exit)
__device__ int findMatchingChild(const FlatChildEntry* rightChildren, int offset, int numChildren, int tupleKey) {
	//printf("Looking for tuple : %d\n", tupleKey);

	for (int i = 0; i < numChildren; i++) {
		int currentKey = rightChildren[offset + i].tupleIndex;
		//printf("child %d t %d\n",currentKey,tupleKey);
		if (currentKey == tupleKey) {
		//	printf("returns %d\n", rightChildren[offset + i].nodeIndex);

			return rightChildren[offset + i].nodeIndex;
		}
		if (currentKey > tupleKey) {
			// Sorted array: no need to check further
			return -2;
		}
	}
	return -1;
}

// Insert child in sorted order by tupleIndex
__device__ void insertChildSorted(
	FlatChildEntry* outChildren,
	int childOffset,
	int& currentCount,
	FlatChildEntry newEntry)
{
	int pos = currentCount;
	for (int i = 0; i < currentCount; ++i) {
		if (outChildren[childOffset + i].tupleIndex > newEntry.tupleIndex) {
			pos = i;
			break;
		}
	}

	// Shift elements to the right
	for (int i = currentCount; i > pos; --i) {
		outChildren[childOffset + i] = outChildren[childOffset + i - 1];
	}

	// Insert new entry
	outChildren[childOffset + pos] = newEntry;
	currentCount++;
}
__device__ int copyAndMultiplySubtreeNew(
	const FlatNode* leftNodes, const FlatChildEntry* leftChildren,
	const FlatNode* rightNodes, const FlatChildEntry* rightChildren,
	FlatNode* outNodes, FlatChildEntry* outChildren,
	int leftIndex, int rightIndex,
	int* outNodeCounter, int* outChildCounter,
	int* multiplications)
{
	const FlatNode& leftNode = leftNodes[leftIndex];
	const FlatNode& rightNode = rightNodes[rightIndex];

	// Leaf case
	if (leftNode.isLeaf && rightNode.isLeaf) {
		int myIndex = atomicAdd(outNodeCounter, 1);
		FlatNode newNode;
		newNode.isLeaf = true;
		newNode.numChildren = 0;
		newNode.childOffset = -1;
		newNode.value = leftNode.value * rightNode.value;

		atomicAdd(multiplications, 1);
		outNodes[myIndex] = newNode;
		return myIndex;
	}

	// Internal node
	int numChildren = 0;
	FlatChildEntry tempChildren[64]; // assume max children per node
	for (int i = 0; i < leftNode.numChildren; ++i) {
		FlatChildEntry leftChild = leftChildren[leftNode.childOffset + i];
		int matchingRightIndex = findMatchingChild(
			rightChildren, rightNode.childOffset, rightNode.numChildren, leftChild.tupleIndex
		);

		if (matchingRightIndex != -1) {
			int childNodeIndex = copyAndMultiplySubtreeNew(
				leftNodes, leftChildren,
				rightNodes, rightChildren,
				outNodes, outChildren,
				leftChild.nodeIndex, matchingRightIndex,
				outNodeCounter, outChildCounter,
				multiplications
			);

			if (childNodeIndex != -1) {
				tempChildren[numChildren++] = { leftChild.tupleIndex, childNodeIndex };
			}
		}
	}

	if (numChildren > 0) {
		int myIndex = atomicAdd(outNodeCounter, 1);
		int childOffset = atomicAdd(outChildCounter, numChildren);

		FlatNode newNode;
		newNode.isLeaf = false;
		newNode.numChildren = numChildren;
		newNode.childOffset = childOffset;
		newNode.value = 0.0;

		// Copy children in sorted order
		for (int i = 0; i < numChildren; ++i) {
			outChildren[childOffset + i] = tempChildren[i];
		}

		outNodes[myIndex] = newNode;
		return myIndex;
	}

	return -1; // No node created
}

// Recursive function: copy subtree and multiply leaf values
__device__ int copyAndMultiplySubtreeOriginal(
	const FlatNode* leftNodes, const FlatChildEntry* leftChildren,
	const FlatNode* rightNodes, const FlatChildEntry* rightChildren,
	FlatNode* outNodes, FlatChildEntry* outChildren,
	int leftIndex, int rightIndex,
	int* outNodeCounter, int* outChildCounter,
	int* multiplications,
	FlatChildEntry *newChildren,
	FlatNode *newNodes,int* newChildrenAmount, int* newNodesAmount
	/*,const int nNodes,const int nChildren*/)
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
			//we found it, we need to dive further

			int childNodeIndex = copyAndMultiplySubtreeOriginal(
				leftNodes, leftChildren,
				rightNodes, rightChildren,
				outNodes, outChildren,
				leftChild.nodeIndex, matchingRightIndex,
				outNodeCounter, outChildCounter,
				multiplications, newChildren, newNodes, newChildrenAmount, newNodesAmount
			);

			FlatChildEntry newEntry = { leftChild.tupleIndex, childNodeIndex };
			insertChildSorted(outChildren, childOffset, numChildren, newEntry);
			hasChildren = true;
		}
	}

	if (hasChildren) {
		newNode.numChildren = numChildren;
		int myIndex = atomicAdd(outNodeCounter, 1);
		outNodes[myIndex] = newNode;
		return myIndex;
	}

	return -1; // No node created
}

__shared__ FlatChildEntry smBuckets[];// [2000] ;

//-1 for not found
//1 for data node
//0 for end
__device__ int copyAndMultiplySubtreeSharedMemory(
	const FlatNode* leftNodes, const FlatChildEntry* leftChildren,
	const FlatNode* rightNodes, const FlatChildEntry* rightChildren,
	int leftIndex, int rightIndex,
	int* multiplications,
	TupleNode* tuples, TupleNode* currentTuple)
{
	const FlatNode& leftNode = leftNodes[leftIndex];
	const FlatNode& rightNode = rightNodes[rightIndex];

	if (leftNode.isLeaf && rightNode.isLeaf) {
		int pos = atomicAdd(multiplications, 1);
		tuples[pos] = { currentTuple->tupleSize,{},leftNode.value * rightNode.value };

		for (int i = 0; i < currentTuple->tupleSize; ++i) {
			tuples[pos].tuple[i] = currentTuple->tuple[i];
		}
		return 1;
	}

	for (int i = 0; i < leftNode.numChildren; ++i) {
		FlatChildEntry leftChild = leftChildren/*smBuckets*/[leftNode.childOffset + i];
		int matchingRightIndex = findMatchingChild(
			rightChildren, rightNode.childOffset, rightNode.numChildren, leftChild.tupleIndex
		);

		if (matchingRightIndex >= 0) {
			currentTuple->tuple[currentTuple->tupleSize++] = leftChild.tupleIndex;

			copyAndMultiplySubtreeSharedMemory(
				leftNodes, leftChildren,
				rightNodes, rightChildren,
				leftChild.nodeIndex, matchingRightIndex,
				multiplications, tuples, currentTuple
			);

			currentTuple->tupleSize--;
			continue;
		}
		return 0;
	}
	return 0;
}


//-1 for not found
//1 for data node
//0 for end
__device__ int copyAndMultiplySubtree(
	const FlatNode* leftNodes, const FlatChildEntry* leftChildren,
	const FlatNode* rightNodes, const FlatChildEntry* rightChildren,
	int leftIndex, int rightIndex,
	int* multiplications,
	TupleNode* tuples, TupleNode *currentTuple)
{
	const FlatNode& leftNode = leftNodes[leftIndex];
	const FlatNode& rightNode = rightNodes[rightIndex];

	if (leftNode.isLeaf && rightNode.isLeaf) {
		int pos = atomicAdd(multiplications, 1);
		tuples[pos] = { currentTuple->tupleSize,{},leftNode.value * rightNode.value };

		for (int i = 0; i < currentTuple->tupleSize; ++i) {
			tuples[pos].tuple[i] = currentTuple->tuple[i];
		}
		return 1;
	}

	for (int i = 0; i < leftNode.numChildren; ++i) {
		FlatChildEntry leftChild = leftChildren[leftNode.childOffset + i];
		int matchingRightIndex = findMatchingChild(
			rightChildren, rightNode.childOffset, rightNode.numChildren, leftChild.tupleIndex
		);

		if (matchingRightIndex >= 0) {
			currentTuple->tuple[currentTuple->tupleSize++] = leftChild.tupleIndex;

			copyAndMultiplySubtree(
				leftNodes, leftChildren,
				rightNodes, rightChildren,
				leftChild.nodeIndex, matchingRightIndex,
				multiplications,  tuples, currentTuple
			);

			currentTuple->tupleSize--;
			continue;
		}
		printf("%d", matchingRightIndex);
		return 0;
	}
	return 0;
}

// Kernel: process all root nodes
__global__ void multiplyTreesKernelTiled(
	const FlatNode* leftNodes, const FlatChildEntry* leftChildren,
	const FlatNode* rightNodes, const FlatChildEntry* rightChildren,
	int* multiplications,
	int numRoots,
	TupleNode* tuples)
{
//	int idx = blockIdx.x * blockDim.x + threadIdx.x;


	int tid = threadIdx.x;
	int blockSize = blockDim.x;

	int totalBuckets = numRoots * 3;

		// Loop over tiles
		for (int tileStart = 0; tileStart < totalBuckets; tileStart += blockSize) {
			int globalIdx = tileStart + tid;

			// Load tile into shared memory
/*			if (globalIdx < totalBuckets) {
				smBuckets[tid] = globalBuckets[globalIdx];
			}*/
			int idx = globalIdx;
			int leftChild = leftNodes[0].childOffset + idx;
			int lidx = __ldg(&leftChildren[leftChild].nodeIndex);
			printf("hereee %d\n", globalIdx);
			for (int i = leftNodes[lidx].childOffset; i < leftNodes[lidx].numChildren + leftNodes[lidx].childOffset; i++) {
				smBuckets[i] = leftChildren[i];
			}

			__syncthreads();
			printf("here\n");
			// Process this tile
			if (globalIdx < totalBuckets) {
				//int val = smBuckets[tid];
				// Do computation here
				int leftChild = leftNodes[0].childOffset + idx;
				int rightChild = __ldg(&rightNodes[0].childOffset);
				TupleNode t;
				t.tuple[0] = __ldg(&leftChildren[leftChild].tupleIndex);
				t.tupleSize = 1;

				copyAndMultiplySubtreeSharedMemory(
					leftNodes, leftChildren,
					rightNodes, rightChildren,
					lidx, rightChildren[rightChild].nodeIndex,
					multiplications, tuples, &t
				);
				return;
			}
			__syncthreads();
		}

}

// Kernel: process all root nodes
__global__ void multiplyTreesKernel(
	const FlatNode* leftNodes, const FlatChildEntry* leftChildren,
	const FlatNode* rightNodes, const FlatChildEntry* rightChildren,
	int* multiplications,
	int numRoots,	
	TupleNode* tuples)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx < numRoots) {

		int leftChild = leftNodes[0].childOffset + idx;
		int lidx = __ldg(&leftChildren[leftChild].nodeIndex);

		int rightChild = __ldg(&rightNodes[0].childOffset);

		TupleNode t;
		t.tuple[0] = __ldg(&leftChildren[leftChild].tupleIndex);
		t.tupleSize = 1;

		copyAndMultiplySubtreeSharedMemory(
			leftNodes, leftChildren,
			rightNodes, rightChildren,
			lidx, rightChildren[rightChild].nodeIndex,
			multiplications, tuples, &t
		);
		return;

/*

		//find left and right indices to start with
		int leftChild = leftNodes[0].childOffset + idx;
		int lidx = leftChildren[leftChild].nodeIndex;

		int rightChild = rightNodes[0].childOffset;

		int rChildren = rightNodes[0].numChildren;
		int lTuple = leftChildren[leftChild].tupleIndex;

		TupleNode t;
		t.tuple[0] = lTuple;
		t.tupleSize = 1;

for (int i = 0; i < rChildren; i++) {
			int rTuple = rightChildren[rightChild + i].tupleIndex;
			if(lTuple == rTuple){

				TupleNode t;
				t.tuple[0] = lTuple;
				t.tupleSize = 1;

				copyAndMultiplySubtree(
					leftNodes, leftChildren,
					rightNodes, rightChildren,
					lidx, rightChildren[rightChild + i].nodeIndex,
					multiplications, tuples, &t
				);
				return;
			}
			if (rTuple > lTuple) {
				break;
			}
		}*/


	}

	__syncthreads();
}

__global__ void intersectionKernel(CommonOffset* c, const int* a, const int* b, const int min,
	const int max, const int* aPos, const int* bPos, int* stackTop) {
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	if (i >= min) {
		return;
	}

	int lDiff = abs(a[i] - b[0]);
	int rDiff = abs(a[i] - b[max - 1]);
	//  printf("%d \n", i);
	//  printf("%d. %d Left %d != Right %d, b[0]: %d ,b[max-1]: %d \n",i,
	//  a[i],lDiff,rDiff,b[0],b[max-1]);
	if (a[i] >= b[0] && a[i] <= b[max - 1]) {
		if (lDiff > rDiff) {
			int j = max - 1;
			while (j >= 0) {
				if (b[j] < a[i]) {
					return;
				}
				if (a[i] == b[j]) {
					int pos = atomicAdd(stackTop, 1);  // Atomically get the next position
					if (pos < min) {
						c[pos] = { aPos[i], bPos[j], a[i] };
					}
					return;
				}
				j--;
			}
		}
		else {
			int j = 0;
			for (; j < max; j++) {
				if (b[j] > a[i]) {
					return;
				}
				if (a[i] == b[j]) {
					int pos = atomicAdd(stackTop, 1);
					if (pos < min) {
						c[pos] = { aPos[i], bPos[j], a[i] };
					}
					return;
				}
			}
		}
	}
}

__global__ void imprIntersectionKernel(CommonOffset* c, const int* a, const int* b, const int min,
	const int max, const int* aPos, const int* bPos, int* stackTop) {
	extern __shared__ int b_shared[];  // dynamically allocated shared memory

	int tid = threadIdx.x;
	int i = blockIdx.x * blockDim.x + tid;

	// Load b[] into shared memory (only first max threads do this)
	for (int j = tid; j < max; j += blockDim.x) {
		b_shared[j] = b[j];
	}
	__syncthreads();  // Ensure all threads have loaded b[]

	if (i >= min) return;

	// Cache a[i] and aPos[i] to avoid repeated global memory reads
	int a_val = a[i];
	int a_pos = aPos[i];

	int first = b_shared[0];
	int last = b[max - 1];

	int lDiff = abs(a_val - first);
	int rDiff = abs(a_val - last);
	if (a_val < first || a_val > last) return;

	if (lDiff > rDiff) {
#pragma unroll
		for (int j = max - 1; j >= 0; --j) {
			int b_val = b_shared[j];
			if (b_val < a_val) return;
			if (a_val == b_val) {
				int pos = atomicAdd(stackTop, 1);
				if (pos < min) {
					c[pos] = { a_pos, bPos[j], a_val };
				}
				return;
			}
		}
	}
	else {
#pragma unroll
		for (int j = 0; j < max; ++j) {
			int b_val = b_shared[j];
			if (b_val > a_val) return;
			if (a_val == b_val) {
				int pos = atomicAdd(stackTop, 1);
				if (pos < min) {
					c[pos] = { a_pos, bPos[j], a_val };
				}
				return;
			}
		}
	}
}

__global__ void multiply(FlatNode *aNodes, FlatNode *bNodes,FlatChildEntry* aChildren,FlatChildEntry* bChildren)
{
	return;
}

int launchMultiplication(ISparseMatrix* A, ISparseMatrix* B, ISparseMatrix* C) {
	FlatNode* d_aNodes;
	FlatNode* d_bNodes;

	FlatChildEntry* d_aChildren;
	FlatChildEntry* d_bChildren;
	TupleNode* tuples;

	int* multiplications;

	cudaMallocManaged(&d_aNodes, A->getNodes().size() * sizeof(FlatNode));
	cudaMallocManaged(&d_bNodes, B->getNodes().size() * sizeof(FlatNode));
	cudaMallocManaged(&d_aChildren, A->getFlatChildren().size() * sizeof(FlatChildEntry));
	cudaMallocManaged(&d_bChildren, B->getFlatChildren().size() * sizeof(FlatChildEntry));
	cudaMallocManaged(&multiplications, sizeof(int));
	cudaMallocManaged(&tuples, A->getNodes().size() * sizeof(TupleNode));

	*multiplications = 0;

	cudaMemcpy(d_aNodes, &A->getNodes()[0], A->getNodes().size() * sizeof(FlatNode), cudaMemcpyHostToDevice);
	cudaMemcpy(d_bNodes, &B->getNodes()[0], B->getNodes().size() * sizeof(FlatNode), cudaMemcpyHostToDevice);
	cudaMemcpy(d_aChildren, &A->getFlatChildren()[0], A->getFlatChildren().size() * sizeof(FlatChildEntry), cudaMemcpyHostToDevice);
	cudaMemcpy(d_bChildren, &B->getFlatChildren()[0], B->getFlatChildren().size() * sizeof(FlatChildEntry), cudaMemcpyHostToDevice);

	int numRoots = A->getNodes()[0].numChildren; // or however many root nodes you want to process

	// Launch kernel
	//int threadsPerBlock = 256;
	size_t stackSize = 16384; // 16 KB
	cudaDeviceSetLimit(cudaLimitStackSize, stackSize);


	int minGridSize = 0;
	int blockSize = 0;

	// Compute optimal block size
	cudaOccupancyMaxPotentialBlockSize(
		&minGridSize,
		&blockSize,
		multiplyTreesKernel,//multiplyTreesKernel,
		0, // dynamic shared memory
		0  // no block size limit
	);


	int numElements = numRoots;
	int gridSize = (numElements + blockSize - 1) / blockSize;


/*	multiplyTreesKernel << <minGridSize, blockSize >> > (
		d_aNodes, d_aChildren,
		d_bNodes, d_bChildren,
		multiplications, numRoots,tuples);*/

	//int blockSize = 256;
	int sharedMemSize = blockSize * sizeof(int);

	multiplyTreesKernel << <minGridSize, blockSize, sharedMemSize >> > (
		d_aNodes, d_aChildren,
		d_bNodes, d_bChildren,
		multiplications, numRoots, tuples);

	cudaDeviceSynchronize(); // Wait for kernel to finish

	// Now safely access the memory on the host

	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("CUDA Error: %s\n", cudaGetErrorString(err));
	}

	cudaDeviceProp prop;
	cudaGetDeviceProperties(&prop, 0);

	TupleNode* hostTuples = new TupleNode[*multiplications];
	cudaMemcpy(hostTuples, tuples, *multiplications * sizeof(TupleNode), cudaMemcpyDeviceToHost);
	C->insert(hostTuples, *multiplications);

	return *multiplications;
}


int launch_intersectionKernel(CommonOffset* c, const int* a, const int* b, const int min,
	const int max, const int* aPos, const int* bPos) {
	int* d_a, * d_b, * d_aPos, * d_bPos;
	CommonOffset* d_c;
	int* d_stackTop;
	int stackTop = 0;

	int minSize = min * sizeof(int);
	int maxSize = max * sizeof(int);

	cudaMalloc(&d_a, minSize);
	cudaMalloc(&d_b, maxSize);
	cudaMalloc(&d_c, min * sizeof(CommonOffset));
	cudaMalloc(&d_aPos, minSize);
	cudaMalloc(&d_bPos, maxSize);

	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("CUDA Error: %s\n", cudaGetErrorString(err));
	}

	cudaMemcpy(d_a, a, minSize, cudaMemcpyHostToDevice);
	cudaMemcpy(d_b, b, maxSize, cudaMemcpyHostToDevice);
	cudaMemcpy(d_aPos, aPos, minSize, cudaMemcpyHostToDevice);
	cudaMemcpy(d_bPos, bPos, maxSize, cudaMemcpyHostToDevice);

	err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("CUDA Error: %s\n", cudaGetErrorString(err));
	}

	cudaMalloc(&d_stackTop, sizeof(int));
	cudaMemset(d_stackTop, 0, sizeof(int));

	err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("CUDA Error: %s\n", cudaGetErrorString(err));
	}

	int minGridSize, blockSize;
	cudaOccupancyMaxPotentialBlockSize(&minGridSize, &blockSize, intersectionKernel);

	int sharedMemSize = max * sizeof(int);
	imprIntersectionKernel << <48, blockSize, sharedMemSize >> > (d_c, d_a, d_b, min, max, d_aPos, d_bPos,
		d_stackTop);


	cudaDeviceSynchronize();

	err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("CUDA Error: %s\n", cudaGetErrorString(err));
	}

	cudaMemcpy(&stackTop, d_stackTop, sizeof(int), cudaMemcpyDeviceToHost);

	//std::cout << "found " << stackTop << " common ones" << std::endl;
	cudaMemcpy(c, d_c, min * sizeof(CommonOffset), cudaMemcpyDeviceToHost);

	err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("CUDA Error: %s\n", cudaGetErrorString(err));
	}

	cudaFree(d_a);
	cudaFree(d_b);
	cudaFree(d_c);
	cudaFree(d_stackTop);
	cudaFree(d_aPos);
	cudaFree(d_bPos);
	return stackTop;
}

std::unique_ptr<FlatIndex> collectIndexInformation(const FlatNode& visitLeft,
	const FlatNode& visitRight, ISparseMatrix* me,
	ISparseMatrix* other) {
	auto ptr = std::make_unique<FlatIndex>();

	int offsetLeft = visitLeft.childOffset;
	int maxOffsetLeft = visitLeft.numChildren;

	ptr->leftIndices.reserve(maxOffsetLeft);
	ptr->leftIndexPos.reserve(maxOffsetLeft);

	const std::vector<FlatChildEntry>& myflatChildren = me->getFlatChildren();

	int max = offsetLeft + maxOffsetLeft;

	for (int i = offsetLeft; i < max; i++) {
		ptr->leftIndices.push_back(myflatChildren[i].tupleIndex);
		ptr->leftIndexPos.push_back(i);
	}

	int offsetRight = visitRight.childOffset;
	int maxOffsetRight = visitRight.numChildren;

	ptr->rightIndices.reserve(maxOffsetRight);
	ptr->rightIndexPos.reserve(maxOffsetRight);

	const std::vector<FlatChildEntry>& flatChildren = other->getFlatChildren();
	max = offsetRight + maxOffsetRight;

	for (int i = offsetRight; i < max; i++) {
		ptr->rightIndices.push_back(flatChildren[i].tupleIndex);
		ptr->rightIndexPos.push_back(i);
	}

	int maxSize = maxOffsetLeft < maxOffsetRight ? maxOffsetRight : maxOffsetLeft;

	ptr->maxSize = maxSize;
	ptr->maxOffsetLeft = maxOffsetLeft;
	ptr->maxOffsetRight = maxOffsetRight;

	return ptr;
}

void launch_reduceOFfsetTrees(CommonOffset* offsets, int indices, ISparseMatrix* A,
	ISparseMatrix* B, ISparseMatrix* C, std::vector<int>* t) {
	const std::vector<FlatNode>& myNodes = A->getNodes();
	const std::vector<FlatNode>& otherNodes = B->getNodes();

	std::vector<FlatChildEntry>& flatChildren = A->getFlatChildren();
	std::vector<FlatChildEntry>& otherChildren = B->getFlatChildren();

	for (int i = 0; i < indices; i++) {
		CommonOffset& offset = offsets[i];
		int left = offset.indexLeft;
		int right = offset.indexRight;
		int leftNode = flatChildren[left].nodeIndex;
		int rightNode = otherChildren[right].nodeIndex;
		FlatNode visitLeft = myNodes[leftNode];
		FlatNode visitRight = otherNodes[rightNode];

		t->push_back(offset.tupleKey);

		if (visitLeft.isLeaf && visitRight.isLeaf) {
			// do the operation
			C->insert(*t, visitLeft.value * visitRight.value);
			t->pop_back();
			continue;
		}

		auto info = collectIndexInformation(visitLeft, visitRight, A, B);
		const int szLeft = info.get()->maxOffsetLeft;
		const int szRight = info.get()->maxOffsetRight;
		const int mx = info.get()->maxSize;

		CommonOffset* theOffsets = (CommonOffset*)malloc(sizeof(CommonOffset) * szLeft);

		int indx = launch_intersectionKernel(
			theOffsets, &info.get()->leftIndices[0], &info.get()->rightIndices[0], szLeft, mx,
			&info.get()->leftIndexPos[0], &info.get()->rightIndexPos[0]);
		if (!indx) {
			return;
		}

		launch_reduceOFfsetTrees(theOffsets, indx, A, B, C, t);
		t->pop_back();
	}
}
