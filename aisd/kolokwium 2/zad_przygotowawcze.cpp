// Słowniki
// 1.
// Podać definicję „zrównoważenia” dla drzew AVL i drzew C-C. Jakie są pesymistyczne i
// średnie złożoności (wraz z współczynnikami) dla operacji wstawiania w tych drzewach? W
// jakich przypadkach należy stosować któreś z tych drzew?
// Narysuj i zaimplementuj rotację występującą przy zstępującym wstawianiu do drzew C-C.
// Rotacja ta występuje po „wykryciu następującej sytuacji:

// Zrównoważenie w drzewach AVL polega na utrzymaniu różnicy wysokości lewego i prawego poddrzewa każdego węzła w zbiorze {-1, 0, 1}
// Zrównoważenie w drzewach CC mówi że dla każdej ścierzki od korzenia do dowolonego liścia, ścierzka przechodzi przez tyle samo węzłów czarnych

// Wysokosc pes.            śr.  
// AVL      1.44 log n      log n + 0.25
// CC       2 log n         1.002 log n

// BST      n               1.39 log n
// Splay    n               3 log n

// Drzewa AVL należy stosować tam gdzie czesto jest wykonywane są zmiany w drzewie, ponieważ prawie przy każdym wstawianiu i usuwaniu należy wykonywać rotacje
// Drzewa CC natomiast dobrze się sprawdzają tam gdzie zmiany w drzewa są częste, ponieważ w wielu przypadkach ograniczają się jedynie do zmiany flag

