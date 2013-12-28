#include<stdlib.h>
#include<stdio.h>
#include<conio.h>
#include<math.h>
#include "Data.h"
#include "ConsoleTable.h"

SDEFData Data;

/****************************************************************************/
// List of accessors to the data
/****************************************************************************/

int T() // Returns the length of the time horizon
{ return Data.TimeHorizon;}
int N() // Returns the number of tasks
{ return Data.NbTasks;}
int M() // Returns the number of machines
{ return Data.NbMachines; }

// For tasks
int nc(unsigned int i) // Returns the required CPU load of task i
{ return Data.ListOfTasks[i].QtyCPU ;}
int ng(unsigned int i) // Returns the required GPU load of task i
{ return Data.ListOfTasks[i].QtyGPU ;}
int nr(unsigned int i) // Returns the required RAM of task i
{ return Data.ListOfTasks[i].QtyRAM ;}
int nh(unsigned int i) // Returns the required harddrive of task i
{ return Data.ListOfTasks[i].QtyHDD ;}
int u(unsigned int i, unsigned int t) // Returns if task i is likely to be processed at time t
{ return Data.ListOfTasks[i].LIsToBeProcessed[t] ;}
int a(unsigned int i, unsigned int k) // Returns 1 if tasks i and k have an affinity; 0 otherwise
{ return Data.ListOfTasks[i].LAffinities[k];}
int q(unsigned int i, unsigned int j) // Returns 1 if task i can be assigned to machine j; 0 otherwise
{ return Data.ListOfTasks[i].LPreAssignement[j] ;}
int b(unsigned int i, unsigned int j) // Returns the bandwidth required by tasks i and j to communicate over the network
{ return i<=j? Data.Network.ComBandwidth[i][j]:Data.Network.ComBandwidth[j][i] ;}
int R(unsigned int i) // Returns 1 if task i is preemptable; 0 otherwise
{ return Data.ListOfTasks[i].isPreemptable ;}
int rho(unsigned int i) // Returns the unitary penalty induced by preempting taks i
{ return Data.ListOfTasks[i].CostPreemption ;}
int mt(unsigned int i) // Returns the migration time of task i
{ return (int)ceil((double)(nh(i)+nr(i))/(double)b(i,i));}
int rt(unsigned int i, unsigned int j) //Returns the resuming time of task i on machine j
{  return (int)ceil((double)nr(i)/(double)v(j));}

// For machines
int mc(unsigned int j) // Returns the maximum accepted CPU load of machine j
{ return  Data.ListOfMachines[j].QtyCPU ;}
int mg(unsigned int j) // Returns the maximum accepted GPU load of machine j
{ return  Data.ListOfMachines[j].QtyGPU ;}
int mr(unsigned int j) // Returns the maximum accepted RAM load of machine j
{ return  Data.ListOfMachines[j].QtyRAM ;}
int mh(unsigned int j) // Returns the maximum accepted harddrive load of machine j
{ return  Data.ListOfMachines[j].QtyHDD ;}
int alphac(unsigned int j) // Returns the cost for using CPU of machine j
{ return  Data.ListOfMachines[j].CostCPU ;}
int alphag(unsigned int j) // Returns the cost for using GPU of machine j
{ return  Data.ListOfMachines[j].CostGPU ;}
int alphar(unsigned int j) // Returns the cost for using RAM of machine j
{ return  Data.ListOfMachines[j].CostRAM ;}
int alphah(unsigned int j) // Returns the cost for using Harddrive of machine j
{ return  Data.ListOfMachines[j].CostHDD ;}
int beta(unsigned int t) // Returns the cost for turning on a machine at time t
{ return  Data.CostTurnOn[t] ;}
int v(unsigned int j) // Returns the speed of machine j in loading a processing context
{ return  Data.ListOfMachines[j].SpeedContext ;}

// For the network
int maxb() // Returns the maximum bandwidth of any edge
{ return  Data.Network.MaxBandwidth ;}
int NbEdges() // Returns the number of Edges in the network
{ return  Data.Network.NbEdges ;}
int NbMachEdge(unsigned int e) // Returns the number of couples of machines that use the edge e
{ return  Data.Network.NbMachinesByEdge[e];}
void CoupleMachines(unsigned int e, unsigned int pos, unsigned int &Mach1, unsigned int &Mach2) // Mach1 and Mach2 are the machines in position pos in the list of couples usinge the edge e
{
	Mach1=Data.Network.ListOfMachinesByEdge[e][pos][0]-1;	///The first machine of the pos'th couple who use the e'th edge
	Mach2=Data.Network.ListOfMachinesByEdge[e][pos][1]-1;
}

/****************************************************************************/
// Function DisplayData
// Print on screen the instance read previously by GetData()
// NB: the printing is done via the getters
/****************************************************************************/

void DisplayData(void)
{
	int iTask;

	printf("Display of the instance:\n");
	printf("\t Time Horizon T: %ld\n",T());
	printf("\t Number of Tasks N: %ld\n",N());
	printf("\t Number of Machines M: %ld\n",M());
	ConsoleTable ct("Tasks", N(), 6);
	ct.SetColHeader(0, "QtyCPU")
		.SetColHeader(1, "QtyGPU")
		.SetColHeader(2, "QtyRAM")
		.SetColHeader(3, "QtyHDD")
		.SetColHeader(4, "IsPreemp")
		.SetColHeader(5, "CostPreemp");
	for (iTask=0;iTask<N();iTask++) 
	{
		ct.Print(nc(iTask));
		ct.Print(ng(iTask));
		ct.Print(nr(iTask));
		ct.Print(nh(iTask));
		ct.Print(R(iTask));
		ct.Print(rho(iTask));
	}
}

/****************************************************************************/
// Function GetData
// Read the instance from donnees.dat
/****************************************************************************/
void GetData(void)
{
 int num,ri_temp,pi_temp,j,i;
 FILE *file;
 int iLoop,iLoop2,iLoop3,iLoop4,iNbJobs;

 //file=fopen("Donnees/donnees1_2.dat","rt");
 file=fopen("donnees.dat","rt");
 fscanf(file,"%d\n",&Data.TimeHorizon);
 fscanf(file,"%d\n",&Data.NbTasks);
 fscanf(file,"%d\n",&Data.NbMachines);

 // Reading of the information related to the tasks
 // Each line of the input file can be decomposed as follows: Qty of CPU, Qty of GPU, Qty of RAM, Qty of Harddrive, Ri boolean indicating if preemptable, Cost for suspending, 
 for (iLoop=0;iLoop<Data.NbTasks;iLoop++)
	 fscanf(file,"%hd %hd %hd %hd %hd %hd\n",&Data.ListOfTasks[iLoop].QtyCPU,&Data.ListOfTasks[iLoop].QtyGPU,&Data.ListOfTasks[iLoop].QtyRAM, &Data.ListOfTasks[iLoop].QtyHDD, &Data.ListOfTasks[iLoop].isPreemptable,&Data.ListOfTasks[iLoop].CostPreemption);

 // We read the uit data
 for (iLoop=0;iLoop<Data.NbTasks;iLoop++)
 {
	 for (iLoop2=0;iLoop2<Data.TimeHorizon;iLoop2++) // We read the variables uit: 1 if task i has to be processed at time t
		fscanf(file,"%hd ",&Data.ListOfTasks[iLoop].LIsToBeProcessed[iLoop2]); 
	 fscanf(file,"\n");
 }

 // We read the matrix of affinities A=(aij)
 for (iLoop=0;iLoop<Data.NbTasks;iLoop++)
 {
	 for (iLoop2=0;iLoop2<Data.NbTasks;iLoop2++) 
			 fscanf(file,"%hd ",&Data.ListOfTasks[iLoop].LAffinities[iLoop2]); 
	 fscanf(file,"\n");
 }

 // We read the matrix of preassignments Q=(qi,k)
 for (iLoop=0;iLoop<Data.NbTasks;iLoop++)
 {
	 for (iLoop2=0;iLoop2<Data.NbMachines;iLoop2++) 
			 fscanf(file,"%hd ",&Data.ListOfTasks[iLoop].LPreAssignement[iLoop2]); 
	 fscanf(file,"\n");
 }

 // Reading of data related to the machines
 for (iLoop=0;iLoop<Data.NbMachines;iLoop++) // We read the CPU, GPU, RAM, Harddrive availability of each machine and at last the speed in loading contexts (vj)
	 fscanf(file,"%hd %hd %hd %hd %hd\n",&Data.ListOfMachines[iLoop].QtyCPU,&Data.ListOfMachines[iLoop].QtyGPU,&Data.ListOfMachines[iLoop].QtyRAM,&Data.ListOfMachines[iLoop].QtyHDD,&Data.ListOfMachines[iLoop].SpeedContext);
 for (iLoop=0;iLoop<Data.NbMachines;iLoop++) // We read the cost for using CPU, GPU, RAM, Harddrive on each machine
	 fscanf(file,"%hd %hd %hd %hd\n",&Data.ListOfMachines[iLoop].CostCPU,&Data.ListOfMachines[iLoop].CostGPU,&Data.ListOfMachines[iLoop].CostRAM,&Data.ListOfMachines[iLoop].CostHDD);
 for (iLoop=0;iLoop<Data.TimeHorizon;iLoop++)
	 fscanf(file,"%hd ",&Data.CostTurnOn[iLoop]);

 // Reading of data related to the network
 fscanf(file,"%hd\n",&Data.Network.MaxBandwidth); // We read the maximum bandwidth of any link (b)
 fscanf(file,"%hd\n",&Data.Network.MigrateBandwidth); // We read the bandwidth reserved for a migration (bii)
 for (iLoop=0;iLoop<Data.NbTasks;iLoop++)
 {
	 for (iLoop2=0;iLoop2<Data.NbTasks;iLoop2++) 
			 fscanf(file,"%hd ",&Data.Network.ComBandwidth[iLoop][iLoop2]); 
	 fscanf(file,"\n");
 }
 // We set the bii into the matrix ComBandwidgth
  for (iLoop=0;iLoop<Data.NbTasks;iLoop++)
	Data.Network.ComBandwidth[iLoop][iLoop]=Data.Network.MigrateBandwidth;
 
  // We read the network structure, link by link
  fscanf(file,"%hd\n",&Data.Network.NbEdges);
  for (iLoop=0;iLoop<Data.Network.NbEdges;iLoop++)
  {
	  fscanf(file,"%hd ",&Data.Network.NbMachinesByEdge[iLoop]);
	  for (iLoop2=0;iLoop2<Data.Network.NbMachinesByEdge[iLoop];iLoop2++)
		  fscanf(file,"%hd %hd ",&Data.Network.ListOfMachinesByEdge[iLoop][iLoop2][0],&Data.Network.ListOfMachinesByEdge[iLoop][iLoop2][1]);
	  fscanf(file,"\n");
  }
 fclose(file); 
}

