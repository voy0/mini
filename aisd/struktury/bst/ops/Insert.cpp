#include "../bst.cpp"
#include "./Search.cpp"

void Insert(int v)
{
    node* last;
    node* p = new node;
    p->key = v;
    if(Search(v, last))
        return;
    if(v < last->key)
    {
        last->left = p; 
    }
    else
    {
        last->right = p;
    }
}

void Insert(int v)
{
    node2 **p = Search2(v);
    if(*p) return;
    *p = new node2;
    (*p)->key = v;
    (*p)->next[0] = (*p)->next[1] = NULL;
}