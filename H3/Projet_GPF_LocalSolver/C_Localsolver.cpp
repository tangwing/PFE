#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include "localsolver.h"
#include "Data.h"

using namespace std;
using namespace localsolver;
#define EPSILON 100000

class C_Localsolver
{
     public:
		 
		 /*
		 C_scp(const C_scp &);
		 ~C_scp();*/
		 
		 LocalSolver localsolver;
		 lsint ValueTC;
		 std::vector<localsolver::LSExpression*> LS_Zt;
		 std::vector<std::vector<localsolver::LSExpression*> >LS_Z;
		 std::vector<std::vector<localsolver::LSExpression*> > LS_D;
		 std::vector<std::vector <std::vector<localsolver::LSExpression*> > > LS_X;
		 std::vector<std::vector<std::vector<std::vector<std::vector<localsolver::LSExpression*> > > > > LS_Y;
		 


void  C_Localsolver::solve(int limit)
{
	int i,iprime;
	int j,jprime;
	int t,k,tt;
	int t1,t2,tprime;
	int var;

	try
	{
		LSModel & model = *(localsolver.getModel());
		/***********************************************Debut de la declaration des variables de decisions ***********************************/

		/******************************************************************************************************************
		                                           |Variable X(i)(j)(t)
		                                           |prend 0 ou 1
		**************************************************************************************************************/
/** initialisation de Xijt***/

		LS_X.resize(N());
		for (i = 0; i < N(); i++)
			LS_X[i].resize(M());
		
		for (i = 0; i < N(); i++) 
		{
			for(j=0; j< M();j++)
			{
				LS_X[i][j].resize(T());
			}
		}

		/** difinition da la variable expression Xijt***/
		for (i = 0; i < N(); i++) 
		{
			for(j=0; j< M();j++)
			{
				for(t=0;t<T();t++)
				{
					LS_X[i][j][t]=model.createExpression(O_Bool);
					stringstream s;
                     s << "LS_X[" << i << "]" << "[" << j << "]" << "[" << t << "]";
                    LS_X[i][j][t]->setName(s.str());
				}
			}
		}
		

		/*************************************************************************************************************************
		                                                |Variable Z(t)(j)
		                                                |prend 0 ou 1
		***************************************************************************************************************************/
/**** initialisation de Ztj***/

		LS_Z.resize(T());
		for (t = 0; t < T(); t++) 
		  LS_Z[t].resize(M());

		/** difinition da la variable expression Ztj***/
		for (t = 0; t < T(); t++) 
		{
			for(j=0; j< M();j++)
			{
				   LS_Z[t][j]=model.createExpression(O_Bool);
					stringstream s;
                     s << "SCP_LS_Z[" << t << "]" << "[" << j << "]";
                    LS_Z[t][j]->setName(s.str());
			}
		}
		
		 /*****************************************************************************************************************************
		                                                   |Variable Zt >= 0
		********************************************************************************************************************************/
           LS_Zt.resize(T());
		   for(t=0;t<T();t++)
		   {
			       LS_Zt[t]=model.createExpression(O_Sum);
			       stringstream s;
                   s << "SCP_LS_Z[" << t << "]";
                   LS_Zt[t]->setName(s.str());

				   for(j=0;j<M();j++)
					    LS_Zt[t]->addOperand(LS_Z[t][j]);
		   }
		/***********************************************************************************************************************
		                                                  |Variable Y(i)(i')(j)(j')(t)
		                                                  |prend 0 ou 1
		**************************************************************************************************************************/
/** initialisation de Yii'jj't***/

		LS_Y.resize(N());
		/**2D*/
		for (i = 0; i < N(); i++) 
		     LS_Y[i].resize(N());
		/**3D***/
		for (i = 0; i < N(); i++) 
		{
			for(iprime=0; iprime< N();iprime++)
				LS_Y[i][iprime].resize(M());
		}
		/***4D***/
		for (i = 0; i < N(); i++) 
		{
			for(iprime=0; iprime< N();iprime++)
			{
				for(j=0;j<M();j++)
					LS_Y[i][iprime][j].resize(M());
			}
		}
		/****5D***/
		for (i = 0; i < N(); i++) 
		{
			for(iprime=0; iprime< N();iprime++)
			{
				for(j=0;j<M();j++)
				{
					for(jprime=0;jprime<M();jprime++)
						LS_Y[i][iprime][j][jprime].resize(T());
				}
			}
		}
		/*** definition de la variable Yii'jj't ***/
		for (i = 0; i < N(); i++) 
		{
			for(iprime=0; iprime< N();iprime++)
			{
				for(j=0;j<M();j++)
				{
					for(jprime=0;jprime<M();jprime++)
					{
						for(t=0;t<T();t++)
						{
							LS_Y[i][iprime][j][jprime][t]=model.createExpression(O_Bool);
					        stringstream s;
							s << "LS_Y[" << i << "]" << "[" << iprime << "]" << "[" << j << "]" << "[" << jprime << "]" << "[" << t << "]";
                            LS_Y[i][iprime][j][jprime][t]->setName(s.str());
						}
					}
				}
			}
		}
		
		 /********************************************************************************************************
		***********************************************************************************************************
		                                        variable Dit >= 0 

		 **********************************************************************************************************
		**********************************************************************************************************/
/**** initialisation de Dit***/

		LS_D.resize(N());
		for (i = 0; i < N(); i++) 
		   LS_D[i].resize(T());

		/** difinition da la variable expression Dit***/
		for (i = 0; i < N(); i++)
		{
			for (t = 0; t < T(); t++) 
			{
				    LS_D[i][t]=model.createExpression(O_Bool);
					stringstream s;
                     s << "LS_D[" << i << "]" << "[" << t << "]";
                     LS_D[i][t]->setName(s.str());
					
			}
		}

		/********************************************************************************************************
		***********************************************************************************************************
		                                           (A)
		 constraint SUM(i in [1....N]) nc(i)*x(i)(j)(t) <= mc(j)      qqs t  in [1....T] , j in [1......M]   

		 **********************************************************************************************************
		**********************************************************************************************************/
        for(t=0;t<T();t++)
		{
			for(j=0;j<M();j++)
			{
				 LSExpression* CPUSum = model.createExpression(O_Sum);
                 for (i = 0; i < N(); i++) 
				 {
                      LSExpression* instance = model.createExpression(O_Prod,LS_X[i][j][t],(lsint)nc(i));
                      CPUSum->addOperand(instance);
                }    
                LSExpression* constraint_A = model.createExpression(O_Leq, CPUSum, (lsint)mc(j));
                model.addConstraint(constraint_A);
			}
		}

		/********************************************************************************************************
		***********************************************************************************************************
		                                           (B)
		  constraint SUM (i in [1....N]) ng(i)*x(i)(j)(t) <= mg(j)      qqs t  in [1....T] , j in [1......M]  

		**********************************************************************************************************
		**********************************************************************************************************/
        for(t=0;t<T();t++)
		{
			for(j=0;j<M();j++)
			{
				 LSExpression* GPUSum = model.createExpression(O_Sum);
                 for (i = 0; i < N(); i++) 
				 {
                      LSExpression* instance = model.createExpression(O_Prod,LS_X[i][j][t],(lsint)nc(i));
                      GPUSum->addOperand(instance);
                }    
                LSExpression* constraint_B = model.createExpression(O_Leq, GPUSum, (lsint)mc(j));
                model.addConstraint(constraint_B);
			}
		}
		/********************************************************************************************************
		***********************************************************************************************************
		                                           (C)
		 constraint SUM (i in [1....N]) nh(i)*x(i)(j)(t) + SUM (k in [1....m] avec k#j) nh(i)*y(i)(i)(k)(j)(t)  <= mh(j) qqs t  in [1....T] , j in [1......M]  

		**********************************************************************************************************
		**********************************************************************************************************/
        for(t=0;t<T();t++)
		{
			for(j=0;j<M();j++)
			{
				LSExpression* RAM_Side1_Sum = model.createExpression(O_Sum);
				//LSExpression* RAM_Side2_Sum = model.createExpression(O_Sum);

                 for (i = 0; i < N(); i++) 
				 {
                      LSExpression* instance = model.createExpression(O_Prod,LS_X[i][j][t],(lsint)nh(i));
                      RAM_Side1_Sum->addOperand(instance);
					  for(k=0;k<M();k++)
					  {
						  if(k!=j)
						  {
							  instance= model.createExpression(O_Prod,LS_Y[i][i][k][j][t],(lsint)nh(i));
							  RAM_Side1_Sum->addOperand(instance);
						  }
                         
					  }
                }  
               // LSExpression* RAMSUM=model.createExpression(O_Sum,RAM_Side1_Sum,RAM_Side2_Sum);

				 LSExpression* constraint_C = model.createExpression(O_Leq, RAM_Side1_Sum, (lsint)mh(j));
                 model.addConstraint(constraint_C);
			}
		}
		/********************************************************************************************************
		***********************************************************************************************************
		                                           (D)
		  constraint SUM (i in [1....N]) nr(i)*x(i)(j)(t) + SUM (k in [1....m] avec k#j) nr(i)*y(i)(i)(k)(j)(t)  <= mr(j) qqs t  in [1....T] , j in [1......M]

		**********************************************************************************************************
		**********************************************************************************************************/
            for(t=0;t<T();t++)
		    {
			     for(j=0;j<M();j++)
			     {
				     LSExpression* ROM_Side1_Sum = model.createExpression(O_Sum);
				     //LSExpression* ROM_Side2_Sum = model.createExpression(O_Sum);
                     for (i = 0; i < N(); i++) 
				     {
                         LSExpression* instance = model.createExpression(O_Prod,LS_X[i][j][t],(lsint)nr(i));
                         ROM_Side1_Sum->addOperand(instance);
					     for(k=0;k<M();k++)
					     {
						     if(k!=j)
						     {
							     instance= model.createExpression(O_Prod,LS_Y[i][i][k][j][t],(lsint)nh(i));
							     ROM_Side1_Sum->addOperand(instance);
							 }
					     }
                     }  
                    // LSExpression* ROMSUM=model.createExpression(O_Sum,ROM_Side1_Sum,ROM_Side2_Sum);

				     LSExpression* constraint_D = model.createExpression(O_Leq, ROM_Side1_Sum, (lsint)mr(j));
                     model.addConstraint(constraint_D);
			     }
		    }
		/********************************************************************************************************
		***********************************************************************************************************
		                                           (E)
		  constraint x(i)(j)(t) +  x(i')(j')(t) - 1 <= y(i)(i')(j)(j')(t)
		  qqs t  in [1....T] ,i i' in [1...N] telque a(i,i')=1 et i#i' ,j j' in [1......M] telque j#j'

		**********************************************************************************************************
		**********************************************************************************************************/
            for(t=0;t<T();t++)
		    {
			     for (i = 0; i < N(); i++) 
		         {

			         for(iprime=i+1; iprime< N();iprime++)
			         {
					     if(a(i,iprime)==1)
					     {
						     for(j=0;j<M();j++)
				             {
					             for(jprime=j+1;jprime<M();jprime++)
					             {
								     LSExpression* instance = model.createExpression(O_Sum,LS_X[i][j][t],LS_X[iprime][jprime][t]);

									 LSExpression* instance1 = model.createExpression(O_Sum,LS_Y[i][iprime][j][jprime][t],(lsint)1);

								     LSExpression* constraint_E = model.createExpression(O_Leq, instance,instance1);
                                     model.addConstraint(constraint_E);
							     }
						     }
					     }
				     }
			      }
		      }
		/********************************************************************************************************
		***********************************************************************************************************
		                                           (F)
		  constraint x(i)(j)(t1) +  x(i)(j')(t2) - 1 + SUM k in(1...M),t' in (t1+1....t2-1) x(i)(k)(t') <= y(i)(i)(j)(j')(t) 
		  qqs t  in [1....T] ,i in [1...N] ,j j' in [1......M] telque j#j',t1 in [t.....min(t+mt(i) , T)]
		       t2 in [t1+1......T]
		**********************************************************************************************************
		**********************************************************************************************************/
		
            for (i = 0; i < N(); i++) 
	        {
			    for(t=mt(i);t<T();t++)
			    {
				    for(t1=t+1; t1 < std::min((t+mt(i)),T());t1++)
				    {
					    for(j=0;j<M();j++)
					    {
							    for(jprime=0;jprime<M();jprime++)
							    {
								    if(j!=jprime)
								    {
									     
								         LSExpression * instance=model.createExpression(O_Sum,LS_X[i][j][t1-1],LS_X[i][jprime][t1]);
									     LSExpression * instance1=model.createExpression(O_Sum,LS_Y[i][i][j][jprime][t],(lsint)1);

							             LSExpression* constraint_F = model.createExpression(O_Leq, instance ,instance1 );
                                         model.addConstraint(constraint_F);
								    }
								
							    }
						    }
					     }
				     }
		     }

		/* try
		{
            for (i = 0; i < N(); i++) 
	        {
			    for(t=0;t<T()-mt(i);t++)
			    {
				    for(t1=t; t1 < std::min((int)(t+mt(i)),(int)T());t1++)
				    {
					    for(j=0;j<M();j++)
					    {
						    for(t2=t1+1 ; t2<T() ;t2++)
						    {
							    for(jprime=0;jprime<M();jprime++)
							    {
								    if(j!=jprime)
								    {
									     LSExpression* bool_sum =model.createExpression(O_Sum);
								         for(k=0; k<M() ; k++)
								         {
									          for(tprime=t1+1;tprime<t2;tprime++)
										      {
										          bool_sum->addOperand(LS_X[i][k][tprime]);
										      }      
								         }
								    

								         LSExpression * instance=model.createExpression(O_Sum,LS_X[i][j][t1],LS_X[i][jprime][t2]);
									     LSExpression * instance1=model.createExpression(O_Sum,bool_sum,(lsint)1);

									     LSExpression * side1  =model.createExpression(O_Dist,instance,instance1);

							             LSExpression* constraint_F = model.createExpression(O_Leq, side1 ,LS_Y[i][i][j][jprime][t]);
                                         model.addConstraint(constraint_F);
								    }
								
							    }
						    }
					     }
				     }
			     }
		     }
		}
		 catch(LSException *e)
	     {
			    cout <<"in F" << endl;
		        cout << "LSException:" << e->getMessage() << std::endl;
                exit(1);
	    }*/
		/********************************************************************************************************
		***********************************************************************************************************
		                                           (F')
		constraint mt[x(i)(j)(t1) +  x(i)(j')(t2) - 1 + SUM k in(1...M),t' in (t1+1....t2-1) x(i)(k)(t')] <= Sum t in[min(t+1-mt(i) , 0)...t) X(i)(j)(t)
		qqs t  in [1....T] ,i in [1...N] ,j j' in [1......M] telque j#j',t1 in [t.....min(t+mt(i) , T)]  t2 in [t1+1......T]

		**********************************************************************************************************
		**********************************************************************************************************/
            for (i = 0; i < N(); i++) 
	        {
			    for(t=0;t<(T()- mt(i));t++)
			    {
					    for(j=0;j<M();j++)
					    {
							    for(jprime=0;jprime<M();jprime++)
							    {
								    if(j!=jprime)
								    {
									     
								         LSExpression * instance=model.createExpression(O_Sum,LS_X[i][j][t],LS_X[i][jprime][t+1]);
									     LSExpression * side2=model.createExpression(O_Prod,instance,(lsint)mt(i));

										 LSExpression * side1  =model.createExpression(O_Sum);
										 for(t1=t+1;t1<(t+mt(i));t1++)
										      side1->addOperand(LS_X[i][jprime][t1]);

										 side1->addOperand((lsint)mt(i));

									     

							             LSExpression* constraint_F1 = model.createExpression(O_Leq, side2 ,side1);
                                         model.addConstraint(constraint_F1);
								    }
								
							    }
						    }
			     }
		     }
		/********************************************************************************************************
		***********************************************************************************************************
		                                           (G)
		constraint SUM j in(1...M) x(i)(j)(t)] = u(i)(t)
		qqs i in [1...N] telque R(i)=0 t  in [1....T] 

		**********************************************************************************************************
		**********************************************************************************************************/
             for (i = 0; i < N(); i++) 
	         {
			      if(R(i)==0)
			      {
				       for(t=0;t<T();t++)
			           {
                              LSExpression* bool_sum =model.createExpression(O_Sum);
					          for(j=0; j<M() ; j++)
					          {
					                 bool_sum->addOperand(LS_X[i][j][t]);
					          }

					          LSExpression* constraint_G = model.createExpression(O_Eq, bool_sum ,(lsint)u(i,t));
                              model.addConstraint(constraint_G);			    
			           }
			      }
		      }
		 /********************************************************************************************************
		***********************************************************************************************************
		                                           (H)
		constraint SUM j in(1...M) x(i)(j)(t)] <= u(i)(t)
		qqs i in [1...N] telque R(i)=1 , t  in [1....T]

		**********************************************************************************************************
		**********************************************************************************************************/
             for (i = 0; i < N(); i++) 
	         {
			     if(R(i)==1)
			     {
				     for(t=0;t<T();t++)
			         {
                        LSExpression* bool_sum =model.createExpression(O_Sum);
					    for(j=0; j<M() ; j++)
					    {
					       bool_sum->addOperand(LS_X[i][j][t]);
					    }

					    LSExpression* constraint_H = model.createExpression(O_Leq, bool_sum ,(lsint)u(i,t));
                        model.addConstraint(constraint_H);			    
			         }
			     }
		      }
		 /********************************************************************************************************
		***********************************************************************************************************
		                                           (I)
		constraint  x(i)(j)(t)] <= U(i,t) * Q(i,t)
		qqs i in [1...N]  , t  in [1....T] , j in [1...M]

		**********************************************************************************************************
		**********************************************************************************************************/
             for (i = 0; i < N(); i++) 
	         {
			     for(t=0;t<T();t++)
			     {
					for(j=0; j<M() ; j++)
					{
					    
						 LSExpression* constraint_I = model.createExpression(O_Leq, LS_X[i][j][t],(lsint)(u(i,t) * q(i,j)));
                         model.addConstraint(constraint_I);	
					}		    
			    }
	         }
		  /********************************************************************************************************
		***********************************************************************************************************
		                                           (J)
		

		**********************************************************************************************************
		**********************************************************************************************************/
         for(t=0;t<T();t++)
		 {
			 for(k=0;k<NbEdges();k++)
			 {
				 LSExpression* side1 =model.createExpression(O_Sum);
				 unsigned int iMachj,iMachjp,iSwap;
				 for(j=0;j<NbMachEdge(k);j++)
				 {
					    CoupleMachines(k,j,iMachj,iMachjp);
						if (iMachj>iMachjp) 
							{ iSwap=iMachj; iMachj=iMachjp; iMachjp=iSwap;}

						for (i=0; i<N(); i++)
							for (iprime=i+1; iprime<N(); iprime++)
								if (a(i,iprime)==1)
								{
									LSExpression * instance=model.createExpression(O_Prod,LS_Y[i][iprime][iMachj][iMachjp][t],(lsint)b(i,iprime));
									side1->addOperand(instance);
								}

                        for (i=0; i<N(); i++)
						{
							LSExpression * instance=model.createExpression(O_Sum,LS_Y[i][i][iMachj][iMachjp][t],LS_Y[i][i][iMachjp][iMachj][t]);
							LSExpression * instance1=model.createExpression(O_Prod,instance,(lsint)b(i,i));

							side1->addOperand(instance1);
						}

				 }

				  LSExpression* constraint_J = model.createExpression(O_Leq, side1,(lsint)maxb());
                  model.addConstraint(constraint_J);
			 }
		 }
		  /********************************************************************************************************
		***********************************************************************************************************
		                                           (K)
		constraint  x(i)(j)(t)] <= Z(t,j)
		qqs i in [1...N]  , t  in [1....T] , j in [1...M]

		**********************************************************************************************************
		**********************************************************************************************************/
           for (i = 0; i < N(); i++) 
	       {
			 for(t=0;t<T();t++)
			  {
					for(j=0; j<M() ; j++)
					{
					    
						 LSExpression* constraint_K = model.createExpression(O_Leq, LS_X[i][j][t],LS_Z[t][j]);
                         model.addConstraint(constraint_K);	
					}		    
			  }
	       }
		/*****************************************************************************************************************************
		                                                 
		                                    contraint L
            SUM j in (1..M)Ztj = Zt   qqs t in (1....T) 
		********************************************************************************************************************************/
		   for(t=0;t<T();t++)
		   {
			    LSExpression* boolsum=model.createExpression(O_Sum);
			    for(j=0; j< M();j++)
		        {
				     boolsum->addOperand(LS_Z[t][j]);
		        }
				LSExpression * side1  =model.createExpression(O_Dist,LS_Zt[t],boolsum);
			   LSExpression* constraint_L = model.createExpression(O_Eq, side1, (lsint)0);
               model.addConstraint(constraint_L);
		   }
		/********************************************************************************************************
		***********************************************************************************************************
		                                           (M)
		constraint  x(i)(j)(t2) <= 1 - x(i)(j)(t1) + x(i)(j)(t1+1)
		qqs i in [1...N] telque Ri=1  , t1  in [1....(T-rt(i,j))] , j in [1...M] , t2 in [t1+1 ......(t1+rt(i,j))]

		**********************************************************************************************************
		**********************************************************************************************************/
			for (i = 0; i < N(); i++) 
	        {
			 if(R(i)==1)
			 {
				 for(j=0; j<M() ; j++)
			     {
					 for(t1=0; t1 < (T()-rt(i,j));t1++)
				     {
						 for(t2=t1+1 ; t2<(t1+rt(i,j)) ;t2++)
						 {
							 LSExpression* bool_sum =model.createExpression(O_Sum,LS_X[i][j][t1],LS_X[i][j][t2]);

							 LSExpression* side1=model.createExpression(O_Dist,bool_sum,LS_X[i][j][t1+1]);

							 LSExpression* constraint_M = model.createExpression(O_Leq, side1 ,(lsint)1);
                             model.addConstraint(constraint_M);
						 }
					 }
			     }
			  }
			}
		 /********************************************************************************************************
		***********************************************************************************************************
		                                           (N)

		**********************************************************************************************************
		**********************************************************************************************************/
		   /* for(i=0;i<N();i++)
			{
                    if(R(i)==1)
					{
						for(j=0;j<M();j++)
						{
                               for(t1=1;t1<T()-rt(i,j)-1;t1++)
							   {
									  for(t2=t1+rt(i,j)+1;t2<T();t2++)
									   {
											   LSExpression* instance = model.createExpression(O_Prod,LS_X[i][j][t1-1],(lsint)(t2-t1));
											   LSExpression* instance1 = model.createExpression(O_Prod,LS_X[i][j][t2],(lsint)(t2-t1)); 
											   LSExpression* side1 = model.createExpression(O_Sum,instance,instance1);
									
											   LSExpression* instance2= model.createExpression(O_Sum);
											   for (k=0;k<M();k++)
											   {
													 for (t=t1+1;t<t2-1;t++)
													 {
															  instance2->addOperand(LS_X[i][k][t]);
													 }
											   }
                                                 LSExpression* instance3=model.createExpression(O_Prod,instance2,(lsint)(t2-t1));
												 LSExpression* side2 = model.createExpression(O_Sum,instance3,LS_D[i][t1],(lsint)(t2-t1));

												LSExpression* constraint_N= model.createExpression(O_Leq,side1,side2);
												model.addConstraint(constraint_N);
										}
							   }
						}
						     
					}
                    
			}*/
			/********************************************************************************************************
		***********************************************************************************************************
		                                           (O)

		**********************************************************************************************************
		**********************************************************************************************************/
			/*for (i=0;i<N();i++)
               for (j=0;j<M();j++)
                  for (jprime=0;jprime<M();jprime++)
                       if (j!=jprime)
                            for (t1=0;t1<T()-mt(i)-1;t1++)
                            {
									LSExpression* instance = model.createExpression(O_Prod,LS_X[i][j][t1],(lsint)mt(i));
									LSExpression* instance1 = model.createExpression(O_Prod,LS_X[i][jprime][t1+1],(lsint)mt(i));

									LSExpression* side1 = model.createExpression(O_Sum,instance,instance1);

									LSExpression* side2 = model.createExpression(O_Sum,LS_D[i][t1],(lsint)mt(i));

									LSExpression* constraint_O = model.createExpression(O_Leq,side1,side2);
									model.addConstraint(constraint_O);
                             }*/

							/**********         RE            *******/
        /*LSExpression * ObjRE= model.createExpression(O_Sum);
		for(i=0;i<N();i++)
			for(t=0;t<T();t++)
				ObjRE->addOperand(LS_D[i][t]);

		LSExpression* constraint_RE = model.createExpression(O_Leq,ObjRE,(lsint)EPSILON);
		model.addConstraint(constraint_RE);*/
		/********************************************************************************************************
		***********************************************************************************************************
		                                           
		                                    Fonction objectif TC

		**********************************************************************************************************
		**********************************************************************************************************/
		LSExpression* ObjTC = model.createExpression(O_Sum);
		for(t=0;t<T();t++)
		{
			for(i=0;i<N();i++)
			{
				for(j=0;j<M();j++)
				{
					LSExpression* instance = model.createExpression(O_Prod,LS_X[i][j][t],(lsint)( (alphac(j)*nc(i)) + (alphag(j)*ng(i)) + (alphah(j)*nh(i)) + (alphar(j)*nr(i)) ));
                      ObjTC->addOperand(instance);
				}
			}
		}

		for(t=0;t<T();t++)
		{
			for(i=0;i<N();i++)
			{
				LSExpression* sumXijt = model.createExpression(O_Sum);
				for(j=0;j<M();j++)
				{
					sumXijt->addOperand(LS_X[i][j][t]);
				}
				LSExpression* temp1= model.createExpression(O_Dist,(lsint)1,sumXijt);
				LSExpression* temp2=  model.createExpression(O_Prod,temp1,(lsint)(rho(i)*u(i,t)));
				ObjTC->addOperand(temp2);
			}
		}

		for(t=0;t<T();t++)
		{

			LSExpression* instance = model.createExpression(O_Prod,LS_Zt[t],(lsint)beta(t));
			ObjTC->addOperand(instance);
		}

		for(t=0;t<T();t++)
		{
			for(i=0;i<N();i++)
			{
				for(j=0;j<M();j++)
				{
					for(k=0;k<M();k++)
					{
						if(j!=k)
						{
							LSExpression* instance = model.createExpression(O_Prod,LS_Y[i][i][j][k][t],(lsint)((alphah(k)*nh(i)) + (alphar(k)*nr(i))));
			                ObjTC->addOperand(instance);
						}
					}
				}
			}

		}
		model.addObjective(ObjTC, OD_Minimize);
		
		model.close();
		 
		/********************************************************************************************/

        
		   
          LSPhase*  phase0 = localsolver.createPhase();
          phase0->setTimeLimit(limit);

		  /*LSPhase* phase1 = localsolver.createPhase();
           phase1->setTimeLimit(limit);*/

		  LSParam* lsParam = localsolver.getParam();
          lsParam->setNbThreads(3);
		  //lsParam->setObjectiveBound(0,(lsint)22000);
   
	     
          localsolver.solve();

		      LSSolution * temp;
		      temp=localsolver.getSolution();
              ValueTC=temp->getValue(ObjTC);
		 }
	      catch(LSException *e)
	      {
		      cout << "in solve " << endl;     
			  cout << "LSException:" << e->getMessage() << std::endl;
		           exit(1);
	      }
}

void writeSolution() 
{
   FILE *SolFic;
   int iMach,iTask,iTime,iEdge;
  
   SolFic=fopen("solution_localsolver.txt","wt");
   fprintf(SolFic,"Optimal solution computed by Localsolver for an instance with:\n");
   fprintf(SolFic,"\t %d tasks\n\t %d machines\n\t Time horizon of %d\n\n",N(),M(),T());
   fprintf(SolFic,"Objectif TC optimal value %d \n",ValueTC);


   fprintf(SolFic,"\n Schedule per task and for each time unit\n");
 for (iTask=0;iTask<N();iTask++)
 {
	 fprintf(SolFic,"Task %d (isPreemptable=%d / migration time=%ld):\n",iTask+1,R(iTask),mt(iTask));
	 fprintf(SolFic,"  Resuming time per machine: ");
	 for (iMach=0;iMach<M();iMach++) fprintf(SolFic,"%d ",rt(iTask,iMach));
	 fprintf(SolFic,"\n");
	 fprintf(SolFic,"  Can be assigned only to machines: ");
	 for (iMach=0;iMach<M();iMach++) 
		 if (q(iTask,iMach)==1) fprintf(SolFic,"%d ",iMach+1);
	 fprintf(SolFic,"\n");
	 fprintf(SolFic,"\t is Processed at times (machine): ");
	 for (iTime=0;iTime<T();iTime++)
	 {
	  for (iMach=0;iMach<M();iMach++)
		  if (LS_X[iTask][iMach][iTime]->getValue()>0.999)
		  { // Task iTask is processed by iMach at time [iTime,iTime+1]
		   fprintf(SolFic,"%d (%d) ",iTime+1,iMach+1);
		  }
	  fprintf(SolFic,"| ");
	 }
	 fprintf(SolFic,"\n");
	 fprintf(SolFic,"\t is likely to be processed at : ");
	 for (iTime=0;iTime<T();iTime++)
		   fprintf(SolFic,"%d | ",u(iTask,iTime));
	 fprintf(SolFic,"\n");
 }
   fclose(SolFic);
    /*SolFic=fopen("Local_solver.txt","wt");
	 LSSolution * temp;
	 LSSolutionStatus  temp1;*/
	 

	//fprintf(SolFic,"Objectif TC optimal value %d \n",);

}

};

void main(void)
{
	
	/*GetData();
	cout<<"nombre de tache :" << N() <<endl;*/
	C_Localsolver model;
	GetData();
	model.solve(30);
	model.writeSolution();

}