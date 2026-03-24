// 1. MF

struct node{
    int key;
    node* next;
}*head;

node* Search(int key){
    node* p = head;
    node* pp = head;
    while(p && p->key != key){
        pp = p;
        p = p->next;
    }
    if(p && p->key == key)
    {
        if(p != head){
            pp->next = p->next;
            p->next = head;
            head = p;
        }
    }   
    return head;
}

// 2. RST

node* DelNode(node* pMin, node* pPrev){
    
    if(!pMin)
        return NULL;
    
    if(!pMin->left && !pMin->right){    // usuwany node jest lisciem - usun wskaznik z rodzica
        if(pPrev){
            if(pPrev->left == pMin)
                pPrev->left = nullptr;
            if(pPrev->right == pMin)
                pPrev->right = nullptr;
        }
        if(pMin == root){    // usuwany node jest rootem, nie ma dzieci - ustaw root
            root = nullptr;
        }
        return pMin;
    }

    // usuwany node ma dziecko(ci) - idz do liscia
    node* p = pMin; // lisc
    node* pp = pMin; // rodzic liscia
    while(p){
        if(p->left){
            pp = p;
            p = p->left;
        }
        else if(p->right){
            pp = p;
            p = p->right;
        }
        else{
            break;
        }
    }
    if(pp->left == p){
        pp->left = nullptr;
    }
    if(pp->right == p){
        pp->right = nullptr;
    } // 67
    // usuwany node jest rootem - ustaw root
    if(pMin == root){ 
        p = root;
    }
    else{
        if(pPrev->left == pMin){
            pPrev->left = p;
        }
        if(pPrev->right == pMin){
            pPrev->right = p;
        }
    }
    p->right = pMin->right;
    p->left = pMin->left;
    pMin->right = nullptr;
    pMin->left = nullptr;
    return pMin
}

node* DelMin(node* pMin, node* pPrev){
    if()
}

// 4. Insertion Sort od najwiekszych do najmniejszych, antystabilny, dla listy

node* InsertionSort(node* &head){
    node* curr = head;
    node* dummy = new node();
    dummy->next = head;
    node* s;
    
    while(curr && curr->next){
        if(curr->next->key >= curr->key){
            s = curr->next;
            curr->next = s->next;
            s->next = nullptr;
            
            
            node* p = dummy;
            while(p->next && s->key < p->next->key){
                p = p->next;
            }

            s->next = p->next;
            p->next = s;
        }
        curr = curr->next;
    }
    head = dummy->next;
    return head;
}