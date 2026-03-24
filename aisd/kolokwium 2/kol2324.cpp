// 1. RST 8 bitowe, funkcja odlaczania znalezionego elementu maksymalnego

struct node{
    int key;
    node* left, *right;
}

node* DelMax(node* pMax, node* pPrev){ // pes. O(log n)
    // czy pMax jest lisciem - tak usun - nie szukaj liscia
    node* pLeft = pMax->left;
    node* pRight = pMax->right;
    if(!pLeft && !pRight) // pmax jest lisciem
    {
        if(pPrev->left == pMax)
            pPrev->left = NULL;
        if(pPrev->right == pMax)
            pPrev->right = NULL;
        return pMax;
    }

    // pmax nie jest lisciem
    // szukaj liscia
    node* p = (pLeft) ? pLeft : pRight;
    node* pp = p;
    bool dir = false;
    while(p->left || p->right){
        pp = p;
        if(p->left)
            p = p->left;
        else if(p->right)
            p = p->right;
    }
    if(pp->left == p)
        pp->left = NULL;
    if(pp->right == p)
        pp->right = NULL;

    p->left = pMax->left;
    p->right = pMax->right;
    if(pPrev->left == pMax){    
        pPrev->left = p;
    }
    if(pPrev->right == pMax){
        pPrev->right = p;
    }
    pMax->left = NULL;
    pMax->right = NULL;
    return pMax;
}

// 2. Search, Insert, Delete dla splay

node* Search(int v, node* &root){
    Splay(v, root);
    return (root->key == v) ? root : NULL;
}

void Insert(int v, node* &root){
    node* pNew(v, NULL, NULL);
    
    if(!root){
        root = pNew;
    }
    
    Splay(v, root);
    
    if(root->key == v){
        return;
    }
    
    if(root->key < v){
        pNew->left = root;
        pNew->right = root->right
        root->right = NULL;
    }
    if(root->key > v){
        pNew->right = root;
        pNew->left = root->left;
        root->left = NULL;
    }
    
    root = pNew;
}

node* Delete(int v, node* &root){
    if(!root)
        return NULL;

    Splay(v, root);
    
    if(root->key != v)
        return root;
    
    node* p = root;
    if(!root->left)
        root = root->right
    else{
        Splay(v, root->left);
        
        root->left->right = root->right;
        root = root->left;
    }
    p->left = NULL;
    p->right = NULL;
    
    return p;
}

// 3. Insert dla haszowania otwartego podwojnego

int Insert(int v){
    int index = h1(v);
    int step = h2(v);

    for(int i = 0; i < m; i++){
        int curr = (index + i*step) % m;

        if(H[curr] == FREE){
            H[curr] = v;
            return curr;
        }
    }
    return -1;
}

// 5. sortowanie kubelkowe dla wartosci 0 - 9
// tablica list node* A[10][2]

node* BucketSort(node* &head){
    node* p = head;

    while(p){
        node* pnext = p->next;
        if(A[p->val][0] == NULL){
            A[p->val][0] = p; // head
        }
        if(A[p->val][1] != NULL){
            A[p->val][1]->next = p // wstawienie na koniec listy
        }
        A[p->val][1] = p; // nowy tail;
        p->next = NULL;

        p = pnext;
    }

    head = NULL;
    node* tail;
    for(int i = 0; i < 10; i++){
        if(!head && A[i][0]){
            head = A[i][0];
            tail = A[i][1];
            continue;
        }
        if(A[i][0]){
            tail->next = A[i][0];
            tail = A[i][1];
        }
    }
    return head;
}