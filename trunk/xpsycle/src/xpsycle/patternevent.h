/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
 *   natti@linux   *
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
#ifndef PATTERNEVENT_H
#define PATTERNEVENT_H

#include "pattern.h"
#include <cstdint>
#include <vector>

/**
@author Stefan Nattkemper
*/


///\todo code has no PatternEntry anymore, but we need still to integrate the enums in InputHandler, songstruct and pattern.h here
///\todo this is just a work in progress, don`t use this enums, naming cdef seems weired to me

//const int CS_KEY_START = 0;

//const int CS_IMM_START = 256;

//const int CS_EDT_START = 512;

//const int CS_LAST = max_cmds;

namespace psycle
{
	namespace host
	{

		class PatternEvent{
		public:

		/// command set.
    enum CmdSet
    {
      cdefNull = -1,
      // keys
      cdefKeyC_0 = 0, // CS_KEY_START,
      cdefKeyCS0,
      cdefKeyD_0,
      cdefKeyDS0,
      cdefKeyE_0,
      cdefKeyF_0,
      cdefKeyFS0,
      cdefKeyG_0,
      cdefKeyGS0,
      cdefKeyA_0,
      cdefKeyAS0,
      cdefKeyB_0,
      cdefKeyC_1, ///< 12
      cdefKeyCS1,
      cdefKeyD_1,
      cdefKeyDS1,
      cdefKeyE_1,
      cdefKeyF_1,
      cdefKeyFS1,
      cdefKeyG_1,
      cdefKeyGS1,
      cdefKeyA_1,
      cdefKeyAS1,
      cdefKeyB_1,
      cdefKeyC_2, ///< 24
      cdefKeyCS2,
      cdefKeyD_2,
      cdefKeyDS2,
      cdefKeyE_2,
      cdefKeyF_2,
      cdefKeyFS2,
      cdefKeyG_2,
      cdefKeyGS2,
      cdefKeyA_2,	

      cdefKeyStop = 120,	///< NOTE STOP
      cdefTweakM = 121,	///< tweak
      cdefTweakE = 122,	///< tweak effect. Old!
      cdefMIDICC = 123,	///< Mcm Command (MIDI CC)
      cdefTweakS = 124,	///< tweak slide command

     };



			enum Cmd
			{
				EXTENDED      = 0xFE, // see below
				SET_TEMPO     = 0xFF,
				NOTE_DELAY    = 0xFD,
				RETRIGGER     = 0xFB,
				RETR_CONT     = 0xFA,
				SET_VOLUME    = 0x0FC,
				SET_PANNING   = 0x0F8,
				BREAK_TO_LINE = 0xF2,
				JUMP_TO_ORDER = 0xF3,
				ARPEGGIO      = 0xF0,

				// Extended Commands from 0xFE
				SET_LINESPERBEAT0  = 0x00, // 
				SET_LINESPERBEAT1  = 0x10, // Range from FE00 to FE1F is reserved for changing lines per beat.
				SET_BYPASS         = 0x20,
				SET_MUTE           = 0x30,
				PATTERN_LOOP       = 0xB0, // Loops the current pattern x times. 0xFEB0 sets the loop start point.
				PATTERN_DELAY      = 0xD0, // causes a "pause" of x rows ( i.e. the current row becomes x rows longer)
				FINE_PATTERN_DELAY = 0xF0  // causes a "pause" of x ticks ( i.e. the current row becomes x ticks longer)
			};


			typedef std::vector< std::pair<std::uint8_t,std::uint8_t> > PcmListType;
			typedef std::pair<std::uint8_t,std::uint8_t> PcmType;

			PatternEvent();

			~PatternEvent();

			void setNote(std::uint8_t value);
			std::uint8_t note() const;

			void setInstrument(std::uint8_t instrument);
			std::uint8_t instrument() const;

			void setMachine(std::uint8_t machine);
			std::uint8_t machine() const;

			void setCommand(std::uint8_t command);
			std::uint8_t command() const;

			void setParameter(std::uint8_t parameter);
			std::uint8_t parameter() const;

			void setVolume(std::uint8_t parameter);
			std::uint8_t volume() const;

			bool empty() const;

			std::string toXml(int track) const;

			PcmListType & paraCmdList();

			void setSharp( bool on );
			bool isSharp() const;

		private:

			std::uint8_t _note;
			std::uint8_t _inst;
			std::uint8_t _mach;
			std::uint8_t _cmd;
			std::uint8_t _parameter;
			std::uint8_t _volume;
			bool _sharp;
			
			PcmListType paraCmdList_;

		};
	}
}

#endif
