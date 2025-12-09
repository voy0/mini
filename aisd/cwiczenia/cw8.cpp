    // BST
    (node*, node*) Search(int v){ // pes. O(n) sr. O(1.39 log n)
        node* pp, *p;
        pp = NULL;
        p = head;

        while( p != NULL && p->key != v){
            pp = p;
            if(v > p->key){
                p = p->right;
            }
            else{
                p = p->left;
            }
        }
        return (p, pp);
    }

    node* Insert(int v){
        node* p, *pp;

        (p, pp) = Search(v);
        if(p)
            return p;
        
        node* pNew = new node(v, NULL, NULL);

        if(!pp){
            root = pNew;
            return pNew;
        }

        if(v > pp->key)
            pp->right = pNew;
        else
            pp->left = pNew;

        return pNew;
    }

    node** Search2(int v){
        node** p = &root;

        while(*p != NULL && (*p)->key != v){
            if((*p)->key > v){
                p = &(*p)->left;
            }
            else{
                p = &(*p)->right;
            }
        }
        return p;
    }

    node* Insert2(int v){
        node** p = Search2(v);

        if(*p){
            return *p;
        }
        *p = new node(v, NULL, NULL);
        return *p;
    }
    node* Insert2(int v){
        node** p = Search2(v);

        if(!*p){
            *p = new node(v, NULL, NULL);
        }
        
        return *p;
    }

    // AVL
    struct node{
        int key;
        int bl; // {-1, 0, 1}
        node* left, *right;
    }

    void RL(node* &root){ // O(1)
        node* x = root;
        node* z = root->right;
        node* y = root->right->left;

        node* b = y->left;
        node* c = y->right;

        y->left = x;
        y->right = z;

        x->right = b;
        z->left = c;

        y->bl = 0;
        x->bl = 0;
        z->bl = 0;

        root = y;
    }

