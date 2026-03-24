#include "../binominalQueue.cpp"

node* DeleteMax()
{
    if(!head)
        return NULL;
    node *p = head;
    node *pMax = head;

    // wyszukanie elementu maksymalnego w kolejce
    while(p)
    {
        if(p->key > pMax->key)
            pMax = p;
        p = p->next;
    }

    // usunięcie i zmiana powiązania na listach
    node* t = pMax->child;
    if(pMax->next) // jeżeli jest następnik
    {
        pMax->next->prev = pMax->prev; // to połącz następnika z poprzednim 
    }
    else // jeżeli jest ostatnim elementem na liscie
    {
        head->prev = pMax->prev; // to head musi łączyć się z poprzednim pMax
    }
    if(pMax != head) // jeżeli pMax nie jest głową
    {
        pMax->prev->next = pMax->next; // to połącz poprzednika z nastepnikiem
    }
    else // jeżeli jest głową
    {
        head = head->next; // to głową jest następnik
    }
    head = Union(head, t);
    return pMax;
}