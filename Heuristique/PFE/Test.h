///Some functions for test purpose
///Author: SHANG Lei - shang.france@gmail.com
#ifndef TEST_H
#define TEST_H

#include <assert.h>
#include <stdlib.h>

#include <algorithm>
#include <iostream>
#include <iterator>

#include "Traitement.h"

#define VERBOSE true

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
	std::cout<<std::endl;
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


void AfficherListes(int indice)
{
	if(VERBOSE)
	{
		//Affichage des listes
		printf("\n *Indice: %d. Listes des taches:\n", indice);
		printf("\n ** Liste HDDRAMGPU: ");
		for (int i=0; i<Traitement.NbHDDRAMGPU; i++){
			printf("%d \t",Traitement.ListOfTasks1GPU[i].IndiceVM);
		}
		printf("\n ** Liste RAMHDDGPU: ");
		for (int i=0; i<Traitement.NbRAMHDDGPU; i++){
			printf("%d \t",Traitement.ListOfTasks2GPU[i].IndiceVM);
		}
		printf("\n ** Liste HDDRAMCPU: ");
		for (int i=0; i<Traitement.NbHDDRAMCPU; i++){
			printf("%d \t",Traitement.ListOfTasks1CPU[i].IndiceVM);
		}
		printf("\n ** Liste RAMHDDCPU: ");
		for (int i=0; i<Traitement.NbRAMHDDCPU; i++){
			printf("%d \t",Traitement.ListOfTasks2CPU[i].IndiceVM);
		}
		///Listes pr�
		printf("\n ** Liste HDDRAMGPUPr: ");
		for (int i=0; i<Traitement.NbHDDRAMGPUPr; i++){
			printf("%d \t",Traitement.ListOfTasks1GPUPr[i].IndiceVM);
		}
		printf("\n ** Liste RAMHDDGPUPr: ");
		for (int i=0; i<Traitement.NbRAMHDDGPUPr; i++){
			printf("%d \t",Traitement.ListOfTasks2GPUPr[i].IndiceVM);
		}
		printf("\n ** Liste HDDRAMCPUPr: ");
		for (int i=0; i<Traitement.NbHDDRAMCPUPr; i++){
			printf("%d \t",Traitement.ListOfTasks1CPUPr[i].IndiceVM);
		}
		printf("\n ** Liste RAMHDDCPUPr: ");
		for (int i=0; i<Traitement.NbRAMHDDCPUPr; i++){
			printf("%d \t",Traitement.ListOfTasks2CPUPr[i].IndiceVM);
		}





		printf("\n");
	}
}

void LoadOrdo()
{
	int n = 8, t = 12;
	short int data[8][12] = {
		{1,1,1,1,-1,-1,-1, 1, 1, -1, 1, 1},
		{0,-1,0,-1,0,0,0, 0, 0, 0, -1, 0},
		{0,-1,0,-1,0,-1,0, -1, 0, -1, 0, -1},
		{1,1,1,1,1,1,1,-1,1,1,1,1},
		{-1,-1,-1,-1,-1,-1,-1, -1, 1, 1, -1, -1},
		{1,1,1,1, 1, 1,1,1,-1, -1, 1, 1},
		{-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1},
		{-1,-1,-1,-1,-1,1,1, 1, -1, -1, -1, -1}
	};
	///solution cplex with only ressource consideration. Checked.136342
	short int data1[8][12] = { 
		{1,1,1,1,-1,-1,-1, 1, 1, -1, 1, 1},
		{0,-1,0,-1,0,0,0, 0, 0, 0, -1, 0},
		{-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1},
		{1,1,1,1,1,1,1,-1,1,1,1,1},
		{-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1},
		{1,1,1,1, 1, 1,1,1,-1, -1, 1, 1},
		{-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1},
		{-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1}
	};

	///solution cplex with ressource and rho consideration. Checked. 485327
	short int data2[8][12] = { 
		{1,1,1,1,-1,-1,-1, 1, 1, -1, 1, 1},
		{0,-1,0,-1,0,0,0, 0, 0, 0, -1, 0},
		{0,-1,0,-1,0,-1,0, -1, 0, -1, 0, -1},
		{1,1,1,1,1,1,1,-1,1,1,1,1},
		{-1,-1,-1,-1,-1,-1,-1, -1, 1, 1, -1, -1},
		{1,1,1,1, 1, 1,1,1,-1, -1, 1, 1},
		{-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1},
		{-1,-1,-1,-1,-1,1,1, 1, -1, -1, -1, -1}
	};

	///solution cplex with ressource and rho and beta consideration. Checked. 485635
	short int data3[8][12] = {  // == data2
		{1,1,1,1,-1,-1,-1, 1, 1, -1, 1, 1},
		{0,-1,0,-1,0,0,0, 0, 0, 0, -1, 0},
		{0,-1,0,-1,0,-1,0, -1, 0, -1, 0, -1},
		{1,1,1,1,1,1,1,-1,1,1,1,1},
		{-1,-1,-1,-1,-1,-1,-1, -1, 1, 1, -1, -1},
		{1,1,1,1, 1, 1,1,1,-1, -1, 1, 1},
		{-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1},
		{-1,-1,-1,-1,-1,1,1, 1, -1, -1, -1, -1}
	};
	for(int i =0; i<n; i++)
	{
		for(int j =0; j<t; j++)
		{
			if(data[i][j] != -1)
				Traitement.ListOfOrdo[j][i].affecter = 1;
			else Traitement.ListOfOrdo[j][i].affecter = 0;
			Traitement.ListOfOrdo[j][i].IndiceMachine = data2[i][j];
		}
	}
}


void AfficherListeOrdo()
{
	printf("\n �������������  Liste ordo ������������\n");
	for(int i=0;i<T();i++){
		for(int j=0;j<N();j++){
			if (u(j,i)==1 && (R(j)==0) && Traitement.ListOfOrdo[i][j].affecter!=1){
				printf("*");
			}
			printf("%d \t",Traitement.ListOfOrdo[i][j].IndiceMachine);
		}
		printf("\n");
	}
}

void Test()
{
	HDDRAM arr[10];
	std::for_each(arr, arr+10, InitPrio);
	PrintArrayPrio(arr, 10);
	SortListByPrio(arr, 10);
	TestOrderByPrio(arr, 10);
	PrintArrayPrio(arr,10);
}

#endif