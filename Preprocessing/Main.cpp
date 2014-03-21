#pragma region INCLUDE
#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <utility>
#include <algorithm>
#include <stdio.h>
#include <math.h>
#include <ctime>
#include <process.h>
#include <Windows.h>
#include "Preprocessing.h"
#include "Data.h"
#include "CplexResult.h"
#include "PreprocessingResult.h"
using namespace std;
#include <ilcplex/ilocplex.h>
#pragma endregion

////////////////////////////////// Marcos and switchs /////
#pragma region MARCOS
#define DEBUG false
#define DEBUG_MOD false
#define CONFIG true

#define MemLimit 1024.0
#define TimeLimit 1800.0

bool ADDCUTS_C1 = false;
bool ADDCUTS_C2 = false;
bool ADDCUTS_C3 = false;
//#define LEVEL_1CUT -1
int LEVEL_1CUT = -1;
int NB_1CUT_SEUIL = 99999999;
#pragma endregion

///////////////////////////////// Declarations ///////////
#pragma  region DECLARATION_CPLEX
ILOSTLBEGIN
typedef IloFloatVarArray IloFloatVarArray1D; // 1-dimension array of float variables
typedef IloArray <IloFloatVarArray> IloFloatVarArray2D; // 2-dimension array of float variables

// Data structure from CPLEX necessary to create the model
IloEnv env;
IloModel model(env);
IloRangeArray con(env);
IloRangeArray con_cuts1(env);
IloRangeArray con_cuts2(env);
IloRangeArray con_cuts3(env);

// Data structures for the LP and preprocessing
IloNumVarArray var(env);	//It contains bool variables (x,y,z) ?be preprocessed afterwards
							//The order of elements are X(ijt),Y(ii'jj't), Z(jt). For each of the 3 parts, the up down hierarchical order is (t,i,j).
IloFloatVarArray1D lp_Z(env);
IloFloatVarArray2D lp_d(env);
IloFloatVar lp_RE(env,0,9999999);

// Functions for identifying xyz in var
int indX( int t, int i, int j){return t*N()*M()+i*M()+j;}
int indY( int t, int i1, int i2, int j1, int j2){return T()*N()*M() + t*N()*N()*M()*M()+ i1*N()*M()*M() + i2*M()*M()+ j1*M() +j2 ;}
int indZ( int t, int j){return T()*N()*M() + T()*N()*N()*M()*M() + t*M() + j;}

typedef pair<short int, IloNumVar> Term;
int Make1Cuts(const IloRangeArray & ConArr, vector<Term> & Left, int right);
void ConstructCut1();
void ConstructCut2();
void ConstructCut3();
#pragma endregion

#pragma region DECLARATION_GLOBAL
enum SolveMode{PRE_MIP_ONLY, PRE_LP_ONLY, PRE_PRE};//Mode de r¨¦solution
double dOptValue = -1, dOptTime = -1, dPreProcessingTime = -1, dNbMach = -1, dUB = -1, dLB = -1;
int isOptimal = -1, isFeasible = -1,iNbNodesIP = -1, isOptiNoPre = -1, isAllFixed = -1;
int isTimeLimit=-1, isMemLimit=-1;
int iNbFixed = -1, nbBool=-1;
PreprocessingResult res; //The final result which will be exported to file
double GetTimeByClockTicks(clock_t ticks0, clock_t ticks1){	return double(ticks1 - ticks0)/CLOCKS_PER_SEC;}
#pragma endregion 

///
/// Preprocess and solve
///
void PreByCalCost(SolveMode sm, int *head, int nbBool,int UB, IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar,IloRangeArray *pcon)
{
	double objValue=-1, temps_cpu=0, temps_cpu_pre=0;
	clock_t temp1,temp2,tempPre1,tempPre2;
	int nbFix = 0, nbNodes=0;

	Preprocessing *prepro=new Preprocessing();
	prepro->PRESetHead(head, nbBool);
	prepro->PRESetDebug(DEBUG);
	prepro->PRESetTomlin(false);
	prepro->PRESetUB(UB);		// Fix the variables using the result of an heuristique algorithme Schrage as the UB
	//pcplex->exportModel("model.lp");

	// CASE 1: No preprocessing is required before solving the IP model
#pragma region MIP_ONLY
	if(sm==PRE_MIP_ONLY)//Only for testing the correctness of LP model
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
#pragma endregion
#pragma region LP_ONLY
	else if(sm==PRE_LP_ONLY)
	{
		delete prepro;
		if (!(pcplex->solve()))							// The LP problem can't solve by cplex
		{
			if (pcplex->getCplexStatus()!=IloCplex::InfeasibleOrUnbounded	// The LP problem isn't infeasible or unbounded
				&& pcplex->getCplexStatus()!=IloCplex::Infeasible			// The LP problem is feasible
				&& pcplex->getCplexStatus()!=IloCplex::InfOrUnbd)			// The LP problem is bounded
			{
				cout<<"Failed to optimise model" <<endl;
				res.errCodeLP = 7;
				//throw(7);
			} else 
			{
				cout<<"Failed to optimise model: it is either infeasible or unbounded" <<endl;
				res.errCodeLP = 8;
				//throw(8);
			}
			
		}

		res.statusCode = pcplex->getCplexStatus();
		res.value = pcplex->getObjValue();
		res.ExportToFile("Pre_LPOnly.txt");
		cout<<"SolOptLP = "<<pcplex->getObjValue()<<endl;
		exit(0);
	}
#pragma endregion
	else // CASE 3: Preprocessing is requested before MIP is solved
	{ 
		if(DEBUG)
			pcplex->exportModel("model.lp");	

		tempPre1=clock();
		cout << "Preprocessing the model...\n";
		// Step 1: give the LP relaxation to the library
		prepro->PREInitializeLP(penv, pcplex, pmodel, pvar, pcon);
		// Step 2: solves the LP relaxation
		try{ prepro->PRESolveLP();}
		catch(int err)
		{
			res.errCodeLP = err;
			return;
		}
		//res.value = pcplex->getObjValue(); //Log first LP value

		// If no exception occurs, the instance is feasible
		//isFeasible = 1;
		
		// Step 3: preprocess by using the LP relaxation
		prepro->PREPreprocessing();
		nbFix = prepro->PREGetNbFix();
		cout<<".................... First fix "<<nbFix<<" .......................... "<<endl;

		int nbBoolExtractable = prepro->PREGetTreatedVarCount();
		// Step3.5: add cuts if needed
		if(!prepro->PREIsOptiNoPRE() && nbFix != nbBoolExtractable)
		{
			if(ADDCUTS_C1) prepro->PREAddLPCuts(&con_cuts1);
			if(ADDCUTS_C2) prepro->PREAddLPCuts(&con_cuts2);
			if(ADDCUTS_C3) prepro->PREAddLPCuts(&con_cuts3);
			if(ADDCUTS_C1 || ADDCUTS_C2 || ADDCUTS_C3) 
			{
				cout<<"\nRepreprocessing after adding cuts...\n";
				prepro->PREAddCutsToLP();
				prepro->PREPreprocessing(); //Redo preprocessing after adding cuts
				if(prepro->PREGetNbFix() >= nbBoolExtractable)//if all fixed.
					nbFix=nbBoolExtractable;
				else nbFix += prepro->PREGetNbFix();
				cout<<"................. Total fix "<<nbFix<<" .......................... "<<endl;
			}
		}

		// Step 4: analyse the situation after preprocessing
		tempPre2  = clock();
		temps_cpu_pre = GetTimeByClockTicks(tempPre1,tempPre2);
		isOptiNoPre = 0;
		isAllFixed = 0;
		nbBoolExtractable = prepro->PREGetTreatedVarCount();
		
		if(prepro->PREIsOptiNoPRE() || nbFix == nbBoolExtractable) // If no preprocessing (i.e. LB=UB before prepro)
		{
			if(prepro->PREIsOptiNoPRE())
			{
				cout << "UB=LB!" << " " << UB << " " << prepro->PREGetLpOpt() << endl;
				isOptiNoPre = 1;
				nbFix = 0;
				dOptValue = UB;
				dOptTime = dPreProcessingTime = temps_cpu_pre;
			}
			else if(nbFix == nbBoolExtractable)
			{
				cout << "IS INTEGRAL!"<<endl;
				isAllFixed = 1;
				dOptTime = dPreProcessingTime = temps_cpu_pre;
				dOptValue = prepro->PREGetLpOpt();
			}
			res.isFeasible = 1;
			res.isOptimal = 1;
			res.value = dOptValue;
		}
		dUB = prepro->PREGetUB();
		dLB = prepro->PREGetLB();
		
		//Recode preprocessing results
		res.isOptiNoPre = isOptiNoPre;
		res.isAllFixed = isAllFixed;
		res.UB = dUB;
		res.LB = dLB;
		res.nbBool = nbBool;
		res.nbBoolExtractable = nbBoolExtractable;
		res.nbFixed = nbFix;
		res.durationPre = temps_cpu_pre;
		
		//Init the reduced IP formulation
		prepro->PREInitializeMIPfromLP(penv , pcplex , pmodel , pvar, pcon,head);
		pcplex->extract(*pmodel);
	}

	// Delete the preprocessing object
	try {
		delete prepro;
	}
	catch(...) {
		cerr << "Error while deleting prepro!" << endl;
	}
}


///
/// This function initializes the LP model
///
void InitializeLPModel()
{
#pragma region NOT_CUTS
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
#pragma endregion

 if(ADDCUTS_C1) ConstructCut1();
 if(ADDCUTS_C2) ConstructCut2();
 if(ADDCUTS_C3) ConstructCut3();
 
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

 //IloExpr Fix(env);
 //Fix = var[9];
 //con.add(Fix<=0);

 if (DEBUG_MOD) printf("[DEBUG] All constraints are created and are now added to the model\n");
 model.add(con);
 if (DEBUG_MOD) 
 {
	 printf("[DEBUG] The model is created\n");
	 getch();
 }
}

/////////////////////// Cuts construction ////////////////////////
void ConstructCut1()
{
	int iLoop,iLoop2,iLoop3,iLoop4,iLoop5,iLoop6; 
 if(ADDCUTS_C1)
 {
	 // Valid inequality Cut1: if there are not enough ressources for i to be executed, not for j either, with j needing more ressources.
	 printf("[Info] Declaration of Cut1: about ressources CPU/GPU/HDD/RAM \n");
	 res.nbConCut1 = 0;
	 for (iLoop=0;iLoop<T();iLoop++)
			for (iLoop3=0;iLoop3<M();iLoop3++)
			  for (iLoop2=0;iLoop2<N();iLoop2++)
				 for (iLoop4=iLoop2+1 ;iLoop4<N();iLoop4++)
				 {	 
					 //Test the pre-assaignment to reduce the amount of cuts, but is this necessary? Should we do the same thing in other contraints?
					 if(u(iLoop2, iLoop)==1 && u(iLoop4, iLoop)==1 && q(iLoop2, iLoop3) == 1 && q(iLoop4, iLoop3) == 1)
					 {
						res.nbConCut1 += 4;
						//CPU
						IloExpr VI1_CPU(env);
						for (iLoop5=0;iLoop5<N();iLoop5++)
							if(iLoop5!=iLoop2 && iLoop5!=iLoop4 &&  u(iLoop5, iLoop)==1 && q(iLoop5, iLoop3)==1 )
								VI1_CPU += (var[indX(iLoop,iLoop5,iLoop3)] * nc(iLoop5));
						if (nc(iLoop4) >= nc(iLoop2))
							 VI1_CPU += nc(iLoop2)*(var[indX(iLoop,iLoop2,iLoop3)] + var[indX(iLoop,iLoop4,iLoop3)]);
						else VI1_CPU += nc(iLoop4)*(var[indX(iLoop,iLoop2,iLoop3)] + var[indX(iLoop,iLoop4,iLoop3)]);
						con_cuts1.add(VI1_CPU <= mc(iLoop3));

						//GPU
						IloExpr VI1_GPU(env);
						for (iLoop5=0;iLoop5<N();iLoop5++)
							if(iLoop5!=iLoop2 && iLoop5!=iLoop4  &&  u(iLoop5, iLoop)==1 && q(iLoop5, iLoop3)==1)
								VI1_GPU += (var[indX(iLoop,iLoop5,iLoop3)] * ng(iLoop5));
						if (ng(iLoop4) >= ng(iLoop2))	
							VI1_GPU += ng(iLoop2)*(var[indX(iLoop,iLoop2,iLoop3)] + var[indX(iLoop,iLoop4,iLoop3)]);
						else VI1_GPU += ng(iLoop4)*(var[indX(iLoop,iLoop2,iLoop3)] + var[indX(iLoop,iLoop4,iLoop3)]);
						con_cuts1.add(VI1_GPU <= mg(iLoop3));
						
						//HDD
						IloExpr VI1_HDD(env);
						for (iLoop5=0;iLoop5<N();iLoop5++)
							if(iLoop5!=iLoop2 && iLoop5!=iLoop4 && q(iLoop5, iLoop3)==1)
							{
								if(u(iLoop5, iLoop)==1)
									VI1_HDD += (var[indX(iLoop,iLoop5,iLoop3)] * nh(iLoop5));
								//HDD used by migration
								for (iLoop6=0;iLoop6<M();iLoop6++)
									if(iLoop6 != iLoop3 && q(iLoop5, iLoop6)==1)
										VI1_HDD += (var[indY(iLoop,iLoop5,iLoop5,iLoop6,iLoop3)] * nh(iLoop5));
							}
						if (nh(iLoop4) >= nh(iLoop2))
							VI1_HDD += nh(iLoop2)*(var[indX(iLoop,iLoop2,iLoop3)] + var[indX(iLoop,iLoop4,iLoop3)]);
						else	VI1_HDD += nh(iLoop4)*(var[indX(iLoop,iLoop2,iLoop3)] + var[indX(iLoop,iLoop4,iLoop3)]);
						con_cuts1.add(VI1_HDD <= mh(iLoop3));

						//RAM
						IloExpr VI1_RAM(env);
						for (iLoop5=0;iLoop5<N();iLoop5++)
							if(iLoop5!=iLoop2 && iLoop5!=iLoop4 && q(iLoop5, iLoop3)==1)
							{
								if(u(iLoop5, iLoop)==1)
									VI1_RAM += (var[indX(iLoop,iLoop5,iLoop3)] * nr(iLoop5));
								//RAM used by migration
								for (iLoop6=0;iLoop6<M();iLoop6++)
									if(iLoop6 != iLoop3 && q(iLoop5, iLoop6)==1)
										VI1_RAM += (var[indY(iLoop,iLoop5,iLoop5,iLoop6,iLoop3)] * nr(iLoop5));
							}
						if (nr(iLoop4) >= nr(iLoop2))
							VI1_RAM += nr(iLoop2)*(var[indX(iLoop,iLoop2,iLoop3)] + var[indX(iLoop,iLoop4,iLoop3)]);
						else VI1_RAM += nr(iLoop4)*(var[indX(iLoop,iLoop2,iLoop3)] + var[indX(iLoop,iLoop4,iLoop3)]);
						con_cuts1.add(VI1_RAM <= mr(iLoop3));
					 }
				 }
				printf("[Info]Num of Cut1 added: %d\n",res.nbConCut1);
 }
}

void ConstructCut2()
{
		printf("[Info] Declaration of Cut2\n");
		NB_1CUT_SEUIL = 0.17*N()*N()*M()*M(); //The max number of cuts 
		int iLoop,iLoop2,iLoop3,iLoop4;
		 res.nbConCut2=0 ;
		 //1-cuts for constraint A/BC/D
		for (iLoop=0;iLoop<T();iLoop++)
		 for (iLoop3=0;iLoop3<M();iLoop3++)
		 {//Loop for constructing each equation from which we will make 1-cuts
			 vector<Term> vEquationA, vEquationB, vEquationC, vEquationD;
			 vEquationA.reserve(N()+1);
			 vEquationB.reserve(N()+1);
			 vEquationC.reserve(N()*(M()+1));
			 vEquationD.reserve(N()*(M()+1));
			for (iLoop2=0;iLoop2<N();iLoop2++)
			{
				vEquationA.push_back(make_pair(nc(iLoop2), var[ indX(iLoop, iLoop2, iLoop3)]));
				if(ng(iLoop2) > 0)vEquationB.push_back(make_pair(ng(iLoop2), var[ indX(iLoop, iLoop2, iLoop3)]));
				vEquationC.push_back(make_pair(nh(iLoop2), var[ indX(iLoop, iLoop2, iLoop3)]));
				vEquationD.push_back(make_pair(nr(iLoop2), var[ indX(iLoop, iLoop2, iLoop3)]));
				for (iLoop4=0;iLoop4<M();iLoop4++)
					if (iLoop4!=iLoop3)
					{
						vEquationC.push_back(make_pair(nh(iLoop2),  var[indY(iLoop,iLoop2,iLoop2,iLoop4,iLoop3)]));
						vEquationD.push_back(make_pair(nr(iLoop2),  var[indY(iLoop,iLoop2,iLoop2,iLoop4,iLoop3)]));
					}
			}
			//Now we have 4 equations, begin to make 1-cuts
			res.nbConCut2 += Make1Cuts( con_cuts2, vEquationA, mc(iLoop3));
			if(res.nbConCut2>NB_1CUT_SEUIL){goto Label;}
			res.nbConCut2 += Make1Cuts( con_cuts2, vEquationB, mg(iLoop3));
			if(res.nbConCut2>NB_1CUT_SEUIL){goto Label;}
			res.nbConCut2 += Make1Cuts( con_cuts2, vEquationC, mh(iLoop3));
			if(res.nbConCut2>NB_1CUT_SEUIL){goto Label;}
			res.nbConCut2 += Make1Cuts( con_cuts2, vEquationD, mr(iLoop3));
			if(res.nbConCut2>NB_1CUT_SEUIL){goto Label;}
		 }
Label:  
		 cout<<"[CUT2]: nbCut = "<<res.nbConCut2<<endl;
}

void ConstructCut3()
{
	//printf("[Info] Declaration of Cut3\n");
	//int iLoop,iLoop2,iLoop3,iLoop4,iLoop5,iLoop6,iLoop7,iLoop8;
	//res.nbConCut3=0 ;
	////1-cuts for constraint A/BC/D
	//for (iLoop=0;iLoop<T();iLoop++)
	//	for (iLoop3=0;iLoop3<M();iLoop3++)
	//	{
	//	}
}

/////////////////////// Programme Principal /////////////////////////
void SomeTest();
double CountPMsTurnedOn(IloCplex *pcplex);//ounts the number of machines which are turned on, on the average, at any time t
void SetVector( IloCplex& cplex, char* filename);
#define ENABLE_CMD_PARAM false

int main(int argc, char* argvs[])
{
	float f=1;double d=1; int i=1;
	printf("%f,%lf,%d",f,d,i);


	//SomeTest();return 1;
	int UB = 99999999;
	//UB = 515201;
	//UB = 465172;
	//UB=594336; //4_10
	UB=831337;//4_4
	if(ENABLE_CMD_PARAM)
	{
		if(argc < 2){cerr<<"Syntax: Preprocessing.exe UB [CutsToAdd]\n   Params: CutsToAdd A bitflag int indicating which cuts to add. Ex: 1->addCut1, 6->addCut3&2. Mind the order.\n"<<endl; abort();}
		for(int i=0; i<argc; i++)
			cout<<argvs[i]<<endl;
		UB = atoi(argvs[1]);
		if(argc==3) //Add Cuts
		{
			int flag = atoi(argvs[2]);
			ADDCUTS_C1 = flag % 2;
			ADDCUTS_C2 = (flag >> 1)%2;
			ADDCUTS_C3 = (flag >> 2)%2;

			///!Tmp
			//LEVEL_1CUT = fla g;
			//ADDCUTS_C1=ADDCUTS_C3=ADDCUTS_C2=0;
		}
		GetData();
	}
	else	
		GetData("Donnees/donnees4_16.dat");



	//ADDCUTS_C1=true;
	ADDCUTS_C2=true;
	SolveMode sm = PRE_PRE; //Solve mode
	clock_t ticks0;
	if (DEBUG) DisplayData();
	//SomeTest();
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
			//if(argc != 3) //argc==3 => preprocess X only
			//{
				//Add all y into head
				for(int i=T()*N()*M(); i<T()*N()*M()+T()*N()*N()*M()*M();i++)
				{
					head[nbBool]=var[i].getId();
					nbBool++;
				}
				//Add all z into head
				for(int i=T()*N()*M()+T()*N()*N()*M()*M();i< var.getSize();i++)
				{
					head[nbBool]=var[i].getId();
					nbBool++;
				}
			//}
			PreByCalCost(sm,head,nbBool, UB, &env , &cplex , &model , &var , &con);	// See above
			delete [] head;

		
			if(  res.errCodeLP!=-1 || res.isOptimal==1)///!TODO. we don't solve MIP if the preprocessing didn't work!
				res.isMIPExecuted = 0; // MIP not executed
			else
			{
				res.isMIPExecuted = 1;
				dNbMach=-1.0;
				///Set Vectors
				SetVector( cplex, "SolVector.out");

				if (!cplex.solve())
				{ // cplex fails to solve the problem
					dOptValue=-1;
					isOptimal=0;
					isFeasible=0;
				} else
				{ // We now test if the solution found is optimal or feasible
					printf("Cplex status: %ld\n",cplex.getCplexStatus());
					fflush(stdout);
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
				//printf("\nValeur de la fonction objectif : %lf\n",(double)cplex.getObjValue());
			}
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
		getchar();
	}
	catch (...)
	{
		throw;
		cerr << "Unknown exception caught"  << endl;
		dOptValue=9999999.0;
		isOptimal=0;
		isFeasible=0;
		//getch();
	}

	dOptTime = GetTimeByClockTicks( ticks0, clock());
	//printf("isFeasible:%d\nisOptimal:%d\nisTimeLim:%d\nisMemLim:%d\ndOptValue:%d\ndOptTime:%lf\niNbNodesIP:%d\ndNbMach:%lf\nPrecTime:%lf\niNbBool:%d\niNbFixed:%d\n",
		//isFeasible,isOptimal,isTimeLimit, isMemLimit,(int)dOptValue,dOptTime,iNbNodesIP,dNbMach, dPreProcessingTime, nbBool, iNbFixed);
		
	if(res.isMIPExecuted)
	{
		//We put the test of limit here because an cplex exception can also be caused by Limit.
		//in which case there is no solution found but the status code is set. We count this case
		//in #TimLim or #MemLim but not in #inFea.
		if (cplex.getCplexStatus()== IloCplex::AbortTimeLim) isTimeLimit = 1;
		else if (cplex.getCplexStatus()== IloCplex::MemLimFeas) isMemLimit = 1;
		res.isFeasible = isFeasible;
		res.isOptimal = isOptimal;
		res.isTimeLimit = isTimeLimit;
		res.isMemLimit = isMemLimit;
		res.nbMachine = dNbMach;
		res.nbNode = iNbNodesIP;
	}
	res.durationCpuClock = dOptTime;
	res.statusCode = cplex.getCplexStatus();
	res.value = dOptValue;
	res.ExportToFile("Preproc.txt");
	res.Test();
	if (DEBUG)
		_getch();
	return 0;
}


////////////////////// Utility functions /////////////////
#pragma region UTILITY
/// This function can generate 1-cuts from the inequality provied. The coefficients of the inequality should > 0;
int Make1Cuts(const IloRangeArray & ConArr, vector<Term> & Left, int Right)
{
	// Sort terms
	struct{bool operator()(Term a, Term b) { return a.first > b.first; }} op;
	sort(Left.begin(), Left.end(), op);
	//for_each(Left.begin(), Left.end(), [](const Term & t){ cout<<t.first<<endl;});

	int nbCuts = 0;
	unsigned int s = Left[0].first;
	unsigned int i=0, j=1, l;
	int k=1;
	while(j+2<Left.size())
	{
		s+= Left[j].first;
		l=j+1;
		if(s > Right)
		{
			while( l<Left.size() && (s- Left[i].first + Left[l].first)> Right )
			{
				s = s- Left[i].first + Left[l].first;
				i++; l++;
			}
			IloExpr C2(env);
			for(int iLoopCon=0; iLoopCon<l; iLoopCon++)
			{
				C2 += Left[iLoopCon].second;
				//cout<<Left[iLoopCon].first<<"+";
			}
			con_cuts2.add(C2 <= k);//cout<<"<="<<k<<endl;
			nbCuts ++;
			j=l;
		}else j++;
		
		if(k == LEVEL_1CUT)break;	//1-Cut level control
		k++;
	}
	return nbCuts;
}

void TestIdenticalMach()
{ 
	//char filename[]="Donnees/donnees8_20.dat ";
	//for(int i=1; i<=8; i++)
	//	for(int j=1; j<=20; j++)
	//	{
	//		sprintf(filename, "Donnees/donnees%d_%d.dat", i,j);
	//		GetDate(filename);

	//	}
}

void SomeTest()
{
	cout<<"[Temporary test:]"<<endl;
	//for(int i=0; i<N(); i++)cout<<ncSortedInd(i)<<endl;
	//Test 1-cuts:
	IloNumVar x1(env), x2(env), x3(env), x4(env), x5(env);
	vector<Term> v;
	v.push_back(make_pair(3, x1));
	v.push_back(make_pair(4, x2));
	v.push_back(make_pair(2, x3));
	v.push_back(make_pair(3, x4));
	v.push_back(make_pair(1, x5));
	Make1Cuts(con_cuts2, v, 6);
	exit(0);
}

double CountPMsTurnedOn(IloCplex *pcplex)
{
 int iLoop;
 double dCount=0;

 for (iLoop=0;iLoop<T();iLoop++)
	 dCount+=pcplex->getValue(lp_Z[iLoop]);
 return (dCount/=(double)T());
}

//Read var vector from file. The file contains the indices of vars in var[], whose value=1 according to the solution of H2
void SetVector( IloCplex& cplex, char* filename)
{
	
	int size = T()*N()*M() + T()*N()*N()*M()*M() + T()*M();
	//IloNumArray *pvals=new IloNumArray(env, size);
	IloNumArray vals(env);//,size);
	IloNumVarArray vars(env);
	//double *vals = new double[size];
	//for(int i=0; i<size; i++){vals[i]=0;}
	FILE* f = fopen(filename, "rt");
	int indVar;
	int counter=0;
	float tmp=0;
	while( fscanf(f,"%d,%f\n", &indVar, &tmp)!=EOF )
	{
		counter++;
		//printf("%d,%f\n",indVar, vals[indVar]);
		//vals[indVar]=tmp;
		vars.add(var[indVar]);
		vals.add(tmp);
		//printf("%f\n",vals[indVar]);
		//var[indVar].setBounds(1,1);
	}
	fclose(f);
	printf("Read: counter=%d, valsSize=%d,size=%d",counter, size,size);
	//for(int i=0; i<size; i++){printf("%d\n",vals[i]);}
	cplex.setVectors(vals, 0,vars,0,0,0);
}
#pragma endregion