/** This is a model for LP */
#ifndef CLP
#define CLP 100

#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>
#include "Variable.h"

ILOSTLBEGIN

/*---------------------------------------------------------------------------
Notice: The class LinearProgram is designed for solving minimization problems
----------------------------------------------------------------------------*/

class LinearProgram
{
private:
	// Attributes related to the Mathematical models
	IloEnv *LPenv;				// A pointer to the LP enviroment for add the new variables to the enviroment
	IloCplex *LPcplex;			// A pointer to the LP cplex that solves the problem
	IloModel *LPmodel;			// A pointer to the LP model to solve by LPcplex
	IloNumVarArray *LPvar;		// A pointer to the real valued variables
	IloRangeArray *LPcon;		// A pointer to the set of contraints of the model
    bool bLPisSet;				// A flag to know if the LP model has been correctly initialized

	// Attributes related to the solution of the model
	bool bLPisSolved;						// Flag to know if the current model has been solved or not
	double dLPOptimalValue;					// The optimal solution value computed by CPLEX
	double *pdLPVariables;					// Values of the variables in the optimal solution computed by CPLEX

	int iLPnbBaseConcert;					// The number of variable in-base obtained by the function in concert
	int *piLPIndiceBaseConcert;				// The indices of the in-base variables obtained by the function in concert
	IloNumVarArray *pLPVarBase;				// The array to store the in-base variables
	IloNumArray *pLPlj,*pLPuj;				// The pseudo-costs computed by Cplex, based on Driebeeck's penalties
	
	int iLPnbBase;							// Number of variables in base
	int *piLPIndiceBase;					// The array for to stock the indice of the in-base variables

	// Attributes related to the Tomlin's penalties
	///! Tomlin related parts are not reviewed, there may be mem leaks
	double *pLPTomlinLj,*pLPTomlinUj;		// The pseudo-costs computed by Cplex, based on Tomlin's penalties
	double **pdLPAij;						// The coeffients to calculate the pseudo cost based on Tomlin's penalties.
	int *piLPVarLibTom;						// An array. The array contains the indices of the variables in the resident basis.
	double *pdLPVarValTom;					// An array. This array contains the values of the basic variables in the order specified by piLPVarLibTom.
	int iLPnbCols;							// Number of columns in the LP model.
	int iLPnbRows;							// Number of rows in the LP model.

	// Attributes related to the Driebeeck's penalties calculated by CPXmdleave
	double *pdLPljCPX, *pdLPujCPX;


public:
	// Methods to manipulate the LP model

	// Methods related to the initialization of the LP model
	LinearProgram(void);				// Default constructor: before using the model, the method LPInitialize() must be called
	void LPInitialize(IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar, IloRangeArray *pcon);
										// This method must be used to initialize the model
	void LPAddCuts(IloConstraintArray*);	// Adds the constraints to the model
	void LPRemoveCuts(IloConstraintArray*);	// Removes the constraints from the model
	~LinearProgram(void)
	{
		delete pLPlj; delete pLPuj; delete pLPVarBase; delete [] pdLPVariables; /*for (int i=0;i<iLPnbRows;i++) free(pdLPAij[i]); free(pdLPAij);*/
		///!Add some "delete"
		if(NULL!=piLPIndiceBaseConcert)delete piLPIndiceBaseConcert;
		if(NULL!=piLPIndiceBase)delete piLPIndiceBase;
		if(NULL!=pdLPljCPX)delete pdLPljCPX;
		if(NULL!=pdLPujCPX)delete pdLPujCPX;
	} // Destructor of the class

	 
	void LPSolveByCplex();				// Solve the LP problem using cplex and return the value of the funtion objective
										// And get the result of each variable after the problem solved by cplex
	void LPGetReducedCost(double *);	// Get the reduced cost of each variable off base after the problem solved by cplex
								  	    // NECESSITATE: the parameter pdReducedCost has been allocated as an array of size sizeof(double)*Number of variables

	int* LPGetBasisStatus(void);		// Get the basic information to decide whether a variable is in base or not
	void LPGetPseudoCost();				// Get the Pseudo cost of each variable in base using Driebeeck's penalties
	void LPCalculateTomlin();			// Get the Pseudo cost of each variable in base using Tomlin's penalties
	IloNumArray* LPGetlj(){return pLPlj;}	// Get pLPlj to fix the variable in base
	IloNumArray* LPGetuj(){return pLPuj;}	// Get pLPuj to fix the variable in base
	IloNumVarArray* LPGetVarBase(){if(!bLPisSolved) throw(1);else return pLPVarBase;}
	double * LPGetljCPX() {return pdLPljCPX;}		// Get lj from CPX
	double * LPGetujCPX() {return pdLPujCPX;}		// Get uj from CPX
	double* LPGetTomlinLj(){return pLPTomlinLj;}	// Get pLPlj to fix the variable in base
	double* LPGetTomlinUj(){return pLPTomlinUj;}	// Get pLPuj to fix the variable in base
	void LPAddFix(int iIndex, int iValue);	// To fix the (iIndex)th variable to iValue
	bool LPisSet(){return bLPisSet;}		// True if the model is set, false otherwise
	bool LPisSolved(){return bLPisSolved;}	// True if the model has been solved, false otherwise
	double LPGetOptValue(){return dLPOptimalValue;}	// Getter for the optimal value
	double* LPGetvariables(){return pdLPVariables;}	// Getter for the optimal values of the variables
	void LPGetVarValue(double *var);	// Get the variable values in the optimal solution
	double LPArSup(double value);
	double** LPGetAij(){return pdLPAij;}	
	void LPGetPseudoCostByCPX();
	int LPGetNbBase(){return iLPnbBaseConcert;}
	int * LPGetIndiceBase(){ return piLPIndiceBase;}
	double LPGetVarValue(IloNumVar var){if(!bLPisSolved) throw(1);else return LPcplex->getValue(var);}
	double LPGetReducedCostX(IloNumVar var){if(!bLPisSolved) throw(1);else return LPcplex->getReducedCost(var);}

	void LPGetVarResults(Variable *var);// Set the reduced costs, pseudo-costs, optimal variable values and basic variable status
private: 
	void LPCalAij();					// Get the coeffients to calculate the pseudo cost based on Tomlin's penalties.
};
#endif