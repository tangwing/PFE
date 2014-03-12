#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <limits>
#include <ctime>
#include <cmath>
#include <sstream>
#include "RandGeneration.h"
#include "PreprocessingResult.h"
#include "DataBis.h"

#define DEBUG false

unsigned int iterations=20;
//Matrix used to stock all result info of one scenario
//Dim1: 0=ExactMethod, 1=H1, 2=H2, 3=prepocess
//Dim2: iteration count
//Dim3: 1-5: isFea, isOpt, value, time(by clock()), nbMach. 
//		  6: nbMemLimit(for ExactMethod);
//		  7: nbTimeLimit(for ExactMethod).
//		  8: not used
//Dim3(preprocessing.exe):
//   ScRes[3][j][0] = pre.isOptiNoPre;
//   ScRes[3][j][1] = pre.isAllFixed;
//	 ScRes[3][j][2] = pre.UB;
//   ScRes[3][j][3] = pre.LB;
//   ScRes[3][j][4] = pre.nbBool;
//   ScRes[3][j][5] = pre.nbFixed;
//   ScRes[3][j][6] = pre.lastIFixed;
//	 ScRes[3][j][7] = pre.value;
//Dim4(Preprocessing for X only)
double ScRes[4][20][8];
char tmp[20];//For sprintf
//char *tmp2="xonly";//For sprintf
int nbBoolX = 0, nbFixX = 0;

double Round(double,int);
double Round(double);
void LogPreprocessingResults(char* , int iSce, int iIter, PreprocessingResult r);
void LogPreprocessingResults1(char* , int iSce, int iIter, PreprocessingResult r);
void LogPreprocessingResults2(char* , int iSce, int iIter, PreprocessingResult r);
void LogPreprocessingResults3(char* , int iSce, int iIter, PreprocessingResult r);
void MakeStatPreproc(int IdSce, int NbTache, int NbMach, int NbIter);

//Variables used to compute statistics
void main(void)
{
 int i,j;
 for (i=0;i<8;i++)
 {
	  srand(1);
	  for (j=0;j<iterations;j++)
	  {
	   printf("\n--------------- Sc %d: Data set %d -------------\n", i+1, j+1);fflush(stdout);
	   GenerateRandomInstance(ScNM[i][0],ScNM[i][1],ScNM[i][2],ScNM[i][3],ScNM[i][4],ScNM[i][5],ScNM[i][6],ScNM[i][7],ScNM[i][8], 60, 5);
	   //We skip no opt instances thanks to Preprocessing
	   if(pbIsInstanceFeasible[i][j] == false)
	   {
		   printf("Instance not feasible, skip...\n");
		   continue;
	   }

	   if (DEBUG)
	   {
		   //DisplayData();
		   printf("[DEBUG] The instance is generated: press a key to continue\n");
		   _getch();
	   }

		//***********************************************
	    // Launch Pre+MIP with cut 2
	    //***********************************************
		sprintf(tmp, "%d",int(pdUBs[i][j]));

		printf("The Preprocessing program is running without cuts2...\n");fflush(stdout);
		spawnl(P_WAIT,"Preprocessing.exe","Preprocessing.exe", tmp, "2", NULL); 
		PreprocessingResult pre;
        pre.ImportFromFile("Preproc.txt");
		LogPreprocessingResults2("pre_cut2.csv" ,i,j,pre);

	  }
  }
}

void LogPreprocessingResults(char* filename, int iSce, int jIter, PreprocessingResult r)
{
	static bool printHeader = true;
	static int lastI=0, lastJ=-1;
	FILE* fRes;
	if(printHeader)
	{
		printHeader = false;
		fRes = fopen(filename,"wt");
		fprintf(fRes,"Sc(N/M); isFea(E); isOpt(E); sol(E); errCodeLP; isOptNoPre; isAllFixed; nbVar; nbVarValid; nbFixed; UB; LB; dureePre; isMIP; isFea; isOpt; TimLim; MemLim; nbMach; nbNode;  statusCode; sol; tempsTotal; nbConCut1; nbConCut2; nbConCut3\n");
	}else
	{
		fRes = fopen(filename,"at");
	}

	//Blank lines
	for(int i=lastI, j=lastJ+1; i<=iSce; i++, j=0)
		for(; j<20; j++)
		{
			if(i==iSce && j==jIter)break;
			fprintf(fRes,"Sc%d-%d; %d; %d; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *\n", i, j, pbIsInstanceFeasible[i][j], pbIsSolOpt[i][j]);
		}
	lastI = iSce;
	lastJ = jIter;
	//Log the current result
	fprintf(fRes,"Sc%d-%d; %d; %d; %3.2lf; %d; %d; %d; %d; %d; %d; %3.2lf; %3.2lf; %3.2lf; %d; %d; %d; %d; %d; %3.2lf; %d; %d; %3.2lf; %3.2lf; %d; %d; %d\n",
		iSce, jIter, pbIsInstanceFeasible[iSce][jIter], pbIsSolOpt[iSce][jIter], pdSol[iSce][jIter],
		r.errCodeLP,
		r.isOptiNoPre,
		r.isAllFixed,
		r.nbBool,
		r.nbBoolExtractable,
		r.nbFixed,
		r.UB,
		r.LB,
		r.durationPre,
		r.isMIPExecuted,
		r.isFeasible,
		r.isOptimal,
		r.isTimeLimit,
		r.isMemLimit,
		r.nbMachine,
		r.nbNode,
		r.statusCode,
		r.value,
		r.durationCpuClock,
		r.nbConCut1,
		r.nbConCut2,
		r.nbConCut3
		);
	fclose(fRes);
}

void LogPreprocessingResults1(char* filename, int iSce, int jIter, PreprocessingResult r)
{
	static bool printHeader = true;
	static int lastI=0, lastJ=-1;
	FILE* fRes;
	if(printHeader)
	{
		printHeader = false;
		fRes = fopen(filename,"wt");
		fprintf(fRes,"Sc(N/M); isFea(E); isOpt(E); sol(E); errCodeLP; isOptNoPre; isAllFixed; nbVar; nbVarValid; nbFixed; UB; LB; dureePre; isMIP; isFea; isOpt; TimLim; MemLim; nbMach; nbNode;  statusCode; sol; tempsTotal; nbConCut1; nbConCut2; nbConCut3\n");
	}else
	{
		fRes = fopen(filename,"at");
	}

	//Blank lines
	for(int i=lastI, j=lastJ+1; i<=iSce; i++, j=0)
		for(; j<20; j++)
		{
			if(i==iSce && j==jIter)break;
			fprintf(fRes,"Sc%d-%d; %d; %d; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *\n", i, j, pbIsInstanceFeasible[i][j], pbIsSolOpt[i][j]);
		}
	lastI = iSce;
	lastJ = jIter;
	//Log the current result
	fprintf(fRes,"Sc%d-%d; %d; %d; %3.2lf; %d; %d; %d; %d; %d; %d; %3.2lf; %3.2lf; %3.2lf; %d; %d; %d; %d; %d; %3.2lf; %d; %d; %3.2lf; %3.2lf; %d; %d; %d\n",
		iSce, jIter, pbIsInstanceFeasible[iSce][jIter], pbIsSolOpt[iSce][jIter], pdSol[iSce][jIter],
		r.errCodeLP,
		r.isOptiNoPre,
		r.isAllFixed,
		r.nbBool,
		r.nbBoolExtractable,
		r.nbFixed,
		r.UB,
		r.LB,
		r.durationPre,
		r.isMIPExecuted,
		r.isFeasible,
		r.isOptimal,
		r.isTimeLimit,
		r.isMemLimit,
		r.nbMachine,
		r.nbNode,
		r.statusCode,
		r.value,
		r.durationCpuClock,
		r.nbConCut1,
		r.nbConCut2,
		r.nbConCut3
		);
	fclose(fRes);
}

void LogPreprocessingResults2(char* filename, int iSce, int jIter, PreprocessingResult r)
{
	static bool printHeader = true;
	static int lastI=0, lastJ=-1;
	FILE* fRes;
	if(printHeader)
	{
		printHeader = false;
		fRes = fopen(filename,"wt");
		fprintf(fRes,"Sc(N/M); isFea(E); isOpt(E); sol(E); errCodeLP; isOptNoPre; isAllFixed; nbVar; nbVarValid; nbFixed; UB; LB; dureePre; isMIP; isFea; isOpt; TimLim; MemLim; nbMach; nbNode;  statusCode; sol; tempsTotal; nbConCut1; nbConCut2; nbConCut3\n");
	}else
	{
		fRes = fopen(filename,"at");
	}

	//Blank lines
	for(int i=lastI, j=lastJ+1; i<=iSce; i++, j=0)
		for(; j<20; j++)
		{
			if(i==iSce && j==jIter)break;
			fprintf(fRes,"Sc%d-%d; %d; %d; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *\n", i, j, pbIsInstanceFeasible[i][j], pbIsSolOpt[i][j]);
		}
	lastI = iSce;
	lastJ = jIter;
	//Log the current result
	fprintf(fRes,"Sc%d-%d; %d; %d; %3.2lf; %d; %d; %d; %d; %d; %d; %3.2lf; %3.2lf; %3.2lf; %d; %d; %d; %d; %d; %3.2lf; %d; %d; %3.2lf; %3.2lf; %d; %d; %d\n",
		iSce, jIter, pbIsInstanceFeasible[iSce][jIter], pbIsSolOpt[iSce][jIter], pdSol[iSce][jIter],
		r.errCodeLP,
		r.isOptiNoPre,
		r.isAllFixed,
		r.nbBool,
		r.nbBoolExtractable,
		r.nbFixed,
		r.UB,
		r.LB,
		r.durationPre,
		r.isMIPExecuted,
		r.isFeasible,
		r.isOptimal,
		r.isTimeLimit,
		r.isMemLimit,
		r.nbMachine,
		r.nbNode,
		r.statusCode,
		r.value,
		r.durationCpuClock,
		r.nbConCut1,
		r.nbConCut2,
		r.nbConCut3
		);
	fclose(fRes);
}void LogPreprocessingResults3(char* filename, int iSce, int jIter, PreprocessingResult r)
{
	static bool printHeader = true;
	static int lastI=0, lastJ=-1;
	FILE* fRes;
	if(printHeader)
	{
		printHeader = false;
		fRes = fopen(filename,"wt");
		fprintf(fRes,"Sc(N/M); isFea(E); isOpt(E); sol(E); errCodeLP; isOptNoPre; isAllFixed; nbVar; nbVarValid; nbFixed; UB; LB; dureePre; isMIP; isFea; isOpt; TimLim; MemLim; nbMach; nbNode;  statusCode; sol; tempsTotal; nbConCut1; nbConCut2; nbConCut3\n");
	}else
	{
		fRes = fopen(filename,"at");
	}

	//Blank lines
	for(int i=lastI, j=lastJ+1; i<=iSce; i++, j=0)
		for(; j<20; j++)
		{
			if(i==iSce && j==jIter)break;
			fprintf(fRes,"Sc%d-%d; %d; %d; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *\n", i, j, pbIsInstanceFeasible[i][j], pbIsSolOpt[i][j]);
		}
	lastI = iSce;
	lastJ = jIter;
	//Log the current result
	fprintf(fRes,"Sc%d-%d; %d; %d; %3.2lf; %d; %d; %d; %d; %d; %d; %3.2lf; %3.2lf; %3.2lf; %d; %d; %d; %d; %d; %3.2lf; %d; %d; %3.2lf; %3.2lf; %d; %d; %d\n",
		iSce, jIter, pbIsInstanceFeasible[iSce][jIter], pbIsSolOpt[iSce][jIter], pdSol[iSce][jIter],
		r.errCodeLP,
		r.isOptiNoPre,
		r.isAllFixed,
		r.nbBool,
		r.nbBoolExtractable,
		r.nbFixed,
		r.UB,
		r.LB,
		r.durationPre,
		r.isMIPExecuted,
		r.isFeasible,
		r.isOptimal,
		r.isTimeLimit,
		r.isMemLimit,
		r.nbMachine,
		r.nbNode,
		r.statusCode,
		r.value,
		r.durationCpuClock,
		r.nbConCut1,
		r.nbConCut2,
		r.nbConCut3
		);
	fclose(fRes);
}

//Add a new line in the statistics table of Preprocessing
void MakeStatPreproc(int IdSce, int NbTache, int NbMach, int NbIter)
{
	static bool printHeader = true;
	int  NbFixMin, NbFixMax,NbBool, NbFixed;
	double Opt, UB, LB;
	double LBDevMin, LBDevMax, LBDevTotal, 
		UBDevMin, UBDevMax, UBDevTotal, 
		RatioFixMin, RatioFixMax, RatioFixTotal;
	double tmp;

	int NbInstanceDev = 0; //Nb d'instance qui participe au calcul de déviation

	NbFixMin = std::numeric_limits<int>::max();
	NbFixMax = std::numeric_limits<int>::min();
	LBDevMin = UBDevMin = RatioFixMin = std::numeric_limits<double>::max();
	LBDevMax = UBDevMax = RatioFixMax = std::numeric_limits<double>::min();
	LBDevTotal = UBDevTotal = RatioFixTotal= 0;
	for(int i=0; i<NbIter; i++)
	{
		if(ScRes[0][i][1] == 0) //No optimal solution, no need to continue
			continue;
		//no ub
		if( 0 == ScRes[1][i][0] &&
			0 == ScRes[2][i][0])
			continue;

		NbInstanceDev++;
		Opt = ScRes[0][i][2];
		UB = ScRes[3][i][2];
		LB = ScRes[3][i][3];
		NbBool = int(ScRes[3][i][4]);
		NbFixed = int(ScRes[3][i][5]);
		//LB
		tmp = (Opt-LB)/Opt;
		if(tmp<LBDevMin)LBDevMin = tmp;
		if(tmp>LBDevMax) LBDevMax = tmp;
		LBDevTotal += tmp;
		//UB
		tmp = (UB - Opt)/Opt;
		if(tmp<UBDevMin)UBDevMin = tmp;
		if(tmp>UBDevMax)UBDevMax = tmp;
		UBDevTotal += tmp;
		//RatioFix
		if(NbFixed< NbFixMin)NbFixMin = NbFixed;
		if(NbFixed> NbFixMax)NbFixMax = NbFixed;
		if(NbBool!=0)
		{
			tmp = NbFixed/double(NbBool);
			if(tmp < RatioFixMin) RatioFixMin=tmp;
			if(tmp > RatioFixMax) RatioFixMax=tmp;
			RatioFixTotal+=tmp;
		}
	}

	FILE* fRes;
	FILE* fichier;
	if(printHeader)
	{
		printHeader = false;
		fRes = fopen("LogStatPre.csv","wt");
	    fichier=fopen("StatPre.csv","wt");
		fprintf(fRes,"Sc(N/M); isFea(E); isOpt(E); sol(E); time(E); isFea(H1); sol(H1); time(H1); isFea(H2); sol(H2); time(H2); isOptNoPre; isAllFixed; LB; UB; nbBool; nbFixed; nbBoolX; nbFixedX\n");
		fprintf(fichier,"Sc(N/M); LBDevMin; LBDevAvg; LBDevMax; UBDevMin; UBDevAvg; UBDevMax; FixedMin(%%); FixedAvg(%%); FixedMax(%%); NbFixedMax\n");
	}else
	{
		fRes = fopen("LogStatPre.csv","at");
	    fichier=fopen("StatPre.csv","at");
	}
	if(NbInstanceDev > 0)
		fprintf(fichier,"Sc%d(%d/%d); %.2lf%%; %.2lf%%; %.2lf%%; %.2lf%%; %.2lf%%; %.2lf%%; %.2lf%%; %.2lf%%; %.2lf%%; %d\n",
		IdSce+1, NbTache, NbMach,
		Round(LBDevMin*100), Round(LBDevTotal/NbInstanceDev*100), Round(LBDevMax*100),
		Round(UBDevMin*100), Round(UBDevTotal/NbInstanceDev*100), Round(UBDevMax*100),
		Round(RatioFixMin*100), Round(RatioFixTotal/NbInstanceDev*100), Round(RatioFixMax*100),
		NbFixMax);
	else fprintf(fichier, "Sc%d(%d/%d); *; *; *; *; *; *; *; *; *; *\n",IdSce+1, NbTache, NbMach);
	fclose(fichier);

	//Export all results
	for(int i = 0; i<NbIter; i++)
	{
		//Sc(N/M); isFea(E); isOpt(E); sol(E); time(E); isFea(H1); sol(H1); time(H1); isFea(H2); sol(H2); time(H2); isOptNoPre; isAllFixed; LB; UB; nbBool; nbFixed; nbBoolX, nbFixedX
		fprintf(fRes,"Sc%d-%d; %d; %d; %3.2lf; %3.2lf; %d; %3.2lf; %3.2lf; %d; %3.2lf; %3.2lf; %d; %d; %3.2lf; %3.2lf; %d; %d; %d; %d\n",
		IdSce+1, i+1,         
		int(ScRes[0][i][0]), 
		int(ScRes[0][i][1]), 
		ScRes[0][i][2], 
		ScRes[0][i][3], 
		
		int(ScRes[1][i][0]), 
		ScRes[1][i][2], 
		ScRes[1][i][3], 
		
		int(ScRes[2][i][0]), 
		ScRes[2][i][2], 
		ScRes[2][i][3], 

		int(ScRes[3][i][0]),
		int(ScRes[3][i][1]),
		(ScRes[3][i][3]),
		(ScRes[3][i][2]),
		int(ScRes[3][i][4]),
		int(ScRes[3][i][5]),
		//Preprocess X only
		int(ScRes[4][i][4]),
		int(ScRes[4][i][5])
		);
	}
	fclose(fRes);
}



//Round a double to int
//Ex: Round(3.141) = 3
//	  Round(2.56) = 3
double Round(double d)
{
	//return int(d+0.5);
	return d;
}

//Round a double in preserving [point] digits after its decimal point
//Ex: Round(3.141) = 3.14
//	  Round(2.3456,3) = 2.346
double Round(double d, int point)
{
	return double(int((d*pow(double(10), point+1) + 5)/10))/pow(double(10),point);
}