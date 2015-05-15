#include "rf_station.h"

#include <math.h>

RF_State_dp RF_State_Allocate_Array(int n)
{
  RF_State_dp rf_state_net = calloc(n, sizeof(RF_State *));

  return rf_state_net;
}

RF_Station_dp RF_Station_Allocate_Array(int n)
{
  RF_Station_dp RF_Station_net = calloc(n, sizeof(RF_Station *));
  return RF_Station_net;
}

void RF_Station_Append(RF_Station** rf_station_arr, RF_Station* rf_station, int index)
{
  // XXX Add some check!!
  rf_station_arr[index] = rf_station;
}

/*
 * Delay
 */
void Delay_State_Allocate(Delay *delay, Delay_State *delay_state)
{
  // Allocate memory buffer
  delay_state -> buffer = (double complex*) calloc(delay->size,sizeof(double complex));
  // Initialize buffer
  for(int i=0 ; i<delay->size; i++){
    delay_state -> buffer[delay_state->index] = (double complex) 0.0;
  }
  // Initialize index
  delay_state -> index = 0;
}

void Delay_State_Deallocate(Delay_State *delay_state)
{
  free(delay_state->buffer);
}

void Delay_Deallocate(Delay *delay)
{
  delay->size = 0;
}

double complex Delay_Step(double complex in, Delay *delay, Delay_State *delay_state)
{
  double complex out;

  if(delay->size==0)
    out = in;
  else{
    out = delay_state -> buffer[delay_state->index];
    delay_state -> buffer[delay_state->index] = in;
    delay_state->index = (delay_state->index+1) % (delay->size);
  }

  return out;
}

void Delay_Clear(Delay *delay, Delay_State *delay_state){
  for(int i=0 ; i<delay->size; i++){
    delay_state -> buffer[delay_state->index] = (double complex) 0.0;
  }
  delay_state->index = 0;
}

/*
 * RF Station
 */
 void RF_Station_Allocate_In(RF_Station * rf_station,
  // General (nonphysical) simulation parameters
  double Tstep,
  // Properties of the triode system
  double Clip,
  double PAmax,
  double PAscale, // Amplifier scaling (from unitless to sqrt(W))

  double complex * p_TRF1, double complex * p_TRF2,
  // Properties of the RX Filter
  double complex * p_RXF,

  // Cavity
  Cavity *cav,
  // FPGA controller
  double stable_gbw, // Gain-Bandwidth product
  double FPGA_out_sat,  // FPGA output saturation limit

  // Loop Delay
  int loop_delay_size,

  // LLRF Noise Sources
  double probe_ns_rms,
  double rev_ns_rms,
  double fwd_ns_rms)
{
  rf_station->Clip = Clip;
  rf_station->PAscale = PAscale;
  rf_station->PAmax = PAmax;

  /*
  * Configure the Filters using their poles
  */

  Filter_Allocate_In(&rf_station->TRF1,2,2);
  for(int i=0;i<2;i++) {
    Filter_Append_Modes(&rf_station->TRF1, p_TRF1+i, 1,Tstep);
  }

  Filter_Allocate_In(&rf_station->TRF2,1,1);
  Filter_Append_Modes(&rf_station->TRF2, p_TRF2, 1, Tstep);

  Filter_Allocate_In(&rf_station->RXF,3,3);
  for(int i=0;i<3;i++) {
    Filter_Append_Modes(&rf_station->RXF,p_RXF+i,1,Tstep);
  }

  /*
  * Assign previously configured Cavity
  */
  rf_station->cav = cav;

  /*
  * Configure FPGA controller
  */
  // Grab cavity's open loop bandwidth and convert into Hz
  double cav_open_loop_bw = cav -> elecMode_net[cav->fund_index]->omega_f/(2.0*M_PI);
  // Calculate proportional gain (kp) based on cavity's open-loop bandwidth and controllers Gain-Bandwidth product
  double kp =  -stable_gbw/cav_open_loop_bw;

  // Find fundamental mode couplings (could emulate some sort of calibration procedure here)
  double complex fund_k_probe = cav -> elecMode_net[cav->fund_index]-> k_probe;
  double complex fund_k_drive = cav -> elecMode_net[cav->fund_index]-> k_drive;

  // Calculate the FPGA controller set-point
  // (scale using probe and drive couplings to the fundamental mode to convert to FPGA units)
  double complex set_point = cav->design_voltage*cexp(I*cav->rf_phase)*fund_k_probe;

  // Configure FPGA: note that ki (integral gain) is defined as kp/10.
  FPGA_Allocate_In(&rf_station->fpga,kp,kp*0.1, set_point, FPGA_out_sat, Tstep);

  // Coarse Loop Delay
  rf_station->loop_delay.size = loop_delay_size;

  rf_station->probe_ns_rms = probe_ns_rms;
  rf_station->rev_ns_rms = rev_ns_rms;
  rf_station->fwd_ns_rms = fwd_ns_rms;

}

RF_Station * RF_Station_Allocate_New(

  // General (non-physical) simulation parameters
  double Tstep,
  // Properties of the triode system
  double Clip,
  double PAmax,
  double PAscale, // Amplifier scaling (from unitless to sqrt(W))

  double complex * p_TRF1, double complex * p_TRF2,
  // Properties of the RX Filter
  double complex * p_RXF,

  // Cavity
  Cavity *cav,
  // FPGA controller
  double stable_gbw, // Gain-Bandwidth product
  double FPGA_out_sat,  // FPGA output saturation limit

  // Loop Delay
  int loop_delay_size,

  // LLRF Noise Sources
  double probe_ns_rms,
  double rev_ns_rms,
  double fwd_ns_rms)
{

  RF_Station * rf_station;
  rf_station = calloc(1,sizeof(RF_Station));

  RF_Station_Allocate_In(rf_station, Tstep, Clip, PAmax, PAscale, p_TRF1, p_TRF2,
    p_RXF, cav, stable_gbw, FPGA_out_sat, loop_delay_size,
    probe_ns_rms, rev_ns_rms, fwd_ns_rms);

  return rf_station;
}

void RF_Station_Deallocate(RF_Station *rf_station)
{

  Filter_Deallocate(&rf_station->RXF);
  Filter_Deallocate(&rf_station->TRF1);
  Filter_Deallocate(&rf_station->TRF2);

  FPGA_Deallocate(&rf_station->fpga);
  Cavity_Deallocate(rf_station->cav);
  Delay_Deallocate(&rf_station->loop_delay);

  rf_station->nom_grad = 0.0;
  rf_station->Clip = 0.0;
  rf_station->PAscale = 0.0;
  rf_station->PAmax = 0.0;

}

void RF_State_Allocate(RF_State *rf_state, RF_Station *rf_station){

  Filter_State_Allocate(&rf_state->RXF,    &rf_station->RXF);
  Filter_State_Allocate(&rf_state->TRF1,   &rf_station->TRF1);
  Filter_State_Allocate(&rf_state->TRF2,   &rf_station->TRF2);
  Cavity_State_Allocate(&rf_state->cav_state, rf_station->cav);

  rf_state->cav_state.E_probe = (double complex) 0.0;
  rf_state->cav_state.E_reverse = (double complex) 0.0;

  rf_state->fpga_state.drive = (double complex) 0.0;
  rf_state->fpga_state.state = (double complex) 0.0;
  rf_state->fpga_state.openloop = (int) 0;

  Delay_State_Allocate(&rf_station->loop_delay, &rf_state->loop_delay_state);
}

void RF_State_Deallocate(RF_State *rf_state, RF_Station *rf_station) {
  Filter_State_Deallocate(&rf_state->RXF);
  Filter_State_Deallocate(&rf_state->TRF1);
  Filter_State_Deallocate(&rf_state->TRF2);
  Cavity_State_Deallocate(&rf_state->cav_state, rf_station->cav);
}

double complex Phase_Shift(double complex in, double theta) {
  return in*cexp(I*theta);
}

void FPGA_Allocate_In(FPGA * fpga,
     double kp, double ki, double complex set_point, double out_sat, double Tstep)
{

  fpga->kp = kp;
  fpga->ki = ki;

  fpga->set_point = set_point;

  // Saturation limits
  fpga->out_sat = out_sat;
  fpga->state_sat = out_sat;

  fpga->Tstep = Tstep;
}

void FPGA_Deallocate(FPGA *fpga)
{
  fpga->kp = 0.0;
  fpga->ki = 0.0;
  fpga->set_point = 0.0;
  fpga->out_sat = 0.0;
  fpga->state_sat = 0.0;
  fpga->Tstep = 0.0;
}

void FPGA_Clear(FPGA_State * stnow)
{
  stnow-> drive = 0.0+0.0*_Complex_I;
  stnow-> state = 0.0+0.0*_Complex_I;
  stnow-> err = 0.0+0.0*_Complex_I;
  stnow-> openloop = 0;
}

double complex FPGA_Step(FPGA *fpga, double complex cavity_vol, FPGA_State *stnow)
{

  double state, drive, scale;

  // Calculate error signal
  double complex err = cavity_vol - fpga->set_point;

  if(stnow->openloop == 1) { // Open loop
    stnow->drive = fpga->set_point;
    stnow->state = fpga->set_point;
  } else {  //Closed loop
    // Integrator state
    state = stnow->state + fpga->Tstep*err*fpga->ki;
    // Compare state magnitude and saturation limit
    scale = cabs(state)/fpga->state_sat;
    // Clip integrator state if above limit
    stnow->state = (scale > 1.0) ? state/scale : state;

    // Drive signal
    drive = stnow->state + fpga->kp*err;
    // Compare output drive magnitude with saturation limit
    scale = cabs(drive)/fpga->out_sat;
    // Clip drive output if above limit
    stnow->drive = (scale > 1.0) ? drive/scale : drive;

    // Store error state
    stnow->err = err;
  }

  return err;
}

double complex Saturate(double complex in, double harshness) {
  return in*cpow(1.0+cpow(cabs(in),harshness), -1.0/harshness);
}

double complex SSA_Step(RF_Station *rf_station, double complex drive_in, RF_State *rf_state)
{
  double complex trf1out, satout, trf2out;

  // Scale input signal (sqrt(W) -> Normalized units)
  drive_in = drive_in/rf_station->PAscale;
  // Apply drive filter (TRF1)
  trf1out = Filter_Step(&rf_station->TRF1, drive_in, &rf_state->TRF1);
  // Clip
  satout = Saturate(trf1out,rf_station->Clip);
  // Scale output signal (sqrt(W) -> Normalized units)
  satout = satout*rf_station->PAscale;
  // Apply output filter (TRF2)
  trf2out = Filter_Step(&rf_station->TRF2, satout, &rf_state->TRF2);

  // Return SSA output
  return trf2out;
}

void SSA_Clear(RF_Station *rf_station, RF_State * rf_state)
{
  Filter_State_Clear(&rf_station->TRF1, &rf_state->TRF1);
  Filter_State_Clear(&rf_station->TRF2, &rf_state->TRF2);
}

void Apply_LLRF_Noise(RF_Station *rf_station, RF_State *rf_state)
{
  rf_state->probe_ns = randn(0.0, rf_station->probe_ns_rms) + _Complex_I*randn(0.0, rf_station->probe_ns_rms);
  rf_state->rev_ns = randn(0.0, rf_station->rev_ns_rms) + _Complex_I*randn(0.0, rf_station->rev_ns_rms);
  rf_state->fwd_ns = randn(0.0, rf_station->fwd_ns_rms) + _Complex_I*randn(0.0, rf_station->fwd_ns_rms);
}

double complex RF_Station_Step(
  // Configuration parameters
  RF_Station *rf_station,
  // Beam amplitude and phase noise (beam loading)
  double delta_tz, double complex beam_charge,
  // State vectors
	RF_State *rf_state)
{

  // Intermediate signals
  double complex E_probe_delayed, E_probe_lp, sig_error, Kg, V_acc;

  // LLRF noise (probe, forward and reverse signals)
  // Generate Gaussian Noise
  Apply_LLRF_Noise(rf_station, rf_state);

  // // Apply LLRF Noise
  rf_state->cav_state.E_probe += rf_state->probe_ns;
  rf_state->cav_state.E_reverse += rf_state->rev_ns;

  // // Apply (overall) loop delay to cavity probe signal
  E_probe_delayed = Delay_Step(rf_state->cav_state.E_probe, &rf_station->loop_delay, &rf_state->loop_delay_state);

  // Apply Low-Pass filter to probe signal (Rx Filter)
  E_probe_lp = Filter_Step(&rf_station->RXF, E_probe_delayed, &rf_state->RXF);

  // // FPGA
  sig_error = FPGA_Step(&rf_station->fpga, E_probe_lp, &rf_state->fpga_state);

  // // SSA (includes Tx filters and saturation)
  Kg = SSA_Step(rf_station, rf_state->fpga_state.drive, rf_state);

  // Cavity
  V_acc = Cavity_Step(rf_station->cav, delta_tz, Kg, beam_charge, &rf_state->cav_state);

  // Store digitized forward signal
  rf_state->cav_state.E_fwd = Kg + rf_state->fwd_ns;

  //  Overall cavity accelerating voltage (as seen by the beam)
  return V_acc;
}

void RF_Station_Clear(RF_Station *rf_station, RF_State * rf_state)
{
  FPGA_Clear(&rf_state->fpga_state);
  Cavity_Clear(rf_station->cav, &rf_state->cav_state);
  Filter_State_Clear(&rf_station->RXF, &rf_state->RXF);
  SSA_Clear(rf_station, rf_state);
  Delay_Clear(&rf_station->loop_delay, &rf_state->loop_delay_state);
}
