#include "stdafx.h"

	#include "player.h"
	#include "Song.h"
	#include "Machine.h"
	#include "Configuration.h"

#if !defined(_WINAMP_PLUGIN_)
	#include "MidiInput.h"
#endif // _WINAMP_PLUGIN_

#include "InputHandler.h"

Player::Player()
{
	_playing = false;
	_playBlock = false;
	_recording = false;
	_ticksRemaining=0;
	_lineCounter=0;
	_loopSong=true;
	tpb=4;
	bpm=125;
	for (int i=0;i<MAX_TRACKS;i++) 
	{
		prevMachines[i]=255;
	}
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

	((Master*)(Global::_pSong->_pMachine[MASTER_INDEX]))->_clip = false;
	((Master*)(Global::_pSong->_pMachine[MASTER_INDEX]))->sampleCount = 0;
	_lineChanged = true;
	_lineCounter = line;
	_playPosition= pos;
	_playPattern = Global::_pSong->playOrder[_playPosition];
	_playTime = 0;
	_playTimem = 0;
	bpm=Global::_pSong->BeatsPerMin;
	tpb=Global::_pSong->_ticksPerBeat;
	
	for (int i=0;i<MAX_TRACKS;i++) 
	{
		prevMachines[i] = 255;
	}
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
		if(Global::_pSong->_pMachine[i])
		{
			Global::_pSong->_pMachine[i]->Stop();
			for (int c = 0; c < MAX_TRACKS; c++)
			{
				Global::_pSong->_pMachine[i]->TriggerDelay[c]._cmd = 0;
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

	unsigned char* const plineOffset = pSong->_ptrackline(_playPattern,0,_lineCounter);

	// Initial Loop. Check for Tracker Commands.
	for (int track=0; track<pSong->SONGTRACKS; track++)
	{
		PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*EVENT_SIZE);
		
		if ( pEntry->_note < cdefTweakM || pEntry->_note == 255 ) // Check for Global Command.
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
				if ( pEntry->_mach == 255 )
				{
					((Master*)(pSong->_pMachine[MASTER_INDEX]))->_outDry = pEntry->_parameter;
				}
				else 
				{
					int mIndex = pEntry->_mach;
					if (mIndex < MAX_MACHINES)
					{
						if (pSong->_pMachine[mIndex] )
						{
							pSong->_pMachine[mIndex]->SetDestWireVolume(mIndex,pEntry->_inst,CValueMapper::Map_255_1(pEntry->_parameter));
						}
					}
				}
				break;

			case  0xF8:
				int mIndex = pEntry->_mach;
				if (mIndex < MAX_MACHINES)
				{
					if (pSong->_pMachine[mIndex] )
					{
						pSong->_pMachine[mIndex]->SetPan(pEntry->_parameter>>1);
					}
				}

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

				if (mac < MAX_MACHINES)
				{
					Machine *pMachine = pSong->_pMachine[mac];
					if (pMachine)
					{
						pMachine->Tick(track, pEntry);
					}
				}
			}
		}
	}

	// Notify all machines that a new Tick() comes.
	for (int tc=0; tc<MAX_MACHINES; tc++)
	{
		if(pSong->_pMachine[tc])
		{
			pSong->_pMachine[tc]->Tick();
			for (int c = 0; c < MAX_TRACKS; c++)
			{
				pSong->_pMachine[tc]->TriggerDelay[c]._cmd = 0;
			}
		}
	}

	// Finally, loop again checking for Notes
	//
	for (int track=0; track<pSong->SONGTRACKS; track++)
	{
		PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*5);

		if (( !pSong->_trackMuted[track]) &&
			( pEntry->_note < cdefTweakM || pEntry->_note == 255 ))
		{
			int mac = pEntry->_mach;
			if (( mac != 255 || prevMachines[track] != 255) &&
				(pEntry->_note != 255 || pEntry->_cmd != 0x00))
			{
				if ( mac != 255 ) prevMachines[track] = mac;
				else mac = prevMachines[track];

				if (mac < MAX_MACHINES)
				{
					Machine *pMachine = pSong->_pMachine[mac];
					if (pMachine)
					{
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


#if defined(_WINAMP_PLUGIN_)
		_playPosition++;

		if (_playPosition >= pSong->playLength)
		{
			_playing= false;
		}
#else
		
		if(!_playBlock)
			_playPosition++;
		else
		{
			_playPosition++;
			while (_playPosition< pSong->playLength && (!pSong->playOrderSel[_playPosition]))
				_playPosition++;
		}
		
		if ( _playPosition >= pSong->playLength)
		{	
			if (_recording) // Don't loop the recording
			{
				StopRecording();
			}
			if ( _loopSong )
			{
				_playPosition = 0;
				if (( _playBlock ) && (pSong->playOrderSel[_playPosition] == false))
				{
					while ( (!pSong->playOrderSel[_playPosition]) &&
						( _playPosition< pSong->playLength)) _playPosition++;
				}
			}
			else 
			{
				_playing = false;
				_playBlock =false;
			}
		}
#endif // _WINAMP_PLUGIN
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
				if (pSong->_pMachine[c])
				{
					pSong->_pMachine[c]->PreWork(amount);
				}
			}
			pSong->_pMachine[MASTER_INDEX]->Work(amount);

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
					if (pSong->_pMachine[c])
					{
						pSong->_pMachine[c]->_wireCost = 0;
						pSong->_pMachine[c]->_cpuCost = 0;
					}
				}
			}

			for (int c=0; c<MAX_MACHINES; c++)
			{
				if (pSong->_pMachine[c])
				{
					pSong->_pMachine[c]->PreWork(amount);
				}
			}

			if ( pSong->PW_Stage == 1 )
			{
			// Mixing preview WAV
			//
				pSong->PW_Work(pSong->_pMachine[MASTER_INDEX]->_pSamplesL,pSong->_pMachine[MASTER_INDEX]->_pSamplesR, amount);
			}
			
			//////////////////////////////////////////////////////////////////////
			// Inject Midi input data
			if( !CMidiInput::Instance()->InjectMIDI( amount ) )
			{
				// if midi not enabled we just do the original tracker thing

				// Master machine initiates work
				//
				pSong->_pMachine[MASTER_INDEX]->Work(amount);
			}

			CPUCOST_CALC(idletime, amount);

			pSong->cpuIdle = idletime;
			pSong->_sampCount += amount;

			if ((pThis->_playing) && (pThis->_recording))
			{
//				float* pData = pThis->_pBuffer; <- this was fuxxxxing up
				float* pL = pSong->_pMachine[MASTER_INDEX]->_pSamplesL;
				float* pR = pSong->_pMachine[MASTER_INDEX]->_pSamplesR;
				int i;
				
				switch (Global::pConfig->_pOutputDriver->_channelmode)
				{
				case 0: // mono mix
					for (i=0; i<amount; i++)
					{
						if (pThis->_outputWaveFile.WriteMonoSample(((*pL++)+(*pR++))/2) != DDC_SUCCESS)
						{
							pThis->StopRecording(false);
						}
					}
					break;
				case 1: // mono L
					for (i=0; i<amount; i++)
					{
						if (pThis->_outputWaveFile.WriteMonoSample((*pL++)) != DDC_SUCCESS)
						{
							pThis->StopRecording(false);
						}
					}
					break;
				case 2: // mono R
					for (i=0; i<amount; i++)
					{
						if (pThis->_outputWaveFile.WriteMonoSample((*pR++)) != DDC_SUCCESS)
						{
							pThis->StopRecording(false);
						}
					}
					break;
				default: // stereo
					for (i=0; i<amount; i++)
					{
						if (pThis->_outputWaveFile.WriteStereoSample((*pL++),(*pR++)) != DDC_SUCCESS)
						{
							pThis->StopRecording(false);
						}
					}
					break;
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
	while (numSamplex>0);

	return pThis->_pBuffer;
}

#if !defined(_WINAMP_PLUGIN_)

void Player::StartRecording(
	char *psFilename,
	int bitdepth, int samplerate, int channelmode)
{
	if (!_recording)
	{
		backup_rate = Global::pConfig->_pOutputDriver->_samplesPerSec;
		backup_bits = Global::pConfig->_pOutputDriver->_bitDepth;
		backup_channelmode = Global::pConfig->_pOutputDriver->_channelmode;

		if (samplerate > 0)
		{
			Global::pConfig->_pOutputDriver->_samplesPerSec = samplerate;
		}

		if (bitdepth > 0)
		{
			Global::pConfig->_pOutputDriver->_bitDepth = bitdepth;
		}

		if (channelmode >= 0)
		{
			Global::pConfig->_pOutputDriver->_channelmode = channelmode;
		}

		int channels = 2;
		if (Global::pConfig->_pOutputDriver->_channelmode != 3)
		{
			channels = 1;
		}

		Global::_pSong->SamplesPerTick = (Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb);

		Stop();
		if (_outputWaveFile.OpenForWrite(psFilename, Global::pConfig->_pOutputDriver->_samplesPerSec, Global::pConfig->_pOutputDriver->_bitDepth, channels) == DDC_SUCCESS)
		{
			_recording = true;
		}
		else
		{
			StopRecording();
			MessageBox(NULL,psFilename,"FAILED",NULL);
		}
	}
}

void Player::StopRecording(bool bOk)
{
	if (_recording)
	{
		Global::pConfig->_pOutputDriver->_samplesPerSec = backup_rate;
		Global::pConfig->_pOutputDriver->_bitDepth = backup_bits;
		Global::pConfig->_pOutputDriver->_channelmode = backup_channelmode;
		Global::_pSong->SamplesPerTick = (Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb);

		_outputWaveFile.Close();
		_recording = false;
		if (!bOk)
		{
			MessageBox(NULL,"Wav recording failed.","ERROR",MB_OK);
		}
	}
}
#endif // ndef _WINAMP_PLUGIN_

void Player::SetSampleRate(int samprate)
{
	// TODO: Add notifications to all machines that the sample Rate has changed
	// samplerate = samprate;
	// for (Allmachines) change it
}

