# Projekt Systemy Operacyjne
Politechnika Krakowska, Informatyka NST, III semestr Michał Kalinowski

Celem projektu jest odwzorowanie kluczowych aspektów zachwowań owadów w ulu, takich jak:
- Ograniczona pojemność ula
- Wielowątkowe i wieloprocesowe zarządzanie ruchem pszczół.
- Cykl składania i wylęgania jaj przez królową.
- Możliwość dynamicznej zmiany pojemności ula przez pszczelarza w trakcie działania programu
- Stopniowe „umieranie” pszczół po pewnej liczbie wizyt w ulu (odzwierciedlające naturalne ograniczenie ich żywotności).

Program został zrealizowany w języku C z wykorzystaniem mechanizmów wielowątkowości i wieloprocesowości (pthreads, fork), wspólnych zasobów (pamięć współdzielona) oraz semaforów (zarówno POSIX-owych, jak i Systemu V).

Opis ogólny kodu

Struktura:
- Proces główny uruchamia procesy „queen” (składanie jaj) i „hatch” (wylęganie się pszczół z jaj).
- Tworzone są wątki dla pszczół-robotnic (każda w osobnym wątku).
- Wątek „pszczelarz” oczekuje na sygnały (SIGUSR1/SIGUSR2) i odpowiednio modyfikuje wartość capacity.
 Komunikacja i synchronizacja:
- Dane o liczbie osobników, jaj itd. przechowywane są w pamięci współdzielonej (segment SysV).
- Semafory binarne (POSIX i SysV) zarządzają dostępem do zasobów i kolejek.
- Ruch pszczół w ulu jest ograniczany semaforem ul_wejscie (liczbowe odzwierciedlenie capacity)




## 1. Test podstawowy
Cel: Sprawdzenie, czy program uruchamia się poprawnie dla przykładowych danych wejściowych i wykonuje podstawowe zadania
Przykładowe parametry wejściowe:
-N=6 (łącznie pszczół “robotnic”)
-P=2(początkowa maksymalna pojemność ula, P<N/2P<N/2)
   ```
    Podaj całkowitą liczbę pszczół w roju (N) :  6
    Podaj maksymalną liczbę pszczół w ulu (P), gdzie P < N/2: 2
    Maksymalna liczba pszczół w ulu: 2
    inside queen_process (child PID: 125373)
    utworzono hatch_eggs (child PID: 125374)
    [Beekeeper] Oczekiwanie na sygnały:
      - SIGUSR1 (kill -USR1 125356) => powiększ ul do min(2*N, capacity*2)
      - SIGUSR2 (kill -USR2 125356) => zmniejsz ul do capacity/2
    Pszczoła 0 startuje w wątku.
    Pszczoła 0:  wchodzi do ula wejściem 1.
    Wolna przestrzeń: 1 (zajęte: 1). Bees: 1, Eggs: 0
    Pszczoła 1 startuje w wątku.
    Pszczoła 1:  wchodzi do ula wejściem 1.
    Ul jest pełny. | Bees: 2, Eggs: 0
    Pszczoła 2 startuje w wątku.
    Pszczoła 2:  czeka - ul pełny.
    szczoła 3:  czeka - ul pełny.
    Pszczoła 4:  czeka - ul pełny.
    Pszczoła 0:  wychodzi z ula wejściem 1.
    Wolna przestrzeń: 1 (zajęte: 1). Bees: 1, Eggs: 0
    Pszczoła 2:  wchodzi do ula wejściem 1.
    Ul jest pełny. | Bees: 2, Eggs: 0
    Pszczoła 3:  czeka - ul pełny.
    Pszczoła 5:  czeka - ul pełny.
    Pszczoła 1:  wychodzi z ula wejściem 1.
    Wolna przestrzeń: 1 (zajęte: 1). Bees: 1, Eggs: 0
    Pszczoła 4:  wchodzi do ula wejściem 1.
    Ul jest pełny. | Bees: 2, Eggs: 0
    Pszczoła 3:  czeka - ul pełny.
    Pszczoła 5:  czeka - ul pełny.
    Krolowa: Ul jest pelny, nie mozna zlozyc jaj.
    Pszczoła 3:  czeka - ul pełny.
```
Zgodnie z założeniem pszczoły czekają aż miejsce w ulu się zwolni, królowa w przypadku pełnego ula nie może złożyć jaj.
### 2. Test wysokich wartości (ponad maksymalną wartość wątków)
Cel: Sprawdzenie czy w przypadku przekroczenia limitu dopuszczalnych wątków program wyłączy się w sposób przewidywalny.

```
...
Failed to create bee thread (errno=11)
Failed to create bee thread (errno=11)
Failed to create bee thread (errno=11)
Repeated thread creation failures. Stopping program.
Pszczoła 4448:  wchodzi do ula wejściem 1.
Wolna przestrzeń: 3526 (zajęte: 474). Bees: 474, Eggs: 0
Pszczoła 4449:  czeka, oba wejścia są zajęte.
Pszczoła 4214 startuje w wątku.
Pszczoła 4214 kończy życie.
Pszczoła 4216 startuje w wątku.
Pszczoła 4216 kończy życie.
Pszczoła 4452 startuje w wątku.
Pszczoła 4452 kończy życie.
Pszczoła 4032 startuje w wątku.
Pszczoła 4032 kończy życie.
Pszczoła 4453:  czeka, oba wejścia są zajęte.
Pszczoła 4218 startuje w wątku.
...
Queen process terminated.
Terminating hatch process (PID: 131278)...
Hatch process terminated.
Canceling beekeeper thread...
Beekeeper thread terminated.
.
Pszczoła 4623 kończy życie.
Pszczoła 4041 startuje w wątku.
Pszczoła 4041 kończy życie.
Pszczoła 4655 kończy życie.
Bee threads cleaned up.
Destroying semaphores...
Semaphores destroyed.
Destroying egg queue...
Egg queue destroyed.
Cleanup completed. Exiting.
```
Po trzykrotnym wywołaniu się: Failed to create bee thread (errno=11) program wciąż tworzy zakolejkowane pszczoły które od razu umierają, dzieje się tak ponieważ program wpierw musi zamknąć procesy queen oraz hatch które mogły by tworzyć nowe pszczoły, następnie wykonuje się ciąg dalszy funkcji cleanup co pozwala zamknąć program w pełni kontrolowany sposób.

###  Test obsługi sygnałów SIGUSR1(zwiększenie pojemności) , SIGUSR2(zmniejszenie pojemności)
Parametry początkowe: N=10,P=4.
Cel: Sprawdzić, czy po wysłaniu sygnału SIGUSR1 do procesu głównego zwiększa się capacity, co umożliwia wejście większej liczbie pszczół oraz analogicznie po wysłaniu SIGUSR2 do procesu głównego zmniejsz się capacity o 50%.
```
Ul jest pełny. | Bees: 4, Eggs: 0
Pszczoła 2:  czeka - ul pełny.
Pszczoła 3:  czeka - ul pełny.
Zwiększanie pojemności ula z 4 do 8
Pszczoła 0:  wchodzi do ula wejściem 1.
Wolna przestrzeń: 3 (zajęte: 5). Bees: 5, Eggs: 0
Pszczoła 3:  wchodzi do ula wejściem 1.
Wolna przestrzeń: 2 (zajęte: 6). Bees: 6, Eggs: 0
Pszczoła 2:  wchodzi do ula wejściem 1.
Wolna przestrzeń: 1 (zajęte: 7). Bees: 7, Eggs: 0
Pszczoła 8:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 2 (zajęte: 6). Bees: 6, Eggs: 0
Pszczoła 9:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 3 (zajęte: 5). Bees: 5, Eggs: 0
Pszczoła 6:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 4 (zajęte: 4). Bees: 4, Eggs: 0
Pszczoła 0 kończy życie.
Pszczoła 7:  wchodzi do ula wejściem 1.
Wolna przestrzeń: 3 (zajęte: 5). Bees: 5, Eggs: 0
Pszczoła 5 kończy życie.
Pszczoła 1:  wchodzi do ula wejściem 1.
Wolna przestrzeń: 2 (zajęte: 6). Bees: 6, Eggs: 0
Krolowa: Zlozono jajo ID: 4 (wykluje sie za 5 s)
Pszczoła 2:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 2 (zajęte: 6). Bees: 5, Eggs: 1
```
Po wysłaniu komendy kill -USR1 <PID> w innym terminalu ul powiększył się dwukrotnie i od razu wypełnił się czekającymi pszczołami. 
```
Zmniejszanie pojemności ula z 8 do 4
Pszczoła 2:  wychodzi z ula wejściem 1.
Ul jest pełny. | Bees: 5, Eggs: 0
Krolowa: Zlozono jajo ID: 8 (wykluje sie za 5 s)
Krolowa: Ul jest pelny, nie mozna zlozyc jaj.
Pszczoła 4:  wychodzi z ula wejściem 1.
Ul jest pełny. | Bees: 4, Eggs: 1
Pszczoła 6:  wychodzi z ula wejściem 1.
Ul jest pełny. | Bees: 3, Eggs: 1
Wykluwanie: Jajko ID: 8 wyklulo sie w pszczole!
Pszczoła 8 startuje w wątku.
Pszczoła 8:  wchodzi do ula wejściem 1.
Ul jest pełny. | Bees: 4, Eggs: 0
Krolowa: Ul jest pelny, nie mozna zlozyc jaj.
Pszczoła 2:  wchodzi do ula wejściem 1.
Ul jest pełny. | Bees: 5, Eggs: 0
Pszczoła 5:  wychodzi z ula wejściem 1.
Ul jest pełny. | Bees: 4, Eggs: 0
Pszczoła 3:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 1 (zajęte: 3). Bees: 3, Eggs: 0
Pszczoła 4:  wchodzi do ula wejściem 1.
Ul jest pełny. | Bees: 4, Eggs: 0
Pszczoła 7:  wychodzi z ula wejściem 1.
...
Pszczoła 10:  wchodzi do ula wejściem 1.
Wolna przestrzeń: 1 (zajęte: 3). Bees: 3, Eggs: 0
Pszczoła 8:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 2 (zajęte: 2). Bees: 2, Eggs: 0
Pszczoła 9:  wchodzi do ula wejściem 1.
Wolna przestrzeń: 1 (zajęte: 3). Bees: 3, Eggs: 0
Pszczoła 7:  wchodzi do ula wejściem 1.
Ul jest pełny. | Bees: 4, Eggs: 0
```
Po wysłaniu komendy kill -USR2 <PID> w innym terminalu ul zmniejszył się dwukrotnie jednak od razu nie "wyrzucił" pszczół z ula oraz wpuścił ponad limit pszczoły czekające w kolejce przed wysłaniem komunikatu.
Napotkany problem: 
Program nie wyrzuca pszczół które już znajdowały się w ulu przed zmniejszeniem pojemności, ani nie blokuje tych, które były w trakcie wejścia. W efekcie przez jakiś czas może być w ulu więcej pszczół niż nowa, zredukowana wartość miejsc.

### Test zamykania programu Ctrl + C
```
Krolowa: Ul jest pelny, nie mozna zlozyc jaj.
Krolowa: Ul jest pelny, nie mozna zlozyc jaj.
^C
SIGINT received. Stopping...
Starting cleanup...
Terminating queen process (PID: 138998)...
Queen process terminated.
Terminating hatch process (PID: 138999)...
Hatch process terminated.
Canceling beekeeper thread...
Beekeeper thread terminated.
Canceling 10 bee threads...
Bee threads cleaned up.
Destroying semaphores...
Semaphores destroyed.
Destroying egg queue...
Egg queue destroyed.
Cleanup completed. Exiting.
```
Po wysłaniu sygnału SIGINT program zamyka wszystkie procesy, czyści utworzone wątki oraz kolejke. 

### Test krótkiej żywotności pszczół
pszczoła ginie po pierwszym wyjściu z ula. Cel:  sprawdzenie , czy occupant_count jest poprawnie dekrementowany i czy wątki „umierają” właściwie.

```
Pszczoła 9:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 1 (zajęte: 3). Bees: 3, Eggs: 0
Pszczoła 8:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 2 (zajęte: 2). Bees: 2, Eggs: 0
Pszczoła 5:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 3 (zajęte: 1). Bees: 1, Eggs: 0
Pszczoła 6:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 4 (zajęte: 0). Bees: 0, Eggs: 0
Pszczoła 0:  wchodzi do ula wejściem 1.
Wolna przestrzeń: 3 (zajęte: 1). Bees: 1, Eggs: 0
Pszczoła 4:  wchodzi do ula wejściem 1.
Wolna przestrzeń: 1 (zajęte: 3). Bees: 3, Eggs: 0
Pszczoła 7:  wchodzi do ula wejściem 2.
Wolna przestrzeń: 1 (zajęte: 3). Bees: 3, Eggs: 0
Pszczoła 0:  kończy życie.
Pszczoła 3:  kończy życie.
Pszczoła 1:  kończy życie.
Pszczoła 2:  kończy życie.
Pszczoła 0:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 2 (zajęte: 2). Bees: 2, Eggs: 0
Pszczoła 4:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 3 (zajęte: 1). Bees: 1, Eggs: 0
Pszczoła 7:  wychodzi z ula wejściem 1.
Wolna przestrzeń: 4 (zajęte: 0). Bees: 0, Eggs: 0
Pszczoła 9:  kończy życie.
Pszczoła 8:  kończy życie.
Pszczoła 5:  kończy życie.
Pszczoła 6:  kończy życie.
Krolowa: Zlozono jajo ID: 1 (wykluje sie za 5 s)
Wykluwanie: Jajko ID: 1 wyklulo sie w pszczole!
Pszczoła 1:  startuje w wątku.
Pszczoła 1:  wchodzi do ula wejściem 1.
Wolna przestrzeń: 3 (zajęte: 1). Bees: 1, Eggs: 0
Pszczoła 0:  kończy życie.
Pszczoła 4:  kończy życie.
Pszczoła 7:  kończy życie.
```
occupant_count spada do zera, gdy wszyscy wyjdą.
Pszczoły z krótkim życiem nie powracają do ula drugi raz.
Królowa i proces wylęgania nadal mogą generować nowe pszczoły (z nowymi ID).
Napotkany problem: Kolejność pojawiania się logów nie jest precyzyjna przez co zachowanie programu może stawać się nie czytelne. Zanim krolowa zlozy pierwsze jajo pszczoly umieraja przez co krolowa tworzy pszczole z ID martwej pszczoly. 


