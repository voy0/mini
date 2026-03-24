//1

//a)
struct node{
    int v;
    node* next;
}
node* A[10];


void Insert(int v){
    node* pNew(v);
    
    node* pNext = A[v];
    A[v] = pNew;
    pNew->next = pNext;
}

node* DeleteMax(){
    for(int i = 9; i >= 0; i--){
        if(A[i]){
            node* max = A[i];
            A[i] = max->next;
            return max;
        }
    }
    return NULL;
}

// b)
// Insert: O(1)
// DeleteMax: O(1)
// dla najlepszej struktury do implementacji pq (kopca Fibonacciego):
// Insert: O(1)
// DeleteMax: O(log n)

// zlożonosci operacji pq struktury danych z zadania są znacznie lepsze od przykładowego kopca fibonacciego, powodem jest limitowana liczba wartości obslugiwanych przez taka strukture, do operacji deleteMax zawsze przejdziemy maksymalnie (w pesymistycznym przypadku) 10 wartosci, kopiec fibonacciego moze i ma wieksze zlozonosci ale w tym przypadku, dla nielimitowanych wartosci priorytetow jest lepszym rozwiązaniem

// 2
void UpBeap(int k){ // O(sqrt(n))
    int i, j;
    (i, j) = k2ij(k);

    int v = A[k];

    while(i >= 1){
        if(j == 1){
            i--;
        }
        else if(i == j){
            i--;
            j--;
        }
        else{
            i--;
            int m = ij2k(i, j);
            if(A[m - 1] < A[m])
                j--;
        }

        int l = ij2k(i, j);
        if(v > A[l]){
            A[k] = A[l];
            k = l;
        }
        else{
            break;
        }
    }
    A[k] = v;
}

// c)
void Insert(int v){
    A[n] = v;
    UpBeap(v);
    n++;
}

// 3

void FibToBinom(node* &head){
    node* p = head;

    while(p){
        if(p->mark == 1){
            if(p->child->prev->h == p->h - 2){
                p->h--;
                p->mark = 0;
            }
            else{
                node* children = p->child;
                while(children){
                    node* child = ExtractFirst(children);
                    AddToEnd(head, child);
                }
                p->h = 0;
                p->mark = 0;
                p->child = NULL;
                AddToEnd(head, p);
            }
        }
    }
    Consolidate(head);
}

void Consolidate(node* &head){
    node* pArr[100]; // w zupelnosci starczy, liczba elementow w drzewie wysokosci 100 to 2^100

    while(head){
        node* p = ExtractFirst(head);
        while(pArr[p->h]){
            p = MergeTrees(p, pArr[p->h]);
            pArr[p->h-1] = NULL;
        }
        pArr[p->h] = p;
    }

    for(int i = 0; i < pArr.size(); i++){
        if(pArr[i])
            AddToEnd(head, pArr[i]);
    }
}