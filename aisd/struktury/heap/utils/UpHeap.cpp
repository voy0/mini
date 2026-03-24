#include <stdlib.h>
int A[];
A[0] = MaxInt;

void UpHeap(int i)
{
    // zakłada że element jest już na dole kopca, algorytm przestawia do wyżej
    // i - indeks elementu do UpHeapowania
    int v = A[i];
    while(v > A[i/2])
    {
        A[i] = A[i/2];
        i /= 2;
    }
    A[i] = v;
}