#include "Preprocessing.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <process.h>
#include <malloc.h>

#define EPSILON 0.0001

ILOSTLBEGIN

/*******************************************************************************************************************************
Name: Preprocessing
********************************************************************************************************************************
Description: Constructor
********************************************************************************************************************************
Input: Nothing
Necessitates: nil
Output: Nothing
leads to: Initialize the attributes of Preprocessing and allocate the memory of pointers.
*******************************************************************************************************************************/
Preprocessing::Preprocessing()
{
	PRElp=new LinearProgram();
	PREmip=new MixedIntegerProgram();
	bPREisUB=false;
	bPREisSet=false;
	bPREisSolved=false;
	iPREnbFix=0;
	iPREVarBeg=0;
	iPREVarEnd=0;
	bPREisDebug=true;
	bPRETomDrie=false;
	bPREhasHead=false;
}

/*******************************************************************************************************************************
Name: PREInitializeLP
********************************************************************************************************************************
Description: Initialize a LP problem
********************************************************************************************************************************
Input: The Envirenment, the variables, the model, the cplex solver and the constraints of a LP problem
Necessitates: All the four input arguments are well allocated
Output: Nothing
leads to: It pass the four arguments to the class PRElp to initialize the LP problem and set PREisSet to true.                           
          It also initialize some attributes of Preprocessing.
*******************************************************************************************************************************/
void Preprocessing::PREInitializeLP(IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar,IloRangeArray *pcon)
{
	PREenv=penv;
	PREvar=pvar;
	PRElp->LPInitialize(penv,pcplex,pmodel,pvar,pcon);
	PREcutsLP=new IloConstraintArray(*penv);
	iPREnbVar=pvar->getSize();
	iPREVarEnd=iPREnbVar;
	pdPREFixedVariables=new double[pvar->getSize()];
	PREvarInfo=new Variable[pvar->getSize()];
	iPREnbBool=0;
	for(int i=0;i<pvar->getSize();i++) {
	pdPREFixedVariables[i]=IloInfinity;
	if((*PREvar)[i].getId() == ipPREhead[iPREnbBool]) {
			pdPREFixedVariables[i]=-1;
			iPREnbBool++;
	}
	}
	bPREisSet=true;
	if(bPREisDebug) cout<<"Succes to initialize a LP problem"<<endl;
}

/*******************************************************************************************************************************
Name: PREInitializeMIP
********************************************************************************************************************************
Description: Initialize a MIP problem
********************************************************************************************************************************
Input: The Environnement, the variables, the model, the cplex solver and the constraints of a MIP problem
Necessitates: All the four input arguments are well allocated
Output: Nothing
leads to: It pass the four arguments to the class PREmip to initialize the MIP problem and set PREisSet to true.
		  And it give the quantity of the variable.
*******************************************************************************************************************************/
void Preprocessing::PREInitializeMIP(IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar,IloRangeArray *pcon)
{
	PREenv=penv;
	PREmip->MIPInitialize(penv,pcplex,pmodel,pvar,pcon);
	PREcutsMIP=new IloConstraintArray(*penv);
	iPREnbVar=pvar->getSize();
	bPREisSet=true;
	if(bPREisDebug) cout<<"Succes to initialize a MIP problem"<<endl;
}

/*******************************************************************************************************************************
Name: PREInitializeMIPfromLP
********************************************************************************************************************************
Description: Initialize a MIP from a existe LP
********************************************************************************************************************************
Input: The Environnement, the variables, the model, the cplex solver and the constraints of the LP and a indice array of the bool variables to be fixed
Necessitates: All the four input arguments are well allocated
Output: Nothing
leads to: It pass the the arguments to the class PREmip from a existe LP to initialize the MIP and set PREisSet to true.
*******************************************************************************************************************************/
void Preprocessing::PREInitializeMIPfromLP(IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar,IloRangeArray *pcon,int *head)
{
  PREenv=penv;
  iPREnbVar=pvar->getSize();
  int i;
  for(i=0;i<iPREnbVar;i++)
	  if(pdPREFixedVariables[i]!=IloInfinity)
	  {
		  pmodel->add(IloConversion(*penv,(*pvar)[i],ILOBOOL));
	  }
  PREmip->MIPInitialize(penv,pcplex,pmodel,pvar,pcon);
  bPREisSet=true;
  if(bPREisDebug) cout<<"Succes to initialize a MIP problem"<<endl;
}

/*******************************************************************************************************************************
Name: PREArSup
********************************************************************************************************************************
Description: Round up a real value.
********************************************************************************************************************************
Input: A value of double
Necessitates: nil
Output: A value of double
leads to: It Returns the smallest integral value that is not less than the value given.
*******************************************************************************************************************************/
double Preprocessing::PREArSup(double value)
{
 return (ceil(value-0.001)); 
}


/*******************************************************************************************************************************
Name: PREFixVarToLP
********************************************************************************************************************************
Description: Fix the value of variables to the LP model
********************************************************************************************************************************
Input: Nothing
Necessitates: nil
Output: Nothing
leads to: It will add the same UB and LB(0 or 1) to a variable in the LP model
*******************************************************************************************************************************/
void Preprocessing::PREFixVarToLP()
{
	iPREnbFix=0;
	for(int i=0;i<iPREnbVar;i++)
	{
		if(pdPREFixedVariables[i]!=IloInfinity && pdPREFixedVariables[i]!=-1)
		{
			PRElp->LPAddFix(i,pdPREFixedVariables[i]);
			if(bPREisDebug) 
				//cout<<"I have fixed the var "<<(*PREvar)[i].getName()<<" to "<<pdPREFixedVariables[i]<<" !"<<endl;
			iPREnbFix++;
		}
	}

}

/*******************************************************************************************************************************
Name: PREFixVarToMIP
********************************************************************************************************************************
Description: Fix the value of variables to the MIP model
********************************************************************************************************************************
Input: Nothing
Necessitates: nil
Output: Nothing
leads to: It will add the same UB and LB(0 or 1) to a variable in the MIP model
*******************************************************************************************************************************/
void Preprocessing::PREFixVarToMIP()
{
	iPREnbFix=0;
	for(int i=0;i<iPREnbVar;i++)
	{
		if(pdPREFixedVariables[i]!=IloInfinity && pdPREFixedVariables[i]!=-1)
		{
			PREmip->MIPAddFix(i,pdPREFixedVariables[i]);
			if(bPREisDebug) 
				//cout<<"I have fixed the var "<<i<<" to "<<pdPREFixedVariables[i]<<"!"<<endl;
			iPREnbFix++;
		}
	}
}

/*******************************************************************************************************************************
Name: PRESolveLP
********************************************************************************************************************************
Description: Solve a LP problem by Cplex 
********************************************************************************************************************************
Input: Nothing
Necessitates: LP problem should be well initialized
Output: Nothing
leads to: It use cplex to solve a LP problem and get the optimal solution
*******************************************************************************************************************************/
void Preprocessing::PRESolveLP() 
{ //Solve a LP problem by Cplex
 if(bPREisDebug) cout<<"I try to solve a LP problem by cplex"<<endl;
 if(PRElp->LPisSet()) 
 {
	 PRElp->LPSolveByCplex();
	 dPRElpOpt=PRElp->LPGetOptValue();
	 dPRELB=dPRElpOpt;
	 PRElp->LPGetVarResults(PREvarInfo);	// Sets the costs, and values of the Variable array
	 for(int i=0; i<PREvar->getSize(); i++)
		 if((PREvarInfo[i]).VARGetExtractable()==false)
			 pdPREFixedVariables[i]==IloInfinity;
	 if(bPREisDebug) cout<<"Solved!"<<endl;
 } else
 {
	 cout << "ERROR: the LP model has not been intialized and thus cannot be solved\n";
	 throw(5);
 }
}

/*******************************************************************************************************************************
Name: PRESolveMIPgui
********************************************************************************************************************************
Description: Solve a MIP problem by Cplex 
********************************************************************************************************************************
Input: Nothing
Necessitates: MIP problem should be well initialized
Output: Nothing
leads to: It use cplex to solve a MIP problem and get the optimal solution
*******************************************************************************************************************************/
void Preprocessing::PRESolveMIP()
{ //Solve a MIP problem by Cplex
	if(bPREisDebug) cout<<"I try to solve a MIP problem by cplex"<<endl;
	if(PREmip->MIPisSet())
	{
		PREmip->MIPSolveByCplex();
		dPREmipOpt=PREmip->MIPGetOptValue();
		if(bPREisDebug) cout<<"Solved!"<<endl;
	}else
	{
		 cout << "ERROR: the MIP model has not been intialized and thus cannot be solved\n";		
		 throw(6);
	}
}

		

/*******************************************************************************************************************************
Name: PREGetMIPResVar
********************************************************************************************************************************
Description: Get the values of the varialbes after the MIP problem solved by Cplex
********************************************************************************************************************************
Input: Nothing
Necessitates: The MIP problem must be solved
Output: A double array who saves the values of the variables
leads to: Obtain the values of the varialbes of MIP problem after it is solved by cplex
*******************************************************************************************************************************/
double* Preprocessing::PREGetMIPResVar()
{ //Get the values of the varialbes after the MIP problem solved by Cplex
	if(bPREisDebug) cout<<"Get the optimal solution of the MIP problem"<<endl;
	if (PREmip->MIPisSolved())
		return PREmip->MIPGetvariables();
	else 
	{
	 cout << "ERROR: the MIP model has not been solved\n";		
	 throw(2);
	}
}	

/*******************************************************************************************************************************
Name: PREPreprocessing
********************************************************************************************************************************
Description: This method implements the preprocessing algorithm
********************************************************************************************************************************
Input: Nothing
Necessitates: The variables between indexed in the head array must be boolean variables
Output: Nothing
leads to: the variables in head have been preprocessed and potentially fixed in the LP model 
          to fix them in the MIP model, use method PREfixVarToMIP()
Return: true if the programm did the preprocessing, false if the LB=UB
*******************************************************************************************************************************/
bool Preprocessing::PREPreprocessing()
{
	if(bPREisDebug) cout<<"I start fixing the variables"<<endl;
	double dOpt;
	bool bLbisUb=false, bisIntegral=true;
	int iOldnbFix;
	bPREOptiNoPRE=false;
	dPRELB=dPRElpOpt;
	iPREnbFix=0;

	if(bPREisDebug) {
		ofstream logs("logs/LOGS.txt", ios::app);
		logs << "LP: " << dPRELB << endl << "Shrage: " << dPREUB << endl;
		logs.close();
	}
	try
	{
		if(dPRELB-dPREUB>-0.0001) {
			bPREOptiNoPRE=true;
		}
		else {
			do
			{
				if(dPRELB-dPREUB>-0.0001) {
					bLbisUb=true;
					iPREnbFix=iPREnbBool;
				}
				else {
					dOpt=PREArSup(dPRElpOpt);		// Rounding up the optimal solution of the LP
					iOldnbFix=iPREnbFix;
					PREFixVar();					// Preprocessing on the variables
					if(bPREisDebug) cout<<"Calculate finish!"<<endl;
					PRESolveLP();
				}
				//cout << "One loop done : nb fix var = "<<iPREnbFix<<endl;
			}
			while(!bLbisUb && (dOpt!=PREArSup(dPRElpOpt)||iOldnbFix<iPREnbFix)); // It repeats until there is no more variable that can be fixed
		}
		bPREisSolved=true;

		if(bPREisDebug) {
			ofstream logs("logs/LOGS.txt", ios::app);
			if (!bPREOptiNoPRE)
				logs<<"Fixed "<<100.0*double(iPREnbFix)/double(iPREnbBool)<< "% of the boolean variables!" << " (" << iPREnbFix << " out of " << iPREnbBool << " boolean variables)" <<endl;	// Showing the percentage of fixed boolean variables
			else 
				logs<<"LB = UB No Preprocessing!"<<endl;
			logs.close();
		}
	}
	catch(int e)
	{
		switch (e)
		{
			case 1:
				cerr<<"ERROR: The LP hasn't been solved"<<endl;
				if(bPREisDebug) getchar();
				break;
			case 2:
				cerr<<"ERROR: The MIP hasn't been solved"<<endl;
				if(bPREisDebug) getchar();
				break;
			case 3:
				cerr<<"ERROR: The Preprocessing hasn't been completed"<<endl;
				if(bPREisDebug) getchar();
				break;
			case 4:
				cerr<<"ERROR: The UB hasn't been set"<<endl;
				if(bPREisDebug) getchar();
				break;
			case 5:
				cerr<<"ERROR: The LP hasn't been initialized"<<endl;
				if(bPREisDebug) getchar();
				break;
			case 6:
				cerr<<"ERROR: The MIP hasn't been initialized"<<endl;
				if(bPREisDebug) getchar();
				break;
			case 7:
				cerr<<"ERROR: CPLEX has failed to optimise the model"<<endl;
				if(bPREisDebug) getchar();
				break;
			case 8:
				cerr<<"ERROR: CPLEX has failed to optimise the model: it's either infeasible or unbounded"<<endl;
				if(bPREisDebug) getchar();
				break;
			default:
				cerr<<"ERROR: Unknown exception in the preprocessing library"<<endl;
				if(bPREisDebug) getchar();
				break;
		}
	}
    
	// We test the integrality of the continuous solution: if it is integral, then it is optimal for the MIP
	for(int i=0;i<PREvar->getSize() && bisIntegral;i++) 
		if (pdPREFixedVariables[i]!=IloInfinity && PREvarInfo[i].VARGetValue()>0.000001 && PREvarInfo[i].VARGetValue()<0.999999)
			bisIntegral=false;
	if (bisIntegral)
	{
		dPRELB=dPREUB=dPRElpOpt;
		iPREnbFix=iPREnbBool;
		cout<<"IS INTEGRAL!"<<endl;
	}

	return !bPREOptiNoPRE;
}

/*******************************************************************************************************************************
Name: PREToFix
********************************************************************************************************************************
Description: This method is to detect if a variable should be check and fixed.
********************************************************************************************************************************
Input: indice is the indice of a variable to be detected
       head is a indice array for the bool variables
Necessitates: nil
Output: true if the variable is in the array to be fixed
        false if not
leads to: nil
Note : This method is outdated and should not be used
*******************************************************************************************************************************/
bool Preprocessing::PREToFix(int VarId,int * head)
{
	if( VarId <0) return false;
	else
	{
		int i;
		for(i=0;;i++)
		{
			if(VarId==head[i])
			{
				return true;
			}
			else if (head[i]==-1) return false;
		}
	}
}

/*******************************************************************************************************************************
Name: PREFixVar
***************************************************************************************************************************************************
Description: Fix the variables off-base by comparing the reduced cost with the difference between UB and LB
             Fix the variables in-base by comparing the pseudo-cost with the difference between UB and LB
***************************************************************************************************************************************************
Input: Nothing
Necessitates: The LP problem should be solved and Preprocessing has a UB(Upper Bound)
Output: Nothing
leads to: Compare the reduced cost of the off-base variables with the difference between UB and LB.
          Compare the pseudo-cost of the in-base variables with the difference between UB and LB.
          And fix the value of variables
**************************************************************************************************************************************************/

void Preprocessing::PREFixVar()
{
	int i;			// This double array is to save the reduced cost and return by the function PREGetReducedCost

	if(bPREisDebug) cout<<"All data get"<<endl;

	ofstream logs;
	if(bPREisDebug) logs.open("logs/LOGS.txt", ios::app);		

	if(!PRElp->LPisSolved()) // ERROR: The LP problem has not been solved!
		throw(1);
	else if(!bPREisUB) // ERROR: The Upper bound(UB) has not been initialized
		throw(4);
	else 
	{
		dPRELB=PRElp->LPGetOptValue();
		if(dPRELB-dPREUB<-0.0001) // LB no more than UB
		for(i=0;i<PREvar->getSize();i++)
		{
		  if(pdPREFixedVariables[i]==-1)
		  {			  
			  if(PREvarInfo[i].VARGetBasisStatus()==0)	// Off-base variable
			  {
			    if(bPREisDebug)
					logs  << "RedCost[" <<i<<"]: " << PREvarInfo[i].VARGetRedCost() << endl;
				if(PREvarInfo[i].VARGetRedCost()>0 //If r_ij>UB-LB, it will fix the variable i to 0
					&& PREvarInfo[i].VARGetRedCost()>dPREUB-dPRELB+EPSILON)	
				{
					if(bPREisDebug)
						logs  << "RedCost[" <<i<<"]: " << PREvarInfo[i].VARGetRedCost() << " > " << dPREUB-dPRELB+EPSILON << " LB: " << dPRELB << " UB: " << dPREUB << endl;
					pdPREFixedVariables[i] = 0;
					(*PREvar)[i].setUB(0.0);
					iPREnbFix++;
				}
				if(PREvarInfo[i].VARGetRedCost()<0 
					&& PREvarInfo[i].VARGetRedCost()<dPRELB-dPREUB-EPSILON && (*PREvar)[i].getLB() != 1)	//If r_ij>UB-LB, it will fix the variable i to 1
				{
					if(bPREisDebug)
						logs << "RedCost["<<i<<"]: " << PREvarInfo[i].VARGetRedCost()<< " < " << dPRELB-dPREUB-EPSILON << " LB: " << dPRELB << " UB: " << dPREUB << endl;
								
					pdPREFixedVariables[i] = 1;
					(*PREvar)[i].setLB(1.0);
					iPREnbFix++;
				}
			  }
			  else if(PREvarInfo[i].VARGetBasisStatus()==1)	// In-base variables
			  {
				  if (PREvarInfo[i].VARGetUj()<99999999.0 // I try to fix variables to 0
					  && (1.0-PREvarInfo[i].VARGetValue())*PREvarInfo[i].VARGetUj()>dPREUB-dPRELB+EPSILON)   
					{ 
						pdPREFixedVariables[i] = 0;
						(*PREvar)[i].setUB(0.0);
						iPREnbFix++;
						
						if(bPREisDebug) {
							logs << "Uj["<<i<<"]: " << PREvarInfo[i].VARGetUj() << " " <<(1.0-PREvarInfo[i].VARGetValue())*PREvarInfo[i].VARGetUj()<< " > " << dPREUB-dPRELB+EPSILON<< " LB: " << dPRELB << " UB: " << dPREUB << endl;
							//cout<<"Fix "<< (*PREvar)[i].getName()<<" to 0 by pseudo-cost"<<endl;
						}
					}
					if (PREvarInfo[i].VARGetLj()<99999999.0 // I try to fix variables to 1
						&& PREvarInfo[i].VARGetValue()*PREvarInfo[i].VARGetLj()>dPREUB-dPRELB+EPSILON)   
					{ 
						pdPREFixedVariables[i] = 1;
						(*PREvar)[i].setLB(1.0);
						iPREnbFix++;
						
						if(bPREisDebug) {
							logs << "Lj["<<i<<"]: " << PREvarInfo[i].VARGetUj() <<" "<<PREvarInfo[i].VARGetValue()*PREvarInfo[i].VARGetLj()<<">"<<dPREUB-dPRELB+EPSILON << " LB: " << dPRELB << " UB: " << dPREUB << endl;
							//cout<<"Fix "<<(*PREvar)[i].getName()<<" to 1 by pseudo-cost"<<endl;
						}
					}
				}	
			}
		}
		  if(bPREisDebug)
			logs.close();
	}
}

void Preprocessing::PRESetHead(int *head) {
	ipPREhead=head;
	bPREhasHead=true;
}

/*******************************************************************************************************************************
Name: PreAddCuts
***************************************************************************************************************************************************
Description: Adds the cuts stored in the object PREcuts into the LP model
***************************************************************************************************************************************************
Input: Nothing
Necessitates: The LP must be initialized.
Output: Nothing
leads to: The LP model has the cuts stored in the object PREcuts
**************************************************************************************************************************************************/
void Preprocessing::PREAddCutsToLP()
{
	/*if (bPREisDebug) */printf("Number of added cuts : %d\n",PREcutsLP->getSize());
	if (PREcutsLP->getSize()>0)
		PRElp->LPAddCuts(PREcutsLP);
}

/*******************************************************************************************************************************
Name: PreAddCutsToMIP
***************************************************************************************************************************************************
Description: Adds the cuts stored in the object PREcuts into the MIP model
***************************************************************************************************************************************************
Input: Nothing
Necessitates: The MIP must be initialized.
Output: Nothing
leads to: The MIP model has the cuts stored in the object PREcuts
**************************************************************************************************************************************************/
void Preprocessing::PREAddCutsToMIP()
{
	/*if (bPREisDebug) */printf("Number of added cuts : %d\n",PREcutsMIP->getSize());
	if (PREcutsMIP->getSize()>0)
		PREmip->LPAddCuts(PREcutsMIP);
}

/*******************************************************************************************************************************
Name: PRERemoveCuts
***************************************************************************************************************************************************
Description: Removes the cuts stored in the object PREcuts from the LP model
***************************************************************************************************************************************************
Input: Nothing
Necessitates: The LP must be initialized.
Output: Nothing
leads to: The LP model doesn't have the cuts stored in the object PREcutsLP
**************************************************************************************************************************************************/
void Preprocessing::PRERemoveCutsFromLP()
{
	if (bPREisDebug) printf("Number of removed cuts : %d\n",PREcutsLP->getSize());
	if (PREcutsLP->getSize()>0)
		PRElp->LPRemoveCuts(PREcutsLP);
	delete PREcutsLP;
	PREcutsLP = new IloConstraintArray(*PREenv);
}

/*******************************************************************************************************************************
Name: PRERemoveCuts
***************************************************************************************************************************************************
Description: Removes the cuts stored in the object PREcuts from the MIP model
***************************************************************************************************************************************************
Input: Nothing
Necessitates: The MIP must be initialized.
Output: Nothing
leads to: The MIP model doesn't have the cuts stored in the object PREcutsMIP
**************************************************************************************************************************************************/
void Preprocessing::PRERemoveCutsFromMIP()
{
	if (bPREisDebug) printf("Number of removed cuts : %d\n",PREcutsMIP->getSize());
	if (PREcutsMIP->getSize()>0)
		PREmip->LPRemoveCuts(PREcutsMIP);
	delete PREcutsMIP;
	PREcutsMIP = new IloConstraintArray(*PREenv);
}

/*******************************************************************************************************************************
Name: PREAddLPCut
***************************************************************************************************************************************************
Description: Adds a constraint to the object PREcutsLP
***************************************************************************************************************************************************
Input: The constraint to be added
Necessitates: Nothing
Output: Nothing
leads to: The constraint is added to the object PREcutsLP
**************************************************************************************************************************************************/
void Preprocessing::PREAddLPCut(IloConstraint constraint) 
{
	PREcutsLP->add(constraint);
}

/*******************************************************************************************************************************
Name: PREAddLPCuts
***************************************************************************************************************************************************
Description: Adds an array of constraint to the object PREcutsLP
***************************************************************************************************************************************************
Input: The array of constraint to be added
Necessitates: Nothing
Output: Nothing
leads to: The array of constraint is added to the object PREcutsLP
**************************************************************************************************************************************************/
void Preprocessing::PREAddLPCuts(IloConstraintArray* constraints) 
{
	cout << "Adding cuts" << endl;
	for(int i=0; i<constraints->getSize(); i++) 
		PREcutsLP->add((*constraints)[i]);

	cout << "Number of total cuts " << PREcutsLP->getSize() << endl;
}

/*******************************************************************************************************************************
Name: PREClearLPCuts
***************************************************************************************************************************************************
Description: Reinitialize the array of cuts for the LP
***************************************************************************************************************************************************
Input: Nothing
Necessitates: PREenv must be set
Output: Nothing
leads to: The array PREcutsLP is empty
**************************************************************************************************************************************************/
void Preprocessing::PREClearLPCuts() 
{
	PREcutsLP=new IloConstraintArray(*PREenv);
}

/*******************************************************************************************************************************
Name: PREAddMIPCut
***************************************************************************************************************************************************
Description: Adds a constraint to the object PREcutsMIP
***************************************************************************************************************************************************
Input: The constraint to be added
Necessitates: Nothing
Output: Nothing
leads to: The constraint is added to the object PREcutsMIP
**************************************************************************************************************************************************/
void Preprocessing::PREAddMIPCut(IloConstraint constraint) 
{
	PREcutsMIP->add(constraint);
}

/*******************************************************************************************************************************
Name: PREAddMIPCuts
***************************************************************************************************************************************************
Description: Adds an array of constraint to the object PREcutsMIP
***************************************************************************************************************************************************
Input: The array of constraint to be added
Necessitates: Nothing
Output: Nothing
leads to: The array of constraint is added to the object PREcutsMIP
**************************************************************************************************************************************************/
void Preprocessing::PREAddMIPCuts(IloConstraintArray* constraints) 
{
	for(int i=0; i<constraints->getSize(); i++) 
		PREcutsMIP->add((*constraints)[i]);
}

/*******************************************************************************************************************************
Name: PREClearMIPCuts
***************************************************************************************************************************************************
Description: Reinitialize the array of cuts for the MIP
***************************************************************************************************************************************************
Input: Nothing
Necessitates: PREenv must be set
Output: Nothing
leads to: The array PREcutsMIP is empty
**************************************************************************************************************************************************/
void Preprocessing::PREClearMIPCuts() 
{
	PREcutsMIP=new IloConstraintArray(*PREenv);
}

/*******************************************************************************************************************************
Name: PREGenAllRedCostCut
***************************************************************************************************************************************************
Description: Generate all the possible cuts using the reduced costs. For all i j if ri+rj>UB-LB then we add the cut xi+xj<=1.
***************************************************************************************************************************************************
Input: Nothing
Necessitates: The LP must be initialised and solved
Output: Nothing
leads to: The object PREcuts as all the generated cuts
**************************************************************************************************************************************************/
void Preprocessing::PREGenAllRedCostCut()
{
	if(!PRElp->LPisSolved())
		throw(1);	// The LP has not been solved
	int ri, rj;
	for(int i=0; i<PREvar->getSize(); i++) {
		if(pdPREFixedVariables[i] != IloInfinity) {
			ri = PREvarInfo[i].VARGetRedCost();
			if(ri<=dPREUB-dPRELB && PREvarInfo[i].VARGetBasisStatus()==0) {
				for(int j=i+1; j<PREvar->getSize(); j++) {
					if(i != j && pdPREFixedVariables[j] != IloInfinity) {
						rj = PREvarInfo[j].VARGetRedCost();
						if(rj<=dPREUB-dPRELB && PREvarInfo[j].VARGetBasisStatus()==0)
							if(ri+rj>dPREUB-dPRELB) {
								IloExpr expr(*PREenv);
								expr += (*PREvar)[i];
								expr += (*PREvar)[j];
								PREAddLPCut(expr<=1);
								expr.end();
							}
					}
				}
			}
		}
	}
}
