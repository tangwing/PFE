#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <limits>
#include <ctime>
#include "RandGeneration.h"
#include "CplexResult.h"

#define DEBUG false

unsigned int iterations=20;
//Matrix used to stock all result info of one scenario
//Dim1: 0=ExactMethod, 1=H2
//Dim2: iteration count
//Dim3: 1-7: isFea, isOpt, value, time(by clock()), nbMach,nbMemLimit, nbTimeLimit. 
//		  8: not used
double ScRes[2][20][8];

void MakeComparationMatrix(int IdSce, int NbTache, int NbMach, int NbIter);

//Variables used to compute statistics
void main(void)
{
 int i,j;
 for (i=0;i<8;i++)
 {
	  srand(time(NULL));
	  for (j=0;j<iterations;j++)
	  {
	   printf("\n--------------- Sc %d: Data set %ld -------------\n", i+1, j+1);
	   GenerateRandomInstance(ScNM[i][0],ScNM[i][1],ScNM[i][2],ScNM[i][3],ScNM[i][4],ScNM[i][5],ScNM[i][6],ScNM[i][7],ScNM[i][8], 60, 5);
	   if (DEBUG)
	   {
		   //DisplayData();
		   printf("[DEBUG] The instance is generated: press a key to continue\n");
		   getch();
	   }

	   printf("The IP model is running...\n");
	   spawnl(P_WAIT,"SCPTimInd.exe","SCPTimInd.exe",NULL);
	   if (DEBUG)
	   {
		 printf("[DEBUG] Problem solved\n");
		 getch();
	   }  
	   CplexResult cplex;
	   cplex.ImportFromFile("SCPres.txt");
	   ///Fill the result matrix
	   ScRes[0][j][0] = cplex.isFeasible;
	   ScRes[0][j][1] = cplex.isOptimal;
	   ScRes[0][j][2] = cplex.value;
	   ScRes[0][j][3] = cplex.durationCpuClock;;
	   ScRes[0][j][4] = cplex.nbMachine;
	   ScRes[0][j][5] = cplex.isMemLimit;
	   ScRes[0][j][6] = cplex.isTimeLimit;
	   ScRes[0][j][7] = -2;

	   //***********************************************
	   //Use the same data to test heuristic program
	   //***********************************************
		printf("The H2 program is running...\n");
		spawnl(P_WAIT,"H2.exe","H2.exe",NULL);
		CplexResult h2;
        h2.ImportFromFile("H2.txt");

	    ///Fill the result matrix
	    ScRes[1][j][0] = h2.isFeasible;
	    ScRes[1][j][1] = h2.isOptimal;
	    ScRes[1][j][2] = h2.value;
	    ScRes[1][j][3] = h2.durationCpuClock;;
	    ScRes[1][j][4] = h2.nbMachine;
	    ScRes[1][j][5] = h2.isMemLimit;
	    ScRes[1][j][6] = h2.isTimeLimit;
	    ScRes[1][j][7] = -2;
	  }

	 MakeComparationMatrix( i,
		 ScNM[i][0] + ScNM[i][1] + ScNM[i][2] + ScNM[i][3] + ScNM[i][4], ///N
		 ScNM[i][5] + ScNM[i][6] + ScNM[i][7] + ScNM[i][8],				 ///M
		 iterations);
  }
}

//Add a new line in the statistics table
void MakeComparationMatrix(int IdSce, int NbTache, int NbMach, int NbIter)
{
	static bool printHeader = true;
	int NbResE = 0, 
		NbResH = 0,
		NbOptE = 0,
		NbOptH = 0,
		NbMemLimitE = 0,
		NbTimeLimitE = 0;
	double TMinE,TMinH, TMaxE, TMaxH,
		   DevMin, DevMax;
	double TTotalE = 0,
		   TTotalH = 0,
		   DevTotal = 0;
	int NbInstanceDev = 0; //Nb d'instance qui participe au calcul de déviation

	TMinE = TMinH = DevMin = std::numeric_limits<double>::max();
	TMaxE = TMaxH = DevMax = std::numeric_limits<double>::min();
	DevTotal = 0;
	for(int i=0; i<NbIter; i++)
	{
		if(ScRes[0][i][0] == 1) //There is at least one feasible solution
		{
			NbResE++; 
			if(ScRes[0][i][1] == 1) 
			{
				NbOptE++;
			}
		}
		NbMemLimitE += ScRes[0][i][5];
		NbTimeLimitE += ScRes[0][i][6];
		TTotalE += ScRes[0][i][3];
		if(ScRes[0][i][3]<TMinE) TMinE = ScRes[0][i][3];
		if(ScRes[0][i][3]>TMaxE) TMaxE = ScRes[0][i][3];

		///Heuristic2
		if(ScRes[1][i][0] == 1) {NbResH++; if(ScRes[1][i][1] == 1) NbOptH++;}
		TTotalH += ScRes[1][i][3];
		if(ScRes[1][i][3]<TMinH) TMinH = ScRes[1][i][3];
		if(ScRes[1][i][3]>TMaxH) TMaxH = ScRes[1][i][3];

		///We calculate the deviation only if they have both found a solution. DONC il faut penser ?ça quand je calcule DevAvg! Bête.
		if(ScRes[0][i][0] && ScRes[1][i][0])
		{
			NbInstanceDev ++;
			double dev = (ScRes[1][i][2]-ScRes[0][i][2])/ScRes[0][i][2];
			DevTotal += dev;
			if(dev < DevMin) DevMin = dev;
			if(dev > DevMax) DevMax = dev;
		}
	}

	FILE* fRes = fopen("LogCplexH2.csv","at");
	FILE* fichier=fopen("StatCplexH2.csv","at");
	if(printHeader)
	{
		printHeader = false;
		fprintf(fRes,"Sc(N/M); isFea(E); isOpt(E); sol(E); time(E); nbMach(E);  MemLim(E); TimeLim(E); isFea(H1); sol(H); time(H); nbMach(H)\n");
		fprintf(fichier,"Sc(N/M); #Infeas; #Solved; #Mem; #Tim; TMin; TAvg; TMax; #Infeas; #Solved; TMin; TAvg; TMax; DevMin; DevAvg; DevMax\n");
	}
	fprintf(fichier,"Sc%d(%d/%d); %d; %d; %d; %d; %3.2lf; %3.2lf; %3.2lf; %d; %d; %3.2lf; %3.2lf; %3.2lf",
		IdSce+1, NbTache, NbMach, 
		iterations - NbOptE - NbMemLimitE - NbTimeLimitE, //We calculate #infea like this because some infeasible situations are caused by *Lim and we count this as *Lim. 
		NbOptE, NbMemLimitE, NbTimeLimitE, 
		TMinE, TTotalE/NbIter, TMaxE,
		iterations - NbResH, NbResH,
		TMinH, TTotalH/NbIter, TMaxH);
	if(NbInstanceDev > 0)
		fprintf(fichier, "; %3.2lf; %3.2lf; %3.2lf\n", DevMin, DevTotal/NbInstanceDev, DevMax);
	else fprintf(fichier, "; *; *;*\n");
	fclose(fichier);

	//Export all results
	for(int i = 0; i<NbIter; i++)
	{
		//Sc(N/M); isFea(E); isOpt(E); sol(E); time(E); nbMach(E);  MemLim(E); TimeLim(E); isFea(H1); sol(H); time(H); nbMach(H)
		fprintf(fRes,"Sc%d-%d; %d; %d; %3.2lf; %3.2lf; %d; %d; %d; %d;  %3.2lf; %3.2lf; %d\n",
		IdSce+1, i+1,         
		int(ScRes[0][i][0]), 
		int(ScRes[0][i][1]), 
		ScRes[0][i][2], 
		ScRes[0][i][3], 
		int(ScRes[0][i][4]), 
		int(ScRes[0][i][5]), 
		int(ScRes[0][i][6]),
		int(ScRes[1][i][0]), 
		ScRes[1][i][2], 
		ScRes[1][i][3], 
		int(ScRes[1][i][4])
		);
	}
	fclose(fRes);
}