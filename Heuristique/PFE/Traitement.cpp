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
	///Init ListOfOrdo
	for(int i=0;i<T();i++){
		for(int j=0;j<N();j++){
			Traitement.ListOfOrdo[i][j].affecter = 0;
			Traitement.ListOfOrdo[i][j].isMigrated = false;
			Traitement.ListOfOrdo[i][j].IndiceMachine=-1;
			Traitement.ListOfOrdo[i][j].IndiceTache=j;
			Traitement.ListOfOrdo[i][j].dureeExe=0;
			Traitement.ListOfOrdo[i][j].dureeSus=0;
			if(u(j,i)==0){
				Traitement.ListOfOrdo[i][j].IndiceMachine=-2;
			}
		}
	}

	///
	Traitement.NbServeurOn = 0;
	
	///La ListofServeurbis contient les caracs résiduelles
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
	std::map< std::pair<int,int>, std::set<int>>::iterator it;
	for (int iEdge=0; iEdge<NbEdges(); iEdge++)
	{
		for(int temps = 0;temps<= T();temps++){
			Traitement.EdgeBdeDispo[temps][iEdge] = maxb();
		}
		for (int iLoop=0; iLoop<NbMachEdge(iEdge); iLoop++)
		{
			CoupleMachines(iEdge,iLoop,mi,mj);
			if (mi>mj) Swap(mi, mj);
			std::pair<int, int> couple(mi, mj);
			///update the set of edges in the map. 
			///The key of the map is the machine couple, and the value is a set of edges.
			///First of all get the corresponding set
			it = Traitement.CoupleEdgeMap.insert( std::pair< 
				std::pair<int, int>, std::set<int>>(
				couple,              std::set<int>())).first;
			///Then update the set
			(it->second).insert(iEdge);
		}
	}
}

/************************************************************************************/
// Function CalculInterval
// Calcule les intervalles tel que sur [T,T+1], les u(i,t) sont consants
/************************************************************************************/
void CalculInterval(){
	Traitement.NbInterval = 0;
	Traitement.ListOfIntervalles[0].BorneInf = 0;
	Traitement.ListOfIntervalles[0].BorneSup = 0;
	for(int instant=0;instant<T()-1;instant++){
		for(int tacheI=0;tacheI<N();tacheI++){
			if(( u(tacheI,instant)!=u(tacheI,instant+1))){///intervalle trouvé
					Traitement.NbInterval++;
					Traitement.ListOfIntervalles[Traitement.NbInterval-1].BorneSup = instant;
					Traitement.ListOfIntervalles[Traitement.NbInterval].BorneInf = instant + 1;
					break;
			}			
		}
	}
	///Traiter le dernier instant	
	Traitement.NbInterval++;
	Traitement.ListOfIntervalles[Traitement.NbInterval-1].BorneSup = T()-1;
	
	//Affichage des intervalles calculés
	printf("Nombre d'interval : %d\n",Traitement.NbInterval);
	for(int i=0;i<Traitement.NbInterval;i++){
		printf("intervalle %d : [%d,%d] \n",i,Traitement.ListOfIntervalles[i].BorneInf,Traitement.ListOfIntervalles[i].BorneSup);
	}
}

/************************************************************************************/
// Calcule le cout normalis?des serveurs et classe la liste des serveurs par
// ordre croissant de leur cout normalis?
/************************************************************************************/
void CreerListeMachineTriee(){

	int i;
	int j=0;
	bool trier =0;
	float CoutTotal;
	float Cout;
	for(i=0; i<M(); i++){
		Traitement.ListOfServer[i].IndiceServeur = i;
		CoutTotal = (mc(i)*alphac(i) + mg(i)*alphag(i) + mr(i)*alphar(i) + mh(i)*alphah(i));
		Traitement.ListOfServer[i].CoutNorm = CoutTotal/(mc(i)+mg(i)+mr(i)+mh(i));
	}
	SortServerList(Traitement.ListOfServer, M());
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
	int CoutMigration = 0;
	int CoutAffect = 0;
	int CoutUnitaire = 0;
	int penality = 0;
	int temps = 0;
	//LoadOrdo();
	AfficherListeOrdo();
	for(int t=0;t<T();t++){
		printf("\nT = %d:\n",t);
		for(int n=0;n<N();n++){
			if(Traitement.ListOfOrdo[t][n].affecter==1){
				printf("%d\t", Traitement.ListOfOrdo[t][n].IndiceMachine);
				CoutAffect += CalculCoutAffectation( n, Traitement.ListOfOrdo[t][n].IndiceMachine);
				if(Traitement.ListOfOrdo[t][n].isMigrated)
					CoutMigration += mt(n)*(alphar(Traitement.ListOfOrdo[t][n].IndiceMachine)*nr(n)+alphah(Traitement.ListOfOrdo[t][n].IndiceMachine)*nh(n));
			}
			else if(u(n,t)==1&& R(n)==1)
			{
				printf("#\t");
				penality += rho(n);
			}
			else printf("*\t");
		}
	}
	for(int indice = 0;indice< Traitement.NbInterval;indice++){
		int nbON = Traitement.ListOfNbServeurOn[indice].NbServeurOn;
		//nbON = 1.8;
		for(int t=Traitement.ListOfIntervalles[indice].BorneInf; t<=Traitement.ListOfIntervalles[indice].BorneSup; t++)
			CoutUnitaire += (nbON * beta(t));
	}
	//printf("penalite total : %d \n",penality);
	printf("CoutUnitaire total :%d \n",CoutUnitaire);
	

	TotalCost = CoutAffect + CoutUnitaire + penality + CoutMigration;
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
	if( tachei == tachej && machinei == machinej) return true;
	if(a(tachei, tachej) != 1)return true;
	int iEdge,iTime;
	std::map< std::pair<int,int>, std::set<int>>::iterator it;
	if(machinei > machinej) Swap(machinei, machinej);
	///Get the edge set
	it = Traitement.CoupleEdgeMap.find( std::pair<int, int>(machinei, machinej));
	///Ces 2 machines ne sont pas connectées.
	if(it==Traitement.CoupleEdgeMap.end())
		return false;
	std::set<int> edgeSet = it->second;
	std::set<int>::iterator itEdge;

	if( tachei == tachej) ///Case of migration
	{
		///For each edge passed by the migration flux, the band width must be enough
		int interInf = Traitement.ListOfIntervalles[indice].BorneInf;
		int debutMigration = interInf - mt(tachei);
		for(itEdge=edgeSet.begin(); itEdge != edgeSet.end(); itEdge++)
		{
			for(int i = interInf-1; i>= debutMigration; i--)
			{
				if( Traitement.EdgeBdeDispo[i][*itEdge]-b(tachei, tachei) < 0)
					return false;
				else
					Traitement.EdgeBdeDispo[i][*itEdge] -= b(tachei, tachei);
			}
		}
	}else  ///Case of affinity
	{
		///For each edge passed by i,j, the band width must be enough
		for(itEdge=edgeSet.begin(); itEdge != edgeSet.end(); itEdge++)
		{
			if( Traitement.EdgeBdeDispo[Traitement.ListOfIntervalles[indice].BorneInf][*itEdge]-b(tachei, tachej) < 0)
				return false;
			else
				Traitement.EdgeBdeDispo[Traitement.ListOfIntervalles[indice].BorneInf][*itEdge] -= b(tachei, tachej);
		}
	}
	return true;
}

void MaJReseau(unsigned tachei,unsigned tachej, unsigned machinei,unsigned machinej,unsigned int indice){
	//if(a(tachei, tachej) != 1)return;
	//int iEdge,iTime;
	//std::map< std::pair<int,int>, std::set<int>>::iterator it;
	//if(machinei > machinej) Swap(machinei, machinej);
	/////Get the edge set
	//it = Traitement.CoupleEdgeMap.find( std::pair<int, int>(machinei, machinej));
	//assert(it != Traitement.CoupleEdgeMap.end());
	//std::set<int> edgeSet = it->second;
	//std::set<int>::iterator itEdge;
	/////int s = edgeSet.size();
	/////printf("%d,", s);
	/////For each edge passed by i,j, update the band width
	//for(itEdge=edgeSet.begin(); itEdge != edgeSet.end(); itEdge++)
	//	Traitement.EdgeBdeDispo[*itEdge] -= b(tachei, tachej);
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
			if((u(i,temps)==1)){
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
		AfficherListes(indice);
}


///Ordonnancer les tâches sur l'intervalle [indice]
void Ordonnancement(unsigned int indice){
	int CPU = 0;
	int GPU = 0;
	int RAM = 0;
	int HDD = 0;
	int iboucleS = 0;
	int indiceTab = 0;

	Traitement.ListOfNbServeurOn[indice].NbServeurOn = 0;
	Traitement.NbNoPrAffected = 0;
	Traitement.NbPrAffected = 0;

	///! Maintenant on a déjà les listes de tâches, alors pour chaque machine, 
	///on essaie de la remplir par des tâches.
	///Tâches non pré d'abord
	for(iboucleS=0;iboucleS<M();iboucleS++){
		int indiceS = Traitement.ListOfServer[iboucleS].IndiceServeur;
		Traitement.ListOfServer[iboucleS].ON = false;	///Pour calculer le NbServeurOn il faut d'abord éteindre la machine	
		CalculPrioEtTrier(Traitement.ListOfTasks1GPU, Traitement.NbHDDRAMGPU,indice, indiceS);
		CalculPrioEtTrier(Traitement.ListOfTasks2GPU, Traitement.NbRAMHDDGPU,indice, indiceS);
		CalculPrioEtTrier(Traitement.ListOfTasks1CPU, Traitement.NbHDDRAMCPU,indice, indiceS);
		CalculPrioEtTrier(Traitement.ListOfTasks2CPU, Traitement.NbRAMHDDCPU,indice, indiceS);
				
		OrdoListeTache(Traitement.ListOfTasks1GPU, Traitement.NbHDDRAMGPU,indice, iboucleS, Traitement.NbNoPrAffected);
		OrdoListeTache(Traitement.ListOfTasks2GPU, Traitement.NbRAMHDDGPU,indice, iboucleS, Traitement.NbNoPrAffected);
		OrdoListeTache(Traitement.ListOfTasks1CPU, Traitement.NbHDDRAMCPU,indice, iboucleS, Traitement.NbNoPrAffected);
		OrdoListeTache(Traitement.ListOfTasks2CPU, Traitement.NbRAMHDDCPU,indice, iboucleS, Traitement.NbNoPrAffected);
		
		if(Traitement.ListOfServer[iboucleS].ON == true)
		{
			///
			///Pour les tâches préemptables
			///D'abord on les traite comme pour les tâches préemptables
			///
			CalculPrioEtTrier(Traitement.ListOfTasks1GPUPr, Traitement.NbHDDRAMGPUPr,indice, indiceS);
			CalculPrioEtTrier(Traitement.ListOfTasks2GPUPr, Traitement.NbRAMHDDGPUPr,indice, indiceS);
			CalculPrioEtTrier(Traitement.ListOfTasks1CPUPr, Traitement.NbHDDRAMCPUPr,indice, indiceS);
			CalculPrioEtTrier(Traitement.ListOfTasks2CPUPr, Traitement.NbRAMHDDCPUPr,indice, indiceS);
				
			OrdoListeTache(Traitement.ListOfTasks1GPUPr, Traitement.NbHDDRAMGPUPr,indice, iboucleS, Traitement.NbPrAffected);
			OrdoListeTache(Traitement.ListOfTasks2GPUPr, Traitement.NbRAMHDDGPUPr,indice, iboucleS, Traitement.NbPrAffected);
			OrdoListeTache(Traitement.ListOfTasks1CPUPr, Traitement.NbHDDRAMCPUPr,indice, iboucleS, Traitement.NbPrAffected);
			OrdoListeTache(Traitement.ListOfTasks2CPUPr, Traitement.NbRAMHDDCPUPr,indice, iboucleS, Traitement.NbPrAffected);
		}
	}

	///
	///Pour les tâches restantes préemptables
	int indiceAllume = -1;
	int debutIndiceMachine = 0;
	while(Traitement.NbPrAffected < Traitement.NbPr){
		///Rallumer les machines si besoin
		indiceAllume = AllumageMachine(indice, debutIndiceMachine);
		if( indiceAllume== -1) break; ///Aucunne machine peut être allumée
		else ///Affectation sur la machine allumée
		{
			///ConstructionListesTachePrMachineJ(indice, indiceAllume); c'est déjà fait.
			CalculPrioEtTrier(Traitement.ListOfTasks1GPUPr, Traitement.NbHDDRAMGPUPr,indice, indiceAllume);
			CalculPrioEtTrier(Traitement.ListOfTasks2GPUPr, Traitement.NbRAMHDDGPUPr,indice, indiceAllume);
			CalculPrioEtTrier(Traitement.ListOfTasks1CPUPr, Traitement.NbHDDRAMCPUPr,indice, indiceAllume);
			CalculPrioEtTrier(Traitement.ListOfTasks2CPUPr, Traitement.NbRAMHDDCPUPr,indice, indiceAllume);
				
			OrdoListeTache(Traitement.ListOfTasks1GPUPr, Traitement.NbHDDRAMGPUPr,indice, indiceAllume, Traitement.NbPrAffected);
			OrdoListeTache(Traitement.ListOfTasks2GPUPr, Traitement.NbRAMHDDGPUPr,indice, indiceAllume, Traitement.NbPrAffected);
			OrdoListeTache(Traitement.ListOfTasks1CPUPr, Traitement.NbHDDRAMCPUPr,indice, indiceAllume, Traitement.NbPrAffected);
			OrdoListeTache(Traitement.ListOfTasks2CPUPr, Traitement.NbRAMHDDCPUPr,indice, indiceAllume, Traitement.NbPrAffected);
		}
		if( Traitement.ListOfServer[indiceAllume].ON == false)
			debutIndiceMachine = indiceAllume;
	}///Fin if affected < nbpr

	///Calcul nb total de serveur on.
	Traitement.NbServeurOn += Traitement.ListOfNbServeurOn[indice].NbServeurOn;
}


///Pour un intervalle et un serveur donnés, calculer la prio des tâches qui peuvent être affectées sur cette machine
void CalculPrioEtTrier(Tache* listeTache, unsigned int nbTache, unsigned int indice,unsigned int indiceServeur){
	int IB = 0;
	int WG = 0;
	int indiceVM;
	int lastIndiceInterval, lastIndiceServeur, duree;
	int MachineRecevoir = 0;
	for(int iboucle = 0; iboucle<nbTache;iboucle++){ ///Pour chaque tâche
		listeTache[iboucle].prio = 0;
		IB = 0;
		WG = 0;
		MachineRecevoir = 0;
		indiceVM = listeTache[iboucle].IndiceVM;
		//duree = GetDureeExeActuelle(indice, indiceVM); 
		LastExecution(indice, indiceVM, lastIndiceInterval, lastIndiceServeur, duree);

		///Si la tâche n'est pas encore affectée
		if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].affecter != 1){
			if(q(indiceVM, indiceServeur)== false)
			{
				listeTache[iboucle].prio = -M();
				continue;
			}

			///Si cette tâche était affectée sur cette machine à l'intervalle précédent, alors elle a plus de prio
			if( duree!=0 && lastIndiceInterval == indice-1 && lastIndiceServeur == indiceServeur){IB = M();}

			///Pour WG. 
			if(duree==0) ///Si la tâche n'a jamais été exécutée
			{
				for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
					if(q(indiceVM, iboucle2)==1){
						MachineRecevoir++;
					}
				}
				///Moins de récepteur, plus de prio
				WG = M() -  MachineRecevoir;
			}
			///Si elle a été executée sur cette machine et suspendue
			else if( lastIndiceServeur == indiceServeur && lastIndiceInterval < indice-1)
			{
				int inf = Traitement.ListOfIntervalles[indice].BorneInf;
				int lastInstant = Traitement.ListOfIntervalles[lastIndiceInterval].BorneSup;
				if( inf > lastInstant + rt(indiceVM, indiceServeur))
					WG = indiceServeur;
				else WG = -2 * M(); ///Pas possible de réveiller
			}
			///Migration nécessaire
			else if(lastIndiceServeur != indiceServeur )
			{ 
				///Migration pas possible
				if(duree < mt(indiceVM))
					WG = -2*M(); ///On ne va pas affecter les tâches qui a une prio négative
				else ///Migration possible mais à vérifier nh et nr et bii à l'affectation
				{
					for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(q(indiceVM, iboucle2)==1){
							MachineRecevoir++;
						}
					}
					///Moins de récepteur, plus de prio
					WG = M() -  MachineRecevoir;
				}
			}
			listeTache[iboucle].prio = IB + WG;
			//printf("priorite de la tache %d : %d \n",indiceVM,listeTache[iboucle].prio);	
		}
	}
	///Trier les tâches en priorité décroissant
	SortListByPrio(listeTache, nbTache);
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


/************************************************************************************/
///Ordonnacement d'une liste de tâches. Meme principle pour pré et non-pré.
///On peut choisir si on a le droit d'allumer la machine en question.
///Prendre en compte la gestion réseau et aussi la mise à jour des serveurs et du réseau
/************************************************************************************/
void OrdoListeTache(Tache* listeTache, unsigned int nbTache, unsigned int indice,unsigned int indiceTabServeur, int & compteurAffect)
{
			int i;
			int iboucle1=0;
			int iboucle2=0;
			int intervalInf = Traitement.ListOfIntervalles[indice].BorneInf;
			int intervalSup = Traitement.ListOfIntervalles[indice].BorneSup;
			int indiceVM, indiceVM2;
			int indiceServeur = Traitement.ListOfServer[indiceTabServeur].IndiceServeur;
			int lastIndiceInterval, lastIndiceServeur, duree;
			bool needMigration = false;
			int bandMigration = 0;
			
			//affectation des tâches appartenant à la liste des tâches
			for(iboucle1=0;iboucle1<nbTache;iboucle1++){ ///Pour chaque tâche
				indiceVM = listeTache[iboucle1].IndiceVM;///Pour simplifier la vie
				needMigration = false;
				bandMigration = 0;
				
				///Si c'est pas encore affectée ///C'est pas très nécessaire ici car bien sûr c'est pas encore affectée.
				if (q(indiceVM, indiceServeur)==1 && 
					(Traitement.ListOfOrdo[intervalInf][indiceVM].affecter!=1)&&
					(listeTache[iboucle1].prio>=0))
				{
					LastExecution(indice, indiceVM, lastIndiceInterval, lastIndiceServeur, duree);
					if(  lastIndiceServeur != indiceServeur && duree>mt(indiceVM)) 
					{///Traitement pour migration
							needMigration = true;
							bandMigration = b(indiceVM, indiceVM);
							int lastInterSup = Traitement.ListOfIntervalles[lastIndiceInterval].BorneSup;
							int debutMigration = lastInterSup - mt(indiceVM) + 1;
							assert(debutMigration>=0);
							bool possible = true;
							for( int temps = lastInterSup; temps >= debutMigration; temps --)
							{
								///Vérifier la machine de destination a assez de ressource pour la migration
								if( Traitement.ListOfServeurbis[ temps][ indiceServeur].HDD < nh(indiceVM)|| Traitement.ListOfServeurbis[ temps][ indiceServeur].RAM < nr(indiceVM))
								{
									possible = false;
									break;
								}
							}
							///Migration pas possible pour  i,j
							if(possible == false)continue;
							///La vérification de Bii sera faite dans la partie CalculFeasiblilitéRéseau
					}

					
					///Si la machine a assez de ressource pour la recevoir
					if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].GPU>=ng(indiceVM)){
						if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].CPU>=nc(indiceVM)){
							if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].HDD>=nh(indiceVM) ){
								if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].RAM>=nr(indiceVM) ){
									///Backup network state
									memcpy(Traitement.EdgeBdeDispoBackUp, Traitement.EdgeBdeDispo, MaxTimeHorizon* MaxEdges* sizeof(unsigned int));

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
													indiceServeur,Traitement.ListOfOrdo[intervalSup][indiceVM2].IndiceMachine,indice)==false)///Machine i et j et intervalle
												{
													///Cette tâche ne peut pas être affectée sur cette machine. Mais on doit voir autres machine.
													Traitement.ListOfOrdo[intervalSup][indiceVM].affecter = 0;///Pour indiquer que c'est pas affectée.
													Traitement.ListOfOrdo[intervalInf][indiceVM].affecter = 0;
													///Rollback
													memcpy(Traitement.EdgeBdeDispo, Traitement.EdgeBdeDispoBackUp, MaxTimeHorizon* MaxEdges* sizeof(unsigned int));
													indiceVM2 = -1; ///On va casser ce passage de boucle, continuer sur la tâche suivante à affecter.
													break;
												}
											}
										}
									}///Fin du parcours de tâches
									if(indiceVM2 == -1)continue; ///La tâche actuelle n'est pas permite par le réseau, donc on continue sur la tâche suivante.

									if(needMigration)
									{
										if(CalculFesabiliteResau(indiceVM,indiceVM, ///tâche i et j
											indiceServeur, lastIndiceServeur,indice)==false)///Machine i et j et intervalle
										{
											///RollBack
											memcpy(Traitement.EdgeBdeDispo, Traitement.EdgeBdeDispoBackUp, MaxTimeHorizon* MaxEdges* sizeof(unsigned int));
											continue;
										}else ///C'est feasable. Alors mise à jour ressouces par rapport à la migration
										{
											///Pour calculer le cout total
											Traitement.ListOfOrdo[intervalInf][indiceVM].isMigrated = true;
											int lastInterSup = Traitement.ListOfIntervalles[lastIndiceInterval].BorneSup;
											int debutMigration = lastInterSup - mt(indiceVM) + 1;
											for( int temps = lastInterSup; temps >= debutMigration; temps --)
											{
												Traitement.ListOfServeurbis[ temps][ indiceServeur].HDD -= nh(indiceVM);
												Traitement.ListOfServeurbis[ temps][ indiceServeur].RAM -= nr(indiceVM);
											}
										}
									}

									/////////////////////From here we know that we can do the asignment///////////////
									
									if(Traitement.ListOfServer[indiceTabServeur].ON != true)
									{
										Traitement.ListOfServer[indiceTabServeur].ON = true;
										Traitement.ListOfNbServeurOn[indice].NbServeurOn++;
									}

									///Mettre à jour les attributs pour chaque intant de temps de l'intervalle
									for(i=intervalInf;i<=intervalSup;i++){
										if(i == intervalInf)///Pour le premier instant, traitement spécial
										{
											///DureeExe s'appuis sur la dureeExe dernière si c'est une execution continue.
											if(duree >0 && lastIndiceInterval == indice-1 && lastIndiceServeur == indiceServeur)
												Traitement.ListOfOrdo[i][indiceVM].dureeExe = duree + 1;
											else Traitement.ListOfOrdo[i][indiceVM].dureeExe = 1;
										}
										else Traitement.ListOfOrdo[i][indiceVM].dureeExe = Traitement.ListOfOrdo[i-1][indiceVM].dureeExe+1;
										
										Traitement.ListOfOrdo[i][indiceVM].IndiceMachine = indiceServeur;
										Traitement.ListOfOrdo[i][indiceVM].affecter=1;
										
										///Mettre à jour les ressources de la machine
										Traitement.ListOfServeurbis[i][indiceServeur].GPU -=  ng(indiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].CPU -=  nc(indiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].HDD -=  nh(indiceVM);
										Traitement.ListOfServeurbis[i][indiceServeur].RAM -=  nr(indiceVM);
									}
									compteurAffect ++;
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
int GetDureeExeActuelle(unsigned int indice, unsigned int indiceVM, unsigned int & lastIndiceInterval, unsigned int & lastIndiceServeur, int & dureeActuelle)
{
	int i= Traitement.ListOfIntervalles[indice].BorneInf-1;
	while(i>=0)
	{
		if(Traitement.ListOfOrdo[i][indiceVM].dureeExe > 0)
				return Traitement.ListOfOrdo[i][indiceVM].dureeExe;
		i--;
	}
	return 0;
}

void LastExecution(unsigned int indice, unsigned int indiceVM, int & lastIndiceInterval, int & lastIndiceServeur, int & dureeActuelle)
{
	lastIndiceServeur = -1;
	dureeActuelle = 0;
	lastIndiceInterval = indice - 1;
	while(lastIndiceInterval >=0 )
	{
		for(int i = Traitement.ListOfIntervalles[lastIndiceInterval].BorneSup; i>= Traitement.ListOfIntervalles[lastIndiceInterval].BorneInf; i--)
		{
			if(Traitement.ListOfOrdo[i][indiceVM].dureeExe > 0)
			{
				lastIndiceServeur = Traitement.ListOfOrdo[i][indiceVM].IndiceMachine;
				dureeActuelle = Traitement.ListOfOrdo[i][indiceVM].dureeExe;
				return;
			}
		}
		lastIndiceInterval --;
	}
}

///Allumer une machine et faire l'affectation au dessus.
///@param indice indice de l'intervalle
int AllumageMachine(unsigned indice, int debutIndiceServeur){
	int indiceVM = -1;
	int indiceServeur = -1;
	int MachineRallume = -1;
	 
	///C'est plutôt la différence entre le coût de rallumer la machine et le coût de ne pas la rallumer. Plus c'est moins, mieux c'est
	int gain = 0, gainMin = 0;
	int intervalInf = Traitement.ListOfIntervalles[indice].BorneInf;
	int intervalSup = Traitement.ListOfIntervalles[indice].BorneSup;
	double coutON = 0; //Coût unitaire d'être ON
	for(int temps=intervalInf; temps<=intervalSup; temps++)
			coutON += beta(temps);
	coutON = coutON/(intervalSup-intervalInf+1);

	bool begin = false;
	///On va chercher une meilleure machine dans la liste triée des machines
	///Principe: Une machine peut être allumée ssi le coût de l'allumer < le coût de ne pas l'allumer. (coutON+coutAffect < rho)
	///Et parmi toutes les machines qui peuvent être allumées, on prend la meilleure.
	for(int iboucle1 = 0; iboucle1<M();iboucle1++){
		if(Traitement.ListOfServer[iboucle1].ON == false){
			gain = 0;
			indiceServeur = Traitement.ListOfServer[iboucle1].IndiceServeur;
			if(indiceServeur==debutIndiceServeur)
			{
				begin = true;
				continue;
			}
			if(begin==false)continue;
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
					gain += coutON + CalculCoutAffectation(indiceVM, indiceServeur) - rho(indiceVM);
				}
			}
			if( gain < gainMin)
			{
				gainMin = gain;
				MachineRallume = indiceServeur;
			}
		}///Fin if current server off
	}///Fin boucle de toutes les machines

	if(gainMin < 0) return MachineRallume;
	else return -1;
}



///Trier la liste de serveur selon coutnorm
void SortServerList(Serveur* arr, int size)
{
	if(size<2)return;
	int mid = arr[size/2].CoutNorm;
	int i=0, j=size-1;
	while(i<j)
	{
		while(arr[i].CoutNorm < mid && i<size)i++;
		while(arr[j].CoutNorm > mid && j>0)j--;
		if(i<=j)
		{
			Swap(arr[i], arr[j]);
			i++; j--;
		}
	}
	if( j > 0 )SortServerList(arr, j+1);
	if(i < size)SortServerList(arr+i, size-i);
}

///Quick sort for sorting task list
void SortListByPrio(Tache* arr, int size)
{
	if(size<2)return;
	int mid = arr[size/2].prio;
	int i=0, j=size-1;
	while(i<j)
	{
		while(arr[i].prio > mid && i<size)i++;
		while(arr[j].prio < mid && j>0)j--;
		if(i<=j)
		{
			Swap(arr[i], arr[j]);
			i++; j--;
		}
	}
	if( j > 0 )SortListByPrio(arr, j+1);
	if(i < size)SortListByPrio(arr+i, size-i);
}
