
int n;
int A[n];

void BuildUpHeap()
{
    int hl = 1;
    for(int i = 2; i <= n; i++)
    {
        Insert(A[i]);
    }
}