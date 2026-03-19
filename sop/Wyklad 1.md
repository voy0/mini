pawel.sobotka@pw.edu.pl
sop.mini.pw.edu.pl/pl/sop1/
u: sop
p:ops
VSCode, CLion
warto mieć linuxa
powtorzyc C
można przyjść z gotowymi funkcjami na laboratoria

System operacyjny
- pośrednik użytkownik sprzęt
- alokator zasobów
- program kontrolujący wykonywanie się innych programów
- jądro - kernel, najbardziej uprzywilejowany kod siedzący w ramie
- oprogramowanie

OS zarządza rdzeniami procesora - mocą obliczniową
zarządza także pamięcią

bez załadowania kodu programu do pamięci nie da się go wykonać - alokacja pamięci

Multiprograming dopuszczenie jednoczesnego wykonywania jednego zadania na wielu rdzeniach
- komplikacja
- potrzeba schedulera do wykorzystywania rdzeni
- potrzeba managera pamięci
- potrzeba interfejsu w formie system calli za pomocą ktorego programy mogą coś wykonać

Tryby działania
 - Batch mode - kolejkowanie zadań w szybkich komputerach ?
 - online system - przęłączanie pomiędzy zadaniami gdy zadania czekają np. na wejście, przy przełączaniu działają context switche, musi występować proces wywłaszczania zasobów procesora od programu - preemption, zwykłe kompy
 - real time system, daje gwarancje latencji odpowiedzi, czasu odpowiedzi, potrzebne w systemach bezpieczeństwa
CPU operation cycle
1. fetch
2. decode
3. execute
Jak będzie pętelka nieskończona to problem
potrzebny wtedy jest mechanizm przerwania, interrupt controler, hardwarowy, programowalny, zapobiega gotowaniu procesora

podczas przerwania, pointer do instrukcji odkładany jest na stos żeby wiedzieć gdzie wrócić po przerwaniu, potem wykonywane są instrukcje po przerwaniu z użyciem wektora przerwań, jest to tablica adresów funkcji co zrobić po przerwaniu, ładując adres komórki do adres pointera

za pomocą syscalli user taski mogą się wykonać np. syscall to read()

Instrukcje uprzywilejowane do ochrony
używamy Mode Bit
- 0 kernel mode uprzywilejowany
- 1 user mode - nieuprzywilejowany
gdy instrukcja nie jest uprzywilejowana na poziomie 2. decode program jest przerywany

strace - syscall trace, nardzędzie które uruchamia program i listuje każde wywołanie systemowe