struct elem
{
    int val;
    elem* next;
}*head = NULL;

void PrintAll()
{
    elem* p = head;

    while(p)
    {
        cout << p->val;
        p = p->next;
    }
}

void InsertFront(int v)
{
    head = new elem(v, head);
    // elem* pNew = new elem;
    // pNew->val = v;

    // pNew->next = head;
    // head = pNew;
}

void InsertEnd(int v)
{

    elem* pNew = new elem(v);

    if(!head)
    {
        head = pNew;
    }

    elem* p = head;

    while(p->next)
    {
        p = p->next;
    }

    p->next = pNew;
}

struct elem
{
    int val;
    elem* next;
}*head, *tail;

void InsertEnd2(int v)
{
    elem* pNew = new elem(v);

    if(!head || !tail)
    {
        head = tail = pNew;
        return;
    }

    tail->next = pNew;
    tail = pNew;
}

elem* DelFirst()
{
    if(!head)
        return NULL;
    elem* p = head;
    head = head->next;
    p->next = NULL;
    return p;
}

elem* DelLast()
{
    if(!head)
        return NULL;

    elem* p = head;
    elem* pp = head;
    while(p->next)
    {
        pp = p;
        p = p->next;
    }

    pp->next = NULL;

    return p;
}

elem* DelMax()
{
    if(!head)
        return NULL;

    elem* p = head->next;
    elem* pp = head;
    elem* pMax = head;
    elem* ppMax = head;

    while(p)
    {
        if(p->val > pMax->val)
        {
            ppMax = pp;
            pMax = p;
        }
            
        pp = p;
        p = p->next;
    }
    if(pMax == head)
    {
        head = head->next;
    }
    else
    {
        ppMax->next = pMax->next;
    }
    pMax->next = NULL;
    return pMax;
}