int Search(int v)
{
    int h = GetDwuKopiecWysokosc();
    int n = GetIloscWezlow();

    int i = h;
    int j = h;
    int k = ij2k(i, j);

    while (i >= 1 && j >= 1)
    {
        if (A[k] == v)
        {
            return k;
        }

        if (v > A[k]) // wartość wieksza
        {
            k = k - i; // do góry
            i--;
        }
        else
        {
            if (k + i <= n) // jeżeli mieści się
            {
                if (v < A[k]) // jezeli mniejsza
                {
                    k = k + i; // do dołu
                    i++;
                }
            }
            else
            {
                k--; // lewo
                j--;
            }
        }
    }
    return 0;
}