#include "doublecompress.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


void Doublecompress_State_Alloc(Doublecompress_State * dcs, int Nlinac) {
  dcs->Ipk =    (double*)calloc(Nlinac,sizeof(double));
  dcs->sz =     (double*)calloc(Nlinac,sizeof(double));
  dcs->dE_E =   (double*)calloc(Nlinac,sizeof(double));
  dcs->sd =     (double*)calloc(Nlinac,sizeof(double));
  dcs->dt =     (double*)calloc(Nlinac,sizeof(double));
  dcs->sdsgn =  (double*)calloc(Nlinac,sizeof(double));
  dcs->k =      (double*)calloc(Nlinac,sizeof(double));
  dcs->Eloss =  (double*)calloc(Nlinac,sizeof(double));
  dcs->dE_Ei =  (double*)calloc(Nlinac,sizeof(double));
  dcs->dE_Ei2 = (double*)calloc(Nlinac,sizeof(double));
  dcs->cor =    (double*)calloc(Nlinac,sizeof(double));
}
void Doublecompress_State_Dealloc(Doublecompress_State * dcs) {
  free(dcs->Ipk);
  free(dcs->sz);
  free(dcs->dE_E);
  free(dcs->sd);
  free(dcs->dt);
  free(dcs->sdsgn);
  free(dcs->k);
  free(dcs->Eloss);
  free(dcs->dE_Ei);
  free(dcs->dE_Ei2);
  free(dcs->cor);
}

void Doublecompress_State_Attach(Doublecompress_State * dcs, int Nlinac,
				 double * payload)
{
  dcs->Ipk = payload;
  dcs->sz = payload + Nlinac;
  dcs->dE_E = payload + 2*Nlinac;
  dcs->sd = payload+ 3*Nlinac;
  dcs->dt = payload+ 4*Nlinac;
  dcs->sdsgn = payload + 5*Nlinac;
  dcs->Eloss = payload + 6*Nlinac;
  dcs->dE_Ei = payload + 7*Nlinac;
  dcs->dE_Ei2 = payload + 8*Nlinac;
  dcs->cor = payload + 9*Nlinac;

  // Payload better have been of size 10*Nlinac!

}

/*
	
  Function to calculate bunch length and energy spread after a 
  compressor system (just like LCLS) where the E-z correlations are generated
  by linacs at off crest rf phases (not zero crossing).  The wakefield is
  included in linear form assuming rectangular z-distributions.  The rms
  bunch lengths and energy spreads calculated here are linear in that they
  do not directly include the T566, or rf curvature non-linearities along the
  bunch.  The calculation does, however, include the E-z correlation dependence
  on incoming timing jitter (due to rf curvature) and charge jitter, and the
  T566 effect on R56 for mean-off-energy beams.  The bunch head is at z<0
  (same as LiTrack), so a chicane compressor has both R56 < 0 and phi < 0.
  Written by P. Emma
  
  Modifications to Octave:
  A reference for most equations used below is 'doc/presentations/PE_slides/COMP_OPT1.pdf'
  Note that there are a few known differences from the above mentioned reference. These
  differences are alright and have been marked as 'changed by author'.
  Deviations of the mean energy, bunch length, energy spread and chirp of the injected bunch
  were added to the input parameters and are taken into account in the computations.
  The sign of the charge is ignored.
  Nov. 2012, S. Paret
  
  Translated:
  Converted to C. New data stuctures added to encapsulate data. Units switched
  to SI units(excepetion: electron volts) 
  NOTE: {} is the length of the array eg. {Nlinacs} indicates input array should have length Nlinacs
  July 2013, Daniel Driver
  
  Inputs: gun.sz0:	Nominal initial rms bunch length [m]
          gun.sd0:	Nominal initial incoh. energy spread at Eg [fraction]
          gun.E:	Nominal gun exit energy [eV]
          gun.N:        Bunch population [e.g. 6.25E9]
  
	  linp_array[j].dE:	Energy difference of bunch from start to finish of linac j [eV]
	  linp_array[j].R56:	Nominal R56 values for linac j (chicane-R56 < 0) [m]
	  linp_array[j].T566:	Nominal T566 values for linac j (always > 0) [m]
	  linp_array[j].phi:	Nominal linac RF phase for linac j (-30 deg accelerates 
	  and puts head energy lower than tail) [radians]
	  linp_array[j].L:	linac length (scales wake) [m]
	  linp_array[j].lam:	RF wavelength for each linac 
	                        (Sband=0.105m, Xband=0.02625m) [m]
	  linp_array[j].s0:	Wakefield characteristic length (Sband=1.322mm), Xband=0.77mm) [m]
	  linp_array[j].a:	Mean iris radius (Sband=11.654mm,Xband=4.72mm) [m]
  
	  dN_Nf:  Relative bunch population error [fraction] 
	          (e.g. -2 => 2-low in bunch charge)
	  dtg:    Timing error of gun wrt RF (<0 is an early bunch) [sec]
	  dEg:    Energy deviation at end of injector [eV]
	  dsig_z: Deviation of bunch length from nominal length [m]
	  dsig_E: Deviation of energy spread from nominal energy 
	  spread [fraction of nominal energy]
	  chirp:  <Ez> correlation [m]
	  dphivr: Vector of 5 linac RF phase errors (<0 is early bunch arrival) [rad] {Nlinacs}
	  dV_Vvr: Vector of 5 linac RF relative voltage errors [fraction] {Nlinacs}
  
  Outputs: 
          Ipk:	Peak current at end of j-th linac [A] {Nlinacs}
	  sz:	rms bunch length after j-th linac [m] {Nlinacs}
	  dE_E:	Relative energy offset after j-th linac [fraction] {Nlinacs}
	  sd:	rms rel. energy spread after j-th linac [fraction] {Nlinacs}
	  dt:	Timing error at end of j-th linac [sec] {Nlinacs}
	  sdsgn:	Signed, correlated E-sprd per linac (dE_E-z slope * sigz) [fraction] {Nlinacs}
	  k:	<Ez> correlation const. of j-th linac [1/m] {Nlinacs}
	  Eloss:	Energy loss per linac due to wake [eV] {Nlinacs}
	  dE_Ei:	Relative energy offset of JUST j-th linac [fraction] {Nlinacs}
	  dE_Ei2:Energy offset error relative to final energy of JUST 
	  j-th linac [fraction] {Nlinacs}
	  cor:  Signed-correlated energy spread (slope*sigz) [fraction] {Nlinacs+1}

 */

#ifndef c
#define c 299792458.0
#endif
#ifndef mu0
#define mu0 4*M_PI*1e-7
#endif
#ifndef Z0 
#define Z0 120.0*M_PI
#endif
#ifndef e
#define e 1.60217656535E-19
#endif
 //mu0*c
//1.60217656535E-19
#define SQ(x) ((x)*(x))

void doublecompress_octave(Gun_Param * gun, Linac_Param ** linp_array,int Nlinac, 
		      //Inputs which change with time potentially
		      double dN_Nf, double dtg, double dEg,
		      double dsig_z, double  dsig_E, double chirp,
		      double * dphivr, double * dV_Vvr,
		      //outputs of doublecompress
		      double * Ipk, double * sz, double * dE_E,
		      double * sd , double * dt, double * sdsgn, 
		      double * k, double * Eloss, double * dE_Ei,
		      double * dE_Ei2, double * cor
		      )
{
  //e used in octave code
  double e_oct = 1.602177E-19;

  // stuff that gets updated with the linac here intial coditions are set
  double Eprev=gun->E; //Energy after leaving gun [ev]
  double szprev=gun->sz0+dsig_z; //rms bunch length [m] 
  double sdprev=gun->sd0+dsig_E; //incoh. energy spread [fraction]
  double dE_Eprev=dEg/gun->E;// relative energy error at start
  double dtprev=dtg; // timing error of prev linac NOT THE TIME STEP!!!!  
  double corprev=chirp;
  
  //crap just to reduce calculation and typeing
  Linac_Param * lin;
  double sqrt12=sqrt(12.0);
  
  double N = fabs(gun->Q)*6.241E18;//e; //particles from gun
  //double N = fabs(gun->Q)/e; //particles from gun
  
  double E; //Energy at the end of the current linac
  double Er;  //energy ratio
  double ds;  //FW bunch lenfht for uniform dist. [m]
  double dphi; //total local phase error (gun, prev-R56 + local-rf) [rad]
  double s0;  //Wakefield characteristic length [m]
  double lambar; // lam/2*pi
  double R56; //R56 value adjust for energy error
  double C; //temp for cos(phir)
  double kR561; double sd2; double sz2; 
  double Nec;  //intermediate in calc of kw
  double kw;  // /wake's effect on linear correlation factor (<0) [1/m]
  double kn;  //rf phase induced linear correlation factor [1/m]

  for(int j=0;j<Nlinac;j++){

    lin=linp_array[j]; //pullout current linac from pointer array
    E=(Eprev+lin->dE); // calculate the energy after linac
    Er=Eprev/E;  //calculate energy ratio
    ds=szprev*sqrt(12.0);
    lambar=lin->lam/2.0/M_PI;
    C = cos(lin->phi);

    dphi= dtprev*c/lambar + dphivr[j];
    s0=lin->s0;

    Nec=2.0*N*e_oct*s0*Z0*c/M_PI/SQ(lin->a);
    //Nec=2.0*fabs(gun->Q)*s0*Z0*c/M_PI/SQ(lin->a);
    
    //more physicsy stuff
    //wake's effect on linear correlation factor (<0) [1/m]
    kw= -(1.0+dN_Nf)*(Nec*lin->L/(SQ(ds)*E))*
      (1.0-(1.0+sqrt(ds/s0))*exp(-sqrt(ds/s0)));  

    // rf phase induced linear correlation factor [1/m]
    kn = (Er-1.0)*sin(lin->phi + dphi)/(lambar*C);
    k[j] = kw + kn;
    

    // relative energy error, but only individual
    // linac contribution (not original)
    //changed by Stefan Paret (introduced dV_Vvr)
    dE_Ei[j]    = (1.0-Er)*((1.0+dV_Vvr[j])*cos(lin->phi + dphi)/C - 1.0);

    //relative energy error due to dphase and dN error [ ]
    //changed to use dE_Ei by Daniel Driver
    dE_E[j]= dE_Eprev*Er + dE_Ei[j]+kw*dN_Nf/(1.0+dN_Nf)*ds/2.0;

    //relative energy error, but only individual linac, relative to 
    //final E (not original) (Note see below for division by final energy)
    //changed to use dE_Ei by Daniel Driver
    dE_Ei2[j]   = dE_Ei[j]*E;
    
    // R56 value changed by T566*dE/E [m]; 
    //changed by author(Paret?) (multiplication by 1 instead of 2)
    R56 = lin->R56 + 1.0*dE_E[j]*lin->T566;

    // approximate energy loss due to wake (>0) [GeV]
    Eloss[j]    = -E*kw*ds/2.0; 
    kR561       = 1.0+k[j]*R56;                       // save computation time
    sd2         = SQ(sdprev);                         // save computation time
    sz2         = SQ(szprev);                         // save computation time

    // rms bunch length after linac and R56 #(j-1) [m]
    sz[j] = sqrt(SQ(kR561)*sz2 + SQ(R56*Er*sdprev) + 
		 2.0*Er*R56*kR561*corprev);

    // rms energy spread after linac and R56 #(j-1) [
    sd[j]       = sqrt(SQ(k[j])*sz2 + SQ(Er)*sd2 + 2.0*Er*k[j]*corprev);

    // save new E-z correlation [m]
    cor[j]   = k[j]*kR561*sz2 + SQ(Er)*R56*sd2 + 
      Er*(1.0+2.0*k[j]*R56)*corprev;

    //signed-correlated energy spread (slope*sigz) [ ]
    sdsgn[j]    = cor[j]/sz[j]; 

    //Calculate peak current
    Ipk[j]=(1.0+dN_Nf)*N*e_oct*c/sqrt12/sz[j];
    //Ipk[j]=(1.0+dN_Nf)*fabs(gun->Q)*c/sqrt12/sz[j];

    //timing error
    dt[j]=dtprev + dE_E[j]*R56/c;  // timing error after linac k [s]

    //move current to prev for the next linac step
    Eprev=E;  
    dtprev=dt[j];
    szprev=sz[j];
    sdprev=sd[j];
    corprev=cor[j];
    dE_Eprev=dE_E[j];

  }
  
  //fininish calculating dE_Ei2 which requires nomalization by the final energy
  //I would like calculation to be all in one line above but we stored dE and added to energy at each
  //step to get the final energy is so it is unknown until the end of the loop
  //The E here is different from above. Before it changed with each loop and was the Energy
  // of the beam after linac j. Now it is static and is the final energy after all linacs

  for(int j=0;j<Nlinac;j++){
    dE_Ei2[j]=dE_Ei2[j]/E;  

  }

}



void doublecompress_better(Gun_Param * gun, Linac_Param ** linp_array,int Nlinac, 
		      //Inputs which change with time potentially
		      double dN_Nf, double dtg, double dEg,
		      double dsig_z, double  dsig_E, double chirp,
		      double * dphivr, double * dV_Vvr,
		      //outputs of doublecompress
		      double * Ipk, double * sz, double * dE_E,
		      double * sd , double * dt, double * sdsgn, 
		      double * k, double * Eloss, double * dE_Ei,
		      double * dE_Ei2, double * cor
		      )
{

  // stuff that gets updated with the linac here intial coditions are set
  double Eprev=gun->E; //Energy after leaving gun [ev]
  double szprev=gun->sz0+dsig_z; //rms bunch length [m] 
  double sdprev=gun->sd0+dsig_E; //incoh. energy spread [fraction]
  double dE_Eprev=dEg/gun->E;// relative energy error at start
  double dtprev=dtg; // timing error of prev linac NOT THE TIME STEP!!!!  
  double corprev=chirp;
  
  //crap just to reduce calculation and typeing
  Linac_Param * lin;
  double sqrt12=sqrt(12.0);
  
  //double N = fabs(gun->Q)*6.241E18;//e; //particles from gun
  //double N = fabs(gun->Q)/e; //particles from gun
  
  double E; //Energy at the end of the current linac
  double Er;  //energy ratio
  double ds;  //FW bunch lenfht for uniform dist. [m]
  double dphi; //total local phase error (gun, prev-R56 + local-rf) [rad]
  double s0;  //Wakefield characteristic length [m]
  double lambar; // lam/2*pi
  double R56; //R56 value adjust for energy error
  double C; //temp for cos(phir)
  double kR561; double sd2; double sz2; 
  double Nec;  //intermediate in calc of kw
  double kw;  // /wake's effect on linear correlation factor (<0) [1/m]
  double kn;  //rf phase induced linear correlation factor [1/m]

  for(int j=0;j<Nlinac;j++){

    lin=linp_array[j]; //pullout current linac from pointer array
    E=(Eprev+lin->dE); // calculate the energy after linac
    Er=Eprev/E;  //calculate energy ratio
    ds=szprev*sqrt(12.0);
    lambar=lin->lam/2.0/M_PI;
    C = cos(lin->phi);

    dphi= dtprev*c/lambar + dphivr[j];
    s0=lin->s0;

    //Nec=2.0*gun->Q*s0*Z0*c/M_PI/SQ(lin->a);
    Nec=2.0*fabs(gun->Q)*s0*Z0*c/M_PI/SQ(lin->a);

    //more physicsy stuff
    //wake's effect on linear correlation factor (<0) [1/m]
    kw= -(1.0+dN_Nf)*(Nec*lin->L/(SQ(ds)*E))*
      (1.0-(1.0+sqrt(ds/s0))*exp(-sqrt(ds/s0)));  

    // rf phase induced linear correlation factor [1/m]
    kn = (Er-1.0)*sin(lin->phi + dphi)/(lambar*C);
    k[j] = kw + kn;

    // relative energy error, but only individual
    // linac contribution (not original)
    //changed by Stefan Paret (introduced dV_Vvr)
    dE_Ei[j]    = (1.0-Er)*((1.0+dV_Vvr[j])*cos(lin->phi + dphi)/C - 1.0);

    //relative energy error due to dphase and dN error [ ]
    //changed to use dE_Ei by Daniel Driver
    dE_E[j]= dE_Eprev*Er + dE_Ei[j]+kw*dN_Nf/(1.0+dN_Nf)*ds/2.0;

    //relative energy error, but only individual linac, relative to 
    //final E (not original) (Note see below for division by final energy)
    //changed to use dE_Ei by Daniel Driver
    dE_Ei2[j]   = dE_Ei[j]*E;
    
    // R56 value changed by T566*dE/E [m]; 
    //changed by author(Paret?) (multiplication by 1 instead of 2)
    R56 = lin->R56 + 1.0*dE_E[j]*lin->T566;

    // approximate energy loss due to wake (>0) [GeV]
    Eloss[j]    = -E*kw*ds/2.0; 
    kR561       = 1.0+k[j]*R56;                       // save computation time
    sd2         = SQ(sdprev);                         // save computation time
    sz2         = SQ(szprev);                         // save computation time

    // rms bunch length after linac and R56 #(j-1) [m]
    sz[j] = sqrt(SQ(kR561)*sz2 + SQ(R56*Er*sdprev) + 
		 2.0*Er*R56*kR561*corprev);

    // rms energy spread after linac and R56 #(j-1) [
    sd[j]       = sqrt(SQ(k[j])*sz2 + SQ(Er)*sd2 + 2.0*Er*k[j]*corprev);

    // save new E-z correlation [m]
    cor[j]   = k[j]*kR561*sz2 + SQ(Er)*R56*sd2 + 
      Er*(1.0+2.0*k[j]*R56)*corprev;

    //signed-correlated energy spread (slope*sigz) [ ]
    sdsgn[j]    = cor[j]/sz[j]; 

    //Calculate peak current
    //Ipk[j]=(1.0+dN_Nf)*gun->Q*c/sqrt12/sz[j];
    Ipk[j]=(1.0+dN_Nf)*fabs(gun->Q)*c/sqrt12/sz[j];

    //timing error
    dt[j]=dtprev + dE_E[j]*R56/c;  // timing error after linac k [s]

    //move current to prev for the next linac step
    Eprev=E;  
    dtprev=dt[j];
    szprev=sz[j];
    sdprev=sd[j];
    corprev=cor[j];
    dE_Eprev=dE_E[j];

  }
  
  //fininish calculating dE_Ei2 which requires nomalization by the final energy
  //I would like calculation to be all in one line above but we stored dE and added to energy at each
  //step to get the final energy is so it is unknown until the end of the loop
  //The E here is different from above. Before it changed with each loop and was the Energy
  // of the beam after linac j. Now it is static and is the final energy after all linacs

  for(int j=0;j<Nlinac;j++){
    dE_Ei2[j]=dE_Ei2[j]/E;  

  }

}


void doublecompress_new(Gun_Param * gun, Linac_Param ** linp_array,int Nlinac, 
			//Inputs which change with time potentially
			Dynamic_Param * dynp, double * dphivr, double * dV_Vvr,
			//double_compress output states
			Doublecompress_State * dcs
			)
{

  //rename for some consistency with the original souble compress
  double dN_Nf = dynp->dQ_Q;

  // stuff that gets updated with the linac here intial coditions are set
  double Eprev=gun->E; //Energy after leaving gun [ev]
  double szprev=gun->sz0+dynp->dsig_z; //rms bunch length [m] 
  double sdprev=gun->sd0+dynp->dsig_E; //incoh. energy spread [fraction]
  double dE_Eprev=dynp->dE_ing/gun->E;// relative energy error at start
  double dtprev=dynp->dtg; // timing error of prev linac NOT THE TIME STEP!!!!  
  double corprev=dynp->dchirp;
  
  //crap just to reduce calculation and typeing
  Linac_Param * lin;
  double sqrt12=sqrt(12.0);
  
  double E; //Energy at the end of the current linac
  double Er;  //energy ratio
  double ds;  //FW bunch lenfht for uniform dist. [m]
  double dphi; //total local phase error (gun, prev-R56 + local-rf) [rad]
  double s0;  //Wakefield characteristic length [m]
  double lambar; // lam/2*pi
  double R56; //R56 value adjust for energy error
  double C; //temp for cos(phir)
  double kR561; double sd2; double sz2; 
  double Nec;  //intermediate in calc of kw
  double kw;  // /wake's effect on linear correlation factor (<0) [1/m]
  double kn;  //rf phase induced linear correlation factor [1/m]
  double k;   //k for the loop to reduce typing stored in dcs->k[j]
  for(int j=0;j<Nlinac;j++){

    lin=linp_array[j]; //pullout current linac from pointer array
    E=(Eprev+lin->dE); // calculate the energy after linac
    Er=Eprev/E;  //calculate energy ratio
    ds=szprev*sqrt(12.0);
    lambar=lin->lam/2.0/M_PI;
    C = cos(lin->phi);

    dphi= dtprev*c/lambar + dphivr[j];
    s0=lin->s0;

    //Nec=2.0*gun->Q*s0*Z0*c/M_PI/SQ(lin->a);
    Nec=2.0*fabs(gun->Q)*s0*Z0*c/M_PI/SQ(lin->a);

    //more physicsy stuff
    //wake's effect on linear correlation factor (<0) [1/m]
    kw= -(1.0+dN_Nf)*(Nec*lin->L/(SQ(ds)*E))*
      (1.0-(1.0+sqrt(ds/s0))*exp(-sqrt(ds/s0)));  

    // rf phase induced linear correlation factor [1/m]
    kn = (Er-1.0)*sin(lin->phi + dphi)/(lambar*C);
    k = kw + kn;
    dcs->k[j]=k;

    // relative energy error, but only individual
    // linac contribution (not original)
    //changed by Stefan Paret (introduced dV_Vvr)
    dcs->dE_Ei[j]    = (1.0-Er)*((1.0+dV_Vvr[j])*cos(lin->phi + dphi)/C - 1.0);

    //relative energy error due to dphase and dN error [ ]
    //changed to use dcs->dE_Ei by Daniel Driver
    dcs->dE_E[j]= dE_Eprev*Er + dcs->dE_Ei[j]+kw*dN_Nf/(1.0+dN_Nf)*ds/2.0;

    //relative energy error, but only individual linac, relative to 
    //final E (not original) (Note see below for division by final energy)
    //changed to use dcs->dE_Ei by Daniel Driver
    dcs->dE_Ei2[j]   = dcs->dE_Ei[j]*E;
    
    // R56 value changed by T566*dE/E [m]; 
    //changed by author(Paret?) (multiplication by 1 instead of 2)
    R56 = lin->R56 + 1.0*dcs->dE_E[j]*lin->T566;

    // approximate energy loss due to wake (>0) [GeV]
    dcs->Eloss[j]    = -E*kw*ds/2.0; 
    kR561       = 1.0+k*R56;                       // save computation time
    sd2         = SQ(sdprev);                         // save computation time
    sz2         = SQ(szprev);                         // save computation time

    // rms bunch length after linac and R56 #(j-1) [m]
    dcs->sz[j] = sqrt(SQ(kR561)*sz2 + SQ(R56*Er*sdprev) + 
		 2.0*Er*R56*kR561*corprev);

    // rms energy spread after linac and R56 #(j-1) [
    dcs->sd[j]       = sqrt(SQ(k)*sz2 + SQ(Er)*sd2 + 2.0*Er*k*corprev);

    // save new E-z correlation [m]
    dcs->cor[j]   = k*kR561*sz2 + SQ(Er)*R56*sd2 + 
      Er*(1.0+2.0*k*R56)*corprev;

    //signed-correlated energy spread (slope*sigz) [ ]
    dcs->sdsgn[j]    = dcs->cor[j]/dcs->sz[j]; 

    //Calculate peak current
    //dcs->Ipk[j]=(1.0+dN_Nf)*gun->Q*c/sqrt12/dcs->sz[j];
    dcs->Ipk[j]=(1.0+dN_Nf)*fabs(gun->Q)*c/sqrt12/dcs->sz[j];

    //timing error
    dcs->dt[j]=dtprev + dcs->dE_E[j]*R56/c;  // timing error after linac k [s]

    //move current to prev for the next linac step
    Eprev=E;  
    dtprev=dcs->dt[j];
    szprev=dcs->sz[j];
    sdprev=dcs->sd[j];
    dE_Eprev=dcs->dE_E[j];
    corprev=dcs->cor[j];
  }
  
  //finish calculating dcs->dE_Ei2 which requires nomalization by the final energy
  //I would like calculation to be all in one line above but we stored dE and added to energy at each
  //step to get the final energy is so it is unknown until the end of the loop
  //The E here is different from above. Before it changed with each loop and was the Energy
  // of the beam after linac j. Now it is static and is the final energy after all linacs

  for(int j=0;j<Nlinac;j++){
    dcs->dE_Ei2[j]=dcs->dE_Ei2[j]/E;  

  }

}
