node* Search(int v)
{
    if(!root)
        return NULL;
    
    Splay(v, root);

    if(root->key == v)
        return root;
    else
        return NULL;
}