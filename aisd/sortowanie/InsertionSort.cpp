#include <bits/stdc++.h>

int* InsertionSort(int* A, int n)
{
    A[0] = -INFINITY;
    for(int i = 2; i <= n; i++)
    {
        int v = A[i];
        int j = i-1;
        while(v < A[j])
        {
            A[j+1] = A[j];
            j--;
        }
        A[j+1] = v;
    }
}