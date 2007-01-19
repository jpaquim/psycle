/***************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper   *
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

#include "tr1stdint.h"
#include <vector>

/**
@author  Stefan Nattkemper
*/

namespace psycle
{
  namespace core
  {

    class PatternEvent{
    public:

      enum CmdSet
      {
        cdefNull = -1,
        cdefKeyC_0, 
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
        cdefKeyC_1, 
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
        cdefKeyC_2, 
        cdefKeyCS2,
        cdefKeyD_2,
        cdefKeyDS2,
        cdefKeyE_2,
        cdefKeyF_2,
        cdefKeyFS2,
        cdefKeyG_2,
        cdefKeyGS2,
        cdefKeyA_2,	
        cdefKeyStop = 120,
        cdefTweakM = 121,
        cdefTweakE = 122,
        cdefMIDICC = 123,
        cdefTweakS = 124,
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


      typedef std::vector< std::pair<psycle::tr1::uint8_t,psycle::tr1::uint8_t> > PcmListType;
      typedef std::pair<psycle::tr1::uint8_t,psycle::tr1::uint8_t> PcmType;

      PatternEvent();

      ~PatternEvent();

      void setNote(psycle::tr1::uint8_t value);
      psycle::tr1::uint8_t note() const;

      void setInstrument(psycle::tr1::uint8_t instrument);
      psycle::tr1::uint8_t instrument() const;

      void setMachine(psycle::tr1::uint8_t machine);
      psycle::tr1::uint8_t machine() const;

      void setCommand(psycle::tr1::uint8_t command);
      psycle::tr1::uint8_t command() const;

      void setParameter(psycle::tr1::uint8_t parameter);
      psycle::tr1::uint8_t parameter() const;

      void setVolume(psycle::tr1::uint8_t parameter);
      psycle::tr1::uint8_t volume() const;

      bool empty() const;

      std::string toXml(int track) const;

      PcmListType & paraCmdList();

      void setSharp( bool on );
      bool isSharp() const;

    private:

      psycle::tr1::uint8_t _note;
      psycle::tr1::uint8_t _inst;
      psycle::tr1::uint8_t _mach;
      psycle::tr1::uint8_t _cmd;
      psycle::tr1::uint8_t _parameter;
      psycle::tr1::uint8_t _volume;
      bool _sharp;

      PcmListType paraCmdList_;

    };
  }
}

#endif
