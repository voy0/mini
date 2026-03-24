#include "../btree.cpp"

node* Search(node* root, int v)
{
    if(!root)
        return NULL;
    root->key[0] = -INFINITY;
    int i = root->k;

    while(root->key[i] > v)
        i--;
    if(root->key[i] == v)
        return root;
    return Search(root->next[i], v);
}