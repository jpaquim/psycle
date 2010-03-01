/*								Blitz (C)2005 by jme
		Programm is based on Arguru Bass. Filter seems to be Public Domain.

		This plugin is free software; you can redistribute it and/or modify
		it under the terms of the GNU General Public License as published by
		the Free Software Foundation; either version 2 of the License, or
		(at your option) any later version.\n"\

		This plugin is distributed in the hope that it will be useful,
		but WITHOUT ANY WARRANTY; without even the implied warranty of
		MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
		GNU General Public License for more details.

		You should have received a copy of the GNU General Public License
		along with this program; if not, write to the Free Software
		Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Chebyshev Lowpass, 3rd order
// Cutoff: 16 KHz
// Samplerate: 705KHz (44KHz x 16)
// 3dB Pass Band Ripple

#include "aaf.h"

AAF16::AAF16(){
	for (int c=0; c<5; c++){
		x[c]=0.0f;
		y[c]=0.0f;
	}
}

AAF16::~AAF16(){
}

