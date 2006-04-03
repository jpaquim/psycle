/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#ifndef SAMPLER_H
#define SAMPLER_H

#include "filter.h"
#include "machine.h"
#include "dsp.h"
#include "deserializer.h"

/**
@author Stefan Nattkemper
*/

#define SAMPLER_MAX_POLYPHONY           16
#define SAMPLER_DEFAULT_POLYPHONY       8

#define SAMPLER_CMD_NONE                        0x00
#define SAMPLER_CMD_PORTAUP                     0x01
#define SAMPLER_CMD_PORTADOWN           0x02
#define SAMPLER_CMD_PORTA2NOTE          0x03
#define SAMPLER_CMD_PANNING                     0x08
#define SAMPLER_CMD_OFFSET                      0x09
#define SAMPLER_CMD_VOLUME                      0x0c
#define SAMPLER_CMD_RETRIG                      0x15
#define SAMPLER_CMD_EXTENDED            0x0e
#define SAMPLER_CMD_EXT_NOTEOFF         0xc0
#define SAMPLER_CMD_EXT_NOTEDELAY       0xd0

// ms typedefs

typedef unsigned long ULONG;
typedef long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef unsigned long DWORD;
typedef ULONG* PULONG;
typedef PULONG ULONG_PTR;
typedef long long int __int64;
typedef long long unsigned int __uint64;


typedef union _LARGE_INTEGER {
        LONGLONG QuadPart;
} LARGE_INTEGER;

typedef union _ULARGE_INTEGER {
  struct {
    DWORD LowPart;
    DWORD HighPart;
  };
  struct {
    DWORD LowPart;
    DWORD HighPart;
  } u;
  ULONGLONG QuadPart;
} ULARGE_INTEGER, 
*PULARGE_INTEGER;

typedef enum {
    ENV_OFF = 0,
    ENV_ATTACK = 1,
    ENV_DECAY = 2,
    ENV_SUSTAIN = 3,
    ENV_RELEASE = 4,
    ENV_FASTRELEASE = 5
} EnvelopeStage;


class WaveData {
   public:
      short* _pL;
      short* _pR;
      bool _stereo;
      ULARGE_INTEGER _pos;
      __int64 _speed;  /// todo not sure ...
      bool _loop;
      ULONG _loopStart;
      ULONG _loopEnd;
      ULONG _length;
      float _vol;
      float _lVolDest;
      float _rVolDest;
      float _lVolCurr;
      float _rVolCurr;
};


class Envelope
{
  public:
    EnvelopeStage _stage;
    float _value;
    float _step;
    float _attack;
    float _decay;
    float _sustain;
    float _release;
};

class Voice
{
  public:
    Envelope _filterEnv;
    Envelope _envelope;
    int _sampleCounter;
    int _triggerNoteOff;
    int _triggerNoteDelay;
    int _instrument;
    WaveData _wave;
    Filter _filter;
    int _cutoff;
    float _coModify;
    int _channel;
    int effVal;
    //int effPortaNote;
    int effCmd;
    int effretMode;
    int effretTicks;
    float effretVol;
    int effOld;
};

class Sampler : public Machine {
public:
    Sampler();
    Sampler(int index);

    ~Sampler();

    void Tick();
    virtual void Init(void);
    virtual void Stop(void);
    virtual void Work(int numSamples);
    virtual void Tick(int channel, PatternEntry* pData);
    virtual char* GetName(void) { return _psName; };
    virtual bool Load(DeSerializer* pFile);

    inline virtual bool LoadSpecificChunk(DeSerializer* pFile, int version)
   {
     int size =  pFile->getInt();
     if (size)
     {
        if (version > CURRENT_FILE_VERSION_MACD)
        {
           // data is from a newer format of psycle, it might be unsafe to load.
           pFile->skip(size);
           return false;
        }
        else {
           int temp;
           temp = pFile->getInt();
           _numVoices=temp;
           temp = pFile->getInt();

           switch (temp)
           {
             case 2:
               _resampler.SetQuality(dsp::R_SPLINE);
             break;
             case 0:
               _resampler.SetQuality(dsp::R_NONE);
             break;
             default:
             case 1:
                _resampler.SetQuality(dsp::R_LINEAR);
             break;
           }
         }
       }
       return true;
};


protected:

    static char* _psName;
    int _numVoices;
    Voice _voices[SAMPLER_MAX_POLYPHONY];
    dsp::Cubic _resampler;

    void PerformFx(int voice);
    void VoiceWork(int numsamples, int voice);
    void NoteOff(int voice);
    void NoteOffFast(int voice);
    int VoiceTick(int channel, PatternEntry* pData);

    inline void TickEnvelope(int voice);
    inline void TickFilterEnvelope(int voice);

    unsigned char lastInstrument[MAX_TRACKS];
    static inline int alteRand(int x)
    {
       return (x*rand())/32768;
    };

};

#endif
