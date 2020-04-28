// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_WIRE_H
#define psy_audio_WIRE_H

#include <stdlib.h>

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

// typedef std::pair<short, short> PinConnection;
// typedef std::vector<PinConnection> Mapping;

typedef struct LegacyWire
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
	///\}
} LegacyWire;

INLINE void psy_audio_legacywire_init_all(LegacyWire* self,
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
}

INLINE LegacyWire* psy_audio_legacywire_alloc(void)
{
	return (LegacyWire*) malloc(sizeof(LegacyWire));
}

INLINE LegacyWire* psy_audio_legacywire_allocinit_all(
	int inputmachine,
	bool inputcon,
	float inputconvol,
	float wiremultiplier,
	int outputmachine,
	bool connection)
{
	LegacyWire* rv;

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
	
#ifdef __cplusplus
}
#endif

#endif /* psy_audio_WIRE_H */
