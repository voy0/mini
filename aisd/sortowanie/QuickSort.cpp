int Partition(int* A, int l, int r)
{
    int pivot = A[r]; // pivot

    int i = l-1;
    
    for(int j = l; j <= r-1; j++)
    {
        if(A[j] < pivot)
        {
            i++;
            std::swap(A[i], A[j]);
        }
    }

    std::swap(A[i+1], A[r])
    return i+1;
}

void QuickSort(int* A, int l, int r)
{
    if(l < r)
    {
        int pivotIndex = Partition(A, l, r);
        QuickSort(A, l, pivotIndex - 1);
        QuickSort(A, pivotIndex + 1, r);
    }
}

void QuickSort(int* A, int l, int r) // pesymistyczna zlozonosc pamieciowa O(log n)
{
    if(l < r)
    {
        int pivotIndex = Partition(A, l, r);
        if(pivotIndex - l < r - pivotIndex)
        {
            QuickSort(A, l, pivotIndex - 1);
            l = pivotIndex + 1;
        }
        else
        {
            QuickSort(A, pivotIndex + 1, r);
            r = pivotIndex + 1;
        }
    }
}

void QuickSortNR() // pesymistyczna zlozonosc pamieciowa O(n)
{
    STACK S;
    S.Push2(1, n);
    while(!S.IsEmpty())
    {
        int l = S.Pop();
        int r = S.Pop();
        int pivotIndex = Partition(l, r);
        if(pivotIndex-1 > l)
            S.Push2(l, pivotIndex-1);

        if(pivotIndex+1 < r)
            S.Push2(pivotIndex+1, r);
    }
}

void QuickSortNR() // pesymistyczna zlozonosc pamieciowa O(log n)
{
    STACK S;
    S.Push2(1, n);
    while(!S.IsEmpty())
    {
        int l = S.Pop();
        int r = S.Pop();
        
        while(r > l)
        {
            int pivotIndex = Partition(l, r);
            if(pivotIndex - l > r - pivotIndex)
            {
                S.Push2(l, pivotIndex - 1);
                l = pivotIndex +1;
            }
            else
            {
                S.Push2(pivotIndex + 1, r);
                r = pivotIndex - 1;
            }
        }
    }
}

