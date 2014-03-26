#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <limits>
#include <ctime>
#include <cmath>
#include <sstream>
#include "RandGeneration.h"
#include "CplexResult.h"
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
char vecFile[20];//For sprintf
int nbBoolX = 0, nbFixX = 0;

double Round(double,int);
double Round(double);
void LogPreprocessingResults(char* , int iSce, int iIter, PreprocessingResult r);

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
	   if(pbIsInstanceFeasible[i][j] == false || pdUBs[i][j]<0)
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

	   /// Get UB wit H2
	  /* printf("The H2 program is running for finding UB...\n");fflush(stdout);
	   sprintf(vecFile, "Vector%d_%d.out",i+1, j+1);
		spawnl(P_WAIT,"H2.exe","H2.exe", vecFile, NULL); 
		CplexResult h2;
        h2.ImportFromFile("H2.txt");
		pdUBs[i][j]=h2.value;*/

		/*static bool firstTime = true;
		FILE *logH2;
		if(firstTime)
		{ 
			firstTime=false;
			logH2 = fopen("H2res.csv","wt");
			fprintf(logH2,"sc_itr, sol; time; isTimLim; isMemLim\n" );
		}else logH2 = fopen("H2res.csv","at");
		fprintf(logH2,"Sc%d-%d; %3.2lf; %3.2lf; %d; %d\n", i+1, j+1, h2.value, h2.durationCpuClock, h2.isTimeLimit, h2.isMemLimit);
		fclose(logH2);*/

		//***********************************************
	    // Launch Pre+MIP with cut 2
	    //***********************************************
		sprintf(tmp, "%d",int(pdUBs[i][j]));

		printf("The Preprocessing program is running...\n");fflush(stdout);
		spawnl(P_WAIT,"Preprocessing.exe","Preprocessing.exe", tmp, "4",NULL); 
		PreprocessingResult pre;
        pre.ImportFromFile("Preproc.txt");
		LogPreprocessingResults("pre_cut3.csv" ,i,j,pre);

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
			fprintf(fRes,"Sc%d-%d; %d; %d; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *; *\n", i+1, j+1, pbIsInstanceFeasible[i][j], pbIsSolOpt[i][j]);
		}
	lastI = iSce;
	lastJ = jIter;
	//Log the current result
	fprintf(fRes,"Sc%d-%d; %d; %d; %3.2lf; %d; %d; %d; %d; %d; %d; %3.2lf; %3.2lf; %3.2lf; %d; %d; %d; %d; %d; %3.2lf; %d; %d; %3.2lf; %3.2lf; %d; %d; %d\n",
		iSce+1, jIter+1, pbIsInstanceFeasible[iSce][jIter], pbIsSolOpt[iSce][jIter], pdSol[iSce][jIter],
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