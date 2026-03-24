struct node{
    int key[3];
    node* next[4];
}*head;

node* Insert (int v){
    node* p = head;
    node* pp = p;
     while(p){
        if(p->key[0] == v)  return NULL;
        pp = p;
        if(p->key[0] < v)   p = p->next[0];
        else    p = p->next[1];
     }
     if(pp->key[0] > v){
        pp->key[1] = pp->key[0];
        pp->key[0] = v;
     }
     else   pp->key[1] = v;
}

node* Insert(node** p, int v){
    while(*p){
        if((*p)->key[0] < v){
            p = &((*p)->next[1]);
        }
        else if((*p)->key[1] > v){
            p = &((*p)->next[0]);
        }
        else{
            return nullptr;
        }
    }
    *p = new node(v);
}