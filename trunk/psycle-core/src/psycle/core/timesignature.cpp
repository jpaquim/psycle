// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/detail/project.private.hpp>
#include "timesignature.h"

namespace psycle { namespace core {

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

TimeSignature::TimeSignature(float ownerDefinedBeats) {
	ownerDefined_ = true;
	ownerDefinedBeats_ = ownerDefinedBeats;
}


TimeSignature::~TimeSignature() {
}

void TimeSignature::setNumerator(int value) {
	numerator_ = value;
}

int TimeSignature::numerator( ) const {
	if (ownerDefined_) return 4;
	return numerator_;
}

void TimeSignature::setDenominator(int value) {
	if (value != 0) {
		denominator_ = value;
	}
}

int TimeSignature::denominator() const {
	if (ownerDefined_) return 4;
	return denominator_;
}

void TimeSignature::setCount(int count) {
	count_ = count;
}

int TimeSignature::count() const {
	return count_;
}

void TimeSignature::incCount() {
	count_++;
}

float TimeSignature::beats() const {
	if(ownerDefined_) return ownerDefinedBeats_;
	else return static_cast< float >(numerator_ * count_);
}

void TimeSignature::set_beats(float beats) {
	ownerDefined_ = true;
	ownerDefinedBeats_ = beats;
}

bool TimeSignature::ownerDefined() const {
	return ownerDefined_;
}

}}
