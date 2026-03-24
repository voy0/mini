#include "../skewHeap.cpp"
#include "./Union.cpp"

node* DeleteMax()
{
    node* p = root;
    root = Union(p->left, p->right);
    p->left = NULL;
    p->right = NULL;

    return p;
}