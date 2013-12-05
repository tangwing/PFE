#include "Data.h"
#include "Traitement.h"
#include "stdio.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "Data.h"
#include <algorithm>

#include "Test.h"
using namespace std;

int main()
{
	Test();

	double dOptValue,dOptTime;
	int NbMach;
	int isFeasible = 1;
	time_t temp1,temp2,tempPre1,tempPre2;
	FILE *fic;
    GetData();
	DisplayData();
	time(&temp1);
	/*printf("Bande passante max pour tout les arcs : %d\n",maxb());
	printf("Nombre d'arcs sur le resau : %d \n",NbEdges());
	for(int arc=0;arc<NbEdges();arc++){
		printf("nombre de couples de machines utilisant l'arc %d : %d \n",arc,NbMachEdge(arc));
	}
	printf("Quantite de bandePassante pour la communication de la VM2 et VM3 : %d \n",b(2,3));
	*/
	printf("\n ****** test ****** \n");
	for(int i=0;i<T();i++){
		for(int j=0;j<N();j++){
			Traitement.ListOfOrdo[i][j].IndiceMachine=-1;
			Traitement.ListOfOrdo[i][j].dureeExe=0;
			if(u(j,i)==0){
				Traitement.ListOfOrdo[i][j].IndiceMachine=-2;
			}
		}
	}

	for(int temps = 0;temps<T();temps++){
		for(int Arc = 0;Arc<NbEdges();Arc++){
			for(int Couple = 0;Couple < NbMachEdge(Arc);Couple++){
				unsigned int Machine1;//(Traitement.ListOfReseau[temps][Arc][Couple].Mach1);
				unsigned int Machine2;//(Traitement.ListOfReseau[temps][Arc][Couple].Mach2);
				CoupleMachines(Arc, Couple, Machine1,Machine2);
				Traitement.ListOfReseau[temps][Arc][Couple].Mach1 = Machine1;
				Traitement.ListOfReseau[temps][Arc][Couple].Mach2 = Machine2;
				Traitement.ListOfReseau[temps][Arc][Couple].BdePassanteDispo = maxb();
			}
		}
	}

	/*or(int temps = 0;temps<T();temps++){
		for(int Arc = 0;Arc<NbEdges();Arc++){
			for(int Couple = 0;Couple < NbMachEdge(Arc);Couple++){
				printf("Le couple numero %d sur l'arc %d est : %d - %d \n",Couple,Arc,Traitement.ListOfReseau[temps][Arc][Couple].Mach1,Traitement.ListOfReseau[temps][Arc][Couple].Mach2);
			}
		}
	}*/

	CalculCoutNorm();
	CalculInterval();
	printf("Nombre d'intervale : %d \n",Traitement.NbInterval);
	for(int i=0;i<Traitement.NbInterval;i++){
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
	for(int i=0;i<T();i++){
		for(int j=0;j<N();j++){
			if ((Traitement.ListOfOrdo[i][j].IndiceMachine == -1)&&(R(j)==0)){
				isFeasible = 0;
			}
			printf("%d \t",Traitement.ListOfOrdo[i][j].IndiceMachine);
		}
		printf("\n");
	}
	dOptValue = TotalCost();
	time(&temp2);
	dOptTime=difftime(temp2,temp1);
	
	fic=fopen("Heuristic.txt","wt");
	fprintf(fic,"%d\n%d\n%lf\n",isFeasible,(int)dOptValue,dOptTime);
	fclose(fic);
	printf("Temps d'execution : %f \n",dOptTime);
	getchar();
}
