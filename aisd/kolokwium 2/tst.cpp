






node* DelMax(node* pMax, node* pPrev) {
    if (!pMax) return NULL;

    node* pLeft = pMax->left;
    node* pRight = pMax->right;
    node* pReplacement = NULL; // To będzie nasz "zastępca" (liść)

    // 1. Jeśli pMax NIE jest liściem - musimy znaleźć liść do zastąpienia
    if (pLeft || pRight) {
        node* pp = pMax;          // Rodzic liścia
        node* p = (pLeft) ? pLeft : pRight; // Potencjalny liść

        // Szukanie liścia w dół drzewa
        while (p->left || p->right) {
            pp = p;
            if (p->left) p = p->left;
            else p = p->right;
        }

        // Odłączamy liść (p) od jego rodzica (pp)
        if (pp->left == p) pp->left = NULL;
        else pp->right = NULL;

        // Podpinamy dzieci pMax do naszego liścia-zastępcy
        // Sprawdzamy, czy potomek nie jest samym zastępcą (unikamy cyklu!)
        if (pLeft != p) p->left = pLeft;
        if (pRight != p) p->right = pRight;
        
        pReplacement = p;
    }

    // 2. Podpinamy pPrev (rodzica pMax) do pReplacement (liścia lub NULL)
    if (pPrev) {
        if (pPrev->left == pMax) pPrev->left = pReplacement;
        else pPrev->right = pReplacement;
    }

    // 3. Czyścimy pMax i zwracamy
    pMax->left = pMax->right = NULL;
    return pMax;
}