#include "stdafx.h"
//#include "machineinterface.h"

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
	_lineCounter=0;
	tpb=4;
	bpm=125;
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

void Player::Start(int pos, int line)
{
	Stop(); // This causes all machines to reset, and samplespertick to init.

	_lineChanged = true;
	_lineCounter = line;
	_playPosition= pos;
	_playPattern = Global::_pSong->playOrder[_playPosition];
	_playTime = 0;
	_playTimem = 0;
	bpm=Global::_pSong->BeatsPerMin;
	tpb=Global::_pSong->_ticksPerBeat;
	
	for (int i=0;i<MAX_TRACKS;i++) prevMachines[i] = 0;
	_playing = true;
	ExecuteLine();
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
			for(int c = 0; c < MAX_TRACKS; c++)
			{
				Global::_pSong->_pMachines[i]->TriggerDelay[c]._cmd = 0;
			}
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
					bpm=pEntry->_parameter;//+0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
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
						pSong->_pMachines[mIndex]->SetDestWireVolume(mIndex,pEntry->_inst,pEntry->_parameter);
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
			for (int c = 0; c < MAX_TRACKS; c++)
			{
				pSong->_pMachines[tc]->TriggerDelay[c]._cmd = 0;
			}
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
					
					if (pEntry->_cmd == 0xfd)
					{
						// delay
						memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
						pMachine->TriggerDelayCounter[track] = ((pEntry->_parameter+1)*Global::_pSong->SamplesPerTick)/256;
					}
					else if (pEntry->_cmd == 0xfb)
					{
						// retrigger
						memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
						pMachine->RetriggerRate[track] = (pEntry->_parameter+1);
						pMachine->TriggerDelayCounter[track] = 0;
					}
					else if (pEntry->_cmd == 0xfa)
					{
						// retrigger continue
						memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
						if (pEntry->_parameter&0xf0)
						{
							pMachine->RetriggerRate[track] = (pEntry->_parameter&0xf0);
						}
					}
					else
					{
						pMachine->TriggerDelay[track]._cmd = 0;
						pMachine->Tick(track, pEntry);
						pMachine->TriggerDelayCounter[track] = 0;
					}
				}
			}
		}
	}
	_ticksRemaining = pSong->SamplesPerTick;
}	


void Player::AdvancePosition()
{
	Song* pSong = Global::_pSong;

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
			while ((_playPosition< MAX_SONG_POSITIONS) && 
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
		if ((pThis->_ticksRemaining <=0) && (pThis->_playing))
		{
			// Advance position in the sequencer
			//
			pThis->AdvancePosition();
			pThis->ExecuteLine();
	
		}

		//////////////////////////////////////////////////////////////////////
		// Processing plant
		
		if (amount > 0)
		{
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
			CSingleLock crit(&Global::_pSong->door, TRUE);
			
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
					else sl = f2i(*pData);
					*pData++;
					if ( *pData > 32767.0) sr = 32767;
					else if ( *pData < -32768.0 ) sr = -32768;
					else sr = f2i(*pData);
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


