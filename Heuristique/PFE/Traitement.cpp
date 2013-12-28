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

///@brief Initialisation des structures de donn�es
void Init()
{
	Traitement.NbServeurOn = 0;

	///Init ListOfOrdo
	for(int i=0;i<T();i++){
		for(int j=0;j<N();j++){
			Traitement.ListOfOrdo[i][j].affecter = false;
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

	///La ListofServeurbis contient les caracs r�siduelles
    for(int temps = 0;temps< T();temps++){
    		for(int indiceS = 0; indiceS<M(); indiceS++){
                    Traitement.ListOfServeurbis[temps][indiceS].CPU = Data.ListOfMachines[indiceS].QtyCPU;
                    Traitement.ListOfServeurbis[temps][indiceS].GPU = Data.ListOfMachines[indiceS].QtyGPU;
                    Traitement.ListOfServeurbis[temps][indiceS].RAM = Data.ListOfMachines[indiceS].QtyRAM;
                    Traitement.ListOfServeurbis[temps][indiceS].HDD = Data.ListOfMachines[indiceS].QtyHDD;
            }
    }

	///Les structures de r�seau. Ici on va adapter un peu les donn�es: CoupleEdgeMap nous permettra d'obtenir facilement tous les arcs utilis�s par une couple de machines
	unsigned int mi, mj;
	std::map< std::pair<int,int>, std::set<int>>::iterator it;
	for (int iEdge=0; iEdge<NbEdges(); iEdge++)
	{
		for(int temps = 0;temps< T();temps++){
			Traitement.EdgeBdeDispo[temps][iEdge] = maxb();
		}
		for (int iLoop=0; iLoop<NbMachEdge(iEdge); iLoop++)
		{
			CoupleMachines(iEdge,iLoop,mi,mj);
			if (mi>mj) Swap(mi, mj);
			std::pair<int, int> couple(mi, mj);
			///Update the set of edges in the map. 
			///The key of the map is the machine couple, and the value is a set of edges.
			///First of all get the corresponding set
			it = Traitement.CoupleEdgeMap.insert( std::pair< 
				std::pair<int, int>, std::set<int>>(
				couple,              std::set<int>())).first;
			///Then insert new edge to the set
			(it->second).insert(iEdge);
		}
	}
}

///
///@brief Calcule les intervalles tel que sur tous les instants d'un m�me intervalle les u(i,t) sont consants
///
void CalculInterval(){
	Traitement.NbInterval = 0;
	Traitement.ListOfIntervalles[0].BorneInf = 0;
	Traitement.ListOfIntervalles[0].BorneSup = 0;
	for(int instant=0;instant<T()-1;instant++){
		for(int tacheI=0;tacheI<N();tacheI++){
			if(( u(tacheI,instant)!=u(tacheI,instant+1))){///intervalle trouv�
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
	
	AfficherIntervalle();
}

///
///@brief Trier la liste des machines en ordre croissante selon leurs couts normalis�s
///
void CreerListeMachineTriee(){
	int i;
	double CoutTotal;
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

///
///@brief Calcule le cout total de l'ordonnancement trouv�
///
int TotalCost(){
	int TotalCost = 0;
	int CoutMigration = 0;
	int CoutAffect = 0;
	int CoutUnitaire = 0;
	int Penalty = 0;

	AfficherOrdo();
	//LoadOrdo();
	//AfficherAffinite();
	//AfficherRt();
	//AfficherCaracMachine();
	AfficherListeServeurBis();
	AfficherEdgeDispo();

	for(int t=0;t<T();t++){
		for(int n=0;n<N();n++){
			if(Traitement.ListOfOrdo[t][n].affecter==true){
				int iMache = Traitement.ListOfOrdo[t][n].IndiceMachine;
				CoutAffect = CoutAffect+ (alphac(iMache)*nc(n)+alphag(iMache)*ng(n)+alphar(iMache)*nr(n)+alphah(iMache)*nh(n));//CalculCoutAffectation( n, Traitement.ListOfOrdo[t][n].IndiceMachine);
				if(Traitement.ListOfOrdo[t][n].isMigrated)
					CoutMigration += mt(n)*(alphar(Traitement.ListOfOrdo[t][n].IndiceMachine)*nr(n)+alphah(Traitement.ListOfOrdo[t][n].IndiceMachine)*nh(n));
			}
			else if(u(n,t)==1&& R(n)==1)
			{
				Penalty += rho(n);
			}
		}
	}
	//printf("\nNbServeurOn:\t");
	for(int indice = 0;indice< Traitement.NbInterval;indice++){
		int nbON = Traitement.ListOfNbServeurOn[indice].NbServeurOn;
		//printf("%d\t", nbON);
		for(int t=Traitement.ListOfIntervalles[indice].BorneInf; t<=Traitement.ListOfIntervalles[indice].BorneSup; t++)
			CoutUnitaire += (nbON * beta(t));
	}
	printf("\nPenalite totale : %d \n",Penalty);
	printf("CoutUnitaire total :%d \n",CoutUnitaire);
	printf("CoutMigration :%d \n",CoutMigration);

	TotalCost = CoutAffect + CoutUnitaire + Penalty + CoutMigration;
	printf("Cout total : %d \n",TotalCost);
	return TotalCost;
}

///@brief Calcule le cout d'affectation de la tache i sur la machine j
double CalculCoutAffectation(unsigned int i,unsigned int j){
	return (alphac(j)*nc(i)+alphag(j)*ng(i)+alphar(j)*nr(i)+alphah(j)*nh(i));
}

///@brief V�rifier si le r�seau permet la communication entre 2 t�ches ou la migration d'une t�che
bool CalculFesabiliteResau(
	unsigned tachei,unsigned tachej, 
	unsigned machinei,unsigned machinej,
	unsigned indice) ///Indice de l'intervalle
{
	if( tachei == tachej && machinei == machinej) return true;	
	if(a(tachei, tachej) != 1)return true;
	
	int intervalInf = Traitement.ListOfIntervalles[indice].BorneInf;
	int intervalSup = Traitement.ListOfIntervalles[indice].BorneSup;

	std::map< std::pair<int,int>, std::set<int>>::iterator it;
	if(machinei > machinej) Swap(machinei, machinej);

	///Get the edge set
	it = Traitement.CoupleEdgeMap.find( std::pair<int, int>(machinei, machinej));
	assert(it!=Traitement.CoupleEdgeMap.end());
	std::set<int> edgeSet = it->second;
	std::set<int>::iterator itEdge;
	if( tachei == tachej) ///Case of migration
	{	///For each edge passed by the migration flux, the band width must be enough
		int lastDuree, lastIndiceInterval, lastIndiceServeur;
		LastExecution(indice, tachei, lastIndiceInterval, lastIndiceServeur, lastDuree);
		int lastInterSup = Traitement.ListOfIntervalles[lastIndiceInterval].BorneSup;
		int debutMigration = lastInterSup - mt(tachei) + 1;
		for(itEdge=edgeSet.begin(); itEdge != edgeSet.end(); itEdge++)
		{
			for(int i = lastInterSup; i>= debutMigration; i--)
			{
				if( Traitement.EdgeBdeDispo[i][*itEdge]<b(tachei, tachei))
					return false;
				else
					Traitement.EdgeBdeDispo[i][*itEdge] -= b(tachei, tachei);
			}
		}
	}else  ///Case of affinity
	{	///For each edge passed by i,j, the band width must be enough
		for(itEdge=edgeSet.begin(); itEdge != edgeSet.end(); itEdge++)
		{
			if( Traitement.EdgeBdeDispo[intervalInf][*itEdge]<b(tachei, tachej) )
				return false;
			else
			{	///! Il faut mettre � jour la valeur pour tout l'intervalle
				for(int i=intervalInf; i<= intervalSup; i++)
					Traitement.EdgeBdeDispo[i][*itEdge] -= b(tachei, tachej);
			}
		}
	}
	return true;
}

///
///@brief Permet de construire les listes des taches
///
void ConstructionListesTache(unsigned int indice){
	Traitement.NbHDDRAMGPU = 0; //nb de t�ches avec des besoins en GPU/CPU et besoins HDD > besoins RAM
	Traitement.NbRAMHDDGPU = 0; //nb de t�ches avec des besoins en GPU/CPU et besoins RAM > besoins HDD
	Traitement.NbHDDRAMCPU = 0;	//nb de t�ches sans GPU et besoins HDD > besoins RAM
	Traitement.NbRAMHDDCPU = 0; //nb de t�ches sans GPU et besoins RAM > besoins HDD
	Traitement.NbHDDRAMGPUPr = 0; 
	Traitement.NbRAMHDDGPUPr = 0; 
	Traitement.NbHDDRAMCPUPr = 0;	
	Traitement.NbRAMHDDCPUPr = 0;
	Traitement.NbPr = 0; 		//nb de t�ches Pr�emptables

		int temps=Traitement.ListOfIntervalles[indice].BorneInf;
		for (int i=0; i<N();i++){
			//Construction des listes t�ches non pr�emtable avec des besoins en GPU et CPU 
			if((u(i,temps)==1)){
				if(R(i) == 0) ///Pour les t�ches non-pr�
				{
					if(Data.ListOfTasks[i].QtyGPU>0)
					{
						//Construction de la liste HDD > RAM
						if(nh(i)>nr(i))
							Traitement.ListOfTasks1GPU[Traitement.NbHDDRAMGPU++].IndiceVM = i;
						else
							Traitement.ListOfTasks2GPU[Traitement.NbRAMHDDGPU++].IndiceVM = i;
					}else ///Pour les t�ches de CPU 
					{
						//Construction de la liste HDD > RAM
						if(nh(i)>nr(i))
							Traitement.ListOfTasks1CPU[Traitement.NbHDDRAMCPU++].IndiceVM = i;
						else
							Traitement.ListOfTasks2CPU[Traitement.NbRAMHDDCPU++].IndiceVM = i;
					}
				}///fin if R=0
				else ///Pour les t�ches pr�
				{
					///La liste de toutes les t�ches pr�
					Traitement.ListofTasksPr[Traitement.NbPr++].IndiceVM = i;
					
					///Les 4 listes selon besoins
					if(Data.ListOfTasks[i].QtyGPU>0)
					{
						//Construction de la liste HDD > RAM
						if(nh(i)>nr(i))
							Traitement.ListOfTasks1GPUPr[Traitement.NbHDDRAMGPUPr++].IndiceVM = i;
						else
							Traitement.ListOfTasks2GPUPr[Traitement.NbRAMHDDGPUPr++].IndiceVM = i;
					}else ///Pour les t�ches de CPU 
					{
						//Construction de la liste HDD > RAM
						if(nh(i)>nr(i))
							Traitement.ListOfTasks1CPUPr[Traitement.NbHDDRAMCPUPr++].IndiceVM = i;
						else
							Traitement.ListOfTasks2CPUPr[Traitement.NbRAMHDDCPUPr++].IndiceVM = i;
					}
				}/// fin t�ches pr�
			}///fin if u=1
		}
		AfficherListesTache(indice);
}


///@brief Ordonnancer les t�ches sur l'intervalle [indice]
void Ordonnancement(int indice){
	int CPU = 0;
	int GPU = 0;
	int RAM = 0;
	int HDD = 0;
	int iboucleS = 0;
	int indiceTab = 0;

	///! Maintenant on a d�j� les listes de t�ches, alors pour chaque machine, 
	///on essaie de la remplir par des t�ches.
	///T�ches non pr� d'abord
	for(iboucleS=0;iboucleS<M();iboucleS++){
		int indiceS = Traitement.ListOfServer[iboucleS].IndiceServeur;
		Traitement.ListOfServer[iboucleS].ON = false;	///Pour calculer le NbServeurOn il faut d'abord �teindre la machine	
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
			///Pour les t�ches pr�emptables
			///D'abord on les traite comme pour les t�ches pr�emptables
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
	///Pour les t�ches restantes pr�emptables
	int indiceAllume = -1;	///!!C'est l'indice pour ListeServeurTri�
	int debutIndiceMachine = 0;
	while(Traitement.NbPrAffected < Traitement.NbPr){
		///Rallumer les machines si besoin
		indiceAllume = AllumageMachine(indice, debutIndiceMachine);
		if( indiceAllume== -1) break; ///Aucunne machine peut �tre allum�e
		else ///Affectation sur la machine allum�e
		{
			int indiceS = Traitement.ListOfServer[indiceAllume].IndiceServeur;
			CalculPrioEtTrier(Traitement.ListOfTasks1GPUPr, Traitement.NbHDDRAMGPUPr,indice, indiceS);
			CalculPrioEtTrier(Traitement.ListOfTasks2GPUPr, Traitement.NbRAMHDDGPUPr,indice, indiceS);
			CalculPrioEtTrier(Traitement.ListOfTasks1CPUPr, Traitement.NbHDDRAMCPUPr,indice, indiceS);
			CalculPrioEtTrier(Traitement.ListOfTasks2CPUPr, Traitement.NbRAMHDDCPUPr,indice, indiceS);
				
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
 

///@brief Pour un intervalle et un serveur donn�s, calculer la prio des t�ches qui peuvent �tre affect�es sur cette machine
void CalculPrioEtTrier(Tache* listeTache, int nbTache, int indice,int indiceServeur){
	int IB = 0;
	int WG = 0;
	int indiceVM;
	int lastIndiceInterval, lastIndiceServeur, duree;
	int MachineRecevoir = 0;
	for(int iboucle = 0; iboucle<nbTache;iboucle++){ ///Pour chaque t�che
		listeTache[iboucle].prio = 0;
		IB = 0;
		WG = 0;
		MachineRecevoir = 0;
		indiceVM = listeTache[iboucle].IndiceVM;
		//duree = GetDureeExeActuelle(indice, indiceVM); 
		LastExecution(indice, indiceVM, lastIndiceInterval, lastIndiceServeur, duree);

		///Si la t�che n'est pas encore affect�e
		if(Traitement.ListOfOrdo[Traitement.ListOfIntervalles[indice].BorneInf][indiceVM].affecter == false){
			if(q(indiceVM, indiceServeur)== false)
			{
				listeTache[iboucle].prio = -M();
				continue;
			}

			///Si cette t�che �tait affect�e sur cette machine � l'intervalle pr�c�dent, alors elle a plus de prio
			if( duree!=0 && lastIndiceInterval == indice-1 && lastIndiceServeur == indiceServeur){IB = M();}

			///Pour WG. 
			if(duree==0) ///Si la t�che n'a jamais �t� ex�cut�e
			{
				for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
					if(q(indiceVM, iboucle2)==1){
						MachineRecevoir++;
					}
				}
				///Moins de r�cepteur, plus de prio
				WG = M() -  MachineRecevoir;
			}
			///Si elle a �t� execut�e sur cette machine et suspendue
			else if( lastIndiceServeur == indiceServeur && lastIndiceInterval < indice-1)
			{
				int inf = Traitement.ListOfIntervalles[indice].BorneInf;
				int lastInstant = Traitement.ListOfIntervalles[lastIndiceInterval].BorneSup;
				if( inf > lastInstant + rt(indiceVM, indiceServeur))
					WG = indiceServeur;
				else WG = -2 * M(); ///Pas possible de r�veiller
			}
			///Migration n�cessaire
			else if(lastIndiceServeur != indiceServeur && lastIndiceInterval == indice-1)
			{ 
				///Migration pas possible
				if(duree < mt(indiceVM))
					WG = -2*M(); ///On ne va pas affecter les t�ches qui a une prio n�gative
				else ///Migration possible mais � v�rifier nh et nr et bii � l'affectation
				{
					for(int iboucle2 = 0; iboucle2<M(); iboucle2++){
						if(q(indiceVM, iboucle2)==1){
							MachineRecevoir++;
						}
					}
					///Moins de r�cepteur, plus de prio
					WG = M() -  MachineRecevoir;
				}
			}
			else WG=0;
			listeTache[iboucle].prio = IB + WG;
			//printf("priorite de la tache %d : %d \n",indiceVM,listeTache[iboucle].prio);	
		}
	}
	///Trier les t�ches en priorit� d�croissant
	SortListByPrio(listeTache, nbTache);
}

///
///@brief Ordonnacement d'une liste de t�ches. Meme principle pour pr� et non-pr�.
///@detail On peut choisir si on a le droit d'allumer la machine en question.
///Prendre en compte la gestion r�seau et aussi la mise � jour des serveurs et du r�seau
///
void OrdoListeTache(Tache* listeTache, int nbTache, int indice, int indiceTabServeur, int & compteurAffect)
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
			
			//affectation des t�ches appartenant � la liste des t�ches
			for(iboucle1=0;iboucle1<nbTache;iboucle1++){ ///Pour chaque t�che
				indiceVM = listeTache[iboucle1].IndiceVM;///Pour simplifier la vie
				needMigration = false;
				bandMigration = 0;
				
				///Si c'est pas encore affect�e ///C'est pas tr�s n�cessaire ici car bien s�r c'est pas encore affect�e.
				if (q(indiceVM, indiceServeur)==1 && 
					(Traitement.ListOfOrdo[intervalInf][indiceVM].affecter == false)&&
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
								///V�rifier la machine de destination a assez de ressource pour la migration
								if( Traitement.ListOfServeurbis[ temps][ indiceServeur].HDD < nh(indiceVM)|| Traitement.ListOfServeurbis[ temps][ indiceServeur].RAM < nr(indiceVM))
								{
									possible = false;
									break;
								}
							}
							///Migration pas possible pour  i,j
							if(possible == false)continue;
							///La v�rification de Bii sera faite dans la partie CalculFeasiblilit�R�seau
					}
					
					///Si la machine a assez de ressource pour la recevoir
					if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].GPU>=ng(indiceVM)){
						if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].CPU>=nc(indiceVM)){
							if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].HDD>=nh(indiceVM) ){
								if(Traitement.ListOfServeurbis[intervalInf][indiceServeur].RAM>=nr(indiceVM) ){
									///Backup network state
									memcpy(Traitement.EdgeBdeDispoBackUp, Traitement.EdgeBdeDispo, MaxTimeHorizon* MaxEdges* sizeof(int));

									///Si la contrainte r�seau le permet (Gestion r�seau pour deux VMs qui poss�dent une affinit�)
									for(indiceVM2 = 0; indiceVM2<N(); indiceVM2++){
										
										///Si on trouve une VM qui a une affinit� de celle qu'on est en train de traiter
										if(a(indiceVM,indiceVM2)==1)
										{
											///Si l'autre VM est affect�e pour cet intervalle et pas sur une autre machine
											if((Traitement.ListOfOrdo[intervalInf][indiceVM2].affecter==true)&&(Traitement.ListOfOrdo[intervalInf][indiceVM2].IndiceMachine != indiceServeur))
											{
												///Si le r�seau ne permet pas cette affectation
												if(CalculFesabiliteResau(indiceVM,indiceVM2, ///t�che i et j
													indiceServeur,Traitement.ListOfOrdo[intervalSup][indiceVM2].IndiceMachine,indice)==false)///Machine i et j et intervalle
												{
													///Cette t�che ne peut pas �tre affect�e sur cette machine. Mais on doit voir autres machine.
													Traitement.ListOfOrdo[intervalSup][indiceVM].affecter = false;///Pour indiquer que c'est pas affect�e.
													Traitement.ListOfOrdo[intervalInf][indiceVM].affecter = false;
													///Rollback
													memcpy(Traitement.EdgeBdeDispo, Traitement.EdgeBdeDispoBackUp, MaxTimeHorizon* MaxEdges* sizeof( int));
													indiceVM2 = -1; ///On va casser ce passage de boucle, continuer sur la t�che suivante � affecter.
													break;
												}
											}
										}
									}///Fin du parcours de t�ches
									if(indiceVM2 == -1)continue; ///La t�che actuelle n'est pas permite par le r�seau, donc on continue sur la t�che suivante.

									if(needMigration)
									{
										if(CalculFesabiliteResau(indiceVM,indiceVM, ///t�che i et j
											indiceServeur, lastIndiceServeur,indice)==false)///Machine i et j et intervalle
										{
											///RollBack
											memcpy(Traitement.EdgeBdeDispo, Traitement.EdgeBdeDispoBackUp, MaxTimeHorizon* MaxEdges* sizeof( int));
											continue;
										}else ///C'est feasable. Alors mise � jour ressouces par rapport � la migration
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

									///Mettre � jour les attributs pour chaque intant de temps de l'intervalle
									for(i=intervalInf;i<=intervalSup;i++){
										if(i == intervalInf)///Pour le premier instant, traitement sp�cial
										{
											///DureeExe s'appuis sur la dureeExe derni�re si c'est une execution continue.
											if(duree >0 && lastIndiceInterval == indice-1 && lastIndiceServeur == indiceServeur)
												Traitement.ListOfOrdo[i][indiceVM].dureeExe = duree + 1;
											else Traitement.ListOfOrdo[i][indiceVM].dureeExe = 1;
										}
										else Traitement.ListOfOrdo[i][indiceVM].dureeExe = Traitement.ListOfOrdo[i-1][indiceVM].dureeExe+1;
										
										Traitement.ListOfOrdo[i][indiceVM].IndiceMachine = indiceServeur;
										Traitement.ListOfOrdo[i][indiceVM].affecter = true;
										
										///Mettre � jour les ressources de la machine
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
				}//else cette t�che est d�j� affect�e
			}			
}


///
///@brief Chercher les informations de l'ex�cution pr�c�dente.
///@param indice L'indice d'intervalle courant
///@param indiceVM L'indice de la t�che en question
///@param lastIndiceInterval Sortie. L'indice d'intervalle de l'ex�cution pr�c�dente
///@param lastIndiceServeur Sortie. L'indice de la machine de l'ex�cution pr�c�dente
///@param dureeActuelle Sortie. La dur�e d'ex�cution pr�c�dente
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

///@brief Allumer une machine et faire l'affectation au dessus.
///@param indice indice de l'intervalle
///!Attention L'indice retourn� est l'indice de la liste tri�e.
int AllumageMachine(unsigned indice, int debutIndiceServeur){
	int indiceVM = -1;
	int indiceServeur = -1;
	int MachineRallume = -1;
	 
	///C'est plut�t la diff�rence entre le co�t de rallumer la machine et le co�t de ne pas la rallumer. Plus c'est moins, mieux c'est
	double gain = 0, gainMin = 0;
	int intervalInf = Traitement.ListOfIntervalles[indice].BorneInf;
	int intervalSup = Traitement.ListOfIntervalles[indice].BorneSup;
	double coutON = 0; //Co�t unitaire d'�tre ON
	for(int temps=intervalInf; temps<=intervalSup; temps++)
			coutON += beta(temps);
	coutON = coutON/(intervalSup-intervalInf+1);

	///On va chercher une meilleure machine dans la liste tri�e des machines
	///Principe: Une machine peut �tre allum�e ssi le co�t de l'allumer < le co�t de ne pas l'allumer. (coutON+coutAffect < rho)
	///Et parmi toutes les machines qui peuvent �tre allum�es, on prend la meilleure.
	for(int iboucle1 = debutIndiceServeur+1; iboucle1<M();iboucle1++){
		if(Traitement.ListOfServer[iboucle1].ON == false){
			gain = 0;
			indiceServeur = Traitement.ListOfServer[iboucle1].IndiceServeur;
			for(int iboucle2 = 0; iboucle2<Traitement.NbPr; iboucle2++){
				indiceVM = Traitement.ListofTasksPr[iboucle2].IndiceVM;
				///Voir si la t�che et la machine sont compatibles
				if(Traitement.ListOfOrdo[intervalInf][indiceVM].affecter == false
					&&(q(indiceVM,indiceServeur)==1) 
					&&(Traitement.ListOfServeurbis[intervalInf][indiceServeur].GPU>=ng(indiceVM)) ///Ici est pour �tre s�r que si la machine �tre allum�e, elle peut au moins recevoir une t�che
					&&(Traitement.ListOfServeurbis[intervalInf][indiceServeur].CPU>=nc(indiceVM))
					&&(Traitement.ListOfServeurbis[intervalInf][indiceServeur].HDD>=nh(indiceVM))
					&&(Traitement.ListOfServeurbis[intervalInf][indiceServeur].RAM>=nr(indiceVM))
				)
				{
					///le co�t d'affection - le co�t de suspension
					gain += coutON + CalculCoutAffectation(indiceVM, indiceServeur) - rho(indiceVM);
				}
			}
			if( gain < gainMin)
			{
				gainMin = gain;
				MachineRallume = iboucle1;
			}
		}///Fin if current server off
	}///Fin boucle de toutes les machines

	if(gainMin < 0) return MachineRallume;
	else return -1;
}

///
///@brief V�rifier qu'� l'intervalle donn�, est-ce que l'ordonnancement effectu� assure bien les affinit�s entre les t�ches
///
bool VerifierAffinite(int indiceInterval)
{
	for(int i=0; i<N(); i++)
		for(int j=i+1; j<N(); j++)
		{
			if(a(i,j) == 1)
				if(Traitement.ListOfOrdo[indiceInterval][i].affecter != Traitement.ListOfOrdo[indiceInterval][j].affecter)
					return false;
		}
	return true;
}

//------------------------ Des fonctions utilitaires ------------------------

///@brief Trier la liste de serveur selon coutnorm
void SortServerList(Serveur* arr, int size)
{
	if(size<2)return;
	int mid = (int)(arr[size/2].CoutNorm);
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

///@brief Quick sort for sorting task list
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
