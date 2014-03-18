::Test preprocessing with cut2 of different levels
time /t > time.log
Testeur1CutLevel.exe > test.log 2>test.err
time /t >> time.log
sounder.exe /loop 5 alert.wav
shutdown -s -t 300

