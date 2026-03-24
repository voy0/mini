#include "../splay.cpp"

void Insert(int v)
{
    if(!root)
    {
        root = new node;
        root->key = v;
        root->left = root->right = NULL;
    }
    else
    {
        Splay(v, root); // w rootcie będzie ostatni element napotkany v'
        node* p = new node;
        p->key = v;
        if(root->key < v)
        {
            p->left = root;
            p->right = root->right;
            root->right = NULL;
        }
        else
        {
            p->right = root;
            p->left = root->left;
            root->left = NULL;
        }
        root = p;
    }
}