#include "../bst.cpp"
#include "./Search.cpp"

void Delete(int v)
{
    node2 *pv, **q, **p = Search2(v);
    if (*p)
    {
        if ((*p)->next[0] && (*p)->next[1])
        {
            int b = int(2.0 * (double(rand()) / (double(RAND_MAX) + 1.0)));
            
            q = &((*p)->next[b]);
            while ((*q)->next[1 - b])
                q = &((*q)->next[1 - b]);
            pv = *q;
            (*p)->key = pv->key;
            *q = pv->next[b];
        }
        else
        {
            pv = *p;
            *p = (*p)->next[(*p)->next[0] == NULL];
        }
        delete pv;
    }
}