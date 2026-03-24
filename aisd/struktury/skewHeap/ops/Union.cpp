#include "../skewHeap.cpp"

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

    // zamiana bezwarunkowa
    if(true)
    {
        node* q = p->left;
        p->left = p->right;
        p->right = q;
    }

    return p;
}