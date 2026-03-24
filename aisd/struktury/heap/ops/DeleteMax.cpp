int DeleteMax()
{
    int max = A[1];
    A[1] = A[hl];
    hl--;
    DownHeap(1);
    return max;
}