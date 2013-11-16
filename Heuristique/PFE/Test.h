///Some functions for test purpose
///Author: SHANG Lei - shang.france@gmail.com

#include <assert.h>
#include <stdlib.h>

#include <algorithm>
#include <iostream>
#include <iterator>

#include "Traitement.h"

template <typename T>
void PrintArray(T* arr, int size)
{
	//copy(arr, arr+size, std::ostream_iterator<T>(std::cout, ", ");
}

template <typename T>
void PrintArrayPrio(T* arr, int size)
{
	for(int i=0; i<size; i++)std::cout<<arr[i].prio<<", ";
	//for_each(arr, arr+size, void print(T&ele){std::cout<<ele.prio<<", ";});
	std::cout<<endl;
}

void InitPrio(HDDRAM &hr){hr.prio = rand()%10;}

template <typename T>
void TestOrderByPrio(T* arr, int size, bool isDecreased = true)
{
	for(int i=0; i<size-1; i++)
	{
		if(isDecreased) 
			assert(arr[i].prio>=arr[i+1].prio);
		else 
			assert(arr[i].prio<=arr[i+1].prio);
	}
}

void Test()
{
	HDDRAM arr[10];
	std::for_each(arr, arr+10, InitPrio);
	PrintArrayPrio(arr, 10);
	SortByPrio(arr, 10);
	TestOrderByPrio(arr, 10);
	PrintArrayPrio(arr,10);
}
