:: this time we do 2 tests (H1 vs Exact and H2 vs Exact) and log all data

::test 1
time /t > time.log
Copy .\TimeIndexedFormulation\SCPTimInd\Release\SCPTimInd.exe .\TesteurExactH1\SCPTimInd.exe
Copy .\Heuristique\Release\PFE.exe .\TesteurExactH1\PFE.exe
cd .\TesteurExactH1
.\Release\EvalIP.exe > test.log 2>test.err

::test 2
cd ..
time /t >> time.log
Copy .\H2\SCPTimInd\Release\H2.exe .\TesteurExactH2\H2.exe
cd .\TesteurExactH2
.\Release\EvalIP.exe > test2.log 2>test2.err
cd ..
time /t >> time.log

shutdown -s -t 60
