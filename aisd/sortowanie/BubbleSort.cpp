#include <bits/stdc++.h>

int* BubbleSort(int* A, int n)
{
    for(int i = 1; i < n; i++)
    {
        for(int j = 1; j <= n - i; j++)
        {
            if(A[j] > A[j+1])
            {
                int tmp = A[j];
                A[j] = A[j+1];
                A[j+1] = tmp;
            }
        }
    }
}