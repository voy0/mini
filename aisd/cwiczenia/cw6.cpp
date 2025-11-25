// kopiec lewostronny
struct node
{
    int key;
    int npl; 
    node *left, *right;
}*root;

// Drzewa dwumianowe z nich buduje się:
// - kolejki dwumianowe
// - kopce fibonacciego (z atrybutem mark)

// Drzewo dwumianowe:
// - liczba elementow n = 2^h
// - wysokosc h = log(n)
struct node
{
    int val;
    int h;
    node* child, *next, *prev;
}

void PrintChildrenValue(node* root)
{
    node* p = root->child;
    while(p)
    {
        cout << val << endl;
        p = p->next;
    }
    return;
}

void PrintAll(node* root)
{
    if(!root)
        return;
    cout << root->val << endl;
    PrintAll(root->next);
    PrintAll(root->child);
}

// z tablicy 
// void PrintAll(node* root)
// {
//     if(root == NULL)
//         return;
//     cout << root->val;
//     node* p = root->child;
//     while(p!= NULL)
//     {
//         PrintAll(p);
//         p = p->next;
//     }
// }

void PrintLastChild(node* root)
{
    cout << root->chlid->prev->val;
}

// DRZEWA DWUMIANOWE MOZNA TYLKO LACZYC ZE SOBA GDY SA TAKIEJ SAMEJ WYSOKOSCI
node* Merge(node* t1, node* t2)
{

    if(t1->h != t2->h)
        return NULL;
    if(t1->val < t2->val)
        swap(t1, t2);
    if(t1->chlid)
    {
        node* p = t1->child->prev;
        p->next = t2;
        t2->prev = p;
        t1->child->prev = t2;
    }
    else
    {
        t1->child = t2;
    }
    t1->h++;
    return t1;
}

// drzewa dwumianowe  w kopcu fibonaciego
struct node
{
    int val;
    int h;
    int mark;
    node* child, *next, *prev;
}

// Ktorego drzewa nie ma
int MissingH(node* root)
{
    //root->mark == 1;
    node* p = root->child;
    int i = 0;
    while(p && p->h == i)
    {
        p = p->next;
        i++;
    }
    return i;
}

// Czy wszystkie drzewa sa
bool IsMarkInRootCorrect(node* root)
{
    node* p = root->child;
    int i = 0;
    while(p)
    {
        i++;
        p = p->next;
    }
    // if(root->mark == 1 && i == root->h - 1)
    //     return true;
    // else if(root->mark == 0 && i == root->h)
    //     return true;
    // else
    //     return false;
    return (i + root->mark == root->h);
}

// Wypisz wysokosc drzewa ktorego nie ma
void PrintMissingH(node* root)
{
    bool H[root->h] = {false};
    node* p = root->child;
    while(p != NULL)
    {
        H[p->h] = true;
        p = p->next;
    }
    for(int i = 0; i < root->h; i++)
    {
        if(!H[i])
            cout << i << endl;
    }
}

