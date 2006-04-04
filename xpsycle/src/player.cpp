/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "player.h"
#include "song.h"
#include "machine.h"
#include "global.h"
#include "configuration.h"
#include "inputhandler.h"
#include "helpers.h"

Player::Player()
{
 _playing = false;
 _playBlock = false;
 _recording = false;
 Tweaker = false;
 _samplesRemaining=0;
 _lineCounter=0;
 _loopSong=true;
 _patternjump=-1;
 _linejump=-1;
 _loop_count=0;
 _loop_line=0;
 m_SampleRate=44100;
 m_SamplesPerRow=(44100*60)/(125*4);
 tpb=4;
 bpm=125;
 _outputWaveFile = 0;
 for(int i=0;i<MAX_TRACKS;i++) prevMachines[i]=255;

}


Player::~Player()
{
  // if(_recording) _outputWaveFile.Close();
}

void Player::Start(int pos, int line)
{
  Stop(); // This causes all machines to reset, and samplesperRow to init.
  ((Master*)(Global::pSong()->_pMachine[MASTER_INDEX]))->_clip = false;
  ((Master*)(Global::pSong()->_pMachine[MASTER_INDEX]))->sampleCount = 0;
  _lineChanged = true;
  _lineCounter = line;
  _SPRChanged = false;
  _playPosition= pos;
  _playPattern = Global::pSong()->playOrder[_playPosition];
  _playTime = 0;
  _playTimem = 0;
  _loop_count =0;
  _loop_line = 0;
  SetBPM(Global::pSong()->BeatsPerMin(),Global::pSong()->LinesPerBeat());
  SampleRate(Global::pConfig()->_pOutputDriver->_samplesPerSec);
  for(int i=0;i<MAX_TRACKS;i++) prevMachines[i] = 255;
  _playing = true;
  ExecuteLine();
}

void Player::Stop(void)
{
   // Stop song enviroment
   _playing = false;
   _playBlock = false;
   for(int i=0; i<MAX_MACHINES; i++)
   {
      if(Global::pSong()->_pMachine[i])
      {
        Global::pSong()->_pMachine[i]->Stop();
        for(int c = 0; c < MAX_TRACKS; c++) Global::pSong()->_pMachine[i]->TriggerDelay[c]._cmd = 0;
      }
   }
   SetBPM(Global::pSong()->BeatsPerMin(),Global::pSong()->LinesPerBeat());
   SampleRate(Global::pConfig()->_pOutputDriver->_samplesPerSec);
}


void Player::SampleRate(const int sampleRate)
{
    ///\todo update the source code of the plugins...
    if(m_SampleRate != sampleRate)
    {
       m_SampleRate = sampleRate;
       RecalcSPR();
       for(int i(0) ; i < MAX_MACHINES; ++i)
       {
          if(Global::pSong()->_pMachine[i]) Global::pSong()->_pMachine[i]->SetSampleRate(sampleRate);
       }
    }
}

void Player::SetBPM(int _bpm,int _tpb)
{
  if ( _tpb != 0) tpb=_tpb;
  if ( _bpm != 0) bpm=_bpm;
  RecalcSPR();
  //\todo : Find out if we should notify the plugins of this change.
}

void Player::ExecuteLine(void)
{
  ExecuteGlobalCommands();
  NotifyNewLine();
  ExecuteNotes();
}


// Initial Loop. Read new line and Interpret the Global commands.
void Player::ExecuteGlobalCommands(void)
{
  Song* pSong = Global::pSong();
  _patternjump = -1;
  _linejump = -1;
  int mIndex = 0;
  unsigned char* const plineOffset = pSong->_ptrackline(_playPattern,0,_lineCounter);

  for(int track=0; track<pSong->SONGTRACKS; track++)
  {
    PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*EVENT_SIZE);
   if(pEntry->_note < cdefTweakM || pEntry->_note == 255) // If This isn't a tweak (twk/tws/mcm) then do
   {
      switch(pEntry->_cmd)
      {
         case PatternCmd::SET_TEMPO:
           if(pEntry->_parameter != 0)
           {   //\todo: implement the Tempo slide
             // SET_SONG_TEMPO= 20, // T0x Slide tempo down . T1x slide tempo up
              bpm = pEntry->_parameter;
              RecalcSPR();
           }
         break;
         case PatternCmd::EXTENDED:
           if(pEntry->_parameter != 0)
           {
             if ( (pEntry->_parameter&0xE0) == 0 ) // range from 0 to 1F for LinesPerBeat.
             {
               tpb=pEntry->_parameter;
               RecalcSPR();
             }
             else if ( (pEntry->_parameter&0xF0) == PatternCmd::SET_BYPASS )
             {
               mIndex = pEntry->_mach;
               if ( mIndex < MAX_MACHINES && pSong->_pMachine[mIndex] && pSong->_pMachine[mIndex]->_mode == MACHMODE_FX ) {
                  if ( pEntry->_parameter&0x0F )
                     pSong->_pMachine[mIndex]->_bypass = true;
                  else
                     pSong->_pMachine[mIndex]->_bypass = false;
               }
             }
             else if ( (pEntry->_parameter&0xF0) == PatternCmd::SET_MUTE )
             {
                mIndex = pEntry->_mach;
                if ( mIndex < MAX_MACHINES && pSong->_pMachine[mIndex] && pSong->_pMachine[mIndex]->_mode == MACHMODE_FX )
                {
                  if ( pEntry->_parameter&0x0F )
                    pSong->_pMachine[mIndex]->_mute = true;
                  else
                    pSong->_pMachine[mIndex]->_mute = false;
                }
             }
             else if ( (pEntry->_parameter&0xF0) == PatternCmd::PATTERN_DELAY )
             {
                SamplesPerRow(SamplesPerRow()*(1+(pEntry->_parameter&0x0F)));
                _SPRChanged=true;
             }
             else if ( (pEntry->_parameter&0xF0) == PatternCmd::FINE_PATTERN_DELAY)
             {
                SamplesPerRow(SamplesPerRow()*(1.0f+((pEntry->_parameter&0x0F)*tpb/24.0f)));
                _SPRChanged=true;
             }
             else if ( (pEntry->_parameter&0xF0) == PatternCmd::PATTERN_LOOP)
             {
                int value = pEntry->_parameter&0x0F;
                if (value == 0 )
                {
                   _loop_line = _lineCounter;
                } else {
                  if ( _loop_count == 0 )
                  {
                   _loop_count = value;
                   _linejump = _loop_line;
                  } else {
                    if (--_loop_count) _linejump = _loop_line;
                    else _loop_line = _lineCounter+1; //This prevents infinite loop in specific cases.
                  }
                }
             }
           }
           break;
           case PatternCmd::JUMP_TO_ORDER:
             if ( pEntry->_parameter < pSong->playLength ){
                _patternjump=pEntry->_parameter;
                _linejump=0;
             }
           break;
           case PatternCmd::BREAK_TO_LINE:
             if (_patternjump ==-1) 
             {
              _patternjump=(_playPosition+1>=pSong->playLength)?0:_playPosition+1;
             }
             if ( pEntry->_parameter >= pSong->patternLines[_patternjump])
             {
              _linejump = pSong->patternLines[_patternjump];
             } else { _linejump= pEntry->_parameter; }
           break;
           case PatternCmd::SET_VOLUME:
             if(pEntry->_mach == 255)
             {
               ((Master*)(pSong->_pMachine[MASTER_INDEX]))->_outDry = pEntry->_parameter;
             }
             else
             {
                int mIndex = pEntry->_mach;
                if(mIndex < MAX_MACHINES)
                {
                  if(pSong->_pMachine[mIndex]) pSong->_pMachine[mIndex]->SetDestWireVolume(mIndex,pEntry->_inst,CValueMapper::Map_255_1(pEntry->_parameter));
                }
             }
           break;
           case  PatternCmd::SET_PANNING:
                mIndex = pEntry->_mach;
                if(mIndex < MAX_MACHINES)
                {
                  if(pSong->_pMachine[mIndex]) pSong->_pMachine[mIndex]->SetPan(pEntry->_parameter>>1);
                }
           break;
         }
       }
       // Check For Tweak or MIDI CC
       else if(!pSong->_trackMuted[track])
       {
         int mac = pEntry->_mach;
         if((mac != 255) || (prevMachines[track] != 255))
         {
           if(mac != 255) prevMachines[track] = mac;
           else mac = prevMachines[track];
           if(mac < MAX_MACHINES)
           {
             Machine *pMachine = pSong->_pMachine[mac];
             if(pMachine)
             {
                if(pEntry->_note == cdefMIDICC && pMachine->_type != MACH_VST && pMachine->_type != MACH_VSTFX)
                {
                  // for native machines,
                  // use the value in the "instrument" field of the event as a voice number
                  int voice(pEntry->_inst);
                  // make a copy of the pattern entry, because we're going to modify it.
                  PatternEntry entry(*pEntry);
                  entry._inst = 0;
                  // check for out of range voice values (with the classic tracker way, it's the same as the pattern tracks)
                  if(voice < pSong->SONGTRACKS)
                  {
                    pMachine->Tick(voice, &entry);
                  }
                  else if(voice == 0xff)
                  {
                    // special voice value which means we want to send the same command to all voices
                    for(int voice(0) ; voice < pSong->SONGTRACKS ; ++voice)
                    {
                      pMachine->Tick(voice, &entry);
                    }
                  }
                  else ; // probably an out of range voice value (with the classic tracker way, it's limited to the number of pattern tracks)
                }
                else // midi cc for vst, or other commands
                {
                  // for vst machines,
                  // classic tracking, use the track number as the channel/voice number
                  pMachine->Tick(track, pEntry);
                }
              }
            }
          }
        }
   }
}

void Player::NotifyNewLine( void )
{
  Song* pSong = Global::pSong();
  for(int tc=0; tc<MAX_MACHINES; tc++)
  {
     if(pSong->_pMachine[tc])
     {
       pSong->_pMachine[tc]->Tick();
       for(int c = 0; c < MAX_TRACKS; c++) pSong->_pMachine[tc]->TriggerDelay[c]._cmd = 0;
     }
  }
}


void Player::ExecuteNotes(void)
		{
			Song* pSong = Global::pSong();
			unsigned char* const plineOffset = pSong->_ptrackline(_playPattern,0,_lineCounter);


			for(int track=0; track<pSong->SONGTRACKS; track++)
			{
				PatternEntry* pEntry = (PatternEntry*)(plineOffset + track*EVENT_SIZE);
				if(( !pSong->_trackMuted[track]) && (pEntry->_note < cdefTweakM || pEntry->_note == 255)) // Is it not muted and is a note?
				{
					int mac = pEntry->_mach;
					if(mac != 255) prevMachines[track] = mac;
					else mac = prevMachines[track];
//					if( mac != 255 && (pEntry->_note != 255 || pEntry->_cmd != 0x00) ) // is there a machine number and it is either a note or a command?
					if( mac != 255 ) // is there a machine number and it is either a note or a command?
					{
						if(mac < MAX_MACHINES) //looks like a valid machine index?
						{
							Machine *pMachine = pSong->_pMachine[mac];
							if(pMachine && !(pMachine->_mute)) // Does this machine really exist and is not muted?
							{
								if(pEntry->_cmd == PatternCmd::NOTE_DELAY)
								{
									// delay
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
									pMachine->TriggerDelayCounter[track] = ((pEntry->_parameter+1)*SamplesPerRow())/256;
								}
								else if(pEntry->_cmd == PatternCmd::RETRIGGER)
								{
									// retrigger
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
									pMachine->RetriggerRate[track] = (pEntry->_parameter+1);
									pMachine->TriggerDelayCounter[track] = 0;
								}
								else if(pEntry->_cmd == PatternCmd::RETR_CONT)
								{
									// retrigger continue
									memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
									if(pEntry->_parameter&0xf0) pMachine->RetriggerRate[track] = (pEntry->_parameter&0xf0);
								}
								else if (pEntry->_cmd == PatternCmd::ARPEGGIO)
								{
									// arpeggio
									//\todo : Add Memory.
									//\todo : This won't work... What about sampler's NNA's?
									if (pEntry->_parameter)
									{
										memcpy(&pMachine->TriggerDelay[track], pEntry, sizeof(PatternEntry));
										pMachine->ArpeggioCount[track] = 1;
									}
									pMachine->RetriggerRate[track] = SamplesPerRow()*tpb/24;
								}
								else
								{
									pMachine->TriggerDelay[track]._cmd = 0;
									pMachine->Tick(track, pEntry);
									pMachine->TriggerDelayCounter[track] = 0;
									pMachine->ArpeggioCount[track] = 0;
								}
							}
						}
					}
				}
			}
			_samplesRemaining = SamplesPerRow();
		}

float * Player::Work( void * context, int & numSamples )
{
  int amount;
  Player* pThis = (Player*)context;
  Song* pSong = Global::pSong();
  Master::_pMasterSamples = pThis->_pBuffer;

  int numSamplex = numSamples;


  do
  {
    //printf("%d\n",numSamplex); fflush(stdout);
    if(numSamplex > STREAM_SIZE) amount = STREAM_SIZE; else amount = numSamplex;
    // Tick handler function
    if((pThis->_playing) && (amount >= pThis->_samplesRemaining)) amount = pThis->_samplesRemaining;
    // Song play
    if((pThis->_samplesRemaining <=0))
    {
      if (pThis->_playing) {
        // Advance position in the sequencer
        pThis->AdvancePosition();
        // Global commands are executed first so that the values for BPM and alike
        // are up-to-date when "NotifyNewLine()" is called.
        pThis->ExecuteGlobalCommands();
        pThis->NotifyNewLine();
        pThis->ExecuteNotes();
      }
      else
      {
        pThis->NotifyNewLine();
      }
    }
    // Processing plant

    if(amount > 0)
    {
      CPUCOST_INIT(idletime);
      if( (int)pSong->_sampCount > Global::pConfig()->_pOutputDriver->_samplesPerSec)
      {
         pSong->_sampCount =0;
         for(int c=0; c<MAX_MACHINES; c++) {
           if(pSong->_pMachine[c])
           {
              pSong->_pMachine[c]->_wireCost = 0;
              pSong->_pMachine[c]->_cpuCost = 0;
           }
         }
      }
      // Reset all machines
      for(int c=0; c<MAX_MACHINES; c++)
      {
         if(pSong->_pMachine[c]) pSong->_pMachine[c]->PreWork(amount);
      }

      if(pSong->PW_Stage == 1)
      {
         // Mixing preview WAV
         pSong->PW_Work(pSong->_pMachine[MASTER_INDEX]->_pSamplesL,pSong->_pMachine[MASTER_INDEX]->_pSamplesR, amount);
      }
         // Inject Midi input data
//         if(!CMidiInput::Instance()->InjectMIDI( amount ))
         {
             // if midi not enabled we just do the original tracker thing
             // Master machine initiates work
             pSong->_pMachine[MASTER_INDEX]->Work(amount);
         }
         //CPUCOST_CALC(idletime, amount);
         pSong->cpuIdle = idletime;
         pSong->_sampCount += amount;
         if((pThis->_playing) && (pThis->_recording))
         {
           float* pL(pSong->_pMachine[MASTER_INDEX]->_pSamplesL);
           float* pR(pSong->_pMachine[MASTER_INDEX]->_pSamplesR);
           int i;
           switch(Global::pConfig()->_pOutputDriver->_channelmode)
           {
             case 0: // mono mix
               for(i=0; i<amount; i++)
               {
                 try {
                   pThis->_outputWaveFile->WriteMonoSample(((*pL++)+(*pR++))/2);
                 } catch (std::exception) {
                   pThis->StopRecording(false);
                 }
               }
             break;
             case 1: // mono L
               for(i=0; i<amount; i++)
               {
                try {
                 pThis->_outputWaveFile->WriteMonoSample((*pL++));
                } catch (std::exception) {
                  pThis->StopRecording(false);
                }
               }
             break;
             case 2: // mono R
               for(i=0; i<amount; i++)
               {
                 try {
                   pThis->_outputWaveFile->WriteMonoSample((*pR++));
                 } catch (std::exception e) {
                   pThis->StopRecording(false);
                 }
               }
             break;
             default: // stereo
                for(i=0; i<amount; i++)
                {
                  try {
                    pThis->_outputWaveFile->WriteStereoSample((*pL++),(*pR++));
                  } catch (std::exception e) {
                     pThis->StopRecording(false);
                  }
                }
                break;
           }
         }
         Master::_pMasterSamples += amount * 2;
         numSamplex -= amount;
       }
       if(pThis->_playing) pThis->_samplesRemaining -= amount;
     } while(numSamplex>0);
  return pThis->_pBuffer;
}

void Player::AdvancePosition( )
{
 Song* pSong = Global::pSong();
 if ( _patternjump!=-1 ) _playPosition= _patternjump;
 if ( _SPRChanged ) { RecalcSPR(); _SPRChanged = true; }
 if ( _linejump!=-1 ) _lineCounter=_linejump; else _lineCounter++;
 _playTime += 60 / float (bpm * tpb);
 if(_playTime>60) {
    _playTime-=60;
    _playTimem++;
 }
 if(_lineCounter >= pSong->patternLines[_playPattern])
 {
   _lineCounter = 0;
   if(!_playBlock) _playPosition++; else
   {
     _playPosition++;
     while(_playPosition< pSong->playLength && (!pSong->playOrderSel[_playPosition]))
     _playPosition++;
   }
  }
  if( _playPosition >= pSong->playLength)
  {
     // Don't loop the recording
     if(_recording) {
        //StopRecording();
     }
     if( _loopSong ) {
        _playPosition = 0;
        if(( _playBlock) && (pSong->playOrderSel[_playPosition] == false)) {
          while((!pSong->playOrderSel[_playPosition]) && ( _playPosition< pSong->playLength)) _playPosition++;
        }
     } else {
        _playing = false;
        _playBlock =false;
     }
  }
  // this is outside the if, so that _patternjump works
  _playPattern = pSong->playOrder[_playPosition];
  _lineChanged = true;
}

void Player::StartRecording(std::string psFilename, int bitdepth, int samplerate, int channelmode)
{
  backup_rate = Global::pConfig()->_pOutputDriver->_samplesPerSec;
  backup_bits = Global::pConfig()->_pOutputDriver->_bitDepth;
  backup_channelmode = Global::pConfig()->_pOutputDriver->_channelmode;

  if(samplerate > 0) {
     SampleRate(samplerate);
     Global::pConfig()->_pOutputDriver->_samplesPerSec = samplerate;
  }

  if(bitdepth > 0) Global::pConfig()->_pOutputDriver->_bitDepth = bitdepth;

  if(channelmode >= 0) Global::pConfig()->_pOutputDriver->_channelmode = channelmode;

  int channels = 2;

  if(Global::pConfig()->_pOutputDriver->_channelmode != 3) channels = 1;

  Stop();

  _recording = true;

  try {
    _outputWaveFile = new Serializer(psFilename,Global::pConfig()->_pOutputDriver->_samplesPerSec, Global::pConfig()->_pOutputDriver->_bitDepth, channels);
  } catch (std::exception) {
    StopRecording(false);
  }


}

void Player::StopRecording( bool bOk )
{
  if(_recording)
  {
     Global::pConfig()->_pOutputDriver->_samplesPerSec = backup_rate;
     SampleRate(backup_rate);
     Global::pConfig()->_pOutputDriver->_bitDepth = backup_bits;
     Global::pConfig()->_pOutputDriver->_channelmode = backup_channelmode;
     delete _outputWaveFile;
     _outputWaveFile = 0;
     _recording = false;
     if(!bOk) {
        std::cerr << "Wav recording failed." << std::endl;
         //MessageBox(0, "Wav recording failed.", "ERROR", MB_OK);
     }
  }
}