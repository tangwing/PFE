For test1, no pb has been found in the result.  

For test2: about the value of TimeMin, we have H2 bigger than Cplex, which is not reasonable.
Problem from:
sc4:  8&9
sc5:  19

Après tester, ce pb vient de:
1. La mesure de temps n'est pas précise. J'ai lancé plusieurs fois H2 et Cplex sur l'instance Sc5-19, le temps se change entre 2s & 3s.
//2. Le traitement supplémentaire (par ex: calcul du paramètre TimLim dans H2) peut prendre un peu de //temps qui est considérable quand le durée total d'éxécution est petite.

Donc c'est plutôt normal.