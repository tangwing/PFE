#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <math.h>
#include <ctime>
#include <process.h>
#include <Windows.h>
#include "Preprocessing.h"
#include "Data.h"
#include "Utility.h"
#include "CplexResult.h"

#define DEBUG false
#define DEBUG_MOD false
#define CONFIG true

#define MemLimit 1024.0
#define TimeLimit 1800.0

using namespace std;

#include <ilcplex/ilocplex.h>

ILOSTLBEGIN
typedef IloIntVarArray IloIntVarArray1D; // 1-dimension array of integer variables
typedef IloArray <IloIntVarArray> IloIntVarArray2D; // 2-dimension array of integer variables
typedef IloArray <IloArray <IloIntVarArray> > IloIntVarArray3D; // 3-dimension array of integer variables
typedef IloArray <IloArray <IloArray <IloIntVarArray> > > IloIntVarArray4D; // 4-dimension array of integer variables
typedef IloArray <IloArray <IloArray <IloArray <IloIntVarArray> > > > IloIntVarArray5D; // 5-dimension array of integer variables

// Data structure from CPLEX necessary to create the model
IloEnv env;
IloModel model(env);
IloNumVarArray var(env);
IloRangeArray con(env);
IloIntVarArray3D x(env);
IloIntVarArray5D y(env);
IloIntVarArray2D z(env);
IloIntVarArray1D Z(env);
IloIntVarArray2D d(env);
IloIntVar RE(env,0,9999999);

double dOptValue, dOptTime;
int isOptimal, isFeasible,iNbNodesIP;
int isTimeLimit=0, isMemLimit=0;

void PreByCalCost(int begin, int end, int *head, int nbBool,int UB, IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar,IloRangeArray *pcon)
{
	FILE *fichier;
	double objValue;
	time_t temp1,temp2,tempPre1,tempPre2;
	Preprocessing *prepro=new Preprocessing();
	prepro->PRESetHead(head);
	prepro->PRESetDebug(DEBUG);
	prepro->PRESetTomlin(false);
	prepro->PRESetUB(UB);		// Fix the variables using the result of an heuristique algorithme Schrage as the UB
	
	pcplex->exportModel("model.lp");	
	time(&temp1);time(&tempPre1);
	prepro->PREInitializeLP(penv, pcplex , pmodel , pvar , pcon);
	prepro->PRESolveLP();
	if(!prepro->PREPreprocessing(head,nbBool)) // If shrage is the optimal solution, i.e. no preprocessing
	{
		objValue=min(prepro->PREGetLpOpt(),(double)UB);
		time(&tempPre2);
	}
	else
	{
		prepro->PREInitializeMIPfromLP(penv , pcplex , pmodel , pvar , pcon,head);
		time(&tempPre2);
		prepro->PRESolveMIP();
		objValue=prepro->PREGetMipOpt();
	}
	cout << "--------------------------" << endl;
	/*************The test of MIP********************************/
	time(&temp2);
	double temps_cpu=difftime(temp2,temp1);
	double temps_cpu_pre=difftime(tempPre2,tempPre1);
	fichier=fopen("optlp.txt","wt");
	fprintf(fichier,"%.2lf\n",objValue);
	fprintf(fichier,"%.2lf\n",temps_cpu);
	fprintf(fichier,"%.2lf%% \n",(100.0*double(prepro->PREGetNbFix())/double(nbBool)));
	fprintf(fichier,"%.2lf\n",temps_cpu_pre);
	fclose(fichier);
	delete prepro;
}
//***********************************************************************************
// This function write in the file solution.txt the optimal solution computed by CPLEX
//***********************************************************************************
void SolutionToFile(IloCplex & cplex)
{
 FILE *SolFic;
 int tmp;
 int iMach,iTask,iTime,iEdge;
 int iCPU[MaxTimeHorizon],iGPU[MaxTimeHorizon],iRAM[MaxTimeHorizon],iHDD[MaxTimeHorizon];

 SolFic=fopen("solution.txt","wt");
 fprintf(SolFic,"Optimal solution computed by CPLEX for an instance with:\n");
 fprintf(SolFic,"\t %d tasks\n\t %d machines\n\t Time horizon of %d\n\n",N(),M(),T());
 fprintf(SolFic,"Optimal solution value: %lf\n",dOptValue); 
 fprintf(SolFic,"Solution time: %lf\n\n",dOptTime);

 // We now write the schedule per machine for each time unit
 fprintf(SolFic,"Schedule per machine and for each time unit\n");
 for (iMach=0;iMach<M();iMach++)
 {
	 fprintf(SolFic,"Machine %d (CPU=%d / GPU=%d / RAM=%d / HDD=%d):\n",iMach+1,mc(iMach),mg(iMach),mr(iMach),mh(iMach));
	 fprintf(SolFic,"\t Tasks: ");
	 for (iTime=0;iTime<T();iTime++)
	 {
	  iCPU[iTime]=0; iGPU[iTime]=0; iRAM[iTime]=0; iHDD[iTime]=0; 
	  for (iTask=0;iTask<N();iTask++)
		  if (cplex.getValue(x[iTask][iMach][iTime])>0.999)
		  { // Task iTask is processed by iMach at time [iTime,iTime+1]
		   iCPU[iTime]+=nc(iTask); iGPU[iTime]+=ng(iTask); iRAM[iTime]+=nr(iTask); iHDD[iTime]+=nh(iTask);
		   fprintf(SolFic,"%d ",iTask+1);
		  }
	  fprintf(SolFic,"| ");
	 }
	 fprintf(SolFic,"\n");
	 fprintf(SolFic,"\t CPU:   ");
	 for (iTime=0;iTime<T();iTime++)
		 fprintf(SolFic,"%d | ",iCPU[iTime]);
	 fprintf(SolFic,"\n");
	 fprintf(SolFic,"\t GPU:   ");
	 for (iTime=0;iTime<T();iTime++)
		 fprintf(SolFic,"%d | ",iGPU[iTime]);
	 fprintf(SolFic,"\n");
	 fprintf(SolFic,"\t RAM:   ");
	 for (iTime=0;iTime<T();iTime++)
		 fprintf(SolFic,"%d | ",iRAM[iTime]);
	 fprintf(SolFic,"\n");
	 fprintf(SolFic,"\t HDD:   ");
	 for (iTime=0;iTime<T();iTime++)
		 fprintf(SolFic,"%d | ",iHDD[iTime]);
	 fprintf(SolFic,"\n");
 }
 // Schedule per task
 fprintf(SolFic,"\n Schedule per task and for each time unit\n");
 for (iTask=0;iTask<N();iTask++)
 {
	 fprintf(SolFic,"Task %d (isPreemptable=%d / migration time=%ld):\n",iTask+1,R(iTask),mt(iTask));
	 fprintf(SolFic,"  Resuming time per machine: ");
	 for (iMach=0;iMach<M();iMach++) fprintf(SolFic,"%d ",rt(iTask,iMach));
	 fprintf(SolFic,"\n");
	 fprintf(SolFic,"  Can be assigned only to machines: ");
	 for (iMach=0;iMach<M();iMach++) 
		 if (q(iTask,iMach)==1) fprintf(SolFic,"%d ",iMach+1);
	 fprintf(SolFic,"\n");
	 fprintf(SolFic,"\t is Processed at times (machine): ");
	 for (iTime=0;iTime<T();iTime++)
	 {
	  for (iMach=0;iMach<M();iMach++)
		  if ((tmp=cplex.getValue(x[iTask][iMach][iTime]))>0.999)
		  { // Task iTask is processed by iMach at time [iTime,iTime+1]
		   fprintf(SolFic,"%d (%d) ",iTime+1,iMach+1);
		  }
	  fprintf(SolFic,"| ");
	 }
	 fprintf(SolFic,"\n");
	 fprintf(SolFic,"\t is likely to be proc?: ");
	 for (iTime=0;iTime<T();iTime++)
		   fprintf(SolFic,"%d | ",u(iTask,iTime));
	 fprintf(SolFic,"\n");
 }

 // Information on the network usage
 fprintf(SolFic,"\n Network usage by links\n Maximum Bandwith for each edge : %ld\n",maxb());
 for (iEdge=0;iEdge<NbEdges();iEdge++)
 {
	 fprintf(SolFic,"Edge %2d:\n\t Time: ",iEdge+1);
	 for (iTime=0;iTime<T();iTime++)
		fprintf(SolFic,"%d | ",iTime+1);
	 fprintf(SolFic,"\n\t Bdw used: ");
	 for (iTime=0;iTime<T();iTime++)
	 {
	    int iBdw=0, iLoop, iTask2;
		for (iLoop=0;iLoop<NbMachEdge(iEdge);iLoop++)
		{
		 unsigned int iMach1,iMach2,iSwap;
		 CoupleMachines(iEdge,iLoop,iMach1,iMach2);
		 if (iMach1>iMach2) 
			{ iSwap=iMach1; iMach1=iMach2; iMach2=iSwap;}
		 for (iTask=0;iTask<N();iTask++)
			 for (iTask2=iTask+1;iTask2<N();iTask2++)
				 if (a(iTask,iTask2)==1 
					 && (tmp=cplex.getValue(y[iTask][iTask2][iMach1][iMach2][iTime]))>0.999 )
						iBdw+=b(iTask,iTask2);
		 for (iTask=0;iTask<N();iTask++)
				 if (cplex.getValue(y[iTask][iTask][iMach1][iMach2][iTime])>0.999 || cplex.getValue(y[iTask][iTask][iMach2][iMach1][iTime])>0.999)
						iBdw+=b(iTask,iTask);
		}
		fprintf(SolFic,"%d | ",iBdw);
	 }
	 fprintf(SolFic,"\n");
 }

 fclose(SolFic);

// Now output of the variables as computed by CPLEX
 SolFic=fopen("ResCplex.txt","wt");
 fprintf(SolFic,"Variables xijt\n");
 for (iTask=0;iTask<N();iTask++)
 {
  fprintf(SolFic,"Variable x_%d_j_t (one line per machine, one column per time)\n",iTask+1);
  for (iMach=0;iMach<M();iMach++)
  {
   for (iTime=0;iTime<T();iTime++)
	   fprintf(SolFic,"%1.1lf ",cplex.getValue(x[iTask][iMach][iTime]));
   fprintf(SolFic,"\n");
  }
 }
 fprintf(SolFic,"Variables yiijjt\n");
 int iTask2,iMach2;
 for (iTask=0;iTask<N();iTask++)
 for (iTask2=iTask;iTask2<N();iTask2++)
 if (a(iTask,iTask2)==1 || iTask==iTask2)
 {
  fprintf(SolFic,"Variable y_%d_%d_j_j'_t (one line per couple of machines, one column per time)\n",iTask+1,iTask2+1);
  fflush(SolFic);

  for (iMach=0;iMach<M();iMach++)
  for (iMach2=0;iMach2<M();iMach2++)
  if ((iTask!=iTask2 && iMach<iMach2) || (iTask==iTask2 && iMach!=iMach2))
  {
   fprintf(SolFic,"(%d/%d) : \n",iMach+1,iMach2+1);
   for (iTime=0;iTime<T();iTime++)
	   fprintf(SolFic,"%1.1lf ",cplex.getValue(y[iTask][iTask2][iMach][iMach2][iTime]));
   fprintf(SolFic,"\n");
  fflush(SolFic);
  }
 } else fprintf(SolFic,"Variable y_%d_%d_j_j'_t has been removed by CPLEX since there is no affinity between these tasks\n",iTask+1,iTask2+1);

 fclose(SolFic);

}
//***********************************************************************************
// This function initializes the IP model
//***********************************************************************************
void InitializeIPModel()
{
 int iLoop,iLoop2,iLoop3,iLoop4,iLoop5,iLoop6,iLoop7,iLoop8;
 int gap[MaxTasks][MaxTimeHorizon];

 // Before creating the model we need to compute the gap(i,t) array: for each task i and time t, what is the smallest t'>t such that u_i_t'=1?
 for (iLoop=0;iLoop<N();iLoop++)
	 for (iLoop2=0;iLoop2<T();iLoop2++)
 {
  iLoop3=iLoop2+1;
  while (u(iLoop,iLoop3)==0 && iLoop3<T()) iLoop3++;
  gap[iLoop][iLoop2]=iLoop3;
  if (DEBUG_MOD) printf("%d ",gap[iLoop][iLoop2]);
 }
 if (DEBUG_MOD) getch();
 if (DEBUG_MOD) printf("[DEBUG] Starting to create the IP model: variables creation\n");
 /***************************/
 // We create the variables
 /***************************/
 // Variables xijt
 if (DEBUG_MOD) printf("Creation of variables x\n");
 for (iLoop=0;iLoop<N();iLoop++)
 {
	 IloIntVarArray2D tmp(env);
	 for (iLoop2=0;iLoop2<M();iLoop2++)
		 tmp.add(IloIntVarArray(env,T(),0,1));
	 x.add(tmp);
 }
 // Variables yii'jj't (CPLEX will automatically eliminate the redundant variables)
 if (DEBUG_MOD) printf("Creation of variables y\n");
 for (iLoop=0;iLoop<N();iLoop++)
 {
	 IloIntVarArray4D tmp1(env);
	 for (iLoop2=0;iLoop2<N();iLoop2++)
	 {
		 IloIntVarArray3D tmp2(env);
		 for (iLoop3=0;iLoop3<M();iLoop3++)
		 {
			IloIntVarArray2D tmp3(env);
			for (iLoop4=0;iLoop4<M();iLoop4++)
				tmp3.add(IloIntVarArray(env,T(),0,1));
			tmp2.add(tmp3);
		 }
		 tmp1.add(tmp2);
	 }
	 y.add(tmp1);
 }
 // Variables ztj
 if (DEBUG_MOD) printf("Creation of variables z\n");
 for (iLoop=0;iLoop<M();iLoop++)
	 z.add(IloIntVarArray(env,T(),0,1));
 // Variables zt
 if (DEBUG_MOD)  printf("Creation of variables Z\n");
 Z.add(IloIntVarArray(env,T(),0,9999999));
 // Variables dit
 if (DEBUG_MOD)  printf("Creation of variables d\n");
 for (iLoop=0;iLoop<N();iLoop++)
	 d.add(IloIntVarArray(env,T(),0,9999999));
 // Variable RE is created while its declaration


 if (DEBUG_MOD) printf("[DEBUG] Declaration of the objective function TC\n");
 /***************************/
 // We create the objectif function TC
 /***************************/
 IloExpr ObjTC(env);
 for (iLoop=0;iLoop<T();iLoop++)  // Cost for using the resources
	 for (iLoop2=0;iLoop2<N();iLoop2++)
		 for (iLoop3=0;iLoop3<M();iLoop3++)
			 ObjTC+=x[iLoop2][iLoop3][iLoop]*(alphac(iLoop3)*nc(iLoop2)+alphag(iLoop3)*ng(iLoop2)+alphah(iLoop3)*nh(iLoop2)+alphar(iLoop3)*nr(iLoop2));
 for (iLoop=0;iLoop<T();iLoop++) // Cost implied by the migration on the cost of resources
	 for (iLoop2=0;iLoop2<N();iLoop2++)
		 for (iLoop3=0;iLoop3<M();iLoop3++)
			for (iLoop5=0;iLoop5<M();iLoop5++)
			 if (iLoop3!=iLoop5)
				ObjTC+=y[iLoop2][iLoop2][iLoop3][iLoop5][iLoop]*(alphah(iLoop5)*nh(iLoop2)+alphar(iLoop5)*nr(iLoop2));
 for (iLoop=0;iLoop<T();iLoop++) 
	 for (iLoop2=0;iLoop2<N();iLoop2++)
	 {
		 IloExpr tmp(env);
		 for (iLoop3=0;iLoop3<M();iLoop3++)
			 tmp+=x[iLoop2][iLoop3][iLoop];
		 ObjTC+=rho(iLoop2)*u(iLoop2,iLoop)*(1.0-tmp);
	 }
 for (iLoop=0;iLoop<T();iLoop++)
	ObjTC+=beta(iLoop)*Z[iLoop];
 model.add(IloMinimize(env,ObjTC));

 if (DEBUG_MOD) printf("[DEBUG] Creating constraints\n");
  /***************************/
 // We create the constraints
 /***************************/
 // Resource capacity constraints on the CPU and GPU uses
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints A\n");
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop3=0;iLoop3<M();iLoop3++)
	 {
		IloExpr A(env);
		for (iLoop2=0;iLoop2<N();iLoop2++) 
			A+=nc(iLoop2)*x[iLoop2][iLoop3][iLoop];
		con.add(A<=mc(iLoop3));
	 }
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints B\n");
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop3=0;iLoop3<M();iLoop3++)
	 {
		IloExpr B(env);
		for (iLoop2=0;iLoop2<N();iLoop2++) 
			B+=ng(iLoop2)*x[iLoop2][iLoop3][iLoop];
		con.add(B<=mg(iLoop3));
	 }
 // Resource capacity constraints on the harddrive and RAM uses
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints C\n");
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop3=0;iLoop3<M();iLoop3++)
	 {
		IloExpr C(env);
		for (iLoop2=0;iLoop2<N();iLoop2++)
		{
			C+=nh(iLoop2)*x[iLoop2][iLoop3][iLoop];
			for (iLoop4=0;iLoop4<M();iLoop4++)
				if (iLoop4!=iLoop3)
					C+=nh(iLoop2)*y[iLoop2][iLoop2][iLoop4][iLoop3][iLoop];
		}
		con.add(C<=mh(iLoop3));
	 }
// if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints D\n");
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop3=0;iLoop3<M();iLoop3++)
	 {
		IloExpr D(env);
		for (iLoop2=0;iLoop2<N();iLoop2++)
		{
			D+=nr(iLoop2)*x[iLoop2][iLoop3][iLoop];
			for (iLoop4=0;iLoop4<M();iLoop4++)
				if (iLoop4!=iLoop3)
					D+=nr(iLoop2)*y[iLoop2][iLoop2][iLoop4][iLoop3][iLoop];
		}
		con.add(D<=mr(iLoop3));
	 }
 // Constraints defining the network use
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints E\n");
 for (iLoop=0;iLoop<T();iLoop++)
   for (iLoop2=0;iLoop2<N();iLoop2++)
	 for (iLoop4=iLoop2+1;iLoop4<N();iLoop4++)
	   if (a(iLoop2,iLoop4)==1)
		   for (iLoop3=0;iLoop3<M();iLoop3++)
			   for (iLoop5=iLoop3+1;iLoop5<M();iLoop5++)
					{
					 IloExpr E(env);
					 E+=x[iLoop2][iLoop3][iLoop];
					 E+=x[iLoop4][iLoop5][iLoop];
					 E-=y[iLoop2][iLoop4][iLoop3][iLoop5][iLoop];
					 con.add(E<=1);
					}
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints F\n");
 for (iLoop2=0;iLoop2<N();iLoop2++)
   for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop4=iLoop;iLoop4<min((int)(iLoop+mt(iLoop2)),(int)T());iLoop4++)
		   for (iLoop3=0;iLoop3<M();iLoop3++)
			for (iLoop6=iLoop4+1;iLoop6<T();iLoop6++)
			   for (iLoop5=0;iLoop5<M();iLoop5++)
				   if (iLoop5!=iLoop3)
					{
					 IloExpr F(env);
					 F+=x[iLoop2][iLoop3][iLoop4];
					 F+=x[iLoop2][iLoop5][iLoop6];
					 F-=y[iLoop2][iLoop2][iLoop3][iLoop5][iLoop];
					 for (iLoop7=0;iLoop7<M();iLoop7++)
						for (iLoop8=iLoop4+1;iLoop8<iLoop6;iLoop8++)
							F-=x[iLoop2][iLoop7][iLoop8];
					 con.add(F<=1);
					}
 //for (iLoop2=0;iLoop2<N();iLoop2++)
 //  for (iLoop=0;iLoop<T();iLoop++)
	// for (iLoop4=iLoop+1;iLoop4<min((int)(iLoop+mt(iLoop2)),(int)T());iLoop4++)
	//	   for (iLoop3=0;iLoop3<M();iLoop3++)
	//		   for (iLoop5=0;iLoop5<M();iLoop5++)
	//			   if (iLoop5!=iLoop3 && gap[iLoop2][iLoop4-1]<T())
	//				{
	//				 IloExpr F(env);
	//				 F+=x[iLoop2][iLoop3][iLoop4-1];
	//				 F+=x[iLoop2][iLoop5][gap[iLoop2][iLoop4-1]];
	//				 //F+=x[iLoop2][iLoop5][iLoop4];
	//				 F-=y[iLoop2][iLoop2][iLoop3][iLoop5][iLoop];
	//				 con.add(F<=1);
	//				}

 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints F1\n");
	for (iLoop2=0;iLoop2<N();iLoop2++)
	 for (iLoop=0;iLoop<T();iLoop++)
	   for (iLoop4=iLoop+1;iLoop4<T();iLoop4++)
		 for (iLoop3=0;iLoop3<M();iLoop3++)
			 for (iLoop5=0;iLoop5<M();iLoop5++)
				 if (iLoop3!=iLoop5)
				 {
					 IloExpr F1(env); 
					 F1+=mt(iLoop2)*(x[iLoop2][iLoop3][iLoop]+x[iLoop2][iLoop5][iLoop4]);
					 for (iLoop6=0;iLoop6<M();iLoop6++)
						 for (iLoop7=iLoop+1;iLoop7<iLoop4;iLoop7++)
							 F1-=mt(iLoop2)*x[iLoop2][iLoop6][iLoop7];
					 for (iLoop6=iLoop;iLoop6>(int)max((int)iLoop+1-mt(iLoop2),(int)0);iLoop6--)
						 F1-=x[iLoop2][iLoop3][iLoop6];
					 con.add(F1<=mt(iLoop2));
				 }
	//for (iLoop2=0;iLoop2<N();iLoop2++)
	// for (iLoop=mt(iLoop2)-1;iLoop<T()-1;iLoop++)
	//	 for (iLoop3=0;iLoop3<M();iLoop3++)
	//		 for (iLoop5=0;iLoop5<M();iLoop5++)
	//			 if (iLoop3!=iLoop5 && gap[iLoop2][iLoop]<T() && u(iLoop2,iLoop)==1)
	//			 {
	//				 IloExpr F1(env); 
	//				 F1+=mt(iLoop2)*(x[iLoop2][iLoop3][iLoop]+x[iLoop2][iLoop5][gap[iLoop2][iLoop]]);
	//				 for (iLoop4=gap[iLoop2][iLoop]-1;iLoop4>gap[iLoop2][iLoop]-1-mt(iLoop2);iLoop4--)
	//					 F1-=x[iLoop2][iLoop5][iLoop4];
	//				 con.add(F1<=mt(iLoop2));
	//			 }
	//for (iLoop2=0;iLoop2<N();iLoop2++)
	// for (iLoop=0;iLoop<T()-mt(iLoop2);iLoop++)
	//	 for (iLoop3=0;iLoop3<M();iLoop3++)
	//		 for (iLoop5=0;iLoop5<M();iLoop5++)
	//			 if (iLoop3!=iLoop5)
	//			 {
	//				 IloExpr F1(env); 
	//				 F1+=mt(iLoop2)*(x[iLoop2][iLoop3][iLoop]+x[iLoop2][iLoop5][iLoop+1]);					 
	//				 for (iLoop4=iLoop+1;iLoop4<iLoop+mt(iLoop2);iLoop4++)
	//					 F1-=x[iLoop2][iLoop5][iLoop4];
	//				 con.add(F1<=mt(iLoop2));
	//			 }
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints F2\n");
	//for (iLoop2=0;iLoop2<N();iLoop2++)
	// for (iLoop=T()-mt(iLoop2);iLoop<T()-1;iLoop++)
	//	 for (iLoop3=0;iLoop3<M();iLoop3++)
	//		 for (iLoop5=0;iLoop5<M();iLoop5++)
	//			 if (iLoop3!=iLoop5)
	//			 {
	//				 IloExpr F1(env);
	//				 F1+=(T()-iLoop-1)*(x[iLoop2][iLoop3][iLoop]+x[iLoop2][iLoop5][iLoop+1]);
	//				 for (iLoop4=iLoop+1;iLoop4<T();iLoop4++)
	//					 F1-=x[iLoop2][iLoop5][iLoop4];
	//				 con.add(F1<=(T()-iLoop-1));
	//			 }

 // Non preemptable tasks must be processed at each time point
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints G\n");
 for (iLoop2=0;iLoop2<N();iLoop2++)
	if (R(iLoop2)==0)
		for (iLoop=0;iLoop<T();iLoop++)
					{
					 IloExpr G(env);
					 for (iLoop3=0;iLoop3<M();iLoop3++)
						G+=x[iLoop2][iLoop3][iLoop];
					 con.add(G==u(iLoop2,iLoop));
					}
 // Preemptable tasks can be processed at each time point
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints H\n");
 for (iLoop2=0;iLoop2<N();iLoop2++)
	if (R(iLoop2)==1)
		for (iLoop=0;iLoop<T();iLoop++)
					{
					 IloExpr H(env);
					 for (iLoop3=0;iLoop3<M();iLoop3++)
						H+=x[iLoop2][iLoop3][iLoop];
					 con.add(H<=u(iLoop2,iLoop));
					}
 // Preassignment constraints of tasks to machines
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints I\n");
 for (iLoop2=0;iLoop2<N();iLoop2++)
	for (iLoop=0;iLoop<T();iLoop++)
		for (iLoop3=0;iLoop3<M();iLoop3++)
					{
					 IloExpr I(env);
					 I+=x[iLoop2][iLoop3][iLoop];
					 con.add(I<=u(iLoop2,iLoop)*q(iLoop2,iLoop3));
					}
 // Bandwidth capacity constraints
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints J\n");
 for (iLoop=0;iLoop<T();iLoop++)
	for (iLoop5=0;iLoop5<NbEdges();iLoop5++)
					{
					 IloExpr J(env);
					 unsigned int iMachj,iMachjp,iSwap;
					 for (iLoop3=0;iLoop3<NbMachEdge(iLoop5);iLoop3++)
					 {
					    CoupleMachines(iLoop5,iLoop3,iMachj,iMachjp);
						if (iMachj>iMachjp) 
							{ iSwap=iMachj; iMachj=iMachjp; iMachjp=iSwap;}
						for (iLoop2=0;iLoop2<N();iLoop2++)
							for (iLoop4=iLoop2+1;iLoop4<N();iLoop4++)
								if (a(iLoop2,iLoop4)==1)
									J+=b(iLoop2,iLoop4)*y[iLoop2][iLoop4][iMachj][iMachjp][iLoop];
						for (iLoop2=0;iLoop2<N();iLoop2++)
									J+=b(iLoop2,iLoop2)*(y[iLoop2][iLoop2][iMachj][iMachjp][iLoop]+y[iLoop2][iLoop2][iMachjp][iMachj][iLoop]);
					 }
					 con.add(J<=maxb());
					}
 // Number of machines turned on at any time t
if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints K\n");
for (iLoop2=0;iLoop2<N();iLoop2++)
	for (iLoop=0;iLoop<T();iLoop++)
		for (iLoop3=0;iLoop3<M();iLoop3++)
					{
					 IloExpr K(env);
					 K=z[iLoop3][iLoop]-x[iLoop2][iLoop3][iLoop];
					 con.add(K>=0);
					}
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints L\n");
 for (iLoop=0;iLoop<T();iLoop++)
					{
					 IloExpr L(env);
					 for (iLoop3=0;iLoop3<M();iLoop3++)
						L+=z[iLoop3][iLoop];
					 L-=Z[iLoop];
					 con.add(L==0);
					}

 // Constraint to answer the minimum resuming time for suspended tasks
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints M\n");
 for (iLoop2=0;iLoop2<N();iLoop2++)
  if (R(iLoop2)==1)
	for (iLoop3=0;iLoop3<M();iLoop3++)
	  for (iLoop=0;iLoop<T()-rt(iLoop2,iLoop3);iLoop++)
		for (iLoop4=iLoop+1;iLoop4<iLoop+rt(iLoop2,iLoop3);iLoop4++)
					{
					 IloExpr Mp(env);
					 Mp+=x[iLoop2][iLoop3][iLoop4]+x[iLoop2][iLoop3][iLoop]-x[iLoop2][iLoop3][iLoop+1];
					 con.add(Mp<=1);
					}
 // Constraints defining the duration of the resume operations
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints N\n");
 for (iLoop2=0;iLoop2<N();iLoop2++)
  if (R(iLoop2)==1)
	for (iLoop3=0;iLoop3<M();iLoop3++)
	  for (iLoop=1;iLoop<T()-rt(iLoop2,iLoop3)-1;iLoop++)
		for (iLoop4=iLoop+rt(iLoop2,iLoop3)+1;iLoop4<T();iLoop4++)
					{
					 IloExpr Np(env);
					 Np+=d[iLoop2][iLoop]-(iLoop4-iLoop)*x[iLoop2][iLoop3][iLoop-1]-(iLoop4-iLoop)*x[iLoop2][iLoop3][iLoop4];
					 for (iLoop5=0;iLoop5<M();iLoop5++)
						 for (iLoop6=iLoop+1;iLoop6<iLoop4-1;iLoop6++)
							 Np+=(iLoop4-iLoop)*x[iLoop2][iLoop5][iLoop6];
					 con.add(Np>=(iLoop-iLoop4));
					}
 // Constraints defining the duration of the migration operations
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints O\n");
 for (iLoop2=0;iLoop2<N();iLoop2++)
	for (iLoop3=0;iLoop3<M();iLoop3++)
	 for (iLoop5=0;iLoop5<M();iLoop5++)
		 if (iLoop3!=iLoop5)
		   for (iLoop=0;iLoop<T()-mt(iLoop2)-1;iLoop++)
					{
					 IloExpr O(env);
					 O+=d[iLoop2][iLoop]-mt(iLoop2)*(x[iLoop2][iLoop3][iLoop]+x[iLoop2][iLoop5][iLoop+1]);
					 con.add(O>=-1*mt(iLoop2));
					}

 
 // Definition of criterion RE
 if (DEBUG_MOD) printf("[DEBUG] Declaration of RE\n");
 IloExpr ExprRE(env);
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop2=0;iLoop2<N();iLoop2++)
		 ExprRE+=d[iLoop2][iLoop];
 ExprRE-=RE;
 //con.add(ExprRE==0);
 // e-constraint on RE
 //con.add(RE<=50000);

 if (DEBUG_MOD) printf("[DEBUG] All constraints are created and are now added to the model\n");
 model.add(con);
 if (DEBUG_MOD) 
 {
	 printf("[DEBUG] The model is created\n");
	 getch();
 }
}


//***********************************************************************************
// This function initializes the LP model
//***********************************************************************************
void InitializeLPModel()
{
 int iLoop,iLoop2,iLoop3,iLoop4,iLoop5,iLoop6;
 
 if (DEBUG_MOD) printf("[DEBUG] Starting to create the model: variables creation\n");
 /***************************/
 // We create the variables
 /***************************/
 // Variables xijt
 for (iLoop=0;iLoop<N()*M()*T();iLoop++) var.add(IloNumVar(env,0.0,1.0,ILOINT));
 // Variables yii'jj't
 for (iLoop=0;iLoop<N()*M()*T()*N()*M();iLoop++) var.add(IloNumVar(env,0.0,1.0,ILOINT));
 // Variables ztj
 for (iLoop=0;iLoop<M()*T();iLoop++) var.add(IloNumVar(env,0.0,1.0,ILOINT));
 // Variables zt
 for (iLoop=0;iLoop<T();iLoop++) var.add(IloNumVar(env,0.0,1.0,ILOINT));
 // Variables dit
 for (iLoop=0;iLoop<N()*T();iLoop++) var.add(IloNumVar(env,0.0,9999999.0,ILOINT));
 // Variable RE
 var.add(IloNumVar(env,0.0,9999999.0,ILOINT));


 if (DEBUG_MOD) printf("[DEBUG] Declaration of the objective function TC\n");
 /***************************/
 // We create the objectif function TC
 /***************************/
 IloExpr ObjTC(env);
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop2=0;iLoop2<N();iLoop2++)
		 for (iLoop3=0;iLoop3<M();iLoop3++)
			 ObjTC+=var[iLoop2*M()*T()+iLoop3*T()+iLoop]*(alphac(iLoop3)*nc(iLoop2)+alphag(iLoop3)*ng(iLoop2)+alphah(iLoop3)*nh(iLoop2)+alphar(iLoop3)*nr(iLoop2));
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop2=0;iLoop2<N();iLoop2++)
	 {
		 IloExpr tmp(env);
		 for (iLoop3=0;iLoop3<M();iLoop3++)
			 tmp+=var[iLoop2*M()*T()+iLoop3*T()+iLoop];
		 ObjTC+=rho(iLoop2)*u(iLoop2,iLoop)*(1.0-tmp);
	 }
 for (iLoop=0;iLoop<T();iLoop++)
	ObjTC+=beta(iLoop)*var[N()*M()*T()+N()*M()*T()*N()*M()+M()*T()];
 model.add(IloMinimize(env,ObjTC));

 if (DEBUG_MOD) printf("[DEBUG] Creating constraints\n");
  /***************************/
 // We create the constraints
 /***************************/
 // Resource capacity constraints on the CPU and GPU uses
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop3=0;iLoop3<M();iLoop3++)
	 {
		IloExpr A(env);
		for (iLoop2=0;iLoop2<N();iLoop2++) 
			A+=nc(iLoop2)*var[iLoop2*M()*T()+iLoop3*T()+iLoop];
		con.add(A<=mc(iLoop3));
	 }
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop3=0;iLoop3<M();iLoop3++)
	 {
		IloExpr B(env);
		for (iLoop2=0;iLoop2<N();iLoop2++) 
			B+=ng(iLoop2)*var[iLoop2*M()*T()+iLoop3*T()+iLoop];
		con.add(B<=mg(iLoop3));
	 }
 // Resource capacity constraints on the harddrive and RAM uses
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop3=0;iLoop3<M();iLoop3++)
	 {
		IloExpr C(env);
		for (iLoop2=0;iLoop2<N();iLoop2++)
		{
			C+=nh(iLoop2)*var[iLoop2*M()*T()+iLoop3*T()+iLoop];
			for (iLoop4=0;iLoop4<M();iLoop4++)
				if (iLoop4!=iLoop3)
					C+=nh(iLoop2)*var[N()*M()*T()+iLoop2*N()*M()*M()*T()+iLoop2*M()*M()*T()+iLoop4*M()*T()+iLoop3*T()+iLoop];
		}
		con.add(C<=mh(iLoop3));
	 }
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop3=0;iLoop3<M();iLoop3++)
	 {
		IloExpr D(env);
		for (iLoop2=0;iLoop2<N();iLoop2++)
		{
			D+=nr(iLoop2)*var[iLoop2*M()*T()+iLoop3*T()+iLoop];
			for (iLoop4=0;iLoop4<M();iLoop4++)
				if (iLoop4!=iLoop3)
					D+=nr(iLoop2)*var[N()*M()*T()+iLoop2*N()*M()*M()*T()+iLoop2*M()*M()*T()+iLoop4*M()*T()+iLoop3*T()+iLoop];
		}
		con.add(D<=mr(iLoop3));
	 }
 // Constraints defining the network use
 for (iLoop=0;iLoop<T();iLoop++)
   for (iLoop2=0;iLoop2<N();iLoop2++)
	 for (iLoop4=0;iLoop4<N();iLoop4++)
	   if (iLoop4!=iLoop2 && a(iLoop2,iLoop4)==1)
		   for (iLoop3=0;iLoop3<M();iLoop3++)
			   for (iLoop5=0;iLoop5<M();iLoop5++)
				   if (iLoop5!=iLoop3)
					{
					 IloExpr E(env);
					 E+=var[iLoop2*M()*T()+iLoop3*T()+iLoop];
					 E+=var[iLoop4*M()*T()+iLoop5*T()+iLoop];
					 E-=var[N()*M()*T()+iLoop2*N()*M()*M()*T()+iLoop4*M()*M()*T()+iLoop3*M()*T()+iLoop5*T()+iLoop];
					 con.add(E<=1);
					}
 for (iLoop2=0;iLoop2<N();iLoop2++)
   for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop4=iLoop+1;iLoop4<min((int)(iLoop+mt(iLoop2)),(int)T());iLoop4++)
		   for (iLoop3=0;iLoop3<M();iLoop3++)
			   for (iLoop5=0;iLoop5<M();iLoop5++)
				   if (iLoop5!=iLoop3)
					{
					 IloExpr F(env);
					 F+=var[iLoop2*M()*T()+iLoop3*T()+iLoop4-1];
					 F+=var[iLoop2*M()*T()+iLoop5*T()+iLoop4];
					 F-=var[N()*M()*T()+iLoop2*N()*M()*M()*T()+iLoop2*M()*M()*T()+iLoop3*M()*T()+iLoop5*T()+iLoop];
					 con.add(F<=1);
					}
 //for (iLoop2=0;iLoop2<N();iLoop2++)
 //  for (iLoop=0;iLoop<T()-mt(iLoop2);iLoop++)
	// for (iLoop4=iLoop+1;iLoop4<iLoop+mt(iLoop2);iLoop4++)
	//	   for (iLoop3=0;iLoop3<M();iLoop3++)
	//		   for (iLoop5=0;iLoop5<M();iLoop5++)
	//			   if (iLoop5!=iLoop3)
	//				{
	//				 IloExpr F(env);
	//				 F+=var[iLoop2*M()*T()+iLoop3*T()+iLoop4-1];
	//				 F+=var[iLoop2*M()*T()+iLoop5*T()+iLoop4];
	//				 F-=var[N()*M()*T()+iLoop2*N()*M()*M()*T()+iLoop2*M()*M()*T()+iLoop3*M()*T()+iLoop5*T()+iLoop];
	//				 con.add(F<=1);
	//				}

	for (iLoop2=0;iLoop2<N();iLoop2++)
	 for (iLoop=0;iLoop<T()-mt(iLoop2);iLoop++)
		 for (iLoop3=0;iLoop3<M();iLoop3++)
			 for (iLoop5=0;iLoop5<M();iLoop5++)
				 if (iLoop3!=iLoop5)
				 {
					 IloExpr F1(env);
					 F1+=mt(iLoop2)*(var[iLoop2*M()*T()+iLoop3*T()+iLoop]+var[iLoop2*M()*T()+iLoop5*T()+iLoop+1]);
					 for (iLoop4=iLoop+1;iLoop4<iLoop+mt(iLoop2);iLoop4++)
						 F1-=var[iLoop2*M()*T()+iLoop5*T()+iLoop4];
					 con.add(F1<=mt(iLoop2));
				 }
	for (iLoop2=0;iLoop2<N();iLoop2++)
	 for (iLoop=T()-mt(iLoop2);iLoop<T();iLoop++)
		 for (iLoop3=0;iLoop3<M();iLoop3++)
			 for (iLoop5=0;iLoop5<M();iLoop5++)
				 if (iLoop3!=iLoop5)
				 {
					 IloExpr F1(env);
					 F1+=(T()-iLoop-1)*(var[iLoop2*M()*T()+iLoop3*T()+iLoop]+var[iLoop2*M()*T()+iLoop5*T()+iLoop+1]);
					 for (iLoop4=iLoop+1;iLoop4<T();iLoop4++)
						 F1-=var[iLoop2*M()*T()+iLoop5*T()+iLoop4];
					 con.add(F1<=(T()-iLoop-1));
				 }

 // Non preemptable tasks must be processed at each time point
 for (iLoop2=0;iLoop2<N();iLoop2++)
	if (R(iLoop2)==0)
		for (iLoop=0;iLoop<T();iLoop++)
					{
					 IloExpr G(env);
					 for (iLoop3=0;iLoop3<M();iLoop3++)
						G+=var[iLoop2*M()*T()+iLoop3*T()+iLoop];
					 con.add(G==u(iLoop2,iLoop));
					}
 // Preemptable tasks can be processed at each time point
 for (iLoop2=0;iLoop2<N();iLoop2++)
	if (R(iLoop2)==1)
		for (iLoop=0;iLoop<T();iLoop++)
					{
					 IloExpr H(env);
					 for (iLoop3=0;iLoop3<M();iLoop3++)
						H+=var[iLoop2*M()*T()+iLoop3*T()+iLoop];
					 con.add(H<=u(iLoop2,iLoop));
					}
 // Preassignment constraints of tasks to machines
 //for (iLoop2=0;iLoop2<N();iLoop2++)
	//for (iLoop=0;iLoop<T();iLoop++)
	//	for (iLoop3=0;iLoop3<M();iLoop3++)
	//				{
	//				 IloExpr I(env);
	//				 I+=var[iLoop2*M()*T()+iLoop3*T()+iLoop];
	//				 con.add(I<=u(iLoop2,iLoop)*q(iLoop2,iLoop3));
	//				}
 // Bandwidth capacity constraints
 for (iLoop=0;iLoop<T();iLoop++)
	for (iLoop5=0;iLoop5<NbEdges();iLoop5++)
					{
					 IloExpr J(env);
					 unsigned int iMachj,iMachjp;
					 for (iLoop3=0;iLoop3<NbMachEdge(iLoop5);iLoop3++)
					 {
					    CoupleMachines(iLoop5,iLoop3,iMachj,iMachjp);
						for (iLoop2=0;iLoop2<N();iLoop2++)
							for (iLoop4=0;iLoop4<N();iLoop4++)
								if (a(iLoop2,iLoop4)==1)
									J+=b(iLoop2,iLoop4)*var[N()*M()*T()+iLoop2*N()*M()*M()*T()+iLoop4*M()*M()*T()+iMachj*M()*T()+iMachjp*T()+iLoop]+b(iLoop2,iLoop4)*var[N()*M()*T()+iLoop2*N()*M()*M()*T()+iLoop4*M()*M()*T()+iMachjp*M()*T()+iMachj*T()+iLoop];
					 }
					 con.add(J<=maxb());
					}
 // Number of machines turned on at any time t
 for (iLoop2=0;iLoop2<N();iLoop2++)
	for (iLoop=0;iLoop<T();iLoop++)
		for (iLoop3=0;iLoop3<M();iLoop3++)
					{
					 IloExpr K(env);
					 K=var[N()*M()*T()+N()*M()*T()*N()*M()+iLoop3*T()+iLoop]-var[iLoop2*M()*T()+iLoop3*T()+iLoop];
					 con.add(K>=0);
					}
 for (iLoop=0;iLoop<T();iLoop++)
					{
					 IloExpr L(env);
					 for (iLoop3=0;iLoop3<M();iLoop3++)
						L+=var[N()*M()*T()+N()*M()*T()*N()*M()+iLoop3*T()+iLoop];
					 L-=var[N()*M()*T()+N()*M()*T()*N()*M()+M()*T()+iLoop];
					 con.add(L==0);
					}

 // Constraint to answer the minimum resuming time for suspended tasks
 for (iLoop2=0;iLoop2<N();iLoop2++)
  if (R(iLoop2)==1)
	for (iLoop3=0;iLoop3<M();iLoop3++)
	  for (iLoop=0;iLoop<T()-rt(iLoop2,iLoop3);iLoop++)
		for (iLoop4=iLoop+1;iLoop4<iLoop+rt(iLoop2,iLoop3);iLoop4++)
					{
					 IloExpr Mp(env);
					 Mp+=var[iLoop2*M()*T()+iLoop3*T()+iLoop4]+var[iLoop2*M()*T()+iLoop3*T()+iLoop]-var[iLoop2*M()*T()+iLoop3*T()+iLoop+1];
					 con.add(Mp<=1);
					}
 // Constraints defining the duration of the resume operations
 for (iLoop2=0;iLoop2<N();iLoop2++)
  if (R(iLoop2)==1)
	for (iLoop3=0;iLoop3<M();iLoop3++)
	  for (iLoop=1;iLoop<T()-rt(iLoop2,iLoop3)-1;iLoop++)
		for (iLoop4=iLoop+rt(iLoop2,iLoop3)+1;iLoop4<T();iLoop4++)
					{
					 IloExpr Np(env);
					 Np+=var[N()*M()*T()+N()*M()*T()*N()*M()+M()*T()+T()+iLoop2*T()+iLoop]-(iLoop4-iLoop)*var[iLoop2*M()*T()+iLoop3*T()+iLoop-1]-(iLoop4-iLoop)*var[iLoop2*M()*T()+iLoop3*T()+iLoop4];
					 for (iLoop5=0;iLoop5<M();iLoop5++)
						 for (iLoop6=iLoop+1;iLoop6<iLoop4-1;iLoop6++)
							 Np+=(iLoop4-iLoop)*var[iLoop2*M()*T()+iLoop5*T()+iLoop6];
					 con.add(Np>=(iLoop-iLoop4));
					}
 // Constraints defining the duration of the migration operations
 for (iLoop2=0;iLoop2<N();iLoop2++)
	for (iLoop3=0;iLoop3<M();iLoop3++)
	 for (iLoop5=0;iLoop5<M();iLoop5++)
		 if (iLoop3!=iLoop5)
		   for (iLoop=0;iLoop<T()-mt(iLoop2)-1;iLoop++)
					{
					 IloExpr O(env);
					 O+=var[N()*M()*T()+N()*M()*T()*N()*M()+M()*T()+T()+iLoop2*T()+iLoop]-mt(iLoop2)*(var[iLoop2*M()*T()+iLoop3*T()+iLoop]+var[iLoop2*M()*T()+iLoop5*T()+iLoop+1]);
					 con.add(O>=-1*mt(iLoop2));
					}

 // Definition of criterion RE
 IloExpr RE(env);
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop2=0;iLoop2<N();iLoop2++)
		 RE+=var[N()*M()*T()+N()*M()*T()*N()*M()+M()*T()+T()+iLoop2*T()+iLoop];
 RE-=var[N()*M()*T()+N()*M()*T()*N()*M()+M()*T()+T()+N()*T()];
 con.add(RE==0);

 // e-constraint on RE
 con.add(var[N()*M()*T()+N()*M()*T()*N()*M()+M()*T()+T()+N()*T()]<=500);


 if (DEBUG_MOD) printf("[DEBUG] All constraints are created and are now added to the model\n");
 model.add(con);
 if (DEBUG_MOD) 
 {
	 printf("[DEBUG] The model is created\n");
	 getch();
 }
}

// This function counts the number of machines which are turned on, on the average, at any time t
double CountPMsTurnedOn(IloCplex *pcplex)
{
 unsigned int iLoop;
 double dCount=0;

 for (iLoop=0;iLoop<T();iLoop++)
	 dCount+=pcplex->getValue(Z[iLoop]);
 return (dCount/=(double)T());
}


double GetTimeByClockTicks(clock_t ticks0, clock_t ticks1)
{
	return double(ticks1 - ticks0)/CLOCKS_PER_SEC;
}

/* Programme Principal */
int main(int argc)
{
	time_t temp1,temp2,tempPre1,tempPre2;
	clock_t ticks0;
	FILE *fic;
	double dNbMach;
	int iTimeLimit = CalculateTimeLimit();
    printf("TimeLimit : %d\n", iTimeLimit);

	GetData();
	if (DEBUG) DisplayData();

	ticks0 = clock();
	/*CREATION OF MODEL, CONSTRAINTS AND UB HERE*/
	//InitializeLPModel();
	InitializeIPModel();

	IloCplex cplex;
	try
    {
        cplex = IloCplex(model);		
		if (DEBUG)
			cplex.exportModel("SCP.lp");
		if(CONFIG)
		{
			cplex.setParam(IloCplex::EpGap, 0.02);// We limit the  mipgap tolerance
			cplex.setParam(IloCplex::TiLim, iTimeLimit);
		}
		//PreByCalCost(0,var.getSize(),head,nbBool,Lmax_Schrage, &env , &cplex , &model , &var , &con);	// See above
	//try 
	//{
		dNbMach=-1.0;
		if (!cplex.solve())
		{ // cplex fails to solve the problem
		 dOptValue=-1;
		 isOptimal=0;
		 isFeasible=0;
		} else
		{ // We now test if the solution found is optimal or feasible
			printf("Cplex status: %ld\n",cplex.getCplexStatus());
			if (cplex.getCplexStatus()==IloCplex::Optimal || cplex.getCplexStatus()==IloCplex::OptimalTol)
			{
				isOptimal=1;
				isFeasible=1;
				dOptValue=cplex.getObjValue();
				dNbMach=CountPMsTurnedOn(&cplex);
			} else if (cplex.getCplexStatus()==IloCplex::Infeasible || cplex.getCplexStatus()==IloCplex::InfeasibleOrUnbounded || cplex.getCplexStatus()==IloCplex::InfOrUnbd)
			{
				isOptimal=0;
				isFeasible=0;
				dOptValue=-1;
			} else 
			{
				isOptimal=0;
				isFeasible=1;
				dOptValue=cplex.getObjValue();
				dNbMach=CountPMsTurnedOn(&cplex);
			} 
	 	    iNbNodesIP=cplex.getNnodes();
		}
		printf("\nValeur de la fonction objectif : %lf\n",(double)cplex.getObjValue());

	}
	catch (IloException& e) {
		cerr << "Concert exception caught: " << e.getMessage() << endl;
		dOptValue=9999999.0;
		isOptimal=0;
		isFeasible=0;
		//getch();
	}
	catch (...)
	{
		cerr << "Unknown exception caught" << endl;
		dOptValue=9999999.0;
		isOptimal=0;
		isFeasible=0;
		//getch();
	}
	//We put the test of limit here because an cplex exception can also be caused by Limit.
	//in which case there is no solution found but the status code is set. We count this case
	//in #TimLim or #MemLim but not in #inFea.
	if (cplex.getCplexStatus()== IloCplex::AbortTimeLim) isTimeLimit = 1;
	else if (cplex.getCplexStatus()== IloCplex::MemLimFeas) isMemLimit = 1;
	
	dOptTime = GetTimeByClockTicks( ticks0, clock());
	printf("isFeasible:%d\nisOptimal:%d\nisTimeLim:%d\nisMemLim:%d\ndOptValue:%d\ndOptTime:%lf\niNbNodesIP:%d\ndNbMach:%lf\n",
		isFeasible,isOptimal,isTimeLimit, isMemLimit,(int)dOptValue,dOptTime,iNbNodesIP,dNbMach);

	CplexResult res(isFeasible
		,isOptimal
		,isTimeLimit
		,isMemLimit
		,dNbMach
		,iNbNodesIP
		,cplex.getCplexStatus()
		,dOptValue
		,-1
		,dOptTime);
	res.ExportToFile("H2.txt");

	if (DEBUG && dOptValue!=9999999.0)
			SolutionToFile(cplex);
	if (DEBUG)
		getch();
}
