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
#ifndef PSYCLE__CORE__PSY3FILTER__INCLUDED
#define PSYCLE__CORE__PSY3FILTER__INCLUDED

#include "psyfilterbase.h"
#include <cstdint>
#include <vector>

namespace psy { namespace core {

class RiffFile;
class MachineCallbacks;
class PatternCategory;
class SequenceLine;
class PatternEvent;

/**
@author  Psycledelics  
*/
class Psy3Filter : public PsyFilterBase
{
	protected:
		typedef enum MachineClass
		{
			MACH_MASTER = 0,
			MACH_SAMPLER = 3,
			MACH_PLUGIN = 8,
			MACH_VST = 9,
			MACH_XMSAMPLER = 12,
			MACH_DUPLICATOR = 13,
			MACH_MIXER = 14,
			MACH_AUDIOINPUT = 15,
			MACH_DUMMY = 255
		} machineclass_t;

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
		Psy3Filter(); 
	private:
		Psy3Filter( Psy3Filter const & );
		Psy3Filter& operator=(Psy3Filter const&);
	public:
		static Psy3Filter* Instance() {
			// don`t use multithreaded
			static Psy4Filter s;
			return &s; 
		}
		///\}

	protected:
		/*override*/ int version() const { return 3; }
		/*override*/ std::string filePostfix() const { return "psy"; }
		/*override*/ bool testFormat(const std::string & fileName);
		/*override*/ bool load(const std::string & fileName, CoreSong& song, MachineFactory& factory);
		/*override*/ bool save(const std::string & /*fileName*/, const CoreSong& /*song*/) {  /* so saving for legacy file format */ return false; }


	protected:
		virtual int LoadSONGv0(RiffFile* file,CoreSong& song);
		virtual bool LoadINFOv0(RiffFile* file,CoreSong& song,int minorversion);
		virtual bool LoadSNGIv0(RiffFile* file,CoreSong& song,int minorversion, MachineCallbacks* callbacks);
		virtual bool LoadSEQDv0(RiffFile* file,CoreSong& song,int minorversion);
		virtual bool LoadPATDv0(RiffFile* file,CoreSong& song,int minorversion);
		virtual bool LoadMACDv0(RiffFile* file,CoreSong& song,int minorversion, MachineFactory& factory);
		virtual bool LoadINSDv0(RiffFile* file,CoreSong& song,int minorversion);

	protected:
		static std::string const FILE_FOURCC;
		static std::uint32_t const VERSION_INFO;
		static std::uint32_t const VERSION_SNGI;
		static std::uint32_t const VERSION_SEQD;
		static std::uint32_t const VERSION_PATD;
		static std::uint32_t const VERSION_MACD;
		static std::uint32_t const VERSION_INSD;
		static std::uint32_t const VERSION_WAVE;

		static std::uint32_t const FILE_VERSION;

	private:
		std::vector<int> seqList;
		PatternCategory* singleCat;
		SequenceLine* singleLine;

		//inter-loading value.
		int linesPerBeat;

		//todo: psy3filtermfc, restore these values.
		unsigned char octave;
		int seqBus;
		int instSelected;
		int midiSelected;
		int auxcolSelected;
		int machineSoloed;
		int trackSoloed;


		void RestoreMixerSendFlags(CoreSong& song);

		PatternEvent convertEntry( unsigned char* data) const;

		void preparePatternSequence(CoreSong& song);
};

}}
#endif
