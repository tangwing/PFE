::Perform 2 tests (H1 vs Exact and H2 vs Exact) and log all data then turn off computer
::Compared to TestCase4, this time we use clock() for better time mesurement, and produce a redesigned StatTable

::test 1
time /t > time.log
Copy .\TimeIndexedFormulation\SCPTimInd\Release\SCPTimInd.exe .\TesteurExactH1H2\SCPTimInd.exe
Copy .\Heuristique\Release\PFE.exe .\TesteurExactH1H2\PFE.exe
Copy .\H2\SCPTimInd\Release\H2.exe .\TesteurExactH1H2\H2.exe
cd .\TesteurExactH1H2
.\Release\TesteurH1H2.exe > test.log 2>test.err

shutdown -s -t 60