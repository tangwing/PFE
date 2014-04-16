#include "LinearProgram.h"
#include <process.h>



/*******************************************************************************************************************************
Name: LinearProgram
********************************************************************************************************************************
Discription: Default constructor: before using the model, the method LPInitialize() must be called
********************************************************************************************************************************
Input: Nothing
Necessitates: nil
Output: Nothing
leads to: Initialize some attributes 
*******************************************************************************************************************************/
LinearProgram::LinearProgram(void)
{
  bLPisSolved=false;
  bLPisSet=false;
  dLPOptimalValue=IloInfinity;
  pdLPVariables=NULL;
  iLPnbBase=0;
}



/*******************************************************************************************************************************
Name: LPInitialize
********************************************************************************************************************************
Discription: Initialize the LP problem
********************************************************************************************************************************
Input: The Envirenment, the variables, the model, the cplex solver and the constraints of a LP problem
Necessitates: All the four input arguments are well allocated
Output: Nothing
leads to: Obtain the four arguments necessite to create a LP model and set bLPisSet to true.                           
          It also initialize some attributes of LinearProgram.
*******************************************************************************************************************************/
void LinearProgram::LPInitialize(IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar,IloRangeArray *pcon)
{
	printf("Initialize a LP problem\n");
	LPenv=penv;
	LPcplex=pcplex;
	LPmodel=pmodel;
	LPvar=pvar;
	LPcon=pcon;
	
	iLPnbBaseConcert=0;
	pdLPVariables=new double[LPvar->getSize()];
	pLPVarBase=new IloNumVarArray(*penv);
	pLPlj=new IloNumArray(*penv);
	pLPuj=new IloNumArray(*penv);

	//LPcplex->exportModel("test.lp");

	bLPisSet=true;
}

/*******************************************************************************************************************************
Name: LPSolveByCplex
********************************************************************************************************************************
Discription: Solve a LP problem by Cplex 
********************************************************************************************************************************
Input: Nothing
Necessitates: LP problem should be well initialized
Output: Nothing
leads to: It use cplex to solve a LP problem and get the optimal solution and set bLPisSolved to true
*******************************************************************************************************************************/
void LinearProgram::LPSolveByCplex()
{
	if(!bLPisSet)
	{
		//cout<<"The LP cannot be solved as it has not been initialized"<<endl;
		throw(5);
	}

	//LPcplex->exportModel("model.lp");
   if (!(LPcplex->solve()))							// The LP problem can't solve by cplex
   {
	   if (LPcplex->getCplexStatus()!=IloCplex::InfeasibleOrUnbounded	// The LP problem isn't infeasible or unbounded
		   && LPcplex->getCplexStatus()!=IloCplex::Infeasible			// The LP problem is feasible
		   && LPcplex->getCplexStatus()!=IloCplex::InfOrUnbd)			// The LP problem is bounded
	   {
		   cout<<"Failed to optimise model" <<endl;
		   throw(7);
	   } else 
	   {
		   cout<<"Failed to optimise model: it is either infeasible or unbounded" <<endl;
		   throw(8);
	   }
   }
  
  dLPOptimalValue = LPcplex->getObjValue();
  //printf("Result of LP: %.2lf\n", dLPOptimalValue);
  /*for(int i=0;i<LPvar->getSize();i++)
	pdLPVariables[i] = LPcplex->getValue((*LPvar)[i]);*/

  bLPisSolved=true;
}

/*******************************************************************************************************************************
Name: LPGetReducedCost
********************************************************************************************************************************
Discription: Get the reduced cost 
********************************************************************************************************************************
Input: A double array to save the reduced cost
Necessitates: LP problem must be solved by cplex and the input double array is well allocated
Output: Nothing
leads to: Get the reduced cost of each variables by calling the function in Cplex after LP problem is solved
*******************************************************************************************************************************/
void LinearProgram::LPGetReducedCost(double *pdReducedCost)
{ // This function returns the reduced costs
  // Necessitate: the parameter pdReducedCost has been allocated as an array of size sizeof(double)*Number of variables
  int i;

	if(!bLPisSolved) 
	{
		//cout<<"ERROR: the reduced costs cannot be returned as the problem as not been solved"<<endl;
		throw(1);
	}
	for (i=0;i<LPvar->getSize();i++)
	{
		pdReducedCost[i]=LPcplex->getReducedCost((*LPvar)[i]);				// Get the reduced cost by calling the function if Cplex
	}

}


void LinearProgram::LPGetVarValue(double *var)
{ // This function returns the variable values in the optimal solutions
  // Necessitate: the parameter var has been allocated as an array of size sizeof(double)*Number of variables
  int i;

	if(!bLPisSolved) 
	{
		//cout<<"ERROR: the reduced costs cannot be returned as the problem as not been solved"<<endl;
		throw(1);
	}
	for (i=0;i<LPvar->getSize();i++)
	{
		var[i]=LPcplex->getValue((*LPvar)[i]);				// Get the variable values in the optimal solutions
	}

}

/*******************************************************************************************************************************
Name: LPGetBasisStatus
********************************************************************************************************************************
Discription: Get the basis status
********************************************************************************************************************************
Input: nothing
Necessitates: LP problem must be solved by cplex
Output: A double array to save the basis status
leads to: Get the basis status of each variables by calling the function in Cplex after LP problem is solved
*******************************************************************************************************************************/
int* LinearProgram::LPGetBasisStatus()
{
	if(!bLPisSolved) 
	{
		//cout<<"ERROR: the basis status cannot be returned as the problem as not been solved"<<endl;
		throw(1);
	}
	int *pdBasisStatuts;											
	pdBasisStatuts=(int *)malloc(sizeof(double)*LPvar->getSize());
	for (int i=0;i<LPvar->getSize();i++)
		pdBasisStatuts[i]=LPcplex->getBasisStatus((*LPvar)[i]);			// This function gathering the basis status is to estimate whether a variable is in base or not
	return pdBasisStatuts;
}

/*******************************************************************************************************************************
Name: LPGetPseudoCost
********************************************************************************************************************************
Discription: Get the pseudo cost
********************************************************************************************************************************
Input: nothing
Necessitates: LP problem must be solved by cplex
Output: nothing
leads to: Get the pseudo cost of each variables in-base by calling the function in concert after LP problem is solved
*******************************************************************************************************************************/
void LinearProgram::LPGetPseudoCost()
{
 unsigned int i;
 pLPVarBase->clear();
 //IloCplex::BasisStatusArray cstat(*LPenv);
 if(!bLPisSolved) 
	{
		//cout<<"ERROR: the basis status cannot be returned as the problem as not been solved"<<endl;
		throw(1);
	} 

 piLPIndiceBaseConcert=new int[LPvar->getSize()];
 for (i=0;i < LPvar->getSize();i++) piLPIndiceBaseConcert[i]=-1;
  for (i=0;i < LPvar->getSize();i++)
	if (LPcplex->getBasisStatus((*LPvar)[i])==1)
	{
		piLPIndiceBaseConcert[iLPnbBaseConcert]=i;
		pLPVarBase->add((*LPvar)[i]);							 // I put the basic variables into VarBases
		iLPnbBaseConcert++;
	}
  LPcplex->getDriebeekPenalties(*pLPlj,*pLPuj,*pLPVarBase);	 // I get the Driebeek's pseudo costs (the equivalent function under CPX callable lib is CPXmdleave)
  
	 /********************************A SUPPRIMER***************************/
    /*FILE * fichier;
	fichier=fopen("LPPseudo.txt","wt");
	for (i = 0; i<pLPVarBase->getSize(); i++)
		{
			fprintf(fichier,"%s\t%.2lf\t%.2lf\n",(*pLPVarBase)[i].getName(),(*pLPlj)[i],(*pLPuj)[i]);
			
		}
	fclose(fichier);*/
	/**********************************************************************/
	//LPCalAij();
	//LPCalculateTomlin();
}

/*******************************************************************************************************************************
Name: LPAddFix
********************************************************************************************************************************
Discription: Fix the value of variables to the LP model
********************************************************************************************************************************
Input: iIndex (the index of the variable will be fixed) and iValue the value to fix
Necessitates: nil
Output: Nothing
leads to: It will add the same UB and LB(0 or 1) to a variable in the LP model
*******************************************************************************************************************************/
void LinearProgram::LPAddFix(int iIndex, int iValue)
{
	if((*LPvar)[iIndex].getLB()!=(*LPvar)[iIndex].getUB())
	{
	 (*LPvar)[iIndex].setLB(iValue);								//Fix the (iIndex)th variable to iValue by change its upper and lower bounds
	 (*LPvar)[iIndex].setUB(iValue);
	}
	
}

/*******************************************************************************************************************************
Name: LPArSup
********************************************************************************************************************************
Discription: Round up a real value.
********************************************************************************************************************************
Input: A value of double
Necessitates: nil
Output: A value of double
leads to: It Returns the smallest integral value that is not less than the value given.
*******************************************************************************************************************************/
double LinearProgram::LPArSup(double value)
{
 return (ceil(value-0.001));
}

/*******************************************************************************************************************************
Name: LPCalAij
********************************************************************************************************************************
Discription: Get the coeffients to calculate the pseudo cost based on the Tomlin's panalties
********************************************************************************************************************************
Input: none
Necessitates: nil
Output: a double array of 2 dimension
leads to: It Returns the coeffients calculated by Cplex.
*******************************************************************************************************************************/

void LinearProgram::LPCalAij()
{
	if(!bLPisSolved) 
	{
		cout<<"ERROR: the basis status cannot be returned as the problem as not been solved"<<endl;
		throw(1);
	}
	LPcplex->exportModel("model.lp");
	int status,i;
	CPXENVptr     cpxenv = NULL;
	CPXLPptr      cpxlp = NULL;

	//cout<<aji<<endl;

	cpxenv = CPXopenCPLEX (&status);
	if ( cpxenv == NULL ) {
      //char  errmsg[1024];
      fprintf (stderr, "Could not open CPLEX environment.\n");
   }
   cpxlp = CPXcreateprob (cpxenv, &status, "diet");
   status = CPXreadcopyprob (cpxenv, cpxlp, "model.lp", NULL);
   status=CPXlpopt (cpxenv, cpxlp);

   iLPnbRows = CPXgetnumrows(cpxenv,  cpxlp);
   iLPnbCols = CPXgetnumcols(cpxenv,  cpxlp) ; 
   pdLPAij=(double **)malloc(sizeof(double *)*iLPnbRows);
   for (i=0;i<iLPnbRows;i++) pdLPAij[i]=(double *)malloc(sizeof(double)*iLPnbCols);
   piLPVarLibTom=(int *)malloc(sizeof(int)*iLPnbRows);
   pdLPVarValTom=(double *)malloc(sizeof(double)*iLPnbRows);

   status = CPXgetbhead (cpxenv, cpxlp, piLPVarLibTom, pdLPVarValTom);

   for (i = 0; i < iLPnbRows; i++)
		{
			CPXbinvarow(cpxenv, cpxlp, i, pdLPAij[i]);
		}
  
  
   status = CPXfreeprob (cpxenv, &cpxlp);
   status = CPXcloseCPLEX (&cpxenv);

    /******************************** TEST *********************************/
 //   FILE * fichier;
	//fichier=fopen("LPAji.txt","wt");
	//for (j = 0; j < iLPnbCols; j++)
	//	{
	//		//cout<<"Xj "<<j<<":"<<endl;
	//		fprintf(fichier,"Xj %d: ",j);
	//		for(i=0;i<iLPnbRows;i++)
	//			fprintf(fichier,"aji %d, %d: %lf\t",j,i,pdLPAij[i][j]);
	//			//cout<<"aji "<<j<<" , "<<i<<": "<<pdLPAij[i][j]<<endl;
	//		fprintf(fichier, "\n");
	//	}
	//fclose(fichier);
	/**********************************************************************/
}
/*******************************************************************************************************************************
Name: LPCalculateTomlin
********************************************************************************************************************************
Discription: Get the pseudo cost based on the Tomlin's penalties
********************************************************************************************************************************
Input: nothing
Necessitates: LP problem must be solved by cplex
Output: nothing
leads to: Get the pseudo cost of each variables in-base based on the Tomlin's penalties.
*******************************************************************************************************************************/
void LinearProgram::LPCalculateTomlin()
{	
	int i,j,lrows=iLPnbRows;
	double minra,qj;
	if (lrows>LPvar->getSize()) lrows=LPvar->getSize();
	pLPTomlinLj=(double *)malloc(sizeof(double)*iLPnbCols);
	pLPTomlinUj=(double *)malloc(sizeof(double)*iLPnbCols);

	//printf("Tomlin penalties\n");
	//printf("Basic Variable | Lj        | Uj\n");

	for (i=1;i<lrows;i++)
	{
		//printf(" %15ld |",piLPVarLibTom[i]-2,pdLPVarValTom[i]);
		// We now compute the lower Tomlin penalty
		minra=999999999.0;
		for (j=1;j<iLPnbCols;j++) 
			if (pdLPAij[i][j]>0)
			{ 
				/*qj=((-1.0*pdLPAij[0][j])/pdLPAij[i][j])*pdLPVarValTom[i];
				if (qj<(-1.0*pdLPAij[0][j])) qj=(-1.0*pdLPAij[0][j]);*/
				qj=-pdLPAij[0][j];
				if (qj<minra) minra=qj;
			}
	 pLPTomlinLj[i]=minra;
	 //printf("%3.3lf ",minra);
	 // We now compute the upper Tomlin penalty
	 minra=999999999.0;
	 for (j=1;j<iLPnbCols;j++) 
	 	 if (pdLPAij[i][j]<0)
		 { 
			 /*qj=((-1.0*pdLPAij[0][j])/(-1.0*pdLPAij[i][j]))*(1.0-pdLPVarValTom[i]);
			 if (qj<(-1.0*pdLPAij[0][j])) qj=(-1.0*pdLPAij[0][j]);*/
			 qj=-pdLPAij[0][j];
			 if (qj<minra) minra=qj;
		 }
	 pLPTomlinUj[i]=minra;
	 //printf("%3.3lf  \n",minra);
	}
	/*FILE * file;
	file=fopen("LPPseudoTomlin.txt","wt");
	fprintf(file, "VarName\tLj\tUj\t");
	for (i = 0; i<iLPnbBase; i++)
		{
			fprintf(file,"%s\t%.2lf\t%.2lf\n",(*LPvar)[i].getName(),pLPTomlinLj[i],pLPTomlinUj[i]);
			
		}
	fclose(file);*/

}

/*******************************************************************************************************************************
Name: LPGetPseudoCostByCPX
********************************************************************************************************************************
Discription: Get the pseudo cost using the function in DLL
********************************************************************************************************************************
Input: nothing
Necessitates: LP problem must be solved by cplex
Output: nothing
leads to: Get the pseudo cost of each variables in-base by calling the function in DLL of Cplex after LP problem is solved
*******************************************************************************************************************************/
void LinearProgram::LPGetPseudoCostByCPX()
{
	if(!bLPisSolved) 
	{
		cout<<"ERROR: the basis status cannot be returned as the problem as not been solved"<<endl;
		throw(1);
	}
	unsigned int i;
	
 
	
	LPcplex->exportModel("model.lp");
	int status,j;
	CPXENVptr     cpxenv = NULL;
	CPXLPptr      cpxlp = NULL;

	cpxenv = CPXopenCPLEX (&status);
	if ( cpxenv == NULL ) 
	{
      char  errmsg[1024];
      fprintf (stderr, "Could not open CPLEX environment.\n");
   }
   cpxlp = CPXcreateprob (cpxenv, &status, "Preprocessing");
   status = CPXreadcopyprob (cpxenv, cpxlp, "model.lp", NULL);
   status=CPXlpopt (cpxenv, cpxlp);
   
   iLPnbRows = CPXgetnumrows(cpxenv,  cpxlp);
   iLPnbCols = CPXgetnumcols(cpxenv,  cpxlp) ; 

   int      solstat;
   double   objval;
   double   *x=new double[iLPnbCols];
   double   *pi=new double[iLPnbRows];
   double   *slack=new double[iLPnbRows];
   double   *dj=new double[iLPnbCols];
   int *cstat=new int[iLPnbCols];
   int *rstat=new int[iLPnbRows];
   /*status = CPXsolution (cpxenv, cpxlp, &solstat, &objval, x, pi, slack, dj);
   if ( status ) {
      fprintf (stderr, "Failed to obtain solution.\n");
      
   }

   printf ("\nSolution status = %d\n", solstat);
   printf ("Solution value  = %f\n", objval);
   printf ("Iteration count = %d\n\n", CPXgetitcnt (cpxenv, cpxlp));*/

   
   
   //piLPVarLibTom=(int *)malloc(sizeof(int)*iLPnbRows);
   //pdLPVarValTom=(double *)malloc(sizeof(double)*iLPnbRows);
   status=CPXgetbase(cpxenv,cpxlp,cstat,rstat);
   //printf("number of variables in CPX: %d\n",iLPnbCols);
   /*for(i=0;i<iLPnbCols;i++)
   {
	   printf("CPX cstat %d: %d\n",i,cstat[i]);
   }*/
   //printf("GETBASE: %d\n",status);
   piLPIndiceBase=new int[iLPnbCols];
   int cnt=0;
   for (i=0;i < iLPnbCols;i++)
	if (cstat[i]==1||cstat[i]==4)
	{
		piLPIndiceBase[cnt]=i;							 // I put the basic variables into VarBases
		cnt++;

	}
	iLPnbBase=cnt;
	printf("CNT: %d\n", cnt);
	pdLPljCPX=new double[cnt];
	pdLPujCPX=new double[cnt];
	int cpxdb=CPXmdleave(cpxenv, cpxlp, piLPIndiceBase, cnt, pdLPljCPX, pdLPujCPX);
	
	delete [] x,pi,slack,dj,cstat,rstat;
}

void LinearProgram::LPGetRedCost(Variable *var)
{
	int i;

	if(!bLPisSolved) 
	{
		//cout<<"ERROR: the reduced costs cannot be returned as the problem as not been solved"<<endl;
		throw(1);
	}
	for (i=0;i<LPvar->getSize();i++)
	{
		var[i].VARSetRedCost(LPcplex->getReducedCost((*LPvar)[i])); // Get the reduced cost by calling the function if Cplex
	}
};	// Get the reduced cost 

void LinearProgram::LPGetPseuCost(Variable *var)
{
 unsigned int i;
 pLPVarBase->clear();
 //IloCplex::BasisStatusArray cstat(*LPenv);
 if(!bLPisSolved) 
	{
		//cout<<"ERROR: the basis status cannot be returned as the problem as not been solved"<<endl;
		throw(1);
	} 

  for (i=0;i < LPvar->getSize();i++)
	if (LPcplex->getBasisStatus((*LPvar)[i])==1)
	{
		var[i].VARSetBasisStatus(1);
		pLPVarBase->add((*LPvar)[i]);							 // I put the basic variables into VarBases
		iLPnbBaseConcert++;
	}
	else if(LPcplex->getBasisStatus((*LPvar)[i])==0)
	{
		var[i].VARSetBasisStatus(0);
	}
  LPcplex->getDriebeekPenalties(*pLPlj,*pLPuj,*pLPVarBase);	 // I get the Driebeek's pseudo costs (the equivalent function under CPX callable lib is CPXmdleave)
  int j=0;
  for(i=0;i<LPvar->getSize();i++)
  {
	  if(var[i].VARGetBasisStatus()==1)
	  {
		  var[i].VARSetPseuCost((*pLPlj)[j],(*pLPuj)[j]);
		  j++;
	  }
  }	
};	// Get the pseudo-cost

void LinearProgram::LPGetVarValue(Variable *var)
{
	int i;

	if(!bLPisSolved) 
	{
		//cout<<"ERROR: the reduced costs cannot be returned as the problem as not been solved"<<endl;
		throw(1);
	}
	for (i=0;i<LPvar->getSize();i++)
	{
		var[i].VARSetValue(LPcplex->getValue((*LPvar)[i]));				// Get the variable values in the optimal solutions
	}
};	// Get the variable value in the optimal solution