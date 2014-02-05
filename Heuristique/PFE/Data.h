#ifndef DATA
#define DATA

#define MAXVALUE 1000000000.0
#define MaxTasks 500
#define MaxMachines 10
#define MaxEdges 1024
#define MaxTimeHorizon 30

typedef struct{ // Data structure associated to the tasks
				int QtyCPU;
				int QtyGPU;
				int QtyRAM;
				int QtyHDD;
				int isPreemptable; // 1 if the task is preemptable, 0 otherwise
				int CostPreemption;
				int LIsToBeProcessed[MaxTimeHorizon]; // A boolean indicating if at a time t a task is likely to be processed (uit)
				int LAffinities[MaxTasks]; // A boolean indicating if the current task has an affinity with another task (aij)
				int LPreAssignement[MaxMachines]; // A boolean indicating if the current VM is preassigned to a machine (qik)

}SDEFTask;

typedef struct{ // Data structure associated to the machines
				int CostCPU;
				int CostGPU;
				int CostRAM;
				int CostHDD;
				int QtyCPU;
				int QtyGPU;
				int QtyRAM;
				int QtyHDD;
				int SpeedContext; // data vj
}SDEFMachines;

typedef struct{ // Data structure associated to the network
				int NbEdges; // The number of links in the network
				int MigrateBandwidth; // The bandwidth reserved for a job migration (bii)
				int MaxBandwidth; // The maximum bandwidth of any link (b)
				int ComBandwidth[MaxTasks][MaxTasks]; // data bij 
				int NbMachinesByEdge[MaxEdges]; // The number of couples of machines (j,j') usigne the links in the network
				///The second dim = max number of machine pairs
				int ListOfMachinesByEdge[MaxEdges][MaxMachines*MaxMachines][2]; // The Pll'
}SDEFNetwork;

typedef struct {
                int TimeHorizon;
				int NbTasks;
				int NbMachines;
				SDEFTask ListOfTasks[MaxTasks];
				SDEFMachines ListOfMachines[MaxMachines];
				SDEFNetwork Network;
				int CostTurnOn[MaxTimeHorizon]; // The cost for turning on a machine at any time t (\beta_t)
} SDEFData;

/**********************************************************************************************/
// Declaration of the general data structure: it contains all the information about the instance
extern SDEFData Data;

/**********************************************************************************************/
// List of functions available to manipulate the data

// Function for reading/displaying the instance from file donnees.dat
extern void GetData(char*filename = 0); 
extern void DisplayData(void);


// List of accessors to the data
extern int T(); // Returns the length of the time horizon
extern int N(); // Returns the number of tasks
extern int M(); // Returns the number of machines
// For tasks
extern int nc(unsigned int i); // Returns the required CPU load of task i
extern int ng(unsigned int i); // Returns the required GPU load of task i
extern int nr(unsigned int i); // Returns the required RAM of task i
extern int nh(unsigned int i); // Returns the required harddrive of task i
extern int u(unsigned int i, unsigned int t); // Returns if task i is likely to be processed at time t
extern int a(unsigned int i, unsigned int k); // Returns 1 if tasks i and k have an affinity; 0 otherwise
extern int q(unsigned int i, unsigned int j); // Returns 1 if task i can be assigned to machine j; 0 otherwise
extern int b(unsigned int i, unsigned int j); // Returns the bandwidth required by tasks i and j to communicate over the network
extern int R(unsigned int i); // Returns 1 if task i is preemptable; 0 otherwise
extern int rho(unsigned int i); // Returns the unitary penalty induced by preempting taks i
extern int mt(unsigned int i); // Returns the migration time of task i
extern int rt(unsigned int i, unsigned int j); //Returns the resuming time of task i on machine j
// For machines
extern int mc(unsigned int j); // Returns the maximum accepted CPU load of machine j
extern int mg(unsigned int j); // Returns the maximum accepted GPU load of machine j
extern int mr(unsigned int j); // Returns the maximum accepted RAM load of machine j
extern int mh(unsigned int j); // Returns the maximum accepted harddrive load of machine j
extern int alphac(unsigned int j); // Returns the cost for using CPU of machine j
extern int alphag(unsigned int j); // Returns the cost for using GPU of machine j
extern int alphar(unsigned int j); // Returns the cost for using RAM of machine j
extern int alphah(unsigned int j); // Returns the cost for using Harddrive of machine j
extern int beta(unsigned int t); // Returns the cost for turning on a machine at time t
extern int v(unsigned int j); // Returns the speed of machine j in loading a processing context
// For the network
extern int maxb(); // Returns the maximum bandwidth of any edge
extern int NbEdges(); // Returns the number of Edges in the network
extern int NbMachEdge(unsigned int e); // Returns the number of couples of machines that use the edge e
extern void CoupleMachines(unsigned int e, unsigned int pos, unsigned int &Mach1, unsigned int &Mach2); // Mach1 and Mach2 are the machines in position pos in the list of couples usinge the edge e

#endif