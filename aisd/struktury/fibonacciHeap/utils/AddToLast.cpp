#include "../fibonacciHeap.cpp"

void AddToLast(node* p1, node* p2)
{
    node* p = p1->prev;
    p->next = p2;
    p1->prev = p2->prev;
    p2->prev = p;
    
}