::Analyse 1-cut with diff number generated. This time contraintes are sorted by sum(lhs)/rhs
time /t > time.log
TesteurCut2SeuilAnalyse2.exe > test.log 2>test.err
time /t >> time.log
::sounder.exe /loop 5 alert.wav
shutdown -s -t 300