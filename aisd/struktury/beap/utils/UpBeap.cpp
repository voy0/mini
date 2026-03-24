void UpBeap(int k)
{
    int x = A[k];
    int kchild = k;
    auto [i, j] = k2ij(k);

    while(i > 1)
    {
        // jeżeli jest na lewej krawędzi to idzie prawo
        if(j == 1)
        {
            k = k - i + 1;
        }
        // jezeli jest na prawej krawędzi to idzie lewo
        else if(j == i)
        {
            k = k - i;
            j--;
        }
        // jezeli ma dwoch rodziców
        else
        {
            k = k - i;
            j--;
            if(A[k+1] < A[k]) // który z rodziców jest mniejszy
            {
                k++;
                j++;
            }
        }

        // jezeli faktycznie wezeł dziecko ma mniejsza wart.
        if(A[kchild] > A[k])
        {
            
            A[kchild] = A[k]; // przepisanie rodzica w dół
            kchild = k;
            i--;
        }
        else
            break;     
    }
    A[kchild] = x; // tu umieszczony jest wstawiony node
}
