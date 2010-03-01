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

class AAF16 {
public:
		AAF16();
		~AAF16();
		float y[4]; //output samples
		float x[4]; //input samples
		inline float process(float NewSample);
};

inline float AAF16::process(float NewSample){
	x[3] = x[2];
	x[2] = x[1];
	x[1] = x[0];
	x[0] = NewSample;

	y[3] = y[2];
	y[2] = y[1];
	y[1] = y[0];

	y[0] = 0.00008706096478462893f * x[0];
	y[0] += 0.00026118289435388679f * x[1] - -2.90004956629741750000f * y[1];
	y[0] += 0.00026118289435388679f * x[2] - 2.81919480076970650000f * y[2];
	y[0] += 0.00008706096478462893f * x[3] - -0.91844977163113484000f * y[3];

	return y[0];
}