#include "../fibonacciHeap.cpp"

node* MergeTree(node* p1, node* p2)
{
    if(p2->key > p1->key)
    {
        std::swap(p1, p2);
    }

    if(p1->h == 0)
    {
        p1->child = p2;
    }
    else
    {
        node* t = p1->child;
        t->prev->next = p2;
        p2->prev = t->prev;
        t->prev = p2;
    }
    p1->h++;
    return p1;
}