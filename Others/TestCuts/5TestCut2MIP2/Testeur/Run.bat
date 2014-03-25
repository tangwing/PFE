::Test preprocessing+MIP with cut2
time /t > time.log
TesteurCuts.exe > test.log 2>test.err
time /t >> time.log
sounder.exe /loop 5 alert.wav
shutdown -s -t 300

