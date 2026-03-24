#include <bits/stdc++.h>

struct node
{
    int key;
    node* next;
}*root;

int* SelectionSort(int* A, int n)
{
    int jMin = 0;
    for(int i = 1; i < n; i++)
    {
        for(int j = i; j < n; j++)
        {
            if(A[j] < A[jMin]){
                jMin = j;
            }
        }

        int tmp = A[i];
        A[i] = A[jMin];
        A[jMin] = tmp;
    }
}
// dla list jest stabilny
node* SelectionSort(node* root)
{
    node* pMin = root;
    node* ppMin = root;
    node* p = root;
    node* pp = root;
    node* newRoot = new node;
    while(p)
    {
        if(p->key < pMin->key)
        {
            ppMin = pp;
            pMin = p;
        }
        pp = p;
        p = p->next;
    }

}