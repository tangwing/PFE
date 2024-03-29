///
///This class represents all result info of H1 ( heuristique de liste).
///
#pragma once
#ifndef CPLEXRESULT_H
#define CPLEXRESULT_H
#include <stdio.h>
class CplexResult
{
public:
	CplexResult()
	{
		isFeasible=isOptimal=isTimeLimit=isMemLimit=-1; 
		value=durationCpuClock=durationWallClock=nbMachine=-1;
		statusCode=nbNode=-1;
	}
	CplexResult(int isFea, int isOpt, int isTimLim, int isMemLim,
		double nbMach, int nbNod, int status, 
		double v, double duree, double dureeCPU)
	  :isFeasible(isFea)
	  ,isOptimal(isOpt)
	  ,isTimeLimit(isTimLim)
	  ,isMemLimit(isMemLim)
	  ,nbMachine(nbMach)
	  ,nbNode(nbNod)
	  ,statusCode(status)
	  ,value(v)
	  ,durationWallClock(duree)
	  ,durationCpuClock(dureeCPU)
	{}

	///@brief export the current result object to file
	void ExportToFile(const char* filename)
	{
		FILE * fic=fopen(filename,"wt");
		fprintf(fic,"%d\n%d\n%lf\n%lf\n%lf\n%d\n%lf\n%d\n%d\n%d\n",
			isFeasible, isOptimal,
			value, durationWallClock, nbMachine, nbNode,durationCpuClock, 
			statusCode, isTimeLimit, isMemLimit);
		fclose(fic);
	}

	///@brief import from file
	void ImportFromFile(const char* filename)
	{
		FILE * fichier=fopen(filename,"rt");
		fscanf(fichier,"%d\n",&isFeasible);
		fscanf(fichier,"%d\n",&isOptimal);
	    fscanf(fichier,"%lf\n",&value);
	    fscanf(fichier,"%lf\n",&durationWallClock);
	    fscanf(fichier,"%lf\n",&nbMachine);
	    fscanf(fichier,"%d\n",&nbNode);
	    fscanf(fichier,"%lf\n",&durationCpuClock);
		fscanf(fichier,"%d\n",&statusCode);
		fscanf(fichier,"%d\n",&isTimeLimit);
		fscanf(fichier,"%d\n",&isMemLimit);
	    fclose(fichier);
	}

	//--------------Members--------------
	int isFeasible;
	int isOptimal;
	int isTimeLimit;
	int isMemLimit;
	double nbMachine;
	int nbNode; //the number of nodes processed so far in the active branch-and-cut search.
	int statusCode;
	double value;
	double durationWallClock;
	double durationCpuClock;
};



#endif