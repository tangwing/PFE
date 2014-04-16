::Analyse 1-cut with diff number generated
time /t > time.log
TesteurCut2SeuilAnalyse.exe > test.log 2>test.err
time /t >> time.log
::sounder.exe /loop 5 alert.wav
shutdown -s -t 300