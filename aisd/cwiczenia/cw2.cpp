struct node
{
    int val;
    node* next;
}*head;

void InsertToPriorityList(int v)
{
    node* pNew = new node(v, NULL);
    if(!head)
    {
        head = pNew;
        return;
    }
    if(v < head->val)
    {
        pNew->next = head;
        head = pNew;
        return;
    }

    node* p = head;
    
    while(p->next && p->next->val < v)
    {
        p = p->next;
    }
    pNew->next = p->next;
    p->next = pNew;
}

void InsertToPriorityListV2(int v)
{
    node* pNew = new node(v);

    node** p = &head;
    while((*p)!=NULL &&(*p)->val < v)
    {
        p = &((*p)->next);
    }
    pNew->next = *p;
    *p = pNew;
}

node* Search(int v)
{
    node* p = head;
    while(p && p->val != v)
    {
        p = p->next;
    }
    return p;
}

void PrintRef(node* p)
{
    if(p)
        PrintRef(p->next);
    cout << p->val;
}

