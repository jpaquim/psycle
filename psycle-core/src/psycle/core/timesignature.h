// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__TIME_SIGNATURE__INCLUDED
#define PSYCLE__CORE__TIME_SIGNATURE__INCLUDED
#pragma once

#include <psycle/core/detail/project.hpp>

namespace psycle { namespace core {

class PSYCLE__CORE__DECL TimeSignature {
	public:
		TimeSignature();
		TimeSignature(int numerator, int denominator);
		TimeSignature(float ownerDefinedBeats);

		~TimeSignature();

		void setNumerator(int value);
		int numerator() const;

		int denominator() const;
		void setDenominator(int value);

		void setCount(int count);
		void incCount();
		int count() const;

		float beats() const;
		bool ownerDefined() const;
		void set_beats(float beats); // owner_defined

	private:
		int numerator_;
		int denominator_;
		int count_;

		bool ownerDefined_;
		float ownerDefinedBeats_;
};

}}
#endif
