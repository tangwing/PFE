#ifndef RGen
#define RGen 200

#pragma once

#include "data.h"

// Definition of the time horizon and time increment (in minutes)
#define TimeInMinutes 60
#define StepForDiscretization 5
#define MaxBandwidth 100
#define BandwidthMigration 10

// Definition of the scenarii for the instance generation
// Meaning of columns: N1 N2 N3 N4 N5 M1 M2 M3 M4
extern unsigned int ScNM [8][9];

// Definition of PMs' features
// Meaning of columns: CPU GPU RAM HDD vj ajc ajg ajr ajh
// One row per PM
extern unsigned int PMs [4][9];
extern unsigned int Beta_t; // Cost for turning on a machine at time t

// Definition of VMs' features
// Meaning of columns: CPU GPU RAM HDD Bandwidth
// One row per VM
extern unsigned int VMs [12][5];

// Pre-assignement of VMs to PMs
// One colum per PM / One row per VM
// 1: can be assigned to / O: otherwise
extern unsigned int PreAss [12][4];


/**********************************************************************************************/
// List of functions available to randomly generate an instance

extern void GenerateRandomInstance(int iN1,int iN2, int iN3, int iN4, int iN5, int iM1, int iM2, int iM3, int iM4, int iTH, int iStep);
	/* iN1: the number of Standard Vms,
	   iN2: the number of VMs with High Memory Requirement,
	   iN3: the number of VMs with High CPU requirement,
	   iN4: the number of Cluster Compute VMs,
	   iN5: the number of Cluster GPU VMs,
	   iM1: the number of D3 PMs,
	   iM2: the number of D51 PMs,
	   iM3: the number of D52 PMs,
	   iM4: the number of D53 PMs,
	   iTH: the time horizon,
	   iStep: the time increment for discretization*/

#endif
