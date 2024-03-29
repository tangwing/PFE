#include "MixedIntegerProgram.h"

/*******************************************************************************************************************************
Name: MixedIntegerProgram
********************************************************************************************************************************
Description: Default constructor: before using the model, the method MIPInitialize() must be called
********************************************************************************************************************************
Input: Nothing
Necessitates: nil
Output: Nothing
leads to: Initialize some attributes of MixedIntegerProgram
*******************************************************************************************************************************/
MixedIntegerProgram::MixedIntegerProgram(void)
{
  bMIPisSolved=false;
  bMIPisSet=false;
  dMIPOptimalValue=IloInfinity;
  bMIPisOptimal=false;
  pdMIPVariables=NULL;
}

/*******************************************************************************************************************************
Name: MIPInitialize
********************************************************************************************************************************
Description: Initialize the MIP problem
********************************************************************************************************************************
Input: The Environnement, the variables, the initial value of the variables, the model, the cplex solver and the constraints of a MIP problem
Necessitates: All the four input arguments are well allocated
Output: Nothing
leads to: Obtain the four arguments needed to create a MIP model and set bMIPisSet to true.                           
          It also initializes some attributes of MixedIntegerProgram.
*******************************************************************************************************************************/
void MixedIntegerProgram::MIPInitialize(IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar, IloRangeArray *pcon)
{
	//printf("Initialize a MIP problem\n");
	MIPcplex=pcplex;
	MIPmodel=pmodel;
	MIPvar=pvar;
	MIPcon=pcon;
	
	pdMIPVariables=new double[MIPvar->getSize()];
	bMIPisSet=true;
}

/*******************************************************************************************************************************
Name: MIPSolveByCplex
********************************************************************************************************************************
Description: Solve a MIP problem by Cplex 
********************************************************************************************************************************
Input: Nothing
Necessitates: MIP problem must be correctly initialized
Output: Nothing
leads to: Use cplex to solve the MIP problem and get the optimal solution and set bMIPisSolved to true
*******************************************************************************************************************************/
void MixedIntegerProgram::MIPSolveByCplex()
{
	int i;
	bool b;

	if(!bMIPisSet)
	{
		cout<<"The MIP cannot be solved as it has not been initialized"<<endl;
		throw(2);
	}

	//printf("Solve the MIP problem with Cplex\n");
	b = MIPcplex->solve();
	if (MIPcplex->getCplexStatus()==IloCplex::Optimal || MIPcplex->getCplexStatus()==IloCplex::OptimalTol) // See if CPLEX found the optimal solution or not
		bMIPisOptimal=true;
    else 
		bMIPisOptimal=false;
	if (!b)							// The MIP problem can't solve by cplex
	{
	   if (MIPcplex->getCplexStatus()!=IloCplex::InfeasibleOrUnbounded	// The MIP problem isn't infeasible or unbounded
		   && MIPcplex->getCplexStatus()!=IloCplex::Infeasible			// The MIP problem is feasible
		   && MIPcplex->getCplexStatus()!=IloCplex::InfOrUnbd)			// The MIP problem is bounded
	   {
		   cout<<"Failed to optimise model" <<endl;
		   throw(7);
	   } else 
	   {
		   cout<<"Failed to optimise model: it is either infeasible or unbounded" <<endl;
		   throw(8);
	   }
	}
	iMIPNbNode=MIPcplex->getNnodes();
	dMIPOptimalValue = MIPcplex->getObjValue();
	bMIPisSolved=true;
}

/*******************************************************************************************************************************
Name: MIPAddFix
********************************************************************************************************************************
Description: Fix the value of variables to the MIP model
********************************************************************************************************************************
Input: iIndex (the index of the variable will be fixed) and iValue the value to fix
Necessitates: nil
Output: Nothing
leads to: It will add the same UB and LB(0 or 1) to a variable in the MIP model
*******************************************************************************************************************************/
void MixedIntegerProgram::MIPAddFix(int iIndex, int iValue)
{
	(*MIPvar)[iIndex].setLB(iValue);	//Fix the (iIndex)th variable to iValue by change its upper and lower bounds
	(*MIPvar)[iIndex].setUB(iValue);
}

void MixedIntegerProgram::LPAddCuts(IloConstraintArray* cuts)
{
	MIPmodel->add(*cuts);
}

void MixedIntegerProgram::LPRemoveCuts(IloConstraintArray* cuts)
{
	MIPmodel->remove(*cuts);
}
