#pragma once

#include "pattern.h"

///\todo: any real reason why this isn't inside pattern.h?

namespace psy
{
	namespace core
	{
		namespace notetypes
		{
			enum notetype {
				c0 = 0,
				b9 = 119,
				release = 120,
				tweak,
				tweak_effect, ///< old. for compatibility only.
				midi_cc,
				tweak_slide,
				// maintain these two as the last ones
				invalid,
				empty = 255
			};
		}
		namespace commandtypes
		{
			enum commandtype
			{
				EXTENDED      = 0xFE, // see below
				SET_TEMPO     = 0xFF,
				NOTE_DELAY    = 0xFD,
				RETRIGGER     = 0xFB,
				RETR_CONT     = 0xFA,
				SET_VOLUME    = 0xFC,
				SET_PANNING   = 0xF8,
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
		}
	}
}
