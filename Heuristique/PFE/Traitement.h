#ifndef TRAITEMENT
#define TRAITEMENT

#include <map>
#include <set>

#define MAXVALUE 1000000000.0
#define MaxTasks 500
#define MaxMachines 10
#define MaxEdges 1024
#define MaxTimeHorizon 30

typedef struct{ //Structure de donn�es pour contenir le num�ro des machines avec leur cout normalis?respectif
	int IndiceServeur;			
	double CoutNorm;
	bool ON;

}Serveur;

typedef struct{ 
	int CPU;			
	int GPU;
	int HDD;
	int RAM;
}CaractServeur;

typedef struct{ ///Structure de donn�es pour contenir le num�ro des machines ON avec leur cout normalis?respectif
	int IndiceServeur;			
	double CoutNorm;

}ServeurON;

typedef struct{ 
	int IndiceVM; 
	signed int prio;
}Tache;

//Structure de donn�es permettant de stocker les indice des VMs tq besoins en HDD > besoins en RAM 
typedef Tache HDDRAM;
typedef Tache RAMHDD;
typedef Tache Pream;

typedef struct{ //Structure de donn�es permettant de stocker les indices des intervalles ainsi que les bornes sup�rieur et inf�rieur pour chaque intervalle
	int IndiceInter;
	int BorneInf;			
	int BorneSup;
}Interval;

typedef struct{ //Structure permettant de stocker la valeur de cout d'utilisation d'une VM i sur une machine j allum�e
	int indiceVM;
	double cout;
}ctij;

typedef struct{ //Structure permettant de stocker la valeur de cout d'utilisation d'une VM i sur une machine j non allum�e
	int indicePM;
	double gain;///It's rather the cost of executing this job on a newly started machine 
}gaj;

typedef struct{ 
	int IndiceTache;
	int IndiceMachine;
	bool affecter;
	bool isMigrated;
	///On d�fini que la duree d'un instant T suppose que cet instant est fini.
	int dureeExe; //permet de stocker le temps d'execution r�el de la tache IndiceVM afin de respecter la contrainte de mti
	int dureeSus; //permet de stocker le temps durant lequel la t�che i a �t?suspendu
}Ordo;

typedef struct{///For a given interval, this structure stock the number of running machine
	int NbServeurOn;
	int IndiceServeur;
}NbServeurOn;

typedef struct{
				///! Cette liste sera tri�e, donc id de serveur est PAS indice de serveur dans la liste tri�e!
				Serveur ListOfServer[MaxMachines];
				CaractServeur ListOfServeurbis[MaxTimeHorizon][MaxMachines]; ///Les ressources restantes. Liste non tri�e! Diff�rente que ListOfServer
				int EdgeBdeDispo[MaxTimeHorizon][MaxEdges]; ///should be inited by maxB
				int EdgeBdeDispoBackUp[MaxTimeHorizon][MaxEdges];
				///Cette structure nous permet d'obtenir l'ensemble des arcs � partir d'un couple de machines
				std::map<std::pair<int,int>, std::set<int>> CoupleEdgeMap;
				NbServeurOn ListOfNbServeurOn[MaxTimeHorizon];
				Interval ListOfIntervalles[MaxTimeHorizon];
				Pream ListofTasksPr[MaxTasks];
				//Listes des taches non pr�amtable GPU
				HDDRAM ListOfTasks1GPU[MaxTasks];
				RAMHDD ListOfTasks2GPU[MaxTasks];
				//Listes des taches non pr�amtable CPU
				HDDRAM ListOfTasks1CPU[MaxTasks];
				RAMHDD ListOfTasks2CPU[MaxTasks];
				//Liste des taches pr�antable GPU
				HDDRAM ListOfTasks1GPUPr[MaxTasks];
				HDDRAM ListOfTasks1GPUMachinej[MaxTasks];
				RAMHDD ListOfTasks2GPUPr[MaxTasks];
				RAMHDD ListOfTasks2GPUMachinej[MaxTasks];
				//Liste des taches pr�antable CPU
				HDDRAM ListOfTasks1CPUPr[MaxTasks];
				HDDRAM ListOfTasks1CPUMachinej[MaxTasks];
				RAMHDD ListOfTasks2CPUPr[MaxTasks];	
				RAMHDD ListOfTasks2CPUMachinej[MaxTasks];
				Ordo  ListOfOrdo[MaxTimeHorizon][MaxTasks];
				int NbPr;
				int NbNoPrAffected; ///nb des t�ches non-pr� d�j� affect�es
				int NbPrAffected; ///nb des t�ches pr� d�j� affect�es
				int NbHDDRAMGPU;
				int NbHDDRAMGPUPr;
				int NbHDDRAMCPU;
				int NbHDDRAMCPUPr;
				int NbRAMHDDGPU;
				int NbRAMHDDGPUPr;
				int NbRAMHDDCPU;
				int NbRAMHDDCPUPr;
				int NbInterval; //Permet de stocker le nombre d'intervalle qui d�coupe l'horizon de planification
				int NbServeurOn; //Permet de stocker le nombre de machine physique allum?

}Trait;

extern Trait Traitement;

extern void Pretraiter();
extern void Init();///Initialisation
extern void CalculInterval(void);
extern void CreerListeMachineTriee(void);
extern void CalculPrioEtTrier(Tache* listeTache,  int nbTache,  int indiceIntervalle, int indiceServeur);
extern void ConstructionListesTache(unsigned int indice);
extern void Ordonnancement( int indice);
extern void OrdoListeTache(Tache* listeTache,  int nbTache,  int indiceIntervalle, int indiceTabServeur, int & compteurAffect);
extern bool CalculFesabiliteResau(unsigned tachei,unsigned machinej,unsigned int tacheaffectee);
extern int GetDureeExeActuelle(unsigned int indice, unsigned int indiceVM);
extern void LastExecution(unsigned int indice, unsigned int indiceVM, int & lastIndiceInterval, int & lastIndiceServeur, int & dureeActuelle);
extern int AllumageMachine(unsigned int indice, int debutIndiceMachine);
extern double CalculCoutAffectation(unsigned int i,unsigned int j);
extern int TotalCost(void);
extern bool VerifierAffinite(int indiceInterval);

///swap two elements
template <typename T> void Swap(T& v1, T& v2)
{
	T tmp = v1;
	v1 = v2;
	v2 = tmp;
}

///Trier la liste de serveur selon cout norm
void SortServerList(Serveur* arr, int size);
///Quick sort for sorting task list
//template <typename T> 
void SortListByPrio(Tache* arr, int size);

double GetCpuTime();

#endif