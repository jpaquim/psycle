/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.7 $
 */
#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

	#include "player.h"
	#include "Song.h"
	#include "Instrument.h"
	#include "Machine.h"
	#include "configuration.h"

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

	((Master*)(Global::_pSong->pMachine(MASTER_INDEX)))->_clip = false;
	((Master*)(Global::_pSong->pMachine(MASTER_INDEX)))->sampleCount = 0;
	_lineChanged = true;
	_lineCounter = line;
	_playPosition= pos;
	_playPattern = Global::_pSong->PlayOrder(_playPosition);
	_playTime = 0;
	_playTimem = 0;
	bpm=Global::_pSong->BeatsPerMin();
	tpb=Global::_pSong->TicksPerBeat();
	
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
		if(Global::_pSong->pMachine(i))
		{
			Global::_pSong->pMachine(i)->Stop();
			for (int c = 0; c < MAX_TRACKS; c++)
			{
				Global::_pSong->pMachine(i)->TriggerDelay[c]._cmd = 0;
			}
		}
	}
	Global::_pSong->SamplesPerTick(
		(Global::pConfig->_pOutputDriver->_samplesPerSec * 15 * 4)
		 / (Global::_pSong->BeatsPerMin() * Global::_pSong->TicksPerBeat()));
}

void Player::ExecuteLine(void)
{
	Song* pSong = Global::_pSong;

	_lineChanged = true;

	unsigned char* const plineOffset = pSong->_ptrackline(_playPattern,0,_lineCounter);
	ATLTRACE2(_T("pattern:%x line:%d \n"),_playPattern,_lineCounter);	

	// Initial Loop. Check for Tracker Commands.
	for (int track=0; track<pSong->SongTracks(); track++)
	{
		PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*EVENT_SIZE);
		

		if ( pEntry->_note < cdefTweakM || (pEntry->_note == 255 && (pEntry->_cmd >= 0xf0 || pEntry->_cmd == PatternCmd::PATTERNBREAK))) // Check for Global Command.
		{
			switch(pEntry->_cmd)
			{
			// Change Beat Per Minute
			case PatternCmd::CHGBPM:
				if ( pEntry->_parameter != 0 )
				{
					bpm=pEntry->_parameter;//+0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
					Global::_pSong->SamplesPerTick(
						(Global::pConfig->_pOutputDriver->_samplesPerSec * 15 * 4)
						 / (bpm * tpb)
					);
				}
				break;

			// Change Ticks Per Beat
			case PatternCmd::CHGTPB:
				if ( pEntry->_parameter != 0 )
				{
					tpb = pEntry->_parameter;
					Global::_pSong->SamplesPerTick((
						Global::pConfig->_pOutputDriver->_samplesPerSec * 15 * 4)
						/ (bpm * tpb));
				}
				break;

			//  Change Volume
			case PatternCmd::VOLCHG:
				if ( pEntry->_mach == 255 )
				{
					((Master*)(pSong->pMachine(MASTER_INDEX)))->_outDry = pEntry->_parameter;
				}
				else 
				{
					int mIndex = pEntry->_mach;
					if (mIndex < MAX_MACHINES)
					{
						if (pSong->pMachine(mIndex) )
						{
							pSong->pMachine(mIndex)->SetDestWireVolume(mIndex,pEntry->_inst,CValueMapper::Map_255_1(pEntry->_parameter));
						}
					}
				}
				break;

			// Set Panning
			case PatternCmd::PAN:
				{
					int mIndex = pEntry->_mach;
					if (mIndex < MAX_MACHINES)
					{
						if (pSong->pMachine(mIndex) )
						{
							pSong->pMachine(mIndex)->SetPan(pEntry->_parameter>>1);
						}
					}
				}
				break;
			case PatternCmd::PATTERNBREAK:
				_lineCounter = pSong->PatternLines(_playPattern);
				break;
			}
		}
		else if ( !pSong->IsTrackMuted(track) ) // Check For Tweak or MIDI CC
		{
			int mac = pEntry->_mach;
			if (( mac != 255) || (prevMachines[track] != 255))
			{
				if ( mac != 255 ) prevMachines[track] = mac;
				else mac = prevMachines[track];

				if (mac < MAX_MACHINES)
				{
					Machine *pMachine = pSong->pMachine(mac);
					if (pMachine)
					{
						pMachine->Tick(track, pEntry);
					}
				}
			}
		}
	}
	// Notify all machines that a new Tick() comes.
	for (int tc = 0; tc < MAX_MACHINES; tc++)
	{
		if(pSong->pMachine(tc))
		{
			pSong->pMachine(tc)->Tick();
			for (int c = 0; c < MAX_TRACKS; c++)
			{
				pSong->pMachine(tc)->TriggerDelay[c]._cmd = 0;
			}
		}
	}

	// Finally, loop again checking for Notes
	//
	for (track=0; track<pSong->SongTracks(); track++)
	{
		PatternEntry* pEntry = (PatternEntry*)(plineOffset + track * EVENT_SIZE);

		if (( !pSong->IsTrackMuted(track)) &&
			( pEntry->_note < cdefTweakM || pEntry->_note == 255 ))
		{
			int mac = pEntry->_mach;
			if (( mac != 255 || prevMachines[track] != 255) &&
				(pEntry->_note != 255 || pEntry->_cmd != 0x00 || pEntry->_volcmd != 0x00 ))
			{
				if ( mac != 255 ) prevMachines[track] = mac;
				else mac = prevMachines[track];

				if (mac < MAX_MACHINES)
				{
					Machine *pMachine = pSong->pMachine(mac);
					if (pMachine)
					{
						if (pEntry->_cmd == 0xfd)
						{
							// delay
							memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
							pMachine->TriggerDelayCounter[track] = ((pEntry->_parameter+1)*Global::_pSong->SamplesPerTick())/256;
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
	_ticksRemaining = pSong->SamplesPerTick();
}	


void Player::AdvancePosition()
{

	Song* pSong = Global::_pSong;

	_lineCounter++;
	_playTime += 60.0f / float (bpm * tpb);
	if (_playTime > 60.0f)
	{
		_playTime -= 60.0f;
		_playTimem++;
	}

	if (_lineCounter >= pSong->PatternLines(_playPattern))
	{
		_lineCounter = 0;


		if(!_playBlock)
			_playPosition++;
		else
		{
			_playPosition++;
			while (_playPosition< pSong->PlayLength() && (!pSong->PlayOrderSel(_playPosition)))
				_playPosition++;
		}
		
		if ( _playPosition >= pSong->PlayLength())
		{	
			if (_recording) // Don't loop the recording
			{
				StopRecording();
			}
			if ( _loopSong )
			{
				_playPosition = 0;
				if (( _playBlock ) && (pSong->PlayOrderSel(_playPosition) == false))
				{
					while ( (!pSong->PlayOrderSel(_playPosition)) &&
						( _playPosition< pSong->PlayLength())) _playPosition++;
				}
			}
			else 
			{
				_playing = false;
				_playBlock =false;
			}
		}
		_playPattern = pSong->PlayOrder(_playPosition);
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

			CComCritSecLock<CComAutoCriticalSection> crit(Global::_pSong->Door(), TRUE);
			
			CPUCOST_INIT(idletime);
			// Reset all machines
			//
			if ( (int)pSong->SampCount() > Global::pConfig->_pOutputDriver->_samplesPerSec )
			{
				pSong->SampCount(0);
				for (int c=0; c<MAX_MACHINES; c++)
				{
					if (pSong->pMachine(c))
					{
						pSong->pMachine(c)->_wireCost = 0;
						pSong->pMachine(c)->_cpuCost = 0;
					}
				}
			}

			for (int c=0; c<MAX_MACHINES; c++)
			{
				if (pSong->pMachine(c))
				{
					pSong->pMachine(c)->PreWork(amount);
				}
			}

			if ( pSong->PW_Stage() == 1 )
			{
			// Mixing preview WAV
			//
				pSong->PW_Work(pSong->pMachine(MASTER_INDEX)->_pSamplesL,pSong->pMachine(MASTER_INDEX)->_pSamplesR, amount);
			}
			
			//////////////////////////////////////////////////////////////////////
			// Inject Midi input data
			if( !CMidiInput::Instance()->InjectMIDI( amount ) )
			{
				// if midi not enabled we just do the original tracker thing

				// Master machine initiates work
				//
				pSong->pMachine(MASTER_INDEX)->Work(amount);
			}

			CPUCOST_CALC(idletime, amount);

			pSong->CpuIdle(idletime);
			pSong->SampCount( pSong->SampCount() + amount);

			if ((pThis->_playing) && (pThis->_recording))
			{
//				float* pData = pThis->_pBuffer; <- this was fuxxxxing up
				float* pL = pSong->pMachine(MASTER_INDEX)->_pSamplesL;
				float* pR = pSong->pMachine(MASTER_INDEX)->_pSamplesR;
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


			Master::_pMasterSamples += amount * 2;
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
	const TCHAR *psFilename,
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

		Global::_pSong->SamplesPerTick(
			(Global::pConfig->_pOutputDriver->_samplesPerSec * 15 * 4)
			/ (Global::pPlayer->bpm*Global::pPlayer->tpb));

		Stop();
		if (_outputWaveFile.OpenForWrite(psFilename, Global::pConfig->_pOutputDriver->_samplesPerSec, Global::pConfig->_pOutputDriver->_bitDepth, channels) == DDC_SUCCESS)
		{
			_recording = true;
		}
		else
		{
			StopRecording();
			MessageBox(NULL,psFilename,SF::CResourceString(IDS_ERR_MSG0041),NULL);
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
		Global::_pSong->SamplesPerTick(
			(Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)
			/(Global::pPlayer->bpm*Global::pPlayer->tpb));

		_outputWaveFile.Close();
		_recording = false;
		if (!bOk)
		{
			MessageBox(NULL,SF::CResourceString(IDS_ERR_MSG0042),SF::CResourceString(IDS_ERR_MSG0043),MB_OK);
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

