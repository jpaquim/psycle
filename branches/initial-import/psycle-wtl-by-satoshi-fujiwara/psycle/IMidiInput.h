#pragma once
/** @file
 *  @brief Psy2SongLoader Implement Class.
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.1 $
 */

namespace model {
struct ATL_NO_VTABLE IMidiInput
{
	virtual ~IMidiInput() = 0;
	virtual void SetDeviceId( const int driver,const UINT devId ) = 0;	// set MIDI input device identifier
	virtual const bool Open() = 0;				// open the midi input devices
	virtual const bool Sync() = 0;				// start input (reset timestamps)
	virtual void ReSync() = 0;			// resync the MIDI with the audio engine
	virtual const bool Close() = 0;				// close the midi input device
	virtual const bool Active() = 0;		// find out if we are open
};

}
