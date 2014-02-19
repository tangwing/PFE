#pragma once
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <process.h>

class Variable
{
private:
	char* VARName;				// String, the variable name
	double VARRedCost;			// The reducedcost of the variable
	double VARLj,VARUj;			// The pseudo-cost of the variable
	double VARValue;			// The variable value in the optimal solution
	int VARBase;				// The basis status of the variable
public:
	Variable(void);										// Class constructor
	///! No dynamically allocated mem. ~Variable(void){delete VARName;}					// Class destructor

	// Get the information of the variable. 
	// The LP problem should be solved, if not, these functions return the default values initilized in the class constructor.
	char* VARGetName() {return VARName;}				// Get the variable name
	double VARGetRedCost(){return VARRedCost;}		// Get the reduced cost of the variable
	
	// Get the pseudo-cost of the variable
	double VARGetLj(){return VARLj;}
	double VARGetUj(){return VARUj;}

	double VARGetValue(){return VARValue;}			// Get the variable value in the optimal solution of LP
	int VARGetBasisStatus(){return VARBase;}		// Get the basis status of the variable

	// Set the value of the attribute in the class
	// name: a string: "" by default
	// reduced cost: 0 by default, computed by CPLEX
	// pseudo-cost: 0 by default, computed by CPLEX
	// value(0~1): -1 by default, computed by CPLEX
	// basis status(0~3): -1 by default, computed by CPLEX
	void VARSetName(char *name){VARName=name;}					// Set the variable name
	void VARSetRedCost(double redcost){VARRedCost=redcost;}		// Set the reduced cost of the variable
	
	void VARSetPseuCost(double lj,double uj){VARLj=lj;VARUj=uj;}	// Set the pseudo-cost of the variable
	void VARSetLj(double lj){VARLj=lj;}
	void VARSetUj(double uj){VARUj=uj;}
	
	void VARSetValue(double value){VARValue=value;}				// Set the variable value in the optimal solution
	void VARSetBasisStatus(int base){VARBase=base;}				// Set the basis status of the variable
	void VAROutput();											// Output the information of the variable

};
