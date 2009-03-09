// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__CORE__TIME_SIGNATURE__INCLUDED
#define PSYCLE__CORE__TIME_SIGNATURE__INCLUDED
#pragma once

#include <psycle/core/config.hpp>

namespace psy { namespace core {

class TimeSignature {
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

	private:
		int numerator_;
		int denominator_;
		int count_;

		bool ownerDefined_;
		float ownerDefinedBeats_;
};

}}
#endif
