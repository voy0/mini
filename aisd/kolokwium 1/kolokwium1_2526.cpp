// 1.
struct node
{
    int val;
    node* next;
}*A[9];
void Insert(int v) // O(1)
{
    A[v] = new node(v, A[v]);
}
node* DelMax() // O(1)

// najlepsza Fibonacci
// O(1)
// O(log n)

// 2.
// DelMax -> DownBeap
// O(sqrt(n))

// 3.
int GetNPL(node* root){
    start:
    if(!root)
        return -1;
    int nplL = GetNPL(root->left);
    L1:
    int nplR = GetNPL(root->right);
    L2:
    int npl = min(nplL, nplR) + 1;
    if(nplL < nplR)
        cout << root->val;
    return npl;
}

int GetNPLNR(node* root){
    STACK s;
start:
    if(!root)
        result = -1;
    else{
        s.push(root);
        s.push(L1);
        root = root->left;
        goto start;
    L1:
        int nplL = result;
        s.push(root);
        s.push(L2);
        root = root->right;
        goto start;
    L2:
        int nplR = result;
    int npl = min(nplL, nplR) + 1;
    if(nplL < nplR)
        cout << root->val;
    result = npl;
    }
    if(!s.empty()){
        root = s.pop();
        et = s.pop();
        goto et;
    }
}

// 4
struct ListNode{
    node* tree;
    ListNode* next;
}*head;
struct Node{
    int val;
    int h;
    node* child;
    node* next, *prev;
}

void Consolidate(){
    NodeList* p = head;
    while(p){
        node* t = p->tree;
        while(H[t->h]!= NULL){
            t = Merge(t, H[t->h]);
            H[t->h-1] = NULL;
        }
        H[t->h] = t;
        p = p->next;
    }
    for(...)
    // dodaj do listy z H[i]
}

// 5
void Insert(int v){
    node* pNew = new Node(v, NULL, NULL);
    if(!root){
        root = pNew
        return;
    }
    node* prev = root;
    node* p = root;
    while(p){
        prev = p;
        if(v < p->val)
            p = p->left;
        else
            p = p->right;
    }
    if(v < prev->val)
        p->left = pNew
    else
        prev->right = pNew;
}