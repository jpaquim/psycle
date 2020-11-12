// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_PATTERNEVENT_H
#define psy_audio_PATTERNEVENT_H

#include "../../detail/stdint.h"

#include <dsptypes.h>
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	uintptr_t low;
	uintptr_t high; // including
	uintptr_t min;
	uintptr_t max;
} psy_audio_ParameterRange;

void psy_audio_parameterrange_init(psy_audio_ParameterRange*, 
	uintptr_t low, uintptr_t high, uintptr_t min, uintptr_t max);
int psy_audio_parameterrange_intersect(psy_audio_ParameterRange*, uintptr_t value);

typedef	enum {
	psy_audio_PATTERNCMD_EXTENDED	=         0xFE, // (see below)
	psy_audio_PATTERNCMD_SET_TEMPO	=         0xFF,
	psy_audio_PATTERNCMD_NOTE_DELAY	=         0xFD,
	psy_audio_PATTERNCMD_RETRIGGER =          0xFB,
	psy_audio_PATTERNCMD_RETR_CONT	=         0xFA,
	psy_audio_PATTERNCMD_SET_VOLUME	=         0x0FC,
	psy_audio_PATTERNCMD_SET_PANNING =        0x0F8,
	psy_audio_PATTERNCMD_BREAK_TO_LINE =      0xF2,
	psy_audio_PATTERNCMD_JUMP_TO_ORDER =      0xF3,
	// 0..80: 0 .. 100%
	// FE: tie	
	psy_audio_PATTERNCMD_GATE		=		  0xF1,
	psy_audio_PATTERNCMD_ARPEGGIO	=		  0xF0,
	// Extended Commands from 0xFE
	psy_audio_PATTERNCMD_SET_LINESPERBEAT0 =  0x00, // 
	psy_audio_PATTERNCMD_SET_LINESPERBEAT1 =  0x10, // Range from FE00 to FE1F is reserved for 
							                        // changing lines per beat.
	psy_audio_PATTERNCMD_SET_BYPASS =		  0x20,
	psy_audio_PATTERNCMD_SET_MUTE =			  0x30,
	psy_audio_PATTERNCMD_PATTERN_LOOP  =	  0xB0, // Loops the current pattern x times.
							                        // 0xFEB0 sets the loop start point.
	psy_audio_PATTERNCMD_PATTERN_DELAY =	  0xD0, // causes a "pause" of x rows 
							                        // ( i.e. the current row becomes x rows longer)
	psy_audio_PATTERNCMD_ROW_EXTRATICKS =	  0xE0, // causes a "pause" of x ticks for all rows
							                        // including this
							                        // ( i.e. all rows becomes x ticks longer)
	psy_audio_PATTERNCMD_FINE_PATTERN_DELAY = 0xF0  // causes a "pause" of x ticks ( i.e. the current
							                        // row becomes x ticks longer)
} psy_audio_PatternCmd;

typedef	enum {
	psy_audio_NOTECOMMANDS_C0 = 0,   // In MIDI, it is actually c minus 1
	psy_audio_NOTECOMMANDS_MIDDLEC = 60,
	psy_audio_NOTECOMMANDS_MIDDLEA = 69,
	psy_audio_NOTECOMMANDS_B9 = 119, // In MIDI, it is actualy b8
	psy_audio_NOTECOMMANDS_RELEASE = 120,
	psy_audio_NOTECOMMANDS_TWEAK,
	psy_audio_NOTECOMMANDS_TWEAKEFFECT, //old. for compatibility only.
	psy_audio_NOTECOMMANDS_MIDICC,
	psy_audio_NOTECOMMANDS_TWEAKSLIDE,
	// Place whatever that can be written in the pattern above invalid, and
	// anything else below it
	psy_audio_NOTECOMMANDS_INVALID,
	psy_audio_NOTECOMMANDS_MIDI_SYNC = 254,
	psy_audio_NOTECOMMANDS_EMPTY = 255,
	psy_audio_NOTECOMMANDS_INST_EMPTY = 0xFFFF,
	psy_audio_NOTECOMMANDS_MACH_EMPTY = 0xFF,
	psy_audio_NOTECOMMANDS_VOL_EMPTY = 0xFF,
	psy_audio_NOTECOMMANDS_GATE_EMPTY = 0xFF
} psy_audio_NoteCommands;

typedef struct psy_audio_PatternEvent {
	uint8_t note;	
	uint16_t inst;	
	uint8_t mach;
	uint16_t vol;	
	uint8_t cmd;
	uint8_t parameter;	
} psy_audio_PatternEvent;

void psy_audio_patternevent_init_all(psy_audio_PatternEvent*,
	uint8_t note,
	uint16_t inst,
	uint8_t mach,
	uint8_t vol,
	uint8_t cmd,
	uint8_t parameter);

void psy_audio_patternevent_clear(psy_audio_PatternEvent*);
int psy_audio_patternevent_empty(const psy_audio_PatternEvent*);
int psy_audio_patternevent_empty_except_volume(const psy_audio_PatternEvent*);
int psy_audio_patternevent_has_volume(const psy_audio_PatternEvent*);
// pattern value starting at zero
INLINE uint16_t psy_audio_patternevent_tweakvalue(const psy_audio_PatternEvent* self)
{		
	return ((uint16_t)(self->cmd) << 8) | ((uint16_t)(self->parameter) & 0xFF);
}
// pattern value starting at zero
INLINE void psy_audio_patternevent_settweakvalue(psy_audio_PatternEvent* self, uint16_t tweakval)
{		
	self->cmd = (uint8_t) ((tweakval >> 8) & 0xFF);
	self->parameter = (uint8_t)(tweakval & 0xFF);
	
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_PATTERNEVENT_H */
