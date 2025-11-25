// struktura drzewa dwumianowego do kolejek dwumianowych
struct node
{
    int val;
    int h;
    node* next, *prev, *child;
}*root;

// struckutra drzewa dwumianowego do kopca fibonacciego
struct node
{
    int val;
    int h;
    int mark; // atrybut mark przyjmuje 0, 1, 2, i mówi o tym ile brakuje poddrzew dla wezla, jak sa 2 nie nadaje sie do dodania do kopca
    node* next, *prev, *child;
}*root;


// odłączenie pierwszego drzewa z kolejki dwumianowej, lub kopca fibonacciego
node* Extract(node* &head)
{
    if(!head)   return NULL;

    node* p = head;
    head = head->next;

    if(!head->next)
    {
        head = NULL;
        return p;
    }

    head->prev = p->prev;
    p->next = NULL;
    p->prev = p;

    return p;
}

// z tablicy
node* Extract(node* &head)
{
    if(!head)   return NULL;

    node* p = head;
    node* last = head->prev;
    head = head->next;

    if(!head)
    {
        head->prev = last;
    }
    p->next = NULL;
    p->prev = p;

    return p;
}

// zakladamy ze dodajemy drzewo wysokosci ktorej nie ma w kolejce dwumianowej
void AddToEnd(node*& head, node* tree)
{
    if(!head)
    {
        head = tree;
        return;
    }
    node* last = head->prev;
    last->next = tree;
    tree->prev = last;
    head->prev = tree;
}


// mark = 1
void CorrectAndAddToEnd(node*& head, node* tree)
{
    if(tree->mark == 0)
    {
        AddToEnd(head, tree);
        return;
    }
    // jesli brakuje ostatniego to drzewo wysokosci h z mark = 1 jest rownowazne drzewu wysokosci h-1 z mark = 0
    if(tree->mark == 1)
    {
        if(tree->child->prev->h ==  tree->h - 2)
        {
            tree->mark = 0;
            tree->h--;
        }
        AddToEnd(head, tree);
        return;
    }

    if(tree->mark == 2)
    {
        // brakuje dwóch ostatnich drzew (h-1, h-2) czyli drzewo ma wysokosc h - 2 i mark = 0
        if(tree->child->prev->h == tree->h - 3)
        {
            tree->mark = 0;
            tree->h = 2;
            AddToEnd(head, tree);
            return;
        }
        // brakuje ostatniego drzewa i w srodku, czyli drzewo ma wysokosc h - 1 i mark = 1
        // odcinamy po kolei dzieci i dodajemy do kolejki
        node* p = tree->child;
        while(p){
            node* t = Extract(p);
            AddToEnd(head, t);
        }
        // po odcieciu dzieci pozostaje nam jeden wezel (drzewo o wysokosci 0), ktore tez dodajemy
        tree->mark = 0;
        tree->h = 0;
        tree->child = NULL;
        AddToEnd(head, tree);
    }
}

// kolokwium do kolejek priorytetowych plus drzewa bst, moze byc ich modyfikacja