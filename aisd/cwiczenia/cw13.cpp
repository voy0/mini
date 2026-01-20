// union find
// adt gdzie funkcjami jest FIND i UNION
// tablicowa 
// FIND pes. O(1)
// UNION pes. O(n)
// numer elementu jest indeksem tablicy, pod tym indeksem w tablicy jest numer zbioru do ktorego naelzy
// drzewiasta
// w tablicy sa wskazniki do wezlow lisci, ktore tworza podstawe drzew grup, wezly maja tylko wskazniki do rodzica, w korzeniu jest zapisany numer zbioru
// mamy setArr tablica korzeni zbiorow


int FIND(int nr_elem){ // pes. O(n)
    node* p = ElemArr[nr_elem];



    while(p->father){
        p = p->father;
    }
    return p->setNo;

// PRACA DOMOWA
// poprawianie Struktury drzewa aby FIND nie był liniowy, a przynajmniej zamortyzowana zlozonosc byla lepsza
// Stosujemy KOMPRESJE SCIERZEK
// czyli przylaczanie wezlow przez ktore przechodzimy do roota

    while(p->father){
        node* r = p;
        p = p->father;
        r->father = root;
    }

}

void UNION(int S1, int S2, int SOut){ // pes. O(1)
    node* s1 = setArr[S1];
    node* s2 = setArr[S2];

    setArr[S1] = nullptr;
    setArr[S2] = nullptr;

    s2->father = s1;
    s1->setNo = SOut;

    setArr[SOut] = s1;
}


// Algorytmy sortowania dla list
// Selection Sort
// Insertion Sort

void InsertionSort(node* head){
    node* headS;

    while(head != NULL){
        // odlacz pFirst z head
        node* pFirst = head;
        head = head->next;
        pFirst->next = NULL;

        // znajdz w headS pPrev (za tym elem. wstawiamy pFirst)
        ...

        // wstaw pFirst ze pPrev
        if(pPrev != NULL){
            pFirst->next = pPrev->next;
            pPrev->next = pFirst;
        }
        else{
            pFirst->next = headS;
            headS = pFirst;
        }
    }
    head = headS;
}

void SelectionSort(node* head){
    node* headS = NULL;
    while(head != NULL){
        // znajdz pMax, ppMax w head

        // odlacz pMax

        // dodaj na pocz headS
    }
    head = headS;
}

// Sortowania bez porownan
// założenia:
// - dane do posortowania maja wartości z ustalonego skończonego przedzialu

// Sortowanie przez zliczanie - tylko dla tablic
// Sortowanie przez kubelkowe - tylko dla list

void SimpleCountSort(int A[n]){
    int count[m];

    for(int i = 0; i < n; i++){
        count[A[i]]++;
    }
    int index = 0;
    for(int i = 0; i < m; i++){
        if(count[i] > 0){
            for(int j = 0; j < count[i]; j++){
                A[index] = i;
                index++;
            }
        }
    }
}