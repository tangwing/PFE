#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <limits>
#include "RandGeneration.h"
#include "H1Result.h"
#include "CplexResult.h"

#define DEBUG false

unsigned int iterations=20;
//Matrix used to stock all result info of one scenario
//Dim1: 0=ExactMethod, 1=H2
//Dim2: iteration count
//Dim3: 1-5: isFea, isOpt, value, time, nbMach. 
//		  6: nbMemLimit
//		  7: nbTimeLimit
//		  8: CpuTime
double ScRes[2][20][8];

void MakeComparationMatrix(int IdSce, int NbTache, int NbMach, int NbIter);
//Variables used to compute statistics
unsigned int uiIntSolv,uiMemLim,uiTimLim, Ndmin,Ndmax,InFeas,Solved;
double Ndmoy, Timoy,Timin,Timax,dNbMachmin,dNbMachmoy,dNbMachmax;

void main(void)
{
 FILE *fichier;
 int i,j;
 int isFeas,isOpt,iOptValue,iNbNodes,iTot,iTot2;
 double dTime,dNbMach, dCpuTime;

 fichier=fopen("ScpIPStats.txt","wt");
 fprintf(fichier,"Results of the IP model on the Amazon-like instances\n");
 fprintf(fichier,"----------------------------------------------------\n\n"); 
 fprintf(fichier,"Number of instances for each scenario: %d\n\n",iterations); 
 fprintf(fichier,"Scenario \#InFeas \#InstSolved \#MemLim \#TimLim \#Nodes\_min \#Nodes\_moy \#Nodes\_max Time\_min Time\_moy Time\_max \#NbMach\_min \#NbMach\_moy \#NbMach\_max\n");
 fprintf(fichier,"---------------------------------------------------------------------------------------------------------------------------------------------------------------\n"); 
 fclose(fichier);

 fichier=fopen("ScpTraces.txt","wt");
 fprintf(fichier,"Scenario isInFeas isInstSolved MemLim TimLim \#Nodes Time NbMachUsed\n\n");
 fclose(fichier);
 
 for (i=0;i<6;i++)
 {
  srand(1);
  uiIntSolv=uiMemLim=uiTimLim=Ndmax=0;
  Timax=0;
  Ndmin=99999999999;
  Timin=999999999999.0;
  Ndmoy=Timoy=0;
  InFeas=0;
  Solved=0;
  iTot=0;
  iTot2=0;
  dNbMachmin=99999999.0;
  dNbMachmoy=dNbMachmax=0;

  printf("Scenario Sc%ld\n",i+1);

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
   //continue;
   printf("The IP model is running...\n");

   spawnl(P_WAIT,"SCPTimInd.exe","SCPTimInd.exe",NULL);
   CplexResult cplex;
   cplex.ImportFromFile("SCPres.txt");
   isFeas = cplex.isFeasible;
   isOpt = cplex.isOptimal;
   iOptValue = int(cplex.value);
   dTime = cplex.durationWallClock;
   iNbNodes = cplex.nbNode;
   dNbMach = cplex.nbMachine;
   dCpuTime = cplex.durationCpuClock;

   if (DEBUG)
   {
	printf("[DEBUG] Problem solved\n");
	getch();
   }

   ///Fill the result matrix
   ScRes[0][j][0] = isFeas;
   ScRes[0][j][1] = isOpt;
   ScRes[0][j][2] = iOptValue;
   ScRes[0][j][3] = dTime;
   ScRes[0][j][4] = dNbMach;
   ScRes[0][j][5] = cplex.isMemLimit;
   ScRes[0][j][6] = cplex.isTimeLimit;
   ScRes[0][j][7] = dCpuTime;


   if (isFeas==0) InFeas++;
   if (isFeas==1 && isOpt==0 && dTime>1798.0)  {uiTimLim++; }
   if (isFeas==1 && isOpt==0 && dTime<=1798.0) {uiMemLim++; }
   if (isFeas==1 && isOpt==1) Solved++;

   if ((!(isFeas==1 && isOpt==0))&&(dTime>0.1 ||iNbNodes>0))
   {
    Ndmoy+=iNbNodes;
    if (iNbNodes<Ndmin) Ndmin=iNbNodes;
    if (iNbNodes>Ndmax) Ndmax=iNbNodes;
    Timoy+=dTime;
	if (dTime<Timin) Timin=dTime;
	if (dTime>Timax) Timax=dTime;
	if (dNbMach!=-1.0)
	{
	 dNbMachmoy+=dNbMach;
	 if (dNbMach>dNbMachmax) dNbMachmax=dNbMach;
	 if (dNbMach<dNbMachmin) dNbMachmin=dNbMach;
	 iTot2++;
	}
	iTot++;
   }

   fichier=fopen("ScpTraces.txt","at");
   fprintf(fichier,"Sc%d %d %d %d %lf %lf %ld\n",i+1,isFeas,isOpt,iNbNodes,dTime,dNbMach, iOptValue);
   fclose(fichier);

   //***********************************************
   //Use the same data to test heuristic program
   //***********************************************
	   spawnl(P_WAIT,"H2.exe","H2.exe",NULL);
	   CplexResult h2;
	   h2.ImportFromFile("H2.txt");
	   isFeas = h2.isFeasible;
	   isOpt = h2.isOptimal;
	   iOptValue = int(h2.value);
	   dTime = h2.durationWallClock;
	   iNbNodes = h2.nbNode;
	   dNbMach = h2.nbMachine;
	   dCpuTime = h2.durationCpuClock;
	  
	   ///Fill the result matrix
	   ScRes[0][j][0] = isFeas;
	   ScRes[0][j][1] = isOpt;
	   ScRes[0][j][2] = iOptValue;
	   ScRes[0][j][3] = dTime;
	   ScRes[0][j][4] = dNbMach;
	   ScRes[0][j][5] = cplex.isMemLimit;
	   ScRes[0][j][6] = cplex.isTimeLimit;
	   ScRes[0][j][7] = dCpuTime;

	   fichier=fopen("ScpTraces.txt","at");
	   fprintf(fichier,"Sc%d %d * * %lf %lf %ld\n",i+1,isFeas,dTime,dNbMach, iOptValue);
	   fclose(fichier);
   
  }

 MakeComparationMatrix( i,
	 ScNM[i][0] + ScNM[i][1] + ScNM[i][2] + ScNM[i][3] + ScNM[i][4], ///N
	 ScNM[i][5] + ScNM[i][6] + ScNM[i][7] + ScNM[i][8],				 ///M
	 iterations);
 fichier=fopen("ScpIPStats.txt","at");
 fprintf(fichier,"Sc%d %d %d %d %d %d %lf %d %3.2lf %3.2lf %3.2lf %2.2lf %2.2lf %2.2lf\n",i+1,InFeas,Solved,uiMemLim,uiTimLim,Ndmin,(double)Ndmoy/(double)iTot,Ndmax,Timin,Timoy/(double)(iTot),Timax,dNbMachmin,dNbMachmoy/(double)(iTot2),dNbMachmax);
 fclose(fichier);
 }
}


void MakeComparationMatrix(int IdSce, int NbTache, int NbMach, int NbIter)
{
	static bool printHeader = true;
	
	int NbResE = 0, 
		NbResH = 0,
		NbOptE = 0,
		NbOptH = 0,
		NbMemLimitE = 0,
		NbMemLimitH = 0,
		NbTimeLimitE = 0,
		NbTimeLimitH = 0;
		//NbAffinityLimitH = 0;
	double TMinE,TMinH, TMaxE, TMaxH,
		   DevMin, DevMax;
	double TTotalE = 0,
		   TTotalH = 0,
		   DevTotal = 0;
	int NbInstanceDev = 0; //Nb d'instance qui participe au calcul de d�viation

	TMinE = TMinH = DevMin = std::numeric_limits<double>::max();
	TMaxE = TMaxH = DevMax = std::numeric_limits<double>::min();
	DevTotal = 0;
	for(int i=0; i<NbIter; i++)
	{
		//Exact
		if(ScRes[0][i][0] == 1) 
		{
			NbResE++; 
			if(ScRes[0][i][1] == 1) 
			{
				NbOptE++;
			}
			else //si feasable mais pas optimal
			{
				NbMemLimitE += ScRes[0][i][5];
				NbTimeLimitE += ScRes[0][i][6];
			}
		}
		TTotalE += ScRes[0][i][3];
		if(ScRes[0][i][3]<TMinE) TMinE = ScRes[0][i][3];
		if(ScRes[0][i][3]>TMaxE) TMaxE = ScRes[0][i][3];		

		//H2
		if(ScRes[1][i][0] == 1) 
		{
			NbResH++; 
			if(ScRes[1][i][1] == 1) 
			{
				NbOptH++;
			}
			else //si feasable mais pas optimal
			{
				NbMemLimitH += ScRes[1][i][5];
				NbTimeLimitH += ScRes[1][i][6];
			}
		}

		TTotalH += ScRes[1][i][3];
		if(ScRes[1][i][3]<TMinH) TMinH = ScRes[1][i][3];
		if(ScRes[1][i][3]>TMaxH) TMaxH = ScRes[1][i][3];

		///We calculate the deviation only if they have both found a solution. DONC il faut penser � �a quand je calcule DevAvg! B�te.
		if(ScRes[0][i][0] && ScRes[1][i][0])
		{
			NbInstanceDev ++;
			double dev = (ScRes[1][i][2]-ScRes[0][i][2])/ScRes[0][i][2];
			DevTotal += dev;
			if(dev < DevMin) DevMin = dev;
			if(dev>DevMax) DevMax = dev;
		}
	}

	FILE* fRes = fopen("FullLog.csv","at");
	FILE* fichier=fopen("ComparationMatrix.csv","at");
	if(printHeader)
	{
		printHeader = false;
		//fprintf(fichier,"Number of instance for each scenario: %d\n", NbIter);
		fprintf(fRes,"Scenario(N/M); isFea(E); isFea(H); isOpt(E); isOpt(H); sol(E); sol(H); time(E); time(H); nbMach(E); nbMach(H);  nbMemLimit(E); nbMemLimit(H);  nbTimeLimit(E); nbTimeLimit(H);  CPUtime(E); CPUtime(H)\n");
		fprintf(fichier,"Scenario(N/M); Infeasable  ;Solved(E/H) ;MemLim(E) ;TimeLim(E) ;TMin(E/H)   ;TAvg(E/H)     ;TMax(E/H)     ;DevMin  ;DevAvg  ;DevMax\n");
	}
	fprintf(fichier,"Sc%d(%d/%d)      ;%d         ;%d/%d         ;%d          ;%d      ;%3.2lf/%3.2lf   ;%3.2lf/%3.2lf     ;%3.2lf/%3.2lf ;",
		IdSce+1, NbTache, NbMach, iterations - NbResE, NbResE, NbResH, NbMemLimitE, NbTimeLimitE, TMinE, TMinH, TTotalE/NbIter, TTotalH/NbIter, TMaxE, TMaxH);
	if(NbResH != 0)
		fprintf(fichier, "     %3.2lf\t  ;%3.2lf\t  ;%3.2lf\n", DevMin, DevTotal/NbInstanceDev, DevMax);
	else fprintf(fichier, "      *      ;*      ;*\n");
	fclose(fichier);

	//Export all results
	for(int i = 0; i<NbIter; i++)
	{
		fprintf(fRes,"Sc%d(%d/%d)      ;%d     ;%d     ;%d    ;%d      ;%.2lf       ;%.2lf     ;%3.2lf     ;%3.2lf   ;%3.2lf     ;%3.2lf     ;%3.2lf      ;%3.2lf       ;%3.2lf      ;%3.2lf     ;%3.2lf      ;%3.2lf\n",
		IdSce+1, NbTache, NbMach,         
		int(ScRes[0][i][0]), 
		int(ScRes[1][i][0]), 
		int(ScRes[0][i][1]), 
		int(ScRes[1][i][1]), 
		ScRes[0][i][2], 
		ScRes[1][i][2], 
		ScRes[0][i][3], 
		ScRes[1][i][3], 
		ScRes[0][i][4], 
		ScRes[1][i][4], 
		ScRes[0][i][5], 
		ScRes[1][i][5], 
		ScRes[0][i][6],
		ScRes[1][i][6],
		ScRes[0][i][7],
		ScRes[1][i][7]
		);
	}
	fclose(fRes);
}