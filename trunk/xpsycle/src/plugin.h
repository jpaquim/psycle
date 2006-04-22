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
 *   59 Temple Place - Suite 330, Boston, MA  02?111-1307, USA.             *
 ***************************************************************************/
#ifndef PLUGIN_H
#define PLUGIN_H

#include "machine.h"
#include "plugin_interface.h"
#include "player.h"
#include "global.h"
#include "configuration.h"
#include <cassert>

/**
@author Stefan
*/




class PluginFxCallback : public CFxCallback
{
  public:
    //HWND hWnd;
    inline virtual void MessBox(char* ptxt,char *caption,unsigned int type) { //MessageBox(hWnd,ptxt,caption,type); 
    }
    inline virtual int GetTickLength() { return Global::pPlayer()->SamplesPerRow(); }
    inline virtual int GetSamplingRate() { return Global::pConfig()->GetSamplesPerSec(); }
    inline virtual int GetBPM() { return Global::pPlayer()->bpm; }
    inline virtual int GetTPB() { return Global::pPlayer()->tpb; }
};

class Plugin; // forward declaration
/// Proxy between the host and a plugin.

class Proxy
{
  private:
    Plugin & host_;
    CMachineInterface * plugin_;
  private:
    inline Plugin & host() throw();
    inline const Plugin & host() const throw();
    inline CMachineInterface & plugin() throw();
    inline const CMachineInterface & plugin() const throw();
public:
    Proxy(Plugin & host, CMachineInterface * plugin = 0) : host_(host), plugin_(0) { (*this)(plugin); }

    ~Proxy() throw() { (*this)(0);}
    inline const bool operator()() const throw();
    inline void operator()(CMachineInterface * plugin) throw(); //exceptions::function_error);
    inline void Init() throw(); //std::exceptions::function_error);
    inline void SequencerTick() throw(); //exceptions::function_error);
    inline void ParameterTweak(int par, int val) throw(); //exceptions::function_error);
    inline void Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) throw(); //exceptions::function_error);
    inline void Stop() throw(); //exceptions::function_error);
    inline void PutData(void * pData) throw(); //exceptions::function_error);
    inline void GetData(void * pData) throw(); //exceptions::function_error);
    inline int GetDataSize() throw(); //exceptions::function_error);
    inline void Command() throw(); //exceptions::function_error);
    inline void MuteTrack(const int i) throw(); //exceptions::function_error);
    inline bool IsTrackMuted(const int i) throw(); //exceptions::function_error);
    inline void MidiNote(const int channel, const int value, const int velocity) throw(); //exceptions::function_error);
    inline void Event(const dword data) throw(); //exceptions::function_error);
    inline bool DescribeValue(char * txt, const int param, const int value) throw(); //exceptions::function_error);
    inline bool PlayWave(const int wave, const int note, const float volume) throw(); //exceptions::function_error);
    inline void SeqTick(int channel, int note, int ins, int cmd, int val) throw(); //exceptions::function_error);
    inline void StopWave() throw(); //exceptions::function_error);
    inline int * Vals() throw(); //exceptions::function_error);
    inline void callback() throw(); //exceptions::function_error);
};


class Plugin : public Machine{
private:
  static PluginFxCallback _callback;
  public:
     inline static PluginFxCallback * GetCallback() throw() { return &_callback; };
public:
    Plugin(int index);

    virtual ~Plugin();

    virtual void Init();
    virtual void Work(int numSamples);
    virtual void Tick();
    virtual void Tick(int channel, PatternEntry * pEntry);
    virtual void Stop();
    inline virtual const char * const GetDllName() const throw() { return _psDllName.c_str(); }
    virtual char * GetName() { return (char *)_psName.c_str(); };

    virtual int GetNumParams() { return GetInfo()->numParameters; };
    virtual int GetNumCols() { return GetInfo()->numCols; };
    virtual void GetParamName(int numparam, char * name);
    virtual void GetParamRange(int numparam,int &minval, int &maxval);
    virtual int GetParamValue(int numparam);
    virtual void GetParamValue(int numparam,char* parval);
    virtual bool SetParameter(int numparam,int value);

    inline Proxy & proxy() throw() { return proxy_; };

    bool Instance(std::string const & file_name);
    bool LoadDll(std::string const & psFileName);
    virtual void SaveDllName(Serializer * pFile);
    virtual bool LoadSpecificChunk(DeSerializer * pFile, int version);
    virtual void SaveSpecificChunk(Serializer* pFile);

    inline CMachineInfo * GetInfo() throw() { return _pInfo; };

private:
   void* _dll;
   char _psShortName[16];
   std::string _psAuthor;
   std::string _psDllName;
   std::string _psName;
   bool _isSynth;
   CMachineInfo * _pInfo;
   Proxy proxy_;
};


inline void Proxy::Init() throw()
{ assert((*this)()); plugin().Init(); }
inline CMachineInterface & Proxy::plugin() throw() { return *plugin_; }
inline void Proxy::SequencerTick() throw() { plugin().SequencerTick(); }
inline void Proxy::ParameterTweak(int par, int val) throw()
{ assert((*this)()); plugin().ParameterTweak(par, val);  }
inline Plugin & Proxy::host() throw() { return host_; }
inline const Plugin & Proxy::host() const throw() { return host_; }

inline void Proxy::callback() throw()
		{ assert((*this)()); plugin().pCB = host().GetCallback(); }

inline const bool Proxy::operator()() const throw() { return plugin_; }
inline void Proxy::operator()(CMachineInterface * plugin) throw()//exceptions::function_error)
{
  zapObject(this->plugin_,plugin);
   //if((*this)())
   if(plugin) {
       callback();
       //Init(); // [bohan] i can't call that here. It would be best, some other parts of psycle want to call it to. We need to get rid of the other calls.
   }
}

inline void Proxy::SeqTick(int channel, int note, int ins, int cmd, int val) throw()
{ assert((*this)()); plugin().SeqTick(channel, note, ins, cmd, val); }
inline void Proxy::StopWave() throw()
{ assert((*this)());plugin().StopWave(); }
inline void Proxy::Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) throw()
{ assert((*this)()); fflush(stdout); plugin().Work(psamplesleft, psamplesright, numsamples, tracks);  }
inline int * Proxy::Vals() throw()
{ assert((*this)()); return plugin().Vals; }
inline void Proxy::Stop() throw()
{ assert((*this)()); plugin().Stop();  }
inline bool Proxy::DescribeValue(char * txt, const int param, const int value) throw()
{ assert((*this)()); plugin().DescribeValue(txt, param, value); }
inline void Proxy::PutData(void * pData) throw()
{ assert((*this)()); plugin().PutData(pData);  }
inline void Proxy::GetData(void * pData) throw()
{ assert((*this)()); plugin().GetData(pData); }
inline int Proxy::GetDataSize() throw()
{ assert((*this)()); return plugin().GetDataSize(); }


#endif
