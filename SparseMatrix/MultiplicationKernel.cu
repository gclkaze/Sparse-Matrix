
#include "Intersection.h"
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <stdio.h>
#include <memory>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <iostream>
__global__ void multiply(float* aNodes, float* bNodes, float* results, int numElements) {
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx < numElements) {
		results[idx] = __fmaf_rn( aNodes[idx] , bNodes[idx] ,0.0);
	}
}
float* gpuMultiplySparseMatrices(Intersection *i) {
	auto valuesA = i->getValuesA();
	auto valuesB = i->getValuesB();
	float* d_aNodes, *d_bNodes, *d_Results;
	int nElements = valuesA.size();
	int size = nElements * sizeof(float);

	cudaMallocManaged(&d_aNodes, size);
	cudaMallocManaged(&d_bNodes, size);
	cudaMallocManaged(&d_Results, size);

	cudaMemcpy(d_aNodes, &valuesA[0], size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_bNodes, &valuesB[0], size, cudaMemcpyHostToDevice);

	// Compute optimal block size
/*	cudaOccupancyMaxPotentialBlockSize(
		&minGridSize,
		&blockSize,
		multiply,
		0, 
		0  
	);
	*/
	multiply << < (nElements + 255) / 256, 256 >> > (d_aNodes, d_bNodes, d_Results, nElements);

	cudaDeviceSynchronize(); // Wait for kernel to finish

	cudaError_t err = cudaGetLastError();
	if (err != cudaSuccess) {
		printf("CUDA Error: %s\n", cudaGetErrorString(err));
	}

	cudaDeviceProp prop;
	cudaGetDeviceProperties(&prop, 0);
	return d_Results;
}
