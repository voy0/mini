#include <bits/stdc++.h>

struct node
{
    int key;
    int rank; // wysokość
    node *next, *prev, *child;
}*head;