#include "../leftistHeap.cpp"
#include "./Union.cpp"

node* DeleteMax()
{
    node* p = root;
    root = Union(p->left, p->right);
    p->left = NULL;
    p->right = NULL;
    p->npl = 0;

    return p;
}