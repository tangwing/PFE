For test1, no pb has been found in the result.  

For test2: about the value of TimeMin, we have H2 bigger than Cplex, which is not reasonable.
Problem from:
sc4:  8&9
sc5:  19

Apr�s tester, ce pb vient de:
1. La mesure de temps n'est pas pr�cise. J'ai lanc� plusieurs fois H2 et Cplex sur l'instance Sc5-19, le temps se change entre 2s & 3s.
//2. Le traitement suppl�mentaire (par ex: calcul du param�tre TimLim dans H2) peut prendre un peu de //temps qui est consid�rable quand le dur�e total d'�x�cution est petite.

Donc c'est plut�t normal.