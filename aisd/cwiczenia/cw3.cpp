struct elem
{
    int val;
    int next;
}A[MAX], int head;

void PrintAll()
{
    int n = head;
    while(n != -1)
    {
        cout << A[n].val;
        n = A[n].next;
    }
}

void Insaert(int v)
{
    int i = GetFreeIndex();
    A[i].val = v;
    A[i].next = head;
    head = i;
}

// drzewa
struct node
{
    int val;
    node *left, *right;
}*root;

// PostOrder == do zapisywania rownan, drzewa z operacjami w wezłach, a w lisciach sa liczby, odwrotna notacja polska
void PrintAll(node* p)
{
    if(!p)
        return;
    PrintAll(p->left);
    PrintAll(p->right);
    cout << p->val;
}
// InOrder == w rdrzewie bst wypisze posortowane elementy
void PrintAll(node* p)
{
    if(!p)
        return;
    PrintAll(p->left);
    cout << p->val;
    PrintAll(p->right);
}
// PreOrder == DFS
void PrintAll(node* p)
{
    if(!p)
        return;
    cout << p->val;
    PrintAll(p->left);
    PrintAll(p->right);
}
// Odwrotny do PostOrder, taki PreOrder "lustrzany"
void NRPrintAll()
{
    STACK s;
    s.Push(head);
    while(s.IsEmpty())
    {
        node* p = s.Pop();
        cout << p->val;
        if(p->left)
            s.Push(p->left);
        if(p->right)
            s.Push(p->right);
    }
    
}
// LevelOrder == BFS
void NRPrintAll()
{
    QUEUE q;
    q.Put(head);
    while(q.IsEmpty())
    {
        node* p = q.Get();
        cout << p->val;
        if(p->left)
            q.Put(p->left);
        if(p->right)
            q.Put(p->right);
    }    
}

// HANOI
main()
{
    int n = 16;
    Hanoi(n, 1, 3, 2)
}
void Hanoi(int n, int src, int dst, int tmp)
{
    if(n == 0)
        return;
    Hanoi(n-1, src, tmp, dst);
    move(src, dst);
    Hanoi(n-1, tmp, dst, src);
}