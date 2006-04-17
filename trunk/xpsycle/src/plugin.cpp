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
#include "plugin.h"
#include "inputhandler.h"
#include "serializer.h"
#include "deserializer.h"
#include <nfile.h>
#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <inttypes.h>

typedef CMachineInfo * (* GETINFO) ();
typedef CMachineInterface * (* CREATEMACHINE) ();


PluginFxCallback Plugin::_callback;

Plugin::Plugin( int index ): _dll(0)
  , proxy_(*this)
  , _psAuthor("")
  , _psDllName("")
  , _psName("")
{
  _macIndex = index;
  _type = MACH_PLUGIN;
  _mode = MACHMODE_FX;
  std::sprintf(_editName, "native plugin");
}

Plugin::~ Plugin( )
{
}

bool Plugin::Instance(std::string file_name) throw()
{
  _dll = dlopen(file_name.c_str(), RTLD_LAZY);
   if (!_dll) {
        std::cerr << "Cannot load library: " << dlerror() << '\n';
        return false;
   } else {
     GETINFO GetInfo  = (GETINFO) dlsym(_dll, "GetInfo");
     if (!GetInfo) {
        std::cerr << "Cannot load symbols: " << dlerror() << '\n';
        return false;
     } else {
       _pInfo = GetInfo();
       std::cout << _pInfo->Author << std::endl;

       if(_pInfo->Version < MI_VERSION) std::cerr << "plugin format is too old" << _pInfo->Version << file_name <<std::endl;
       fflush(stdout);
       _isSynth = _pInfo->Flags == 3;
       if(_isSynth) _mode = MACHMODE_GENERATOR;
      strncpy(_psShortName,_pInfo->ShortName,15);
      _psShortName[15]='\0';
      strncpy(_editName, _pInfo->ShortName,31);
      _editName[31]='\0';
      _psAuthor = _pInfo->Author;
      _psName = _pInfo->Name;
      CREATEMACHINE GetInterface = (CREATEMACHINE) dlsym(_dll, "CreateMachine");
      if(!GetInterface) {
         std::cerr << "Cannot load symbol: " << dlerror() << "\n";
         return false;
      } else {
          proxy()(GetInterface());
      }
    }
   }
  return true;
}


void Plugin::Init( )
{
  Machine::Init();
  if(proxy()())
  {
    proxy().Init();
    for(int gbp(0) ; gbp < GetInfo()->numParameters ; ++gbp)
    {
      proxy().ParameterTweak(gbp, _pInfo->Parameters[gbp]->DefValue);
    }
  }
}

void Plugin::Work( int numSamples )
{
  if(_mode != MACHMODE_GENERATOR) Machine::Work(numSamples);
  if (!_mute) {
    if ((_mode == MACHMODE_GENERATOR) || (!_bypass && !_stopped)) {
      int ns = numSamples;
      int us = 0;
         while (ns)
         {
            int nextevent = (TWSActive)?TWSSamples:ns+1;
            for (int i=0; i < Global::pSong()->SONGTRACKS; i++)
            {
              if (TriggerDelay[i]._cmd) {
                  if (TriggerDelayCounter[i] < nextevent)
                  {
                    nextevent = TriggerDelayCounter[i];
                  }
              }
            }
            if (nextevent > ns)
            {
               if (TWSActive)
               {
                 TWSSamples -= ns;
               }
               for (int i=0; i < Global::pSong()->SONGTRACKS; i++)
               {
                  // come back to this
                  if (TriggerDelay[i]._cmd)
                  {
                    TriggerDelayCounter[i] -= ns;
                  }
               }
               try
               {
                  proxy().Work(_pSamplesL+us, _pSamplesR+us, ns, Global::pSong()->SONGTRACKS);
               }
               catch(const std::exception &)
               {
               }
               ns = 0;
           } else {
                    if(nextevent) {
                       ns -= nextevent;
                       try
                       {
                          proxy().Work(_pSamplesL+us, _pSamplesR+us, nextevent, Global::pSong()->SONGTRACKS);
                       }
                       catch(const std::exception &)
                       {
                       }
                       us += nextevent;
                    }
                    if (TWSActive)
                    {
                      if (TWSSamples == nextevent)
                      {
                         int activecount = 0;
                         TWSSamples = TWEAK_SLIDE_SAMPLES;
                         for (int i = 0; i < MAX_TWS; i++)
                         {
                           if (TWSDelta[i] != 0)
                           {
                              TWSCurrent[i] += TWSDelta[i];
                              if (((TWSDelta[i] > 0) && (TWSCurrent[i] >= TWSDestination[i])) || ((TWSDelta[i] < 0) && (TWSCurrent[i] <= TWSDestination[i])))
                              {
                                 TWSCurrent[i] = TWSDestination[i];
                                 TWSDelta[i] = 0;
                              } else
                              {
                                activecount++;
                              }
                              try
                              {
                                 proxy().ParameterTweak(TWSInst[i], int(TWSCurrent[i]));
                              }
                              catch(const std::exception &)
                              {
                              }
                          }
                        }
                        if(!activecount) TWSActive = false;
                      }
                    }
                    for (int i=0; i < Global::pSong()->SONGTRACKS; i++)
                    {
                       // come back to this
                       if (TriggerDelay[i]._cmd == PatternCmd::NOTE_DELAY)
                       {
                          if (TriggerDelayCounter[i] == nextevent)
                          {
                             // do event
                             try
                             {
                               proxy().SeqTick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
                             }
                             catch(const std::exception &)
                            {
                            }
                            TriggerDelay[i]._cmd = 0;
                          }
                          else
                          {
                            TriggerDelayCounter[i] -= nextevent;
                          }
                      }
                      else if (TriggerDelay[i]._cmd == PatternCmd::RETRIGGER)
                      {
                         if (TriggerDelayCounter[i] == nextevent)
                         {
                           // do event
                           try
                           {
                             proxy().SeqTick(i, TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
                           }
                           catch(const std::exception &)
                           {
                           }
                           TriggerDelayCounter[i] = (RetriggerRate[i]*Global::pPlayer()->SamplesPerRow())/256;
                         } else
                         {
                           TriggerDelayCounter[i] -= nextevent;
                         }
                      }
                      else if (TriggerDelay[i]._cmd == PatternCmd::RETR_CONT)
                      {
                         if (TriggerDelayCounter[i] == nextevent)
                         {
                            // do event
                            try
                            {
                              proxy().SeqTick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
                            }
                            catch(const std::exception &)
                            {
                            }
                            TriggerDelayCounter[i] = (RetriggerRate[i]*Global::pPlayer()->SamplesPerRow())/256;
                            int parameter = TriggerDelay[i]._parameter&0x0f;
                            if (parameter < 9)
                            {
                               RetriggerRate[i]+= 4*parameter;
                            } else
                            {
                              RetriggerRate[i]-= 2*(16-parameter);
                              if (RetriggerRate[i] < 16)
                              {
                                 RetriggerRate[i] = 16;
                              }
                            }
                          }
                          else
                          {
                            TriggerDelayCounter[i] -= nextevent;
                          }
                        } else if (TriggerDelay[i]._cmd == PatternCmd::ARPEGGIO)
                        {
                          if (TriggerDelayCounter[i] == nextevent)
                          {
                             PatternEntry entry =TriggerDelay[i];
                             switch(ArpeggioCount[i])
                             {
                               case 0:
                               try
                               {
                                  proxy().SeqTick(i ,TriggerDelay[i]._note, TriggerDelay[i]._inst, 0, 0);
                               }
                               catch(const std::exception &)
                               {
                               }
                               ArpeggioCount[i]++;
                               break;
                               case 1:
                                 entry._note+=((TriggerDelay[i]._parameter&0xF0)>>4);
                                 try
                                 {
                                    proxy().SeqTick(i ,entry._note, entry._inst, 0, 0);
                                 }
                                 catch(const std::exception &)
                                 {
                                 }
                                 ArpeggioCount[i]++;
                               break;
                               case 2:
                                 entry._note+=(TriggerDelay[i]._parameter&0x0F);
                                 try
                                   {
                                     proxy().SeqTick(i ,entry._note, entry._inst, 0, 0);
                                   }
                                   catch(const std::exception &)
                                   {
                                   }
                                   ArpeggioCount[i]=0;
                                   break;
                                }
                                TriggerDelayCounter[i] = Global::pPlayer()->SamplesPerRow()*Global::pPlayer()->tpb/24;
                               } else {
                                   TriggerDelayCounter[i] -= nextevent;
                               }
                             }
                          }
                       }

           }
           Machine::SetVolumeCounter(numSamples);
           if ( Global::pConfig()->autoStopMachines )
           {
              if (_volumeCounter < 8.0f)
              {
                _volumeCounter = 0.0f;
                _volumeDisplay = 0;
                _stopped = true;
              }
              else _stopped = false;
           }
        }
     }
     //CPUCOST_CALC(cost, numSamples);
     //_cpuCost += cost;
     _worked = true;
}

void Plugin::Tick()
{
  try {
     proxy().SequencerTick();
  }
  catch(const std::exception &) {
  }
}

void Plugin::Tick( int channel, PatternEntry * pData )
{
  try
  {
      proxy().SeqTick(channel, pData->_note, pData->_inst, pData->_cmd, pData->_parameter);
  }
  catch(const std::exception &)
  {
     return;
  }
  if(pData->_note == cdefTweakM || pData->_note == cdefTweakE)
  {
     if(pData->_inst < _pInfo->numParameters)
     {
        int nv = (pData->_cmd<<8)+pData->_parameter;
					int const min = _pInfo->Parameters[pData->_inst]->MinValue;
					int const max = _pInfo->Parameters[pData->_inst]->MaxValue;
					nv += min;
					if(nv > max) nv = max;
					try
					{
						proxy().ParameterTweak(pData->_inst, nv);
					}
					catch(const std::exception &)
					{
					}
					Global::pPlayer()->Tweaker = true;
				}
			}
			else if(pData->_note == cdefTweakS)
			{
				if(pData->_inst < _pInfo->numParameters)
				{
					int i;
					if(TWSActive)
					{
						// see if a tweak slide for this parameter is already happening
						for(i = 0; i < MAX_TWS; i++)
						{
							if((TWSInst[i] == pData->_inst) && (TWSDelta[i] != 0))
							{
								// yes
								break;
							}
						}
						if(i == MAX_TWS)
						{
							// nope, find an empty slot
							for (i = 0; i < MAX_TWS; i++)
							{
								if (TWSDelta[i] == 0)
								{
									break;
								}
							}
						}
					}
					else
					{
						// wipe our array for safety
						for (i = MAX_TWS-1; i > 0; i--)
						{
							TWSDelta[i] = 0;
						}
					}
					if (i < MAX_TWS)
					{
						TWSDestination[i] = float(pData->_cmd<<8)+pData->_parameter;
						float min = float(_pInfo->Parameters[pData->_inst]->MinValue);
						float max = float(_pInfo->Parameters[pData->_inst]->MaxValue);
						TWSDestination[i] += min;
						if (TWSDestination[i] > max)
						{
							TWSDestination[i] = max;
						}
						TWSInst[i] = pData->_inst;
						try
						{
							TWSCurrent[i] = float(proxy().Vals()[TWSInst[i]]);
						}
						catch(const std::exception &)
						{
						}
						TWSDelta[i] = float((TWSDestination[i]-TWSCurrent[i])*TWEAK_SLIDE_SAMPLES)/Global::pPlayer()->SamplesPerRow();
						TWSSamples = 0;
						TWSActive = true;
					}
					else
					{
						// we have used all our slots, just send a twk
						int nv = (pData->_cmd<<8)+pData->_parameter;
						int const min = _pInfo->Parameters[pData->_inst]->MinValue;
						int const max = _pInfo->Parameters[pData->_inst]->MaxValue;
						nv += min;
						if (nv > max) nv = max;
						try
						{
							proxy().ParameterTweak(pData->_inst, nv);
						}
						catch(const std::exception &)
						{
						}
					}
				}
				Global::pPlayer()->Tweaker = true;
			}
}

void Plugin::Stop( )
{
  try {
     proxy().Stop();
  }
  catch(const std::exception &)
  {
  }
}

struct ToLower
   {
     char operator() (char c) const  { return std::tolower(c); }
   };

bool Plugin::LoadDll( std::string psFileName )
{
  std::transform(psFileName.begin(),psFileName.end(),psFileName.begin(),ToLower());
  if (psFileName.find(".so")== std::string::npos) {
    _psDllName = psFileName;
    int i = psFileName.find(".dll");
    std::string withoutSuffix = psFileName.substr(0,i);
    std::string soName = withoutSuffix + ".so";
    psFileName = "lib-psycle.plugin."+soName;
    psFileName = Global::pConfig()->pluginPath + psFileName;
    int pos;
    while((pos = psFileName.find(' ')) != std::string::npos) psFileName[pos] = '_';
  } else {
      int i = psFileName.find("lib-psycle.plugin.");
      if (i!=std::string::npos) {
         int j = psFileName.find(".so");
         if (j!=0) {
            _psDllName = psFileName.substr(0,j);
            _psDllName.erase(0,std::string("lib-psycle.plugin.").length());
            _psDllName = _psDllName + ".dll";
         } else {
            _psDllName = psFileName;
            _psDllName.erase(0,std::string("lib-psycle.plugin.").length());
            _psDllName = _psDllName + ".dll";
         }
      } else _psDllName = psFileName;

      psFileName = Global::pConfig()->pluginPath + psFileName;
  }

  return Instance(psFileName);
}


void Plugin::GetParamName(int numparam, char * name)
		{
			if( numparam < _pInfo->numParameters ) std::strcpy(name,_pInfo->Parameters[numparam]->Name);
			else std::strcpy(name, "Out of Range");

		}
		void Plugin::GetParamRange(int numparam,int &minval,int &maxval)
		{
			if(GetInfo()->Parameters[numparam]->Flags & MPF_STATE)
			{
				minval = GetInfo()->Parameters[numparam]->MinValue;
				maxval = GetInfo()->Parameters[numparam]->MaxValue;
			}
			else
			{
				minval = maxval = 0;
			}
		}
		int Plugin::GetParamValue(int numparam)
		{
			if(numparam < _pInfo->numParameters)
			{
				try
				{
					return proxy().Vals()[numparam];
				}
				catch(const std::exception &)
				{
					return -1; // hmm
				}
			}
			else return -1; // hmm
		}

		void Plugin::GetParamValue(int numparam, char * parval)
		{
			if(numparam < _pInfo->numParameters)
			{
				try
				{
					if(!proxy().DescribeValue(parval, numparam, proxy().Vals()[numparam]))
						std::sprintf(parval, "%i", proxy().Vals()[numparam]);
				}
				catch(const std::exception &)
				{
				}
			}
			else std::strcpy(parval,"Out of Range");
		}

bool Plugin::SetParameter(int numparam,int value)
		{
			if(numparam < _pInfo->numParameters)
			{
				try
				{
					proxy().ParameterTweak(numparam,value);
				}
				catch(const std::exception &)
				{
					return false;
				}
				return true;
			}
			else return false;
		}

void Plugin::SaveDllName( Serializer * pFile )
{
  const char* str  = _psDllName.c_str();
  std::cout << _psDllName << std::endl;
  //char str2[256];
  //strcpy(str2,str.Mid(str.ReverseFind('\\')+1));
  pFile->PutString(str);
}

bool Plugin::LoadSpecificChunk( DeSerializer * pFile, int version )
{
  uint32_t size = pFile->getInt(); // size of whole structure
  if(size) {
      if(version > CURRENT_FILE_VERSION_MACD)  {
          pFile->skip(size);
         std::ostringstream s; 
         s << version << " > " << CURRENT_FILE_VERSION_MACD << std::endl
         << "Data is from a newer format of psycle, it might be unsafe to load." << std::endl;
         //MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
          return false;
      } else
      {
         uint32_t count = pFile->getInt(); // size of vars
         for(unsigned int i(0) ; i < count ; ++i)
         {
            uint32_t temp = pFile->getInt();
            SetParameter(i, temp);
         }
         size -= sizeof(count) + sizeof(uint32_t) * count;
         if(size) {
           char * pData = new char[size];
           pFile->read(pData, size); // Number of parameters
           try
           {
              proxy().PutData(pData); // Internal load
           }
           catch(std::exception const &)
           {
              return false;
           }
           return true;
        }
      }
   }
   return true;
}


void Plugin::SaveSpecificChunk(Serializer* pFile)
{
  int count = GetNumParams();
  int size2(0);
  try
  {
     size2 = proxy().GetDataSize();
  }
  catch(const std::exception &)
  {
    // data won't be saved
  }
  int size = size2 + sizeof(count) + sizeof(int)*count;
  pFile->PutInt(size);
  pFile->PutInt(count);
  //pFile->Write(_pInterface->Vals,sizeof(_pInterface->Vals[0])*count);
  for (int i = 0; i < count; i++)
  {
    int temp = GetParamValue(i);
    pFile->PutInt(temp);
  }
  if(size2)
  {
    byte * pData = new byte[size2];
    try
    {
       proxy().GetData(pData); // Internal save
    }
    catch(const std::exception &)
    {
        // this sucks because we already wrote the size,
       // so now we have to write the data, even if they are corrupted.
    }
    pFile->PutPChar((char*)pData, size2); // Number of parameters
    delete pData; pData = 0;
  }
};

