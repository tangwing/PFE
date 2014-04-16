/** This is a model for MIP */
#ifndef CMixInPr
#define CMixInPr 100

#include <ilcplex/ilocplex.h>
#include <ilcplex/cplex.h>

ILOSTLBEGIN

/*---------------------------------------------------------------------------------
Notice: The class MixedIntegerProgram is designed for solving minimization problems
----------------------------------------------------------------------------------*/

class MixedIntegerProgram
{
private:
	// Attributes related to the Mathematical models
	IloCplex *MIPcplex;			// A pointer to the MIP cplex that solves the problem
	IloModel *MIPmodel;			// A pointer to the MIP model to solve by MIPcplex
	IloNumVarArray *MIPvar;		// A pointer to the mixed integer variables
	IloRangeArray *MIPcon;		// A pointer to the set of contraints of the model
    bool bMIPisSet;				// A flag to know if the MIP model has been correctly initialized

	// Attributes related to the solution of the MIP model
	bool bMIPisSolved;			// Flag to know if the current model has been solved or not
	bool bMIPisOptimal;			// Flag to know if CPLEX found the optimal solution or not
	double dMIPOptimalValue;	// The optimal solution value computed by CPLEX
	double *pdMIPVariables;		// Values of the variables in the optimal solution computed by CPLEX
	int iMIPNbNode;				// The number of node traveled while solving the MIP

public:
	// Methods to manipulate the MIP model

	// Methods related to the initialization of the MIP model
	MixedIntegerProgram(void);								// Default constructor: before using the model, the method MIPInitialize() must be called
	void MIPInitialize(IloEnv *penv, IloCplex *pcplex, IloModel *pmodel, IloNumVarArray *pvar, IloRangeArray *pcon);
															// This method must be used to initialize the model
	void LPAddCuts(IloConstraintArray*);					// Adds the constraints to the model
	void LPRemoveCuts(IloConstraintArray*);					// Removes the constraints from the model
	~MixedIntegerProgram(void){delete [] pdMIPVariables;}	// Destructor of the class
	void MIPSolveByCplex();									// Solve the model with CPLEX solver
	void MIPAddFix(int iIndex, int iValue);					// To fix the (iIndex)th variable to iValue
	bool MIPisSet(){return bMIPisSet;}						// True if the MIP has been set, false otherwise
	bool MIPisSolved(){return bMIPisSolved;}				// True if the MIP has been solved, false otherwise
	bool MIPisOpt(){return bMIPisOptimal;}					// True if the solver has found an optimal value, false otherwise
	double MIPGetOptValue(){return dMIPOptimalValue;}		// Getter for the optimal value
	double* MIPGetvariables(){return pdMIPVariables;}		// Getter for the values of the variables of the optimal solution found by the solver
	int MIPGetNbNode(){return iMIPNbNode;}					// Getter for the number of nodes the solver traveled to solve the model
};
#endif
