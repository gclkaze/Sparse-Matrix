# Sparse-Matrix
A class that represents a Sparse Matrix by storing tuple/data nodes and tuple connections in std::vectors rather than pointers. A SparseMatrix iterator is provided for looping through the tuples.

# Reason
Insert non-continuous tuples and their associated values and be able to iterate over them in a sorted and intuitive way.

# Usage
See below the API usage of the Sparse Matrix.

### API
```
#include "SparseMatrix/SparseMatrix.h"

...
SparseMatrix A;
int I = 100;
int J = 100;
int K = 100;
int stride = 3;
int aSize = 0;

for (int i = I / 2; i < I; i+=stride) {
    for (int j = J / 2; j < J; j++) {
        for (int k = K / 2; k < K; k++) {
            //if there was a success, insert returns true
            assert ( A.insert({i, j, k}, (i + j + k + 1)) );
            aSize ++;
        }
    }
}

//getValue, if the tuple doesn't exists, the method returns 0.0f;
assert(A.getValue({1,1,1}) == 0.0f);
assert(A.getValue({I/2,J/2,K/2}) == 151.0f);

//size()
assert(A.size() == aSize);

//erase will return false if the tuple is not present in the Sparse Matrix
assert(A.erase({0,0,0}) == false);

//erase will return true if the tuple is present
assert(A.erase({I/2,J/2,K/2}) == true);

//clear will clean the internal state of the Sparse Matrix
A.clear();
assert ( A.size() == 0);


```

### Iterator
```
#include "SparseMatrix/SparseMatrix.h"
...

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

SparseMatrixIterator iterator = A.iterator();
int i = 0;
for (const SparseMatrixTuple& tuple : iterator) {
    tuple.dump();
}

/* The output of the ranged for-loop will be sorted by tuple index as follows: 
{1, 1, 1}:= 1
{1, 1, 10}:= 2
{1, 2, 3}:= 3
{1, 3, 1}:= 10
{10, 5, 2}:= 4
{10, 5, 3}:= 5
{10, 5, 4}:= 6
{10, 5, 5}:= 7
{10, 5, 6}:= 8
{10, 6, 1}:= 9
*/
```

# Implementation
Used a ___flatten generic tree implementation approach___, where the Sparse Matrix uses two Arrays for the information representation; a __Children__ array and a __Node__ array. We used arrays in order to not use pointer logic where an object can be stored anywhere in memory (pointer chasing) while by using arrays we utilize contiguous memory and obtain better __cache locality__.

# Thread Safety
Yes, it uses an atomic flag for test and set semantics, locking the data structure allowing insertions/deletions by
one thread at a time. In the todos, a lock-free implementation of the Sparse Matrix will be designed (or adjust the current design).

# Tuple Insertion Representation
The Sparse Matrix uses two Arrays to maintain its state and values; the Children and the Nodes arrays.
We use the nodes to hold values and tuple hop information, and the children to hold the tuple keys and node hop information. Thus, every new tuple (with 3 dimensions) will create maximum 3 new children and 3 new nodes to maintain its information. It is important to mention that only the Children array gets resorted based on the tuple keys that are inserted; if the tuple {5,2,3} is inserted, and then the tuple {5,1,3} is inserted, the tuple key that holds the information for child "1" will be inserted before child that holds information for "2".

The following scenarios will show you the insertion process; where all changes in the existing registrations are showed in __"bold"__.  

Initially:
| Children |             |            |
| -------- | ----------- | ---------- |
| I        | Tuple Index | Node Index |

| Nodes |             |            |
| -------- | ----------- | ---------- |
| I | Child Offset | NumChildren | Leaf | Value |   
| 0 | 0            | 0           | 0    |       |

Insert: {1,5,6} = 11
| Children |             |            |
| -------- | ----------- | ---------- |
| I        | Tuple Index | Node Index |
| 0        | 1           | 1          |
| 1        | 5           | 2          |
| 2        | 6           | 3          |

| Nodes |             |            |
| -------- | ----------- | ---------- |
| I | Child Offset | NumChildren | Leaf | Value |   
| 0 | 0            | 1           | 0    |       |
| 1 | 1            | 1           | 0    |       |
| 2 | 2            | 1           | 0    |       |
| 3 | \-1          | 0           | 1    | 11    |

Depicted Tuples: {1,5,6} = 11

Insert: {6,5,6} = 13

| Children |             |            |  |  
| -------- | ----------- | ---------- | -------- 
| I | Tuple Index | Node Index |          |
| 0 | 1           | 1          |          |
| __1__ | __6__           | __4__          | __<-INSERT__ |
| 2 | 5           | 2          |          |
| 3 | 6           | 3          |          |
| 4 | 5           | 5          |          |
| 5 | 6           | 6          |          |

| Nodes |             |            |
| -------- | ----------- | ---------- |
| I | Child Offset | NumChildren | Leaf | Value |
| 0 | 0            | __2__           | 0    |       |
| __1__ | __2__            | 1           | 0    |       |
| __2__ | __3__            | 1           | 0    |       |
| 3 | \-1          | 0           | 1    | 11    |
| 4 | 4            | 1           | 0    |       |
| 5 | 5            | 1           | 0    |       |
| 6 | \-1          | 0           | 1    | 13    |

Depicted Tuples: {1,5,6} = 11, {6,5,6} = 13


Insert: {4,5,6} = 133

| Children |             |            |          |
| ------------- | ----------- | ---------- | -------- |
| I             | Tuple Index | Node Index |          |
| 0             | 1           | __1__          |          |
| __1__             | __4__           | __7__          | __<-INSERT__ |
| 2             | 6           | __4__          |          |
| 3             | 5           | 2          |          |
| 4             | 6           | 3          |          |
| 5             | 5           | 5          |          |
| 6             | 6           | 6          |          |
| 7             | 5           | 8          |          |
| 8             | 6           | 9          |          |


| Nodes |             |            |
| -------- | ----------- | ---------- |
| I | Child Offset | NumChildren | Leaf | Value |
| 0 | 0            | __3__           | 0    |       |
| __1__ | __3__            | 1           | 0    |       |
| __2__ | __4__            | 1           | 0    |       |
| 3 | \-1          | 0           | 1    | 11    |
| __4__ | __5__            | 1           | 0    |       |
| __5__ | __6__            | 1           | 0    |       |
| 6 | \-1          | 0           | 1    | 13    |
| 7 | 7            | 1           | 0    |       |
| 8 | 8            | 1           | 0    |       |
| 9 | \-1          | 0           | 1    | 133   |

Depicted Tuples: {1,5,6} = 11, {6,5,6} = 13, {4,5,6} =  133

Insert: {5,5,6} = 143

| Children |             |            |          |
| ------------- | ----------- | ---------- | -------- |
| I  | Tuple Index | Node Index |          |
| 0  | 1           | 1          |          |
| 1  | 4           | 7          |          |
| __2__  | __5__           | __10__         | __<-INSERT__ |
| 3  | 6           | 4          |          |
| 4  | 5           | 2          |          |
| 5  | 6           | 3          |          |
| 6  | 5           | 5          |          |
| 7  | 6           | 6          |          |
| 8  | 5           | 8          |          |
| 9  | 6           | 9          |          |
| 10 | 5           | 11         |          |
| 11 | 6           | 12         |          |

| Nodes |             |            |
| -------- | ----------- | ---------- |
| I  | Child Offset | NumChildren | Leaf | Value |
| 0  | 0            | __4__           | 0    |       |
| __1__  | __4__            | 1           | 0    |       |
| __2__  | __5__            | 1           | 0    |       |
| 3  | \-1          | 0           | 1    | 11    |
| __4__  | __6__            | 1           | 0    |       |
| __5__  | __7__            | 1           | 0    |       |
| 6  | \-1          | 0           | 1    | 13    |
| __7__  | __8__            | 1           | 0    |       |
| __8__  | __9__            | 1           | 0    |       |
| 9  | \-1          | 0           | 1    | 133   |
| 10 | 10           | 1           | 0    |       |
| 11 | 11           | 1           | 0    |       |
| 12 | \-1          | 0           | 1    | 143   |

Depicted Tuples: {1,5,6} = 11, {6,5,6} = 13, {4,5,6} =  133, {5,5,6} = 143

Insert: {0,5,6} = 155

| Children |             |            |          |
| ------------- | ----------- | ---------- | -------- |
| I             | Tuple Index | Node Index |          |
| __0__             | __0__           | __13__         | __<-INSERT__ |
| 1             | 1           | 1          |          |
| 2             | 4           | 7          |          |
| 3             | 5           | 10         |          |
| 4             | 6           | 4          |          |
| 5             | 5           | 2          |          |
| 6             | 6           | 3          |          |
| 7             | 5           | 5          |          |
| 8             | 6           | 6          |          |
| 9             | 5           | 8          |          |
| 10            | 6           | 9          |          |
| 11            | 5           | 11         |          |
| 12            | 6           | 12         |          |
| 13            | 5           | 14         |          |
| 14            | 6           | 15         |          |


| Nodes |             |            |          |
| ------------- | ----------- | ---------- | -------- |
| I  | Child Offset | NumChildren | Leaf | Value |
| 0  | 0            | __5__           | 0    |       |
| __1__  | __5__            | 1           | 0    |       |
| __2__  | __6__            | 1           | 0    |       |
| 3  | \-1          | 0           | 1    | 11    |
| __4__  | __7__            | 1           | 0    |       |
| __5__  | __8__            | 1           | 0    |       |
| __6__  | \-1          | 0           | 1    | 13    |
| __7__  | __9__            | 1           | 0    |       |
| __8__  | __10__           | 1           | 0    |       |
| 9  | \-1          | 0           | 1    | 133   |
| __10__ | __11__           | 1           | 0    |       |
| __11__ | __12__           | 1           | 0    |       |
| 12 | \-1          | 0           | 1    | 143   |
| 13 | 13           | 1           | 0    |       |
| 14 | 14           | 1           | 0    |       |
| 15 | \-1          | 0           | 1    | 155   |


Depicted Tuples: {1,5,6} = 11, {6,5,6} = 13, {4,5,6} =  133, {5,5,6} = 143, {0,5,6} = 155
