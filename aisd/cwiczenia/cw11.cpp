void Splay(node *&root, int v) {
  node *p = root;
  while (p->up != NULL) {
    node *pg = p->up; // parent
    if (pg->up != NULL)
      pg = pg->up;
    int i = WhereIs(p, pg);
    switch (i) {
    case 1:
      LZigZig(pg);
      break;
    case 2:
      LZigZag(pg);
      break;
      ...
    }
    root = p;
  }