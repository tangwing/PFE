#ifndef CPrePro
#define CPrePro 100

#include "LinearProgram.h"
#include "MixedIntegerProgram.h"
#include "Variable.h"
#include <stdio.h>

/*---------------------------------------------------------------------------
Notice: The class Preprocessing is designed for solving minimization problems
----------------------------------------------------------------------------*/

class Preprocessing
{
private:
    // Attributes related to the Mathematical models
	IloEnv *PREenv;					// A pointer to the CPLEX environment: must be initialized outside the class Preprocessing
	IloNumVarArray *PREvar;			// A pointer to the CPLEX variable array
	LinearProgram *PRElp;			// A pointer to the LP model that is solved during the preprocessing phase
	MixedIntegerProgram *PREmip;	// A pointer to the MIP model that can be solved either before or after the preprocessing
	bool bPREisSet;					// A flag to note if the models and preprocessing objects have been correctly initialized
	bool bPREisSolved;				// A flag to note if the preprocessing has been applied or not
	Variable *PREvarInfo;			// A pointer to the variable array

	///!NotUsed
	IloConstraintArray *PREcutsMIP;	// A pointer to the CPLEX cuts to be added to the MIP model 
	IloConstraintArray *PREcutsLP;	// A pointer to the CPLEX cuts to be added to the LP model 

	// Attributes related to the preprocessing phase
	int *ipPREhead;					// A tableau to save the indices of bool variables
	double dPRELB;					// The Lower Bound given by the optimal solution of PRElp
	double dPREUB;					// An Upper Bound computed by an heuristic, outside the class Preprocessing
	bool bPREisUB;					// A flag to note is an Upper Bound has been set by the user or not
	int iPREnbFix;					// Number of variables fixed by the preprocessing phase
    double *pdPREFixedVariables;	// Array that contains the status of variables: IloInfinity=> The corresponding variable is not fixed. -1 => the variable is a boolean and can be fixed
									// Otherwise to value to which 
									// it has been fixed to.
	///!NotUsed
	bool *pbPREisFixed;				// Array of flags. If true, the variables has been fixed by preprocessing.
	
	int iPREVarBeg;					// Index of the first fixed variable
	int iPREVarEnd;					// Index of the last fixed variable
	int iPREnbVar;					// The number of variables
	int iPREnbBool;					// The number of boolean variables
	
	double dPRElpOpt;				// The optimal solution of the LP problem
	double dPREmipOpt;				// The optimal solution of the MIP problem

	bool bPREisDebug;				// If true, it will print the debug information.
	bool bPRETomDrie;				// If true, it calculates the pseudo cost based on the Tomlin's Penalties.
	                                // Else, it calculates the pseudo cost based on the Driebeek's penalties.
	bool bPREhasHead;				// If true, it has a indice table saved the indice of bool variables
	bool bPREOptiNoPRE;				// If true, the optimal solution has been found without preprocessing

public:
	// Methods to manipulate the Mathematical models and the preprocessing phase (USABLE BY THE USERS)

	// Methods related to the initialization of the models and the preprocessing objects
	Preprocessing();			// Default constructor: before using the models, the method PREInitializeLP() and/or PREInitializeMIP() must be called
	~Preprocessing(void) {delete PRElp; delete PREmip;delete [] PREvarInfo; delete [] pdPREFixedVariables;}	// Destructor
	/* This method enables to initialize the LP model */
	void PREInitializeLP(IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar,IloRangeArray *pcon);
	/* This method enables to initialize the MIP model */
	void PREInitializeMIP(IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar, IloRangeArray *pcon);
	void PREInitializeMIPfromLP(IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar,IloRangeArray *pcon,int *head);
	void PRESetDebug(bool debug){bPREisDebug=debug;}	// The user can decide whether he check the debug information or not
	void PRESetTomlin(bool tomlin){bPRETomDrie=tomlin;}
	bool PREIsOptiNoPRE(){return bPREOptiNoPRE;}
	
	// Methods related to the solution of the LP
	void PRESolveLP();				// Solve a LP problem by Cplex
	double PREGetLpOpt()			// Get the optimal solution value
		{if (PRElp->LPisSolved()) return dPRElpOpt; else throw(1);}

	// Methods related to the cuts
	void PREAddCutsToLP();
	void PREAddCutsToMIP();
	void PREClearLPCuts();
	void PREClearMIPCuts();
	void PRERemoveCutsFromLP();
	void PRERemoveCutsFromMIP();
	void PREAddLPCut(IloConstraint);
	void PREAddMIPCut(IloConstraint);
	void PREAddLPCuts(IloConstraintArray*);
	void PREAddMIPCuts(IloConstraintArray*);
	void PREGenAllRedCostCut();

	// Methods related to the solution of the MIP
	void PRESolveMIP();				// Solve a MIP problem by Cplex
	double* PREGetMIPResVar();		// Get the values of the variables after the MIP problem solved by Cplex										
	double PREGetMipOpt()			// Get the optimal solution value
		{if (PREmip->MIPisSolved()) return dPREmipOpt; else throw(2);}
	int PREGetMIPNbNode(){if(PREmip->MIPisSolved()) return PREmip->MIPGetNbNode();else throw(2);}
	bool PREisMIPOpt(){return PREmip->MIPisOpt();}

	// Methods relate to the preprocessing algorithm
	bool PREPreprocessing();
	void PRESetHead(int * head, int length);
	void PRESetUB(double UB)		// Enables to set the UB required by the preprocessing
		{dPREUB=UB;bPREisUB=true;}				
	double PREGetUB()				// Get the best known UB after the preprocessing
		{if (bPREisSolved) return dPREUB; else throw(3);}
	double PREGetLB()				// Get the best known LB after the preprocessing
		{if (bPREisSolved) return dPRELB; else throw(3);}
	int PREGetNbFix()				// Get the number of variables fixed by the preprocessing
		{if (bPREisSolved) return iPREnbFix; else throw(3);}		
	Variable* PREGetVarInfo()		// Get the variable array containing all the variable informations
		{if(PRElp->LPisSolved()) return PREvarInfo; else throw(1);}
	void PREFixVarToMIP();			// Fix the variables deduced by the preprocessing in the MIP model
									// Necessitate: the LP and the MIP model have the same variables declared in the same order

private:
	// Methods to manipulate the Mathematical models in the preprocessing phase (UNUSABLE BY THE USERS)
	double PREArSup(double value);	// It rounds up a real value (Used by the preprocessing algorithm)
	void PREFixVarToLP();			// Fix the variables(add upper and lower bound to a variable) in the LP problem (Used by the preprocessing algorithm)
																	
	bool PREToFix(int indice,int * head); // To Test if this indice is in the list of indices. indice should be a positive integer.
	void PREFixVar();				// Fix the variables using the class "Variable" to get the information.
};
#endif