::Same test as TestCase5. Except that we reuse srand(1)
time /t > time.log
Copy .\TimeIndexedFormulation\SCPTimInd\Release\SCPTimInd.exe .\TesteurExactH1H2\SCPTimInd.exe
Copy .\Heuristique\Release\PFE.exe .\TesteurExactH1H2\PFE.exe
Copy .\H2\SCPTimInd\Release\H2.exe .\TesteurExactH1H2\H2.exe
cd .\TesteurExactH1H2
.\Release\TesteurH1H2.exe > test.log 2>test.err
cd ..
time /t >> time.log
shutdown -s -t 60
