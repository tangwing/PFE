#ifndef DATA
#define DATA 100

#include<stdlib.h>
#include<stdio.h>
#include<conio.h>

#define MAXVALUE 1000000000.0
#define MaxTasks 500
#define MaxMachines 10
#define MaxEdges 1024
#define MaxTimeHorizon 30

typedef struct{ // Data structure associated to the tasks
				short int QtyCPU;
				short int QtyGPU;
				short int QtyRAM;
				short int QtyHDD;
				short int isPreemptable; // 1 if the task is preemptable, 0 otherwise
				short int CostPreemption;
				short int LIsToBeProcessed[MaxTimeHorizon]; // A boolean indicating if at a time t a task is likely to be processed (uit)
				short int LAffinities[MaxTasks]; // A boolean indicating if the current task has an affinity with another task (aij)
				short int LPreAssignement[MaxMachines]; // A boolean indicating if the current VM is preassigned to a machine (qik)

}SDEFTask;

typedef struct{ // Data structure associated to the machines
				short int CostCPU;
				short int CostGPU;
				short int CostRAM;
				short int CostHDD;
				short int QtyCPU;
				short int QtyGPU;
				short int QtyRAM;
				short int QtyHDD;
				short int SpeedContext; // data vj
}SDEFMachines;

typedef struct{ // Data structure associated to the network
				short int NbEdges; // The number of links in the network
				short int MigrateBandwidth; // The bandwidth reserved for a job migration (bii)
				short int MaxBandwidth; // The maximum bandwidth of any link (b)
				short int ComBandwidth[MaxTasks][MaxTasks]; // data bij 
				short int NbMachinesByEdge[MaxEdges]; // The number of couples of machines (j,j') usigne the links in the network
				short int ListOfMachinesByEdge[MaxEdges][MaxMachines*MaxMachines][2]; // The Pll'
}SDEFNetwork;

typedef struct {
                int TimeHorizon;
				int NbTasks;
				int NbMachines;
				SDEFTask ListOfTasks[MaxTasks];
				SDEFMachines ListOfMachines[MaxMachines];
				SDEFNetwork Network;
				short int CostTurnOn[MaxTimeHorizon]; // The cost for turning on a machine at any time t (\beta_t)
} SDEFData;

/**********************************************************************************************/
// Declaration of the general data structure: it contains all the information about the instance
extern SDEFData Data;

/**********************************************************************************************/
// List of functions available to manipulate the data

// Function for reading/displaying the instance from file donnees.dat
extern void GetData(void); 
extern void DisplayData(void);
extern void WriteData(void);


// List of accessors to the data
extern int T(); // Returns the length of the time horizon
extern int N(); // Returns the number of tasks
extern int M(); // Returns the number of machines
// For tasks
extern short int nc(unsigned int i); // Returns the required CPU load of task i
extern short int ng(unsigned int i); // Returns the required GPU load of task i
extern short int nr(unsigned int i); // Returns the required RAM of task i
extern short int nh(unsigned int i); // Returns the required harddrive of task i
extern short int u(unsigned int i, unsigned int t); // Returns if task i is likely to be processed at time t
extern short int a(unsigned int i, unsigned int k); // Returns 1 if tasks i and k have an affinity; 0 otherwise
extern short int q(unsigned int i, unsigned int j); // Returns 1 if task i can be assigned to machine j; 0 otherwise
extern short int b(unsigned int i, unsigned int j); // Returns the bandwidth required by tasks i and j to communicate over the network
extern short int R(unsigned int i); // Returns 1 if task i is preemptable; 0 otherwise
extern short int rho(unsigned int i); // Returns the unitary penalty induced by preempting taks i
extern short int mt(unsigned int i); // Returns the migration time of task i
extern short int rt(unsigned int i, unsigned int j); //Returns the resuming time of task i on machine j
// For machines
extern short int mc(unsigned int j); // Returns the maximum accepted CPU load of machine j
extern short int mg(unsigned int j); // Returns the maximum accepted GPU load of machine j
extern short int mr(unsigned int j); // Returns the maximum accepted RAM load of machine j
extern short int mh(unsigned int j); // Returns the maximum accepted harddrive load of machine j
extern short int alphac(unsigned int j); // Returns the cost for using CPU of machine j
extern short int alphag(unsigned int j); // Returns the cost for using GPU of machine j
extern short int alphar(unsigned int j); // Returns the cost for using RAM of machine j
extern short int alphah(unsigned int j); // Returns the cost for using Harddrive of machine j
extern short int beta(unsigned int t); // Returns the cost for turning on a machine at time t
extern short int v(unsigned int j); // Returns the speed of machine j in loading a processing context
// For the network
extern short int maxb(); // Returns the maximum bandwidth of any edge
extern short int MigrateBdw(); // Returns the bandwidth used for migration
extern short int NbEdges(); // Returns the number of Edges in the network
extern short int NbMachEdge(unsigned int e); // Returns the number of couples of machines that use the edge e
extern void CoupleMachines(unsigned int e, unsigned int pos, unsigned int & Mach1, unsigned int & Mach2); // Mach1 and Mach2 are the machines in position pos in the list of couples usinge the edge e

// Below are the setter functions: they are used to instanciate the data structures
extern void SetT(short int val); // Sets the length of the time horizon
extern void SetN(short int val); // Sets the number of tasks
extern void SetM(short int val); // Sets the number of machines
// For tasks
extern void Setnc(unsigned int i, short int val); // Sets the required CPU load of task i
extern void Setng(unsigned int i, short int val); // Sets the required GPU load of task i
extern void Setnr(unsigned int i, short int val); // Sets the required RAM of task i
extern void Setnh(unsigned int i, short int val); // Sets the required harddrive of task i
extern void Setu(unsigned int i, unsigned int t, short int val); // Sets if task i is likely to be processed at time t
extern void Seta(unsigned int i, unsigned int k, short int val); // Sets 1 if tasks i and k have an affinity; 0 otherwise
extern void Setq(unsigned int i, unsigned int j, short int val); // Sets 1 if task i can be assigned to machine j; 0 otherwise
extern void Setb(unsigned int i, unsigned int j, short int val); // Sets the bandwidth required by tasks i and j to communicate over the network
extern void SetR(unsigned int i, short int val); // Sets 1 if task i is preemptable; 0 otherwise
extern void Setrho(unsigned int i, short int val); // Sets the unitary penalty induced by preempting taks i
// For machines
extern void Setmc(unsigned int j, short int val); // Sets the maximum accepted CPU load of machine j
extern void Setmg(unsigned int j, short int val); // Sets the maximum accepted GPU load of machine j
extern void Setmr(unsigned int j, short int val); // Sets the maximum accepted RAM load of machine j
extern void Setmh(unsigned int j, short int val); // Sets the maximum accepted harddrive load of machine j
extern void Setalphac(unsigned int j, short int val); // Sets the cost for using CPU of machine j
extern void Setalphag(unsigned int j, short int val); // Sets the cost for using GPU of machine j
extern void Setalphar(unsigned int j, short int val); // Sets the cost for using RAM of machine j
extern void Setalphah(unsigned int j, short int val); // Sets the cost for using Harddrive of machine j
extern void Setbeta(unsigned int t, short int val); // Sets the cost for turning on a machine at time t
extern void Setv(unsigned int j, short int val); // Sets the speed of machine j in loading a processing context
// For the network
extern void Setmaxb(short int val); // Sets the maximum bandwidth of any edge
extern void SetMigrateBdw(short int val); // Sets the bandwidth used for migration
extern void SetNbEdges(short int val); // Sets the number of Edges in the network
extern void SetNbMachEdge(unsigned int e, short int val); // Sets the number of couples of machines that use the edge e
extern void SetCoupleMachines(unsigned int e, unsigned int pos, unsigned int Mach1, unsigned int  Mach2); // Mach1 and Mach2 are the machines in position pos in the list of couples usinge the edge e

#endif
