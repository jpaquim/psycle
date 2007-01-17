/***************************************************************************
  *   Copyright (C) 2006 by  Stefan   *
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
#include <cassert>
#include "global.h"

/**
@author  Stefan
*/

namespace psycle {
	namespace host {


class PluginFxCallback : public CFxCallback
{
  public:
    //HWND hWnd;
    inline virtual void MessBox(char* ptxt,char *caption,unsigned int type) { //MessageBox(hWnd,ptxt,caption,type); 
    }
    inline virtual int GetTickLength() { return static_cast<int>(Player::Instance()->timeInfo().samplesPerRow()); }
    inline virtual int GetSamplingRate() { return Player::Instance()->timeInfo().sampleRate(); }
    inline virtual int GetBPM() { return static_cast<int>(Player::Instance()->timeInfo().bpm()); }
    inline virtual int GetTPB() { return Player::Instance()->timeInfo().linesPerBeat(); }
};

class Plugin;  // forward declaration
/// Proxy between the host and a plugin.

class Proxy
{
  private:
    Plugin & host_;
    CMachineInterface * plugin_;
  private:
    inline Plugin & host();
    inline const Plugin & host() const ;
    inline CMachineInterface & plugin();
    inline const CMachineInterface & plugin() const ;
public:
    Proxy(Plugin & host, CMachineInterface * plugin = 0) : host_(host), plugin_(0) { (*this)(plugin); }

    ~Proxy() throw () { 
      // (*this)(0);  ///\todo this segfaults under windows .. investigate 
    }
    
    inline const bool operator()() const ;
    inline void operator()(CMachineInterface * plugin); //exceptions::function_error);
    inline void Init(); //std::exceptions::function_error);
    inline void SequencerTick(); //exceptions::function_error);
    inline void ParameterTweak(int par, int val); //exceptions::function_error);
    inline void Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks); //exceptions::function_error);
    inline void Stop(); //exceptions::function_error);
    inline void PutData(void * pData); //exceptions::function_error);
    inline void GetData(void * pData); //exceptions::function_error);
    inline int GetDataSize(); //exceptions::function_error);
    inline void Command(); //exceptions::function_error);
    inline void MuteTrack(const int i); //exceptions::function_error);
    inline bool IsTrackMuted(const int i); //exceptions::function_error);
    inline void MidiNote(const int channel, const int value, const int velocity); //exceptions::function_error);
    inline void Event(const dword data); //exceptions::function_error);
    inline bool DescribeValue(char * txt, const int param, const int value); //exceptions::function_error);
    inline bool PlayWave(const int wave, const int note, const float volume); //exceptions::function_error);
    inline void SeqTick(int channel, int note, int ins, int cmd, int val); //exceptions::function_error);
    inline void StopWave(); //exceptions::function_error);
    inline int * Vals(); //exceptions::function_error);
    inline void callback(); //exceptions::function_error);
};


class Plugin : public Machine{
private:
  static PluginFxCallback _callback;
  public:
      inline static PluginFxCallback * GetCallback()  { return &_callback; };
public:
    Plugin(int index, Song* song);

    virtual ~Plugin();

    virtual void Init();
    virtual int GenerateAudioInTicks( int startSample, int numSamples );
    virtual void Tick( );
    virtual void Tick(int channel, const PatternEvent & pEntry );
    virtual void Stop();
    inline virtual std::string GetDllName() const  { return _psDllName; }
    virtual std::string GetName() const { return _psName; };

    virtual int GetNumParams() { return GetInfo()->numParameters; };
    virtual int GetNumCols() { return GetInfo()->numCols; };
    virtual void GetParamName(int numparam, char * name);
    virtual void GetParamRange(int numparam,int &minval, int &maxval);
    virtual int GetParamValue(int numparam);
    virtual void GetParamValue(int numparam,char* parval);
    virtual bool SetParameter(int numparam,int value);

    inline Proxy & proxy()  { return proxy_; };

    bool Instance(const std::string & file_name);
    bool LoadDll (std::string psFileName);

///\name (de)serialization
			///\{
				public:
					/// Loader for psycle fileformat version 2.
					virtual bool LoadPsy2FileFormat(RiffFile* pFile);
					virtual bool LoadSpecificChunk(RiffFile * pFile, int version);
					virtual void SaveSpecificChunk(RiffFile * pFile);
					virtual void SaveDllName      (RiffFile * pFile);
			///\}




    inline CMachineInfo * GetInfo()  { return _pInfo; };

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


inline void Proxy::Init() 
{ assert((*this)()); plugin().Init(); }
inline CMachineInterface & Proxy::plugin()  { return *plugin_; }
inline void Proxy::SequencerTick()  { plugin().SequencerTick(); }
inline void Proxy::ParameterTweak(int par, int val) 
{ assert((*this)()); plugin().ParameterTweak(par, val);  }
inline Plugin & Proxy::host()  { return host_; }
inline const Plugin & Proxy::host() const  { return host_; }

inline void Proxy::callback() 
    { assert((*this)()); plugin().pCB = host().GetCallback(); }

inline const bool Proxy::operator()() const  { return !!plugin_; }
inline void Proxy::operator()(CMachineInterface * plugin) //exceptions::function_error)
{
  zapObject(this->plugin_,plugin);
    //if((*this)())
    if(plugin) {
        callback();
        //Init(); // [bohan] i can't call that here. It would be best, some other parts of psycle want to call it to. We need to get rid of the other calls.
    }
}

inline void Proxy::SeqTick(int channel, int note, int ins, int cmd, int val) 
{ assert((*this)()); plugin().SeqTick(channel, note, ins, cmd, val); }
inline void Proxy::StopWave() 
{ assert((*this)());plugin().StopWave(); }
inline void Proxy::Work(float * psamplesleft, float * psamplesright , int numsamples, int tracks) 
{ assert((*this)()); fflush(stdout); plugin().Work(psamplesleft, psamplesright, numsamples, tracks);  }
inline int * Proxy::Vals() 
{ assert((*this)()); return plugin().Vals; }
inline void Proxy::Stop() 
{ assert((*this)()); plugin().Stop();  }
inline bool Proxy::DescribeValue(char * txt, const int param, const int value) 
{ assert((*this)()); return plugin().DescribeValue(txt, param, value); }
inline void Proxy::PutData(void * pData) 
{ assert((*this)()); plugin().PutData(pData);  }
inline void Proxy::GetData(void * pData) 
{ assert((*this)()); plugin().GetData(pData); }
inline int Proxy::GetDataSize() 
{ assert((*this)()); return plugin().GetDataSize(); }

}
}
#endif
