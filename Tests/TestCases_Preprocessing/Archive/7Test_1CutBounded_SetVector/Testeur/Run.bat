::Test preprocessing. with 1-cut number limited by 0.17*(NM)²; with SetVector
time /t > time.log
Testeur1CutBoundedVector.exe > test.log 2>test.err
time /t >> time.log
::sounder.exe /loop 5 alert.wav
shutdown -s -t 300