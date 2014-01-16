#pragma once
#ifndef UTILITY_H
#define UTILITY_H
#include <string>
///@brief return the cpu info string
std::string GetCpuInfo();


///@brief Heuristically determine a time limit for Cplex solver. According to an analyse, this value is 400s for 
int CalculateTimeLimit();
#endif