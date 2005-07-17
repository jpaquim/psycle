/*		Blitz (C)2005 by Jan-Marco Edelmann [voskomo], voskomo_at_voskomo_dot_com
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


// pwm.cpp: implementation of the pwm class.
//
//////////////////////////////////////////////////////////////////////

#include "pwm.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

pwm::pwm()
{

}

pwm::~pwm()
{

}

int pwm::getPosition(){
 return realpos;
}

int pwm::getLast(){ return (int)last; }
void pwm::setRange(int val) { frange=(float)val*0.000488519785f; }
void pwm::setSpeed(int val) { speed=val+val; }
void pwm::reset(){ last=9999; pos=0; }
void pwm::next(){
	if (frange != 0) move = speed; else speed = 0;
	last=realpos;
	realpos=(int)((float)pos*frange)&2047;
	if (direction){
		pos-=move;
		if (pos<=0){
			pos=0;
			direction=0;
		}
	}else{
		pos+=move;
		if (pos>=2047){
			pos=2047;
			direction=1;
		}
	}
};
