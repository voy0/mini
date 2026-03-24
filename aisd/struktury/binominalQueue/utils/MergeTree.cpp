#include "../binominalQueue.cpp"

node* MergeTree(node* p1, node* p2) // łaczenie drzew o tej samej wysokości!!
{
    if(p2->key > p1->key)
    {
        node* tmp = p2;
        p2 = p1;
        p1 = tmp;
    }
    if(p1->rank == 0) // jeżeli drzewa są o wysokości 0 (1 węzeł)
        p1->child = p2;
    else
    {
        p2->prev = p1->child->prev;
        p2->prev->next = p2;
        p1->child->prev = p2;
    }
    p1->rank++;
    return p1;
}