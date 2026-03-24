#include "../binominalQueue.cpp"

void Insert(int v)
{
    node* p = new node;
    p->key = v;
    p->next = NULL;
    p->prev = p;
    p->child = NULL;
    p->rank = 0;
    head = Union(head, p);
}