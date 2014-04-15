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
int CUT2_ORDER = 2;//1->RHS; 2->LHS/RHS
bool MIP_START = false;

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
char tmp2[20];//For sprintf
char vecFile[30];//For sprintf. vector file for addMipStart
//char *tmp2="xonly";//For sprintf
int nbBoolX = 0, nbFixX = 0;

double Round(double,int);
double Round(double);
void LogCut2Level(char* , int iSce, int iIter, int level, PreprocessingResult r);

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
	   //We skip no fea instances 
	   if(!pbIsInstanceFeasible[i][j] || pdUBs[i][j]<0)
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
		PreprocessingResult pre;
		int nbCut2 = 0;
		int cut2Level = 0; //NB_Seuil not levels
		int N = ScNM[i][0] + ScNM[i][1] + ScNM[i][2] + ScNM[i][3] + ScNM[i][4];
		int M = ScNM[i][5] + ScNM[i][6] + ScNM[i][7] + ScNM[i][8];
		//Apply the threshold fonction when M > 4
		if(M>4)
		{
			if(CUT2_ORDER == 1)	cut2Level = int(-66.66666667*N + 1400*M - 5200);
			else if(CUT2_ORDER == 2) cut2Level = int(-200*N + 2000*M - 4600);
		}

		sprintf(tmp2, "%d", cut2Level);
		printf("The Preprocessing program is running with cuts2 seuil %d...\n",cut2Level);fflush(stdout);
		if(MIP_START)
		{
			sprintf(vecFile, "Vectors/Vector%d_%d.out",i+1, j+1);
			spawnl(P_WAIT,"Preprocessing.exe","Preprocessing.exe", tmp, tmp2, vecFile, NULL); 
		}
		else spawnl(P_WAIT,"Preprocessing.exe","Preprocessing.exe", tmp, tmp2, NULL); 
        pre.ImportFromFile("Preproc.txt");

		LogCut2Level("pre_cut2_seuil2_X.csv" ,i,j, cut2Level, pre);
	  }
  }
}

void LogCut2Level(char* filename, int iSce, int jIter, int level, PreprocessingResult r)
{
	static bool printHeader = true;
	FILE* fRes;
	if(printHeader)
	{
		printHeader = false;
		fRes = fopen(filename,"wt");
		fprintf(fRes,"Sc(N/M); 1-cutSeuil; sol_nopre; UB; LB; isOptNoPre; isAllFixed; isMipExecuted; sol_pre; statusCode; nbVar; nbFixed; nbX; nbXFixed; nbNode; tempsTotal; nbConCut2\n");
	}else
	{
		fRes = fopen(filename,"at");
	}
	//Log the current result
	fprintf(fRes,"Sc%d-%d; %d; %3.2lf; %3.2lf; %3.2lf; %d; %d; %d; %3.2lf; %d; %d; %d; %d; %d; %d; %3.2lf; %d\n",
		iSce+1, jIter+1, 
		level,
		pdSol[iSce][jIter],
		r.UB,
		r.LB,
		r.isOptiNoPre,
		r.isAllFixed,
		r.isMIPExecuted,
		r.value,
		r.statusCode,
		r.nbBoolExtractable,
		r.nbFixed,
		r.nbXExtractable,
		r.nbXFixed,
		r.nbNode,
		r.durationCpuClock,
		r.nbConCut2
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