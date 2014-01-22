Copy .\HeuristiqueBis\SCPTimInd\Release\H2.exe .\TesteurExact\SCPTimInd.exe
cd .\TesteurExact
.\Release\EvalIP.exe >> cplex.log 2>>cplex_err.log
shutdown -s -t 300

