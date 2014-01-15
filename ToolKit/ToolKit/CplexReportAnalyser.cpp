///
///Functions used to extract informations from the Cplex report
///

#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "TinyLog.h"
using namespace std;
TinyLog debug(TinyLog::DEBUG);

///@brief Analyse the cplex report file and extract time/gap pairs for further analyse of the best gap value.
///@param sLogFileName the log file name
///@param sOutFile the output file name, containing the extracted information
void ExtractTimeElapsedGapFromLog(string sLogFileName, string sOutFile)
{
	//A vector which contain for each instance of problme a vector of time/gap pair
	vector<vector< pair<double, double> >> vResult;
	int iTimeThreshold = 180; //We only consider instances whose duration is greater tha 180s
	string sBegin = "Root relaxation solution time";
	string sEnd = "GUB cover cuts applied";
	string sTarget = "Elapsed time = ";
	size_t lenBegin = sBegin.size();
	size_t lenEnd = sEnd.size();
	size_t lenTarget = sTarget.size();

	//Begin analyse
	ifstream LogReader(sLogFileName);
	if(!LogReader.good())
	{
		cerr<<"Log file input err"<<endl;
		return;
	}

	char buf[2][100];
	char cIndice = 0;
	bool isBeginReached = false;
	bool isEndReached = false;

	while(true) //Pour chaque instance
	{
		vector< pair<double, double> > vInstance;
		isBeginReached = false;
		//find the beginning of instance
		debug<<"Looking for the beginning...\n";
		while(true)
		{
			cIndice ^= char(1);//buffer change
			LogReader.getline(buf[cIndice], 100);
			if(LogReader.eof()) break;
			if(!sBegin.compare(0, lenBegin, buf[cIndice],lenBegin))
			{
				debug<<"Beginning found...\n";
				isBeginReached =true;
				break;
			}
		}
		
		if(isBeginReached == false) break; //Stop

		//begin instance
		isEndReached = false;
		while(true)
		{
			cIndice ^= char(1);//buffer change
			LogReader.getline(buf[cIndice], 100);
			if(LogReader.eof() ) break;
			//EOF ?
			if(!sEnd.compare(0, lenEnd, buf[cIndice],lenEnd))
			{
				isEndReached =true;
				if(vInstance.size()>0 && vInstance.back().first > iTimeThreshold)
					vResult.push_back(vInstance);
				break;
			}
			//value pair?
			if(!sTarget.compare(0, lenTarget, buf[cIndice],lenTarget))
			{
				
				double dTime = -1;
				dTime = strtod(buf[cIndice]+lenTarget, NULL);
				cIndice ^= char(1);
				double dGap = -1;
				//We assume the gap value is at the end of the line and has 7 chars
				dGap = strtod(buf[cIndice]+ strlen(buf[cIndice]) - 7, NULL);
				vInstance.push_back(make_pair<double, double>(dTime, dGap));
				debug<<"Value pair:"<<dTime<<","<<dGap<<"\n";
			}
		}

		if(isEndReached == false) break; //Stop
	}

	//Now we got the result vector. Output to csv file.
	ofstream Out(sOutFile);
	vector< vector< pair<double, double> >>::iterator itResult;
	vector< pair<double, double> >::iterator itInstance;
	int counter=0;
	for(itResult=vResult.begin(); itResult!=vResult.end(); itResult++)
	{
		Out<<"#Instance "<<++counter<<endl;
		for(itInstance = itResult->begin(); itInstance!= itResult->end(); itInstance++)
		{
			Out<<itInstance->first<<" ; "<<itInstance->second<<endl;
		}
	}

}