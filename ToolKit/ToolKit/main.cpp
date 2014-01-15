#include <utility>
#include <iostream>
#include <fstream>
#include <vector>
#include "declaration.h"
#include "TinyLog.h"

int main(int argn, char** argvs)
{
	using namespace std;
	ExtractTimeElapsedGapFromLog("cplex.log","timegap.csv");
	
	return 0;
}