For test1, no pb has been found in the result.  

For test2: for the value of TimeMin, we have H2 bigger than Cplex, which is not reasonable.
Problem made from:
sc4:  8&9
sc5:  19
Après tester, ce pb vient de:
1. Le mesure de temps n'est pas précis. On lance plusieurs fois prog avec sc5, le temps peut changer entre 2 & 3.
2. Le traitement supplémentaire (calcul du paramètre) peut prendre un peu de temps qui est considérable quand le temps d'éxécution est courte.

Donc c'est plutôt normal.