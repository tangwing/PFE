::Test preprocessing. See how many variables can we fix.
time /t > time.log
TesteurProc.exe > test.log 2>test.err
time /t >> time.log
sounder.exe /loop 5 alert.wav
shutdown -s -t 300