#include "../leftistHeap.cpp"

node* Union(node* p1, node* p2)
{
    node* p;
    if(!p1)
        return p2;
    if(!p2)
        return p1;

    // kótry węzeł jest większy do tego węzła dołączamy
    if(p1->key > p2->key)
    {
        p = p1;
        p->right = Union(p1->right, p2);
    }
    else
    {
        p = p2;
        p->right = Union(p2->right, p1);
    }

    // zamiana gdy nie ma lewego węzłą lub npl lewe jest mniejsze od prawego
    if(!p->left || p->left->npl < p->right->npl)
    {
        node* q = p->left;
        p->left = p->right;
        p->right = q;
    }

    // update npl
    if(!p->right)
        p->npl = 0;
    else
        p->npl = p->right->npl + 1;
    return p;
}