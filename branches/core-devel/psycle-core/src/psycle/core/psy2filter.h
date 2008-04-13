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
#ifndef PSYCLE__CORE__PSY2FILTER__INCLUDED
#define PSYCLE__CORE__PSY2FILTER__INCLUDED
#pragma once

#include "psyfilter.h"
#include "cstdint.h"

namespace psy { namespace core {

class RiffFile;
class PatternCategory;
class SequenceLine;
class Machine;
class PatternEvent;
class MachineCallbacks;

namespace convert_internal_machines
{
	class Converter;
}

/**
@author  Psycledelics  
*/
template class<T>
class Psy2Filter<t> : public PsyFilterBase<T>
{
	protected:
		//Note: convert_internal_machines uses its own enum.
		typedef enum MachineClass
		{
			MACH_MASTER = 0,
			MACH_SINE = 1, 
			MACH_DIST = 2,
			MACH_SAMPLER = 3,
			MACH_DELAY = 4,
			MACH_2PFILTER = 5,
			MACH_GAIN = 6, 
			MACH_FLANGER = 7, 
			MACH_PLUGIN = 8,
			MACH_VST = 9,
			MACH_VSTFX = 10,
			MACH_DUMMY = 255
		} machineclass_t;

		class VSTLoader
		{
			public:
				bool valid;
				char dllName[128];
				int numpars;
				float * pars;
		};
		class PatternEntry
		{
		public:
			inline PatternEntry()
				:
				_note(255),
				_inst(255),
				_mach(255),
				_cmd(0),
				_parameter(0)
			{}

			std::uint8_t _note;
			std::uint8_t _inst;
			std::uint8_t _mach;
			std::uint8_t _cmd;
			std::uint8_t _parameter;
		};

	///\name Singleton Pattern
	///\{
	protected:
		Psy2Filter() friend class SongFactory;
	private:
		Psy2Filter( Psy2Filter const & );
		Psy2Filter& operator=(Psy2Filter const &);
	///\}

	public:
		/*override*/ int version() const { return 2; }
		/*override*/ std::string filePostfix() const { return "psy"; }
		/*override*/ bool testFormat(const std::string & fileName);
		/*override*/ bool load(const std::string & fileName, T& song, MachineFactory& factory);
		/*override*/ bool save(const std::string & /*fileName*/, const T& /*song*/) {  /* so saving for legacy file format */ return false; }


	protected:
		virtual bool LoadINFO(RiffFile* file,T& song);
		virtual bool LoadSNGI(RiffFile* file,T& song);
		virtual bool LoadSEQD(RiffFile* file,T& song);
		virtual bool LoadPATD(RiffFile* file,T& song,int index);
		virtual bool LoadINSD(RiffFile* file,T& song);
		virtual bool LoadWAVD(RiffFile* file,T& song);
		virtual bool PreLoadVSTs(RiffFile* file,T& song);
		virtual bool LoadMACD(RiffFile* file,T& song,convert_internal_machines::Converter& converter, MachineFactory& factory);
		virtual bool TidyUp(RiffFile* file,T &song,convert_internal_machines::Converter& converter);

	protected:
		static std::string const FILE_FOURCC;
		/// PSY2-fileformat Constants
		static int const PSY2_MAX_TRACKS;
		static int const PSY2_MAX_WAVES;
		static int const PSY2_MAX_INSTRUMENTS;
		static int const PSY2_MAX_PLUGINS;

	private:
		std::vector<int> seqList;
		PatternCategory* singleCat;
		SequenceLine* singleLine;
		Machine* pMac[128];
		bool _machineActive[128];
		unsigned char busMachine[64];
		unsigned char busEffect[64];
		VSTLoader vstL[256];
		float volMatrix[128][12];

		//inter-loading value
		int linesPerBeat;

		//todo: psy2filtermfc, restore these two values.
		unsigned char octave;
		int instSelected; //-> map to auxcolselected

		void preparePatternSequence(T & song);
		PatternEvent convertEntry( unsigned char * data ) const;
};

}}
#endif
