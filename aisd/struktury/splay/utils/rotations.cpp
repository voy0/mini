#include "../splay.cpp"

void RR(node *&p)
{
    node* x = p->right;
    p->right = x->left;
    x->left = p;

    p = x;
}

void LL(node *&p) // lewy staje się rodzicem prawego
{
    node* x = p->left;
    p->left = x->right;
    x->right = p;

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









void LZig(node *&p)
{
    LL(p);
}
void RZig(node *&p)
{
    RR(p);
}



void LZigZig(node *&p)
{
    LL(p);
    LL(p);
}
void RZigZig(node *&p)
{
    RR(p);
    RR(p);
}



void LZigZag(node *&p)
{
    // LR(p);
    RR(p->left);
    LL(p);
}
void RZigZag(node *&p)
{
    // RL(p);
    LL(p->right);
    RR(p);
}