#include "../fibonacciHeap.cpp"

node* Extract(node *root)
{
    if(root == NULL)
        return NULL;
    node* p = root;
    root = p->next;
    if(root)
    {
        root->prev = p->prev;
    }
    p->prev = p;
    p->next = NULL;
    return p;
}