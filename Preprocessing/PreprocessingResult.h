///
///This class represents all result info of Preprocessing. Ex: Result about LP procedure and result about the amount of fixed variables
///
#pragma once
#ifndef PREPROCESSINGRESULT_H
#define PREPROCESSINGRESULT_H
#include <stdio.h>
class PreprocessingResult
{
public:
	PreprocessingResult()
	{
		isOptiNoPre = isAllFixed = isFeasible=isOptimal=isTimeLimit=isMemLimit=-1; 
		value=durationCpuClock=durationPre=nbMachine=-1;
		statusCode=nbNode=nbBool = nbFixed= errCodeLP = -1;
	}
	PreprocessingResult(int isPre,int isInteg, int isFea, int isOpt, int isTimLim, int isMemLim,
		int nbMach, int nbNod, int status, 
		double v, double duree, double dureeCPU, int nbbool, int nbfixed, double ub, double lb, int errLP)
	  :isOptiNoPre(isPre)
	  ,isAllFixed(isInteg)
	  ,isFeasible(isFea)
	  ,isOptimal(isOpt)
	  ,isTimeLimit(isTimLim)
	  ,isMemLimit(isMemLim)
	  ,nbMachine(nbMach)
	  ,nbNode(nbNod)
	  ,statusCode(status)
	  ,value(v)
	  ,durationPre(duree)
	  ,durationCpuClock(dureeCPU)
	  ,nbBool(nbbool)
	  ,nbFixed(nbfixed)
	  ,UB(ub)
	  ,LB(lb)
	  ,errCodeLP(errLP)
	{}

	///@brief export the current result object to file
	void ExportToFile(const char* filename)
	{
		FILE * fic=fopen(filename,"wt");
		fprintf(fic,"%d\n%d\n%d\n%d\n%lf\n%lf\n%lf\n%d\n%lf\n%d\n%d\n%d\n%d\n%d\n%lf\n%lf\n%d\n",
			isOptiNoPre,isAllFixed,isFeasible, isOptimal,
			value, durationPre, nbMachine, nbNode,durationCpuClock, 
			statusCode, isTimeLimit, isMemLimit,nbBool,nbFixed, UB, LB, errCodeLP);
		fclose(fic);
	}

	///@brief import from file
	void ImportFromFile(const char* filename)
	{
		FILE * fichier=fopen(filename,"rt");
		fscanf(fichier,"%d\n",&isOptiNoPre);
		fscanf(fichier,"%d\n",&isAllFixed);
		fscanf(fichier,"%d\n",&isFeasible);
		fscanf(fichier,"%d\n",&isOptimal);
	    fscanf(fichier,"%lf\n",&value);
	    fscanf(fichier,"%lf\n",&durationPre);
	    fscanf(fichier,"%lf\n",&nbMachine);
	    fscanf(fichier,"%d\n",&nbNode);
	    fscanf(fichier,"%lf\n",&durationCpuClock);
		fscanf(fichier,"%d\n",&statusCode);
		fscanf(fichier,"%d\n",&isTimeLimit);
		fscanf(fichier,"%d\n",&isMemLimit);
		fscanf(fichier,"%d\n",&nbBool);
		fscanf(fichier,"%d\n",&nbFixed);
		fscanf(fichier,"%lf\n",&UB);
		fscanf(fichier,"%lf\n",&LB);
		fscanf(fichier,"%d\n",&errCodeLP);
	    fclose(fichier);
	}

	//--------------Members--------------
	int errCodeLP;
	int isOptiNoPre;
	int isAllFixed;
	int nbBool;
	int nbFixed;
	double UB;
	double LB;
	double durationPre;
	
	int isFeasible;
	int isOptimal;
	int isTimeLimit;
	int isMemLimit;
	double nbMachine;
	int nbNode; //the number of nodes processed so far in the active branch-and-cut search.
	int statusCode;
	double value;
	double durationCpuClock;
	
};

#endif PREPROCESSINGRESULT_H