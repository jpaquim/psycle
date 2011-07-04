// -*- mode:c++; indent-tabs-mode:t -*-
/**************************************************************************
*   Copyright 2007 Psycledelics http://psycle.sourceforge.net             *
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
#ifndef PSYCLE__CORE__PATTERN_LINE_INCLUDED
#define PSYCLE__CORE__PATTERN_LINE_INCLUDED
#pragma once

#include "patternevent.h"

#include <map>
#include <string>

namespace psy { namespace core {

/**
@author  Psycledelics  
*/
class PatternLine {
	public:
		PatternLine() : sequencerTrack_(0) {}

		void setSequenceTrack(int track) { sequencerTrack_ = track; }
		int sequenceTrack() const { return sequencerTrack_; }

		std::string toXml( float pos ) const;

		std::map<int, PatternEvent> & notes() { return noteMap; }
		const std::map<int, PatternEvent> & notes() const { return noteMap; }

		std::map<int, PatternEvent> & tweaks() { return tweakMap; }
		const std::map<int, PatternEvent> & tweaks() const { return tweakMap; }

		///\todo why is this virtual?
		virtual bool empty() const { return noteMap.empty() && tweakMap.empty(); }

	private:

		int sequencerTrack_;

		std::map<int, PatternEvent> noteMap;
		std::map<int, PatternEvent> tweakMap;
};

}}
#endif
