#ifndef GPU_SPARSE_MULTIPLICATION_H
#define GPU_SPARSE_MULTIPLICATION_H
#include <thread>

#include "../../CommonOffset.h"
#include "IMultiplicationStrategy.h"

extern int launch_intersectionKernel(CommonOffset* c, const int* a, const int* b, const int min,
                                     const int max, const int* aPos, const int* bPos);

extern void launch_reduceOFfsetTrees(CommonOffset* c, int offset, ISparseMatrix* A,
                                     ISparseMatrix* B, ISparseMatrix* C,std::vector<int>* t);


extern int launchMultiplication( ISparseMatrix* A,
    ISparseMatrix* B, ISparseMatrix* C);
class GPUSparseMultiplication : public IMultiplicationStrategy {
   public:
    ~GPUSparseMultiplication() {}

    ISparseMatrix* multiply(ISparseMatrix* A, ISparseMatrix* B, ISparseMatrix* C) {
        m_Multi = 0;
        if (A->size() == 0 || B->size() == 0) {
            return C;
        }

        /*
        const int size = 5;
        FlatNode visitLeft = A->getNodes()[0];
        FlatNode visitRight = B->getNodes()[0];

        auto info = collectIndexInformation(visitLeft, visitRight, A, B);
        const int szLeft = info.get()->maxOffsetLeft;
        const int szRight = info.get()->maxOffsetRight;
        const int mx = info.get()->maxSize;

        CommonOffset* offsets = (CommonOffset*)malloc(sizeof(CommonOffset) * szLeft);

        int indices = launch_intersectionKernel(
            offsets, &info.get()->leftIndices[0], &info.get()->rightIndices[0], szLeft, mx,
            &info.get()->leftIndexPos[0], &info.get()->rightIndexPos[0]);

        if (indices == 0) {
            free(offsets);
            return C;
        }

        std::vector<int> t;
        launch_reduceOFfsetTrees(offsets,indices,A,B,C,&t);
        */
		int multiplications = launchMultiplication(A, B, C);
		m_Multi += multiplications;
        return C;
    }
};
#endif