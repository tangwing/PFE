::Test preprocessing with cut1 & cut2 combination
time /t > time.log
TesteurCuts.exe > test.log 2>test.err
time /t >> time.log
sounder.exe /loop 5 alert.wav
