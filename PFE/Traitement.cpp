#include "Data.h"
#include "Traitement.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <algorithm>
#include <list>
#include <iostream>

Trait Traitement;

/************************************************************************************/
// Function CalculInterval
// Calcule les intervalles tel que sur [T,T+1], les u(i,t) sont consants
/************************************************************************************/
void CalculInterval(){
	bool t = 0;
	int indice=0;
	int indice2 = 0;
	int tacheI = 0;
	Traitement.NbInterval = 0;

	
	for(int instant=0;instant<T()-1;instant++){
		for(tacheI=0;tacheI<N()-1;tacheI++){
			if(((Data.ListOfTasks[tacheI].LIsToBeProcessed[instant]==Data.ListOfTasks[tacheI].LIsToBeProcessed[instant+1])&&(Data.ListOfTasks[tacheI+1].LIsToBeProcessed[instant]!=Data.ListOfTasks[tacheI+1].LIsToBeProcessed[instant+1]))||((Data.ListOfTasks[tacheI].LIsToBeProcessed[instant]!=Data.ListOfTasks[tacheI].LIsToBeProcessed[instant+1])&&(Data.ListOfTasks[tacheI+1].LIsToBeProcessed[instant]==Data.ListOfTasks[tacheI+1].LIsToBeProcessed[instant+1]))){
					if(indice==0){
						Traitement.ListOfIntervalles[indice].BorneInf = 0;
						Traitement.ListOfIntervalles[indice].BorneSup = instant;
						indice++;
						Traitement.NbInterval++;
					}
					else{
						Traitement.ListOfIntervalles[indice].BorneInf = Traitement.ListOfIntervalles[indice-1].BorneSup + 1;
						Traitement.ListOfIntervalles[indice].BorneSup = instant;
						if(Traitement.ListOfIntervalles[indice].BorneInf>Traitement.ListOfIntervalles[indice].BorneSup){
							Traitement.ListOfIntervalles[indice].BorneInf = 0;
							Traitement.ListOfIntervalles[indice].BorneSup = 0;
						}
						else{
							indice++;
							Traitement.NbInterval++;
						}
					}			
			}			
		}
	}
	//permet de vérifier que le dernier intervalles contient le dernier instant de plannification et on l'ajoute si ce n'est pas le cas.
	if(Traitement.ListOfIntervalles[indice - 1].BorneSup == T()-1){
		indice--;
	}
	else{
		Traitement.ListOfIntervalles[indice].BorneInf = Traitement.ListOfIntervalles[indice-1].BorneSup + 1;
		Traitement.ListOfIntervalles[indice].BorneSup = T()-1;
		Traitement.NbInterval++;
	}

	//Affichage des intervalles calculés
	printf("Nombre d'interval : %d\n",Traitement.NbInterval);
	for(int i=0;i<=indice;i++){
		printf("intervalle %d : [%d,%d] \n",i,Traitement.ListOfIntervalles[i].BorneInf,Traitement.ListOfIntervalles[i].BorneSup);
	}
}

/************************************************************************************/
// Function CalculCoutNorm
// Calcule le cout normalisé des serveurs et classe la liste des serveurs par
// ordre croissant de leur cout normalisé
/************************************************************************************/
void CalculCoutNorm(){

	int i;
	int j=0;
	bool trier =0;
	float CoutTotal;
	float Cout;
	for(i=0; i<M(); i++){

		Traitement.ListOfServer[j].IndiceServeur = i;
		CoutTotal = (mc(i)*alphac(i) + mg(i)*alphag(i) + mr(i)*alphar(i) + mh(i)*alphah(i));
		Cout = CoutTotal/(mc(i)+mg(i)+mr(i)+mh(i));
		Traitement.ListOfServer[j].CoutNorm = Cout;
		if(Traitement.ListOfServer[j].CoutNorm < Traitement.ListOfServer[j-1].CoutNorm){
			Serveur memoire;
			memoire = Traitement.ListOfServer[j-1];
			Traitement.ListOfServer[j-1]=Traitement.ListOfServer[j];
			Traitement.ListOfServer[j]=memoire;
			if(Traitement.ListOfServer[j-1].CoutNorm<Traitement.ListOfServer[j-2].CoutNorm){
				Serveur memoire;
				memoire = Traitement.ListOfServer[j-2];
				Traitement.ListOfServer[j-2]=Traitement.ListOfServer[j-1];
				Traitement.ListOfServer[j-1]=memoire;
			}

		}
		j++;

	}
	for (int i=0; i<M(); i++){
		printf("Cout Normalise de la machine %d : %f \n",Traitement.ListOfServer[i].IndiceServeur,Traitement.ListOfServer[i].CoutNorm);
	}
}

/************************************************************************************/
// Function CalculTotalCost
// Calcule le cout total de la plannification sur l'ensemble des machine physique
/************************************************************************************/
int TotalCost(){

	int TotalCost = 0;
	int CoutGPU = 0;
	int CoutCPU = 0;
	int CoutRAM = 0;
	int CoutHDD = 0;
	int CoutUnitaire = 0;
	int penality = 0;
	int temps = 0;

	for(int t=0;t<T();t++){
		for(int n=0;n<N();n++){
			if((Traitement.ListOfOrdo[t][n].IndiceMachine!=-1) && (Traitement.ListOfOrdo[t][n].IndiceMachine!=-2) && (Traitement.ListOfOrdo[t][n].affecter!=0)){
				CoutGPU = CoutGPU + alphag(Traitement.ListOfOrdo[t][n].IndiceMachine)*ng(n);
				CoutCPU = CoutCPU + alphac(Traitement.ListOfOrdo[t][n].IndiceMachine)*nc(n);
				CoutRAM = CoutRAM + alphar(Traitement.ListOfOrdo[t][n].IndiceMachine)*nr(n);
				CoutHDD = CoutHDD + alphah(Traitement.ListOfOrdo[t][n].IndiceMachine)*nh(n);
			}
			if(Traitement.ListOfOrdo[t][n].IndiceMachine==-1){
				penality = penality + rho(n);
			}
		}
		for(int indice = 0;indice<Traitement.ListOfNbServeurOn[indice].NbServeurOn;indice++){
			//printf("valeur de t : %d \n",t);
			//printf("nombre de machine allumé %d \n",Traitement.ListOfNbServeurOn[indice].NbServeurOn);
			//printf("CoutUnitaire total :%d \n",CoutUnitaire);
			CoutUnitaire = CoutUnitaire + (indice * beta(t));

		}
	}

	//printf("penalite total : %d \n",penality);
	//printf("CoutUnitaire total :%d \n",CoutUnitaire);
	
	TotalCost = CoutGPU + CoutCPU + CoutRAM + CoutHDD + CoutUnitaire + penality;
	printf("Cout total : %d \n",TotalCost);
	return TotalCost;
}

/************************************************************************************/
// Function CalculCoutAffectation
// Calcule le cout d'affectation de la tache i sur la machine j
/************************************************************************************/
float CalculCoutAffectation(unsigned int i,unsigned int j){
	float cout;
	cout = (alphac(j)*nc(i)+alphag(j)*ng(i)+alphar(j)*nr(i)+alphah(j)*nh(i));
	return cout;

}

bool CalculFesabiliteResau(unsigned tachei,unsigned tachej, unsigned machinei,unsigned machinej,unsigned indice){
	int iEdge,iTime;
	for (iEdge=0;iEdge<NbEdges();iEdge++)
	{
		for (iTime=Traitement.ListOfIntervalles[indice].BorneInf;iTime<=Traitement.ListOfIntervalles[indice].BorneSup;iTime++)
		{
			int iBdw=0, iLoop;
			for (iLoop=0;iLoop<NbMachEdge(iEdge);iLoop++)
			{
				unsigned int iSwap;
				CoupleMachines(iEdge,iLoop,machinei,machinej);
				if (machinei>machinej){ 
					iSwap=machinei; 
					machinei=machinej; 
					machinej=iSwap;
				}
				iBdw+=b(tachei,tachej);
				Traitement.ListOfReseau[iTime][iEdge][iLoop].Mach1 = machinei;
				Traitement.ListOfReseau[iTime][iEdge][iLoop].Mach2 = machinej;
				Traitement.ListOfReseau[iTime][iEdge][iLoop].BdePassanteDispo = Traitement.ListOfReseau[iTime][iEdge][iLoop].BdePassanteDispo - iBdw;				
			}
			if(Traitement.ListOfReseau[iTime][iEdge][iLoop].BdePassanteDispo > 0){
				return true;
			}
			else
				return false;
		}
	}


}

void MaJReseau(unsigned tachei,unsigned tachej, unsigned machinei,unsigned machinej,unsigned int indice){
	int iEdge,iTime;
	for (iEdge=0;iEdge<NbEdges();iEdge++)
	{
		for (iTime=Traitement.ListOfIntervalles[indice].BorneInf;iTime<=Traitement.ListOfIntervalles[indice].BorneSup;iTime++)
		{
			int iBdw=0, iLoop;
			for (iLoop=0;iLoop<NbMachEdge(iEdge);iLoop++)
			{
			unsigned int iSwap;
			CoupleMachines(iEdge,iLoop,machinei,machinej);
			if (machinei>machinej){ 
				iSwap=machinei; 
				machinei=machinej; 
				machinej=iSwap;
			}
			iBdw+=b(tachei,tachej);
			Traitement.ListOfReseau[iTime][iEdge][iLoop].Mach1 = machinei;
			Traitement.ListOfReseau[iTime][iEdge][iLoop].Mach2 = machinej;
			Traitement.ListOfReseau[iTime][iEdge][iLoop].BdePassanteDispo = Traitement.ListOfReseau[iTime][iEdge][iLoop].BdePassanteDispo - iBdw;				
			}

		}
	}

}

void MaJServeur(unsigned int IndiceServeur, unsigned int indice){
	for(int temps = Traitement.ListOfIntervalles[indice].BorneInf;temps<=Traitement.ListOfIntervalles[indice].BorneSup;temps++){
				Traitement.ListOfServeurbis[temps][IndiceServeur].CPU = Data.ListOfMachines[IndiceServeur].QtyCPU;
				Traitement.ListOfServeurbis[temps][IndiceServeur].GPU = Data.ListOfMachines[IndiceServeur].QtyGPU;
				Traitement.ListOfServeurbis[temps][IndiceServeur].RAM = Data.ListOfMachines[IndiceServeur].QtyRAM;
				Traitement.ListOfServeurbis[temps][IndiceServeur].HDD = Data.ListOfMachines[IndiceServeur].QtyHDD;
	}
}

/************************************************************************************/
// Function ConstructionListesTacheNonPr
// Permet de construire les listes des taches non préamtable
/************************************************************************************/
void ConstructionListesTacheNonPr(unsigned int indice){

	int j=0;
	int k=0;
	int l=0;
	int m=0;
	Traitement.NbHDDRAMGPU = 0; //permet de stocker le nombre de machine virtuelle avec des besoins en GPU/CPU et besoins HDD > besoins RAM
	Traitement.NbRAMHDDGPU = 0; //permet de stocker le nombre de machine virtuelle avec des besoins en GPU/CPU et besoins RAM > besoins HDD
	Traitement.NbHDDRAMCPU = 0;	//permet de stocker le nombre de machine virtuelle sans GPU et besoins HDD > besoins RAM
	Traitement.NbRAMHDDCPU = 0; //permet de stocker le nombre de machine virtuelle sans GPU et besoins RAM > besoins HDD

	for (int temps=Traitement.ListOfIntervalles[indice].BorneInf;temps<=Traitement.ListOfIntervalles[indice].BorneSup;temps++){
		for (int i=0; i<N();i++){
			//Construction des listes tâches non préemtable avec des besoins en GPU et CPU 
			if((Data.ListOfTasks[i].LIsToBeProcessed[temps]==1)&&(Data.ListOfTasks[i].QtyGPU>0)&&(R(i)==0)){
				//Construction de la liste HDD > RAM
				if(nh(i)>nr(i))
				{
					Traitement.ListOfTasks1GPU[j].IndiceVM = i;
					j++;
					Traitement.NbHDDRAMGPU++;
				}
				//Construction de la liste RAM > HDD
				else
				{
					Traitement.ListOfTasks2GPU[k].IndiceVM = i;
					k++;
					Traitement.NbRAMHDDGPU++;
				}
			}
			//Construction des listes VM non préemtable avec des besoins en CPU uniquement
			else if((Data.ListOfTasks[i].LIsToBeProcessed[temps]==1)&&(R(i)==0)){
				//Construction de la liste HDD > RAM
				if(nh(i)>nr(i))
				{
					Traitement.ListOfTasks1CPU[l].IndiceVM = i;
					l++;
					Traitement.NbHDDRAMCPU++;
				}
				//Construction de la liste RAM > HDD
				else
				{
					Traitement.ListOfTasks2CPU[m].IndiceVM = i;
					m++;
					Traitement.NbRAMHDDCPU++;
				}

			}
		}
	}
	/*
	//Affichage des listes
	printf("\n **** Avec GPU **** \n");
	printf("\n **** Liste HDDRAM **** \n");
	for (int i=0; i<j; i++){
		printf("%d \t",Traitement.ListOfTasks1GPU[i].IndiceVM);
	}
	printf("\n **** Liste RAMHDD **** \n");
	for (int i=0; i<k; i++){
		printf("%d \t",Traitement.ListOfTasks2GPU[i].IndiceVM);
	}
	printf("\n **** Sans GPU **** \n");
	printf("\n **** Liste HDDRAM **** \n");
	for (int i=0; i<l; i++){
		printf("%d \t",Traitement.ListOfTasks1CPU[i].IndiceVM);
	
	}
	printf("\n **** Liste RAMHDD **** \n");
	for (int i=0; i<m; i++){
		printf("%d \t",Traitement.ListOfTasks2CPU[i].IndiceVM);
	}
	printf("\n");*/
}

void ConstructionListesTachePr(unsigned int indice){

	int j=0;
	int k=0;
	int l=0;
	int m=0;
	Traitement.NbHDDRAMGPUPr = 0; //permet de stocker le nombre de machine virtuelle avec des besoins en GPU/CPU et besoins HDD > besoins RAM
	Traitement.NbRAMHDDGPUPr = 0; //permet de stocker le nombre de machine virtuelle avec des besoins en GPU/CPU et besoins RAM > besoins HDD
	Traitement.NbHDDRAMCPUPr = 0;	//permet de stocker le nombre de machine virtuelle sans GPU et besoins HDD > besoins RAM
	Traitement.NbRAMHDDCPUPr = 0; //permet de stocker le nombre de machine virtuelle sans GPU et besoins RAM > besoins HDD

	for (int temps=Traitement.ListOfIntervalles[indice].BorneInf;temps<=Traitement.ListOfIntervalles[indice].BorneInf;temps++){
		for (int i=0; i<N();i++){
			//Construction des listes tâches non préemtable avec des besoins en GPU et CPU 
			if((Data.ListOfTasks[i].LIsToBeProcessed[temps]==1)&&(Data.ListOfTasks[i].QtyGPU>0)&&(R(i)==1)){
				//Construction de la liste HDD > RAM
				if(nh(i)>nr(i))
				{
					Traitement.ListOfTasks1GPUPr[j].IndiceVM = i;
					j++;
					Traitement.NbHDDRAMGPUPr++;
				}
				//Construction de la liste RAM > HDD
				else
				{
					Traitement.ListOfTasks2GPUPr[k].IndiceVM = i;
					k++;
					Traitement.NbRAMHDDGPUPr++;
				}
			}
			//Construction des listes VM non préemtable avec des besoins en CPU uniquement
			else if((Data.ListOfTasks[i].LIsToBeProcessed[temps]==1)&&(R(i)==1)){
				//Construction de la liste HDD > RAM
				if(nh(i)>nr(i))
				{
					Traitement.ListOfTasks1CPUPr[l].IndiceVM = i;
					l++;
					Traitement.NbHDDRAMCPUPr++;
				}
				//Construction de la liste RAM > HDD
				else
				{
					Traitement.ListOfTasks2CPUPr[m].IndiceVM = i;
					m++;
					Traitement.NbRAMHDDCPUPr++;
				}

			}
		}
	}
	/*
	//Affichage des listes
	printf("\n **** Avec GPU **** \n");
	printf("\n **** Liste HDDRAM **** \n");
	for (int i=0; i<j; i++){
		printf("%d \t",Traitement.ListOfTasks1GPUPr[i].IndiceVM);
	}
	printf("\n **** Liste RAMHDD **** \n");
	for (int i=0; i<k; i++){
		printf("%d \t",Traitement.ListOfTasks2GPUPr[i].IndiceVM);
	}
	printf("\n **** Sans GPU **** \n");
	printf("\n **** Liste HDDRAM **** \n");
	for (int i=0; i<l; i++){
		printf("%d \t",Traitement.ListOfTasks1CPUPr[i].IndiceVM);
	}
	printf("\n **** Liste RAMHDD **** \n");
	for (int i=0; i<m; i++){
		printf("%d \t",Traitement.ListOfTasks2CPUPr[i].IndiceVM);
	}
	printf("\n");*/
}

void ConstructionListesTachePrbis(unsigned int indice){

	int j=0;
	int k=0;
	int l=0;
	int m=0;
	Traitement.NbHDDRAMGPUPr = 0; //permet de stocker le nombre de machine virtuelle avec des besoins en GPU/CPU et besoins HDD > besoins RAM
	Traitement.NbRAMHDDGPUPr = 0; //permet de stocker le nombre de machine virtuelle avec des besoins en GPU/CPU et besoins RAM > besoins HDD
	Traitement.NbHDDRAMCPUPr = 0;	//permet de stocker le nombre de machine virtuelle sans GPU et besoins HDD > besoins RAM
	Traitement.NbRAMHDDCPUPr = 0; //permet de stocker le nombre de machine virtuelle sans GPU et besoins RAM > besoins HDD

	for (int temps=Traitement.ListOfIntervalles[indice].BorneInf;temps<=Traitement.ListOfIntervalles[indice].BorneInf;temps++){
		for (int i=0; i<Traitement.NbPr;i++){
			//Construction des listes tâches non préemtable avec des besoins en GPU et CPU 
			if((Data.ListOfTasks[Traitement.ListofTasksPr[i].IndiceVM].LIsToBeProcessed[temps]==1)&&(Data.ListOfTasks[Traitement.ListofTasksPr[i].IndiceVM].QtyGPU>0)&&(R(i)==1)){
				//Construction de la liste HDD > RAM
				if(nh(i)>nr(i))
				{
					Traitement.ListOfTasks1GPUPr[j].IndiceVM = Traitement.ListofTasksPr[i].IndiceVM;
					j++;
					Traitement.NbHDDRAMGPUPr++;
				}
				//Construction de la liste RAM > HDD
				else
				{
					Traitement.ListOfTasks2GPUPr[k].IndiceVM = Traitement.ListofTasksPr[i].IndiceVM;
					k++;
					Traitement.NbRAMHDDGPUPr++;
				}
			}
			//Construction des listes VM non préemtable avec des besoins en CPU uniquement
			else if((Data.ListOfTasks[Traitement.ListofTasksPr[i].IndiceVM].LIsToBeProcessed[temps]==1)&&(R(i)==1)){
				//Construction de la liste HDD > RAM
				if(nh(Traitement.ListofTasksPr[i].IndiceVM)>nr(Traitement.ListofTasksPr[i].IndiceVM))
				{
					Traitement.ListOfTasks1CPUPr[l].IndiceVM = Traitement.ListofTasksPr[i].IndiceVM;
					l++;
					Traitement.NbHDDRAMCPUPr++;
				}
				//Construction de la liste RAM > HDD
				else
				{
					Traitement.ListOfTasks2CPUPr[m].IndiceVM = Traitement.ListofTasksPr[i].IndiceVM;
					m++;
					Traitement.NbRAMHDDCPUPr++;
				}

			}
		}
	}
	/*
	//Affichage des listes
	printf("\n **** Avec GPU **** \n");
	printf("\n **** Liste HDDRAM **** \n");
	for (int i=0; i<j; i++){
		printf("%d \t",Traitement.ListOfTasks1GPUPr[i].IndiceVM);
	}
	printf("\n **** Liste RAMHDD **** \n");
	for (int i=0; i<k; i++){
		printf("%d \t",Traitement.ListOfTasks2GPUPr[i].IndiceVM);
	}
	printf("\n **** Sans GPU **** \n");
	printf("\n **** Liste HDDRAM **** \n");
	for (int i=0; i<l; i++){
		printf("%d \t",Traitement.ListOfTasks1CPUPr[i].IndiceVM);
	}
	printf("\n **** Liste RAMHDD **** \n");
	for (int i=0; i<m; i++){
		printf("%d \t",Traitement.ListOfTasks2CPUPr[i].IndiceVM);
	}
	printf("\n");*/

}

void ConstructionListeTachePr(unsigned int indice){

	int j=0;
	int k=0;
	int l=0;
	int m=0;
	Traitement.NbPr = 0;

	for (int temps=Traitement.ListOfIntervalles[indice].BorneInf;temps<=Traitement.ListOfIntervalles[indice].BorneInf;temps++){
		for (int i=0; i<N();i++){
			//Construction des listes tâches non préemtable avec des besoins en GPU et CPU 
			if((Data.ListOfTasks[i].LIsToBeProcessed[temps]==1)&&(R(i)==1)){
					Traitement.ListofTasksPr[j].IndiceVM=i;
					j++;
					Traitement.NbPr++;
		
			}
		}
	}

	/*
	//Affichage des listes
	printf("\n **** Avec GPU **** \n");
	printf("\n **** Liste HDDRAM **** \n");
	for (int i=0; i<j; i++){
		printf("%d \t",Traitement.ListOfTasks1GPUPr[i].IndiceVM);
	}
	printf("\n **** Liste RAMHDD **** \n");
	for (int i=0; i<k; i++){
		printf("%d \t",Traitement.ListOfTasks2GPUPr[i].IndiceVM);
	}
	printf("\n **** Sans GPU **** \n");
	printf("\n **** Liste HDDRAM **** \n");
	for (int i=0; i<l; i++){
		printf("%d \t",Traitement.ListOfTasks1CPUPr[i].IndiceVM);
	}
	printf("\n **** Liste RAMHDD **** \n");
	for (int i=0; i<m; i++){
		printf("%d \t",Traitement.ListOfTasks2CPUPr[i].IndiceVM);
	}
	printf("\n");*/
}

void CalculPrioGPU(unsigned int indiceServeur,unsigned int indice){
	int IB = 0;
	int WG = 0;
	int MachineRecevoir;
	//Calcule de la prioriété pour la liste contenant les VM ayant besoin de GPU et tel que besoins HDD > besoins RAM
	for(int iboucle = 0; iboucle<Traitement.NbHDDRAMGPU;iboucle++){
		Traitement.ListOfTasks1GPU[iboucle].prio = 0;
		MachineRecevoir = 0;
		if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks1GPU[iboucle].IndiceVM].affecter == 0){
			if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1GPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
					IB = M();
					Traitement.ListOfTasks1GPU[iboucle].prio = Traitement.ListOfTasks1GPU[iboucle].prio + IB;
			}
			else{
				IB = 0;
				Traitement.ListOfTasks1GPU[iboucle].prio = Traitement.ListOfTasks1GPU[iboucle].prio + IB;
			}
			if(indice!=0){
				if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks1GPU[iboucle].IndiceVM].dureeExe < mt(Traitement.ListOfTasks1GPU[iboucle].IndiceVM)){
					if(u(Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfIntervalles[indice-1].BorneSup)==0){
						int indiceInterval = indice-1;
						while(u(Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfIntervalles[indiceInterval].BorneSup)==0){
							indiceInterval--;
						}
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInterval].BorneSup][Traitement.ListOfTasks1GPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -M();
						}
					}
					else{
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1GPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -2*M();
						}
					}
				}
				else{
					for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks1GPU[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
						}
					}
					WG = M() -  MachineRecevoir;
				//printf("Valeur de WG : %d \n",WG);
				}
			}
			else{
				for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks1GPU[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
							//printf("Machine Recevoir %d\n",MachineRecevoir);
						}
					}
					WG = M() -  MachineRecevoir;
			}
			//printf("Valeur de WG : %d \n",WG);
			//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfTasks1GPU[iboucle].prio);
			Traitement.ListOfTasks1GPU[iboucle].prio = Traitement.ListOfTasks1GPU[iboucle].prio + WG;
			//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfTasks1GPU[iboucle].prio);	
			
		}
	}
	for(int iboucle=1;iboucle<Traitement.NbHDDRAMGPU;iboucle++){
		if(Traitement.ListOfTasks1GPU[iboucle].prio > Traitement.ListOfTasks1GPU[iboucle-1].prio){
				HDDRAM memoire = Traitement.ListOfTasks1GPU[iboucle-1];
				Traitement.ListOfTasks1GPU[iboucle-1]=Traitement.ListOfTasks1GPU[iboucle];
				Traitement.ListOfTasks1GPU[iboucle]=memoire;
		}
	}

	//Calcule de la prioriété pour la liste contenant les VM ayant besoin de GPU et tel que besoins RAM > besoins HDD
	for(int iboucle = 0; iboucle<Traitement.NbRAMHDDGPU;iboucle++){
		Traitement.ListOfTasks2GPU[iboucle].prio = 0;
		MachineRecevoir = 0;
		if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks2GPU[iboucle].IndiceVM].affecter == 0){
			if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks2GPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
				IB = M();
				Traitement.ListOfTasks2GPU[iboucle].prio = Traitement.ListOfTasks2GPU[iboucle].prio + IB;
			}
			else{
				IB = 0;
				Traitement.ListOfTasks2GPU[iboucle].prio = Traitement.ListOfTasks2GPU[iboucle].prio + IB;
			}
			if(indice!=0){
				if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks2GPU[iboucle].IndiceVM].dureeExe < mt(Traitement.ListOfTasks2GPU[iboucle].IndiceVM)){
					if(u(Traitement.ListOfTasks2GPU[iboucle].IndiceVM,Traitement.ListOfIntervalles[indice-1].BorneSup)==0){
						int indiceInterval = indice-1;
						while(u(Traitement.ListOfTasks2GPU[iboucle].IndiceVM,Traitement.ListOfIntervalles[indiceInterval].BorneSup)==0){
							indiceInterval--;
						}
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInterval].BorneSup][Traitement.ListOfTasks2GPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -M();
						}
					}
					else{
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks2GPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -2*M();
						}
					}
				}
				else{
					for(int iboucle2 = 0; iboucle2<M()-1; iboucle2++){
						if(Data.ListOfTasks[iboucle].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
						}
					}
					WG = M() -  MachineRecevoir;
				//printf("Valeur de WG : %d \n",WG);
				}
			}
			else{
				for(int iboucle2 = 0; iboucle2<M()-1; iboucle2++){
						if(Data.ListOfTasks[iboucle].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
						}
					}
					WG = M() -  MachineRecevoir;
			}
			Traitement.ListOfTasks2GPU[iboucle].prio = Traitement.ListOfTasks2GPU[iboucle].prio + WG;	
		//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks2GPU[iboucle].IndiceVM,Traitement.ListOfTasks2GPU[iboucle].prio);	
	
		}
	}
	for(int iboucle=1;iboucle<Traitement.NbRAMHDDGPU;iboucle++){
		if(Traitement.ListOfTasks2GPU[iboucle].prio < Traitement.ListOfTasks2GPU[iboucle-1].prio){
				RAMHDD memoire = Traitement.ListOfTasks2GPU[iboucle-1];
				Traitement.ListOfTasks2GPU[iboucle-1]=Traitement.ListOfTasks2GPU[iboucle];
				Traitement.ListOfTasks2GPU[iboucle]=memoire;
		}
	}
	//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfTasks1GPU[iboucle].prio);
	//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[0].IndiceVM,Traitement.ListOfTasks1GPU[0].prio);
}

void CalculPrioCPU(unsigned int indiceServeur,unsigned int indice){
	int IB,WG;
	IB = 0;
	WG = 0;
	int MachineRecevoir;
	//Calcule de la prioriété pour la liste contenant les VM ayant uniquement besoin de CPU et tel que besoins HDD > besoins RAM
	for(int iboucle = 0; iboucle<Traitement.NbHDDRAMCPU;iboucle++){
		Traitement.ListOfTasks1CPU[iboucle].prio = 0;
		MachineRecevoir = 0;
		if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks1CPU[iboucle].IndiceVM].affecter == 0){
			if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1CPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
				IB = M();
				Traitement.ListOfTasks1CPU[iboucle].prio = Traitement.ListOfTasks1CPU[iboucle].prio + IB;
			}
			else{
				IB = 0;
				Traitement.ListOfTasks1CPU[iboucle].prio = Traitement.ListOfTasks1CPU[iboucle].prio + IB;
			}
			if(indice!=0){
				if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks1CPU[iboucle].IndiceVM].dureeExe < mt(Traitement.ListOfTasks1CPU[iboucle].IndiceVM)){
					if((u(Traitement.ListOfTasks1CPU[iboucle].IndiceVM,Traitement.ListOfIntervalles[indice-1].BorneSup)==0)&&(Traitement.ListOfIntervalles[indice-1].BorneInf!=0)){
						int indiceInterval = indice-1;
						while(u(Traitement.ListOfTasks1CPU[iboucle].IndiceVM,Traitement.ListOfIntervalles[indiceInterval].BorneSup)==0){
							indiceInterval--;
						}
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInterval].BorneSup][Traitement.ListOfTasks1CPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -M();
						}
					}
					else if (Traitement.ListOfIntervalles[indice-1].BorneInf==0){
						for(int iboucle2 = 0; iboucle2<M()-1; iboucle2++){
							if(Data.ListOfTasks[Traitement.ListOfTasks1CPU[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
								MachineRecevoir++;
							}
						}
						WG = M() -  MachineRecevoir;
					}
					else{
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1CPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -M();
						}
					}
				}
				else{
					for(int iboucle2 = 0; iboucle2<M()-1; iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks1CPU[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
						}
					}
					WG = M() -  MachineRecevoir;
					//printf("Valeur de WG : %d \n",WG);
				}
			}
			else{
				for(int iboucle2 = 0; iboucle2<M()-1; iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks1CPU[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
						}
					}
					WG = M() -  MachineRecevoir;
					//printf("Valeur de WG : %d \n",WG);
			}
			Traitement.ListOfTasks1CPU[iboucle].prio = Traitement.ListOfTasks1CPU[iboucle].prio + WG;
		}
	//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1CPU[iboucle].IndiceVM,Traitement.ListOfTasks1CPU[iboucle].prio);	
	}
	for(int iboucle=2;iboucle<Traitement.NbHDDRAMCPU;iboucle++){
		if(Traitement.ListOfTasks1CPU[iboucle].prio > Traitement.ListOfTasks1CPU[iboucle-1].prio){
				HDDRAM memoire = Traitement.ListOfTasks1CPU[iboucle-1];
				Traitement.ListOfTasks1CPU[iboucle-1]=Traitement.ListOfTasks1CPU[iboucle];
				Traitement.ListOfTasks1CPU[iboucle]=memoire;
		}
	}

	/*for(int test=0;test<Traitement.NbHDDRAMCPUMachinej;test++){
		printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1CPUMachinej[test].IndiceVM,Traitement.ListOfTasks1CPUMachinej[test].prio);

	}*/

	//Calcule de la prioriété pour la liste contenant les VM ayant uniquement besoin de CPU et tel que besoins RAM > besoins HDD
	for(int iboucle = 0; iboucle<Traitement.NbRAMHDDCPU;iboucle++){
		Traitement.ListOfTasks2CPU[iboucle].prio = 0;
		MachineRecevoir = 0;
		if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks2CPU[iboucle].IndiceVM].affecter == 0){
			if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks2CPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
				IB = M();
				Traitement.ListOfTasks2CPU[iboucle].prio = Traitement.ListOfTasks2CPU[iboucle].prio + IB;
			}
			else{
				IB = 0;
				Traitement.ListOfTasks2CPU[iboucle].prio = Traitement.ListOfTasks2CPU[iboucle].prio + IB;
			}
			if(indice!=0){
				if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks2CPU[iboucle].IndiceVM].dureeExe < mt(Traitement.ListOfTasks2CPU[iboucle].IndiceVM)){
					if((u(Traitement.ListOfTasks2CPU[iboucle].IndiceVM,Traitement.ListOfIntervalles[indice-1].BorneSup)==0)&&(Traitement.ListOfIntervalles[indice-1].BorneInf!=0)){
						int indiceInterval = indice-1;
						while(u(Traitement.ListOfTasks2CPU[iboucle].IndiceVM,Traitement.ListOfIntervalles[indiceInterval].BorneSup)==0){
							indiceInterval--;
						}
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInterval].BorneSup][Traitement.ListOfTasks2CPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -M();
						}
					}
					else if (Traitement.ListOfIntervalles[indice-1].BorneInf==0){
						for(int iboucle2 = 0; iboucle2<M()-1; iboucle2++){
							if(Data.ListOfTasks[Traitement.ListOfTasks2CPU[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
								MachineRecevoir++;
							}
						}
						WG = M() -  MachineRecevoir;
					}
					else{
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks2CPU[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -M();
						}
					}
				}
				else{
					for(int iboucle2 = 0; iboucle2<M()-1; iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks2CPU[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
						}
					}
					WG = M() -  MachineRecevoir;
					//printf("Valeur de WG : %d \n",WG);
				}
			}
			else{
				for(int iboucle2 = 0; iboucle2<M()-1; iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks2CPU[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
						}
				}
				WG = M() -  MachineRecevoir;
				//printf("Valeur de WG : %d \n",WG);
			}
			Traitement.ListOfTasks2CPU[iboucle].prio = Traitement.ListOfTasks2CPU[iboucle].prio + WG;
		}
		//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks2CPU[iboucle].IndiceVM,Traitement.ListOfTasks2CPU[iboucle].prio);	
	}
	//while(t==false){
		for(int iboucle=2;iboucle<Traitement.NbRAMHDDCPU;iboucle++){
			if(Traitement.ListOfTasks2CPU[iboucle].prio > Traitement.ListOfTasks2CPU[iboucle-1].prio){
				RAMHDD memoire = Traitement.ListOfTasks2CPU[iboucle-1];
				Traitement.ListOfTasks2CPU[iboucle-1]=Traitement.ListOfTasks2CPU[iboucle];
				Traitement.ListOfTasks2CPU[iboucle]=memoire;
			}
			//if(Traitement.ListOfTasks2CPU[iboucle-1].prio
		}
	//}
}
void CalculPrioGPUPr(unsigned int indiceServeur,unsigned int indice){
	int IB,WG;
	IB = 0;
	int MachineRecevoir = 0;
	//Calcule de la prioriété pour la liste contenant les VM ayant besoin de GPU et tel que besoins HDD > besoins RAM
	for(int iboucle = 0; iboucle<Traitement.NbHDDRAMGPUMachinej;iboucle++){
		Traitement.ListOfTasks1GPUMachinej[iboucle].prio = 0;
		MachineRecevoir = 0;
		if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks1GPUMachinej[iboucle].IndiceVM].affecter == 0){
			if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1GPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
					IB = M();
					Traitement.ListOfTasks1GPUMachinej[iboucle].prio = Traitement.ListOfTasks1GPUMachinej[iboucle].prio + IB;
			}
			else{
				IB = 0;
				Traitement.ListOfTasks1GPUMachinej[iboucle].prio = Traitement.ListOfTasks1GPUMachinej[iboucle].prio + IB;
			}
			if(indice!=0){
				if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks1GPUMachinej[iboucle].IndiceVM].dureeExe < mt(Traitement.ListOfTasks1GPUMachinej[iboucle].IndiceVM)){
					if(u(Traitement.ListOfTasks1GPUMachinej[iboucle].IndiceVM,Traitement.ListOfIntervalles[indice-1].BorneSup)==0){
						int indiceInterval = indice-1;
						while(u(Traitement.ListOfTasks1GPUMachinej[iboucle].IndiceVM,Traitement.ListOfIntervalles[indiceInterval].BorneSup)==0){
							indiceInterval--;
						}
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInterval].BorneSup][Traitement.ListOfTasks1GPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -M();
						}
					}
					else{
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1GPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -2*M();
						}
					}
				}
				else{
					for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks1GPUMachinej[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
						}
					}
					WG = M() -  MachineRecevoir;
				//printf("Valeur de WG : %d \n",WG);
				}
			}
			else{
				for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks1GPUMachinej[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
							//printf("Machine Recevoir %d\n",MachineRecevoir);
						}
					}
					WG = M() -  MachineRecevoir;
			}
			//printf("Valeur de WG : %d \n",WG);
			//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfTasks1GPU[iboucle].prio);
			Traitement.ListOfTasks1GPUMachinej[iboucle].prio = Traitement.ListOfTasks1GPUMachinej[iboucle].prio + WG;
			//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfTasks1GPU[iboucle].prio);	
			if(Traitement.ListOfTasks1GPUMachinej[iboucle].prio < Traitement.ListOfTasks1GPUMachinej[iboucle-1].prio){
				HDDRAM memoire = Traitement.ListOfTasks1GPUMachinej[iboucle-1];
				Traitement.ListOfTasks1GPUMachinej[iboucle-1]=Traitement.ListOfTasks1GPUMachinej[iboucle];
				Traitement.ListOfTasks1GPUMachinej[iboucle]=memoire;
			}
		}
	}

	//Calcule de la prioriété pour la liste contenant les VM ayant besoin de GPU et tel que besoins RAM > besoins HDD
	for(int iboucle = 0; iboucle<Traitement.NbRAMHDDGPUMachinej;iboucle++){
		Traitement.ListOfTasks2GPUMachinej[iboucle].prio = 0;
		MachineRecevoir = 0;
		if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks2GPUMachinej[iboucle].IndiceVM].affecter == 0){
			if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks2GPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
					IB = M();
					Traitement.ListOfTasks2GPUMachinej[iboucle].prio = Traitement.ListOfTasks2GPUMachinej[iboucle].prio + IB;
			}
			else{
				IB = 0;
				Traitement.ListOfTasks2GPUMachinej[iboucle].prio = Traitement.ListOfTasks2GPUMachinej[iboucle].prio + IB;
			}
			if(indice!=0){
				if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks2GPUMachinej[iboucle].IndiceVM].dureeExe < mt(Traitement.ListOfTasks2GPUMachinej[iboucle].IndiceVM)){
					if(u(Traitement.ListOfTasks2GPUMachinej[iboucle].IndiceVM,Traitement.ListOfIntervalles[indice-1].BorneSup)==0){
						int indiceInterval = indice-1;
						while(u(Traitement.ListOfTasks2GPUMachinej[iboucle].IndiceVM,Traitement.ListOfIntervalles[indiceInterval].BorneSup)==0){
							indiceInterval--;
						}
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInterval].BorneSup][Traitement.ListOfTasks2GPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -M();
						}
					}
					else{
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks2GPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -2*M();
						}
					}
				}
				else{
					for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks2GPUMachinej[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
						}
					}
					WG = M() -  MachineRecevoir;
				//printf("Valeur de WG : %d \n",WG);
				}
			}
			else{
				for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks2GPUMachinej[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
							//printf("Machine Recevoir %d\n",MachineRecevoir);
						}
					}
					WG = M() -  MachineRecevoir;
			}
			//printf("Valeur de WG : %d \n",WG);
			//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfTasks1GPU[iboucle].prio);
			Traitement.ListOfTasks2GPUMachinej[iboucle].prio = Traitement.ListOfTasks2GPUMachinej[iboucle].prio + WG;
			//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfTasks1GPU[iboucle].prio);	
			if(Traitement.ListOfTasks2GPU[iboucle].prio < Traitement.ListOfTasks2GPUMachinej[iboucle-1].prio){
				RAMHDD memoire = Traitement.ListOfTasks2GPUMachinej[iboucle-1];
				Traitement.ListOfTasks2GPUMachinej[iboucle-1]=Traitement.ListOfTasks2GPUMachinej[iboucle];
				Traitement.ListOfTasks2GPUMachinej[iboucle]=memoire;
			}
		}
	}

}

void CalculPrioCPUPr(unsigned int indiceServeur,unsigned int indice){
	int IB,WG;
	IB = 0;
	int MachineRecevoir = 0;
	//Calcule de la prioriété pour la liste contenant les VM ayant besoin de GPU et tel que besoins HDD > besoins RAM
	for(int iboucle = 0; iboucle<Traitement.NbHDDRAMCPUMachinej;iboucle++){
		Traitement.ListOfTasks1CPUMachinej[iboucle].prio = 0;
		MachineRecevoir = 0;
		if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks1CPUMachinej[iboucle].IndiceVM].affecter == 0){
			if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1CPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
					IB = M();
					Traitement.ListOfTasks1CPUMachinej[iboucle].prio = Traitement.ListOfTasks1CPUMachinej[iboucle].prio + IB;
			}
			else{
				IB = 0;
				Traitement.ListOfTasks1CPUMachinej[iboucle].prio = Traitement.ListOfTasks1CPUMachinej[iboucle].prio + IB;
			}
			if(indice!=0){
				if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks1CPUMachinej[iboucle].IndiceVM].dureeExe < mt(Traitement.ListOfTasks1CPUMachinej[iboucle].IndiceVM)){
					if(u(Traitement.ListOfTasks1CPUMachinej[iboucle].IndiceVM,Traitement.ListOfIntervalles[indice-1].BorneSup)==0){
						int indiceInterval = indice-1;
						while(u(Traitement.ListOfTasks1CPUMachinej[iboucle].IndiceVM,Traitement.ListOfIntervalles[indiceInterval].BorneSup)==0){
							indiceInterval--;
						}
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInterval].BorneSup][Traitement.ListOfTasks1CPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -M();
						}
					}
					else{
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1CPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -2*M();
						}
					}
				}
				else{
					for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks1CPUMachinej[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
						}
					}
					WG = M() -  MachineRecevoir;
				//printf("Valeur de WG : %d \n",WG);
				}
			}
			else{
				for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks1CPUMachinej[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
							//printf("Machine Recevoir %d\n",MachineRecevoir);
						}
					}
					WG = M() -  MachineRecevoir;
			}
			//printf("Valeur de WG : %d \n",WG);
			//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfTasks1GPU[iboucle].prio);
			Traitement.ListOfTasks1CPUMachinej[iboucle].prio = Traitement.ListOfTasks1CPUMachinej[iboucle].prio + WG;
			//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfTasks1GPU[iboucle].prio);	
			for(int iboucle=2;iboucle<Traitement.NbHDDRAMCPUMachinej;iboucle++){
				if(Traitement.ListOfTasks1CPUMachinej[iboucle].prio > Traitement.ListOfTasks1CPUMachinej[iboucle-1].prio){
					HDDRAM memoire = Traitement.ListOfTasks1CPUMachinej[iboucle-1];
					Traitement.ListOfTasks1CPUMachinej[iboucle-1]=Traitement.ListOfTasks1CPUMachinej[iboucle];
					Traitement.ListOfTasks1CPUMachinej[iboucle]=memoire;
				}
			}
		}
	}

	//Calcule de la prioriété pour la liste contenant les VM ayant besoin de GPU et tel que besoins HDD > besoins RAM
	for(int iboucle = 0; iboucle<Traitement.NbRAMHDDCPUMachinej;iboucle++){
		Traitement.ListOfTasks2CPUMachinej[iboucle].prio = 0;
		MachineRecevoir = 0;
		if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks2CPUMachinej[iboucle].IndiceVM].affecter == 0){
			if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks2CPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
					IB = M();
					Traitement.ListOfTasks2CPUMachinej[iboucle].prio = Traitement.ListOfTasks2CPUMachinej[iboucle].prio + IB;
			}
			else{
				IB = 0;
				Traitement.ListOfTasks2CPUMachinej[iboucle].prio = Traitement.ListOfTasks2CPUMachinej[iboucle].prio + IB;
			}
			if(indice!=0){
				if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks2CPUMachinej[iboucle].IndiceVM].dureeExe < mt(Traitement.ListOfTasks2CPUMachinej[iboucle].IndiceVM)){
					if(u(Traitement.ListOfTasks2CPUMachinej[iboucle].IndiceVM,Traitement.ListOfIntervalles[indice-1].BorneSup)==0){
						int indiceInterval = indice-1;
						while(u(Traitement.ListOfTasks2CPUMachinej[iboucle].IndiceVM,Traitement.ListOfIntervalles[indiceInterval].BorneSup)==0){
							indiceInterval--;
						}
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInterval].BorneSup][Traitement.ListOfTasks2CPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -M();
						}
					}
					else{
						if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks2CPUMachinej[iboucle].IndiceVM].IndiceMachine == indiceServeur){
							WG = indiceServeur;
						}
						else{
						WG = -2*M();
						}
					}
				}
				else{
					for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks2CPUMachinej[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
						}
					}
					WG = M() -  MachineRecevoir;
				//printf("Valeur de WG : %d \n",WG);
				}
			}
			else{
				for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(Data.ListOfTasks[Traitement.ListOfTasks2CPUMachinej[iboucle].IndiceVM].LPreAssignement[iboucle2]==1){
							MachineRecevoir++;
							//printf("Machine Recevoir %d\n",MachineRecevoir);
						}
					}
					WG = M() -  MachineRecevoir;
			}
			//printf("Valeur de WG : %d \n",WG);
			//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfTasks1GPU[iboucle].prio);
			Traitement.ListOfTasks2CPUMachinej[iboucle].prio = Traitement.ListOfTasks2CPUMachinej[iboucle].prio + WG;
			//printf("priorite de la tache %d : %d \n",Traitement.ListOfTasks1GPU[iboucle].IndiceVM,Traitement.ListOfTasks1GPU[iboucle].prio);	
			for(int iboucle=2;iboucle<Traitement.NbRAMHDDCPUMachinej;iboucle++){
				if(Traitement.ListOfTasks2CPUMachinej[iboucle].prio > Traitement.ListOfTasks2CPUMachinej[iboucle-1].prio){
					RAMHDD memoire = Traitement.ListOfTasks2CPUMachinej[iboucle-1];
					Traitement.ListOfTasks2CPUMachinej[iboucle-1]=Traitement.ListOfTasks2CPUMachinej[iboucle];
					Traitement.ListOfTasks2CPUMachinej[iboucle]=memoire;
				}
			}
		}
	}
}

void Ordonnancement(unsigned int indice){
	//Traitement.NbServeurOn = 0;
	Traitement.ListOfNbServeurOn[indice].NbServeurOn = 0;
	int CPU = 0;
	int GPU = 0;
	int RAM = 0;
	int HDD = 0;
	int iboucleS = 0;
	int indiceTab = 0;
	bool tachepre = 0;
	for(iboucleS;iboucleS<M();iboucleS++){
			//printf("Le serveur utilisé est %d \n",Traitement.ListOfServer[iboucleS].IndiceServeur);
			int indiceS = Traitement.ListOfServer[iboucleS].IndiceServeur;
			//printf("Valeurs de caract de la machine %d : %d %d %d %d \n",indiceS,mg(indiceS),mc(indiceS),mh(indiceS),mr(indiceS));

			/*CPU = Data.ListOfMachines[indiceS].QtyCPU;
			GPU = Data.ListOfMachines[indiceS].QtyGPU;
			RAM = Data.ListOfMachines[indiceS].QtyRAM;
			HDD = Data.ListOfMachines[indiceS].QtyHDD;*/

			for(int temps = Traitement.ListOfIntervalles[indice].BorneInf;temps<=Traitement.ListOfIntervalles[indice].BorneSup;temps++){
				Traitement.ListOfServeurbis[temps][indiceS].CPU = Data.ListOfMachines[indiceS].QtyCPU;
				Traitement.ListOfServeurbis[temps][indiceS].GPU = Data.ListOfMachines[indiceS].QtyGPU;
				Traitement.ListOfServeurbis[temps][indiceS].RAM = Data.ListOfMachines[indiceS].QtyRAM;
				Traitement.ListOfServeurbis[temps][indiceS].HDD = Data.ListOfMachines[indiceS].QtyHDD;
			}
			
			if((Traitement.NbHDDRAMGPU != 0)||(Traitement.NbRAMHDDGPU != 0)){
				CalculPrioGPU(indiceS,indice);
				OrdoGPU(indice,indiceS);
			}

			if((Traitement.NbHDDRAMCPU != 0)||(Traitement.NbRAMHDDCPU!=0)){
				CalculPrioCPU(indiceS,indice);
				OrdoCPU(indice,indiceS);
			}		
		}

				if((Traitement.NbHDDRAMGPUPr != 0)||(Traitement.NbRAMHDDGPUPr != 0)){
					//OrdoGPUPr(indice,indiceS);
					OrdoTachePreSurServeurOn(indice);
				}

				if((Traitement.NbHDDRAMCPUPr != 0)||(Traitement.NbRAMHDDCPUPr != 0)){
					OrdoTachePreSurServeurOn(indice);
				}
			/*Data.ListOfMachines[indiceS].QtyCPU = CPU;
			Data.ListOfMachines[indiceS].QtyGPU = GPU;
			Data.ListOfMachines[indiceS].QtyRAM = RAM;
			Data.ListOfMachines[indiceS].QtyHDD = HDD;*/

}

/************************************************************************************/
// Function OrdoGPU
// Permet de construire l'ordonnancement des tâches avec des besoins GPU
// non préamtable 
/************************************************************************************/
void OrdoGPU(unsigned int indice,unsigned int indiceServeur){
			int iboucle1=0;
			int iboucle2=0;	
			int i;
			//affectation des tâches appartenant à la liste des tâches qui ont HDD > RAM
			for(iboucle1=0;iboucle1<Traitement.NbHDDRAMGPU;iboucle1++){
				if ((Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].affecter!=1)&&(Traitement.ListOfTasks1GPU[iboucle1].prio>=0)/*&&(Traitement.ListOfTasks1GPU[iboucle1].prio == max(Traitement.ListOfTasks1GPU[].prio))*/){
					if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].GPU>=ng(Traitement.ListOfTasks1GPU[iboucle1].IndiceVM)){
						if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU>=nc(Traitement.ListOfTasks1GPU[iboucle1].IndiceVM)){
							if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD>=nh(Traitement.ListOfTasks1GPU[iboucle1].IndiceVM)){
								if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM>=nr(Traitement.ListOfTasks1GPU[iboucle1].IndiceVM)){
									//if(CalculFesabiliteResau(iboucle1,indiceServeur) == true){
									for(int indiceVM = 0;indiceVM<N();indiceVM++){
										if(a(Traitement.ListOfTasks1GPU[iboucle1].IndiceVM,indiceVM)==1){
											if((Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].affecter==1)&&(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].IndiceMachine != indiceServeur)){
												if(CalculFesabiliteResau(Traitement.ListOfTasks1GPU[iboucle1].IndiceVM,indiceVM,Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].IndiceMachine,indiceServeur,indice)==0){
													//printf("Le reseau le permet pas\n");
													for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].IndiceMachine = -3;
														Traitement.ListOfOrdo[i][indiceVM].IndiceMachine = -1;
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
													}
												}
												MaJReseau(indiceVM,Traitement.ListOfTasks1GPU[iboucle1].IndiceVM, Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].IndiceMachine,indiceServeur,indice);
											}
										}
									}
									if((Data.ListOfTasks[Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indice-1].BorneInf] == 0)){
										int indiceInt = indice-1;
										while(Data.ListOfTasks[Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indiceInt].BorneInf] == 0){
											indiceInt--;
										}
										Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].dureeExe = Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInt].BorneSup][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].dureeExe;

									}
									if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].affecter!=1){
									for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].IndiceMachine = indiceServeur;
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].dureeExe=Traitement.ListOfOrdo[i-1][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].dureeExe + 1;
										Traitement.ListOfServer[indiceServeur].ON = 1;
										Traitement.ListOfServeurbis[i][indiceServeur].GPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].GPU - ng(Traitement.ListOfTasks1GPU[iboucle1].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].CPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU - nc(Traitement.ListOfTasks1GPU[iboucle1].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].HDD = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD - nh(Traitement.ListOfTasks1GPU[iboucle1].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].RAM = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM - nr(Traitement.ListOfTasks1GPU[iboucle1].IndiceVM);
										//printf("Duree execution tache %d : %d\n",Traitement.ListOfTasks1GPU[iboucle1].IndiceVM,Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].dureeExe);
									}
									//printf("Duree execution tache %d : %d\n",Traitement.ListOfTasks1GPU[iboucle1].IndiceVM,Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].dureeExe);
									/*if((Traitement.ListOfServerOn[Traitement.NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.NbServeurOn == 0)){
										Traitement.ListOfServerOn[Traitement.NbServeurOn].IndiceServeur = indiceServeur;
										Traitement.NbServeurOn++;
									}*/
									if((Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.ListOfNbServeurOn[indice].NbServeurOn == 0)){
										Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn].IndiceServeur = indiceServeur;
										Traitement.ListOfNbServeurOn[indice].NbServeurOn++;
									}
									
									//printf("La tache %d est affecter sur la machine %d \n",Traitement.ListOfTasks1GPU[iboucle1].IndiceVM,indiceServeur);//Traitement.ListOfOrdo[i][Traitement.ListOfTasks1[iboucle1].IndiceVM].IndiceMachine);
									//printf("Nouvelle valeurs de caract de la machine : %d %d %d %d \n",mg(indiceServeur),mc(indiceServeur),mh(indiceServeur),mr(indiceServeur));
									//}
									}
								}
									//else
										//printf("Pas possible d'ajouter cette machine, pas assez de RAM \n");
							}
								//else
									//printf("Pas possible d'ajouter cette machine, pas assez de HDD \n");								
						}
							//else
								//printf("Pas possible d'ajouter cette machine, pas assez de CPU \n");
					}
						//else
							//printf("Pas possible d'ajouter cette machine, pas assez de GPU \n");*/
				}

			}
			
			//affectation des tâches appartenant à la liste des tâches qui ont RAM > HDD
			for(iboucle2 = 0;iboucle2<Traitement.NbRAMHDDGPU;iboucle2++){
				if((Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks2GPU[iboucle2].IndiceVM].affecter!=1)&&(Traitement.ListOfTasks2GPU[iboucle2].prio>=0)){
					if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].GPU>=ng(Traitement.ListOfTasks2GPU[iboucle2].IndiceVM)){
						if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU>=nc(Traitement.ListOfTasks2GPU[iboucle2].IndiceVM)){
							if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM>=nr(Traitement.ListOfTasks2GPU[iboucle2].IndiceVM)){
								if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD>=nh(Traitement.ListOfTasks2GPU[iboucle2].IndiceVM)){
									for(int indiceVM = 0;indiceVM<N();indiceVM++){
										if(a(Traitement.ListOfTasks2GPU[iboucle2].IndiceVM,indiceVM)==1){
											//printf("test \n");
											if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].affecter==1){
												if(CalculFesabiliteResau(Traitement.ListOfTasks2GPU[iboucle1].IndiceVM,indiceVM,Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks2GPU[iboucle2].IndiceVM].IndiceMachine,indiceServeur,indice)==0){
													//printf("Le reseau le permet pas\n");
													for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks2GPU[iboucle2].IndiceVM].IndiceMachine = -3;
														Traitement.ListOfOrdo[i][indiceVM].IndiceMachine = -1;
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks2GPU[iboucle2].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
													}
												}
												MaJReseau(indiceVM,Traitement.ListOfTasks2GPU[iboucle1].IndiceVM, Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].IndiceMachine,indiceServeur,indice);
											}
										}
									}
									if((Data.ListOfTasks[Traitement.ListOfTasks2GPU[iboucle2].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indice-1].BorneInf] == 0)){
										int indiceInt = indice-1;
										while(Data.ListOfTasks[Traitement.ListOfTasks2GPU[iboucle2].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indiceInt].BorneInf] == 0){
											indiceInt--;
										}
										Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks2GPU[iboucle2].IndiceVM].dureeExe = Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInt].BorneSup][Traitement.ListOfTasks2GPU[iboucle2].IndiceVM].dureeExe;

									}
									for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks2GPU[iboucle2].IndiceVM].IndiceMachine = indiceServeur;
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks2GPU[iboucle2].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks2GPU[iboucle2].IndiceVM].dureeExe=Traitement.ListOfOrdo[i-1][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].dureeExe + 1;
										Traitement.ListOfServer[indiceServeur].ON = 1;
										Traitement.ListOfServeurbis[i][indiceServeur].GPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].GPU - ng(Traitement.ListOfTasks2GPU[iboucle2].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].CPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU - nc(Traitement.ListOfTasks2GPU[iboucle2].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].HDD = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD - nh(Traitement.ListOfTasks2GPU[iboucle2].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].RAM = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM - nr(Traitement.ListOfTasks2GPU[iboucle2].IndiceVM);
										//printf("Duree execution tache %d : %d\n",Traitement.ListOfTasks1GPU[iboucle1].IndiceVM,Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].dureeExe);
									}
										//printf("Duree execution tache %d : %d\n",Traitement.ListOfTasks1GPU[iboucle1].IndiceVM,Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks1GPU[iboucle1].IndiceVM].dureeExe);
										/*if((Traitement.ListOfServerOn[Traitement.NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.NbServeurOn == 0)){
											Traitement.ListOfServerOn[Traitement.NbServeurOn].IndiceServeur = indiceServeur;
											Traitement.NbServeurOn++;
										}*/
									if((Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.ListOfNbServeurOn[indice].NbServeurOn == 0)){
										Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn].IndiceServeur = indiceServeur;
										Traitement.ListOfNbServeurOn[indice].NbServeurOn++;
									}
									//Traitement.ListOfServerOn[iboucle2].IndiceServeur=indiceServeur;
									//Traitement.ListOfServerOn[iboucle2].CoutNorm = Traitement.ListOfServer[iboucle2].CoutNorm;
									//printf("La tache %d est affecter sur la machine %d \n",Traitement.ListOfTasks2GPU[iboucle2].IndiceVM,Traitement.ListOfOrdo[i][iboucle2].IndiceMachine);
									//printf("Nouvelle valeurs de caract de la machine : %d %d %d %d \n",mg(indiceServeur),mc(indiceServeur),mh(indiceServeur),mr(indiceServeur));
								}
								//else
									//printf("Pas possible d'ajouter cette machine, pas assez de HDD \n");
							}
							//else
								//printf("Pas possible d'ajouter cette machine, pas assez de RAM \n");
						}
						//else
							//printf("Pas possible d'ajouter cette machine, pas assez de CPU \n");
					}
					//else
						//printf("Pas possible d'ajouter cette machine, pas assez de GPU \n");
				}
			}
}

/************************************************************************************/
// Function OrdoCPU
// Permet de construire l'ordonnancement des tâches avec des besoins CPU
// non préamtable 
/************************************************************************************/
void OrdoCPU(unsigned int indice,unsigned int indiceServeur){
	int i;
	int iboucle3=0;
	int iboucle2=0;
	//affectation des tâches appartenant à la liste des tâches qui ont HDD > RAM
			for(iboucle3;iboucle3<Traitement.NbHDDRAMCPU;iboucle3++){
				if((Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].affecter!=1)&&(Traitement.ListOfTasks1CPU[iboucle3].prio>=0)){
						if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU>=nc(Traitement.ListOfTasks1CPU[iboucle3].IndiceVM)){
							if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD>=nh(Traitement.ListOfTasks1CPU[iboucle3].IndiceVM)){
								if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM>=nr(Traitement.ListOfTasks1CPU[iboucle3].IndiceVM)){
									for(int indiceVM = 0;indiceVM<N();indiceVM++){
										if(a(Traitement.ListOfTasks1CPU[iboucle3].IndiceVM,indiceVM)==1){
											//printf("test \n");
											if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].affecter==1){
												if(CalculFesabiliteResau(Traitement.ListOfTasks1CPU[iboucle3].IndiceVM,indiceVM,Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].IndiceMachine,indiceServeur,indice)==0){
													//printf("Le reseau le permet pas\n");
													for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].IndiceMachine = -1;
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
													}
												}
												MaJReseau(indiceVM,Traitement.ListOfTasks1CPU[iboucle3].IndiceVM, Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].IndiceMachine,indiceServeur,indice);
											}
										}
									}
									if((Data.ListOfTasks[Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indice-1].BorneInf] == 0)){
										int indiceInt = indice-1;
										while(Data.ListOfTasks[Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indiceInt].BorneInf] == 0){
											indiceInt--;
										}
										Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].dureeExe = Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInt].BorneSup][Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].dureeExe;

									}
									for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].IndiceMachine = indiceServeur;
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].dureeExe=Traitement.ListOfOrdo[i-1][Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].dureeExe + 1;
										Traitement.ListOfServer[indiceServeur].ON = 1;
										Traitement.ListOfServeurbis[i][indiceServeur].CPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU - nc(Traitement.ListOfTasks1CPU[iboucle3].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].HDD = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD - nh(Traitement.ListOfTasks1CPU[iboucle3].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].RAM = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM - nr(Traitement.ListOfTasks1CPU[iboucle3].IndiceVM);

										//printf("Duree execution tache %d : %d\n",Traitement.ListOfTasks1CPU[iboucle3].IndiceVM,Traitement.ListOfOrdo[i][Traitement.ListOfTasks1CPU[iboucle3].IndiceVM].dureeExe);
									}
									/*if((Traitement.ListOfServerOn[Traitement.NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.NbServeurOn == 0)){
											Traitement.ListOfServerOn[Traitement.NbServeurOn].IndiceServeur = indiceServeur;
											Traitement.NbServeurOn++;
									}*/
									if((Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.ListOfNbServeurOn[indice].NbServeurOn == 0)){
										Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn].IndiceServeur = indiceServeur;
										Traitement.ListOfNbServeurOn[indice].NbServeurOn++;
									}
									//printf("La tache %d est affecter sur la machine %d \n",Traitement.ListOfTasks1CPU[iboucle3].IndiceVM,indiceServeur);//Traitement.ListOfOrdo[i][Traitement.ListOfTasks1[iboucle1].IndiceVM].IndiceMachine);
									//printf("Nouvelle valeurs de caract de la machine : %d %d %d %d \n",mg(indiceServeur),mc(indiceServeur),mh(indiceServeur),mr(indiceServeur));
									}
									//else
										//printf("Pas possible d'ajouter cette machine, pas assez de RAM \n");
								}
								//else
									//printf("Pas possible d'ajouter cette machine, pas assez de HDD \n");								
							}
							//else
								//printf("Pas possible d'ajouter cette machine, pas assez de CPU \n");
					}

			}
			
			//affectation des tâches appartenant à la liste des tâches qui ont RAM > HDD
			for(iboucle2 = 0;iboucle2<Traitement.NbRAMHDDCPU;iboucle2++){
				if((Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].affecter!=1)&&(Traitement.ListOfTasks2CPU[iboucle2].prio>=0)){
						if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU>=nc(Traitement.ListOfTasks2CPU[iboucle2].IndiceVM)){
							if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM>=nr(Traitement.ListOfTasks2CPU[iboucle2].IndiceVM)){
								if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD>=nh(Traitement.ListOfTasks2CPU[iboucle2].IndiceVM)){
									for(int indiceVM = 0;indiceVM<N();indiceVM++){
										if(a(Traitement.ListOfTasks2CPU[iboucle2].IndiceVM,indiceVM)==1){
											//printf("test \n");
											if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].affecter==1){
												if(CalculFesabiliteResau(Traitement.ListOfTasks2CPU[iboucle3].IndiceVM,indiceVM,Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].IndiceMachine,indiceServeur,indice)==0){
													//printf("Le reseau le permet pas\n");
													for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].IndiceMachine = -1;
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
													}
												}
												MaJReseau(indiceVM,Traitement.ListOfTasks2CPU[iboucle2].IndiceVM, Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].IndiceMachine,indiceServeur,indice);
											}
										}
									}
									if((Data.ListOfTasks[Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indice-1].BorneInf] == 0)){
										int indiceInt = indice-1;
										while(Data.ListOfTasks[Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indiceInt].BorneInf] == 0){
											indiceInt--;
										}
										Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].dureeExe = Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInt].BorneSup][Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].dureeExe;

									}
									for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
											Traitement.ListOfOrdo[i][Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].IndiceMachine = indiceServeur;
											Traitement.ListOfOrdo[i][Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
											Traitement.ListOfOrdo[i][Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].dureeExe=Traitement.ListOfOrdo[i-1][Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].dureeExe + 1;
											Traitement.ListOfServer[indiceServeur].ON = 1;
											Traitement.ListOfServeurbis[i][indiceServeur].CPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU - nc(Traitement.ListOfTasks2CPU[iboucle2].IndiceVM);
											Traitement.ListOfServeurbis[i][indiceServeur].HDD = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD - nh(Traitement.ListOfTasks2CPU[iboucle2].IndiceVM);
											Traitement.ListOfServeurbis[i][indiceServeur].RAM = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM - nr(Traitement.ListOfTasks2CPU[iboucle2].IndiceVM);
											//printf("Duree execution tache %d : %d\n",Traitement.ListOfTasks2CPU[iboucle2].IndiceVM,Traitement.ListOfOrdo[i][Traitement.ListOfTasks2CPU[iboucle2].IndiceVM].dureeExe);
										}
									/*if((Traitement.ListOfServerOn[Traitement.NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.NbServeurOn == 0)){
											Traitement.ListOfServerOn[Traitement.NbServeurOn].IndiceServeur = indiceServeur;
											Traitement.NbServeurOn++;
									}*/
									if((Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.ListOfNbServeurOn[indice].NbServeurOn == 0)){
										Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn].IndiceServeur = indiceServeur;
										Traitement.ListOfNbServeurOn[indice].NbServeurOn++;
									}
									//Traitement.ListOfServerOn[iboucle2].IndiceServeur=indiceServeur;
									//Traitement.ListOfServerOn[iboucle2].CoutNorm = Traitement.ListOfServer[iboucle2].CoutNorm;
									//printf("La tache %d est affecter sur la machine %d \n",Traitement.ListOfTasks2CPU[iboucle2].IndiceVM,Traitement.ListOfOrdo[i][iboucle2].IndiceMachine);
									//printf("Nouvelle valeurs de caract de la machine : %d %d %d %d \n",mg(indiceServeur),mc(indiceServeur),mh(indiceServeur),mr(indiceServeur));
								}
								//else
									//printf("Pas possible d'ajouter cette machine, pas assez de HDD \n");
							}
							//else
								//printf("Pas possible d'ajouter cette machine, pas assez de RAM \n");
						}
						//else
							//printf("Pas possible d'ajouter cette machine, pas assez de CPU \n");
				}
			}
}

void OrdoTachePreSurServeurOn(unsigned int indice){
	int indiceListe3 = 0;
	int indiceListe4 = 0;
	Traitement.NbHDDRAMCPUMachinej = 0;
	Traitement.NbRAMHDDCPUMachinej = 0;
	
	for(int IndiceServeurON = 0;IndiceServeurON < Traitement.ListOfNbServeurOn[indice].NbServeurOn;IndiceServeurON++){
		OrdoGPUPr(indice,Traitement.ListOfServerOn[IndiceServeurON].IndiceServeur);
	}

	if((Traitement.NbPr > 0)&&(Traitement.ListOfNbServeurOn[indice].NbServeurOn<=M())){
		AllumageMachine(indice);
	}

	for(int IndiceServeurON = 0;IndiceServeurON < Traitement.ListOfNbServeurOn[indice].NbServeurOn;IndiceServeurON++){
		OrdoCPUPr(indice,Traitement.ListOfServerOn[IndiceServeurON].IndiceServeur);
	}

	if((Traitement.NbPr > 0)&&(Traitement.ListOfNbServeurOn[indice].NbServeurOn<=M())){
		AllumageMachine(indice);
	}
}

void OrdoGPUPr(unsigned int indice,unsigned int indiceServeur){
	int iboucle;
	int iboucle1;
	int indiceListe1 = 0;
	int indiceListe2 = 0;
	Traitement.NbHDDRAMGPUMachinej = 0;
	Traitement.NbRAMHDDGPUMachinej = 0;

	//Remplissage de la liste des taches préamtable ayant des besoins en terme de GPU et tel que les besoins HDD > RAM
	for(iboucle=0;iboucle<Traitement.NbServeurOn;iboucle++){
		//printf("Test \n");
		for(iboucle1=0;iboucle1<Traitement.NbHDDRAMGPUPr;iboucle1++){
			if(q(Traitement.ListOfTasks1GPUPr[iboucle1].IndiceVM,Traitement.ListOfServer[iboucle].IndiceServeur)==1){
				Traitement.ListeOfctij[iboucle1][iboucle].indiceVM = Traitement.ListOfTasks1GPUPr[iboucle1].IndiceVM;
				Traitement.ListeOfctij[iboucle1][iboucle].cout = CalculCoutAffectation(Traitement.ListOfTasks1GPUPr[iboucle1].IndiceVM,indiceServeur);	
			}
			else{
				Traitement.ListeOfctij[iboucle1][iboucle].indiceVM = Traitement.ListOfTasks1GPUPr[iboucle1].IndiceVM;
				Traitement.ListeOfctij[iboucle1][iboucle].cout = -1;

			}
		}
	}

	//Détermination de la machine j tel que le cout d'affectation soit minimum
	for(iboucle1=0;iboucle1<Traitement.NbHDDRAMGPUPr;iboucle1++){
		float coutMinij = Traitement.ListeOfctij[iboucle1][Traitement.ListOfServerOn[0].IndiceServeur].cout;
		if(Traitement.NbServeurOn > 1){
			for(iboucle=Traitement.ListOfServerOn[1].IndiceServeur;iboucle<Traitement.NbServeurOn;iboucle++){
				if((Traitement.ListeOfctij[iboucle1][iboucle].cout<coutMinij)&&(Traitement.ListeOfctij[iboucle1][iboucle].cout!=-1)){
					coutMinij = Traitement.ListeOfctij[iboucle1][iboucle].cout;
				}
	
			}
		}
		if(coutMinij == Traitement.ListeOfctij[iboucle1][indiceServeur].cout){
			if(Traitement.ListeOfctij[iboucle1][indiceServeur].cout < rho(Traitement.ListOfTasks1GPUPr[iboucle1].IndiceVM)){	
					Traitement.ListOfTasks1GPUMachinej[indiceListe1].IndiceVM = Traitement.ListOfTasks1GPUPr[iboucle1].IndiceVM;
					indiceListe1++;
					Traitement.NbHDDRAMGPUMachinej++;
			}
		}
		
	}

	//Remplissage de la liste des taches préamtable ayant des besoins en terme de GPU et tel que les besoins RAM > HDD
	for(iboucle=0;iboucle<Traitement.NbServeurOn;iboucle++){
		for(iboucle1=0;iboucle1<Traitement.NbRAMHDDGPUPr;iboucle1++){
			if(q(Traitement.ListOfTasks2GPUPr[iboucle1].IndiceVM,Traitement.ListOfServer[iboucle].IndiceServeur)==1){
				Traitement.ListeOfctij[Traitement.NbHDDRAMGPUMachinej+iboucle1][iboucle].indiceVM = Traitement.ListOfTasks2GPUPr[iboucle1].IndiceVM;
				Traitement.ListeOfctij[Traitement.NbHDDRAMGPUMachinej+iboucle1][iboucle].cout = CalculCoutAffectation(Traitement.ListOfTasks2GPUPr[iboucle1].IndiceVM,indiceServeur);
			}
			else{
				Traitement.ListeOfctij[Traitement.NbHDDRAMGPUMachinej+iboucle1][iboucle].indiceVM = Traitement.ListOfTasks2GPUPr[iboucle1].IndiceVM;
				Traitement.ListeOfctij[Traitement.NbHDDRAMGPUMachinej+iboucle1][iboucle].cout = -1;

			}
		}
	}

	for(iboucle1=0;iboucle1<Traitement.NbRAMHDDGPUPr;iboucle1++){
		float coutMinij = Traitement.ListeOfctij[Traitement.NbRAMHDDGPUPr+iboucle1][0].cout;
		if(Traitement.NbServeurOn != 1){
			for(iboucle=Traitement.ListOfServerOn[1].IndiceServeur;iboucle<Traitement.NbServeurOn;iboucle++){
				if((Traitement.ListeOfctij[Traitement.NbRAMHDDGPUPr+iboucle1][iboucle].cout<coutMinij)&&(Traitement.ListeOfctij[Traitement.NbRAMHDDGPUPr+iboucle1][iboucle].cout!=-1)){
					coutMinij = Traitement.ListeOfctij[Traitement.NbRAMHDDGPUPr+iboucle1][iboucle].cout;
				}

			}
		}
		if(coutMinij == Traitement.ListeOfctij[Traitement.NbRAMHDDGPUPr+iboucle1][indiceServeur].cout){
			if(Traitement.ListeOfctij[Traitement.NbRAMHDDGPUPr+iboucle1][indiceServeur].cout < rho(Traitement.ListOfTasks2GPUPr[iboucle1].IndiceVM)){	
					Traitement.ListOfTasks2GPUMachinej[indiceListe2].IndiceVM = Traitement.ListOfTasks2GPUPr[iboucle1].IndiceVM;
					indiceListe2++;
					Traitement.NbRAMHDDGPUMachinej++;
			}
		}
		
	}

	 CalculPrioGPUPr(indiceServeur,indice);
	 AffectationGPUPre(indice,indiceServeur);

	
}

void OrdoCPUPr(unsigned int indice,unsigned int indiceServeur){
	int iboucle;
	int iboucle1;
	int indiceListe1 = 0;
	int indiceListe2 = 0;
	Traitement.NbHDDRAMCPUMachinej = 0;
	Traitement.NbRAMHDDCPUMachinej = 0;
	//Remplissage de la liste des taches préamtable ayant des besoins uniquement en terme de CPU et tel que les besoins HDD > RAM
	for(iboucle=0;iboucle<Traitement.ListOfNbServeurOn[indice].NbServeurOn;iboucle++){
		for(iboucle1=0;iboucle1<Traitement.NbHDDRAMCPUPr;iboucle1++){
			if(q(Traitement.ListOfTasks1CPUPr[iboucle1].IndiceVM,Traitement.ListOfServer[iboucle].IndiceServeur)==1){
				Traitement.ListeOfctij[iboucle1][iboucle].indiceVM = Traitement.ListOfTasks1CPUPr[iboucle1].IndiceVM;
				Traitement.ListeOfctij[iboucle1][iboucle].cout = CalculCoutAffectation(Traitement.ListOfTasks1CPUPr[iboucle1].IndiceVM,indiceServeur);
			}
			else{
				Traitement.ListeOfctij[iboucle1][iboucle].indiceVM = Traitement.ListOfTasks1CPUPr[iboucle1].IndiceVM;
				Traitement.ListeOfctij[iboucle1][iboucle].cout = -1;

			}
		}
	}

	//Détermination de la machine j tel que le cout d'affectation soit minimum
	for(iboucle1=0;iboucle1<Traitement.NbHDDRAMCPUPr;iboucle1++){
		float coutMinij = Traitement.ListeOfctij[iboucle1][Traitement.ListOfServerOn[0].IndiceServeur].cout;
		if(Traitement.ListOfNbServeurOn[indice].NbServeurOn > 1){
			for(iboucle=Traitement.ListOfServerOn[(Traitement.ListOfServer[indiceServeur].IndiceServeur)+1].IndiceServeur;iboucle<Traitement.NbServeurOn;iboucle++){
				if((Traitement.ListeOfctij[iboucle1][iboucle].cout<coutMinij)&&(Traitement.ListeOfctij[iboucle1][iboucle].cout!=-1)){
					coutMinij = Traitement.ListeOfctij[iboucle1][iboucle].cout;
				}
	
			}
		}
		if(coutMinij == Traitement.ListeOfctij[iboucle1][indiceServeur].cout){
			if(Traitement.ListeOfctij[iboucle1][indiceServeur].cout < rho(Traitement.ListOfTasks1CPUPr[iboucle1].IndiceVM)){	
					Traitement.ListOfTasks1CPUMachinej[indiceListe1].IndiceVM = Traitement.ListOfTasks1CPUPr[iboucle1].IndiceVM;
					indiceListe1++;
					Traitement.NbHDDRAMCPUMachinej++;
			}
		}
		
	}

	//Remplissage de la liste des taches préamtable ayant des besoins uniquement en terme de CPU et tel que les besoins RAM > HDD
	for(iboucle=0;iboucle<Traitement.ListOfNbServeurOn[indice].NbServeurOn;iboucle++){
		for(iboucle1=0;iboucle1<Traitement.NbRAMHDDCPUPr;iboucle1++){
			if(q(Traitement.ListOfTasks2CPUPr[iboucle1].IndiceVM,Traitement.ListOfServer[iboucle].IndiceServeur)==1){
				Traitement.ListeOfctij[Traitement.NbHDDRAMCPUMachinej+iboucle1][iboucle].indiceVM = Traitement.ListOfTasks2CPUPr[iboucle1].IndiceVM;
				Traitement.ListeOfctij[Traitement.NbHDDRAMCPUMachinej+iboucle1][iboucle].cout = CalculCoutAffectation(Traitement.ListOfTasks2CPUPr[iboucle1].IndiceVM,indiceServeur);
			}
			else{
				Traitement.ListeOfctij[Traitement.NbHDDRAMGPUMachinej+iboucle1][iboucle].indiceVM = Traitement.ListOfTasks2CPUPr[iboucle1].IndiceVM;
				Traitement.ListeOfctij[Traitement.NbHDDRAMGPUMachinej+iboucle1][iboucle].cout = -1;

			}
		}
	}

	for(iboucle1=0;iboucle1<Traitement.NbRAMHDDGPUPr;iboucle1++){
		float coutMinij = Traitement.ListeOfctij[Traitement.NbHDDRAMCPUPr+iboucle1][0].cout;
		if(Traitement.ListOfNbServeurOn[indice].NbServeurOn != 1){
			for(iboucle=Traitement.ListOfServerOn[1].IndiceServeur;iboucle<Traitement.NbServeurOn;iboucle++){
				if((Traitement.ListeOfctij[Traitement.NbHDDRAMCPUPr+iboucle1][iboucle].cout<coutMinij)&&(Traitement.ListeOfctij[Traitement.NbHDDRAMCPUPr+iboucle1][iboucle].cout!=-1)){
					coutMinij = Traitement.ListeOfctij[Traitement.NbHDDRAMCPUPr+iboucle1][iboucle].cout;
				}

			}
		}
		if(coutMinij == Traitement.ListeOfctij[Traitement.NbRAMHDDCPUPr+iboucle1][indiceServeur].cout){
			if(Traitement.ListeOfctij[Traitement.NbRAMHDDCPUPr+iboucle1][indiceServeur].cout < rho(Traitement.ListOfTasks2CPUPr[iboucle1].IndiceVM)){	
					Traitement.ListOfTasks2CPUMachinej[indiceListe2].IndiceVM = Traitement.ListOfTasks2CPUPr[iboucle1].IndiceVM;
					indiceListe2++;
					Traitement.NbRAMHDDGPUMachinej++;
			}
		}
		
	}

	 CalculPrioCPUPr(indiceServeur,indice);
	 AffectationCPUPre(indice,indiceServeur);

	
}

void AllumageMachine(unsigned indice){
	int iboucle;
	//int iboucle1 = Traitement.NbServeurOn;
	int iboucle1 = 0;
	int indiceTab = 0;
	int MachineRallume;
	int gain;
	for(iboucle1;iboucle1<M();iboucle1++){
		if(Traitement.ListOfServer[iboucle1].ON == 0){
		Traitement.ListeOfgaj[indiceTab].indicePM = Traitement.ListOfServer[iboucle1].IndiceServeur;
		gain = 0;
		Traitement.ListeOfgaj[indiceTab].gain = gain;
		for(iboucle = 0; iboucle<Traitement.NbPr; iboucle++){
			if((q(Traitement.ListofTasksPr[iboucle].IndiceVM,Traitement.ListOfServer[iboucle1].IndiceServeur)==1)){
				//printf("La machine %d doit elle être rallume ?",Traitement.ListeOfgaj[indiceTab].indicePM);
				for(int t = Traitement.ListOfIntervalles[indice].BorneInf;t<=Traitement.ListOfIntervalles[indice].BorneSup;t++){
					gain = gain + beta(Traitement.ListOfServer[iboucle1].IndiceServeur);
				}
				/*printf("Cout d'affectation de la tache %d sur la machine %d : %f \n",Traitement.ListofTasksPr[iboucle].IndiceVM,Traitement.ListOfServer[iboucle1].IndiceServeur,CalculCoutAffectation(Traitement.ListofTasksPr[iboucle].IndiceVM,Traitement.ListOfServer[iboucle1].IndiceServeur));
				printf("Penalite de pas affecter la machine %d : %d \n",Traitement.ListofTasksPr[iboucle].IndiceVM,rho(Traitement.ListofTasksPr[iboucle].IndiceVM));*/
				if((CalculCoutAffectation(Traitement.ListofTasksPr[iboucle].IndiceVM,Traitement.ListOfServer[iboucle1].IndiceServeur))<rho(Traitement.ListofTasksPr[iboucle].IndiceVM)){
					/*if(Traitement.ListOfIntervalles[indice].BorneSup == Traitement.ListOfIntervalles[indice].BorneInf){
						gain = gain + ((CalculCoutAffectation(Traitement.ListofTasksPr[iboucle].IndiceVM,Traitement.ListOfServer[iboucle1].IndiceServeur))-rho(Traitement.ListofTasksPr[iboucle].IndiceVM));
					}
					else*/
						gain = gain + (Traitement.ListOfIntervalles[indice].BorneSup - Traitement.ListOfIntervalles[indice].BorneInf) * ((CalculCoutAffectation(Traitement.ListofTasksPr[iboucle].IndiceVM,Traitement.ListOfServer[iboucle1].IndiceServeur))-rho(Traitement.ListofTasksPr[iboucle].IndiceVM));
				}
			}
			else{
				gain = gain + 0;
			}
			
		}
		Traitement.ListeOfgaj[indiceTab].gain = Traitement.ListeOfgaj[indiceTab].gain + gain;
		//printf("Le gain de la machine %d est de %d \n",iboucle1,Traitement.ListeOfgaj[indiceTab].gain);
		indiceTab++;
	}
}

	/*for(int i = 0;i<indiceTab;i++){
		printf("gain de la machine %d : %f \n",Traitement.ListeOfgaj[i].indicePM,Traitement.ListeOfgaj[i].gain);
		
	}*/

	/*for(int i = 0;i<=indiceTab;i++){
		gaj swap;
		if((Traitement.ListeOfgaj[i].gain>Traitement.ListeOfgaj[i+1].gain)&&(Traitement.ListeOfgaj[i].gain<0)){
			swap = Traitement.ListeOfgaj[i];
			Traitement.ListeOfgaj[i]=Traitement.ListeOfgaj[i+1];
			Traitement.ListeOfgaj[i+1]=swap;
		}
	}*/
	if(Traitement.ListeOfgaj[0].gain < 0){
		MachineRallume = Traitement.ListeOfgaj[0].indicePM;
		MaJServeur(MachineRallume,indice);
		//Traitement.NbServeurOn++;
		Traitement.ListOfNbServeurOn[indice].NbServeurOn++;
		ConstructionListesTachePrMachineON(indice,MachineRallume);
		if((Traitement.NbHDDRAMGPUMachinej !=0)||(Traitement.NbRAMHDDGPUMachinej!=0)){
			AffectationGPUPre(indice,MachineRallume);
		}
		if((Traitement.NbHDDRAMCPUMachinej !=0)||(Traitement.NbRAMHDDCPUMachinej!=0)){
			AffectationCPUPre(indice,MachineRallume);
		}
	}
}

void AffectationGPUPre(unsigned int indice,unsigned int indiceServeur){
	int i;
	int indiceListe2 = 0;
	int iboucle3=0;
	int iboucle4=0;
	//affectation des tâches appartenant à la liste des tâches qui ont HDD > RAM
	for(iboucle3=0;iboucle3<Traitement.NbHDDRAMGPUMachinej;iboucle3++){
			if ((Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM].affecter!=1)/*&&(Traitement.ListOfTasks1GPU[iboucle1].prio == max(Traitement.ListOfTasks1GPU[].prio))*/){
				if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].GPU>=ng(Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM)){
					if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU>=nc(Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM)){
						if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD>=nh(Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM)){
							if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM>=nr(Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM)){
								/*for(int indiceVM = 0;indiceVM<N();indiceVM++){
										if(a(Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM,indiceVM)==1){
											if((Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].affecter==1)&&(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].IndiceMachine != indiceServeur)){
												if(CalculFesabiliteResau(Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM,indiceVM,Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM].IndiceMachine,indiceServeur,indice)==0){
													//printf("Le reseau le permet pas\n");
													for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM].IndiceMachine = -1;
														Traitement.ListOfOrdo[i][indiceVM].IndiceMachine = -1;
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
													}
												}
												MaJReseau(indiceVM,Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM, Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].IndiceMachine,indiceServeur,indice);
											}
										}
									}*/
								
								if((Data.ListOfTasks[Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indice-1].BorneInf] == 0)){
										int indiceInt = indice-1;
										while(Data.ListOfTasks[Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indiceInt].BorneInf] == 0){
											indiceInt--;
										}
										Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM].dureeExe = Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInt].BorneSup][Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM].dureeExe;

								}
								for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
									Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM].IndiceMachine = indiceServeur;
									Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
									Traitement.ListOfOrdo[i][Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM].dureeExe++;
									Traitement.ListOfServeurbis[i][indiceServeur].GPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].GPU - ng(Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM);
									Traitement.ListOfServeurbis[i][indiceServeur].CPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU - nc(Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM);
									Traitement.ListOfServeurbis[i][indiceServeur].HDD = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD - nh(Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM);
									Traitement.ListOfServeurbis[i][indiceServeur].RAM = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM - nr(Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM);
								}
									//printf("La tache %d est affecter sur la machine %d \n",Traitement.ListOfTasks1GPU[iboucle1].IndiceVM,indiceServeur);//Traitement.ListOfOrdo[i][Traitement.ListOfTasks1[iboucle1].IndiceVM].IndiceMachine);
									//printf("Nouvelle valeurs de caract de la machine : %d %d %d %d \n",mg(indiceServeur),mc(indiceServeur),mh(indiceServeur),mr(indiceServeur));
								}
									//else
										//printf("Pas possible d'ajouter cette machine, pas assez de RAM \n");
							}
								//else
									//printf("Pas possible d'ajouter cette machine, pas assez de HDD \n");								
						}
							//else
								//printf("Pas possible d'ajouter cette machine, pas assez de CPU \n");
					}
					else{
							//printf("Pas possible d'ajouter cette machine, pas assez de GPU \n");*/
						Traitement.ListofTasksPr[indiceListe2].IndiceVM = Traitement.ListOfTasks1GPUMachinej[iboucle3].IndiceVM;
						Traitement.NbPr++;
						indiceListe2++;
					}	
				}
			
			}

	for(iboucle4=0;iboucle4<Traitement.NbRAMHDDGPUMachinej;iboucle4++){
			if ((Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks2GPUMachinej[iboucle4].IndiceVM].affecter!=1)){
				if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].GPU>=ng(Traitement.ListOfTasks2GPUMachinej[iboucle4].IndiceVM)){
					if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU>=nc(Traitement.ListOfTasks2GPUMachinej[iboucle4].IndiceVM)){
						if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM>=nr(Traitement.ListOfTasks2GPUMachinej[iboucle4].IndiceVM)){
							if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD>=nh(Traitement.ListOfTasks2GPUMachinej[iboucle4].IndiceVM)){
									for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks2GPUMachinej[iboucle4].IndiceVM].IndiceMachine = indiceServeur;
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks2GPUMachinej[iboucle4].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks2GPUMachinej[iboucle4].IndiceVM].dureeExe++;
										Traitement.ListOfServeurbis[i][indiceServeur].GPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].GPU - ng(Traitement.ListOfTasks2GPUMachinej[iboucle3].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].CPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU - nc(Traitement.ListOfTasks2GPUMachinej[iboucle3].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].HDD = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD - nh(Traitement.ListOfTasks2GPUMachinej[iboucle3].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].RAM = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM - nr(Traitement.ListOfTasks2GPUMachinej[iboucle3].IndiceVM);
									}
									//printf("La tache %d est affecter sur la machine %d \n",Traitement.ListOfTasks1GPU[iboucle1].IndiceVM,indiceServeur);//Traitement.ListOfOrdo[i][Traitement.ListOfTasks1[iboucle1].IndiceVM].IndiceMachine);
									//printf("Nouvelle valeurs de caract de la machine : %d %d %d %d \n",mg(indiceServeur),mc(indiceServeur),mh(indiceServeur),mr(indiceServeur));
									}
									//else
										//printf("Pas possible d'ajouter cette machine, pas assez de RAM \n");
								}
								//else
									//printf("Pas possible d'ajouter cette machine, pas assez de HDD \n");								
							}
							//else
								//printf("Pas possible d'ajouter cette machine, pas assez de CPU \n");
						}
						else{
							//printf("Pas possible d'ajouter cette machine, pas assez de GPU \n");
							Traitement.ListofTasksPr[indiceListe2].IndiceVM = Traitement.ListOfTasks1GPUMachinej[iboucle4].IndiceVM;
							Traitement.NbPr++;
							indiceListe2++;
						}	
					}
			
	}
}

void AffectationCPUPre(unsigned int indice,unsigned int indiceServeur){
	int i;
	int iboucle3=0;
	int iboucle2=0;
	//affectation des tâches appartenant à la liste des tâches qui ont HDD > RAM
	for(iboucle3;iboucle3<Traitement.NbHDDRAMCPUMachinej;iboucle3++){
				if (Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM].affecter==0){
						if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU>=nc(Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM)){
							if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD>=nh(Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM)){
								if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM>=nr(Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM)){
									/*for(int indiceVM = 0;indiceVM<N();indiceVM++){
										if(a(Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM,indiceVM)==1){
											if((Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].affecter==1)&&(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].IndiceMachine != indiceServeur)){
												if(u(indiceVM,Traitement.ListOfIntervalles[indice].BorneInf)==1){
												if(CalculFesabiliteResau(Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM,indiceVM,Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneSup][Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM].IndiceMachine,indiceServeur,indice)==0){
													//printf("Le reseau le permet pas\n");
													for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM].IndiceMachine = -1;
														//Traitement.ListOfOrdo[i][indiceVM].IndiceMachine = -1;
														Traitement.ListOfOrdo[i][Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
													}
												}
												MaJReseau(indiceVM,Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM, Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].IndiceMachine,indiceServeur,indice);
											
												}
											}
										}
									}*/
										if((Data.ListOfTasks[Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indice-1].BorneInf] == 0)){
											int indiceInt = indice-1;
											while(Data.ListOfTasks[Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indiceInt].BorneInf] == 0){
												indiceInt--;
											}
											Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM].dureeExe = Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInt].BorneSup][Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM].dureeExe;

										}
									for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM].IndiceMachine = indiceServeur;
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM].affecter=1;//Permet de définir que la tâche est afecter sur une machine
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM].dureeExe++;
										Traitement.ListOfServer[indiceServeur].ON = 1;
										Traitement.ListOfServeurbis[i][indiceServeur].CPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU - nc(Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].HDD = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD - nh(Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].RAM = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM - nr(Traitement.ListOfTasks1CPUMachinej[iboucle3].IndiceVM);
									}
									/*if((Traitement.ListOfServerOn[Traitement.NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.NbServeurOn == 0)){
											Traitement.ListOfServerOn[Traitement.NbServeurOn].IndiceServeur = indiceServeur;
											Traitement.NbServeurOn++;
									}*/
									if((Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.ListOfNbServeurOn[indice].NbServeurOn == 0)){
										Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn].IndiceServeur = indiceServeur;
										Traitement.ListOfNbServeurOn[indice].NbServeurOn++;
									}
									//Traitement.ListOfServerOn[iboucle3].IndiceServeur=indiceServeur;
									//Traitement.ListOfServerOn[iboucle3].CoutNorm = Traitement.ListOfServer[iboucle3].CoutNorm;
									//printf("La tache %d est affecter sur la machine %d \n",Traitement.ListOfTasks1CPU[iboucle3].IndiceVM,indiceServeur);//Traitement.ListOfOrdo[i][Traitement.ListOfTasks1[iboucle1].IndiceVM].IndiceMachine);
									//printf("Nouvelle valeurs de caract de la machine : %d %d %d %d \n",mg(indiceServeur),mc(indiceServeur),mh(indiceServeur),mr(indiceServeur));
									}
									//else
										//printf("Pas possible d'ajouter cette machine, pas assez de RAM \n");
								}
								//else
									//printf("Pas possible d'ajouter cette machine, pas assez de HDD \n");								
							}
							//else
								//printf("Pas possible d'ajouter cette machine, pas assez de CPU \n");
					}

			}
			
			//affectation des tâches appartenant à la liste des tâches qui ont RAM > HDD
			for(iboucle2 = 0;iboucle2<Traitement.NbRAMHDDCPU;iboucle2++){
				if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM].affecter==0){
						if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU>=nc(Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM)){
							if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM>=nr(Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM)){
								if(Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD>=nh(Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM)){
										if((Data.ListOfTasks[Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indice-1].BorneInf] == 0)){
											int indiceInt = indice-1;
											while(Data.ListOfTasks[Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM].LIsToBeProcessed[Traitement.ListOfIntervalles[indiceInt].BorneInf] == 0){
												indiceInt--;
											}
											Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM].dureeExe = Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInt].BorneSup][Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM].dureeExe;

										}									
									for(i=Traitement.ListOfIntervalles[indice].BorneInf;i<=Traitement.ListOfIntervalles[indice].BorneSup;i++){
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM].IndiceMachine = indiceServeur;
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM].affecter=1;//Permet de définir que la tâche est affecter sur une machine
										Traitement.ListOfOrdo[i][Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM].dureeExe++;
										Traitement.ListOfServer[indiceServeur].ON = 1;
										Traitement.ListOfServeurbis[i][indiceServeur].CPU = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].CPU - nc(Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].HDD = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].HDD - nh(Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].RAM = Traitement.ListOfServeurbis[Traitement.ListOfIntervalles[indice].BorneInf][indiceServeur].RAM - nr(Traitement.ListOfTasks2CPUMachinej[iboucle2].IndiceVM);
									}
									/*if((Traitement.ListOfServerOn[Traitement.NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.NbServeurOn == 0)){
											Traitement.ListOfServerOn[Traitement.NbServeurOn].IndiceServeur = indiceServeur;
											Traitement.NbServeurOn++;
									}*/
									if((Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn-1].IndiceServeur != indiceServeur)||(Traitement.ListOfNbServeurOn[indice].NbServeurOn == 0)){
										Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn].IndiceServeur = indiceServeur;
										Traitement.ListOfNbServeurOn[indice].NbServeurOn++;
									}
									//Traitement.ListOfServerOn[iboucle2].IndiceServeur=indiceServeur;
									//Traitement.ListOfServerOn[iboucle2].CoutNorm = Traitement.ListOfServer[iboucle2].CoutNorm;
									//printf("La tache %d est affecter sur la machine %d \n",Traitement.ListOfTasks2CPU[iboucle2].IndiceVM,Traitement.ListOfOrdo[i][iboucle2].IndiceMachine);
									//printf("Nouvelle valeurs de caract de la machine : %d %d %d %d \n",mg(indiceServeur),mc(indiceServeur),mh(indiceServeur),mr(indiceServeur));
								}
								//else
									//printf("Pas possible d'ajouter cette machine, pas assez de HDD \n");
							}
							//else
								//printf("Pas possible d'ajouter cette machine, pas assez de RAM \n");
						}
						//else
							//printf("Pas possible d'ajouter cette machine, pas assez de CPU \n");
				}
			}
}

void ConstructionListesTachePrMachineON(unsigned int indice,unsigned int indiceServeur){
	Traitement.NbHDDRAMGPUMachinej = 0;
	Traitement.NbRAMHDDGPUMachinej = 0;
	Traitement.NbHDDRAMCPUMachinej = 0;
	Traitement.NbRAMHDDCPUMachinej = 0;
	int indiceListe1 = 0;
	int indiceListe2 = 0;
	int indiceListe3 = 0;
	int indiceListe4 = 0;
	for(int i=0;i<Traitement.NbPr;i++){
		if(ng(Traitement.ListofTasksPr[i].IndiceVM) != 0){
			if(nh(Traitement.ListofTasksPr[i].IndiceVM)>nr(Traitement.ListofTasksPr[i].IndiceVM)){
				Traitement.ListOfTasks1GPUMachinej[indiceListe1].IndiceVM = Traitement.ListofTasksPr[i].IndiceVM;
				Traitement.NbHDDRAMGPUMachinej++;
				indiceListe1++;
			}
			else{
				Traitement.ListOfTasks2GPUMachinej[indiceListe2].IndiceVM = Traitement.ListofTasksPr[i].IndiceVM;
				Traitement.NbRAMHDDGPUMachinej++;
				indiceListe2++;
			}
		}
		else if(ng(Traitement.ListofTasksPr[i].IndiceVM) == 0){
				//Construction de la liste HDD > RAM
				if(nh(Traitement.ListofTasksPr[i].IndiceVM)>nr(Traitement.ListofTasksPr[i].IndiceVM))
				{
					Traitement.ListOfTasks1CPUMachinej[indiceListe3].IndiceVM = Traitement.ListofTasksPr[i].IndiceVM;
					Traitement.NbHDDRAMCPUMachinej++;
					indiceListe3++;
				}
				//Construction de la liste RAM > HDD
				else
				{
					Traitement.ListOfTasks2CPUMachinej[indiceListe4].IndiceVM = Traitement.ListofTasksPr[i].IndiceVM;
					Traitement.NbRAMHDDCPUMachinej++;
					indiceListe4++;
				}

			}	
	
	
	}
	/*
	//printf("Nombre de machine dans liste HDD > RAM : %d \n",Traitement.NbHDDRAMGPUMachinej);
	//printf("Nombre de machine dans liste RAM > HDD : %d \n",Traitement.NbRAMHDDGPUMachinej);
	//Affichage des listes
	printf("\n **** Avec GPU **** \n");
	printf("\n **** Liste HDDRAM **** \n");
	for (int i=0; i<Traitement.NbHDDRAMGPUMachinej; i++){
		printf("%d \t",Traitement.ListOfTasks1GPUMachinej[i].IndiceVM);
	}
	printf("\n **** Liste RAMHDD **** \n");
	for (int i=0; i<Traitement.NbRAMHDDGPUMachinej; i++){
		printf("%d \t",Traitement.ListOfTasks2GPUMachinej[i].IndiceVM);
	}
	printf("\n **** Sans GPU **** \n");
	printf("\n **** Liste HDDRAM **** \n");
	for (int i=0; i<Traitement.NbHDDRAMCPUMachinej; i++){
		printf("%d \t",Traitement.ListOfTasks1CPUPr[i].IndiceVM);
	}
	printf("\n **** Liste RAMHDD **** \n");
	for (int i=0; i<Traitement.NbRAMHDDCPUMachinej; i++){
		printf("%d \t",Traitement.ListOfTasks2CPUPr[i].IndiceVM);
	}
	printf("\n");*/


}