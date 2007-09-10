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

// pwm.h: interface for the lfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class pwm  
{
public:
	pwm();
	virtual ~pwm();
	int getPosition();
	int getLast();
	void setRange(int val);
	void setSpeed(int val);
	void reset();
	void next();
	bool once;
	bool twice;
private:
	int sym;
	float frange;
	int speed;
	int pos;
	int realpos;
	int last;
	int move;
	int direction;
};
