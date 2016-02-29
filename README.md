Parallel dominating set
==================

This is a project for classes:
Parallel programming - https://edux.fit.cvut.cz/courses/MI-PPR.2/
Parallel Computer Architectures - https://edux.fit.cvut.cz/courses/MI-PAP/
CUDA programming - https://edux.fit.cvut.cz/courses/MI-PRC/

Thanks to Adam Lehar for working on PPR.2 with me.


This algorithm is: Lokální (L-PBB-DFS-D):


Každý procesor si udržuje informaci o lokálně nejlepším řešení a pokud řešení s cenou rovnou dolní mezi neexistuje, bude prohledávat celý stavový prostor a po distribuovaném ukončení výpočtu pomocí algoritmu ADUV bude globálně nejlepší řešení ziskano paralelni redukci lokalnich reseni. Existuje-li reseni s cenou rovnou dolni mezi, procesor, který jej nalezne, ukončí výpočet vysláním zprávy typu jeden-všem.

