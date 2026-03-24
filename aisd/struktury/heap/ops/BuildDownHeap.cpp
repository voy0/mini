
int n;
int A[n];

void BuildDownHeap()
{
    int hl = n;
    // zaczynamy od węzła wewnętrzego
    // (ostatni który ma jeszcze dzieci, jego indeks to i = n/2)
    for(int i = n/2; i >= 1; i--)
    {
        DownHeap(i, hl);
    }
}