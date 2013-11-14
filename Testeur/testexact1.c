#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include "RandGeneration.h"

#define DEBUG false

unsigned int iterations=20;

//Variables used to compute statistics
unsigned int uiIntSolv,uiMemLim,uiTimLim, Ndmin,Ndmax,InFeas,Solved;
double Ndmoy, Timoy,Timin,Timax,dNbMachmin,dNbMachmoy,dNbMachmax;

void main(void)
{
 FILE *fichier;
 int i,j;
 int isFeas,isOpt,iOptValue,iNbNodes,iTot,iTot2;
 double dTime,dNbMach;

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
   printf("Data set n°%ld\n",j+1);
   
   GenerateRandomInstance(ScNM[i][0],ScNM[i][1],ScNM[i][2],ScNM[i][3],ScNM[i][4],ScNM[i][5],ScNM[i][6],ScNM[i][7],ScNM[i][8], 60, 5);

   if (DEBUG)
   {
	   printf("[DEBUG] The instance is generated: press a key to continue\n");
	   //getch();
   }
   printf("The IP model is running...\n");

   spawnl(P_WAIT,"SCPTimInd.exe","SCPTimInd.exe",NULL);

   fichier=fopen("SCPres.txt","rt");
   fscanf(fichier,"%ld\n",&isFeas);
   fscanf(fichier,"%ld\n",&isOpt);
   fscanf(fichier,"%ld\n",&iOptValue);
   fscanf(fichier,"%lf\n",&dTime);
   fscanf(fichier,"%ld\n",&iNbNodes);
   fscanf(fichier,"%lf\n",&dNbMach);
   fclose(fichier);

   fichier=fopen("ScpTraces.txt","at");
   fprintf(fichier,"Sc%d %d %d %d %lf %lf\n",i+1,isFeas,isOpt,iNbNodes,dTime,dNbMach);
   fclose(fichier);

   if (DEBUG)
   {
	printf("[DEBUG] Problem solved\n");
	getch();
   }

   if (isFeas==0) InFeas++;
   if (isFeas==1 && isOpt==0 && dTime>1798.0)  uiTimLim++;
   if (isFeas==1 && isOpt==0 && dTime<=1798.0) uiMemLim++;
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
  }
 
 fichier=fopen("ScpIPStats.txt","at");
 fprintf(fichier,"Sc%d %d %d %d %d %d %lf %d %3.2lf %3.2lf %3.2lf %2.2lf %2.2lf %2.2lf\n",i+1,InFeas,Solved,uiMemLim,uiTimLim,Ndmin,(double)Ndmoy/(double)iTot,Ndmax,Timin,Timoy/(double)(iTot),Timax,dNbMachmin,dNbMachmoy/(double)(iTot2),dNbMachmax);
 fclose(fichier);
 }
}
