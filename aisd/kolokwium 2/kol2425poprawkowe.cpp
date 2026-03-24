// 3. Split dla kolejki konkatenowalnej zaimplementowanej jako tablica haszujaca
// (z haszowaniem łańcuchowym) w ten sposób, że elementami tablicy haszującej A
// są słowniki reprezentowane jako uporządkowane wg kluczy listy
// jednokierunkowe.

// - Zaproponuj struktury danych
// - Dana jest jedna globalna funkcja haszujaca h(v)
// - postaraj się w sposób mozliwie maksymalny zoptymalizować operację dzielenia
// - podaj złożoność pesymistyczną napisanej funkcji

// 1. przejdz po kazdym indeksie tablicy
// odwiedz kazdy element listy wiekszy od a i przepisz go do odpowiednej komorki
// nowej tablicy, (elementy w listach są malejąco)

struct node {
  int key;
  node *next;
};

void Split(node *H, int a, node *&TL, node *&TH) { // pes. O(m + n)
  int m = H.size();
  for (int i = 0; i < m; i++) {
    node *high = H[i];
    node *phigh = high;
    node *low = high;
    if (high) {
      if (high->key > a) {
        while (high && high->key > a) { // high
          phigh = high;
          high = high->next;
        }
        low = phigh->next;
        if (phigh && phigh->key > a) {
          TH[i] = H[i];
          phigh->next = NULL;
        }
      }
      if (low && low->key <= a) {
        TL[i] = low;
      }
    }
  }
}

node *MergeRecursive(node **A, int start, int end) {
  if (start == end) {
    return A[start];
  }

  int mid = start + (end - start) / 2;
  node *left = MergeRecursive(node * *A, int start, int mid);
  node *right = MergeRecursive(node * *A, int mid + 1, int end);

  return Merge(left, right);
}

node *MergeLists(node **A, int k) {
  return MergeRecursive(node * *A, 0, k - 1);
}