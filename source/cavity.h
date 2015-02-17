#ifndef CAVITY_H
#define CAVITY_H

#include <complex.h>
#include <math.h>
#include <stdlib.h>

#include "filter.h"


typedef struct str_elecmode{
	double omega_0_mode, drive_mod_theta, omega_f;
	double complex k_drive, k_beam, k_probe, k_em;
	double Tstep;
	Filter fil;
	double *mech_couplings;
} ElecMode;

typedef ElecMode * ElecMode_p;
typedef ElecMode ** ElecMode_dp;

typedef struct str_elecmode_state{
	Filter_State fil_state;
	double complex drive_mod;
	double delta_omega;
	double d_phase;
	double complex v_2;
} ElecMode_State;

typedef struct str_cavity{
	// XXX Check if used in the future: Inherited properties
	double nom_beam_phase, rf_phase, design_voltage, unity_voltage;
	double open_loop_bw;
	// XXX

	double L;
	double nom_grad;
	int n_modes;
	ElecMode_dp elecMode_net;
	double complex v_probe, v_rev;
} Cavity;

typedef struct str_cavity_state{
	double complex E_probe, E_reverse;
	ElecMode_State **elecMode_state_net;
} Cavity_State;

void Cavity_Allocate_In(Cavity *cav, 
  ElecMode_dp elec_mode_net, int n_modes,
  double L, double nom_grad, 
  double nom_beam_phase, double rf_phase, double design_voltage, double unity_voltage,
  double open_loop_bw);

Cavity * Cavity_Allocate_New(ElecMode_dp elec_mode_net, int n_modes, 
  double L, double nom_grad,
  double nom_beam_phase, double rf_phase, double design_voltage, double unity_voltage,
  double open_loop_bw);
void Cavity_Deallocate(Cavity *cav);

double complex Cavity_Step(Cavity *cav, double delta_tz, double complex drive_in, double complex beam_charge, Cavity_State *cav_state);
void Cavity_Clear(Cavity *cav, Cavity_State *cav_state);

void Cavity_State_Allocate(Cavity_State *cav_state, Cavity *cav);
void Cavity_State_Deallocate(Cavity_State *cav_state, Cavity *cav);

void ElecMode_Allocate_In(ElecMode *elecMode,
  double RoverQ, double foffset, double omega_0_mode,
  double Q_0, double Q_drive, double Q_probe,
  double beam_phase,  double phase_rev, double phase_probe,
  double Tstep
  // , double *mech_couplings
  );

ElecMode *ElecMode_Allocate_New(
  double RoverQ, double foffset, double omega_0_mode,
  double Q_0, double Q_drive, double Q_probe,
  double beam_phase,  double phase_rev, double phase_probe,
  double Tstep
  // , double *mech_couplings
  );
void ElecMode_Deallocate(ElecMode * elecMode);

ElecMode_dp ElecMode_Allocate_Array(int n);
void ElecMode_Append(ElecMode** elecMode_arr, ElecMode* elecMode, int index);

void ElecMode_State_Allocate(ElecMode_State *elecMode_state, ElecMode *elecMode);
void ElecMode_State_Deallocate(ElecMode_State *elecMode_state, ElecMode * elecMode);

double complex ElecMode_Step(ElecMode *elecMode, double complex Kg_fwd, 
	double beam_charge, double delta_tz,
  ElecMode_State *elecMode_state,
  double complex *v_probe, double complex *v_em);

double complex ElecMode_Get(Cavity *cav, int idx, const char *k_type);
void ElecMode_Set(Cavity *cav, int idx, const char *k_type, double complex new_value);

#endif