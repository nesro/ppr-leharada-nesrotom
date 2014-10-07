ppr-leharada-nesrotom
==================

- kompilace: gcc -Wall -pedantic -ggdb -o main main.c -lm
- valgrind: valgrind --leak-check=full --show-reachable=yes ./main tests/0-graph.txt

zadani:
https://edux.fit.cvut.cz/courses/MI-PPR.2/labs/zadani_semestralnich_praci#uloha_domi-dominujici_mnozina_grafu

paralelni algoritmus je L-PBB-DFS-D:
https://edux.fit.cvut.cz/courses/MI-PPR.2/labs/prohledavani_do_hloubky#paralelni_bb-dfs_s_prohledavanim_dde_stavoveho_prostoru_pbb-dfs-d

TODO:
Odstranit chyby hlasene Valgrindem
