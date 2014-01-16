#include "Preprocessing.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <process.h>
#include <malloc.h>

#define EPSILON 0.0001

ILOSTLBEGIN

/*******************************************************************************************************************************
Name: Preprocessing
********************************************************************************************************************************
Discription: Constructor
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
Discription: Initialize a LP problem
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
  iPREnbVar=pvar->getSize();
  iPREVarEnd=iPREnbVar;
  pdPREFixedVariables=new double[pvar->getSize()];
  PREvarInfo=new Variable[pvar->getSize()];
  for(int i=0;i<pvar->getSize();i++)
	pdPREFixedVariables[i]=IloInfinity;
  bPREisSet=true;
  if(bPREisDebug) cout<<"Succes to initialize a LP problem"<<endl;
}

/*******************************************************************************************************************************
Name: PREInitializeMIP
********************************************************************************************************************************
Discription: Initialize a MIP problem
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
  PREmip->MIPInitializeCplexParameters(0,750);
  iPREnbVar=pvar->getSize();
  bPREisSet=true;
  if(bPREisDebug) cout<<"Succes to initialize a MIP problem"<<endl;
}

/*******************************************************************************************************************************
Name: PREInitializeMIPfromLP
********************************************************************************************************************************
Discription: Initialize a MIP from a existe LP
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
	  if(PREToFix((*pvar)[i].getId(),head))
	  {
		  //printf("change the type of variable: %s\n",(*pvar)[i].getName());
		  pmodel->add(IloConversion(*penv,(*pvar)[i],ILOBOOL));
	  }
  PREmip->MIPInitialize(penv,pcplex,pmodel,pvar,pcon);
  PREmip->MIPInitializeCplexParameters(0,750);
  bPREisSet=true;
  if(bPREisDebug) cout<<"Succes to initialize a MIP problem"<<endl;
}

/*******************************************************************************************************************************
Name: PREArSup
********************************************************************************************************************************
Discription: Round up a real value.
********************************************************************************************************************************
Input: A value of double
Necessitates: nil
Output: A value of double
leads to: It Returns the smallest integral value that is not less than the value given.
*******************************************************************************************************************************/
double Preprocessing::PREArSup(double value)
{
 return (ceil(value-0.001)); // ???
}


/*******************************************************************************************************************************
Name: PREFixVarToLP
********************************************************************************************************************************
Discription: Fix the value of variables to the LP model
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
		if(pdPREFixedVariables[i]!=IloInfinity)
		{
			PRElp->LPAddFix(i,pdPREFixedVariables[i]);
			if(bPREisDebug) cout<<"I have fixed the var "<<(*PREvar)[i].getName()<<"to "<<pdPREFixedVariables[i]<<" !"<<endl;
			iPREnbFix++;
		}
	}

}

/*******************************************************************************************************************************
Name: PREFixVarToMIP
********************************************************************************************************************************
Discription: Fix the value of variables to the MIP model
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
		if(pdPREFixedVariables[i]!=IloInfinity)
		{
			PREmip->MIPAddFix(i,pdPREFixedVariables[i]);
			cout<<"I have fixed the var "<<i<<"to "<<pdPREFixedVariables[i]<<" !"<<endl;
			iPREnbFix++;
		}
	}
}

/*******************************************************************************************************************************
Name: PRESolveLP
********************************************************************************************************************************
Discription: Solve a LP problem by Cplex 
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
 } else
 {
	 //cout << "ERROR: the LP model has not been intialized and thus cannot be solved\n";
	 throw(5);
 }
}

/*******************************************************************************************************************************
Name: PRESolveMIP
********************************************************************************************************************************
Discription: Solve a MIP problem by Cplex 
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
	}else
	{
		 //cout << "ERROR: the MIP model has not been intialized and thus cannot be solved\n";		
		 throw(6);
	}
}

		

/*******************************************************************************************************************************
Name: PREGetMIPResVar
********************************************************************************************************************************
Discription: Get the values of the varialbes after the MIP problem solved by Cplex
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
Discription: This method implements the preprocessing algorithm
********************************************************************************************************************************
Input: head is a indice array for the bool variables
	   nb is the number of the bool variables the to fix
Necessitates: 0 <= iVarBeg <= iVarEnd < Number of variables
			  The variables between iVarBeg and iVarEnd must be boolean variables
Output: Nothing
leads to: the variables in head have been preprocessed and potentially fixed in the LP model 
          to fix them in the MIP model, use method PREfixVarToMIP()
Return: true if the programm did the preprocessing, false if the LB=UB (i.e. shrage is the optimal solution)
*******************************************************************************************************************************/
bool Preprocessing::PREPreprocessing(int * head,int nb)
{
	if(bPREisDebug) cout<<"I start fixing the variables"<<endl;
	double dOpt;
	bool bLbisUb=false;
	dPRELB=dPRElpOpt;
	iPREnbFix=0;
	try
	{
		do
		{
			if(dPRELB-dPREUB>-0.0001) 
				bLbisUb=true;
			else {
				dOpt=PREArSup(dPRElpOpt);		// Rounding up the optimal solution of the LP
				PREFixVar();					// Preprocessing on the variables
				if(bPREisDebug) cout<<"Calculate finish!"<<endl;
				//PREFixVarToLP();	// Attention !!!
				PRESolveLP();
				if(bPREisDebug) 
				{
					printf("Opt lp: %lf, dOpt: %lf, UB: %lf\n",dPRElpOpt,dOpt, dPREUB);
					getchar();
				}
			}
		}
		while(!bLbisUb && dOpt!=PREArSup(dPRElpOpt)); // It repeats until there is no more variable that can be fixed
		bPREisSolved=true;
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
	return !bLbisUb;
}

/*******************************************************************************************************************************
Name: PREToFix
********************************************************************************************************************************
Discription: This method is to detect if a variable should be check and fixed.
********************************************************************************************************************************
Input: indice is the indice of a variable to be detected
       head is a indice array for the bool variables
Necessitates: nil
Output: true if the variable is in the array to be fixed
        false if not
leads to: nil
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
				//printf("the variable %s is to be fixed\n",(*PREvar)[indice].getName());
				return true;
			}
			else if (head[i]==-1) return false;
		}
	}
}

/*******************************************************************************************************************************
Name: PREFixVar
***************************************************************************************************************************************************
Discription: Fix the variables off-base by comparing the reduced cost with the difference between UB and LB
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
	if(bPREisDebug) cout<<"Fix the variables using the class <Variabl>"<<endl;
	int i;			// This double array is to save the reduced cost and return by the function PREGetReducedCost
	PREGetRedCost();	// Get the reduced cost from the LP et set the variables from the variable class with the values
	PREGetPseuCost();	// Get the pseudo cost from the LP et set the variables from the variable class with the values
	PREGetVarValue();	// Get the values of the variable from the LP et set the variable from the variable class with the values
	if(bPREisDebug)
		for(i=0;i<PREvar->getSize();i++)
			PREvarInfo[i].VAROutput();

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
		  if(PREToFix((*PREvar)[i].getId(),ipPREhead))
		  {
			  if(PREvarInfo[i].VARGetBasisStatus()==0)	// Off-base variable
			  {
				if(PREvarInfo[i].VARGetRedCost()>0 //If r_ij>UB-LB, it will fix the variable i to 0
					&& PREvarInfo[i].VARGetRedCost()>dPREUB-dPRELB+EPSILON)	
				{
					pdPREFixedVariables[i] = 0;
					(*PREvar)[i].setUB(0.0);
					iPREnbFix++;
					if(bPREisDebug)
					{
						//printf("Reduced cost for %s is: %.2lf\n",(*PREvar)[i].getName(),RedCost);
					}
				}
				if(PREvarInfo[i].VARGetRedCost()<0 
					&& PREvarInfo[i].VARGetRedCost()<dPRELB-dPREUB-EPSILON)	//If r_ij>UB-LB, it will fix the variable i to 1
				{
					pdPREFixedVariables[i] = 1;
					(*PREvar)[i].setLB(1.0);
					iPREnbFix++;
					if(bPREisDebug)
					{
						//printf("Reduced cost for %s is: %.2lf\n",(*PREvar)[i].getName(),RedCost);
					}
				}
			  }
			  else if(PREvarInfo[i].VARGetBasisStatus()==1)	// In-base variables
			  {
				  if (PREvarInfo[i].VARGetUj()<99999999.0 // I try to fix variables to 0
					  && (1.0-PREvarInfo[i].VARGetValue())*PREvarInfo[i].VARGetUj()>dPREUB-dPRELB+EPSILON)   
					{ 
						pdPREFixedVariables[i] = 0;
						(*PREvar)[i].setLB(0.0);
						iPREnbFix++;
						
						if(bPREisDebug)
						{
							printf("Uj for %s is %.2lf\n",(*PREvar)[i].getName(),PREvarInfo[i].VARGetUj());
							cout<<"Fix "<< (*PREvar)[i].getName()<<" to 0 by pseudo-cost"<<endl;
						}
					}
					if (PREvarInfo[i].VARGetLj()<99999999.0 // I try to fix variables to 1 PROBLEME ICI!!!
						&& PREvarInfo[i].VARGetValue()*PREvarInfo[i].VARGetLj()>dPREUB-dPRELB+EPSILON)   
					{ 
						pdPREFixedVariables[i] = 1;
						(*PREvar)[i].setLB(1);
						iPREnbFix++;
						
						if(bPREisDebug)
						{
							printf("Lj for %s is %.2lf\n",(*PREvar)[i].getName(),PREvarInfo[i].VARGetLj());
							cout<<"Fix "<<(*PREvar)[i].getName()<<" to 1 by pseudo-cost"<<endl;
						}
					}
				}	
			}
		}
	}
}
