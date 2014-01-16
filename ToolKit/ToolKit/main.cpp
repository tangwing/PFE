//#include "TinyLog.h"
#include <gtest\gtest.h>

//extern TinyLog debug;
int main(int argn, char** argvs)
{
	testing::InitGoogleTest(&argn, argvs);
	RUN_ALL_TESTS();
	return 0;
}