For test1, no pb has been found in the result.  

For test2: for the value of TimeMin, we have H2 bigger than Cplex, which is not reasonable.
Problem made from:
sc4:  8&9
sc5:  19
Apr�s tester, ce pb vient de:
1. Le mesure de temps n'est pas pr�cis. On lance plusieurs fois prog avec sc5, le temps peut changer entre 2 & 3.
2. Le traitement suppl�mentaire (calcul du param�tre) peut prendre un peu de temps qui est consid�rable quand le temps d'�x�cution est courte.

Donc c'est plut�t normal.