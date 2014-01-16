:: this time we do 2 tests (with and without gap/time limit) and log all data

::test 1
time /t > time.log
Copy .\TimeIndexedFormulation\SCPTimInd\Release\SCPTimInd.exe .\Testeur\SCPTimInd.exe
Copy .\Heuristique\Release\PFE.exe .\Testeur\PFE.exe
cd .\Testeur
.\Release\EvalIP.exe > test.log 2>test.err

::test 2
cd ..
time /t >> time.log
Copy .\HeuristiqueBis\SCPTimInd\Release\H2.exe .\Testeur\SCPTimInd.exe
cd .\Testeur
.\Release\EvalIP.exe > test2.log 2>test2.err
cd ..
time /t >> time.log

::beep to let me know :)
for /l %%x in (1, 1, 100) do (
   echo %%x
   timeout 1
   python -c "print '\7'"
)

