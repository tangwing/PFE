///Some functions for test purpose
///Author: SHANG Lei - shang.france@gmail.com
#ifndef TEST_H
#define TEST_H
#include <Windows.h>
#include <WinNT.h>
#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iterator>
#include <iomanip>
#include "Data.h"
#include "Traitement.h"
#include "ConsoleTable.h"

#define CACHER_TOUT true
#define AFFICHER_AFFINITE true
#define AFFICHER_MACHINE true
#define AFFICHER_ARC_DISPO true
#define AFFICHER_INTERVALLE true
#define AFFICHER_SERVEUR_BIS true
#define AFFICHER_LISTS_TACHES false
#define AFFICHER_ORDO true
#define AFFICHER_RT true
 
//HANDLE consolehwnd = GetStdHandle(STD_OUTPUT_HANDLE);

//================= Fonctions pour l'affichage ====================
extern void AfficherIntervalle();
extern void AfficherCaracMachine();
extern void AfficherListeServeurBis();
extern void AfficherRt();
extern void AfficherAffinite();
extern void AfficherEdgeDispo();
extern void AfficherOrdo();
extern void AfficherListesTache(int indice);


//===================== Fonctions pour tests ===================
template <typename T>
void PrintArrayPrio(T* arr, int size)
{
	for(int i=0; i<size; i++)std::cout<<arr[i].prio<<", ";
	std::cout<<std::endl;
}

extern void InitPrio(HDDRAM &hr);

template <typename T>
void TestOrderByPrio(T* arr, int size, bool isDecreased = true)
{
	for(int i=0; i<size-1; i++)
	{
		if(isDecreased) 
			assert(arr[i].prio>=arr[i+1].prio);
		else 
			assert(arr[i].prio<=arr[i+1].prio);
	}
}
extern void Test();
#endif