errno - typ enum
syscalle kiedy sie nie powiada zwracaja -1
read zwraca 0 -> nie ma juz danych, koniec pliku!!
uprawnienia sa zapisywane gdy plik powstaje
umask dla powloki, odejmowane z moda, bitowo uprawnien, zabezpieczenie

O_EXCL - argument do open, gwarantuje ze nie utworzy tego samego pliku jeszcze raz, POSIX gwarantuje ze operacja sprawdzenia istnienia i create pliku jest atomowa - synchronizacja 

read moze odczytac mniej zalezy ile system mu przydzieli

trawersowac po drzewie plikow i czytac pliki
co plusiki znacza w uprawnieniach w man 3p fopen man 3p fclose
scanf co zwraca 

fflush(stdout) do wypisania buffora