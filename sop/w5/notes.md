lepiej blokowac sigpipa i obslugiwac epipe

jak trzymac tyle file descriptorow?
tworzymy najpierw wszystkie m i n pipow a potem latwo mozemy sie do niej dostac

to tworzy problemy
```
for(1..n)
    pipe()
    fork()
```

nalezy tworzyc tablice
```
arr = malloc((n + m)*2*sizeof(int))

for(int i = 0; i < n+m; i+=2){
    pipe(&arr[i]);
}

//ale najlepiej osobne tablice na read / write

fd_r
fd_w

int fd[2];
pipe(fd);
fd_r[i] = fd[0];
fd_r[i] = fd[1];
}
```



nie powinno sie tworzyc 
`int tab[n]`
bo slaba zlozonosc, uzywac malloca
chyba ze mamy zdefiniowana maksymalny rozmiar danych, jakis maly np20, mozna wtedy zdefiniowac `#define N_MAX`
i zrobic `int tab[N_MAX]`

