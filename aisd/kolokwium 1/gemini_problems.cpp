// Kopce Skośne (Skew Heaps):

// Napisz nierekurencyjną funkcję Union (łączenia) dwóch kopców skośnych. W funkcji powinna znajdować się tylko jedna instrukcja powrotu (return). Podaj pesymistyczną i zamortyzowaną złożoność czasową tej operacji.

node* Union(node* p1, node* p2){

    node* p = (p1)? p1 : p2;
    if(p){
        if(p2 > p1){
            swap(p1, p2);
            p = p1;
        }
        node* child = p1->right;
        Union(child, p2);
        swap(p1->left, p1->right);
    }
    return p;
}

node* UnionNR(node* p1, node* p2){
    STACK s;
    s.push(p2);
    s.push(p1);
start:
    p1 = s.pop();
    p2 = s.pop();
    node* p = (p1)? p1 : p2;
    if(p){
        if(p2 > p1){
            swap(p1, p2);
            p = p1;
        }
        node* child = p1->right;
        s.push(child, p2);
        s.push(L2);
        goto start;
L1:
        swap(p1->left, p1->right);
    }
    while(!s.empty()){
        Label = s.pop();
        goto label;
    }
    return p;
}

// Kolejki Priorytetowe (Złożoność):

// Wyjaśnij różnicę między złożonością pesymistyczną a zamortyzowaną (amortyzowaną). Podaj przykład operacji na Kopcu Fibonacciego, dla której złożoność zamortyzowana jest znacznie lepsza niż pesymistyczna i krótko opisz, dzięki jakiemu mechanizmowi to osiągnięto.

// zlozonosc pesymistyczna, jest to zlozonosc czasowa wykonania algorytmu raz, w najgorszym mozliwym przypadku
// zlozonosc zamortyzowana, jest to pesymistyczna zlozonosc czasowa wykonania algorytmu ciagiem n razy, jeden po drugim, podzielona przez n pozwala na zobaczenie jak zachowuje sie strukutra danych po ciagu operacji
// zlozonosc srednia, opisuje czas dzialania algorytmu wykonanego wiele razy, dla losowych danych, nie musi byc ciagiem, (w jakim czasie zazwyczaj wykonuje sie algorytm)

// W kopcu fibonacciego dla operacji DeleteMax, zlozonosc pesymistyczna jest O(n), a zamortyzowana O(log n)
// dzieje sie tak poniewaz w najgorszym przypadku, Kopiec Fibonacciego jest po prostu lista dwukierunkowa i scalenie go po DeleteMax wiaze sie z przejsciem przez kazdy wezel
// gdy jednak operacja ta jest wykonywana w ciagu, kopiec sie konsoliduje, i wtedy przypomina bardziej kolejke dwumianowa



// Drzewa BST (Wyszukiwanie):

// Napisz funkcję, która w niepustym drzewie BST znajduje i zwraca węzeł, z którego ścieżka do najbliższego pustego węzła (Null/None) ma maksymalną długość (czyli jest to węzeł o najmniejszej wysokości). Podaj pesymistyczną złożoność czasową i pamięciową napisanej funkcji.


// Kopiec Fibonacciego / Kolejka Dwumianowa (Konwersja):

// Dany jest Kopiec Fibonacciego, w którym atrybut mark jest równy 1 tylko w korzeniach, a w pozostałych węzłach wynosi 0. Napisz funkcję, która przekonwertuje taki kopiec do Kolejki Dwumianowej. Wskaż, czy atrybut mark jest potrzebny w docelowej strukturze.


// W pewnym drzewie dwumianowym usunięto dla korzenia $k$ poddrzew (ale nie uaktualniono atrybutu mark, który zawiera $k$). Napisz funkcję, która uaktualni atrybut mark w korzeniu oraz utworzy i zwróci jednokierunkową listę wysokości brakujących poddrzew, uporządkowaną rosnąco.

node* MissingTrees(node* root){
    node* p = root->child;
    STACK s;
    
    int k = 0;
    int i = 0;
    for(i = 0; i < root->h; i++){
        if(p->h != i){
            k++;
            s.push(i);
        }
        p = p->next;
    }
    root->mark = k;
}

// Dwukopiec (Double-Ended Heap) – Implementacja i złożoność:

// Zdefiniuj struktury danych dla Dwukopca (np. w tablicy) – kolejki priorytetowej umożliwiającej efektywne znajdowanie i usuwanie zarówno elementu maksymalnego, jak i minimalnego. Napisz funkcję UpBeap(k) poprawiającą Dwukopiec do góry. Podaj złożoność pesymistyczną operacji Insert i DelMax.

// Insert O(sqrt(n))
// DelMax O(sqrt(n))



// Kopiec Parujący (Pairing Heap) – Podstawowe operacje:

// Kopiec parujący jest uproszczoną wersją Kopca Fibonacciego. Napisz dwie podstawowe funkcje kolejki priorytetowej implementowanej jako kopiec parujący (np. Insert(v) i DelMax()) oraz podaj ich pesymistyczną złożoność czasową.


