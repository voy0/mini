void DownBeap(int k, int n)
{
    auto [i, j] = k2ij(k);
    int x = A[k];
    int kparent = k;
    k = k + i;

    while (k <= n)
    {
        if (k < n) // potrzebne do sytuacji gdy jesteśmy na nodzie który ma tylko jednego childa 
        {
            if (A[k + 1] > A[k])
            {
                k++;
            }
        }
        if(A[kparent] < A[k])
        {
            A[kparent] = A[k];
            kparent = k;
            k = k + i;
            i++; 
        }
        else
            break;
    }
    A[kparent] = x;
}