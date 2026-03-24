struct node{
    int val;
    int h, mark;
    node* next, *prev, *child;
}*head;

node* FibToBinom(node* head)
{
    node* p = head;

    //sprawdz wszystkie marki, popraw wysokosci jezeli trzeba, lub dodaj dzieci na koniec
    while(p){
        if(p->mark == 1){
            if(p->child->prev->h == p->h - 2){
                p->mark = 0;
                p->h -= 1;
            }
            else{
                node* ch = p->child;
                while(ch){
                    ch = ExtractFirst(p->child);
                    AddToEnd(head, ch);
                }
                p->mark = 0;
                p->h = 0;
                AddToEnd(head, p);
            }
        }
        p = p->next;
    }

    // przejdz po kolejce i zkonsoliduj
    node* pArr[INT_MAX];
    while(head){
        node* p = ExtractFirst();
        while(pArr[p->h])
        {
            p = MergeTrees(pArr[p->h], p);
            pArr[p->h-1] = NULL;
        }
        pArr[p->h] = p;
    }

    for(int i = 0; i < pArr.size(); i++){
        if(pArr[i])
            AddToEnd(head, pArr[i]);
    }
}
