#include <cuda.h>
#include <cuda_runtime_api.h>
#include <stdio.h>

#include <memory>

#include "../../CommonOffset.h"
#include "../../FlatIndex.h"
#include "../../ISparseMatrix.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

__global__ void intersectionKernel(CommonOffset* c, const int* a, const int* b, const int min,
                                   const int max, const int* aPos, const int* bPos, int* stackTop) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int lDiff = abs(a[i] - b[0]);
    int rDiff = abs(a[i] - b[max - 1]);
    //  printf("%d \n", i);
    //  printf("%d. %d Left %d != Right %d, b[0]: %d ,b[max-1]: %d \n",i,
    //  a[i],lDiff,rDiff,b[0],b[max-1]);
    if (a[i] >= b[0] && a[i] <= b[max - 1]) {
        if (lDiff > rDiff) {
            int j = max - 1;
            while (i >= 0) {
                if (a[i] == b[j]) {
                    int pos = atomicAdd(stackTop, 1);  // Atomically get the next position
                    if (pos < min) {
                        c[pos] = {aPos[i], bPos[j], a[i]};
                    }
                    return;
                }
                j--;
            }
        } else {
            int j = 0;
            for (; j < max; j++) {
                if (a[i] == b[j]) {
                    int pos = atomicAdd(stackTop, 1);
                    if (pos < min) {
                        c[pos] = {aPos[i], bPos[j], a[i]};
                    }
                    return;
                }
            }
        }
    } else {
        return;
    }
}

int launch_intersectionKernel(CommonOffset* c, const int* a, const int* b, const int min,
                              const int max, const int* aPos, const int* bPos) {
    int *d_a, *d_b, *d_aPos, *d_bPos;
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

    int totalThreads = min;
    int threadsPerBlock = min > 256 ? 256 : min;
    int blocks = (totalThreads + threadsPerBlock - 1) / threadsPerBlock;

    /*std::cout << "Operating in " << blocks << " blocks and " << threadsPerBlock
              << " threads per block" << std::endl;
*/
    intersectionKernel<<<blocks, threadsPerBlock>>>(d_c, d_a, d_b, min, max, d_aPos, d_bPos,
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
                              ISparseMatrix* B, ISparseMatrix* C,std::vector<int>* t) {
    const std::vector<FlatNode>& myNodes = A->getNodes();
    const std::vector<FlatNode>& otherNodes = B->getNodes();

    std::vector<FlatChildEntry>& flatChildren = A->getFlatChildren();
    std::vector<FlatChildEntry>& otherChildren = B->getFlatChildren();

    for (int i = 0; i < indices; i++) {
        // std::cout << "I = "<< i << std::endl;
        CommonOffset& offset = offsets[i];
        //std::vector<int> t;

        // std::cout << "assigned" << std::endl;
        int left = offset.indexLeft;
        int right = offset.indexRight;

        int leftNode = flatChildren[left].nodeIndex;
        int rightNode = otherChildren[right].nodeIndex;

        // std::cout << leftNode << "," << rightNode << std::endl;

        FlatNode visitLeft = myNodes[leftNode];
        FlatNode visitRight = otherNodes[rightNode];

        t->push_back(offset.tupleKey);

        // std::cout << "offset tuple " << offset.tupleKey << std::endl;

        if (visitLeft.isLeaf && visitRight.isLeaf) {
            // do the operation
            /*            m_Multi++;
                        double result = visitLeft.value * visitRight.value;
                        destination->insert(t, result);
                        t.pop_back();*/
/*            for (int i = 0; i < t->size(); i++) {
                std::cout << t->at(i);                
            }*/

            C->insert(*t,visitLeft.value * visitRight.value);
            //std::cout << "are we here?" << visitLeft.value * visitRight.value << std::endl;
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

        launch_reduceOFfsetTrees(theOffsets, indx, A, B, C,t);
        t->pop_back();
    }
}
