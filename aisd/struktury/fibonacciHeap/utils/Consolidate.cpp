#include "../fibonacciHeap.cpp"
#include "./Extract.cpp"
#include "./MergeTree.cpp"
#include "./AddToLast.cpp"

void Consolidate(node **head)
{
    node *pArr[INT_MAX] = {};

    while (*head)
    {
        node *p = Extract(head);
        while (pArr[p->h])
        {
            p = MergeTree(p, pArr[p->h]);
            pArr[p->h - 1] = NULL;
        }
        pArr[p->h] = p;
    }
    for (int i = 0; i < INT_MAX; i++)
        if (pArr[i])
            AddToLast(head, pArr[i]);
}