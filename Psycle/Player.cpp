#include "stdafx.h"

#if defined(_WINAMP_PLUGIN_)
	#include "global.h"
	#include "player.h"
	#include "Song.h"
	#include "Machine.h"
	#include "Configuration.h"
#else 
	#include "player.h"
	#include "Song.h"
	#include "Machine.h"
	#include "Configuration.h"
	#include "MidiInput.h"
#endif // _WINAMP_PLUGIN_


Player::Player()
{
	_playing = false;
	_playBlock = false;
	_recording = false;
	_ticksRemaining=0;
	for (int i=0;i<MAX_TRACKS;i++) prevMachines[i]=255;
}

Player::~Player()
{
#if !defined(_WINAMP_PLUGIN_)
	if (_recording)
	{
		_outputWaveFile.Close();
	}
#endif // ndef _WINAMP_PLUGIN_
}

void Player::Start(int line)
{
	Stop();
	_lineChanged = true;
	_lineCounter = line;
	_playTime = 0;
	_playTimem = 0;
#if defined(_WINAMP_PLUGIN_)
	_playPosition=0;
#else
	if(_playPosition<0)	_playPosition=0;	// DAN FIXME
#endif // _WINAMP_PLUGIN_
	for (int i=0;i<MAX_TRACKS;i++) prevMachines[i] = 0;
	_playPattern = Global::_pSong->playOrder[_playPosition];
	_ticksRemaining = Global::_pSong->SamplesPerTick;
	bpm=Global::_pSong->BeatsPerMin;
	tpb=Global::_pSong->_ticksPerBeat;
	_playing = true;
}

void Player::Stop(void)
{
	// Stop song enviroment
	_playing = false;
	_playBlock =false;
	
	for (int i=0; i<MAX_MACHINES; i++)
	{
		if(Global::_pSong->_machineActive[i])
		{
			Global::_pSong->_pMachines[i]->Stop();
		}
	}
#if defined(_WINAMP_PLUGIN_)
	Global::_pSong->SamplesPerTick = (Global::pConfig->_samplesPerSec*15*4)/(Global::_pSong->BeatsPerMin*Global::_pSong->_ticksPerBeat);
#else
	Global::_pSong->SamplesPerTick = (Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(Global::_pSong->BeatsPerMin*Global::_pSong->_ticksPerBeat);
#endif
}

void Player::ExecuteLine(void)
{
	Song* pSong = Global::_pSong;
	if ((_ticksRemaining <=0) && (_playing))
	{
		_lineChanged = true;

		unsigned char* const plineOffset = pSong->pPatternData + _playPattern*MULTIPLY2+_lineCounter*MULTIPLY;

		// Initial Loop. Check for Tracker Commands.
		for (int track=0; track<pSong->SONGTRACKS; track++)
		{
			PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*5);
			
			if ( pEntry->_note < 121 || pEntry->_note == 255 ) // Check for Global Command.
			{
				switch(pEntry->_cmd)
				{
				case 0xFF:
					if ( pEntry->_parameter != 0 )
					{
						bpm=pEntry->_parameter;
#if defined(_WINAMP_PLUGIN_)
						Global::_pSong->SamplesPerTick = (Global::pConfig->_samplesPerSec*15*4)/(bpm*tpb);
#else
						Global::_pSong->SamplesPerTick = (Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(bpm*tpb);
#endif
					}
					break;
				
				case 0xFE:
					if ( pEntry->_parameter != 0 )
					{
						tpb=pEntry->_parameter;
#if defined(_WINAMP_PLUGIN_)
						Global::_pSong->SamplesPerTick = (Global::pConfig->_samplesPerSec*15*4)/(bpm*tpb);
#else
						Global::_pSong->SamplesPerTick = (Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(bpm*tpb);
#endif
					}
					break;

				case 0xFC:
					if ( pEntry->_mach == 255 ) pSong->_pMachines[0]->_outDry = pEntry->_parameter;
					else 
					{
						int mIndex;
						if ( pEntry->_mach < MAX_BUSES ) /*Gen*/ mIndex = pSong->busMachine[pEntry->_mach];
						else /*Fx*/ mIndex = pSong->busEffect[(pEntry->_mach & (MAX_BUSES-1))];

						if ( mIndex < MAX_MACHINES && pSong->_machineActive[mIndex] )
							pSong->_pMachines[mIndex]->SetWireVolume(mIndex,pEntry->_inst,pEntry->_parameter);
					}
					break;

				case  0xF8:
					int mIndex;
					if ( pEntry->_mach < MAX_BUSES ) /*Gen*/ mIndex = pSong->busMachine[pEntry->_mach];
					else /*Fx*/ mIndex = pSong->busEffect[(pEntry->_mach & (MAX_BUSES-1))];

					if ( mIndex < MAX_MACHINES && pSong->_machineActive[mIndex] )
						pSong->_pMachines[mIndex]->SetPan(pEntry->_parameter>>1);

					break;
				}
			}
			else if ( !pSong->_trackMuted[track] ) // Check For Tweak or MIDI CC
			{
				int mac = pEntry->_mach;
				if (( mac != 255) || (prevMachines[track] != 255))
				{
					if ( mac != 255 ) prevMachines[track] = mac;
					else mac = prevMachines[track];

					int mIndex;
					if ( mac & MAX_BUSES ) mIndex = pSong->busEffect[(mac&(MAX_BUSES-1))];
					else if ( pEntry->_note == 122 ) mIndex = pSong->busEffect[(mac&(MAX_BUSES-1))];
					else mIndex = pSong->busMachine[(mac&(MAX_BUSES-1))];
					
					if (mIndex < MAX_MACHINES && pSong->_machineActive[mIndex])
					{
						Machine *pMachine = pSong->_pMachines[mIndex];
						
						pMachine->Tick(track, pEntry);
					}
				}
			}
		}

		// Notify all machines that a new Tick() comes.
		for (int tc=0; tc<MAX_MACHINES; tc++)
		{
			if(pSong->_machineActive[tc])
			{
				pSong->_pMachines[tc]->Tick();
			}
		}

		// Finally, loop again checking for Notes
		//
		for (track=0; track<pSong->SONGTRACKS; track++)
		{
			PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*5);

			if (( !pSong->_trackMuted[track]) &&
				( pEntry->_note < 121 || pEntry->_note == 255 ))
			{
				int mac = pEntry->_mach;
				if (( mac != 255 || prevMachines[track] != 255) &&
				    (pEntry->_note != 255 || pEntry->_cmd != 0x00))
				{
					if ( mac != 255 ) prevMachines[track] = mac;
					else mac = prevMachines[track];

					int mIndex;
					if ( mac & MAX_BUSES ) mIndex = pSong->busEffect[(mac&(MAX_BUSES-1))];
					else mIndex = pSong->busMachine[(mac&(MAX_BUSES-1))];
					
					if (mIndex < MAX_MACHINES && pSong->_machineActive[mIndex])
					{
						Machine *pMachine = pSong->_pMachines[mIndex];
						
						pMachine->Tick(track, pEntry);
					}
				}
			}
		}
		
		// Advance position in the sequencer
		//
		_ticksRemaining = pSong->SamplesPerTick;
		_lineCounter++;
		_playTime += 60 / float (bpm * tpb);
		if (_playTime>60)
		{
			_playTime-=60;
			_playTimem++;
		}

		if (_lineCounter >= pSong->patternLines[_playPattern])
		{
			_lineCounter = 0;
			_playPosition++;

#if defined(_WINAMP_PLUGIN_)
			if (_playPosition >= pSong->playLength)
			{
				_playing= false;
			}
#else
			if ( _playBlock )
			{
				while (( _playPosition< MAX_SONG_POSITIONS) && 
					    (!pSong->playOrderSel[_playPosition])) _playPosition++;
			}
			
			if ((!_playBlock && _playPosition >= pSong->playLength) ||
				(_playBlock && _playPosition == MAX_SONG_POSITIONS))
			{	// Don't loop the recording
				//
				if (_recording)
				{
					StopRecording();
				}
				_playPosition = 0;
				if (( _playBlock ) && (pSong->playOrderSel[_playPosition] == false)) {
					while ( (!pSong->playOrderSel[_playPosition]) &&
							( _playPosition< pSong->playLength)) _playPosition++;
				}
			}
#endif // _WINAMP_PLUGIN_

			_playPattern = pSong->playOrder[_playPosition];
		}
	}
}

float * Player::Work(
	void* context,
	int& numSamples)
{
	int amount;
	Player* pThis = (Player*)context;
	Song* pSong = Global::_pSong;
	Master::_pMasterSamples = pThis->_pBuffer;
	int numSamplex = numSamples;
	do
	{
		if (numSamplex > STREAM_SIZE)
		{
			amount = STREAM_SIZE;
		}
		else
		{
			amount = numSamplex;
		}

		//////////////////////////////////////////////////////////////////////
		// Tick handler function

		if ((pThis->_playing) && (amount >= pThis->_ticksRemaining))
		{
			amount = pThis->_ticksRemaining;
		}

		//////////////////////////////////////////////////////////////////////
		//	Song play

		pThis->ExecuteLine();

		//////////////////////////////////////////////////////////////////////
		// Processing plant
		
		if (amount > 0)
		{

			CSingleLock crit(&Global::_pSong->door, TRUE);

#if defined(_WINAMP_PLUGIN_)
			for (int c=0; c<MAX_MACHINES; c++)
			{
				if (pSong->_machineActive[c])
				{
					pSong->_pMachines[c]->PreWork(amount);
				}
			}
			pSong->_pMachines[0]->Work(amount);

#else

			CPUCOST_INIT(idletime);
			// Reset all machines
			//
			if ( (int)pSong->_sampCount > Global::pConfig->_pOutputDriver->_samplesPerSec )
			{
				pSong->_sampCount =0;
				for (int c=0; c<MAX_MACHINES; c++)
				{
					if (pSong->_machineActive[c])
					{
						pSong->_pMachines[c]->_wireCost = 0;
						pSong->_pMachines[c]->_cpuCost = 0;
					}
				}
			}

			for (int c=0; c<MAX_MACHINES; c++)
			{
				if (pSong->_machineActive[c])
				{
					pSong->_pMachines[c]->PreWork(amount);
				}
			}

			if ( pSong->PW_Stage == 1 )
			{
			// Mixing preview WAV
			//
				pSong->PW_Work(pSong->_pMachines[0]->_pSamplesL,pSong->_pMachines[0]->_pSamplesR, amount);
			}
			
			//////////////////////////////////////////////////////////////////////
			// Inject Midi input data
			if( !CMidiInput::Instance()->InjectMIDI( amount ) )
			{
				// if midi not enabled we just do the original tracker thing

				// Master machine initiates work
				//
				pSong->_pMachines[0]->Work(amount);
			}

			CPUCOST_CALC(idletime, amount);

			pSong->cpuIdle = idletime;
			pSong->_sampCount += amount;

			if ((pThis->_playing) && (pThis->_recording))
			{
				short sl,sr;
				float* pData = Master::_pMasterSamples;
				for (int i=0; i<amount; i++)
				{
					
					// It's an ugly solution, I know, but We are recording, so
					// speed is not that crucial
					//
					if ( *pData > 32767.0) sl = 32767;
					else if ( *pData < -32768.0 ) sl = -32768;
					else sl = Dsp::F2I(*pData);
					*pData++;
					if ( *pData > 32767.0) sr = 32767;
					else if ( *pData < -32768.0 ) sr = -32768;
					else sr = Dsp::F2I(*pData);
					*pData++;
					pThis->_outputWaveFile.WriteStereoSample(sl,sr);
				}
			}

#endif // ndef _WINAMP_PLUGIN_

			Master::_pMasterSamples += amount*2;
			numSamplex -= amount;
		}
		if (pThis->_playing)
		{
			pThis->_ticksRemaining -= amount;
		}
	}
	while (numSamplex);
	
	return pThis->_pBuffer;
}

#if !defined(_WINAMP_PLUGIN_)

void Player::StartRecording(
	char *psFilename)
{
	if (!_recording)
	{
		_outputWaveFile.OpenForWrite(psFilename, Global::pConfig->_pOutputDriver->_samplesPerSec, 16, 2);
		_recording = true;
	}
}

void Player::StopRecording(void)
{
	if (_recording)
	{
		_outputWaveFile.Close();
		_recording = false;
	}
}
#endif // ndef _WINAMP_PLUGIN_