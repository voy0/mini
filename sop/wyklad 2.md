obowiazki OS
- zdefiniowac system plikow
- zapewnic interfejs do syscalli

aktualny model plikow pozwala na wiele nazw wskazujacych na ten sam plik, drzewo acykliczny

POSIX - filesystem
- plik to obiekt do ktorego mozna zapisywac odczytywac albo oba
- dostep do pliku jest sesyjny, sesje trzeba otworzyc i zamknac f = open(file) close(f)
- wystepuja inode wskazujace na foldery, pliki
- inode nie zawiera nazwy, nazwa pliku to wpis w katalogu ktory wskazuje na numer inoda, ktory jest unikalny
- jak liczba linkow do inoda spadnie do zera dane sa usuwane
man 7 inode - do doczytania

na 9 najmlodszych bitach sa uprawnienia w systemie usemkowym 3 grupy po 3
potem 3 bity na sticky bits?
kolejne 4 na typ pliku

ctr+d do grzecznego zakonczenia programu

