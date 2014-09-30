ppr-leharada-nesrotom
==================


- kompilace: gcc -Wall -pedantic -ggdb -o main main.c
- valgrind: valgrind --leak-check=full --show-reachable=yes ./main tests/0-graph.txt

co je hotovo:
- snad funguje nacitani grafu a bitove pole
- nacrt hledani reseni (neni otestovano, jsou potreba testy)

zadani:
https://edux.fit.cvut.cz/courses/MI-PPR.2/labs/zadani_semestralnich_praci#uloha_domi-dominujici_mnozina_grafu

paralelni algoritmus je L-PBB-DFS-D:
https://edux.fit.cvut.cz/courses/MI-PPR.2/labs/prohledavani_do_hloubky#paralelni_bb-dfs_s_prohledavanim_dde_stavoveho_prostoru_pbb-dfs-d

co je potreba:
- nacitani grafu (potreba vygenerovat grafa a nahrat ho)
- zasobnik
- kontrola hotoveho reseni
- prohledavani celeho prostoru
- testy (bez reseni, 1 reseni v centru grafu, nejaky testovaci graf)
