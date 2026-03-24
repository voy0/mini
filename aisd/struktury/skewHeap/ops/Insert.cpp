#include "../skewHeap.cpp"
#include "./Union.cpp"

void Insert(int v)
{
    node* p = new node;
    p->key = v;
    p->left = NULL;
    p->right = NULL;
    root = Union(root, p);
}