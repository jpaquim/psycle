/*

		Copyright (C) 1999 Juhana Sadeharju
						kouhia at nic.funet.fi

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	*/

#include "gverb.h"
#include "gverbdsp.h"
#include <cstdlib>
#include <malloc.h>
#include <cmath>
#include <cstring>

ty_gverb * gverb_new(
	int srate, float maxroomsize, float roomsize,
	float revtime, float damping, float spread,
	float inputbandwidth, float earlylevel, float taillevel
) {
	ty_gverb * p = (ty_gverb*) std::malloc(sizeof(ty_gverb));
	p->rate = srate;
	p->fdndamping = damping;
	p->maxroomsize = maxroomsize;
	p->roomsize = roomsize;
	p->revtime = revtime;
	p->earlylevel = earlylevel;
	p->taillevel = taillevel;

	p->maxdelay = p->rate * p->maxroomsize / 340.0;
	p->largestdelay = p->rate * p->roomsize / 340.0;

	// Input damper

	p->inputbandwidth = inputbandwidth;
	p->inputdamper = damper_make(1.0 - p->inputbandwidth);

	// FDN section
	{
		p->fdndels = (ty_fixeddelay**) std::calloc(FDNORDER, sizeof(ty_fixeddelay*));
		for(int i = 0; i < FDNORDER; ++i) {
			p->fdndels[i] = fixeddelay_make((int)p->maxdelay + 1000);
		}
		p->fdngains = (float*) std::calloc(FDNORDER, sizeof(float));
		p->fdnlens = (int*) std::calloc(FDNORDER, sizeof(int));

		p->fdndamps = (ty_damper**) std::calloc(FDNORDER, sizeof(ty_damper*));
		for(int i = 0; i < FDNORDER; ++i) {
			p->fdndamps[i] = damper_make(p->fdndamping);
		}

		float ga = 60;
		float gt = p->revtime;
		ga = std::pow(10.0f, -ga / 20.0f);
		int n = p->rate * gt;
		p->alpha = std::pow((double) ga, 1.0 / n);

		float gb = 0;
		for(int i = 0; i < FDNORDER; ++i) {
			if (i == 0) gb = 1.000000 * p->largestdelay;
			if (i == 1) gb = 0.816490 * p->largestdelay;
			if (i == 2) gb = 0.707100 * p->largestdelay;
			if (i == 3) gb = 0.632450 * p->largestdelay;

			#if 0
				p->fdnlens[i] = nearest_prime((int)gb, 0.5);
			#else
				p->fdnlens[i] = std::floor(gb);
			#endif

			p->fdngains[i] = -std::pow((float)p->alpha, p->fdnlens[i]);
		}

		p->d = (float*) std::calloc(FDNORDER, sizeof(float));
		p->u = (float*) std::calloc(FDNORDER, sizeof(float));
		p->f = (float*) std::calloc(FDNORDER, sizeof(float));
	}

	// Diffuser section
	{
		float diffscale = (float) p->fdnlens[3] / (210 + 159 + 562 + 410);
		float spread1 = spread;
		float spread2 = spread * 3.0f;

		int a, b, c, cc, d, dd, e;
		float r;
		b = 210;
		r = 0.125541f;
		a = spread1 * r;
		c = 210 + 159 + a;
		cc = c-b;
		r = 0.854046f;
		a = spread2 * r;
		d = 210 + 159 + 562 + a;
		dd = d - c;
		e = 1341 - d;

		p->ldifs = (ty_diffuser**) std::calloc(4, sizeof(ty_diffuser*));
		p->ldifs[0] = diffuser_make((int)(diffscale * b), 0.75);
		p->ldifs[1] = diffuser_make((int)(diffscale * cc), 0.75);
		p->ldifs[2] = diffuser_make((int)(diffscale * dd), 0.625);
		p->ldifs[3] = diffuser_make((int)(diffscale * e), 0.625);

		b = 210;
		r = -0.568366f;
		a = spread1 * r;
		c = 210 + 159 + a;
		cc = c - b;
		r = -0.126815f;
		a = spread2 * r;
		d = 210 + 159 + 562 + a;
		dd = d - c;
		e = 1341 - d;

		p->rdifs = (ty_diffuser**) std::calloc(4, sizeof(ty_diffuser*));
		p->rdifs[0] = diffuser_make((int)(diffscale * b), 0.75);
		p->rdifs[1] = diffuser_make((int)(diffscale * cc), 0.75);
		p->rdifs[2] = diffuser_make((int)(diffscale * dd), 0.625);
		p->rdifs[3] = diffuser_make((int)(diffscale * e), 0.625);
	}

	// Tapped delay section

	p->tapdelay = fixeddelay_make(44000);
	p->taps = (int*) std::calloc(FDNORDER, sizeof(int));
	p->tapgains = (float*) std::calloc(FDNORDER, sizeof(float));

	p->taps[0] = 5 + 0.410 * p->largestdelay;
	p->taps[1] = 5 + 0.300 * p->largestdelay;
	p->taps[2] = 5 + 0.155 * p->largestdelay;
	p->taps[3] = 5 + 0.000 * p->largestdelay;

	for(int i = 0; i < FDNORDER; ++i) {
		p->tapgains[i] = std::pow(p->alpha, (double)p->taps[i]);
	}

	return p;
}

void gverb_free(ty_gverb * p) {
	damper_free(p->inputdamper);
	for(int i = 0; i < FDNORDER; ++i) {
		fixeddelay_free(p->fdndels[i]);
		damper_free(p->fdndamps[i]);
		diffuser_free(p->ldifs[i]);
		diffuser_free(p->rdifs[i]);
	}
	std::free(p->fdndels);
	std::free(p->fdngains);
	std::free(p->fdnlens);
	std::free(p->fdndamps);
	std::free(p->d);
	std::free(p->u);
	std::free(p->f);
	std::free(p->ldifs);
	std::free(p->rdifs);
	std::free(p->taps);
	std::free(p->tapgains);
	fixeddelay_free(p->tapdelay);
	std::free(p);
}

void gverb_flush(ty_gverb * p) {
	damper_flush(p->inputdamper);
	for(int i = 0; i < FDNORDER; ++i) {
		fixeddelay_flush(p->fdndels[i]);
		damper_flush(p->fdndamps[i]);
		diffuser_flush(p->ldifs[i]);
		diffuser_flush(p->rdifs[i]);
	}
	std::memset(p->d, 0, FDNORDER * sizeof(float));
	std::memset(p->u, 0, FDNORDER * sizeof(float));
	std::memset(p->f, 0, FDNORDER * sizeof(float));
	fixeddelay_flush(p->tapdelay);
}
