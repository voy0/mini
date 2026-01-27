// sortowanie przez zliczanie dla tablic ktory jest STABILNY

void CountSort(){               // O(n + m)  = O(n) liniowa, wiemy z jakiego przedzialu sa liczby w A, dlatego możemy użyć CountSort
    int count[m] = {0};
    for(int i = 0; i < m; i++){ // O(n)
        count[A[i]]++;
    }

    for(int i = 1; i < m; i++){ // sumy prefiksowe zeby wiedziec gdzie koncza sie 0, 1, 2 ...
        count[i] += count[i-1]; // O(m)
    }

    int B[n+1];
    for(int i = n; i >= 1; i--){ // przechodzenie od konca zeby bylo stabilnie i wpisywanie do nowej B
        B[count[A[i]]] = A[i];  // O(n)
        count[A[i]]--;
    }
    A<-B;                       // O(n)
}

// sortowanie kubełkowe dla list ktory jest STABILNY

void BucketSort(node*& head){ // O(n), dana jest lista jednokierunkowa, zdefiniuj struktury danych, trzeba dodac taila zeby zlozonosc byla lepsza
    node* head[m] = {NULL}
    node* tail[m] = {NULL}
    while(head){
        node* p = head;
        head = head->next;
        p->next = NULL;

        // dodawanie node na koniec listy
    }
    for(int i = 0; i < m, i++){
        // dodac kubelek na koniec listy
    }
}

// sortowanie pozycyjne
// sortowanie leksykografaiczne - RadixSort dla liter

// - sortowanie pozycyjne list, mozna wybrac dowolny algorytm sortowania aby tylko byl stabilny, wyjasnimy na kubelkowym
//      - metoda od przodu (od najbardziej znaczacych pozycji)
//          wstawiamy w kubelki wedlug pierwszej pozycji, i sortujemy rekurencyjnie kubelki po kazdej kolejnej pozycji
//      - metoda od tylu (od najmniej znaczacych pozycji) LEPSZA
//          sortujemy kubelkowo zaczynajac od ostatnich pozycji, sortujac kolejno wybierajac kolejne pozycje od tylu

// - sortowanie pozycyjne tablic
//      - metoda od tylu jest nadal lepsza uzywamy CountSort od ostatnich pozycji
// sortowanie posortuje binarne liczby poprawnie tylko jezeli sa jeszcze ujemne i dodatnie trzeba bedzie zwrocic uwage na ta pierwsza pozycje
void RadixSort(){

}

// Algorytm sortowania Hoare
// wybieramy pivota

node* Hoare(node* head, int k){ // szukanie elementu na pozycji k, do szukania np. mediany
    // pes. O(n^2) (istnieje wersja z magicznymi piątkami gdzie pesymistyczna zlozonosc liniowa)
    // sr. O(n)
    if(!head->next) return head;
    int v = head->val;

    node* headM, *headR, *headW;    // listy o elementach: Mniejszych, Wiekszych, Równych; od pivota
    int cM, cR, cW;                 // ile elementow jest na danej liscie

    while(head){
        node* p = head;             // odlaczamy element
        head = head->next;
        p->next = NULL;

        if(p->key < v){
            p->next = headM;
            headM = p;
            cM++;
        }
        if(p->key == v){
            p->next = headR;
            headR = p;
            cR++;
        }
        if(p->key > v){
            p->next = headW;
            headW = p;
            cW++;
        }
    }

    if(k <= cM)
        return Hoare(headM, k);
    if(cM < k <= cM + cR)
        return headR;
    if(cM + cR < k)
        reutnr Hoare(headW, k - (cM + cR))
}

// funkcja partition zwraca numer indeksu
// tablice haszujące są słownikami, nie można wstawiać powtarzających się elementów