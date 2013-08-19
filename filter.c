

#include "filter.h"
#include "stdlib.h"

Filter * Filter_Allocate_New(int alloc_order, int alloc_coeffs)
{
  Filter * fil;
  fil = calloc(1,sizeof(Filter));
  
  fil->alloc_order = alloc_order;
  fil->alloc_coeffs = alloc_coeffs;
  
  fil->n_coeffs=0;
  fil->order = 0;
  
  fil->modes = (int*)calloc(alloc_order,sizeof(int));
  fil->coeff_start = (int*)calloc(alloc_order,sizeof(int));
  fil->coeffs = (double complex *)calloc(3*alloc_coeffs,sizeof(double complex));
  fil->poles = (double complex *)calloc(alloc_coeffs,sizeof(double complex));
  return fil;
}

void Filter_Allocate_In(Filter * fil, int alloc_order, int alloc_coeffs)
{
  
  fil->alloc_order = alloc_order;
  fil->alloc_coeffs = alloc_coeffs;
  
  fil->n_coeffs=0;
  fil->order = 0;
  
  fil->modes = (int*)calloc(alloc_order,sizeof(int));
  fil->coeff_start = (int*)calloc(alloc_order,sizeof(int));
  
  fil->coeffs = (double complex *)calloc(3*alloc_coeffs,sizeof(double complex));
  fil->poles = (double complex *)calloc(alloc_coeffs,sizeof(double complex));
}
void Filter_Deallocate(Filter * fil)
{
  free(fil->poles);
  free(fil->coeffs);
  free(fil->coeff_start);
  free(fil->modes);
  fil->order = 0;
  fil->n_coeffs = 0;
  fil->alloc_order = 0;
  fil->alloc_coeffs = 0;
}

void Filter_Append_Modes(Filter * fil, double complex * poles, int mod, double dt)
{
  int i;
  /*
   * Append to the data structures
   */
  // Increment order of the filter and reallocate needed arrays
  fil->order++;
  if(fil->order >= fil->alloc_order) {
    fil->modes = realloc(fil->modes,fil->order*sizeof(int));
    fil->alloc_order = fil->order;
    fil->coeff_start = realloc(fil->coeff_start,fil->order*sizeof(int));
  }
  // Update the indexing arrays for the new entry
  fil->modes[fil->order-1] = mod;
  if(fil->order>1) {
    fil->coeff_start[fil->order-1] = 
      fil->coeff_start[fil->order-2]+fil->modes[fil->order-2];
  }
  // Allocate more coefficients
  fil->n_coeffs += mod;
  if(fil->n_coeffs >= fil->alloc_coeffs) {
    fil->coeffs = realloc(fil->coeffs, fil->n_coeffs* 3*sizeof(double complex));
    fil->poles = realloc(fil->poles, fil->n_coeffs *sizeof(double complex));

    fil->alloc_coeffs = fil->n_coeffs;
  }

  /*
   * Calculate new coefficients
   */
  for(i=0; i<mod; i++) {
    int cs = fil->coeff_start[fil->order-1]+i;
    fil->poles[cs]=poles[i];
    fil->coeffs[3*cs+0] = (1.0 + 0.5*dt*poles[i])
                        /(1.0 - 0.5*dt*poles[i]);
    fil->coeffs[3*cs+1] =     dt
                        /(1.0-0.5*dt*poles[i]);
    fil->coeffs[3*cs+2] = cabs(poles[i]);
  }
  // TODO
}
void Filter_State_Allocate(Filter_State * sf, Filter * fil) {
  sf->state = calloc(fil->n_coeffs,sizeof(double complex));
  sf->input = calloc(fil->order,sizeof(double complex));
}
void Filter_State_Deallocate(Filter_State * sf) {
  free(sf->state);
  free(sf->input);
}

double complex Filter_Step(Filter * fil, double complex innow,
			   Filter_State * filnow, Filter_State * filpast)
{
  int o,m,cs;
  double complex voltage_in, a,b,scale;
  double complex output = innow;

  for(o=0;o<fil->order;o++) {
    filnow->input[o]=output;
    voltage_in = 0.5*(filnow->input[o] + filpast->input[o]);

    
    output = 0.0;
    for(m=0;m<fil->modes[m];m++) {
      cs = fil->coeff_start[o]+m;
      a = fil->coeffs[3*cs+0];
      b = fil->coeffs[3*cs+1];
      scale = fil->coeffs[3*cs+2];
      filnow->state[cs] = a*filpast->state[cs]+b*voltage_in;
      output += filnow->state[cs]*scale;
    }

  }
  return output;
}
