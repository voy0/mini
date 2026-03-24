#include "../splay.cpp"
void Delete(int v)
{
    if(!root)
        return;
    Splay(v, root);

    if(root->key != v) // jeżeli nie ma węzła
        return;
    
    node* p = root;
    if(!root->left) // gdy nie ma lewego poddrzewa, bierzemy prawe jako wynikowe i kończymy
        root = root->right;
    else
    {
        Splay(v, root->left);
        root->left->right = root->right;
        root = root->left;
    }
    delete p;
}