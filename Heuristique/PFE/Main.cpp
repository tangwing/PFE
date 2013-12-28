#include "Data.h"
#include "Traitement.h"
#include "stdio.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <algorithm>
#include "ConsoleTable.h"
///#include "easylogging++.h"
//#include "Test.h"

int main()
{
	using namespace std;
	double dOptValue,dOptTime;
	int NbMach;
	int isFeasible = 1;
	time_t temp1,temp2,tempPre1,tempPre2;
	FILE *fic;
    GetData();
	//DisplayData();
	Init();

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
		/*for(int serveur=0;serveur<M();serveur++){
			printf("Le serveur %d est utilise : %d\n",Traitement.ListOfServer[serveur].IndiceServeur,Traitement.ListOfServer[serveur].ON);
		}	*/
		/*for(int indice = 0;indice<M();indice++){
			printf("Le serveur utilise %d avec comme cout : %d \n",Traitement.ListOfServerOn[indice].IndiceServeur,Traitement.ListOfServerOn[indice].CoutNorm);
		}*/
	}
	//printf("nombre de machine active : %d  \n",Traitement.NbServeurOn);
	//for(int i=0;i<Traitement.NbServeurOn;i++){
			//printf("La machine %d est allumee \n",Traitement.ListOfServerOn[i].IndiceServeur);
	//}
	//NbMach = Traitement.NbServeurOn;

	///Tester si les t^^aches non-pré sont toutes affectées. Sinon alors on a pas trouvé la solution.
	///Afficher la matrice d'ordo
	for(int i=0;i<T();i++){
		for(int j=0;j<N();j++){
			if ((Traitement.ListOfOrdo[i][j].IndiceMachine == -1)&&(R(j)==0)){
				isFeasible = 0;
				printf("*");
			}
			//printf("%d \t",Traitement.ListOfOrdo[i][j].IndiceMachine);
		}
		//printf("\n");
	}
	dOptValue = TotalCost();
	time(&temp2);
	dOptTime=difftime(temp2,temp1);

	fic=fopen("Heuristic.txt","wt");
	fprintf(fic,"%d\n%d\n%lf\n%lf\n",isFeasible,(int)dOptValue,dOptTime, (double)Traitement.NbServeurOn/T());
	fclose(fic);
	printf("Feasible:%d\n ValeurOpt:%d\n Temps:%lf\n NbMoyServeurOn:%lf\n",isFeasible,(int)dOptValue,dOptTime, (double)Traitement.NbServeurOn/T());
	getchar();
}
