#include <bits/stdc++.h>

#define m 4

struct node // jedna strona w b
{
    int key[2*m + 1];
    node* next[2*m+1];
    int k; // ilosc elementow na stronie
};