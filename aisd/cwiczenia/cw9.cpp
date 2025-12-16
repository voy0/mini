// B-drzewa ( szeroko stosowane w bazach danych )
// - maja rzad m
// - rzad m mowi o maksymalnej ilosci elemntow na stronie (welze) (z wyjatkiem korzenia)
//  - minimum m
//  - maksimum 2*m
// - jesli k-elem to strona ma k+1 elementow ( z wyjatkiem korzenia )
// - wszystkie liscie sa na jednym poziomie
// - wszystkie elementy na stronie musza byc uporzadkowane
// - przy wstawianiu, gdy przepelni sie strona, bierzemy wartosc najblizsza mediany i wyrzucamy ja wyzej
// - przy usuwaniu, zamieniamy wartosc losowo z najmniejsza z prawego poddrzewa, lub najwieksza z lewego podrzewa
// - przy usuwaniu, jezeli brakuje elementow na stronie, robimy rownowazenie stron
// - przy usuwaniu, jezeli brakuje elementow na stronie oraz wszystkie strony sasiady maja rowno m elementow, robimy laczenie stron

struct node{
    int key[2*m]; // wartosci kluczy p->key[i]; pierwszy p->key[0]; ostatni: p->key[k-1] gdzie k = 2*m
    node* next[2*m + 1]; // wskazniki lewy: p->next[i]; prawy: p->next[i+1]
    int k; // ile elementow na stronie
}

// 2-3 PP (poziomo pionowe, binarne) <=rownowazne=> B-drzewa rzedu 1
