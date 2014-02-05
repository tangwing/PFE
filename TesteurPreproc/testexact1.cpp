#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <limits>
#include <ctime>
#include <cmath>
#include <sstream>
#include "RandGeneration.h"
#include "H1Result.h"
#include "PreprocessingResult.h"
#include "CplexResult.h"

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
double ScRes[3][20][8];
char tmp[20];//For sprintf

int Round(double);
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
	   if (DEBUG)
	   {
		   //DisplayData();
		   printf("[DEBUG] The instance is generated: press a key to continue\n");
		   _getch();
	   }

	   printf("The IP model is running...\n");fflush(stdout);
	   spawnl(P_WAIT,"SCPTimInd.exe","SCPTimInd.exe",NULL);
	   if (DEBUG)
	   {
		 printf("[DEBUG] Problem solved\n");
		 _getch();
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

	   if(!cplex.isOptimal)//If not optimal, no need to continue
			continue;

	   //***********************************************
	   //Use the same data to test H1
	   //***********************************************
		printf("The H1 program is running...\n");fflush(stdout);
		spawnl(P_WAIT,"PFE.exe","PFE.exe",NULL);
		H1Result h1;
		h1.ImportFromFile("Heuristic.txt");
		ScRes[1][j][0] = h1.isFeasible;
		ScRes[1][j][1] = (h1.value==ScRes[0][j][2])? 1: 0;
		ScRes[1][j][2] = h1.value;
		ScRes[1][j][3] = h1.durationCpuClock;
		ScRes[1][j][4] = h1.nbMachine;
		ScRes[1][j][5] = -2;
		ScRes[1][j][6] = -2;
		ScRes[1][j][7] = -2;

		//***********************************************
	    //Use the same data to test H2
	    //***********************************************
		printf("The H2 program is running...\n");fflush(stdout);
		spawnl(P_WAIT,"H2.exe","H2.exe",NULL);
		CplexResult h2;
        h2.ImportFromFile("H2.txt");
	    ///Fill the result matrix
	    ScRes[2][j][0] = h2.isFeasible;
	    ScRes[2][j][1] = h2.isOptimal;
	    ScRes[2][j][2] = h2.value;
	    ScRes[2][j][3] = h2.durationCpuClock;;
	    ScRes[2][j][4] = h2.nbMachine;
	    ScRes[2][j][5] = h2.isMemLimit;
	    ScRes[2][j][6] = h2.isTimeLimit;
	    ScRes[2][j][7] = -2;
		


		//***********************************************
	    // Launch Pre
	    //***********************************************
		//if no UB
		if(0==h1.isFeasible && 0==h2.isFeasible)continue;
		double UB = std::numeric_limits<double>::max();
		if( h1.isFeasible) UB = h1.value;
		if( h2.isFeasible && h2.value<UB) UB = h2.value;
		sprintf(tmp, "%d",int(UB));

		printf("The Preprocessing program is running...\n");fflush(stdout);
		spawnl(P_WAIT,"Preprocessing.exe","Preprocessing.exe", tmp,NULL);
		printf("fini\n");
		PreprocessingResult pre;
        pre.ImportFromFile("Preproc.txt");
	    ///Fill the result matrix
		ScRes[3][j][0] = pre.isOptiNoPre;
	    ScRes[3][j][1] = pre.isAllFixed;
		ScRes[3][j][2] = pre.UB;
	    ScRes[3][j][3] = pre.LB;
	    ScRes[3][j][4] = pre.nbBool;
	    ScRes[3][j][5] = pre.nbFixed;
	    ScRes[3][j][6] = pre.lastIFixed;
		ScRes[3][j][7] = pre.value;
	  }

	 MakeStatPreproc( i,
		 ScNM[i][0] + ScNM[i][1] + ScNM[i][2] + ScNM[i][3] + ScNM[i][4], ///N
		 ScNM[i][5] + ScNM[i][6] + ScNM[i][7] + ScNM[i][8],				 ///M
		 iterations);
  }
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

	NbFixMin = 99999999;
	NbFixMax = -1;
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
		Opt = ScRes[0][i][1];
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
			tmp = NbFixed/NbBool;
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
		fprintf(fRes,"Sc(N/M); isFea(E); isOpt(E); sol(E); time(E); isFea(H1); sol(H1); time(H1); isFea(H2); sol(H2); time(H2); isOptNoPre; isAllFixed; LB; UB; nbBool; nbFixed; lastFixedI\n");
		fprintf(fichier,"Sc(N/M); LBDevMin; LBDevAvg; LBDevMax; UBDevMin; UBDevAvg; UBDevMax; FixedMin(%); FixedAvg(%); FixedMax(%); NbFixedMax\n");
	}else
	{
		fRes = fopen("LogStatPre.csv","at");
	    fichier=fopen("StatPre.csv","at");
	}
	if(NbInstanceDev > 0)
		fprintf(fichier,"Sc%d(%d/%d); %d%%; %d%%; %d%%; %d%%; %d%%; %d%%; %d%%; %d%%; %d%%; %d\n",
		IdSce+1, NbTache, NbMach,
		Round(LBDevMin*100), Round(LBDevTotal/NbInstanceDev), Round(LBDevMax*100),
		Round(UBDevMin*100), Round(UBDevTotal/NbInstanceDev), Round(UBDevMax*100),
		Round(RatioFixMin*100), Round(RatioFixTotal/NbInstanceDev), Round(RatioFixMax*100),
		NbFixMax);
	else fprintf(fichier, "Sc%d(%d/%d); *; *; *; *; *; *; *; *; *; *\n",IdSce+1, NbTache, NbMach);
	fclose(fichier);

	//Export all results
	for(int i = 0; i<NbIter; i++)
	{
		//Sc(N/M); isFea(E); isOpt(E); sol(E); time(E); isFea(H1); sol(H1); time(H1); isFea(H2); sol(H2); time(H2); isOptNoPre; isAllFixed; LB; UB; nbBool; nbFixed; lastFixedI
		fprintf(fRes,"Sc%d-%d; %d; %d; %3.2lf; %3.2lf; %d; %3.2lf; %3.2lf; %d; %3.2lf; %3.2lf; %d; %d; %3.2lf; %3.2lf; %d; %d; %d\n",
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
		int(ScRes[3][i][6])
		);
	}
	fclose(fRes);
}



//Round a double to int
//Ex: Round(3.141) = 3
//	  Round(2.56) = 3
int Round(double d)
{
	return int(d+0.5);
}

//Round a double in preserving [point] digits after its decimal point
//Ex: Round(3.141) = 3.14
//	  Round(2.3456,3) = 2.346
double Round(double d, int point)
{
	return double(int((d*pow(double(10), point+1) + 5)/10))/pow(double(10),point);
}