This testcase contains 2 test: The first one is the old one. The second one we've done some modif in the cplex program: set the time limit dynamically and set the epgap value to 0.02.

Here are the meaning of each file:
TestCase3: The script.
*.log: All stdoutput of the testeur.
*.err: All stderr output.
Resultlog: The results of all instances.
ComparationMatrix: Statistics.
time.log: 3time points, begin of test1, end of test1 (begin of test2), end of test2. 

Problems:
+ In *.log, there miss some informations which should have been output by printf. (Resolved.those info are only in test2.log)
+ In the ResultLog of the test2, the time used by cplex for sc5-6 sc6-9 and sc6-17 has passed 400s, which is not coherent with our setting. (Resolved. Because they are cpu time)

Alors de toute façons, dans ce résultat de test, la partie qui concerne le temps n'est pas bonne. En plus, le test2 a été fait entre H2 et H1. Il faut en fait de faire entre H2 et CPlex je pense. Voir TestCase3.1