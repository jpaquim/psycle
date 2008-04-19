// -*- mode:c++; indent-tabs-mode:t -*-
/**************************************************************************
*   Copyright 2007 Psycledelics http://psycle.sourceforge.net             *
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
#include <psycle/core/psycleCorePch.hpp>

#include "plugin.h"
#include "player.h"
#include "fileio.h"

#include <sstream>
#if defined __unix__ || defined __APPLE__
	#include <dlfcn.h>
#elif defined _WIN32
	#include <windows.h>
#endif
// win32 note: plugins produced by mingw and msvc are binary-incompatible due to c++ abi ("this" pointer and std calling convention)

namespace psy { namespace core {


/**************************************************************************/
// PluginFxCallback

PluginFxCallback Plugin::_callback;

PluginFxCallback::~PluginFxCallback() throw() {}

void PluginFxCallback::MessBox(char const * /*ptxt*/, char const * /*caption*/, unsigned int /*type*/) {
	//MessageBox(hWnd,ptxt,caption,type); 
}

int PluginFxCallback::GetTickLength() { return static_cast<int>(Player::Instance()->timeInfo().samplesPerTick()); }
int PluginFxCallback::GetSamplingRate() { return Player::Instance()->timeInfo().sampleRate(); }
int PluginFxCallback::GetBPM() { return static_cast<int>(Player::Instance()->timeInfo().bpm()); }
int PluginFxCallback::GetTPB() { return Player::Instance()->timeInfo().ticksSpeed(); }

// dummy body
int PluginFxCallback::CallbackFunc(int, int, int, int) { return 0; }
float * PluginFxCallback::unused0(int, int) { return 0; }
float * PluginFxCallback::unused1(int, int) { return 0; }

/**************************************************************************/
// Plugin

Plugin::Plugin(MachineCallbacks* callbacks, MachineKey key,Machine::id_type id, void* libHandle, CMachineInfo* info, CMachineInterface* macIface )
:
	Machine(callbacks, id),
	libHandle_(libHandle),
	key_(key),
	info_(info),
	proxy_(*this,macIface)
{
	SetAudioRange(32768.0f);

	_isSynth = (info_->Flags == 3);
	if(!_isSynth) {
		defineInputAsStereo();
	}
	defineOutputAsStereo();

	strncpy(_psShortName,info_->ShortName,15);
	_psShortName[15]='\0';
	char buf[32];
	strncpy(buf, info_->ShortName,31);
	buf[31]='\0';
	SetEditName(buf);
	_psAuthor = info_->Author;
	_psName = info_->Name;

}

Plugin::~ Plugin( ) throw()
{
	if (proxy_()) {
		proxy_(0); // i.e. delete CMachineInterface.
	}
	if(libHandle_)
	#if defined __unix__ || defined __APPLE__
		::dlclose(libHandle_);
	#else
		::FreeLibrary((HINSTANCE)libHandle_);
	#endif
}

void Plugin::Init( )
{
	Machine::Init();

	if(proxy()())
	{
		proxy().Init();
		for(int gbp(0) ; gbp < GetInfo().numParameters ; ++gbp)
		{
			proxy().ParameterTweak(gbp, GetInfo().Parameters[gbp]->DefValue);
		}
	}
}

int Plugin::GenerateAudioInTicks(int startSample,  int numSamples )
{
	int ns = numSamples;
	int us = startSample;
	if(_isSynth)
	{
		if (!_mute) Standby(false);
		else Standby(true);
	}

	if (!_mute) {
		if((_isSynth) || (!_bypass && !Standby())) {
			proxy().Work(_pSamplesL+us, _pSamplesR+us, ns, callbacks->song().tracks());
		}
	}
	//CPUCOST_CALC(cost, numSamples);
	//_cpuCost += cost;
	_worked = true;
	return numSamples;
#if 0
	if (!_mute) {
		if ((mode() == MACHMODE_GENERATOR) || (!_bypass && !Standby())) {
			int ns = numSamples;
			int us = startSample;
			while (ns)
			{
				int nextevent = (TWSActive)?TWSSamples:ns+1;
				for (int i=0; i < song()->tracks(); i++)
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
					for (int i=0; i < song()->tracks(); i++)
					{
						// come back to this
						if (TriggerDelay[i]._cmd)
						{
							TriggerDelayCounter[i] -= ns;
						}
					}
					try
					{
						proxy().Work(_pSamplesL+us, _pSamplesR+us, ns, song()->tracks());
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
							proxy().Work(_pSamplesL+us, _pSamplesR+us, nextevent, song()->tracks());
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
					for (int i=0; i < song()->tracks(); i++)
					{
						// come back to this
						if (TriggerDelay[i]._cmd == commandtypes::NOTE_DELAY)
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
						else if (TriggerDelay[i]._cmd == commandtypes::RETRIGGER)
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
								TriggerDelayCounter[i] = (RetriggerRate[i]*Gloxxxxxxxxxxxxxxxxxxxxbal::pPlayer()->SamplesPerRow())/256;
							} else
							{
								TriggerDelayCounter[i] -= nextevent;
							}
						}
						else if (TriggerDelay[i]._cmd == commandtypes::RETR_CONT)
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
								TriggerDelayCounter[i] = (RetriggerRate[i]*Gloxxxxxxxxxxxxxxxxxxxxxxxbal::pPlayer()->SamplesPerRow())/256;
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
						} else if (TriggerDelay[i]._cmd == commandtypes::ARPEGGIO)
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
								TriggerDelayCounter[i] = Gloxxxxxxxxxxxxxxxbal::pPlayer()->SamplesPerRow()*Gloxxxxxxxxxxxxxxxxxxbal::pPlayer()->tpb/24;
							} else {
								TriggerDelayCounter[i] -= nextevent;
							}
						}
					}
				}

			}
			Machine::UpdateVuAndStanbyFlag(numSamples);
		}
	}
	//CPUCOST_CALC(cost, numSamples);
	//_cpuCost += cost;
	_worked = true;
#endif // 0
}

void Plugin::Tick( )
{
	try {
		proxy().SequencerTick();
	}
	catch(const std::exception &) {
	}
}

void Plugin::Tick( int channel, const PatternEvent & pData )
{
	const PlayerTimeInfo & timeInfo = Player::Instance()->timeInfo();
///FIXME: Add the Information about the tweaks.
#if 0
	if(pData.note() == notetypes::tweak)
	{
		if( pData.instrument() < info_->numParameters)
		{
			int nv = (pData.command() << 8) +pData.parameter();
			int const min = info_->Parameters[pData.instrument()]->MinValue;
			int const max = info_->Parameters[pData.instrument()]->MaxValue;
			nv += min;
			if(nv > max) nv = max;
			try
			{
				proxy().ParameterTweak(pData.instrument(), nv);
			}
			catch(const std::exception &)
			{
			}
			Player::Instance()->Tweaker = true;
		}
	}
	else if(pData.note() == notetypes::tweak_slide)
	{
		if(pData.instrument() < info_->numParameters)
		{
			int i;
			if(TWSActive)
			{
				// see if a tweak slide for this parameter is already happening
				for(i = 0; i < MAX_TWS; i++)
				{
					if((TWSInst[i] == pData.instrument()) && (TWSDelta[i] != 0))
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
				TWSDestination[i] = float(pData.command() << 8)+pData.parameter();
				float min = float(info_->Parameters[pData.instrument()]->MinValue);
				float max = float(info_->Parameters[pData.instrument()]->MaxValue);
				TWSDestination[i] += min;
				if (TWSDestination[i] > max)
				{
					TWSDestination[i] = max;
				}
				TWSInst[i] = pData.instrument();
				try
				{
					TWSCurrent[i] = float(proxy().Vals()[TWSInst[i]]);
				}
				catch(const std::exception &)
				{
				}
				TWSDelta[i] = float((TWSDestination[i]-TWSCurrent[i])*TWEAK_SLIDE_SAMPLES)/ timeInfo.samplesPerTick();
				TWSSamples = 0;
				TWSActive = true;
			}
			else
			{
				// we have used all our slots, just send a twk
				int nv = (pData.command() << 8)+pData.parameter();
				int const min = info_->Parameters[pData.instrument()]->MinValue;
				int const max = info_->Parameters[pData.instrument()]->MaxValue;
				nv += min;
				if (nv > max) nv = max;
				try
				{
					proxy().ParameterTweak(pData.instrument(), nv);
				}
				catch(const std::exception &)
				{
				}
			}
		}
		Player::Instance()->Tweaker = true;
	}
	else
	{
		try
		{
			proxy().SeqTick(channel, pData.note(), pData.instrument(), pData.command(), pData.parameter());
		}
		catch(const std::exception &)
		{
			return;
		}
	}
#endif
}

void Plugin::Stop( )
{
	try {
		proxy().Stop();
	} catch(const std::exception &) {
		///\todo huh!
	}
	Machine::Stop();
}

void Plugin::GetParamName(int numparam, char * name) const
{
	if( numparam < info_->numParameters ) std::strcpy(name,info_->Parameters[numparam]->Name);
	else std::strcpy(name, "Out of Range");

}

void Plugin::GetParamRange(int numparam,int &minval,int &maxval) const
{
	if( numparam < info_->numParameters ) {
		if(GetInfo().Parameters[numparam]->Flags & MPF_STATE) {
			minval = GetInfo().Parameters[numparam]->MinValue;
			maxval = GetInfo().Parameters[numparam]->MaxValue;
		} else minval = maxval = 0;
	} else minval = maxval = 0;
}

int Plugin::GetParamValue(int numparam) const
{
	if(numparam < info_->numParameters) {
		try {
			return proxy().Vals()[numparam];
		} catch(const std::exception &) {
			return -1; // hmm
		}
	} else return -1; // hmm
}

void Plugin::GetParamValue(int numparam, char * parval) const
{
	if(numparam < info_->numParameters) {
		try {
			if(!proxy().DescribeValue(parval, numparam, proxy().Vals()[numparam]))
				std::sprintf(parval, "%i", proxy().Vals()[numparam]);
		}
		catch(const std::exception &)
		{}
		catch (...)
		{}
	}
	else std::strcpy(parval,"Out of Range");
}

bool Plugin::SetParameter(int numparam,int value)
{
	if(numparam < info_->numParameters) {
		try {
			proxy().ParameterTweak(numparam,value);
		}
		catch(const std::exception &) {
			return false;
		}
		return true;
	} else return false;
}

bool Plugin::LoadSpecificChunk(RiffFile* pFile, int version)
{
	std::uint32_t size;
	pFile->Read(size); // size of whole structure
	if(size)
	{
		if(version > CURRENT_FILE_VERSION_MACD)
		{
			pFile->Skip(size);
			std::ostringstream s; s
				<< version << " > " << CURRENT_FILE_VERSION_MACD << std::endl
				<< "Data is from a newer format of psycle, it might be unsafe to load." << std::endl;
			//MessageBox(0, s.str().c_str(), "Loading Error", MB_OK | MB_ICONWARNING);
			return false;
		}
		else
		{
			std::uint32_t count;
			pFile->Read(count);  // size of vars
			/*
			if (count)
			{
			pFile->ReadChunk(_pInterface->Vals,sizeof(_pInterface->Vals[0])*count);
			}
			*/
			for(unsigned int i(0) ; i < count ; ++i)
			{
				std::uint32_t temp;
				pFile->Read(temp);
				SetParameter(i, temp);
			}
			size -= sizeof(count) + sizeof(std::uint32_t) * count;
			if(size)
			{
				char * pData = new char[size];
				pFile->ReadArray(pData, size); // Number of parameters
				try
				{
					proxy().PutData(pData); // Internal load
				}
				catch(std::exception const &)
				{
					delete pData;
					return false;
				}
				delete pData;
				return true;
			}
		}
	}
	return true;
};

void Plugin::SaveSpecificChunk(RiffFile* pFile) const
{
	std::uint32_t count = GetNumParams();
	std::uint32_t size2(0);
	try
	{
		size2 = proxy().GetDataSize();
	}
	catch(std::exception const &)
	{
		// data won't be saved
	}
	std::uint32_t size = size2 + sizeof count  + sizeof(std::uint32_t) * count;
	pFile->Write(size);
	pFile->Write(count);
	for(unsigned int i(0) ; i < count ; ++i) {
		std::uint32_t temp = GetParamValue(i);
		pFile->Write(temp);
	}
	if(size2)
	{
		char * pData = new char[size2];
		try
		{
			proxy().GetData(pData); // Internal save
		}
		catch(std::exception const &)
		{
			// this sucks because we already wrote the size,
			// so now we have to write the data, even if they are corrupted.
		}
		pFile->WriteArray(pData, size2); // Number of parameters
		delete[] pData;
	}
};

}}
