/***************************************************************************
	*   Copyright (C) 2007 Psycledelics     *
	*   psycle.sf.net   *
	*                                                                         *
	*   This program is free software; you can redistribute it and/or modify  *
	*   it under the terms of the GNU General Public License as published by  *
	*   the Free Software Foundation; either version 2 of the License, or     *
	*   (at your option) any later version.                                   *
	*                                                                         *
	*   This program is distributed in the hope that it will be useful,       *
	*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
	*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
	*   GNU General Public License for more details.                          *
	*                                                                         *
	*   You should have received a copy of the GNU General Public License     *
	*   along with this program; if not, write to the                         *
	*   Free Software Foundation, Inc.,                                       *
	*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
	***************************************************************************/


#include "timesignature.h"

namespace psy
{
	namespace core
	{

TimeSignature::TimeSignature() :
	numerator_(4),
	denominator_(4)
{
	count_ = 1;
	ownerDefined_ = false;
	ownerDefinedBeats_ = 0;
}

TimeSignature::TimeSignature(int numerator, int denominator) :
	numerator_(numerator),
	denominator_(denominator)
{
	count_ = 1;
	ownerDefined_ = false;
	ownerDefinedBeats_ = 0;
}

TimeSignature::TimeSignature( float ownerDefinedBeats )
{
	ownerDefined_ = true;
	ownerDefinedBeats_ = ownerDefinedBeats;
}


TimeSignature::~TimeSignature()
{
}

void TimeSignature::setNumerator( int value )
{
	numerator_ = value;
}

int TimeSignature::numerator( ) const
{
	if (ownerDefined_) return 4;
	return numerator_;
}

void TimeSignature::setDenominator( int value )
{
	if (value != 0) {
		denominator_ = value;
	}
}

int TimeSignature::denominator( ) const
{
	if (ownerDefined_) return 4;
	return denominator_;
}

void TimeSignature::setCount( int count )
{
	count_ = count;
}

int TimeSignature::count( ) const
{
	return count_;
}

void TimeSignature::incCount( )
{
	count_++;
}

float TimeSignature::beats( ) const
{
	if (ownerDefined_)
			return ownerDefinedBeats_;
	else
			return static_cast< float >(numerator_ * count_);
}

bool TimeSignature::ownerDefined( ) const
{
	return ownerDefined_;
}

	}
}
