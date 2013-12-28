///Some functions for test purpose
///Author: SHANG Lei - shang.france@gmail.com
#include "Test.h"

//================= Fonctions pour l'affichage ====================
void AfficherIntervalle()
{
	if(!CACHER_TOUT && AFFICHER_INTERVALLE)
	{
		char title[50];
		sprintf(title, "Intervalles");
		ConsoleTable ct (title, T(), 1);
		ct.SetColWidth(10);
		for(int i=0;i<T();i++){
			sprintf(title, " [%d,%d] ",Traitement.ListOfIntervalles[i].BorneInf,Traitement.ListOfIntervalles[i].BorneSup);
			ct.Print(title);
		}
	}
}

void AfficherCaracMachine()
{
	if(!CACHER_TOUT && AFFICHER_MACHINE)
	{
		ConsoleTable ct(" Machines  ", M(), 4);
		ct.SetColHeader(0, "QtyCPU(cost)")
			.SetColHeader(1, "QtyGPU(cost)")
			.SetColHeader(2, "QtyRAM(cost)")
			.SetColHeader(3, "QtyHDD(cost)");
		
		ostringstream EleBuilder;
		for(int j=0; j<M(); j++)
		{
			EleBuilder.str("");
			EleBuilder<< Data.ListOfMachines[j].QtyCPU<< "(" << Data.ListOfMachines[j].CostCPU << ")";
			ct.Print(EleBuilder.str());
			EleBuilder.str("");
			EleBuilder<< Data.ListOfMachines[j].QtyGPU<< "(" << Data.ListOfMachines[j].CostGPU << ")";
			ct.Print(EleBuilder.str());
			EleBuilder.str("");
			EleBuilder<< Data.ListOfMachines[j].QtyRAM<< "(" << Data.ListOfMachines[j].CostRAM << ")";
			ct.Print(EleBuilder.str());
			EleBuilder.str("");
			EleBuilder<< Data.ListOfMachines[j].QtyHDD<< "(" << Data.ListOfMachines[j].CostHDD << ")";
			ct.Print(EleBuilder.str());
		}
	}
}

void AfficherListeServeurBis()
{
	if(!CACHER_TOUT && AFFICHER_SERVEUR_BIS)
	{
		ConsoleTable ct("Liste Serveur Bis  (CPU/GPU/HDD/RAM)", T(), M());
		ct.SetColWidth(15);
		ostringstream EleBuilder;
		for(int i=0; i< T(); i++)
		{
			for(int j=0; j<M(); j++)
			{
				EleBuilder.str("");
				EleBuilder<<Traitement.ListOfServeurbis[i][j].CPU<<"/"
					<<Traitement.ListOfServeurbis[i][j].GPU<<"/"
					<<Traitement.ListOfServeurbis[i][j].HDD<<"/"
					<<Traitement.ListOfServeurbis[i][j].RAM;
				ct.Print(EleBuilder.str());
			}
		}
	}
}


void AfficherRt()
{
	if(!CACHER_TOUT && AFFICHER_RT)
	{
		ConsoleTable ct("Resuming time(N*M)", N(), M());
		ct.SetColWidth(3);
		for(int i=0; i< N(); i++)
			for(int j=0; j<M(); j++)
				ct.Print(rt(i,j));
	}
}

void AfficherAffinite()
{
	if(!CACHER_TOUT && AFFICHER_AFFINITE)
	{
		ConsoleTable ct("Affinity (N*N)", N(), N());
		ct.SetColWidth(3);
		for(int i=0; i< N(); i++)
			for(int j=0; j<N(); j++)
				ct.Print(a(i,j));
	}
}


void AfficherEdgeDispo()
{
	if(!CACHER_TOUT && AFFICHER_ARC_DISPO)
	{
		ConsoleTable ct("Arc dispo (T*NbEdge)", T(), Data.Network.NbEdges);
		ct.SetColWidth(8); 
		for(int i=0; i< T(); i++)
			for(int j=0; j< Data.Network.NbEdges; j++)
				ct.Print(Traitement.EdgeBdeDispo[i][j]);
	}
}

void AfficherOrdo()
{
	if(!CACHER_TOUT && AFFICHER_ORDO)
	{
		ConsoleTable ct("Ordonnancement (T*N)", T(), N());
		ct.SetColWidth(4);
		for(int i=0; i< T(); i++)
		{
			for(int j=0; j< N(); j++)
			{
				if(Traitement.ListOfOrdo[i][j].affecter == true)
					ct.Print(Traitement.ListOfOrdo[i][j].IndiceMachine);
				else if(u(j, i)==1)
					ct.Print("sus");
				else ct.Print("-");
			}
		}
	}
}


void AfficherListesTache(int indice)
{
	//SetConsoleTextAttribute(consolehwnd, 0x0007);
	if(!CACHER_TOUT && AFFICHER_LISTS_TACHES)
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

void InitPrio(HDDRAM &hr){hr.prio = rand()%10;}

void Test()
{
	HDDRAM arr[10];
	std::for_each(arr, arr+10, InitPrio);
	PrintArrayPrio(arr, 10);
	SortListByPrio(arr, 10);
	TestOrderByPrio(arr, 10);
	PrintArrayPrio(arr,10);
}