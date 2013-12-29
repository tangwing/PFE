#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include "Data.h"
#include "Traitement.h"
#include "ConsoleTable.h"
#include "Test.h"

int main()
{
	using namespace std;
	double dOptValue=-1,dOptTime=-1;
	bool isFeasible = true;
	time_t temp1,temp2;
	FILE *fic;
    GetData();

	AfficherUit();
	AfficherAffinite();
	
	///Ajouter ici un prétraitement pour garantir la compatibilité des u(i,t) et a(i,j)
	///Si 2 taches (i,j) ont une affinité, alors il faut que u(i,t)==u(j,t) pour tous les t
	Pretraiter();

	//DisplayData();
	AfficherUit();
	AfficherAffinite();
	Init();

	printf("The heuristic program is running...\n");
	time(&temp1);	
	CalculInterval();
	CreerListeMachineTriee();///Trier les serveurs
	for(int i=0;i<Traitement.NbInterval;i++){
		
		Traitement.ListOfNbServeurOn[i].NbServeurOn = 0;
		Traitement.NbNoPrAffected = 0;
		Traitement.NbPrAffected = 0;

		///! On construit les listes de tâche pour chaque intervalle. Pour une tâche donnée, sa valeur de Uit varie selon l'intervalle
		ConstructionListesTache(i);
		Ordonnancement(i);
		if(false == VerifierAffinite(i)) 
		{
			isFeasible = false;
			break;
		}
	}

	if(isFeasible)
	{
		///Tester si les taches non-pré sont toutes affectées. Sinon alors on a pas trouvé la solution.
		///Afficher la matrice d'ordo
		for(int i=0;i<Traitement.NbInterval;i++)
			for(int j=0;j<N();j++)
			{
				if ((Traitement.ListOfOrdo[i][j].IndiceMachine == -1)&&(R(j)==0))
					isFeasible = 0;
			}
		dOptValue = TotalCost();
	}else printf("Les affinites n'ont pas ete satisfaites\n");
	time(&temp2);
	 
	dOptTime=difftime(temp2,temp1);
	fic=fopen("Heuristic.txt","wt");
	fprintf(fic,"%d\n%d\n%lf\n%lf\n",isFeasible,(int)dOptValue,dOptTime, (double)Traitement.NbServeurOn/T());
	fclose(fic);
	printf("Feasible:%d\n ValeurOpt:%d\n Temps:%lf\n NbMoyServeurOn:%lf\n",isFeasible,(int)dOptValue,dOptTime, (double)Traitement.NbServeurOn/T());
	getchar();
}
