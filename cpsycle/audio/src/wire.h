// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_WIRE_H
#define psy_audio_WIRE_H

#include "connections.h"
#include <hashtbl.h>
#include <stdlib.h>
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

// typedef std::pair<short, short> PinConnection;
// typedef std::vector<PinConnection> Mapping;

typedef struct psy_audio_LegacyWire
{
	///\name input ports
	///\{
		/// Incoming connections Machine number
		int _inputMachine;
		/// Incoming connections activated
		bool _inputCon;
		/// Incoming connections Machine vol
		float _inputConVol;
		/// Value to multiply _inputConVol[] with to have a 0.0...1.0 range
		// The reason of the _wireMultiplier variable is because VSTs output wave data
		// in the range -1.0 to +1.0, while natives and internals output at -32768.0 to +32768.0
		// Initially (when the format was made), Psycle did convert this in the "Work" function,
		// but since it already needs to multiply the output by inputConVol, I decided to remove
		// that extra conversion and use directly the volume to do so.
		float _wireMultiplier;
		//Pin mapping (for loading)
		// Wire::Mapping pinMapping;
		///\}

		///\name output ports
		///\{
			/// Outgoing connections Machine number
		int _outputMachine;
		/// Outgoing connections activated
		bool _connection;
		/// PinMapping
		psy_audio_PinMapping pinmapping;
	///\}
} psy_audio_LegacyWire;

INLINE void psy_audio_legacywire_init(psy_audio_LegacyWire* self)
{
	self->_inputMachine = -1;
	self->_inputCon = FALSE;
	self->_inputConVol = 1.f;;
	self->_wireMultiplier = 1.f;
	self->_outputMachine = -1;
	self->_connection = FALSE;
	psy_audio_pinmapping_init(&self->pinmapping, 2);
}

INLINE void psy_audio_legacywire_init_all(psy_audio_LegacyWire* self,
	int inputmachine,
	bool inputcon,
	float inputconvol,
	float wiremultiplier,
	int outputmachine,
	bool connection)
{
	self->_inputMachine = inputmachine;
	self->_inputCon = inputcon;
	self->_inputConVol = inputconvol;
	self->_wireMultiplier = wiremultiplier;
	self->_outputMachine = outputmachine;
	self->_connection = connection;
	psy_audio_pinmapping_init(&self->pinmapping, 2);
}

INLINE void psy_audio_legacywire_dispose(psy_audio_LegacyWire* self)
{
	psy_audio_pinmapping_dispose(&self->pinmapping);
}

INLINE psy_audio_LegacyWire* psy_audio_legacywire_alloc(void)
{
	return (psy_audio_LegacyWire*) malloc(sizeof(psy_audio_LegacyWire));
}

INLINE psy_audio_LegacyWire* psy_audio_legacywire_allocinit(void)
{
	psy_audio_LegacyWire* rv;

	rv = psy_audio_legacywire_alloc();
	if (rv) {
		psy_audio_legacywire_init(rv);
	}
	return rv;
}

INLINE psy_audio_LegacyWire* psy_audio_legacywire_allocinit_all(
	int inputmachine,
	bool inputcon,
	float inputconvol,
	float wiremultiplier,
	int outputmachine,
	bool connection)
{
	psy_audio_LegacyWire* rv;

	rv = psy_audio_legacywire_alloc();
	if (rv) {
		psy_audio_legacywire_init_all(rv,
			inputmachine,
			inputcon,
			inputconvol,
			wiremultiplier,
			outputmachine,
			connection);
	}
	return rv;
}

typedef struct psy_audio_LegacyWires {
	// ConnectionID X psy_audio_LegacyWire*
	psy_Table legacywires;
} psy_audio_LegacyWires;

struct psy_audio_SongFile;

void psy_audio_legacywires_init(psy_audio_LegacyWires*);
void psy_audio_legacywires_dispose(psy_audio_LegacyWires*);
psy_Table* psy_audio_legacywires_at(psy_audio_LegacyWires*,
	uintptr_t machineslot);
int psy_audio_legacywires_findlegacyoutput(psy_audio_LegacyWires*,
	int sourcemac, int macindex);
void legacywires_load_psy2(struct psy_audio_SongFile*, uintptr_t slot);
	
#ifdef __cplusplus
}
#endif

#endif /* psy_audio_WIRE_H */
