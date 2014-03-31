#include<stdlib.h>
#include<stdio.h>
#include<conio.h>
#include<math.h>
#include "RandGeneration.h"

// Definition of the scenarii for the instance generation
// Meaning of columns: N1 N2 N3 N4 N5 M1 M2 M3 M4
unsigned int ScNM [8][9]={
	{5,3,0,0,0,1,1,0,0},
	{6,5,0,0,0,2,1,0,0},
	{6,4,2,2,1,2,0,1,1},
	{6,5,3,2,2,2,1,1,1},
	{9,5,4,2,1,2,0,2,1},
	{7,5,4,5,3,2,1,2,1},
	{9,6,4,5,3,2,1,2,1}, //27+6
	{9,6,6,5,4,3,1,2,2}//30+8
	//{8,7,6,6,3,2,2,2,1},
	//{10,8,8,7,3,3,2,2,1}
//	{12,9,9,8,4,3,2,2,2}
};

// Definition of PMs' features
// Meaning of columns: CPU GPU RAM HDD vj ajc ajg ajr ajh
// One row per PM
unsigned int PMs [4][9]={
	{12,0,160,164,10,10,0,1,7},
	{147,0,1280,491,20,14,0,8,22},
	{333,0,2560,614,30,34,0,17,22},
	{147,6,1280,614,20,14,18,8,22}
};
unsigned int Beta_t=14; // Cost for turning on a machine at time t

// Definition of VMs' features
// Meaning of columns: CPU GPU RAM HDD Bandwidth
// One row per VM
unsigned int VMs [12][5]={
	{1,0,17,16,10},
	{2,0,37,41,10},
	{4,0,75,85,20},
	{8,0,150,196,20},
	{7,0,171,42,10},
	{13,0,342,85,20},
	{26,0,684,196,20},
	{5,0,17,35,10},
	{20,0,70,196,20},
	{36,0,230,196,30},
	{88,0,605,337,30},
	{34,2,220,196,30}
};

// Pre-assignement of VMs to PMs
// One colum per PM / One row per VM
// 1: can be assigned to / O: otherwise
unsigned int PreAss [12][4]={
	{1,1,1,1},
	{1,1,1,1},
	{1,1,1,1},
	{0,1,1,1},
	{0,1,1,1},
	{0,1,1,1},
	{0,1,1,1},
	{1,1,1,1},
	{0,1,1,1},
	{0,1,1,1},
	{0,1,1,1},
	{0,0,0,1},
};


int min(int a, int b)
{
 if (a<b) return (a);
 else return(b);
}

/****************************************************************************/
// Function GenerateRandomInstance
// Randomly generate an instance and write it to the data file
/****************************************************************************/

void GenerateRandomInstance(int iN1,int iN2, int iN3, int iN4, int iN5, int iM1, int iM2, int iM3, int iM4,int iTH, int iStep)
	/* iN1: the number of Standard Vms,
	   iN2: the number of VMs with High Memory Requirement,
	   iN3: the number of VMs with High CPU requirement,
	   iN4: the number of Cluster Compute VMs,
	   iN5: the number of Cluster GPU VMs,
	   iM1: the number of D3 PMs,
	   iM2: the number of D51 PMs,
	   iM3: the number of D52 PMs,
	   iM4: the number of D53 PMs,
	   iTH: the time horizon in minutes,
	   iStep: the time increment for discretization in minutes*/
{
 int iLoop,iLoop2,iInd,iLoop3;
 int iBdw[MaxTasks],iTaskType[MaxTasks];
 unsigned int *psiSize;

 SetT(iTH/iStep);

 SetN(iN1+iN2+iN3+iN4+iN5);
 for (iLoop=0;iLoop<iN1;iLoop++)
 { // We select randomly a task from SVM
  iInd=(int)(((float)rand()/(float)RAND_MAX)*4.0);
  if (iInd==4) iInd=3;
  Setnc(iLoop,VMs[iInd][0]);
  Setng(iLoop,VMs[iInd][1]);
  Setnr(iLoop,VMs[iInd][2]);
  Setnh(iLoop,VMs[iInd][3]);
  iBdw[iLoop]=VMs[iInd][4]*iStep*60;
  iTaskType[iLoop]=iInd;
  for (iLoop2=0;iLoop2<T();iLoop2++)
    if (((float)rand()/(float)RAND_MAX)<=0.8)
	  Setu(iLoop,iLoop2,1);
	else 
	  Setu(iLoop,iLoop2,0);
  if (((float)rand()/(float)RAND_MAX)<=0.5)
  {
	  SetR(iLoop,1);
      Setrho(iLoop,(short int)(((float)rand()/(float)RAND_MAX)*21000.0+1000.0));
  }	else 
  {
	  SetR(iLoop,0);
	  Setrho(iLoop,0);
  }
 }
 for (iLoop=iN1;iLoop<iN1+iN2;iLoop++)
 { // We select randomly a task from SVM
  iInd=(int)(((float)rand()/(float)RAND_MAX)*3.0+4.0);
  if (iInd==7) iInd=6;
  Setnc(iLoop,VMs[iInd][0]);
  Setng(iLoop,VMs[iInd][1]);
  Setnr(iLoop,VMs[iInd][2]);
  Setnh(iLoop,VMs[iInd][3]);
  iBdw[iLoop]=VMs[iInd][4]*iStep*60;
  iTaskType[iLoop]=iInd;
  for (iLoop2=0;iLoop2<T();iLoop2++)
    if (((float)rand()/(float)RAND_MAX)<=0.8)
	  Setu(iLoop,iLoop2,1);
	else 
	  Setu(iLoop,iLoop2,0);
  if (((float)rand()/(float)RAND_MAX)<=0.5)
  {
	  SetR(iLoop,1);
      Setrho(iLoop,(short int)(((float)rand()/(float)RAND_MAX)*21000.0+1000.0));
  }	else 
  {
	  SetR(iLoop,0);
	  Setrho(iLoop,0);
  }
 }
 for (iLoop=iN1+iN2;iLoop<iN1+iN2+iN3;iLoop++)
 { // We select randomly a task from SVM
  iInd=(int)(((float)rand()/(float)RAND_MAX)*2.0+7.0);
  if (iInd==9) iInd=8;
  Setnc(iLoop,VMs[iInd][0]);
  Setng(iLoop,VMs[iInd][1]);
  Setnr(iLoop,VMs[iInd][2]);
  Setnh(iLoop,VMs[iInd][3]);
  iBdw[iLoop]=VMs[iInd][4]*iStep*60;
  iTaskType[iLoop]=iInd;
  for (iLoop2=0;iLoop2<T();iLoop2++)
    if (((float)rand()/(float)RAND_MAX)<=0.8)
	  Setu(iLoop,iLoop2,1);
	else 
	  Setu(iLoop,iLoop2,0);
  if (((float)rand()/(float)RAND_MAX)<=0.5)
  {
	  SetR(iLoop,1);
      Setrho(iLoop,(short int)(((float)rand()/(float)RAND_MAX)*21000.0+1000.0));
  }	else 
  {
	  SetR(iLoop,0);
	  Setrho(iLoop,0);
  }
 }
 for (iLoop=iN1+iN2+iN3;iLoop<iN1+iN2+iN3+iN4;iLoop++)
 { // We select randomly a task from SVM
  iInd=(int)(((float)rand()/(float)RAND_MAX)*2.0+9.0);
  if (iInd==11) iInd=10;
  Setnc(iLoop,VMs[iInd][0]);
  Setng(iLoop,VMs[iInd][1]);
  Setnr(iLoop,VMs[iInd][2]);
  Setnh(iLoop,VMs[iInd][3]);
  iBdw[iLoop]=VMs[iInd][4]*iStep*60;
  iTaskType[iLoop]=iInd;
  for (iLoop2=0;iLoop2<T();iLoop2++)
    if (((float)rand()/(float)RAND_MAX)<=0.8)
	  Setu(iLoop,iLoop2,1);
	else 
	  Setu(iLoop,iLoop2,0);
  if (((float)rand()/(float)RAND_MAX)<=0.5)
  {
	  SetR(iLoop,1);
      Setrho(iLoop,(short int)(((float)rand()/(float)RAND_MAX)*21000.0+1000.0));
  }	else 
  {
	  SetR(iLoop,0);
	  Setrho(iLoop,0);
  }
 }
 for (iLoop=iN1+iN2+iN3+iN4;iLoop<iN1+iN2+iN3+iN4+iN5;iLoop++)
 { // We select randomly a task from SVM
  iInd=11;
  Setnc(iLoop,VMs[iInd][0]);
  Setng(iLoop,VMs[iInd][1]);
  Setnr(iLoop,VMs[iInd][2]);
  Setnh(iLoop,VMs[iInd][3]);
  iBdw[iLoop]=VMs[iInd][4]*iStep*60;
  iTaskType[iLoop]=iInd;
  for (iLoop2=0;iLoop2<T();iLoop2++)
    if (((float)rand()/(float)RAND_MAX)<=0.8)
	  Setu(iLoop,iLoop2,1);
	else 
	  Setu(iLoop,iLoop2,0);
  if (((float)rand()/(float)RAND_MAX)<=0.5)
  {
	  SetR(iLoop,1);
      Setrho(iLoop,(short int)(((float)rand()/(float)RAND_MAX)*21000.0+1000.0));
  }	else 
  {
	  SetR(iLoop,0);
	  Setrho(iLoop,0);
  }
 }

 // We generate the affinities and the bandwidth required
 for (iLoop=0;iLoop<N();iLoop++)
	 for (iLoop2=0;iLoop2<N();iLoop2++)
	 {
		 Seta(iLoop,iLoop2,0);
		 Setb(iLoop,iLoop2,0);
	 }
 for (iLoop=0;iLoop<N();iLoop++)
	 for (iLoop2=iLoop;iLoop2<N();iLoop2++)
		if (iLoop!=iLoop2)
		    if (((float)rand()/(float)RAND_MAX)<=0.2)
			{
					Seta(iLoop,iLoop2,1);
					Seta(iLoop2,iLoop,1);
					Setb(iLoop,iLoop2,min(iBdw[iLoop],iBdw[iLoop2]));
			} else 	
			{ 
				Seta(iLoop,iLoop2,0);
				Seta(iLoop2,iLoop,0);
			}
		else Setb(iLoop,iLoop,BandwidthMigration*iStep*60);
 
 // We now turn to the generation of PMs
 SetM(iM1+iM2+iM3+iM4);
 for (iLoop=0;iLoop<T();iLoop++)
	Setbeta(iLoop,Beta_t);
 for (iLoop=0;iLoop<iM1;iLoop++)
 { // We select randomly a machine from D3
  Setmc(iLoop,PMs[0][0]);
  Setmg(iLoop,PMs[0][1]);
  Setmr(iLoop,PMs[0][2]);
  Setmh(iLoop,PMs[0][3]);
  Setv(iLoop,PMs[0][4]*iStep*60);
  Setalphac(iLoop,PMs[0][5]);
  Setalphag(iLoop,PMs[0][6]);
  Setalphar(iLoop,PMs[0][7]);
  Setalphah(iLoop,PMs[0][8]);
  for (iLoop2=0;iLoop2<N();iLoop2++)
	  Setq(iLoop2,iLoop,PreAss[iTaskType[iLoop2]][0]);
 }
 for (iLoop=iM1;iLoop<iM1+iM2;iLoop++)
 { // We select randomly a machine from D51
  Setmc(iLoop,PMs[1][0]);
  Setmg(iLoop,PMs[1][1]);
  Setmr(iLoop,PMs[1][2]);
  Setmh(iLoop,PMs[1][3]);
  Setv(iLoop,PMs[1][4]*iStep*60);
  Setalphac(iLoop,PMs[1][5]);
  Setalphag(iLoop,PMs[1][6]);
  Setalphar(iLoop,PMs[1][7]);
  Setalphah(iLoop,PMs[1][8]);
   for (iLoop2=0;iLoop2<N();iLoop2++)
	  Setq(iLoop2,iLoop,PreAss[iTaskType[iLoop2]][1]);
}
 for (iLoop=iM1+iM2;iLoop<iM1+iM2+iM3;iLoop++)
 { // We select randomly a machine from D52
  Setmc(iLoop,PMs[2][0]);
  Setmg(iLoop,PMs[2][1]);
  Setmr(iLoop,PMs[2][2]);
  Setmh(iLoop,PMs[2][3]);
  Setv(iLoop,PMs[2][4]*iStep*60);
  Setalphac(iLoop,PMs[2][5]);
  Setalphag(iLoop,PMs[2][6]);
  Setalphar(iLoop,PMs[2][7]);
  Setalphah(iLoop,PMs[2][8]);
  for (iLoop2=0;iLoop2<N();iLoop2++)
	  Setq(iLoop2,iLoop,PreAss[iTaskType[iLoop2]][2]);
 }
 for (iLoop=iM1+iM2+iM3;iLoop<iM1+iM2+iM3+iM4;iLoop++)
 { // We select randomly a machine from D53
  Setmc(iLoop,PMs[3][0]);
  Setmg(iLoop,PMs[3][1]);
  Setmr(iLoop,PMs[3][2]);
  Setmh(iLoop,PMs[3][3]);
  Setv(iLoop,PMs[3][4]*iStep*60);
  Setalphac(iLoop,PMs[3][5]);
  Setalphag(iLoop,PMs[3][6]);
  Setalphar(iLoop,PMs[3][7]);
  Setalphah(iLoop,PMs[3][8]);
  for (iLoop2=0;iLoop2<N();iLoop2++)
	  Setq(iLoop2,iLoop,PreAss[iTaskType[iLoop2]][3]);
 }

 // We now turn to the generation of the network
 Setmaxb(MaxBandwidth*iStep*60);
 SetMigrateBdw(BandwidthMigration*iStep*60);
 SetNbEdges(2*M()-1);

 psiSize=(unsigned int*)malloc(sizeof(unsigned int)*NbEdges());
 for (iLoop=0;iLoop<NbEdges();iLoop++)
	 psiSize[iLoop]=0;
 for (iLoop=0;iLoop<M();iLoop++)
 {
  // Left Child node from the current switch i
  for (iLoop2=iLoop-1;iLoop2>=0;iLoop2--)
  {
   SetCoupleMachines(iLoop*2,psiSize[iLoop*2],iLoop2,iLoop);
   psiSize[iLoop*2]++;
  }
  for (iLoop2=iLoop+1;iLoop2<M();iLoop2++)
  {
   SetCoupleMachines(iLoop*2,psiSize[iLoop*2],iLoop,iLoop2);
   psiSize[iLoop*2]++;
  }
  // Right Child node from the current switch i
  for (iLoop2=0;iLoop2<=iLoop;iLoop2++)
	  for (iLoop3=iLoop+1;iLoop3<M();iLoop3++)
		{
		 SetCoupleMachines(iLoop*2+1,psiSize[iLoop*2+1],iLoop2,iLoop3);
		 psiSize[iLoop*2+1]++;
		}
 }
 for (iLoop=0;iLoop<NbEdges();iLoop++)
	 SetNbMachEdge(iLoop,psiSize[iLoop]);
 free(psiSize);


 WriteData();
}
