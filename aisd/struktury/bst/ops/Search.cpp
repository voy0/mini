#include "../bst.cpp"

node* Search(int v, node *&last)
{
    node* p = root;
    while(p && p->key != v)
    {
        last = p;
        if(v < p->key)
        {
            p = p->right;
        }
        else
        {
            p = p->left;
        }
    }
    return p;
}

node2** Search2(int v) // dla struktury bst node* next [2]
{
    node2** p = &root2;
    while((*p) && (*p)->key != v)
    {
        p = &((*p)->next[v > (*p)->key]);
    }
    return p;
}