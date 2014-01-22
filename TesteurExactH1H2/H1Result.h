///
///This class represents all result info of H1 ( heuristique de liste).
///
#pragma once
#ifndef H1RESULT_H
#define H1RESULT_H
#include <stdio.h>
class H1Result
{
public:
	H1Result(){isFeasible=-1; value=durationCpuClock=durationWallClock=nbMachine=-1;}
	H1Result(int isFea, double nbMach, double v, double duree, double dureeCPU)
	  :isFeasible(isFea)
	  ,nbMachine(nbMach)
	  ,value(v)
	  ,durationWallClock(duree)
	  ,durationCpuClock(dureeCPU)
	{}

	///@brief export the current result object to file
	void ExportToFile(const char* filename)
	{
		FILE * fic=fopen(filename,"wt");
		fprintf(fic,"%d\n%lf\n%lf\n%lf\n%lf\n",isFeasible, value, durationWallClock, nbMachine, durationCpuClock);
		fclose(fic);
	}

	///@brief import from file
	void ImportFromFile(const char* filename)
	{
		FILE * fichier=fopen(filename,"rt");
		fscanf(fichier,"%d\n",&isFeasible);
	    fscanf(fichier,"%lf\n",&value);
	    fscanf(fichier,"%lf\n",&durationWallClock);
	    fscanf(fichier,"%lf\n",&nbMachine);
	    fscanf(fichier,"%lf\n",&durationCpuClock);
	    fclose(fichier);
	}

	//--------------Members--------------
	int isFeasible;
	double nbMachine;
	double value;
	double durationWallClock;
	double durationCpuClock;
};



#endif