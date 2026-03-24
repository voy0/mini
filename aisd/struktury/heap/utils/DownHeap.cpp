#include <stdlib.h>
int n;
int A[n];

void DownHeap(int i, int n)
{
    int k = 2*i;
    int v = A[i];
    while(k <= n)
    {
        // wybranie który child jest większy
        if(k+1 <= n)
            if(A[k+1] > A[k])
                k = k + 1;

        // zjeżdzanie w dół dopóki element następny nie jest większy
        // każdy element przesuwany jest o jeden a na końcu dopiero dodajemy v
        if(v < A[k])
        {
            A[i] = A[k];
            i = k;
            k = 2*i;
        }
        else
            break;
    }
    //Finalny krok !!!!
    A[i] = v;
}