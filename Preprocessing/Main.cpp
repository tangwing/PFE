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
#include "CplexResult.h"
#include "PreprocessingResult.h"

#define DEBUG false
#define DEBUG_MOD false
#define CONFIG true

#define MemLimit 1024.0
#define TimeLimit 1800.0

using namespace std;

#include <ilcplex/ilocplex.h>

ILOSTLBEGIN
typedef IloFloatVarArray IloFloatVarArray1D; // 1-dimension array of float variables
typedef IloArray <IloFloatVarArray> IloFloatVarArray2D; // 2-dimension array of float variables

// Data structure from CPLEX necessary to create the model
IloEnv env;
IloModel model(env);
IloRangeArray con(env);

// Data structures for the LP and preprocessing
IloNumVarArray var(env);	//It contains bool variables (x,y,z) à be preprocessed afterwards
							//The order of elements are X(ijt),Y(ii'jj't), Z(jt). For each of the 3 parts, the up down hierarchical order is (t,i,j).
IloFloatVarArray1D lp_Z(env);
IloFloatVarArray2D lp_d(env);
IloFloatVar lp_RE(env,0,9999999);

// Functions for identifying xyz in var
int indX( int t, int i, int j){return t*N()*M()+i*M()+j;}
int indY( int t, int i1, int i2, int j1, int j2){return T()*N()*M() + t*N()*N()*M()*M()+ i1*N()*M()*M() + i2*M()*M()+ j1*M() +j2 ;}
int indZ( int t, int j){return T()*N()*M() + T()*N()*N()*M()*M() + t*M() + j;}

double dOptValue, dOptTime, dPreProcessingTime, dNbMach, dUB, dLB;
int isOptimal, isFeasible,iNbNodesIP, isOptiNoPre, isAllFixed;
int isTimeLimit=0, isMemLimit=0;
int iNbFixed = 0, nbBool=0;

double GetTimeByClockTicks(clock_t ticks0, clock_t ticks1){	return double(ticks1 - ticks0)/CLOCKS_PER_SEC;}

// This function counts the number of machines which are turned on, on the average, at any time t
double CountPMsTurnedOn(IloCplex *pcplex)
{
 unsigned int iLoop;
 double dCount=0;

 for (iLoop=0;iLoop<T();iLoop++)
	 dCount+=pcplex->getValue(lp_Z[iLoop]);
 return (dCount/=(double)T());
}


//Preprocess and solve
void PreByCalCost(bool isMIPOnly, int *head, int nbBool,int UB, IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar,IloRangeArray *pcon)
{
	double objValue, temps_cpu=0, temps_cpu_pre=0;
	clock_t temp1,temp2,tempPre1,tempPre2;
	int nbFix = 0, nbNodes=0;

	Preprocessing *prepro=new Preprocessing();
	prepro->PRESetHead(head);
	prepro->PRESetDebug(DEBUG);
	prepro->PRESetTomlin(false);
	prepro->PRESetUB(UB);		// Fix the variables using the result of an heuristique algorithme Schrage as the UB
	pcplex->exportModel("model.lp");

	// CASE 1: No preprocessing is required before solving the IP model
	if(isMIPOnly)//Only for testing the correctness of LP model
	{ 
		temp1 = clock();
		// Step 1: give the LP relaxation to the library
		prepro->PREInitializeLP(penv, pcplex, pmodel, pvar, pcon);
		// Step 2: convert the LP model into an IP model (real valued variables are turned into integer valued variables) 
		prepro->PREInitializeMIPfromLP(penv , pcplex , pmodel , pvar, pcon,head);
		// Step 3: solve the IP formulation
		prepro->PRESolveMIP();

		objValue = prepro->PREGetMipOpt();
		nbNodes = prepro->PREGetMIPNbNode();
		temp2 = clock();
		if (DEBUG)
			cout<<"Optimal solution found: "<<prepro->PREGetMipOpt()<<endl; 
		temps_cpu = GetTimeByClockTicks(temp1,temp2);

		if(DEBUG) {
				ofstream logs("logs/LOGS.txt", ios::app);
				if(prepro->PREisMIPOpt())
					logs << "OPTIMAL" << endl;
				else
					logs << "NOT_OPTIMAL" << endl;
				logs<<"Optimal value: "<<objValue<<endl;
				logs<<"CPU time: "<<temps_cpu<<endl;
				logs<<"Nb Nodes: "<<nbNodes<<endl<<endl;
				logs.close();
		}

		ofstream outFile("MIP.txt");
		if(prepro->PREisMIPOpt())
			outFile << "OPTIMAL" << endl;
		else
			outFile << "NOT_OPTIMAL" << endl;
		//outFile << nbJobs << endl;
		outFile << objValue << endl;
		outFile << temps_cpu << endl;
		outFile << nbNodes << endl;
		outFile.close();
		exit(0);
	} // END CASE 1
	else	// CASE 2: Preprocessing is requested before MIP is solved
	{ 
		if(DEBUG)
			pcplex->exportModel("model.lp");	

		tempPre1=clock();
		cout << "Preprocessing the model...\n";
		// Step 1: give the LP relaxation to the library
		prepro->PREInitializeLP(penv, pcplex, pmodel, pvar, pcon);
		// Step 2: solves the LP relaxation
		prepro->PRESolveLP();
		// Step 3: preprocess by using the LP relaxation
		prepro->PREPreprocessing();
		
		// Step 4: analyse the situation after preprocessing
		isOptiNoPre = 0;
		isAllFixed = 0;
		nbFix += prepro->PREGetNbFix();
		if(prepro->PREIsOptiNoPRE() || nbFix == nbBool) // If no preprocessing (i.e. LB=UB before prepro)
		{
			tempPre2  = clock();
			temps_cpu_pre+= GetTimeByClockTicks(tempPre1,tempPre2);
			if(prepro->PREIsOptiNoPRE())
			{
				cout << "UB=LB!" << " " << UB << " " << prepro->PREGetLpOpt() << endl;
				isOptiNoPre = 1;
				iNbFixed = 0;
				dOptValue = UB;
				dOptTime = dPreProcessingTime = temps_cpu_pre;
			}
			else if(nbFix == nbBool)
			{
				cout << "IS INTEGRAL!"<<endl;
				isAllFixed = 1;
				iNbFixed = nbFix;
				dOptTime = temps_cpu;
				dPreProcessingTime = temps_cpu_pre;
			}
		}
		dUB = prepro->PREGetUB();
		dLB = prepro->PREGetLB();
		PreprocessingResult res;
		res.isOptiNoPre = isOptiNoPre;
		res.isAllFixed = isAllFixed;
		res.UB = dUB;
		res.LB = dLB;
		res.nbBool = nbBool;
		res.nbFixed = nbFix;
		res.lastIFixed = prepro->PRELastFixedI;
		res.durationPre = GetTimeByClockTicks(tempPre1, clock());
		res.ExportToFile("Preproc.txt");
		
											//Pour le but de test de preprocessing, on va pas résoudre le MIP.
											//else // Step 5: solve the reduced IP formulation
											//{
											// tempPre2=clock();
											// temps_cpu_pre += GetTimeByClockTicks(tempPre1,tempPre2);
											// cout << "NbFix " << nbFix << " on " << nbBool << " variables" <<endl;
											// // Step 5.1: convert the LP model into an IP model (real valued variables are turned into integer valued variables) 
											// prepro->PREInitializeMIPfromLP(penv , pcplex , pmodel , pvar, pcon,head);
											// // Step 5.2: clear data structures of cplex
											// pcplex->extract(*pmodel);
											// // Step 5.3: fix variables to the IP formulation, as deduced during the preprocessing phase (normally useless, since we convert reduced LP model)
											// prepro->PREFixVarToMIP();
											// // Step 5.4: solve the reduced IP formulation
											// prepro->PRESolveMIP();

											// // Step 5.5: retrieving information from the solution
											// objValue=prepro->PREGetMipOpt();
											// nbNodes = prepro->PREGetMIPNbNode();
											// //opti = prepro->PREisMIPOpt();
											// temps_cpu=temps_cpu_pre + GetTimeByClockTicks(tempPre2, clock());
											// if(DEBUG)
											//		cout << "--------------------------" << endl;
											//}

	    // Step 6: output the results
		//ofstream outFile("PreMIP.txt");
		//if (prepro->PREIsOptiNoPRE() && nbFix == 0)
		//			outFile << "NO_PRE" << endl;
		//	else
		//			outFile << "PRE" << endl;
		//if(prepro->PREisMIPOpt())
		//	outFile << "OPTIMAL" << endl;
		//else
		//	outFile << "NOT_OPTIMAL" << endl;
		////outFile << nbJobs << endl;
		//outFile << objValue << endl;
		//outFile << temps_cpu << endl; // CPU time for the whole solution process
		//outFile << temps_cpu_pre << endl; // CPU time of the preprocessing
		//outFile << nbNodes << endl;
		//if (nbBool>0) // Percentage of fixed variables
		//		outFile << 100.0*double(nbFix)/double(nbBool) << endl;
		//	else 
		//		outFile << 0 << endl;
		//outFile.close();

		//Step 7	
		// We now test if the solution found is optimal or feasible
		//printf("Cplex status: %ld\n",pcplex->getCplexStatus());
		//fflush(stdout);
		
		//if(isOptiNoPre)
		//{
		//	//do nothing
		//}
		//else if (pcplex->getCplexStatus()==IloCplex::Optimal || pcplex->getCplexStatus()==IloCplex::OptimalTol)
		//{
		//	isOptimal=1;
		//	isFeasible=1;
		//	dOptValue=pcplex->getObjValue();
		//	dNbMach=CountPMsTurnedOn(pcplex);
		//} else if (pcplex->getCplexStatus()==IloCplex::Infeasible || pcplex->getCplexStatus()==IloCplex::InfeasibleOrUnbounded || pcplex->getCplexStatus()==IloCplex::InfOrUnbd)
		//{
		//	isOptimal=0;
		//	isFeasible=0;
		//	dOptValue=-1;
		//} else 
		//{
		//	isOptimal=0;
		//	isFeasible=1;
		//	dOptValue=pcplex->getObjValue();
		//	dNbMach=CountPMsTurnedOn(pcplex);
		//} 
	 //	iNbNodesIP=pcplex->getNnodes();
		//dOptTime = temps_cpu;
		//dPreProcessingTime = temps_cpu_pre;
		//iNbFixed = nbFix;
		//printf("\nValeur de la fonction objectif : %lf\n",(double)pcplex->getObjValue());
	}

	// Delete the preprocessing object
	try {
		delete prepro;
	}
	catch(...) {
		cerr << "Error while deleting prepro!" << endl;
	}
	//TODO
	exit(0);
}


//***********************************************************************************
// This function initializes the LP model
//***********************************************************************************
void InitializeLPModel()
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
 // We create the variables. The order of loops is important (for var[]).
 /***************************/
 //var.setSize( T()*N()*M() + T()*N()*N()*M()*M() + T()*M());

 // Variables xijt
 if (DEBUG_MOD) printf("Creation of variables x\n");
 for (iLoop=0;iLoop<T()*N()*M();iLoop++)var.add(IloFloatVar(env,0,1));

 // Variables yii'jj't (CPLEX will automatically eliminate the redundant variables)
 if (DEBUG_MOD) printf("Creation of variables y\n");
 for(iLoop=0;iLoop<T()*N()*N()*M()*M();iLoop++)var.add(IloFloatVar(env,0,1));

 // Variables ztj
 if (DEBUG_MOD) printf("Creation of variables z\n");
 for (iLoop=0;iLoop<T()*M();iLoop++)var.add(IloFloatVar(env,0,1));

 // Variables zt
 if (DEBUG_MOD)  printf("Creation of variables Z\n");
 lp_Z.add(IloFloatVarArray(env,T(),0,9999999));
 
 // Variables dit
 if (DEBUG_MOD)  printf("Creation of variables d\n");
 for (iLoop=0;iLoop<N();iLoop++)
	 lp_d.add(IloFloatVarArray(env,T(),0,9999999));
 
 // Variable RE is created while its declaration

 /***************************/
 // We create the objectif function TC
 /***************************/
 if (DEBUG_MOD) printf("[DEBUG] Declaration of the objective function TC\n");
 IloExpr ObjTC(env);
 for (iLoop=0;iLoop<T();iLoop++)  // Cost for using the resources
	 for (iLoop2=0;iLoop2<N();iLoop2++)
		 for (iLoop3=0;iLoop3<M();iLoop3++)
			 ObjTC+= var[indX(iLoop,iLoop2,iLoop3)]*(alphac(iLoop3)*nc(iLoop2)+alphag(iLoop3)*ng(iLoop2)+alphah(iLoop3)*nh(iLoop2)+alphar(iLoop3)*nr(iLoop2));
 for (iLoop=0;iLoop<T();iLoop++) // Cost implied by the migration on the cost of resources
	 for (iLoop2=0;iLoop2<N();iLoop2++)
		 for (iLoop3=0;iLoop3<M();iLoop3++)
			for (iLoop5=0;iLoop5<M();iLoop5++)
			 if (iLoop3!=iLoop5)
				ObjTC+=var[ indY(iLoop,iLoop2,iLoop2,iLoop3,iLoop5) ]*(alphah(iLoop5)*nh(iLoop2)+alphar(iLoop5)*nr(iLoop2));
 for (iLoop=0;iLoop<T();iLoop++) 
	 for (iLoop2=0;iLoop2<N();iLoop2++)
	 {
		 IloExpr tmp(env);
		 for (iLoop3=0;iLoop3<M();iLoop3++)
			 tmp+=var[ indX(iLoop,iLoop2,iLoop3)];
		 ObjTC+=rho(iLoop2)*u(iLoop2,iLoop)*(1.0-tmp);
	 }
 for (iLoop=0;iLoop<T();iLoop++)
	ObjTC+=beta(iLoop)*lp_Z[iLoop];
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
			A+=nc(iLoop2)*var[ indX(iLoop,iLoop2,iLoop3)];
		con.add(A<=mc(iLoop3));
	 }
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints B\n");
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop3=0;iLoop3<M();iLoop3++)
	 {
		IloExpr B(env);
		for (iLoop2=0;iLoop2<N();iLoop2++) 
			B+=ng(iLoop2)* var[ indX(iLoop,iLoop2,iLoop3)];
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
			C+=nh(iLoop2)* var[indX(iLoop,iLoop2,iLoop3)];
			for (iLoop4=0;iLoop4<M();iLoop4++)
				if (iLoop4!=iLoop3)
					C+=nh(iLoop2)* var[indY(iLoop,iLoop2,iLoop2,iLoop4,iLoop3)];
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
			D+=nr(iLoop2)*var[indX(iLoop,iLoop2,iLoop3)];
			for (iLoop4=0;iLoop4<M();iLoop4++)
				if (iLoop4!=iLoop3)
					D+=nr(iLoop2)* var[indY(iLoop,iLoop2,iLoop2,iLoop4,iLoop3)];
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
					 E+=var[indX(iLoop,iLoop2,iLoop3)];
					 E+=var[indX(iLoop,iLoop4,iLoop5)];
					 E-= var[indY(iLoop,iLoop2,iLoop4,iLoop3,iLoop5)];
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
					 F+=var[indX(iLoop4,iLoop2,iLoop3)];//i,j,t1
					 F+=var[indX(iLoop6,iLoop2,iLoop5)];//i,j',t2
					 F-= var[indY(iLoop,iLoop2,iLoop2,iLoop3,iLoop5)];
					 for (iLoop7=0;iLoop7<M();iLoop7++)
						for (iLoop8=iLoop4+1;iLoop8<iLoop6;iLoop8++)
							F-= var[indX(iLoop8,iLoop2,iLoop7)];
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
					 F1+=mt(iLoop2)*(var[indX(iLoop,iLoop2,iLoop3)]+var[indX(iLoop4,iLoop2,iLoop5)]);
					 for (iLoop6=0;iLoop6<M();iLoop6++)
						 for (iLoop7=iLoop+1;iLoop7<iLoop4;iLoop7++)
							 F1-=mt(iLoop2)*var[indX(iLoop7,iLoop2,iLoop6)];
					 for (iLoop6=iLoop;iLoop6>=(int)max((int)iLoop+1-mt(iLoop2),(int)0);iLoop6--)
						 F1-=var[indX(iLoop6,iLoop2,iLoop3)];
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
						G+=var[indX(iLoop,iLoop2,iLoop3)];
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
						H+=var[indX(iLoop,iLoop2,iLoop3)];
					 con.add(H<=u(iLoop2,iLoop));
					}
 // Preassignment constraints of tasks to machines
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints I\n");
 for (iLoop2=0;iLoop2<N();iLoop2++)
	for (iLoop=0;iLoop<T();iLoop++)
		for (iLoop3=0;iLoop3<M();iLoop3++)
					{
					 IloExpr I(env);
					 I+=var[indX(iLoop,iLoop2,iLoop3)];
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
									J+=b(iLoop2,iLoop4)* var[indY(iLoop,iLoop2,iLoop4,iMachj,iMachjp)];
						for (iLoop2=0;iLoop2<N();iLoop2++)
									J+=b(iLoop2,iLoop2)*( var[indY(iLoop,iLoop2,iLoop2,iMachj,iMachjp)]+ var[indY(iLoop,iLoop2,iLoop2,iMachjp,iMachj)]);
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
			K= var[indZ(iLoop,iLoop3)]-var[indX(iLoop,iLoop2,iLoop3)];
			con.add(K>=0);
		}
 if (DEBUG_MOD) printf("[DEBUG] Declaration of constraints L\n");
 for (iLoop=0;iLoop<T();iLoop++)
					{
					 IloExpr L(env);
					 for (iLoop3=0;iLoop3<M();iLoop3++)
						L+= var[indZ(iLoop,iLoop3)];
					 L-= lp_Z[iLoop];
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
					 Mp+=var[indX(iLoop4,iLoop2,iLoop3)]+var[indX(iLoop,iLoop2,iLoop3)]-var[indX(iLoop+1,iLoop2,iLoop3)];
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
					 Np+= lp_d[iLoop2][iLoop]-(iLoop4-iLoop)*var[indX(iLoop-1,iLoop2,iLoop3)]-(iLoop4-iLoop)*var[indX(iLoop4,iLoop2,iLoop3)];
					 for (iLoop5=0;iLoop5<M();iLoop5++)
						 for (iLoop6=iLoop+1;iLoop6<iLoop4-1;iLoop6++)
							 Np+=(iLoop4-iLoop)*var[indX(iLoop6,iLoop2,iLoop5)];
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
					 O+= lp_d[iLoop2][iLoop]-mt(iLoop2)*(var[indX(iLoop,iLoop2,iLoop3)]+var[indX(iLoop+1,iLoop2,iLoop5)]);
					 con.add(O>=-1*mt(iLoop2));
					}

 
 // Definition of criterion RE
 if (DEBUG_MOD) printf("[DEBUG] Declaration of RE\n");
 IloExpr ExprRE(env);
 for (iLoop=0;iLoop<T();iLoop++)
	 for (iLoop2=0;iLoop2<N();iLoop2++)
		 ExprRE+= lp_d[iLoop2][iLoop];
 ExprRE-= lp_RE;
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



//
/* Programme Principal */
//
enum SolveMode{PRE_MIP_ONLY, PRE_PRE};
int main(int argc, char* argvs[])
{
	if(argc < 2)abort();
	for(int i=0; i<argc; i++)cout<<argvs[i]<<endl;
	int UB = 9999999;
	UB = atoi(argvs[1]);
	time_t temp1,temp2,tempPre1,tempPre2;
	clock_t ticks0;
	FILE *fic;

	SolveMode sm = PRE_PRE;

	if(argc==3)GetData(argvs[2]);
	else GetData();
	if (DEBUG) DisplayData();

	ticks0 = clock();
	
	IloCplex cplex;
	try
    {
		InitializeLPModel();
        cplex = IloCplex(model);		
		if (DEBUG)
			cplex.exportModel("SCP.lp");
		if(CONFIG)
		{
			cplex.setParam(IloCplex::TreLim,MemLimit);// We limit the size of the search tree
			cplex.setParam(IloCplex::TiLim,TimeLimit);// We limit the time for exploring of the search tree
			//cplex.setParam(IloCplex::Threads,3);
		}
			//prepare preprocessing
			nbBool=0;
			int * head = new int[var.getSize()];
			//Add all x into head
			for(int i=0;i< T()*N()*M();i++)
			{
				head[nbBool]=var[i].getId();
				nbBool++;
			}
			//Add all y into head
			for(int i=T()*N()*M(); i<T()*N()*M()+T()*N()*N()*M()*M();i++)
			{
				head[nbBool]=var[i].getId();
				nbBool++;
			}
			//Some Yii'jj't have no effect on Objectif so they are not extractable. We should avoid adding them otherwise exception will occur during preprocessing
			//for(int iT=0; iT<T(); iT++)
			// for(int iN=0; iN<N(); iN++)
			//	for(int iN2=iN; iN2<N(); iN2++)
			//		if(iN2!=iN && a(iN, iN2)!=1)continue;
			//		else
			//		for(int iM=0; iM<M(); iM++)
			//		{
			//			int iM2=0;
			//			if(iN!=iN2)iM2=iM+1;
			//			for(; iM2<M(); iM2++)
			//				if(iM==iM2)continue;
			//				else
			//				{
			//					 head[nbBool]=var[ indY(iT,iN,iN2,iM,iM2)].getId();
			//					 nbBool++;
			//				}
			//		}
			//Add all z into head
			for(int i=T()*N()*M()+T()*N()*N()*M()*M();i< var.getSize();i++)
			{
				head[nbBool]=var[i].getId();
				nbBool++;
			}
			 
			if(sm == PRE_MIP_ONLY)
			PreByCalCost(true,head,nbBool,UB, &env , &cplex , &model , &var , &con);	// See above
			else PreByCalCost(false,head,nbBool,UB, &env , &cplex , &model , &var , &con);	// See above

	}
	catch (IloException& e) {
		cerr << "Concert exception caught: " << e.getMessage() << endl;
		dOptValue=9999999.0;
		isOptimal=0;
		isFeasible=0;
		//getch();
	}
	catch (int e) {
		cerr << "Known exception caught : " << e << endl;
		getch();
	}
	catch (...)
	{
		cerr << "Unknown exception caught"  << endl;
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
	printf("isFeasible:%d\nisOptimal:%d\nisTimeLim:%d\nisMemLim:%d\ndOptValue:%d\ndOptTime:%lf\niNbNodesIP:%d\ndNbMach:%lf\nPrecTime:%lf\niNbBool:%d\niNbFixed:%d\n",
		isFeasible,isOptimal,isTimeLimit, isMemLimit,(int)dOptValue,dOptTime,iNbNodesIP,dNbMach, dPreProcessingTime, nbBool, iNbFixed);

	if (DEBUG)
		getch();
	return 0;
}
