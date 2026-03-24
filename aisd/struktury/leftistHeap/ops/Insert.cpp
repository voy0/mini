#include "../leftistHeap.cpp"
#include "./Union.cpp"

void Insert(int v)
{
    node* p = new node;
    p->key = v;
    p->left = NULL;
    p->right = NULL;
    p->npl = 0;
    root = Union(root, p);
}