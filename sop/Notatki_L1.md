## FIFO (named pipe)
- first in - first out special file
- podobny do pipe, tyle że dostep do niego odbywa się poprzez filesystem 
- pipe i FIFO są właściwie identyczne, różnią się tylko sposobem tworzenia i otwierania
- może zostać otwarty przez wiele procesów do zapisywania i odczytu
- wszystkie dane są przekazywane poprzez to łączne, nie są zapisywane w filesystemie a jedynie nazwa FIFO jest tam przechowywana jako punkt referencyjny do łącza
- FIFO musi być otwarte na zapis odczyt po obu stronach aby dane zaczęły płynąć
- można otworzyć w `O_NONBLOCK` wtedy można zawsze otworzyć dla read-only i nie można nadal dla write-only gdy druga strona jeszcze się nie otworzyła
- na Linuxie można otwierać dla odczytu i zapisu w blokującym trybie i nie blokującym, POSIX ma to niezdefiniowane 
- `SIGPIPE` jest wysyłany gdy proces próbuje zapisać do FIFO który nie został jeszcze otwarty do odczytu po drugiej stronie
- #mkfifo
	- użyte do tworzenia FIFO
- **`open()`**
	- otwieranie FIFO
	- każdy proces może otworzyć FIFO
	- `O_RDONLY` flaga do otwarcia cześci odczytu
	- `O_WRONLY` flaga do otwarcia części zapisu


## pipe
- pipe ma część odczytującą i zapisującą
- dane zapisane do części zapisującej mogą zostać odczytane w części odczytującej
- pipe jest tworzony przy pomocy `pipe(2)` które zwraca dwa file deskryptory do części odczytującej i zapisującej

#### I/O dla pipe i FIFO
- blokowanie następuje gdy
	- proces próbuje odczytać pusty pipe do czasu gdy dane są dostępne
	- proces próbuje zapisać do pełnego pipe do czasu gdy wystarczająco danych zostało przeczytanych 
	- można wyłączyć używając flagi `O_NOBLOCK`
- jeżeli dostęp do części zapisu został zamknięty próba odczytu `read()` zwróci wtedy EOF (zwróci 0)
- jeżeli dostęp do części odczytu został zamknięty próba zapisu `write()` wywoła sygnał `SIGPIPE` , ponadto jeżeli proces wywołujący `write()` ignoruje `SIGPIPE` to write failuje i zwraca error `EPIPE` 

#### pipe capacity
- jeżeli pipe jest pełny to `write()` zablokuje się lub zfailuje w zależności od tego czy flaga `O_NONBLOCK` została ustawiona
- **`fcntl()`**
	- `F_GETPIPE_SZ`
		- zwraca pojemność pipe
	- `F_SETPIPE_SZ`
		- ustawia pojemność pipe

#### `PIPE_BUF`
- `write()` zapisujący mniej niż `PIPE_BUF` jest atomiczny 
- O_NONBLOCK disabled, n <= PIPE_BUF
	- All  n bytes are written atomically;
	- write(2) may block if there is not room for n bytes to be written immediately
- O_NONBLOCK enabled, n <= PIPE_BUF
	- If there is room to write n bytes to  the  pipe,  then  write(2) succeeds  immediately,  writing  all n bytes; 
	- otherwise write(2) fails, with errno set to EAGAIN.
- O_NONBLOCK disabled, n > PIPE_BUF
	- The write is nonatomic: the data given to write(2) may be interleaved  with write(2)s by other process; 
	- the write(2) blocks until n bytes have been written.
- O_NONBLOCK enabled, n > PIPE_BUF
	- If the pipe is full, then write(2) fails, with errno set to  EAGAIN. 
	- Otherwise,  from  1  to  n bytes may be written (i.e., a "partial write" may occur; 
	- the caller should  check  the  return value  from  write(2)  to see how many bytes were actually written), and these bytes may be interleaved with  writes  by  other processes.



# #functions
---
### `mkfifo()` 
#mkfifo 

```c
#include <sys/stat.h>

// tworzy plik FIFO
int mkfifo(const char *path, mode_t mode); 

char *path; // nazwa pliku FIFO

mode_t mode; // maska uprawnień

return EXIT_SUCCESS; // 0 
return EXIT_FAILURE; // -1, ustawia errno, nie tworzy FIFO

```

### `isalpha()`
#isalpha 

```c
#include <ctype.h>

// sprawdza czy jest literą
int isalpha(int c); 

int c // znak o wartości ASCII

// jeżeli jest alfabetyczny 
return non-zero;
// jeżeli nie jest alfabetyczny
return 0; 

```

### `pipe()`
#pipe

```c
#include <unistd.h>

int pipe(int fildes[2]);

// jest tablica dwoch file descriptorow do odczytu/zapisu
int filedes[2] 

// read
filedes[0]
// write
filedes[1]

// success
return 0;
// failure
// jeżeli otwarty jest jakis fd przed pipe()
return -1; // errno is set

```

# #libraries
---
### `<limits.h>`
#limits