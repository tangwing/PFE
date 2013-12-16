///Some functions for test purpose
///Author: SHANG Lei - shang.france@gmail.com
#ifndef TEST_H
#define TEST_H
///#include "JLib\ConsoleCore.h"
#include <Windows.h>
#include <WinNT.h>

#include <assert.h>
#include <stdlib.h>

#include <algorithm>
#include <iostream>
#include <iterator>
#include <iomanip>

#include "Traitement.h"

#define VERBOSE true
HANDLE consolehwnd = GetStdHandle(STD_OUTPUT_HANDLE);
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

void AfficherIntervalle()
{
	if(VERBOSE)
	{
		printf("Nombre d'interval : %d\n",Traitement.NbInterval);
		for(int i=0;i<Traitement.NbInterval;i++){
			printf("intervalle %d : [%d,%d] \n",i,Traitement.ListOfIntervalles[i].BorneInf,Traitement.ListOfIntervalles[i].BorneSup);
		}
	}
}
void AfficherCaracMachine()
{
	if(VERBOSE)
	{
		using namespace std;
		int dataWidth = 4;
		int unitWidth = dataWidth * 4;
		int lineWidth = (unitWidth+2) * M();
		int lineWWithId = lineWidth + 1;

		cout.setf(std::ios::left);
		cout<<setfill('-')<<setw(lineWWithId)<<"\n-"<<endl;
		cout<<setfill('*')<<setw(lineWWithId)<<"|***** Caract Machine  (CPU/GPU/HDD/RAM) "<<"|"<<endl;
		cout<<setfill('-')<<setw(lineWWithId)<<"|-"<<"|"<<endl;
		cout<<setfill(' ');
		for(int j=0; j<M(); j++)
		{
			printf("| M%2d: %d/%d\t%d/%d\t%d/%d\t%d/%d |\n"
				,j
				,Data.ListOfMachines[j].QtyCPU
				,Data.ListOfMachines[j].CostCPU
				,Data.ListOfMachines[j].QtyGPU
				,Data.ListOfMachines[j].CostGPU
				,Data.ListOfMachines[j].QtyHDD
				,Data.ListOfMachines[j].CostHDD
				,Data.ListOfMachines[j].QtyRAM
				,Data.ListOfMachines[j].CostRAM
				);
		}
		cout<<"\n\n";
	}
}

void AfficherListeServeurBis()
{
	if(VERBOSE)
	{
		using namespace std;
		int idColWidth = 3;
		int dataWidth = 4;
		int unitWidth = dataWidth * 4;
		int lineWidth = (unitWidth+2) * M();
		int lineWWithId = lineWidth + idColWidth + 1;

		cout.setf(std::ios::left);
		cout<<setfill('-')<<setw(lineWWithId)<<"\n-"<<endl;
		cout<<setfill('*')<<setw(lineWWithId)<<"|***** Liste Serveur Bis  (CPU/GPU/HDD/RAM) "<<"|"<<endl;
		cout<<setfill('-')<<setw(lineWWithId)<<"|-"<<"|"<<endl;
		cout<<setfill(' ');
		for(int i=0; i< T(); i++)
		{
			cout<<"|"<<setw(idColWidth)<<i;
			for(int j=0; j<M(); j++)
			{
				cout<<"| "<<setw(dataWidth)
					<<Traitement.ListOfServeurbis[i][j].CPU<<setw(dataWidth)
					<<Traitement.ListOfServeurbis[i][j].GPU<<setw(dataWidth)
					<<Traitement.ListOfServeurbis[i][j].HDD<<setw(dataWidth)
					<<Traitement.ListOfServeurbis[i][j].RAM;
			}
			cout<<"|\n";
		}
		cout<<setfill('-')<<setw(lineWWithId)<<"-"<<endl<<endl;
	}
}


void AfficherRt()
{
	if(VERBOSE)
	{
		using namespace std;
		int idColWidth = 3;
		int dataWidth = 3;
		int unitWidth = dataWidth;
		int lineWidth = (unitWidth+2) * M();
		int lineWWithId = lineWidth + idColWidth + 1;

		cout.setf(std::ios::left);
		cout<<setfill('-')<<setw(lineWWithId)<<"\n-"<<endl;
		cout<<setfill('*')<<setw(lineWWithId)<<"|** Rt (tache*M) "<<"|"<<endl;
		cout<<setfill('-')<<setw(lineWWithId)<<"|-"<<"|"<<endl;
		cout<<setfill(' ');
		for(int i=0; i< N(); i++)
		{
			cout<<"|"<<setw(idColWidth)<<i;
			for(int j=0; j<M(); j++)
			{
				cout<<"| "<<setw(dataWidth)<< rt(i,j);
			}
			cout<<"|\n";
		}
		cout<<setfill('-')<<setw(lineWWithId)<<"-"<<endl<<endl;
	}
}
void AfficherAffinite()
{
	if(VERBOSE)
	{
		using namespace std;
		int lineWWithId =24;
		cout.setf(std::ios::left);
		cout<<setfill('-')<<setw(lineWWithId)<<"\n-"<<endl;
		cout<<setfill('*')<<setw(lineWWithId)<<"|***Affinity "<<"|"<<endl;
		cout<<setfill('-')<<setw(lineWWithId)<<"|-"<<"|"<<endl;
		for(int i=0; i< N(); i++)
		{
			for(int j=0; j<N(); j++)
			{
				cout<<"| "<<a(i,j);
			}
			cout<<"|\n";
		}
		cout<<setfill('-')<<setw(lineWWithId)<<"-"<<endl<<endl;
	}
}


void AfficherEdgeDispo()
{
	if(VERBOSE)
	{
		using namespace std;
		int idColWidth = 3;
		int unitWidth = 6;
		int lineWidth = (unitWidth+2) * Data.Network.NbEdges;
		int lineWWithId = lineWidth + idColWidth + 1;

		cout.setf(std::ios::left);
		cout<<setfill('-')<<setw(lineWWithId)<<"\n-"<<endl;
		cout<<setfill('*')<<setw(lineWWithId)<<"|***** Reseau edge bande "<<"|"<<endl;
		cout<<setfill('-')<<setw(lineWWithId)<<"|-"<<"|"<<endl;
		cout<<setfill(' ');
		for(int i=0; i< T(); i++)
		{
			cout<<"|"<<setw(idColWidth)<<i;
			for(int j=0; j< Data.Network.NbEdges; j++)
			{
				cout<<"| "<<setw(unitWidth)
					<<Traitement.EdgeBdeDispo[i][j];
			}
			cout<<"|\n";
		}
		cout<<setfill('-')<<setw(lineWWithId)<<"-"<<endl<<endl;
	}
}

void AfficherOrdo()
{
	if(VERBOSE)
	{
		using namespace std;
		int idColWidth = 3;
		int unitWidth = 3;
		int lineWidth = (unitWidth+2) * N();
		int lineWWithId = lineWidth + idColWidth + 1;

		cout.setf(std::ios::left);
		cout<<setfill('-')<<setw(lineWWithId)<<"\n-"<<endl;
		cout<<setfill('*')<<setw(lineWWithId)<<"|***** Ordonnancement (interval*tache)"<<"|"<<endl;
		cout<<setfill('-')<<setw(lineWWithId)<<"|-"<<"|"<<endl;
		cout<<setfill(' ');
		for(int i=0; i< T(); i++)
		{
			cout<<"|"<<setw(idColWidth)<<i;
			for(int j=0; j< N(); j++)
			{
				if(Traitement.ListOfOrdo[i][j].affecter == true)
					cout<<"| "<<setw(unitWidth)
						<<Traitement.ListOfOrdo[i][j].IndiceMachine;
				else if(u(j, i)==1)
					cout<<"| "<<setw(unitWidth)<<"rho";
				else cout<<"| "<<setw(unitWidth)<<"*";
			}
			cout<<"|\n";
		}
		cout<<setfill('-')<<setw(lineWWithId)<<"-"<<endl<<endl;
	}
}
void AfficherListesTache(int indice)
{
	//SetConsoleTextAttribute(consolehwnd, 0x0007);

	if(false)
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
		///Listes pré
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
	int data[8][12] = {
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
	int data1[8][12] = { 
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
	int data2[8][12] = { 
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
	int data3[8][12] = {  // == data2
		{1,1,1,1,-1,-1,-1, 1, 1, -1, 1, 1},
		{0,-1,0,-1,0,0,0, 0, 0, 0, -1, 0},
		{0,-1,0,-1,0,-1,0, -1, 0, -1, 0, -1},
		{1,1,1,1,1,1,1,-1,1,1,1,1},
		{-1,-1,-1,-1,-1,-1,-1, -1, 1, 1, -1, -1},
		{1,1,1,1, 1, 1,1,1,-1, -1, 1, 1},
		{-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1},
		{-1,-1,-1,-1,-1,1,1, 1, -1, -1, -1, -1}
	};

	///solution cplex donnees1_2
	int data1_2[8][12] = {  // == data2
		{-1,0,0,0,-1,0,0,0,0,0,-1,0},
		{-1,-1,-1,-1,-1,-1,-1, -1, -1, -1, -1, -1},
		{-1,-1,0,-1,0,-1,0, -1, 0,-1, 0, -1},
		{1,1,1,1,1,1,1,1,1,1,1,1},
		{0,-1,0,-1,0,-1,0, -1, 0, -1, 0, -1},
		{-1,-1,-1,-1,1,-1,-1, -1, -1, -1, -1, -1},
		{1,1,1,1,-1,-1,1,1,1,1,1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1}
	};
	for(int i =0; i<n; i++)
	{
		for(int j =0; j<t; j++)
		{
			if(data1_2[i][j] != -1)
				Traitement.ListOfOrdo[j][i].affecter = true;
			else Traitement.ListOfOrdo[j][i].affecter = false;
			Traitement.ListOfOrdo[j][i].IndiceMachine = data1_2[i][j];
		}
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