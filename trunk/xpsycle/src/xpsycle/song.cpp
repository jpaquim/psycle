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
#include "song.h"
#include "global.h"
#include "configuration.h"
#include "datacompression.h"
#include "machine.h"
#include "sampler.h"
#include "xmsampler.h"
#include "plugin.h"
#include "deserializer.h"
#include "serializer.h"
#include <inttypes.h>

/// Safer version of delete that clears the pointer automatically. Don't use it for arrays!
/// \param pointer pointer to single object to be deleted.
/// \param new_value the new value pointer will be set to. By default it is null.



template<typename object_array> inline object_array * zapArray(object_array *& pointer, object_array * const new_value = 0)
		{
			if(pointer) delete [] pointer;
			return pointer = new_value;
		}



using namespace std;

bool Song::CreateMachine(MachineType type, int x, int y, char const* psPluginDll, int index)
{
  Machine* pMachine;
  Master* pMaster;
  Sampler* pSampler;
  XMSampler* pXMSampler;
  DuplicatorMac* pDuplicator;
  Mixer* pMixer;
  Plugin* pPlugin;
//  vst::plugin* pVstPlugin;
  switch (type)
  {
    case MACH_MASTER:
       if(_pMachine[MASTER_INDEX]) return false;
       pMachine = pMaster = new Master(index);
       index = MASTER_INDEX;
    break;
    case MACH_SAMPLER:
         pMachine = pSampler = new Sampler(index);
    break;
    case MACH_XMSAMPLER:
//       pMachine = pXMSampler = new XMSampler(index);
    break;
    case MACH_DUPLICATOR:
       pMachine = pDuplicator = new DuplicatorMac(index);
    break;
    case MACH_MIXER:
       pMachine = pMixer = new Mixer(index);
    break;
    case MACH_PLUGIN:
    {
        pMachine = pPlugin = new Plugin(index);
//       if(!CNewMachine::TestFilename(psPluginDll))
 //      {
 //        zapObject(pMachine);
 //        return false;
 //      }
   //    try
   //    {
           pPlugin->LoadDll(psPluginDll);
   //    }
  //     catch(std::exception const & e)
       //{
//         loggers::exception(e.what());
//         zapObject(pMachine);
//         return false;
//       }
//       catch(...)
//       {
//         zapObject(pMachine); 
//         return false;
//       }
       break;
     }
     case MACH_VST:
     {
  /*     pMachine = pVstPlugin = new vst::instrument(index);
       if(!CNewMachine::TestFilename(psPluginDll)) 
       {
         zapObject(pMachine);
         return false;
       }
       try
       {
         pVstPlugin->Instance(psPluginDll); // <bohan> why not using Load?
       }
       catch(std::exception const & e)
       {
         loggers::exception(e.what());
         zapObject(pMachine);
         return false;
       }
       catch(...)
       {
         zapObject(pMachine);
         return false;
       }
       break;*/
     }
     case MACH_VSTFX:
     {
       /*pMachine = pVstPlugin = new vst::fx(index);
       if(!CNewMachine::TestFilename(psPluginDll)) 
       {
         zapObject(pMachine);
         return false;
       }
       try
       {
         pVstPlugin->Instance(psPluginDll); // <bohan> why not using Load?
       }
       catch(std::exception const & e)
       {
         loggers::exception(e.what());
         zapObject(pMachine); 
         return false;
       }
       catch(...)
       {
         zapObject(pMachine);
         return false;
       }*/
       break;
     }
     case MACH_DUMMY:
        pMachine = new Dummy(index);
     break;
     default:
        return false; ///< hmm?
    }
    if(index < 0)
    {
      index = GetFreeMachine();
      if(index < 0) return false;
    }
    if(_pMachine[index]) DestroyMachine(index);
    if(pMachine->_type == MACH_VSTFX || pMachine->_type == MACH_VST )
    {
      // Do not call VST Init() function after Instance.
      pMachine->Machine::Init();
    }
    else pMachine->Init();
    pMachine->_x = x;
    pMachine->_y = y;
    // Finally, activate the machine
    _pMachine[index] = pMachine;
    return true;
}

Song::Song( )
{
  _machineLock = false;
  Invalided = false;
  PW_Phase = 0;
  PW_Stage = 0;
  PW_Length = 0;
  // setting the preview wave volume to 25%
  preview_vol = 0.25f;

  for(int i(0) ; i < MAX_PATTERNS; ++i) ppPatternData[i] = NULL;
  for(int i(0) ; i < MAX_MACHINES; ++i) _pMachine[i] = NULL;
  for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) _pInstrument[i] = new Instrument;
  Reset();
  //SONGTRACKS  = 16;
}

Song::~ Song( ) throw()
{
  DestroyAllMachines();
  DestroyAllInstruments();
  DeleteAllPatterns();
}

void Song::Reset( )
{
  cpuIdle=0;
  _sampCount=0;
 //  Cleaning pattern allocation info
//  for(int i(0) ; i < MAX_INSTRUMENTS; ++i) _pInstrument[i]->waveLength=0;
  for(int i(0) ; i < MAX_MACHINES ; ++i)
  {
    zapObject(_pMachine[i]);
  }
  for(int i(0) ; i < MAX_PATTERNS; ++i)
  {
  // All pattern reset
    if(Global::pConfig()) patternLines[i]=Global::pConfig()->defaultPatLines;
    else patternLines[i]=64;
    std::sprintf(patternName[i], "Untitled"); 
  }
  _trackArmedCount = 0;
  for(int i(0) ; i < MAX_TRACKS; ++i)
  {
    _trackMuted[i] = false;
    _trackArmed[i] = false;
  }
  machineSoloed = -1;
  _trackSoloed = -1;
  playLength=1;
  for(int i(0) ; i < MAX_SONG_POSITIONS; ++i)
  {
    playOrder[i]=0; // All pattern reset
    playOrderSel[i]=false;
  }
  playOrderSel[0]=true;
  CreateNewPattern(0);
}

unsigned char * Song::CreateNewPattern( int ps )
{
  RemovePattern(ps);
  ppPatternData[ps] = new unsigned char[MULTIPLY2];
  PatternEntry blank;
  unsigned char * pData = ppPatternData[ps];
  for(int i = 0; i < MULTIPLY2; i+= EVENT_SIZE)
  {
    memcpy(pData,&blank,EVENT_SIZE);
    pData+= EVENT_SIZE;
  }
  return ppPatternData[ps];
}

void Song::RemovePattern( int ps )
{
  zapArray(ppPatternData[ps]);
}

void Song::New( )
{
  seqBus=0;
  // Song reset
  std::memset(&Name, 0, sizeof Name);
  std::memset(&Author, 0, sizeof Author);
  std::memset(&Comment, 0, sizeof Comment);
  std::sprintf(Name, "Untitled");
  std::sprintf(Author, "Unnamed");
  std::sprintf(Comment, "No Comments");

  currentOctave=4;
  // General properties
  m_BeatsPerMin=125;
  m_LinesPerBeat=4;
//			LineCounter=0;
//			LineChanged=false;
			//MessageBox(0, "Machines", 0, 0);
			// Clean up allocated machines.
  DestroyAllMachines(true);
			//MessageBox(0, "Insts", 0, 0);
			// Cleaning instruments
   DeleteInstruments();
			//MessageBox(0, "Pats", 0, 0);
			// Clear patterns
  DeleteAllPatterns();
			// Clear sequence
  Reset();
  instSelected = 0;
  midiSelected = 0;
  auxcolSelected = 0;
  _saved=false;
  fileName ="Untitled.psy";
/*  if((CMainFrame *)theApp.m_pMainWnd)
			{*/
   //CreateMachine
   //(
     //MACH_MASTER,
     // (viewSize.x - static_cast<CMainFrame*>(theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width) / 2, 
	//					(viewSize.y - static_cast<CMainFrame*>(theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height) / 2, 
	//					0,
	//					MASTER_INDEX
	//				);
	//		}
	//		else
	//		{
				CreateMachine(MACH_MASTER, 320, 200, 0, MASTER_INDEX);
	//		}*/
}

bool Song::load( const std::string & fName )
{
  DeSerializer f(fName.c_str());

  int fileSize = f.fileSize();
  int fcounter = 0;

  if (f.getHeader8()=="PSY3SONG") {
     if (f.getVersion() > CURRENT_FILE_VERSION) { }
     unsigned int size    = f.getInt();
     int chunkcount = 0;
     if (size == 4) chunkcount = f.getInt();
     if (size > 4 ) f.skip(size - 4);

     int solo = 0;
     try {
       while (!f.eof()) {
          fcounter++;
          string header = f.getHeader4();
          if (fcounter % 100 == 0) loadProgress.emit(f.getPos(),fileSize,header);
          if (header == "INFO") {
             --chunkcount;
             if (f.checkVersion(CURRENT_FILE_VERSION_INFO))
             {
               f.readString(Name,sizeof(Name));
               f.readString(Author,sizeof(Author));
               f.readString(Comment,sizeof(Comment));
             }
          } else
          if (header == "SNGI") {
             --chunkcount;
             if (f.checkVersion(CURRENT_FILE_VERSION_SNGI)) {
                SONGTRACKS       = f.getInt();
                m_BeatsPerMin    = f.getInt();
                m_LinesPerBeat   = f.getInt();
                currentOctave    = f.getInt();
                solo             = f.getInt();
                _trackSoloed     = f.getInt();
                seqBus           = f.getInt();
                midiSelected     = f.getInt();
                auxcolSelected   = f.getInt();
                instSelected     = f.getInt();
                _trackArmedCount = 0; f.getInt();
                for(int i(0) ; i < SONGTRACKS; ++i) {
                  f.read(&_trackMuted[i],sizeof(_trackMuted[i]));
                  // remember to count them
                  f.read(&_trackArmed[i],sizeof(_trackArmed[i]));
                  if(_trackArmed[i]) ++_trackArmedCount;
                }
              }
            } else
          if (header == "SEQD") {
             --chunkcount;
             if (f.checkVersion(CURRENT_FILE_VERSION_SEQD)) {
               unsigned int index = f.getInt();
               if (index < MAX_SEQUENCES) {
                 // play length for this sequence
                 playLength = f.getInt();
                 // name, for multipattern, for now unused
                 char pTemp[256];
                 f.readString(pTemp,sizeof(pTemp));
                 for (int i(0) ; i < playLength; ++i) {
                   playOrder[i] = f.getInt();
                 }
               }
             }
          }
          if (header == "PATD") {
             --chunkcount;
             if (f.checkVersion(CURRENT_FILE_VERSION_PATD)) {
                unsigned int index = f.getInt();
                if(index < MAX_PATTERNS) {
                  // num lines
                  int num = f.getInt();
                  // clear it out if it already exists
                  RemovePattern(index);
                  patternLines[index] = num;
                  // num tracks per pattern
                  // eventually this may be variable per pattern,
                  // like when we get multipattern
                  f.getInt();
                  f.readString(patternName[index], sizeof(*patternName));
                  unsigned int size = f.getInt();
                  byte* pSource = new byte[size];
                  f.read(pSource, size);
                  byte* pDest;
                  DataCompression::BEERZ77Decomp2(pSource, &pDest);
                  zapArray(pSource,pDest);
                  for(int y(0) ; y < patternLines[index] ; ++y) {
                    unsigned char* pData(_ppattern(index) + (y * MULTIPLY));
                    std::memcpy(pData, pSource, SONGTRACKS * EVENT_SIZE);
                    pSource += SONGTRACKS * EVENT_SIZE;
                  }
                  zapArray(pDest);
                }
             }
          } else
          if (header == "MACD") {
               chunkcount--;
//               if(!fullopen) {
               //   curpos = pFile->GetPos();
  //             }
               if (f.checkVersion(CURRENT_FILE_VERSION_MACD)) {

               }
                  unsigned int index = f.getInt();
                  if(index < MAX_MACHINES) {
                      // we had better load it
                        DestroyMachine(index); bool fullopen = true;
                        _pMachine[index] = Machine::LoadFileChunk(&f, index, CURRENT_FILE_VERSION_MACD, fullopen);
                      // skips specific chunk.
    //                    if(!fullopen) pFile->seek(curpos + size);
                  } else {
           //MessageBox(0, "Instrument section of File is from a newer version of psycle!", 0, 0);
//                      pFile->Skip(size - sizeof index);
                  }
          } else 
          if (header== "INSD") {
            if (f.checkVersion(CURRENT_FILE_VERSION_INSD)) {
               int index = f.getInt();
               if(index < MAX_INSTRUMENTS) {
                 _pInstrument[index]->LoadFileChunk(&f, CURRENT_FILE_VERSION_INSD, true);
               } else
               {
                 //MessageBox(0, "Instrument section of File is from a newer version of psycle!", 0, 0);
                 //pFile->skip(size - sizeof index);
               }
            }
          }

        }
     }  catch (int e) {};
  }

  f.close();
  fileName = fName;
}



void Song::DestroyMachine(int mac, bool write_locked)
{
  Machine *iMac = _pMachine[mac];
  Machine *iMac2;
  if(iMac)
  {
    // Deleting the connections to/from other machines
    for(int w=0; w<MAX_CONNECTIONS; w++)
    {
      // Checking In-Wires
      if(iMac->_inputCon[w])
      {
         if((iMac->_inputMachines[w] >= 0) && (iMac->_inputMachines[w] < MAX_MACHINES))
         {
           iMac2 = _pMachine[iMac->_inputMachines[w]];
           if(iMac2)
           {
              for(int x=0; x<MAX_CONNECTIONS; x++)
              {
                if( iMac2->_connection[x] && iMac2->_outputMachines[x] == mac)
                {
                  iMac2->_connection[x] = false;
                  iMac2->_numOutputs--; 
                  break;
                }
              }
           }
         }
       }
       // Checking Out-Wires
       if(iMac->_connection[w])
       {
         if((iMac->_outputMachines[w] >= 0) && (iMac->_outputMachines[w] < MAX_MACHINES))
         {
            iMac2 = _pMachine[iMac->_outputMachines[w]];
            if(iMac2)
            {
               for(int x=0; x<MAX_CONNECTIONS; x++)
               {
                 if(iMac2->_inputCon[x] && iMac2->_inputMachines[x] == mac)
                 {
                   iMac2->_inputCon[x] = false;
                   iMac2->_numInputs--;
                   break;
                 }
               }
             }
           }
         }
       }
     }
     if(mac == machineSoloed) machineSoloed = -1;
     // If it's a (Vst)Plugin, the destructor calls to release the underlying library
     zapObject(_pMachine[mac]);
}


int Song::GetFreeMachine()
{
  int tmac = 0;
  for(;;)
  {
     if(!_pMachine[tmac]) return tmac;
     if(tmac++ >= MAX_MACHINES) return -1;
  }
}


void Song::DeleteAllPatterns()
{
  SONGTRACKS = 16;
  for(int i=0; i<MAX_PATTERNS; i++) RemovePattern(i);
}

void Song::DestroyAllMachines(bool write_locked)
{
  _machineLock = true;
  for(int c(0) ;  c < MAX_MACHINES; ++c)
  {
    if(_pMachine[c])
    {
       for(int j(c + 1) ; j < MAX_MACHINES; ++j)
       {
          if(_pMachine[c] == _pMachine[j])
          {
            ///\todo wtf? duplicate machine? could happen if loader messes up?
            char buf[128];
            std::sprintf(buf,"%d and %d have duplicate pointers", c, j);
//            MessageBox(0, buf, "Duplicate Machine", 0);
            _pMachine[j] = 0;
          }
       }
       DestroyMachine(c, write_locked);
    }
    _pMachine[c] = 0;
  }
  _machineLock = false;
}

void Song::DestroyAllInstruments()
{
  for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) zapObject(_pInstrument[i]);
}


void Song::PW_Work(float *pInSamplesL, float *pInSamplesR, int numSamples)
{
  float *pSamplesL = pInSamplesL;
  float *pSamplesR = pInSamplesR;
  --pSamplesL;
  --pSamplesR;

  signed short *wl=_pInstrument[PREV_WAV_INS]->waveDataL;
  signed short *wr=_pInstrument[PREV_WAV_INS]->waveDataR;
  bool const stereo=_pInstrument[PREV_WAV_INS]->waveStereo;
  float ld=0;
  float rd=0;

  do {
    ld=(*(wl+PW_Phase))*preview_vol;
    if(stereo)
       rd=(*(wr+PW_Phase))*preview_vol; 
    else 
       rd=ld;

    *++pSamplesL+=ld;
    *++pSamplesR+=rd;

    if(++PW_Phase>=PW_Length) {
      PW_Stage=0;
      return;
    }
  } while(--numSamples);
}


int Song::GetBlankPatternUnused(int rval)
{
  for(int i(0) ; i < MAX_PATTERNS; ++i) if(!IsPatternUsed(i)) return i;
    PatternEntry blank;
    bool bTryAgain(true);
    while(bTryAgain && rval < MAX_PATTERNS - 1)
    {
      for(int c(0) ; c < playLength ; ++c)
      {
        if(rval == playOrder[c])  {
          ++rval;
          c = -1;
        }
      }
      // now test to see if data is really blank
      bTryAgain = false;
      if(rval < MAX_PATTERNS - 1) {
        unsigned char *offset_source(_ppattern(rval));
        for(int t(0) ; t < MULTIPLY2 ; t += EVENT_SIZE) {
          if(memcmp(offset_source+t,&blank,EVENT_SIZE) != 0 ) {
            ++rval;
            bTryAgain = true;
            t = MULTIPLY2;
          }
       }
    }
  }
  if(rval > MAX_PATTERNS - 1) {
    rval = 0;
    for(int c(0) ; c < playLength ; ++c) {
      if(rval == playOrder[c]) {
        ++rval;
        c = -1;
      }
    }
    if(rval > MAX_PATTERNS - 1) rval = MAX_PATTERNS - 1;
  }
  return rval;
}


bool Song::AllocNewPattern(int pattern,char *name,int lines,bool adaptsize)
		{
			PatternEntry blank;
			unsigned char *toffset;
			if(adaptsize)
			{
				float step;
				if( patternLines[pattern] > lines ) 
				{
					step= (float)patternLines[pattern]/lines;
					for(int t=0;t<SONGTRACKS;t++)
					{
						toffset=_ptrack(pattern,t);
						int l;
						for(l = 1 ; l < lines; ++l)
						{
							std::memcpy(toffset + l * MULTIPLY, toffset + f2i(l * step) * MULTIPLY,EVENT_SIZE);
						}
						while(l < patternLines[pattern])
						{
							// This wouldn't be necessary if we really allocate a new pattern.
							std::memcpy(toffset + (l * MULTIPLY), &blank, EVENT_SIZE);
							++l;
						}
					}
					patternLines[pattern] = lines; ///< This represents the allocation of the new pattern
				}
				else if(patternLines[pattern] < lines)
				{
					step= (float)lines/patternLines[pattern];
					int nl= patternLines[pattern];
					for(int t=0;t<SONGTRACKS;t++)
					{
						toffset=_ptrack(pattern,t);
						for(int l=nl-1;l>0;l--)
						{
							std::memcpy(toffset + f2i(l * step) * MULTIPLY, toffset + l * MULTIPLY,EVENT_SIZE);
							int tz(f2i(l * step) - 1);
							while (tz > (l - 1) * step)
							{
								std::memcpy(toffset + tz * MULTIPLY, &blank, EVENT_SIZE);
								--tz;
							}
						}
					}
					patternLines[pattern] = lines; ///< This represents the allocation of the new pattern
				}
			}
			else
			{
				int l(patternLines[pattern]);
				while(l < lines)
				{
					// This wouldn't be necessary if we really allocate a new pattern.
					for(int t(0) ; t < SONGTRACKS ; ++t)
					{
						toffset=_ptrackline(pattern,t,l);
						memcpy(toffset,&blank,EVENT_SIZE);
					}
					++l;
				}
				patternLines[pattern] = lines;
			}
			std::sprintf(patternName[pattern], name);
			return true;
		}


bool Song::IsPatternUsed(int i)
		{
			bool bUsed = false;
			if (ppPatternData[i])
			{
				// we could also check to see if pattern is unused AND blank.
				for (int j = 0; j < playLength; j++)
				{
					if (playOrder[j] == i)
					{
						bUsed = true;
						break;
					}
				}

				if (!bUsed)
				{
					// check to see if it is empty
					PatternEntry blank;
					unsigned char * pData = ppPatternData[i];
					for (int j = 0; j < MULTIPLY2; j+= EVENT_SIZE)
					{
						if (memcmp(pData+j,&blank,EVENT_SIZE) != 0 )
						{
							bUsed = true;
							j = MULTIPLY2;
							break;
						}
					}
				}
			}
			return bUsed;
		}


int Song::GetFreeBus()
		{
			for(int c(0) ; c < MAX_BUSES ; ++c) if(!_pMachine[c]) return c;
			return -1; 
		}

		int Song::GetFreeFxBus()
		{
			for(int c(MAX_BUSES) ; c < MAX_BUSES * 2 ; ++c) if(!_pMachine[c]) return c;
			return -1; 
		}

bool Song::InsertConnection(int src, int dst, float value)
		{
			int freebus=-1;
			int dfreebus=-1;
			bool error=false;
			Machine *srcMac = _pMachine[src];
			Machine *dstMac = _pMachine[dst];
			if(!srcMac || !dstMac) return false;
			if(dstMac->_mode == MACHMODE_GENERATOR) return false;
			// Get a free output slot on the source machine
			for(int c(MAX_CONNECTIONS - 1) ; c >= 0 ; --c)
			{
				if(!srcMac->_connection[c]) freebus = c;
				// Checking that there's not an slot to the dest. machine already
				else if(srcMac->_outputMachines[c] == dst) error = true;
			}
			if(freebus == -1 || error) return false;
			// Get a free input slot on the destination machine
			error=false;
			for(int c=MAX_CONNECTIONS-1; c>=0; c--)
			{
				if(!dstMac->_inputCon[c]) dfreebus = c;
				// Checking if the destination machine is connected with the source machine to avoid a loop.
				else if(dstMac->_outputMachines[c] == src) error = true;
			}
			if(dfreebus == -1 || error) return false;
			// Calibrating in/out properties
			srcMac->_outputMachines[freebus] = dst;
			srcMac->_connection[freebus] = true;
			srcMac->_numOutputs++;
			dstMac->_inputMachines[dfreebus] = src;
			dstMac->_inputCon[dfreebus] = true;
			dstMac->_numInputs++;
			dstMac->InitWireVolume(srcMac->_type,dfreebus,value);
			return true;
		}

void Song::DeleteInstruments()
		{
			for(int i(0) ; i < MAX_INSTRUMENTS ; ++i) DeleteInstrument(i);
		}



		void Song::DeleteInstrument(int i)
		{
			Invalided=true;
			_pInstrument[i]->Delete();
			Invalided=false;
		}





bool Song::save(const std::string & fileName)
{
  Serializer f(fileName);
  uint32_t version, size, temp, chunkcount;

  //
  //===================
  //FILE HEADER
  //===================
  //id = "PSY3SONG";

  chunkcount = 3; // 3 chunks plus:
  for(unsigned int i(0) ; i < MAX_PATTERNS    ; ++i) if(IsPatternUsed(i))          ++chunkcount; // check every pattern for validity
  for(unsigned int i(0) ; i < MAX_MACHINES    ; ++i) if(_pMachine[i])              ++chunkcount;
  for(unsigned int i(0) ; i < MAX_INSTRUMENTS ; ++i) if(!_pInstrument[i]->Empty()) ++chunkcount;

  // chunk header
  f.PutPChar("PSY3SONG", 8);

  version = CURRENT_FILE_VERSION;
  f.PutInt(version);
  size = sizeof chunkcount;
  f.PutInt(size);
  // chunk data
  f.PutInt(chunkcount);

  // the rest of the modules can be arranged in any order

  //
  // ===================
  // SONG INFO TEXT
  // ===================
  // id = "INFO";
  //

  // chunk header
  f.PutPChar("INFO",4);

  version = CURRENT_FILE_VERSION_INFO;
  f.PutInt(version);
  size = strlen(Name)+strlen(Author)+strlen(Comment)+3; 
  // [bohan] since those are variable length, we could change from fixed size arrays to std::string
  f.PutInt(size);

  // chunk data

  std::cout << Name << std::endl;
  std::cout << Author << std::endl;
  std::cout << Comment << std::endl;


  f.PutString(Name);
  f.PutString(Author);
  f.PutString(Comment);

  //
  // ===================
  // SONG INFO
  // ===================
  // id = "SNGI"; 
  //

  // chunk header

  f.PutPChar("SNGI",4);
  version = CURRENT_FILE_VERSION_SNGI;
  f.PutInt(version);
  size = (11*sizeof(temp))+(SONGTRACKS*(sizeof(_trackMuted[0])+sizeof(_trackArmed[0])));
  f.PutInt(size);

  // chunk data

  f.PutInt( SONGTRACKS     );
  f.PutInt( m_BeatsPerMin  );
  f.PutInt( m_LinesPerBeat );
  f.PutInt( currentOctave  );
  f.PutInt( machineSoloed  );
  f.PutInt( _trackSoloed   );
  f.PutInt( seqBus         );
  f.PutInt( midiSelected   );
  f.PutInt( auxcolSelected );
  f.PutInt( instSelected   );
  f.PutInt( 1              ); // sequence width

  // chunk data

  for(unsigned int i = 0; i < SONGTRACKS; i++) {
    f.PutBool( _trackMuted[i] );
    f.PutBool( _trackArmed[i] ); // remember to count them
  }

  //
  // ===================
  // SEQUENCE DATA
  // ===================
  // id = "SEQD";
  //

  for(uint32_t index(0) ; index < MAX_SEQUENCES ; ++index) {

    char* pSequenceName = "seq0\0"; // This needs to be replaced when converting to Multisequenc

    // chunk header

    f.PutPChar("SEQD",4);

    version = CURRENT_FILE_VERSION_SEQD;
    f.PutInt(version);

    size = ((playLength+2)*sizeof(temp))+strlen(pSequenceName)+1;
    f.PutInt(size);

    // chunk data

    f.PutInt(index);      // Sequence Track number
    f.PutInt(playLength); // Sequence length

    f.PutString(pSequenceName); // Sequence Name

    for (unsigned int i = 0; i < playLength; i++)
    {
      f.PutInt(playOrder[i]);
    }
  }

  //
  // ===================
  // PATTERN DATA
  // ===================
  // id = "PATD"; 
  //

  for(uint32_t index(0) ; index < MAX_PATTERNS; ++index)
  {
     // check every pattern for validity
     if (IsPatternUsed(index))
     {
       // ok save it
       unsigned char * pSource = new unsigned char[SONGTRACKS*patternLines[index]*EVENT_SIZE];

       unsigned char * pCopy = pSource;

       for (int y = 0; y < patternLines[index]; y++)
       {
         unsigned char * pData = ppPatternData[index]+(y*MULTIPLY);
         std::memcpy(pCopy,pData,EVENT_SIZE*SONGTRACKS);
         pCopy+=EVENT_SIZE*SONGTRACKS;
       }

       uint32_t sizez77 = DataCompression::BEERZ77Comp2(pSource, &pCopy, SONGTRACKS*patternLines[index]*EVENT_SIZE);
       zapArray(pSource);

       // chunk header
       f.PutPChar("PATD",4);
       version = CURRENT_FILE_VERSION_PATD;
       f.PutInt(version);
       size = sizez77 + 4 * sizeof temp + strlen(patternName[index]) + 1;
       f.PutInt(size);

       // chunk data

       f.PutInt(index);
       f.PutInt(patternLines[index]);
       f.PutInt(SONGTRACKS); // eventually this may be variable per pattern
       f.PutString(patternName[index]);

       f.PutInt(sizez77);
       f.PutPChar((char*)pCopy,sizez77);

       zapArray(pCopy);
     }
   }

   //
   // ===================
   // MACHINE DATA
   // ===================
   // id = "MACD";
   // machine and instruments handle their save and load in their respective classes

   for(uint32_t index(0) ; index < MAX_MACHINES; ++index)
   {
      if (_pMachine[index])
      {
        std::size_t pos;
        // chunk header
        f.PutPChar("MACD",4);
        version = CURRENT_FILE_VERSION_MACD;
        f.PutInt(version);
        pos = f.GetPos();
        size = 0;
        f.PutInt(size);
        // chunk data
        f.PutInt(index);
          _pMachine[index]->SaveFileChunk(&f);
        // chunk size in header
          std::size_t const pos2(f.GetPos());
          size = pos2 - pos - sizeof size;
          f.Seek(pos);
          f.PutInt(size);
          f.Seek(pos2);
      }

   }

   //
   // ===================
   // Instrument DATA
   // ===================
   // id = "INSD"; 
   //
   for(uint32_t index(0) ; index < MAX_INSTRUMENTS; ++index)
   {
     if (!_pInstrument[index]->Empty())
     {
       std::size_t pos;
       // chunk header
       f.PutPChar("INSD",4);
       version = CURRENT_FILE_VERSION_INSD;
       f.PutInt(version);

       pos = f.GetPos();
       size = 0;
       f.PutInt(size);

       // chunk data

       f.PutInt(index);
       //_pInstrument[index]->SaveFileChunk(f);

       // chunk size in header

       std::size_t const pos2(f.GetPos());
       size = pos2 - pos - sizeof size;
       f.Seek(pos);
       f.PutInt(size);
       f.Seek(pos2);
     }
   }

   f.close();
}

int Song::WavAlloc(int iInstr, bool bStereo, long iSamplesPerChan, const char * sName)
		{
			///\todo what is ASSERT? some msicrosoft thingie?
			assert(iSamplesPerChan<(1<<30)); ///< Since in some places, signed values are used, we cannot use the whole range.
			DeleteLayer(iInstr);
			_pInstrument[iInstr]->waveDataL = new signed short[iSamplesPerChan];
			if(bStereo)
			{	_pInstrument[iInstr]->waveDataR = new signed short[iSamplesPerChan];
				_pInstrument[iInstr]->waveStereo = true;
			} else {
				_pInstrument[iInstr]->waveStereo = false;
			}
			_pInstrument[iInstr]->waveLength = iSamplesPerChan;
			std::strncpy(_pInstrument[iInstr]->waveName, sName, 31);
			_pInstrument[iInstr]->waveName[31] = '\0';
			std::strncpy(_pInstrument[iInstr]->_sName,sName,31);
			_pInstrument[iInstr]->_sName[31]='\0';
			return true;
		}

unsigned long Song::FourCC( const char *ChunkName)
{
   long retbuf = 0x20202020;   // four spaces (padding)
   char *p = ((char *)&retbuf);
   // Remember, this is Intel format!
   // The first character goes in the LSB
   for( int i(0) ; i < 4 && ChunkName[i]; ++i) *p++ = ChunkName[i];
   return retbuf;
}



int Song::WavAlloc(int instrument,const char * Wavfile)
{
  try {
    WaveDeSerializer file(Wavfile);
    ExtRiffChunkHeader hd;
    Invalided = true;

    // sample type	
    int st_type(file.NumChannels());
    int bits(file.BitsPerSample());
    long Datalen(file.NumSamples());

    // Initializes the layer.
    WavAlloc(instrument, st_type == 2, Datalen, Wavfile);

    // Reading of Wave data.
    // We don't use the WaveFile "ReadSamples" functions, because there are two main differences:
    // We need to convert 8bits to 16bits, and stereo channels are in different arrays.

    short * sampL(_pInstrument[instrument]->waveDataL);

    ///\todo use template code for all this semi-repetitive code.

    long io;

   // mono
   if(st_type == 1) {
     uint8_t smp8;
     switch(bits) {
        case 8:
           for(io = 0 ; io < Datalen ; ++io) {
             file.ReadData(&smp8, 1);
             *sampL = (smp8 << 8) - 32768;
             ++sampL;
           }
        break;
        case 16:
           file.ReadData(sampL, Datalen);
        break;
        case 24:
           for(io = 0 ; io < Datalen ; ++io) {
             file.ReadData(&smp8, 1);
             file.ReadData(sampL, 1);
             ++sampL;
           }
        break;
        default:
        break;
     }
   }
   // stereo
   else {
     short *sampR(_pInstrument[instrument]->waveDataR);
     uint8_t smp8;
     switch(bits) {
        case 8:
           for(io = 0 ; io < Datalen ; ++io) {
             file.ReadData(&smp8, 1);
             *sampL = (smp8 << 8) - 32768;
             ++sampL;
             file.ReadData(&smp8, 1);
             *sampR = (smp8 << 8) - 32768;
             ++sampR;
           }
        break;
        case 16:
          for(io = 0 ; io < Datalen ; ++io) {
            file.ReadData(sampL, 1);
            file.ReadData(sampR, 1);
            ++sampL;
            ++sampR;
          }
        break;
        case 24:
          for(io = 0 ; io < Datalen ; ++io) {
            file.ReadData(&smp8, 1);
            file.ReadData(sampL, 1);
            ++sampL;
            file.ReadData(&smp8, 1);
            file.ReadData(sampR, 1);
            ++sampR;
          }
        break;
        default:
           throw "something wrong";
        break;
      }
    }
    while (!file.eof()) {
      file.read((char*)(&hd), 8);
      if(hd.ckID == FourCC("smpl")) {
         char pl(0);
         file.skip(28);
         file.read(&pl, 1);
         if(pl == 1) {
           file.skip(15);
           unsigned int ls(0);
           unsigned int le(0);
           file.read((char*)&ls, 4);
           file.read((char*)&le, 4);
           _pInstrument[instrument]->waveLoopStart = ls;
           _pInstrument[instrument]->waveLoopEnd = le;
           // only for my bad sample collection
           //if(!((ls <= 0) && (le >= Datalen - 1)))
           {
             _pInstrument[instrument]->waveLoopType = true;
           }
         }
         file.skip(9);
       } else if(hd.ckSize > 0)
                file.skip(hd.ckSize);
              else
                file.skip(1);
        try {
          file.read((char*)(&hd), 8);
        }
        catch (const char*) {}
    }
    Invalided = false;
    return 1;
  } catch (const char*) {
    Invalided = false;
    return 0;
  }
}

void Song::DeleteLayer(int i)
		{
			_pInstrument[i]->DeleteLayer();
		}
