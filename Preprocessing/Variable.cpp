#include "Variable.h"


Variable::Variable(void)
{
	VARName=new char[20];
	VARName="";
	VARRedCost=0;
	VARLj=0;
	VARUj=0;
	VARValue=-1;
	VARBase=-1;
	VARExtractable = true;
}


void Variable::VAROutput()
{
	printf("%s\t%.2lf\t%.2lf\t%.2lf\t%.2lf\t%d\n",VARName,VARRedCost,VARLj,VARUj,VARValue,VARBase);
}