/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#ifndef _FLUID_IIR_FILTER_H
#define _FLUID_IIR_FILTER_H

#include <utils/fluidsynth_priv.h>

typedef struct _fluid_iir_filter_t fluid_iir_filter_t;


void fluid_iir_filter_apply(fluid_iir_filter_t* iir_filter,
                            fluid_real_t *dsp_buf, int dsp_buf_count); 

void fluid_iir_filter_reset(fluid_iir_filter_t* iir_filter);

void fluid_iir_filter_set_q_dB(fluid_iir_filter_t* iir_filter, 
                               fluid_real_t q_dB);

void fluid_iir_filter_set_fres(fluid_iir_filter_t* iir_filter, 
                               fluid_real_t fres);

void fluid_iir_filter_calc(fluid_iir_filter_t* iir_filter, 
                           fluid_real_t output_rate, 
                           fluid_real_t fres_mod); 

/* We can't do information hiding here, as fluid_voice_t includes the struct
   without a pointer. */
struct _fluid_iir_filter_t
{
	/* filter coefficients */
	/* The coefficients are normalized to a0. */
	/* b0 and b2 are identical => b02 */
	fluid_real_t b02;              /* b0 / a0 */
	fluid_real_t b1;              /* b1 / a0 */
	fluid_real_t a1;              /* a0 / a0 */
	fluid_real_t a2;              /* a1 / a0 */

	fluid_real_t b02_incr;
	fluid_real_t b1_incr;
	fluid_real_t a1_incr;
	fluid_real_t a2_incr;
	int filter_coeff_incr_count;
	fluid_real_t hist1, hist2;      /* Sample history for the IIR filter */
	int filter_startup;             /* Flag: If set, the filter will be set directly.
					   Else it changes smoothly. */

	fluid_real_t fres;              /* the resonance frequency, in cents (not absolute cents) */
	fluid_real_t last_fres;         /* Current resonance frequency of the IIR filter */
	/* Serves as a flag: A deviation between fres and last_fres */
	/* indicates, that the filter has to be recalculated. */
	fluid_real_t q_lin;             /* the q-factor on a linear scale */
	fluid_real_t filter_gain;       /* Gain correction factor, depends on q */
};

#endif

