// Budowanie kopca
// 1. Od Góry n*Insert -> O(n log(n)) z UpHeap w Insert
// 2. Od Dołu Wstaw wszystkie po kolei, DownHeap, (n/2) * DownHeap -> szacowana O(n log n),  dokładnie to O(n)

// Dwukopiec (Beap)
// implementacja w tablicy
// h = sqrt(n)

int ij2k(int i, int j){}

int k2ij(int k){}

int Search(int v)
{
    int h = k2ij(n);
    int i, j, k;
    k = n;
    (i, j) = k2ij(k);
    if(i != j)
    {
        i--;
        j = i;
    }
    k = ij2k(i, j);

    while(A[k] != v && j > 0)
    {
        if(v > A[k]) // lewo gora
        {
            i--;
            j--;
            k = ij2k(i, j);
            continue;
        }
        if(n >= ij2k(i + 1, j)) // lewo dol
        {
            i++;
            k = ij2k(i, j);
            continue;
        }
        else // lewo
        {
            j--;
            k = ij2k(i, j);
        }
        
    }
    return j == 0 ? -1 : k;
}

// Kopiec lewostronny / skośny
// drzewo binarne z porządkiem kopcowym

struct node
{
    int key;
    // int npl; // null path length, dla lewostronnego
    node *left, *right;
}*root;

void Insert(int v) // O(Union)
{
    node* pNew = new node(v, NULL, NULL);
    root = Union(root, pNew);
}
node* DelMax()
{
    node* pmax = root;
    node* pl, *pr;
    pl = pmax->left;
    pr = pmax->right;
    root = Union(pl, pr);
    return pmax;
}