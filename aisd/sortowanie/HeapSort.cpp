#include <bits/stdc++.h>

void HeapSort()
{
    CreateHeap();

    while(hl>1)
    {
        double tmp = A[1];
        A[1] = A[hl];
        A[hl] = tmp;
        hl--;
        DownHeap(A[1],hl);
    }
}

struct node
{
    int key;
    node* left, *right;
}
node* head; // lista
node* root = NULL; // kopiec

void HeapSort()
{
    while(head)
    {
        node* ptr = head;
        head = head->right;
     
        ptr->left = ptr->right = NULL;
        root = Union(root, ptr);
    }

    head = NULL;
    while(root)
    {
        node* ptr = DeleteMax();
        ptr->left = NULL;
        ptr->right = head;
        if(head)
            head->left = ptr;
        head = ptr;
    }

}