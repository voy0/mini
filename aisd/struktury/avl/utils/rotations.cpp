#include "../avl.cpp"

void RR(node *&p)
{
    node* x = p->right;
    p->right = x->left;
    x->left = p;
    p->bl = (x->bl == -1) ? 0 : -1;
    x->bl = (x->bl == 0) ? 1 : 0;
    p = x;
}

void LL(node *&p)
{
    node* x = p->left;
    p->left = x->right;
    x->right = p;
    p->bl = (x->bl == 1) ? 0 : 1; // nw czy dobrze 
    x->bl = (x->bl == 0) ? -1 : 0;
    p = x;
}

void LR(node *&p)
{
    RR(p->left);
    LL(p);
}

void RL(node *&p)
{
    LL(p->right); // nw
    RR(p);
}