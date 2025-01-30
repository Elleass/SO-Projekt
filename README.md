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
        Proces główny uruchamia procesy „queen” (składanie jaj) i „hatch” (wylęganie się pszczół z jaj).
        Tworzone są wątki dla pszczół-robotnic (każda w osobnym wątku).
        Wątek „pszczelarz” oczekuje na sygnały (SIGUSR1/SIGUSR2) i odpowiednio modyfikuje wartość capacity.
    Komunikacja i synchronizacja:
        Dane o liczbie osobników, jaj itd. przechowywane są w pamięci współdzielonej (segment SysV).
        Semafory binarne (POSIX i SysV) zarządzają dostępem do zasobów i kolejek.
        Ruch pszczół w ulu jest ograniczany semaforem ul_wejscie (liczbowe odzwierciedlenie capacity)


Wykonane testy:

1. Test podstawowy
Cel: Sprawdzenie, czy program uruchamia się poprawnie dla przykładowych danych wejściowych i wykonuje podstawowe zadania
Przykładowe parametry wejściowe:
    N=10N=10 (łącznie pszczół “robotnic”)
    P=4P=4 (początkowa maksymalna pojemność ula, P<N/2P<N/2)

