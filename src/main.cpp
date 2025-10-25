#include "SparseMatrix/SparseMatrix.h"
#include <assert.h>
#include <chrono>
#include <iostream>
using namespace std;

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

void testInsert();
void testDelete();
void testIterator();
void testIteratorPerf();
void assertTupleEquality(const SparseMatrixTuple &t1,
                         const SparseMatrixTuple &t2);
void testMultiplication();
void testMultiplicationPerf();
void testMultiplicationPerfMulti();

int main() {
    /*    testInsert();
        testDelete();
        testIterator();
        testIteratorPerf();*/
    //      testMultiplication();
   //testMultiplicationPerf();

    testMultiplicationPerfMulti();
}
void testMultiplicationPerfMulti() {
    auto t1 = high_resolution_clock::now();
    SparseMatrix A;
    int I = 100;
    int J = 100;
    int K = 100;
    int stride = 15;
    int executionTimes = 200;
    int aSize = 0;
    int bSize = 0;
    std::cout << "Matrix construction started!" << std::endl;

    // dense
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            for (int k = 0; k < K; k++) {
                A.insert({i, j, k}, (i + j + k + 1));
                aSize++;
            }
        }
    }

    // sparse
    SparseMatrix B;
    std::vector<SparseMatrixTuple> groundTruth;

    for (int i = 0; i < I; i += stride) {
        for (int j = 0; j < J; j += stride) {
            for (int k = 0; k < K; k += stride) {
                B.insert({i, j, k}, 2);
                groundTruth.push_back({{i, j, k}, 2.0 * (i + j + k + 1)});
                bSize++;
            }
        }
    }

    auto t2 = high_resolution_clock::now();
    auto ms_int = duration_cast<milliseconds>(t2 - t1);

    duration<double, std::milli> ms_double = t2 - t1;
    std::cout << "A size = " << aSize << std::endl;
    std::cout << "B size = " << bSize << std::endl;

    std::cout << "Matrix construction ended!" << std::endl;

    t1 = high_resolution_clock::now();
    for (int i = 0; i < executionTimes; i++) {
        SparseMatrix C = B * A;
    }
    t2 = high_resolution_clock::now();
    ms_int = duration_cast<milliseconds>(t2 - t1);

    ms_double = t2 - t1;
    std::cout << ms_int.count() << "ms\n";
    std::cout << ms_double.count() << "ms\n";
    std::cout << "New Multiplication ended!" << std::endl;

    t1 = high_resolution_clock::now();
    for (int i = 0; i < executionTimes; i++) {
        SparseMatrix C = B.oldMultiplication(A);
    }
    t2 = high_resolution_clock::now();
    ms_int = duration_cast<milliseconds>(t2 - t1);

    ms_double = t2 - t1;
    std::cout << ms_int.count() << "ms\n";
    std::cout << ms_double.count() << "ms\n";
    std::cout << "New Multiplication ended!" << std::endl;
}

void testMultiplicationPerf() {
    auto t1 = high_resolution_clock::now();
    SparseMatrix A;
    int I = 100;
    int J = 100;
    int K = 100;
    int stride = 8;

    int aSize = 0;
    int bSize = 0;
    std::cout << "Matrix construction started!" << std::endl;

    // dense
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            for (int k = 0; k < K; k++) {
                A.insert({i, j, k}, (i + j + k + 1));
                aSize++;
            }
        }
    }

    // sparse
    SparseMatrix B;
    std::vector<SparseMatrixTuple> groundTruth;

    for (int i = 0; i < I; i += stride) {
        for (int j = 0; j < J; j += stride) {
            for (int k = 0; k < K; k += stride) {
                B.insert({i, j, k}, 2);
                groundTruth.push_back({{i, j, k}, 2.0 * (i + j + k + 1)});
                bSize++;
            }
        }
    }

    auto t2 = high_resolution_clock::now();
    auto ms_int = duration_cast<milliseconds>(t2 - t1);

    duration<double, std::milli> ms_double = t2 - t1;
    std::cout << "A size = " << aSize << std::endl;
    std::cout << "B size = " << bSize << std::endl;

    std::cout << "Matrix construction ended!" << std::endl;
    std::cout << ms_int.count() << "ms\n";
    std::cout << ms_double.count() << "ms\n";

    std::cout << "Multiplication started!" << std::endl;
    t1 = high_resolution_clock::now();
    {
        SparseMatrix C = A * B;

        t2 = high_resolution_clock::now();
        ms_int = duration_cast<milliseconds>(t2 - t1);

        ms_double = t2 - t1;

        SparseMatrixIterator iterator = C.iterator();
        int i = 0;
        for (const SparseMatrixTuple &tuple : iterator) {
            //       std::cout << "{" << tuple.tuple[0] << "," << tuple.tuple[1]
            //       <<
            //       "," << tuple.tuple[2] << "} := " << tuple.value <<
            //       std::endl;
            assertTupleEquality(tuple, groundTruth[i++]);
        }
        assert(i == (int)groundTruth.size());
        std::cout << ms_int.count() << "ms\n";
        std::cout << ms_double.count() << "ms\n";
        std::cout << "Multiplication ended!" << std::endl;
    }
    {
        std::cout << "OLD Multiplication started!" << std::endl;
        t1 = high_resolution_clock::now();

        SparseMatrix OC = A.oldMultiplication(B);

        t2 = high_resolution_clock::now();
        ms_int = duration_cast<milliseconds>(t2 - t1);

        ms_double = t2 - t1;

        SparseMatrixIterator iteratorD = OC.iterator();
        int i = 0;
        for (const SparseMatrixTuple &tuple : iteratorD) {
            //       std::cout << "{" << tuple.tuple[0] << "," << tuple.tuple[1]
            //       <<
            //       "," << tuple.tuple[2] << "} := " << tuple.value <<
            //       std::endl;
            assertTupleEquality(tuple, groundTruth[i++]);
        }
        assert(i == (int)groundTruth.size());
        std::cout << ms_int.count() << "ms\n";
        std::cout << ms_double.count() << "ms\n";
        std::cout << "OLD Multiplication ended!" << std::endl;
    }
}

void testMultiplication() {

    SparseMatrix A;
    A.insert({1, 1, 1}, 2);
    A.insert({1, 1, 2}, 3);
    A.insert({1, 2, 1}, 4);
    A.insert({50, 2, 1}, 5);

    SparseMatrix B;
    B.insert({1, 1, 1}, 2);
    B.insert({1, 1, 20}, 3);
    B.insert({1, 20, 1}, 4);
    B.insert({50, 2, 1}, 5);

    SparseMatrix C = A * B;
    SparseMatrixIterator iterator = C.iterator();

    std::vector<SparseMatrixTuple> groundTruth;
    groundTruth.push_back({{1, 1, 1}, 4});
    groundTruth.push_back({{50, 2, 1}, 25});
    int i = 0;
    for (const SparseMatrixTuple &tuple : iterator) {
        std::cout << "{" << tuple.tuple[0] << "," << tuple.tuple[1] << ","
                  << tuple.tuple[2] << "} := " << tuple.value << std::endl;
        assertTupleEquality(tuple, groundTruth[i++]);
    }
    assert(i == (int)groundTruth.size());
}

void testIterator() {
    SparseMatrix A;
    A.insert({10, 5, 2}, 4);
    A.insert({10, 5, 3}, 5);
    A.insert({10, 5, 4}, 6);
    A.insert({10, 5, 5}, 7);
    A.insert({10, 5, 6}, 8);

    A.insert({1, 1, 10}, 2);
    A.insert({1, 2, 3}, 3);
    A.insert({1, 1, 1}, 1);
    A.insert({10, 6, 1}, 9);
    A.insert({1, 3, 1}, 10);
    /*
        A.insert({1,1,4},4);
        A.insert({1,1,5},5);
        A.insert({1,1,6},6);

        A.insert({1,1,1},1);
        A.insert({1,1,2},2);
        A.insert({1,1,3},3);

    A.insert({255,2,222},7);*/
    std::vector<SparseMatrixTuple> groundTruth;
    groundTruth.push_back({{1, 1, 1}, 1});
    groundTruth.push_back({{1, 1, 10}, 2});
    groundTruth.push_back({{1, 2, 3}, 3});
    groundTruth.push_back({{1, 3, 1}, 10});
    groundTruth.push_back({{10, 5, 2}, 4});
    groundTruth.push_back({{10, 5, 3}, 5});
    groundTruth.push_back({{10, 5, 4}, 6});
    groundTruth.push_back({{10, 5, 5}, 7});
    groundTruth.push_back({{10, 5, 6}, 8});
    groundTruth.push_back({{10, 6, 1}, 9});

    SparseMatrixIterator iterator = A.iterator();
    int i = 0;
    for (const SparseMatrixTuple &tuple : iterator) {
        std::cout << "{" << tuple.tuple[0] << "," << tuple.tuple[1] << ","
                  << tuple.tuple[2] << "} := " << tuple.value << std::endl;
        assertTupleEquality(tuple, groundTruth[i++]);
    }
    assert(i == (int)groundTruth.size());
}

void assertTupleEquality(const SparseMatrixTuple &t1,
                         const SparseMatrixTuple &t2) {
    int sz = (int)t2.tuple.size();

    assert((int)t1.tuple.size() == sz);
    assert(t1.value == t2.value);
    for (int i = 0; i < sz; i++) {
        assert(t1.tuple[i] == t2.tuple[i]);
    }
}

void testIteratorPerf() {
    auto t1 = high_resolution_clock::now();
    int items = 0;
    SparseMatrix A;
    int I = 100;
    int J = 100;
    int K = 100;
    for (int i = 0; i < I; i++) {
        for (int j = 0; j < J; j++) {
            for (int k = 0; k < K; k++) {
                A.insert({i, j, k}, i);
                items++;
            }
        }
    }
    auto t2 = high_resolution_clock::now();
    auto ms_int = duration_cast<milliseconds>(t2 - t1);

    duration<double, std::milli> ms_double = t2 - t1;
    std::cout << ms_int.count() << "ms\n";
    std::cout << ms_double.count() << "ms\n";
    std::cout << "Inserted " << items << " items." << std::endl;

    t1 = high_resolution_clock::now();
    SparseMatrixIterator iterator = A.iterator();
    int i = 0;
    for (const SparseMatrixTuple &tuple : iterator) {
        // std::cout << "{" << tuple.tuple[0] << "," << tuple.tuple[1] << "," <<
        // tuple.tuple[2] << "} := " << tuple.value << std::endl;
    }
    t2 = high_resolution_clock::now();
    ms_int = duration_cast<milliseconds>(t2 - t1);

    ms_double = t2 - t1;
    std::cout << "Iterated in " << ms_int.count() << "ms\n";
    std::cout << "Iterated in " << ms_double.count() << "ms\n";
    std::cout << "Inserted " << items << " items." << std::endl;
}

void testDelete() {
    SparseMatrix A;
    A.insert({1, 1, 1}, 2);
    assert(A.size() == 1);
    assert(A.insert({1, 1, 1}, 0) == true);

    SparseMatrix B;
    B.insert({1, 5, 6}, 11);
    B.insert({6, 5, 6}, 13);
    B.insert({4, 5, 6}, 133);
    B.insert({5, 5, 6}, 143);
    B.insert({0, 5, 6}, 155);

    assert(B.size() == 5);

    // delete unique tuple that first index is the highest
    assert(B.erase({6, 5, 6}));

    B.assertFlatChildrenValues({{0, 10},
                                {1, 1},
                                {4, 4},
                                {5, 7},
                                {5, 2},
                                {6, 3},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9},
                                {5, 11},
                                {6, 12}});
    B.assertFlatNodeValues({{0, 4, 0, 0},
                            {4, 1, 0, 0},
                            {5, 1, 0, 0},
                            {-1, 0, 1, 11},
                            {6, 1, 0, 0},
                            {7, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {8, 1, 0, 0},
                            {9, 1, 0, 0},
                            {-1, 0, 1, 143},
                            {10, 1, 0, 0},
                            {11, 1, 0, 0},
                            {-1, 0, 1, 155}});
    assert(B.size() == 4);

    assert(B.erase({0, 5, 6}));
    assert(B.size() == 3);

    B.assertFlatChildrenValues({{1, 1},
                                {4, 4},
                                {5, 7},
                                {5, 2},
                                {6, 3},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9}});
    B.assertFlatNodeValues({{0, 3, 0, 0},
                            {3, 1, 0, 0},
                            {4, 1, 0, 0},
                            {-1, 0, 1, 11},
                            {5, 1, 0, 0},
                            {6, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {7, 1, 0, 0},
                            {8, 1, 0, 0},
                            {-1, 0, 1, 143}});

    assert(B.erase({4, 5, 6}));
    assert(B.size() == 2);

    B.assertFlatChildrenValues(
        {{1, 1}, {5, 4}, {5, 2}, {6, 3}, {5, 5}, {6, 6}});
    B.assertFlatNodeValues({{0, 2, 0, 0},
                            {2, 1, 0, 0},
                            {3, 1, 0, 0},
                            {-1, 0, 1, 11},
                            {4, 1, 0, 0},
                            {5, 1, 0, 0},
                            {-1, 0, 1, 143}});

    assert(B.erase({5, 5, 6}));
    assert(B.size() == 1);

    B.assertFlatChildrenValues({{1, 1}, {5, 2}, {6, 3}});
    B.assertFlatNodeValues(
        {{0, 1, 0, 0}, {1, 1, 0, 0}, {2, 1, 0, 0}, {-1, 0, 1, 11}});

    assert(B.erase({1, 5, 6}));
    assert(B.size() == 0);

    B.assertFlatChildrenValues({});
    B.assertFlatNodeValues({{0, 0, 0, 0}});
    return;
}

void testInsert() {
    auto t1 = high_resolution_clock::now();

    SparseMatrix A;
    A.insert({1, 5, 6}, 11.0);

    assert(A.getValue({1, 5, 6}) == 11.0);
    assert(A.size() == 1);

    A.assertFlatChildrenValues({{1, 1}, {5, 2}, {6, 3}});
    A.assertFlatNodeValues(
        {{0, 1, 0, 0}, {1, 1, 0, 0}, {2, 1, 0, 0}, {-1, 0, 1, 11}});

    // first index different
    A.insert({6, 5, 6}, 13.0);

    assert(A.getValue({1, 5, 6}) == 11.0);
    assert(A.getValue({6, 5, 6}) == 13.0);
    assert(A.size() == 2);

    A.assertFlatChildrenValues(
        {{1, 1}, {6, 4}, {5, 2}, {6, 3}, {5, 5}, {6, 6}});
    A.assertFlatNodeValues({{0, 2, 0, 0},
                            {2, 1, 0, 0},
                            {3, 1, 0, 0},
                            {-1, 0, 1, 11},
                            {4, 1, 0, 0},
                            {5, 1, 0, 0},
                            {-1, 0, 1, 13}});

    // first index between first indices 1,6=>4
    A.insert({4, 5, 6}, 133.0);

    assert(A.getValue({1, 5, 6}) == 11.0);
    assert(A.getValue({6, 5, 6}) == 13.0);
    assert(A.getValue({4, 5, 6}) == 133.0);
    assert(A.size() == 3);

    A.assertFlatChildrenValues({{1, 1},
                                {4, 7},
                                {6, 4},
                                {5, 2},
                                {6, 3},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9}});
    A.assertFlatNodeValues({{0, 3, 0, 0},
                            {3, 1, 0, 0},
                            {4, 1, 0, 0},
                            {-1, 0, 1, 11},
                            {5, 1, 0, 0},
                            {6, 1, 0, 0},
                            {-1, 0, 1, 13},
                            {7, 1, 0, 0},
                            {8, 1, 0, 0},
                            {-1, 0, 1, 133}});

    A.insert({5, 5, 6}, 143.0);

    assert(A.getValue({1, 5, 6}) == 11.0);
    assert(A.getValue({6, 5, 6}) == 13.0);
    assert(A.getValue({4, 5, 6}) == 133.0);
    assert(A.getValue({5, 5, 6}) == 143.0);
    assert(A.size() == 4);

    A.assertFlatChildrenValues({{1, 1},
                                {4, 7},
                                {5, 10},
                                {6, 4},
                                {5, 2},
                                {6, 3},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9},
                                {5, 11},
                                {6, 12}});
    A.assertFlatNodeValues({{0, 4, 0, 0},
                            {4, 1, 0, 0},
                            {5, 1, 0, 0},
                            {-1, 0, 1, 11},
                            {6, 1, 0, 0},
                            {7, 1, 0, 0},
                            {-1, 0, 1, 13},
                            {8, 1, 0, 0},
                            {9, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {10, 1, 0, 0},
                            {11, 1, 0, 0},
                            {-1, 0, 1, 143}});

    A.insert({0, 5, 6}, 155.0);

    assert(A.getValue({1, 5, 6}) == 11.0);
    assert(A.getValue({6, 5, 6}) == 13.0);
    assert(A.getValue({4, 5, 6}) == 133.0);
    assert(A.getValue({5, 5, 6}) == 143.0);
    assert(A.getValue({0, 5, 6}) == 155.0);

    assert(A.size() == 5);

    A.assertFlatChildrenValues({{0, 13},
                                {1, 1},
                                {4, 7},
                                {5, 10},
                                {6, 4},
                                {5, 2},
                                {6, 3},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9},
                                {5, 11},
                                {6, 12},
                                {5, 14},
                                {6, 15}});
    A.assertFlatNodeValues({{0, 5, 0, 0},
                            {5, 1, 0, 0},
                            {6, 1, 0, 0},
                            {-1, 0, 1, 11},
                            {7, 1, 0, 0},
                            {8, 1, 0, 0},
                            {-1, 0, 1, 13},
                            {9, 1, 0, 0},
                            {10, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {11, 1, 0, 0},
                            {12, 1, 0, 0},
                            {-1, 0, 1, 143},
                            {13, 1, 0, 0},
                            {14, 1, 0, 0},
                            {-1, 0, 1, 155}});

    // First Index Match, Second Lower
    SparseMatrix B;
    B.insert({1, 5, 6}, 11.0);
    B.insert({6, 5, 6}, 13.0);
    B.insert({4, 5, 6}, 133.0);
    B.insert({1, 4, 7}, 2);

    assert(B.getValue({1, 5, 6}) == 11.0);
    assert(B.getValue({6, 5, 6}) == 13.0);
    assert(B.getValue({4, 5, 6}) == 133.0);
    assert(B.getValue({1, 4, 7}) == 2.0);
    assert(B.size() == 4);

    B.assertFlatChildrenValues({{1, 1},
                                {4, 7},
                                {6, 4},
                                {4, 10},
                                {5, 2},
                                {6, 3},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9},
                                {7, 11}});
    B.assertFlatNodeValues({{0, 3, 0, 0},
                            {3, 2, 0, 0},
                            {5, 1, 0, 0},
                            {-1, 0, 1, 11},
                            {6, 1, 0, 0},
                            {7, 1, 0, 0},
                            {-1, 0, 1, 13},
                            {8, 1, 0, 0},
                            {9, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {10, 1, 0, 0},
                            {-1, 0, 1, 2}});

    B.insert({1, 10, 7}, 7);

    assert(B.getValue({1, 5, 6}) == 11.0);
    assert(B.getValue({6, 5, 6}) == 13.0);
    assert(B.getValue({4, 5, 6}) == 133.0);
    assert(B.getValue({1, 4, 7}) == 2.0);
    assert(B.getValue({1, 10, 7}) == 7.0);
    assert(B.size() == 5);

    B.assertFlatChildrenValues({{1, 1},
                                {4, 7},
                                {6, 4},
                                {4, 10},
                                {5, 2},
                                {10, 12},
                                {6, 3},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9},
                                {7, 11},
                                {7, 13}});
    B.assertFlatNodeValues({{0, 3, 0, 0},
                            {3, 3, 0, 0},
                            {6, 1, 0, 0},
                            {-1, 0, 1, 11},
                            {7, 1, 0, 0},
                            {8, 1, 0, 0},
                            {-1, 0, 1, 13},
                            {9, 1, 0, 0},
                            {10, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {11, 1, 0, 0},
                            {-1, 0, 1, 2},
                            {12, 1, 0, 0},
                            {-1, 0, 1, 7}});

    B.insert({1, 7, 7}, 18);

    assert(B.getValue({1, 5, 6}) == 11.0);
    assert(B.getValue({6, 5, 6}) == 13.0);
    assert(B.getValue({4, 5, 6}) == 133.0);
    assert(B.getValue({1, 4, 7}) == 2.0);
    assert(B.getValue({1, 10, 7}) == 7.0);
    assert(B.getValue({1, 7, 7}) == 18.0);
    assert(B.size() == 6);

    B.assertFlatChildrenValues({{1, 1},
                                {4, 7},
                                {6, 4},
                                {4, 10},
                                {5, 2},
                                {7, 14},
                                {10, 12},
                                {6, 3},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9},
                                {7, 11},
                                {7, 13},
                                {7, 15}});
    B.assertFlatNodeValues({{0, 3, 0, 0},
                            {3, 4, 0, 0},
                            {7, 1, 0, 0},
                            {-1, 0, 1, 11},
                            {8, 1, 0, 0},
                            {9, 1, 0, 0},
                            {-1, 0, 1, 13},
                            {10, 1, 0, 0},
                            {11, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {12, 1, 0, 0},
                            {-1, 0, 1, 2},
                            {13, 1, 0, 0},
                            {-1, 0, 1, 7},
                            {14, 1, 0, 0},
                            {-1, 0, 1, 18}});

    // First two Indices Match
    SparseMatrix C;
    C.insert({1, 5, 6}, 11.0);
    C.insert({6, 5, 6}, 13.0);
    C.insert({4, 5, 6}, 133.0);
    C.insert({1, 5, 1}, 2);

    assert(C.getValue({1, 5, 6}) == 11.0);
    assert(C.getValue({6, 5, 6}) == 13.0);
    assert(C.getValue({4, 5, 6}) == 133.0);
    assert(C.getValue({1, 5, 1}) == 2.0);
    assert(C.size() == 4);

    C.assertFlatChildrenValues({{1, 1},
                                {4, 7},
                                {6, 4},
                                {5, 2},
                                {1, 10},
                                {6, 3},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9}});
    C.assertFlatNodeValues({{0, 3, 0, 0},
                            {3, 1, 0, 0},
                            {4, 2, 0, 0},
                            {-1, 0, 1, 11},
                            {6, 1, 0, 0},
                            {7, 1, 0, 0},
                            {-1, 0, 1, 13},
                            {8, 1, 0, 0},
                            {9, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {-1, 0, 1, 2}});

    // First two Indices Match and 3rd is Larger
    C.insert({1, 5, 10}, 22);

    assert(C.getValue({1, 5, 6}) == 11.0);
    assert(C.getValue({6, 5, 6}) == 13.0);
    assert(C.getValue({4, 5, 6}) == 133.0);
    assert(C.getValue({1, 5, 1}) == 2.0);
    assert(C.getValue({1, 5, 10}) == 22.0);
    assert(C.size() == 5);

    C.assertFlatChildrenValues({{1, 1},
                                {4, 7},
                                {6, 4},
                                {5, 2},
                                {1, 10},
                                {6, 3},
                                {10, 11},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9}});
    C.assertFlatNodeValues({{0, 3, 0, 0},
                            {3, 1, 0, 0},
                            {4, 3, 0, 0},
                            {-1, 0, 1, 11},
                            {7, 1, 0, 0},
                            {8, 1, 0, 0},
                            {-1, 0, 1, 13},
                            {9, 1, 0, 0},
                            {10, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {-1, 0, 1, 2},
                            {-1, 0, 1, 22}});

    C.insert({1, 5, 7}, 33);

    assert(C.getValue({1, 5, 6}) == 11.0);
    assert(C.getValue({6, 5, 6}) == 13.0);
    assert(C.getValue({4, 5, 6}) == 133.0);
    assert(C.getValue({1, 5, 1}) == 2.0);
    assert(C.getValue({1, 5, 10}) == 22.0);
    assert(C.getValue({1, 5, 7}) == 33.0);
    assert(C.size() == 6);

    C.assertFlatChildrenValues({{1, 1},
                                {4, 7},
                                {6, 4},
                                {5, 2},
                                {1, 10},
                                {6, 3},
                                {7, 12},
                                {10, 11},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9}});
    C.assertFlatNodeValues({{0, 3, 0, 0},
                            {3, 1, 0, 0},
                            {4, 4, 0, 0},
                            {-1, 0, 1, 11},
                            {8, 1, 0, 0},
                            {9, 1, 0, 0},
                            {-1, 0, 1, 13},
                            {10, 1, 0, 0},
                            {11, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {-1, 0, 1, 2},
                            {-1, 0, 1, 22},
                            {-1, 0, 1, 33}});

    // Auxiliary

    SparseMatrix D;
    D.insert({1, 5, 6}, 11.0);
    D.insert({6, 5, 6}, 13.0);
    D.insert({4, 5, 6}, 133.0);
    D.insert({1, 5, 1}, 2);
    D.insert({1, 5, 10}, 22.0);
    D.insert({1, 5, 7}, 33);

    assert(D.getValue({1, 5, 6}) == 11.0);
    assert(D.getValue({6, 5, 6}) == 13.0);
    assert(D.getValue({4, 5, 6}) == 133.0);
    assert(D.getValue({1, 5, 10}) == 22.0);
    assert(D.getValue({1, 5, 7}) == 33.0);
    assert(D.getValue({1, 5, 1}) == 2.0);

    assert(D.size() == 6);

    D.assertFlatChildrenValues({{1, 1},
                                {4, 7},
                                {6, 4},
                                {5, 2},
                                {1, 10},
                                {6, 3},
                                {7, 12},
                                {10, 11},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9}});
    D.assertFlatNodeValues({{0, 3, 0, 0},
                            {3, 1, 0, 0},
                            {4, 4, 0, 0},
                            {-1, 0, 1, 11},
                            {8, 1, 0, 0},
                            {9, 1, 0, 0},
                            {-1, 0, 1, 13},
                            {10, 1, 0, 0},
                            {11, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {-1, 0, 1, 2},
                            {-1, 0, 1, 22},
                            {-1, 0, 1, 33}});

    D.insert({2, 2, 2}, 66);

    assert(D.getValue({1, 5, 6}) == 11.0);
    assert(D.getValue({6, 5, 6}) == 13.0);
    assert(D.getValue({4, 5, 6}) == 133.0);
    assert(D.getValue({1, 5, 10}) == 22.0);
    assert(D.getValue({1, 5, 7}) == 33.0);
    assert(D.getValue({1, 5, 1}) == 2.0);
    assert(D.getValue({2, 2, 2}) == 66.0);

    assert(D.size() == 7);

    D.assertFlatChildrenValues({{1, 1},
                                {2, 13},
                                {4, 7},
                                {6, 4},
                                {5, 2},
                                {1, 10},
                                {6, 3},
                                {7, 12},
                                {10, 11},
                                {5, 5},
                                {6, 6},
                                {5, 8},
                                {6, 9},
                                {2, 14},
                                {2, 15}});
    D.assertFlatNodeValues({{0, 4, 0, 0},
                            {4, 1, 0, 0},
                            {5, 4, 0, 0},
                            {-1, 0, 1, 11},
                            {9, 1, 0, 0},
                            {10, 1, 0, 0},
                            {-1, 0, 1, 13},
                            {11, 1, 0, 0},
                            {12, 1, 0, 0},
                            {-1, 0, 1, 133},
                            {-1, 0, 1, 2},
                            {-1, 0, 1, 22},
                            {-1, 0, 1, 33},
                            {13, 1, 0, 0},
                            {14, 1, 0, 0},
                            {-1, 0, 1, 66}});

    SparseMatrix K;
    int items = 0;
    for (int i = 0; i < 10000;) {
        for (int j = 0; j < 10000;) {
            for (int k = 0; k < 10900;) {
                K.insert({i, j, k}, 1);
                j += 11;
                k += 13;
                items++;
            }
        }
        i += 100;
    }

    int overriden = 0;
    for (int i = 0; i < 1000;) {
        for (int j = 0; j < 1009;) {
            for (int k = 0; k < 109;) {
                int old = K.getValue({i, j, k});
                if (old == 0) {
                    items++;
                } else {
                    overriden++;
                }
                K.insert({i, j, k}, 2);
                j += 11;
                k += 13;
            }
        }
        i += 1;
    }
    assert(K.size() == items);

    auto t2 = high_resolution_clock::now();
    auto ms_int = duration_cast<milliseconds>(t2 - t1);

    duration<double, std::milli> ms_double = t2 - t1;
    std::cout << ms_int.count() << "ms\n";
    std::cout << ms_double.count() << "ms\n";
    std::cout << "Inserted " << items << " items." << std::endl;
    std::cout << "Overriden " << overriden << " items." << std::endl;
}
