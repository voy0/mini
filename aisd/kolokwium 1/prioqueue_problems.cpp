// Napisz funkcję DownHeap dla kopca z elementem minimalnym w korzeniu oraz indeksowanego od 0
// (bez wartownika; A[0]-element w korzeniu). Podaj jej złożoność. 

void DownHeap(int i) // O(log n)
{
    int v = A[i];
    int k = 2*(i+1);

    while(k <= n)
    {
        if(A[k - 1] < A[k])
        {
            k -= 1;
        }

        if(v > A[k])
        {
            A[k/2] = A[k];
            k *= 2;
        }
        else
        {
            break;
        }
    }
    A[k] = v;
}
// Podaj postać kopca po utworzeniu
// metodą wykorzystującą UpHeap z ciągu wejściowego składającego się z liter: A S D I M N O P W.
// Jaka jest złożoność takiej metody budowania kopca?

// O(n log n)
// 1:A 8:S 2:D 3:I 4:M 5:N 6:O 7:P 9:W

// +A +S +D
//     A
//    / \
//   S   D

// +I +M +N +O
//     A
//    / \
//   I   D
//  / \ / \
// S  M N  O

// +P +W
//         A
//        / \
//       I   D
//      / \ / \
//     P  M N  O
//    / \
//   S   W

// Napisz Funkcję Search wyszukującą element z zadanym priorytetem. Jaka jest jej złożoność?
int Search(int v) // O(n)
{
    for(int i = 0; i < n; i++)
    {
        if(A[i] == v)
            return i;
    }
    return -1;
}


// Napisz funkcję znajdowania skrajnie prawego elementu w ostatnim pełnym wierszu dwukopca.
// Procedura powinna zwracać numer wiersza, numer elementu w wierszu oraz indeks elementu w
// tablicy

(int i, int j, int k) RightMostEl(int n)
{
    int i, j, k;
    (i, j) = k2ij(n);

    if(i == j)
        return (i, j, n);
    
    i--;
    j = i;
    k = ij2k(i, j);
    return(i, j, k);
}

// W której (których) z procedur dotyczących dwukopca (UpBeap, DownBeap, Min, Insert,
// Search, DeleteMin, Delete, Replace,....) może być zastosowana napisana procedura. Wyjaśnij, w
// którym miejscu.
//
// Funkcja wyszukiwania skrajnie prawego elementu w ostatnim pelnym wierszu dwukopca jest uzywana w:
// - Search, wyszukiwanie zaczyna sie od skrajnie prawego elementu w ostatnim pelnym wierszu
// - Delete, Replace, obie fukcje musza wyszukac element zeby go potem odpowiedno usunac lub zamienic


// Zaproponuj strukturę węzła kolejki dwumianowej.

struct node{
    int v;
    int h;
    node* next, *prev, *child;
}

// Jaka jest złożoność operacji usuwania
// elementu największego z kolejki dwumianowej (wyjaśnij)?. Porównaj ją ze złożonością tej
// operacji dla kopca skośnego. 
//
// Zlozonosc operacji usuwania elementu z najwiekszym priorytetem z kolejki dwumianowej jest rzedu O(log n)
// poniewaz przy najpierw należy wyszukac drzewo z maksymalnym elementem w kolejce,
// usunac korzen tego drzewa co zajmuje maksymalnie O(log n)
// oraz wykonac operacje Union rzedu O(log n) laczenia kolejki dwumianowej z dziecmi tego korzenia,
// ktore tworza niezalezna kolejke dwumianowa 
// 
// Dla kopca skosnego zlozonosc pojedynczej operacji DelMax jest rzedu O(n), poniewaz usuwamy korzen 
// i laczymy oba podrzewa operacja Union ktora ma zlozonosc O(n), jednak wykonanie operacji Union n razy
// daje czas wykonania algorytmu O(n log n) czyli koszt zamortyzowany O(log n)


// Zaproponuj strukturę węzła dla kopca Fibonacciego. Napisz funkcję dodawania do kopca - kopca
// Fibonnaciego składającego się z jednego elementu. Podaj jej złożoność. 

struct node{
    int val;
    int h;
    int mark;
    node* next, *prev, *child;
}*head;

void Insert(node* &head, node* t){ // O(1)
    if(!head){
        head = t;
        return;
    }
    node* last = head->prev;
    last->next = t;
    t->prev = last;
    head->prev = t;
    return;
}


// Zaproponuj odpowiednie struktury danych i napisz procedury dołączania do listy drzew
// dwumianowych oraz do kopca Fibbonaciego jednego elementu (drzewa o wysokości 0).

// struktura drzewa dwumianowego
struct node{
    int val;
    int h;
    node* next, *prev, *child;
}

void Insert(node* &head, node* t){
    if(!head){
        head = t;
        return;
    }

    node* p = head;
    if(t->h == head->h){
    while(p->h == t->h){
        t = MergeTrees(p, t);
        p = p->next;
    }}
    else{
        t->next = head;
        t->prev = head->prev;
        head->prev = t;
        head = t;
    }
}


// Zaproponuj strukturę węzła dla kopca Fibonacciego. Napisz funkcję dodawania do kopca (z poprawą)
// nieuporządkowanego drzewa dwumianowego (flaga mark, informująca ile brakuje poddrzew może
// wynosić 0,1 lub 2). Podaj jej złożoność. Narysuj dowolny kopiec Fibonacciego składający się z 7
// elementów.

void CorrectAndAddToEnd(node* &head, node* t){ // O(log n)
    if(t->mark == 0){
        AddToEnd(head, t);
        return;
    }
    if(t->mark == 1){
        if(t->child->prev->h == t->h - 2){
            t->mark = 0;
            t->h--;
        }
        AddtoEnd(head, t);
        return;
    }
    if(t->mark == 2){
        if(t->child->prev->h == t->h - 3){
            t->mark = 0;
            t->h -= 2;
            AddToEnd(head, t);
            return;
        }
        if(t->child->prev->h == t->h - 2){
            t->mark = 1;
            t->h--;
            AddToEnd(head, t);
            return;
        }
        
        node* p = t->child;
        while(p){
            node* child = Extract(p);
            AddToEnd(head, child);
        }
        t->h = 0;
        t->mark = 0;
        AddToEnd(head, t);
    }
}

// Zaproponuj odpowiednie struktury danych i napisz funkcję Max oraz DeleteMax dla kopców
// złączalnych implementowanych jako kopiec skośny, kolejka dwumianowa oraz 2-3 drzewo+.
// Dane są funkcje Union. Podaj złożoności napisanych procedur.

// kopiec skosny
node* Max(node* head){ // O(1)
    return head;
}
node* DeleteMax(node* &head){ // O(log n)
    node* max = head;

    node* left = head->left;
    node* right = head->right;
    head = Union(left, right);

    max->left = NULL;
    max->right = NULL;
    return max;
}

// kolejka dwumianowa
node* Max(node* head){ // O(log n)
    if(!head)
        return NULL;

    node* max = head;
    node* p = head->next;

    while(p){
        if(p->v > max->v)
            max = p;
        p = p->next;
    }

    return max;
}

node* DeleteMax(node* &head){ // O(log n)
    // znajdz najwiekszy element i zapamietaj go
    node* max = head;
    node* p = head->next;
    while(p){
        if(p->val > max->val)
            max = p;
        p = p->next;
    }

    // odetnij najwiekszy element
    if(max == head){
        head = head->next;
        head->prev = max->prev
        max->prev = max;
        max->next = NULL;
    }
    else{
        if(max->next == NULL)
            head->prev = max->prev;
        max->prev->next = max->next;
        max->prev = max;
        max->next = NULL
    }

    // odetnij dzieci
    node* child = max->child;
    max->child = NULL;

    // przylacz dzieci z powrotem do kolejki
    Union(head, child);

    // zwroc najwiekszy element
    return max;
}

// 2-3 drzewo+
int Max(node* head){ // O(1)
    return head->val;
}

node* DeleteMax(node* &head){ // O(log n)
    node* max = head;
    node* left = head->left;
    node* right = head->right
    node* mid = head->mid;
    max->left = max->right = max->mid = NULL;

    head = Union(left, right);
    head = Union(head, mid);

    return max;
}

// 300 linijek JD
