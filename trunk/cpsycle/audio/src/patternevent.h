// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_PATTERNEVENT_H)
#define PSY_AUDIO_PATTERNEVENT_H

#include "../../detail/stdint.h"

#include <dsptypes.h>


typedef struct {
	uintptr_t low;
	uintptr_t high; // including
	uintptr_t min;
	uintptr_t max;
} psy_audio_ParameterRange;

void parameterrange_init(psy_audio_ParameterRange*, 
	uintptr_t low, uintptr_t high, uintptr_t min, uintptr_t max);
int parameterrange_intersect(psy_audio_ParameterRange*, uintptr_t value);

typedef	enum {
	EXTENDED	=		0xFE, // (see below)
	SET_TEMPO	=		0xFF,
	NOTE_DELAY	=		0xFD,
	RETRIGGER   =		0xFB,
	RETR_CONT	=		0xFA,
	SET_VOLUME	=		0x0FC,
	SET_PANNING =		0x0F8,
	BREAK_TO_LINE =		0xF2,
	JUMP_TO_ORDER =		0xF3,
	// 0..80: 0 .. 100%
	// FE: tie	
	GATE		=		0xF1,
	ARPEGGIO	=		0xF0,
	// Extended Commands from 0xFE
	SET_LINESPERBEAT0 = 0x00, // 
	SET_LINESPERBEAT1 = 0x10, // Range from FE00 to FE1F is reserved for 
							  // changing lines per beat.
	SET_BYPASS =		0x20,
	SET_MUTE =			0x30,
	PATTERN_LOOP  =		0xB0, // Loops the current pattern x times.
							  // 0xFEB0 sets the loop start point.
	PATTERN_DELAY =		0xD0, // causes a "pause" of x rows 
							  // ( i.e. the current row becomes x rows longer)
	ROW_EXTRATICKS =	0xE0, // causes a "pause" of x ticks for all rows
							  // including this
							  // ( i.e. all rows becomes x ticks longer)
	FINE_PATTERN_DELAY=	0xF0  // causes a "pause" of x ticks ( i.e. the current
							  // row becomes x ticks longer)
} PatternCmd;

typedef	enum {
	NOTECOMMANDS_C0 = 0,   // In MIDI, it is actually c minus 1
	NOTECOMMANDS_MIDDLEC = 60,
	NOTECOMMANDS_MIDDLEA = 69,
	NOTECOMMANDS_B9 = 119, // In MIDI, it is actualy b8
	NOTECOMMANDS_RELEASE = 120,
	NOTECOMMANDS_TWEAK,
	NOTECOMMANDS_TWEAKEFFECT, //old. for compatibility only.
	NOTECOMMANDS_MIDICC,
	NOTECOMMANDS_TWEAKSLIDE,
	// Place whatever that can be written in the pattern above invalid, and
	// anything else below it
	NOTECOMMANDS_INVALID,
	NOTECOMMANDS_MIDI_SYNC = 254,
	NOTECOMMANDS_EMPTY = 255,
	NOTECOMMANDS_INST_EMPTY = 0xFFFF,
	NOTECOMMANDS_MACH_EMPTY = 0xFF,
	NOTECOMMANDS_VOL_EMPTY = 0xFF,
	NOTECOMMANDS_GATE_EMPTY = 0xFF
} NoteCommands;

typedef struct {
	uint8_t note;	
	uint16_t inst;	
	uint8_t mach;
	uint8_t vol;	
	uint8_t cmd;
	uint8_t parameter;	
} psy_audio_PatternEvent;

void patternevent_init_all(psy_audio_PatternEvent*,
	uint8_t note,
	uint16_t inst,
	uint8_t mach,
	uint8_t vol,
	uint8_t cmd,
	uint8_t parameter);
void patternevent_clear(psy_audio_PatternEvent*);
int patternevent_empty(psy_audio_PatternEvent*);

#endif
