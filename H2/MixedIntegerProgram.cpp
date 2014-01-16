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
  pdMIPVariables=NULL;
}

/*******************************************************************************************************************************
Name: MIPInitialize
********************************************************************************************************************************
Description: Initialize the MIP problem
********************************************************************************************************************************
Input: The Envirenment, the variables, the model, the cplex solver and the constraints of a MIP problem
Necessitates: All the four input arguments are well allocated
Output: Nothing
leads to: Obtain the four arguments needed to create a MIP model and set bMIPisSet to true.                           
          It also initializes some attributes of MixedIntegerProgram.
*******************************************************************************************************************************/
void MixedIntegerProgram::MIPInitialize(IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar,IloRangeArray *pcon)
{
	printf("Initialize a MIP problem\n");
	MIPcplex=pcplex;
	MIPmodel=pmodel;
	MIPvar=pvar;
	MIPcon=pcon;
	
	pdMIPVariables=new double[MIPvar->getSize()];
	bMIPisSet=true;
}


/*******************************************************************************************************************************
Name: MIPInitializeCplexParameters
********************************************************************************************************************************
Description: Initialize the Cplex parameters
********************************************************************************************************************************
Input: iDisplay 0 to hide the logs of cplex, 1 to show them
	   dMaxMo is the maximum tree memory limit (in megabytes) usable by cplex (used to avoid out of memory problems)
	      default=1e+75
	   iNodeSel MIP node selection strategy (0 Depth-first search, 1 Best-bound search, 2 Best-estimate search, 3 Alternative best-estimate search)
		  default=1
Necessitates: iDisplay = {0,2}
			  dMaxMo >=0
			  iNodeSel = {0,3}
Output: Nothing
leads to: MIPcplex parameters changed with the input values
*******************************************************************************************************************************/
void MixedIntegerProgram::MIPInitializeCplexParameters(int iDisplay, double dMaxMo, int iNodeSel) {
	MIPcplex->setParam(IloCplex::MIPDisplay,iDisplay);		// Showing the resolution of the MIP
	MIPcplex->setParam(IloCplex:: SimDisplay,iDisplay);		// Showing the details of the simulation of cplex
	MIPcplex->setParam(IloCplex::TreLim, dMaxMo);			// Limitate the tree size memory
	MIPcplex->setParam(IloCplex::NodeSel,iNodeSel);			// Change de node exploration strategy
}

/*******************************************************************************************************************************
Name: MIPSolveByCplex
********************************************************************************************************************************
Description: Solve a MIP problem by Cplex 
********************************************************************************************************************************
Input: Nothing
Necessitates: MIP problem should be well initialized
Output: Nothing
leads to: Use cplex to solve the MIP problem and get the optimal solution and set bMIPisSolved to true
*******************************************************************************************************************************/
void MixedIntegerProgram::MIPSolveByCplex()
{
	int i;
	bool b;

	if(!bMIPisSet)
	{
		//cout<<"The MIP cannot be solved as it has not been initialized"<<endl;
		throw(2);
	}

	printf("Solve the MIP problem with Cplex\n");
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
	(*MIPvar)[iIndex].setLB(iValue);								//Fix the (iIndex)th variable to iValue by change its upper and lower bounds
	(*MIPvar)[iIndex].setUB(iValue);
}
