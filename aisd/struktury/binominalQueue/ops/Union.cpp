#include "../binominalQueue.cpp"
#include "../utils/MergeTree.cpp"

node* Union(node* p1, node* p2)
{
    node *t1, *t2, *t3, *p3;
    // i tak zwróci NULL jeżeli oba są NULL
    if(!p1) return p2;
    if(!p2) return p1;

    if(p1->rank > p2->rank) // chcemy w p1 miec mniejsze drzewo
    {
        std::swap(p1, p2);
    }

    if(p1->rank < p2->rank) // jest to właściwie po to aby wejść w ifa gdy drzewa są róznej wysokości
    {
        t1 = Extract(p1); // wyjmij z kolejki p1
        p3 = Union(p1, p2); // i połącz z p2 jako jedno drzewo
        // teraz możemy odnosić się do p1 jako t1
        t1->prev = p3->prev;
        p1->next = p3;
        p3->prev = t1;
        return t1;
    }

    t1 = Extract(p1);
    t2 = Extract(p2);
    t3 = MergeTree(t1, t2);
    p3 = Union(p1, p2);
    return Union(p3, t3);
}