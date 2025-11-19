void Hanoi(int n, int src, int dst, int tmp)
{
    // start
    if(n > 0)
    {
        Hanoi(n-1, src, tmp, dst);
        // L1 etykieta do zrozumienia wersji nierekurencyjnej
        move(src->dst); 
        Hanoi(n-1, tmp, dst, src);
        // L2
    }
    return;
}

void NRHanoi()
{
    int n;
    int src = 1, tmp = 2, dst = 3;
    STACK S; // dla uproszczenia kodu pomijamy dokladna implementacje stosu

    start:
        if(n > 0)
        {
            S.Push(n, src, tmp, dst);
            n--;
            swap(tmp, dst);
            S.Push(L1); 
            goto start;

            L1: 
                move(src, dst);
                S.Push(n, src, tmp, dst);
                n--;
                swap(src, tmp);
                S.Push(L2);
                goto start;
            
            L2:

        }
        // Teoretycznie to to samo jezeli L2 bylo by tutaj, napisane w ten sposob zeby pokazac jak automatycznie mozna
        // przepisac rekurencje 
        if(!S.Empty())
        {
            (n, src, dst, tmp) = S.Pop();
            label = S.Pop();
            goto label;
        }
}

// Kopiec

void DownHeap(int i, int n) // FUNKCJA WLASNA
{
    int v = A[i];
    int k = 2*i;

    while(k < n)
    {
        if(k+1 <= n && A[k+1] > A[k])
        {
            k = k+1;
        }

        if(v < A[k])
        {
            A[i] = A[k];
            i = k;
            k = 2*i;
        }
        else
            break;
    }
    A[i] = v;
}

void DownHeap(int i, int hl) // FUNKCJA Z TABLICY
{
    while(true)
    {
        if(2*i > hl)
        {
            break;
        }
        int imax = 2*i;
        if(2*i + 1 <= hl && A[2*i] < A[2*i +1])
        {
            imax = 2*i + 1;
        }

        if(A[i] >= A[imax])
        {
            break;
        }
        swap(A[i], A[imax]);
        i = imax;
    }
}