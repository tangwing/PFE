#include <Windows.h>

//Functions for time mesurement.
//Copied from: http://stackoverflow.com/questions/17432502/how-can-i-measure-cpu-time-and-wall-clock-time-on-both-linux-windows
//Part of code for Linux was removed.

#ifndef TIMER_H
#define TIMER_H

double GetWallTime(){
    LARGE_INTEGER time,freq;
    if (!QueryPerformanceFrequency(&freq)){
        //  Handle error
        return 0;
    }
    if (!QueryPerformanceCounter(&time)){
        //  Handle error
        return 0;
    }
    return (double)time.QuadPart / freq.QuadPart;
}
double GetCpuTime(){
    FILETIME a,b,c,d;
    if (GetProcessTimes(GetCurrentProcess(),&a,&b,&c,&d) != 0){
        //  Returns total user time.
        //  Can be tweaked to include kernel times as well.
        return
            (double)(d.dwLowDateTime |
            ((unsigned long long)d.dwHighDateTime << 32)) * 0.0000001;
    }else{
        //  Handle error
        return 0;
    }
}


void TestTimers()
{
	time_t temp1,temp2;

	//  Start Timers
	double wall0 = GetWallTime();
	double cpu0  = GetCpuTime();
	time(&temp1);
    //  Perform some computation.
    double sum = 0;
    for (long long i = 1; i < 1000000000; i++){
        sum += i;
    }

    //  Stop timers
    double wall1 = GetWallTime();
    double cpu1  = GetCpuTime();
	time(&temp2);

    cout << "Wall Time = " << wall1 - wall0 << endl;
    cout << "CPU Time  = " << cpu1  - cpu0  << endl;
    cout << "Old Time mesurement = " << difftime(temp2,temp1)  << endl;
}

#endif