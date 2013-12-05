#include "Data.h"
#include "Traitement.h"
#include "Test.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <algorithm>
#include <list>
#include <iostream>

Trait Traitement;


void Init()
{

	Traitement.NbNoPrAffected = 0;
	Traitement.NbPrAffected = 0;
	
	///La ListofServeurbis contient les caracs actuelles
	for(int indiceS = 0; indiceS<M(); indiceS++){
		for(int temps = 0;temps<= T();temps++){
			Traitement.ListOfServeurbis[temps][indiceS].CPU = Data.ListOfMachines[indiceS].QtyCPU;
			Traitement.ListOfServeurbis[temps][indiceS].GPU = Data.ListOfMachines[indiceS].QtyGPU;
			Traitement.ListOfServeurbis[temps][indiceS].RAM = Data.ListOfMachines[indiceS].QtyRAM;
			Traitement.ListOfServeurbis[temps][indiceS].HDD = Data.ListOfMachines[indiceS].QtyHDD;
		}
	}

	///
	///--------- Les structures de réseau--------------
	///
	unsigned int mi, mj;
	for (int iEdge=0; iEdge<NbEdges(); iEdge++)
	{
		Traitement.EdgeBdeDispo[iEdge] = maxb();
		for (int iLoop=0; iLoop<NbMachEdge(iEdge); iLoop++)
		{
			CoupleMachines(iEdge,iLoop,mi,mj);
			if (mi>mj)Swap(mi, mj);
			std::pair<int, int> couple(mi, mj);
			///update the set of edges
			Traitement.CoupleEdgeMap.insert( std::pair< std::pair<int, int>, std::set<int>>(couple, std::set<int>()));
		}
	}
}

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
// Calcule le cout normalis?des serveurs et classe la liste des serveurs par
// ordre croissant de leur cout normalis?
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
			//printf("nombre de machine allum?%d \n",Traitement.ListOfNbServeurOn[indice].NbServeurOn);
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
	if(a(machinei, machinej) != 1)return true;
	int iEdge,iTime;
	std::map< std::pair<int,int>, std::set<int>>::iterator it;
	if(machinei > machinej) Swap(machinei, machinej);
	///Get the edge set
	it = Traitement.CoupleEdgeMap.find( std::pair<int, int>(machinei, machinej));
	std::set<int> edgeSet = it->second;
	std::set<int>::iterator itEdge;
	///For each edge passed by i,j, the band width must be enough
	for(itEdge=edgeSet.begin(); itEdge != edgeSet.end(); itEdge++)
	{
		if( Traitement.EdgeBdeDispo[*itEdge]-b(machinei, machinej) < 0)
			return false;
	}
	return true;
}

void MaJReseau(unsigned tachei,unsigned tachej, unsigned machinei,unsigned machinej,unsigned int indice){
	if(a(machinei, machinej) != 1)return;
	int iEdge,iTime;
	std::map< std::pair<int,int>, std::set<int>>::iterator it;
	if(machinei > machinej) Swap(machinei, machinej);
	///Get the edge set
	it = Traitement.CoupleEdgeMap.find( std::pair<int, int>(machinei, machinej));
	std::set<int> edgeSet = it->second;
	std::set<int>::iterator itEdge;
	///For each edge passed by i,j, update the band width
	for(itEdge=edgeSet.begin(); itEdge != edgeSet.end(); itEdge++)
		Traitement.EdgeBdeDispo[*itEdge] -= b(machinei, machinej);
}

/************************************************************************************/
// Function ConstructionListesTache
// Permet de construire les listes des taches
/************************************************************************************/
void ConstructionListesTache(unsigned int indice){
	
	Traitement.NbHDDRAMGPU = 0; //nb de tâches avec des besoins en GPU/CPU et besoins HDD > besoins RAM
	Traitement.NbRAMHDDGPU = 0; //nb de tâches avec des besoins en GPU/CPU et besoins RAM > besoins HDD
	Traitement.NbHDDRAMCPU = 0;	//nb de tâches sans GPU et besoins HDD > besoins RAM
	Traitement.NbRAMHDDCPU = 0; //nb de tâches sans GPU et besoins RAM > besoins HDD
	Traitement.NbHDDRAMGPUPr = 0; 
	Traitement.NbRAMHDDGPUPr = 0; 
	Traitement.NbHDDRAMCPUPr = 0;	
	Traitement.NbRAMHDDCPUPr = 0;
	Traitement.NbPr = 0; //nb de tâches Préemptables

		int temps=Traitement.ListOfIntervalles[indice].BorneInf;
		for (int i=0; i<N();i++){
			//Construction des listes tâches non préemtable avec des besoins en GPU et CPU 
			if((Data.ListOfTasks[i].LIsToBeProcessed[temps]==1)){
				if(R(i) == 0) ///Pour les tâches non-pré
				{
					if(Data.ListOfTasks[i].QtyGPU>0)
					{
						//Construction de la liste HDD > RAM
						if(nh(i)>nr(i))
							Traitement.ListOfTasks1GPU[Traitement.NbHDDRAMGPU++].IndiceVM = i;
						else
							Traitement.ListOfTasks2GPU[Traitement.NbRAMHDDGPU++].IndiceVM = i;
					}else ///Pour les tâches de CPU 
					{
						//Construction de la liste HDD > RAM
						if(nh(i)>nr(i))
							Traitement.ListOfTasks1CPU[Traitement.NbHDDRAMCPU++].IndiceVM = i;
						else
							Traitement.ListOfTasks2CPU[Traitement.NbRAMHDDCPU++].IndiceVM = i;
					}
				}///fin if R=0
				else ///Pour les tâches pré
				{
					///La liste de toutes les tâches pré
					Traitement.ListofTasksPr[Traitement.NbPr++].IndiceVM = i;
					
					///Les 4 listes selon besoins
					if(Data.ListOfTasks[i].QtyGPU>0)
					{
						//Construction de la liste HDD > RAM
						if(nh(i)>nr(i))
							Traitement.ListOfTasks1GPUPr[Traitement.NbHDDRAMGPUPr++].IndiceVM = i;
						else
							Traitement.ListOfTasks2GPUPr[Traitement.NbRAMHDDGPUPr++].IndiceVM = i;
					}else ///Pour les tâches de CPU 
					{
						//Construction de la liste HDD > RAM
						if(nh(i)>nr(i))
							Traitement.ListOfTasks1CPUPr[Traitement.NbHDDRAMCPUPr++].IndiceVM = i;
						else
							Traitement.ListOfTasks2CPUPr[Traitement.NbRAMHDDCPUPr++].IndiceVM = i;
					}
				}/// fin tâches pré
			}///fin if u=1
		}
		AfficherListes();
}

///Pour un intervalle et un serveur donnés, calculer la prio des tâches qui peuvent être affectées sur cette machine
void CalculPrioEtTrier(Tache* listeTache, unsigned int nbTache, unsigned int indice,unsigned int indiceServeur){
	int IB = 0;
	int WG = 0;
	int indiceVM;
	int MachineRecevoir = 0;
	int duree; //La durée d'exécution de la tâche, c'est pour simplifier la notion.
	//Calcule de la prioriét?pour la liste contenant les VM ayant besoin de CPU et tel que besoins HDD > besoins RAM
	for(int iboucle = 0; iboucle<nbTache;iboucle++){ ///Pour chaque tâche
		listeTache[iboucle].prio = 0;
		IB = 0;
		WG = 0;
		MachineRecevoir = 0;
		indiceVM = listeTache[iboucle].IndiceVM;
		duree = GetDureeExeActuelle(indice, indiceVM); 

		///Si la tâche n'est pas encore affectée
		if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].affecter != 1){
			///Si cette tâche était affectée sur cette machine à l'intervalle précédent, alors elle a plus de prio
			if( duree!=0 && Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice-1].BorneSup][indiceVM].IndiceMachine == indiceServeur){IB = M();}

			///Pour WG. Si le temps d'exécution de la tâche ne permet pas de faire la migration, alors pas de choix...
			if( duree!=0 && duree < mt(indiceVM)){
				///On cherche l'intervalle où elle a exécuté.
				///Si sur l'intervalle trouvé la tâche été exécutée sur cette machine, alors un peu de priorité, sinon pas possible de la mettre sur cette machine.
				int indiceInterval = indice-1;
				while(indiceInterval>=0 && u(indiceVM,Traitement.ListOfIntervalles[indiceInterval].BorneSup)==0)
					indiceInterval--;
					
				if(indiceInterval != -1 && Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indiceInterval].BorneSup][indiceVM].IndiceMachine == indiceServeur){///Elle était là à l'époque...
					WG = indiceServeur;
				}
				else{
					WG = -M(); ///ça veut dire no way
				}
			}
			else{///dureeExe > mt ou duree==0
				for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
					if(q(indiceVM, iboucle2)==1){
						MachineRecevoir++;
					}
				}
				///Moins de récepteur, plus de prio
				WG = M() -  MachineRecevoir;
				//printf("Valeur de WG : %d \n",WG);
			}
			listeTache[iboucle].prio = IB + WG;
			//printf("priorite de la tache %d : %d \n",indiceVM,listeTache[iboucle].prio);	
		}
	}
	///Trier les tâches en priorité décroissant
	SortByPrio(listeTache, nbTache);
}

///(obsolète)Initialiser le champ dureeExe des tâche dans un intervalle donné avec les valeurs de l'intervalle précédent.
///Attention!!! Seulement les tâches dont u(indice)=1 sont comprises dans les listes. 
///Donc quand on met à jour le dureeExe pendant l'affection il faut aussi continuer 
///cette mise à jour de dureeExe pour l'intervalle suivant si dans ce dercier intervalle u()=0. 
///Sinon on aura des coupes pour la valeur dureeExe.
//void InitDureeExe(Tache* listeTache, unsigned int nbTache, unsigned int indiceI)
//{
//	for(int i=Traitement.ListOfIntervalles[indiceI].BorneInf; i<=Traitement.ListOfIntervalles[indiceI].BorneSup;i++)
//	{
//		for(int j=0; j<nbTache; j++)
//		{
//			if(i == 0)///Pour le premier instant, traitement spécial
//				Traitement.ListOfOrdo[i][listeTache[j].IndiceVM].dureeExe = 0;
//			else Traitement.ListOfOrdo[i][listeTache[j].IndiceVM].dureeExe = Traitement.ListOfOrdo[i-1][listeTache[j].IndiceVM].dureeExe;
//		}
//	}
//}


///Ordonnancer les tâches sur l'intervalle [indice]
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

	///! Maintenant on a déjà les listes de tâches, alors pour chaque machine, 
	///on essaie de la remplir par des tâches. 
	///Tâches non pré d'abord
	for(iboucleS=0;iboucleS<M();iboucleS++){
		int indiceS = Traitement.ListOfServer[iboucleS].IndiceServeur;
				
		CalculPrioEtTrier(Traitement.ListOfTasks1GPU, Traitement.NbHDDRAMGPU,indice, indiceS);
		CalculPrioEtTrier(Traitement.ListOfTasks2GPU, Traitement.NbRAMHDDGPU,indice, indiceS);
		CalculPrioEtTrier(Traitement.ListOfTasks1CPU, Traitement.NbHDDRAMCPU,indice, indiceS);
		CalculPrioEtTrier(Traitement.ListOfTasks2CPU, Traitement.NbRAMHDDCPU,indice, indiceS);
				
		OrdoNoPr(Traitement.ListOfTasks1GPU, Traitement.NbHDDRAMGPU,indice, indiceS, Traitement.NbNoPrAffected);
		OrdoNoPr(Traitement.ListOfTasks2GPU, Traitement.NbRAMHDDGPU,indice, indiceS, Traitement.NbNoPrAffected);
		OrdoNoPr(Traitement.ListOfTasks1CPU, Traitement.NbHDDRAMCPU,indice, indiceS, Traitement.NbNoPrAffected);
		OrdoNoPr(Traitement.ListOfTasks2CPU, Traitement.NbRAMHDDCPU,indice, indiceS, Traitement.NbNoPrAffected);
		
		///
		///Pour les tâches préemptables
		///D'abord on les traite comme pour les tâches préemptables
		///
		CalculPrioEtTrier(Traitement.ListOfTasks1GPUPr, Traitement.NbHDDRAMGPUPr,indice, indiceS);
		CalculPrioEtTrier(Traitement.ListOfTasks2GPUPr, Traitement.NbRAMHDDGPUPr,indice, indiceS);
		CalculPrioEtTrier(Traitement.ListOfTasks1CPUPr, Traitement.NbHDDRAMCPUPr,indice, indiceS);
		CalculPrioEtTrier(Traitement.ListOfTasks2CPUPr, Traitement.NbRAMHDDCPUPr,indice, indiceS);
				
		OrdoNoPr(Traitement.ListOfTasks1GPUPr, Traitement.NbHDDRAMGPUPr,indice, indiceS, Traitement.NbPrAffected, false);
		OrdoNoPr(Traitement.ListOfTasks2GPUPr, Traitement.NbRAMHDDGPUPr,indice, indiceS, Traitement.NbPrAffected, false);
		OrdoNoPr(Traitement.ListOfTasks1CPUPr, Traitement.NbHDDRAMCPUPr,indice, indiceS, Traitement.NbPrAffected, false);
		OrdoNoPr(Traitement.ListOfTasks2CPUPr, Traitement.NbRAMHDDCPUPr,indice, indiceS, Traitement.NbPrAffected, false);

		///Si toutes les tâche non pré sont déjà affectées alors on doit cesser cette boucle et traiter les tâches pré
		if(Traitement.NbNoPrAffected == N() - Traitement.NbPr)
			break;
	}

	///
	///Pour les tâches restantes préemptables
	int indiceAllume = -1;
	while(Traitement.NbPrAffected < Traitement.NbPr){
		///Rallumer les machines si besoin
		indiceAllume = AllumageMachine(indice);
		if( indiceAllume== -1) return; ///Aucunne machine peut être allumée
		else ///Affectation sur la machine allumée
		{
			///ConstructionListesTachePrMachineJ(indice, indiceAllume); c'est déjà fait.
			CalculPrioEtTrier(Traitement.ListOfTasks1GPUPr, Traitement.NbHDDRAMGPUPr,indice, indiceAllume);
			CalculPrioEtTrier(Traitement.ListOfTasks2GPUPr, Traitement.NbRAMHDDGPUPr,indice, indiceAllume);
			CalculPrioEtTrier(Traitement.ListOfTasks1CPUPr, Traitement.NbHDDRAMCPUPr,indice, indiceAllume);
			CalculPrioEtTrier(Traitement.ListOfTasks2CPUPr, Traitement.NbRAMHDDCPUPr,indice, indiceAllume);
				
			OrdoNoPr(Traitement.ListOfTasks1GPUPr, Traitement.NbHDDRAMGPUPr,indice, indiceAllume, Traitement.NbPrAffected, true);
			OrdoNoPr(Traitement.ListOfTasks2GPUPr, Traitement.NbRAMHDDGPUPr,indice, indiceAllume, Traitement.NbPrAffected, true);
			OrdoNoPr(Traitement.ListOfTasks1CPUPr, Traitement.NbHDDRAMCPUPr,indice, indiceAllume, Traitement.NbPrAffected, true);
			OrdoNoPr(Traitement.ListOfTasks2CPUPr, Traitement.NbRAMHDDCPUPr,indice, indiceAllume, Traitement.NbPrAffected, true);
		}
	}///Fin if affected < nbpr
}


/************************************************************************************/
///Ordonnacement des tâches non préamtables
///Prendre en compte la gestion réseau et aussi la mise à jour des serveurs et du réseau
/************************************************************************************/
void OrdoNoPr(Tache* listeTache, unsigned int nbTache, unsigned int indice,unsigned int indiceServeur, int & compteurAffect, bool canTurnOn){
			if(canTurnOn == false && Traitement.ListOfServer[indiceServeur].ON != 1)
				return;
	
			int i;
			int iboucle1=0;
			int iboucle2=0;
			int intervalInf = Traitement.ListOfIntervalles[indice].BorneInf;
			int intervalSup = Traitement.ListOfIntervalles[indice].BorneSup;
			int indiceVM, indiceVM2;
			
			//affectation des tâches appartenant à la liste des tâches qui ont HDD > RAM
			for(iboucle1=0;iboucle1<nbTache;iboucle1++){ ///Pour chaque tâche
				indiceVM = listeTache[iboucle1].IndiceVM;///Pour simplifier la vie

				///Si c'est pas encore affectée ///C'est pas très nécessaire ici car bien sûr c'est pas encore affectée.
				if (q(indiceVM, indiceServeur)==1 && 
					(Traitement.ListOfOrdo[intervalInf][indiceVM].affecter!=1)&&
					(listeTache[iboucle1].prio>=0)){
					
					///Si la machine a assez de ressource pour la recevoir
					if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].GPU>=ng(indiceVM)){
						if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].CPU>=nc(indiceVM)){
							if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].HDD>=nh(indiceVM)){
								if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].RAM>=nr(indiceVM)){
									
									///Si la contrainte réseau le permet (Gestion réseau pour deux VMs qui possèdent une affinité)
									for(indiceVM2 = 0; indiceVM2<N(); indiceVM2++){
										
										///Si on trouve une VM qui a une affinité de celle qu'on est en train de traiter
										if(a(indiceVM,indiceVM2)==1)
										{
											///Si l'autre VM est affectée pour cet intervalle et pas sur une autre machine
											if((Traitement.ListOfOrdo[intervalInf][indiceVM2].affecter==1)&&(Traitement.ListOfOrdo[intervalInf][indiceVM2].IndiceMachine != indiceServeur))
											{
												///Si le réseau ne permet pas cette affectation
												if(CalculFesabiliteResau(indiceVM,indiceVM2, ///tâche i et j
													indiceServeur,Traitement.ListOfOrdo[intervalSup][indiceVM2].IndiceMachine,indice)==0)///Machine i et j et intervalle
												{
													///Cette tâche ne peut pas être affectée sur cette machine. Mais on doit voir autres machine.
													Traitement.ListOfOrdo[intervalSup][indiceVM].affecter = 0;///Pour indiquer que c'est pas affectée.
													Traitement.ListOfOrdo[intervalInf][indiceVM].affecter = 0;
													indiceVM2 = -1; ///On va casser ce passage de boucle, continuer sur la tâche suivante à affecter.
													break;
												}
											}
										}
									}///Fin du parcours de tâches

									if(indiceVM2 == -1)continue; ///La tâche actuelle n'est pas permite par le réseau, donc on continue sur la tâche suivante.
									
									///Cette affectation est faisable, donc on met à jour le réseau ainsi que les carac de la machine
									MaJReseau(indiceVM, indiceVM2, indiceServeur, Traitement.ListOfOrdo[intervalInf][indiceVM2].IndiceMachine,indice);

									///Chercher la dureeExe actuelle
									int duree = GetDureeExeActuelle(indice, indiceVM);
									
									///Mettre à jour les attributs pour chaque intant de temps de l'intervalle
									for(i=intervalInf;i<=intervalSup;i++){
										if(i == intervalInf)///Pour le premier instant, traitement spécial
											Traitement.ListOfOrdo[i][indiceVM].dureeExe = duree + 1;
										else Traitement.ListOfOrdo[i][indiceVM].dureeExe = Traitement.ListOfOrdo[i-1][indiceVM].dureeExe+1;
										
										if(Traitement.ListOfServer[indiceServeur].ON != 1)
										{
											Traitement.ListOfServer[indiceServeur].ON = 1;
											///Ajouter cette machine à la liste des machines ON
											Traitement.ListOfServerOn[Traitement.ListOfNbServeurOn[indice].NbServeurOn].IndiceServeur = indiceServeur;
											Traitement.ListOfNbServeurOn[indice].NbServeurOn++;
										}

										Traitement.ListOfOrdo[i][indiceVM].IndiceMachine = indiceServeur;
										Traitement.ListOfOrdo[i][indiceVM].affecter=1;
										///Mettre à jour les ressources de la machine
										Traitement.ListOfServeurbis[i][indiceServeur].GPU = Traitement.ListOfServeurbis[intervalInf][indiceServeur].GPU - ng(indiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].CPU = Traitement.ListOfServeurbis[intervalInf][indiceServeur].CPU - nc(indiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].HDD = Traitement.ListOfServeurbis[intervalInf][indiceServeur].HDD - nh(indiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].RAM = Traitement.ListOfServeurbis[intervalInf][indiceServeur].RAM - nr(indiceVM);
									}
									///Mettre à jour le compteur
									compteurAffect++ ;

								}//else printf("Pas possible d'ajouter cette machine, pas assez de RAM \n");
							}//else printf("Pas possible d'ajouter cette machine, pas assez de HDD \n");								
						}//else printf("Pas possible d'ajouter cette machine, pas assez de CPU \n");
					}//else printf("Pas possible d'ajouter cette machine, pas assez de GPU \n");*/
				}//else cette tâche est déjà affectée
			}			
}


///-------------------------------------
///Chercher la duree d'exécution que la tâche a déjà effectué avant l'intervalle [indice].
///
///-------------------------------------
int GetDureeExeActuelle(unsigned int indice, unsigned int indiceVM)
{
	int duree = 0;
	int i= Traitement.ListOfIntervalles[indice].BorneInf-1;
	while(i>=0)
	{
		if(Traitement.ListOfOrdo[i][indiceVM].dureeExe > 0)
		{
				duree = Traitement.ListOfOrdo[i][indiceVM].dureeExe;
				break;
		}
		i--;
	}
	return duree;
}

///Allumer une machine et faire l'affectation au dessus.
int AllumageMachine(unsigned indice){
	int indiceVM = -1;
	int indiceServeur = -1;
	int MachineRallume = -1;
	 
	///C'est plutôt la différence entre le coût de rallumer la machine et le coût de ne pas la rallumer. Plus c'est moins, mieux c'est
	int gain = 0, gainMin = 0;
	int intervalInf = Traitement.ListOfIntervalles[indice].BorneInf;
	int intervalSup = Traitement.ListOfIntervalles[indice].BorneSup;
	///On va travailler dans la liste triée des machines
	for(int iboucle1 = 0; iboucle1<M();iboucle1++){
		if(Traitement.ListOfServer[iboucle1].ON == 0){
			indiceServeur = Traitement.ListOfServer[iboucle1].IndiceServeur;
			
			///Le coût d'être ON
			gain = (intervalSup - intervalInf + 1 ) * beta(indiceServeur);
			for(int iboucle2 = 0; iboucle2<Traitement.NbPr; iboucle2++){
				indiceVM = Traitement.ListofTasksPr[iboucle2].IndiceVM;
				///Voir si la tâche et la machine sont compatibles
				if(Traitement.ListOfOrdo[intervalInf][indiceVM].affecter!=1
					&&(q(indiceVM,indiceServeur)==1) 
					&&(Traitement.ListOfServeurbis[intervalInf][indiceServeur].GPU>=ng(indiceVM)) ///Ici est pour être sûr que si la machine être allumée, elle peut au moins recevoir une tâche
					&&(Traitement.ListOfServeurbis[intervalInf][indiceServeur].CPU>=nc(indiceVM))
					&&(Traitement.ListOfServeurbis[intervalInf][indiceServeur].HDD>=nh(indiceVM))
					&&(Traitement.ListOfServeurbis[intervalInf][indiceServeur].RAM>=nr(indiceVM))
				)
				{
					///le coût d'affection - le coût de suspension
					gain += (intervalSup - intervalInf + 1 ) * CalculCoutAffectation(indiceVM, indiceServeur) - rho(indiceVM);
				}
			}
			if( gain < gainMin)
			{
				gainMin = gain;
				MachineRallume = indiceServeur;
			}
		}///Fin if current server off
	}///Fin boucle de toutes les machines

	///MaJServeur(MachineRallume,indice); //C'est déjà fait dans Ordonnancement
	if(gainMin < 0) return MachineRallume;
	else return -1;
}
