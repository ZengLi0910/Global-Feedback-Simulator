#ifndef DOUBLECOMPRESS_H
#define DOUBLECOMPRESS_H

#include "linac_param.h"
//#include "state_space_top.h"
/*
	
  Functions to calculate bunch length and energy spread after a 
  compressor system (just like LCLS) where the E-z correlations are generated
  by linacs at off crest rf phases (not zero crossing).

*/

typedef struct str_doublecompress_state {
  // Variables of length Nlinac
  double *Ipk, *sz, *dE_E, *sd, *dt, *sdsgn, *k,
    *Eloss, *dE_Ei, *dE_Ei2;
  // Of length Nlinac
  double *cor;
} Doublecompress_State;

/*
 * A data structure to store dynamically set simulation variables,
 * i.e. they take on a new value each timestep
 */

typedef struct str_dynamic_param {
  
  double dQ_Q; //rename him to dQ_Q 
  double dtg;
  double dE_ing;
  double dsig_z;
  double dsig_E;
  double dchirp; //go into gun

  double complex adc_noise;
} Dynamic_Param;


void Doublecompress_State_Alloc(Doublecompress_State * dcs, int Nlinac);
void Doublecompress_State_Dealloc(Doublecompress_State * dcs);
void Doublecompress_State_Attach(Doublecompress_State * dcs, int Nlinac,
				 double * payload);
void doublecompress_octave(Gun_Param * gun, Linac_Param ** linp_array,int Nlinac, 
		      //Inputs which change with time potentially
		      double dN_Nf, double dtg, double dEg,
		      double dsig_z, double dsig_E, double chirp,
		      double * dphivr, double * dV_Vvr,
		      //outputs of doublecompress
		      double * Ipk, double * sz, double * dE_E,
		      double * sd , double * dt, double * sdsgn, 
		      double * k, double * Eloss, double * dE_Ei,
		      double * dE_Ei2, double * cor1
		    );

void doublecompress_better(Gun_Param * gun, Linac_Param ** linp_array,int Nlinac, 
		      //Inputs which change with time potentially
		      double dN_Nf, double dtg, double dEg,
		      double dsig_z, double dsig_E, double chirp,
		      double * dphivr, double * dV_Vvr,
		      //outputs of doublecompress
		      double * Ipk, double * sz, double * dE_E,
		      double * sd , double * dt, double * sdsgn, 
		      double * k, double * Eloss, double * dE_Ei,
		      double * dE_Ei2, double * cor1
		    );

void doublecompress_new(Gun_Param * gun, Linac_Param ** linp_array,int Nlinac, 
		      //Inputs which change with time potentially
			Dynamic_Param * dynp, double * dphivr, double * dV_Vvr,
		      //double_compress output states
		      Doublecompress_State * dcs
		    );

#endif
