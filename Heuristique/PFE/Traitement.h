#ifndef TRAITEMENT
#define TRAITEMENT

#define MAXVALUE 1000000000.0
#define MaxTasks 500
#define MaxMachines 10
#define MaxEdges 1024
#define MaxTimeHorizon 30

typedef struct{ //Structure de données pour contenir le numéro des machines avec leur cout normalis?respectif
	int IndiceServeur;			
	float CoutNorm;
	bool ON;

}Serveur;

typedef struct{ 
	int CPU;			
	int GPU;
	int HDD;
	int RAM;
}CaractServeur;

typedef struct{ ///Structure de données pour contenir le numéro des machines ON avec leur cout normalis?respectif
	int IndiceServeur;			
	float CoutNorm;

}ServeurON;

typedef struct{ 
	int Mach1;			
	int Mach2;
	int BdePassanteDispo;

}Reseau;

typedef struct{ 
	short int IndiceVM; 
	signed int prio;
}Tache;

//Structure de données permettant de stocker les indice des VMs tq besoins en HDD > besoins en RAM 
typedef Tache HDDRAM;
typedef Tache RAMHDD;
typedef Tache Pream;

typedef struct{ //Structure de données permettant de stocker les indices des intervalles ainsi que les bornes supérieur et inférieur pour chaque intervalle
	short int IndiceInter;
	short int BorneInf;			
	short int BorneSup;
}Interval;

typedef struct{ //Structure permettant de stocker la valeur de cout d'utilisation d'une VM i sur une machine j allumée
	short int indiceVM;
	float cout;
}ctij;

typedef struct{ //Structure permettant de stocker la valeur de cout d'utilisation d'une VM i sur une machine j non allumée
	short int indicePM;
	float gain;///It's rather the cost of executing this job on a newly started machine 
}gaj;

typedef struct{ 
	short int IndiceTache;
	short int IndiceMachine;
	bool affecter;
	///On défini que la duree d'un instant T suppose que cet instant est fini.
	int dureeExe; //permet de stocker le temps d'execution réel de la tache IndiceVM afin de respecter la contrainte de mti
	int dureeSus; //permet de stocker le temps durant lequel la tâche i a ét?suspendu
}Ordo;

typedef struct{///For a given interval, this structure stock the number of running machine
	int NbServeurOn;
	int IndiceServeur;
}NbServeurOn;

typedef struct{
				Serveur ListOfServer[MaxMachines];
				CaractServeur ListOfServeurbis[MaxTimeHorizon][MaxMachines]; ///Les ressources restantes
				Reseau ListOfReseau[MaxTimeHorizon][MaxEdges][MaxMachines];
				ServeurON ListOfServerOn[MaxMachines];
				NbServeurOn ListOfNbServeurOn[MaxTimeHorizon];
				Interval ListOfIntervalles[MaxTimeHorizon];
				Pream ListofTasksPr[MaxTasks];
				//Listes des taches non préamtable GPU
				HDDRAM ListOfTasks1GPU[MaxTasks];
				RAMHDD ListOfTasks2GPU[MaxTasks];
				//Listes des taches non préamtable CPU
				HDDRAM ListOfTasks1CPU[MaxTasks];
				RAMHDD ListOfTasks2CPU[MaxTasks];
				//Liste des taches préantable GPU
				HDDRAM ListOfTasks1GPUPr[MaxTasks];
				HDDRAM ListOfTasks1GPUMachinej[MaxTasks];
				RAMHDD ListOfTasks2GPUPr[MaxTasks];
				RAMHDD ListOfTasks2GPUMachinej[MaxTasks];
				//Liste des taches préantable CPU
				HDDRAM ListOfTasks1CPUPr[MaxTasks];
				HDDRAM ListOfTasks1CPUMachinej[MaxTasks];
				RAMHDD ListOfTasks2CPUPr[MaxTasks];	
				RAMHDD ListOfTasks2CPUMachinej[MaxTasks];
				ctij ListeOfctij[MaxTasks][MaxMachines];
				gaj ListeOfgaj[MaxMachines];
				Ordo  ListOfOrdo[MaxTimeHorizon][MaxTasks];
				int NbPr;
				int NbNoPrAffected; ///nb des tâches non-pré déjà affectées
				int NbPrAffected; ///nb des tâches pré déjà affectées
				int NbHDDRAMGPU;
				int NbHDDRAMGPUPr;
				int NbHDDRAMGPUMachinej;
				int NbRAMHDDGPUMachinej;
				int NbHDDRAMCPUMachinej;
				int NbRAMHDDCPUMachinej;
				int NbHDDRAMCPU;
				int NbHDDRAMCPUPr;
				int NbRAMHDDGPU;
				int NbRAMHDDGPUPr;
				int NbRAMHDDCPU;
				int NbRAMHDDCPUPr;
				int NbInterval; //Permet de stocker le nombre d'intervalle qui découpe l'horizon de planification
				int NbServeurOn; //Permet de stocker le nombre de machine physique allum?

}Trait;

extern Trait Traitement;

extern void Init();///Initialisation
extern void CalculInterval(void);
extern void CalculCoutNorm(void);
extern void CalculCoutAffectation();
extern void MaJReseau(unsigned int indiceServeur1, unsigned int indiceServeur2, unsigned int indice);
///extern void MaJServeur(unsigned int IndiceServeur, unsigned int indice);
extern bool CalculFesabiliteResau(unsigned tachei,unsigned machinej,unsigned int tacheaffectee);
extern void ConstructionListesTache(unsigned int indice);
///extern void ConstructionListesTacheNonPr(unsigned int indice);
///extern void ConstructionListesTachePr(unsigned int indice);
///extern void ConstructionListesTachePrbis(unsigned int indice);
///extern void ConstructionListeTachePr(unsigned int indice);
//extern void ConstructionListesTachePrMachineJ(unsigned int indice,unsigned int indiceServeur);
extern void CalculPrioEtTrier(Tache* listeTache, unsigned int nbTache, unsigned int indiceIntervalle,unsigned int indiceServeur);
///extern void CalculPrioGPU(unsigned int indiceServeur,unsigned int indice);
///extern void CalculPrioCPU(unsigned int indiceServeur,unsigned int indice);
extern void Ordonnancement(unsigned int indice);
extern void OrdoNoPr(Tache* listeTache, unsigned int nbTache, unsigned int indiceIntervalle,unsigned int indiceServeur, int & compteurAffect, bool canTurnOn=true);
///extern void OrdoPr(Tache* listeTache, unsigned int nbTache, unsigned int indiceIntervalle,unsigned int indiceServeur, unsigned int compteurAffect);
extern int GetDureeExeActuelle(unsigned int indice, unsigned int indiceVM);
///extern void OrdoGPU(unsigned int i,unsigned int indiceServeur);
///extern void OrdoGPUPr(unsigned int indice,unsigned int indiceServeur);
///extern void OrdoCPU(unsigned int i,unsigned int indiceServeur);
///extern void OrdoCPUPr(unsigned int indice,unsigned int indiceServeur);
///extern void OrdoTachePreSurServeurOn(unsigned int indice,unsigned int indiceServeur);
///extern void OrdoTachePreSurServeurOn(unsigned int indice);
extern int AllumageMachine(unsigned int indice);
///extern void OrdoGPUPreamtable(unsigned int indice, unsigned int indiceServeur);
///extern void AffectationGPUPre(unsigned int indice,unsigned int indiceServeur);
///extern void AffectationCPUPre(unsigned int indice,unsigned int indiceServeur);
extern int TotalCost(void);

///swap two elements
template <typename T> void Swap(T& v1, T& v2)
{
	T tmp = v1;
	v1 = v2;
	v2 = tmp;
}

template <typename T> void SortByPrio(T* arr, int size, bool isDecreased=true)
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
	if( j > 0 )SortByPrio(arr, j+1);
	if(i < size)SortByPrio(arr+i, size-i);
}
#endif