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
		isMIPExecuted = isOptiNoPre = isAllFixed = isFeasible=isOptimal=isTimeLimit=isMemLimit=-1; 
		UB = LB = value=durationCpuClock=durationPre=-1;
		nbConCut1=nbConCut2=nbConCut3=statusCode=nbNode=nbMachine=nbBool = nbBoolExtractable =nbFixed= errCodeLP = -1;
	}
	//PreprocessingResult(int isPre,int isInteg, int isFea, int isOpt, int isTimLim, int isMemLim,
	//	int nbMach, int nbNod, int status, 
	//	double v, double duree, double dureeCPU, int nbbool,  int nbboole, int nbfixed, double ub, double lb, int errLP)
	//  :isOptiNoPre(isPre)
	//  ,isAllFixed(isInteg)
	//  ,isFeasible(isFea)
	//  ,isOptimal(isOpt)
	//  ,isTimeLimit(isTimLim)
	//  ,isMemLimit(isMemLim)
	//  ,nbMachine(nbMach)
	//  ,nbNode(nbNod)
	//  ,statusCode(status)
	//  ,value(v)
	//  ,durationPre(duree)
	//  ,durationCpuClock(dureeCPU)
	//  ,nbBool(nbbool)
	//  ,nbBoolExtractable(nbboole)
	//  ,nbFixed(nbfixed)
	//  ,UB(ub)
	//  ,LB(lb)
	//  ,errCodeLP(errLP)
	//{}

	///@brief export the current result object to file
	void ExportToFile(const char* filename)
	{
		FILE * fic=fopen(filename,"wt");
		fprintf(fic,"%d\n%d\n%d\n%d\n%d\n%d\n%lf\n%lf\n%lf\n",
			errCodeLP, isOptiNoPre,isAllFixed,nbBool, nbBoolExtractable,nbFixed, UB, LB,durationPre);
			
		fprintf(fic,"%d\n%d\n%d\n%d\n%d\n%lf\n%d\n%d\n%lf\n%lf\n%d\n%d\n%d\n",
			isMIPExecuted, isFeasible, isOptimal, isTimeLimit, isMemLimit, nbMachine, nbNode, statusCode,value, durationCpuClock, nbConCut1,nbConCut2,nbConCut3);
		fclose(fic);
	}

	void Test()
	{
		ImportFromFile("PreProc.txt");
		printf("%d\n%d\n%d\n%d\n%d\n%d\n%lf\n%lf\n%lf\n",
			errCodeLP, isOptiNoPre,isAllFixed,nbBool, nbBoolExtractable,nbFixed, UB, LB,durationPre);
			
		printf("%d\n%d\n%d\n%d\n%d\n%lf\n%d\n%d\n%lf\n%lf\n%d\n%d\n%d\n",
			isMIPExecuted, isFeasible, isOptimal, isTimeLimit, isMemLimit, nbMachine, nbNode, statusCode,value, durationCpuClock, nbConCut1,nbConCut2,nbConCut3);
		
	}

	///@brief import from file
	void ImportFromFile(const char* filename)
	{
		FILE * fichier=fopen(filename,"rt");
		fscanf(fichier,"%d\n",&errCodeLP);
		fscanf(fichier,"%d\n",&isOptiNoPre);
		fscanf(fichier,"%d\n",&isAllFixed);
		fscanf(fichier,"%d\n",&nbBool);
		fscanf(fichier,"%d\n",&nbBoolExtractable);
		fscanf(fichier,"%d\n",&nbFixed);
		fscanf(fichier,"%lf\n",&UB);
		fscanf(fichier,"%lf\n",&LB);
	    fscanf(fichier,"%lf\n",&durationPre);

		fscanf(fichier,"%d\n",&isMIPExecuted);
		fscanf(fichier,"%d\n",&isFeasible);
		fscanf(fichier,"%d\n",&isOptimal);
		fscanf(fichier,"%d\n",&isTimeLimit);
		fscanf(fichier,"%d\n",&isMemLimit);
	    fscanf(fichier,"%lf\n",&nbMachine);
	    fscanf(fichier,"%d\n",&nbNode);
		fscanf(fichier,"%d\n",&statusCode);
	    fscanf(fichier,"%lf\n",&value);
	    fscanf(fichier,"%lf\n",&durationCpuClock);
		fscanf(fichier,"%d\n", &nbConCut1);
		fscanf(fichier,"%d\n", &nbConCut2);
		fscanf(fichier,"%d\n", &nbConCut3);
	    fclose(fichier);
	}

	//--------------Members--------------
	// Preprocessing/LP part
	int errCodeLP;		//LP exception
	int isOptiNoPre;
	int isAllFixed;
	int nbBool;
	int nbBoolExtractable;
	int nbFixed;
	double UB;
	double LB;
	double durationPre;
	
	// MIP part: 
	//If errCodeLP!=-1 ou isOptiNoPre==1 ou isAllFixed==1 ou nbFixed==0, the MIP won't be executed!!
	int isMIPExecuted;
	int isFeasible;
	int isOptimal;
	int isTimeLimit;
	int isMemLimit;
	double nbMachine;
	int nbNode; //the number of nodes processed so far in the active branch-and-cut search.
	int statusCode;
	double value;
	double durationCpuClock;

	int nbConCut1;
	int nbConCut2;
	int nbConCut3;
	
};

#endif PREPROCESSINGRESULT_H