#include <utility>
#include <iostream>
#include <fstream>
#include <vector>
#include <Windows.h>
#include <intrin.h>
#include "Data.h"
using namespace std;

///@brief return the cpu info string
string GetCpuInfo()
{
    // Get extended ids.
    int CPUInfo[4] = {-1};
    __cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    // Get the information associated with each extended ID.
    char CPUBrandString[0x40] = { 0 };
    for( unsigned int i=0x80000000; i<=nExIds; ++i)
    {
        __cpuid(CPUInfo, i);

        // Interpret CPU brand string and cache information.
        if  (i == 0x80000002)
        {
            memcpy( CPUBrandString,
            CPUInfo,
            sizeof(CPUInfo));
        }
        else if( i == 0x80000003 )
        {
            memcpy( CPUBrandString + 16,
            CPUInfo,
            sizeof(CPUInfo));
        }
        else if( i == 0x80000004 )
        {
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        }
	}
    return string(CPUBrandString);
}


///@brief Heuristically determine a time limit for Cplex solver. According to an analyse, this value is 400s for 
int CalculateTimeLimit()
{
	int iTimeLimit = 400; //s
	//Get logical cpu num
	SYSTEM_INFO sysinfo;
	GetSystemInfo( &sysinfo );
	int iNumCPU = sysinfo.dwNumberOfProcessors;

	//Get cpu info
	string sCPUInfo = GetCpuInfo();
	//cout << "Cpu String: " << sCPUInfo<<endl;

	//Looking for cpu frequency
	size_t pos = sCPUInfo.find("GHz");
	if(pos == string::npos) 
		cerr<<"CalculateTimeLimit: Can't get cpu info! Set time limit to 400s\n";
	else{
		int iPosFre = pos;
		while(iPosFre >0 && sCPUInfo[iPosFre]!=' ' && sCPUInfo[iPosFre]!='\t') iPosFre --;
		double iFre = strtod(sCPUInfo.c_str() + iPosFre, NULL);
		//The default value 400s is for 2cores * 1.6GHz
		iTimeLimit = 400 * 3.2 / (iNumCPU * iFre);
	}
	if (iTimeLimit<200) iTimeLimit=200;
	return iTimeLimit;
}

