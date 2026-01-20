// kolokwium do
// quick sort wszystko co nie dotyczy zlozonosci pamieciowej
// heap sort
// sortowania elementarne

// RST
// gdy nie znajdzie elementu musi zwrocic parent oraz poziom i na ktorym sie zatrzymalismy
node* SearchRST(T v){
    node* p = root;
    int i = 3;
    while(p && p->key != v){
        if(bit(v, i) == 0)
            p = p->left;
        else
            p = p->right;
        i--;
    }
    return p;
}
// usuwanie w rst
// jezeli lisc to nic
// jezeli ma dzieci to zastepujemy lisciem


// Drzewa reTRIEval
// wezly sa dwoch typow
// wewnetrzene i liscie
node* Search(int v){
    node* p = root;
    int i = 3;
    while(p && (p->left || p->right)){
        if(bit(v, i) == 0)
            p = p->left;
        else
            p = p->right;
        i--;
    }
    return (p && p->key) ? p : NULL;
}

// Haszowanie
// funkcja haszujaca (klucz) = zwraca indeks w tablicy haszujacej
// gdy funkcja jest nieroznowartosciowa, musimy zaimplementowac rozwiazywanie konfliktow
// rozwiazania:
// 1. haszowanie lancuchowe, kazdy element w tablicy wskazuje na osobny slownik gdzie umieszczamy klucze, np. lista, złozonosci: pes. O(n) sred. O(1)
// 2. haszowanie otwarte, wstawiamy klucze w tablicy, gdy miejsce jest zajete, wstawiamy klucz w pierwsze nastepne puste miejsce
// 3. haszowanie otwarte podwójne gdzie sa dwie funkcje haszujace, gdy zajete jest miejsce uzywamy h2 ktora musi byc wzglednie pierwsza z m

// haszowanie otwarte
// zalozenie ze na pewno jest pusta komorka
// FREE, DEL to np. jakis minusowy int 
int Search(int v){
    int i = h(v);
    
    while(A[i] != v && i < A.size() && A[i] != FREE)
    {
        i = (i+1)%m;
    }

    return A[i] == v ? i : -1;
}

int Insert(int v){
    int i = h(v);
    while(A[i] != FREE){
        if(A[i] == v){
            return i;
        }
        i = (i+1)%m;
    }
    A[i] = v;
    return i;
}

// przy usuwaniu wstawiamy znacznik DEL w miejsce klucza
// modyfikujemy wtedy funkcje search i insert, zapamietujemy DEL przez ktore przechodzimy, dalej idziemy do pierwszego FREE tak aby zobaczyc czy moze nie ma tego klucza dalej, jezeli nie bylo mozemy wstawic w miejsce DEL

// struktury UNION-FIND
// operacje na zbiorach rozłącznych
// na początku n zbiorów 1-elemntowych
// reprezentacja tablicowa, gdzie indeks w tablicy jest elementem, a wartosc pod tym indeksem jest numerem zbioru do ktorego nalezy

int FIND(nr_elem){ // zwroc nr zbioru
    return R[nr_elem]; // O(1)
}

void UNION(int setIN1, int setIN2, int setOUT){ // O(n)
    for(int i = 0; i < R.size(); i++)
    {
        if(R[i] == setIN1 || R[i] == setIN2){
            R[i] = setOUT;
        }
    }
}