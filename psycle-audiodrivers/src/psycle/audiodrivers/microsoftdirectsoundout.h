/******************************************************************************
*  copyright 2007 members of the psycle project http://psycle.sourceforge.net *
*                                                                             *
*  This program is free software; you can redistribute it and/or modify       *
*  it under the terms of the GNU General Public License as published by       *
*  the Free Software Foundation; either version 2 of the License, or          *
*  (at your option) any later version.                                        *
*                                                                             *
*  This program is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
*  GNU General Public License for more details.                               *
*                                                                             *
*  You should have received a copy of the GNU General Public License          *
*  along with this program; if not, write to the                              *
*  Free Software Foundation, Inc.,                                            *
*  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                  *
******************************************************************************/
#pragma once

#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
#include "audiodriver.h"
#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>
#include <map>

namespace psy { namespace core {

/// output device interface implemented by direct sound.
///\todo this driver is (over)filling the buffer to fast!
class MsDirectSound : public AudioDriver {
	class PortEnums {
	public:
		PortEnums():guid() {};
		PortEnums(LPGUID _guid,std::string _pname):guid(_guid),portname(_pname){}
		std::string portname;
		LPGUID guid;
	};
	class PortCapt {
	public:
		PortCapt():pleft(0),pright(0),_pGuid(0),_pDs(0),_pBuffer(0),_lowMark(0),_machinepos(0) {};

		LPGUID _pGuid;
		LPDIRECTSOUNDCAPTURE8 _pDs;
		LPDIRECTSOUNDCAPTUREBUFFER8  _pBuffer;
		int _lowMark;
		float *pleft;
		float *pright;
		int _machinepos;
	};
	public:

		MsDirectSound();
		virtual ~MsDirectSound();

		AudioDriverInfo info( ) const;

		virtual void Initialize( AUDIODRIVERWORKFN pCallback, void * context );
		virtual bool Enable( bool );
		virtual bool Enabled() { return _running; }
		virtual void GetCapturePorts(std::vector<std::string>&ports);
		virtual bool AddCapturePort(int idx);
		virtual bool RemoveCapturePort(int idx);
		virtual bool CreateCapturePort(PortCapt &port);
		virtual void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples);
		static BOOL CALLBACK DSEnumCallback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);
		virtual int GetInputLatency() { return _dsBufferSize; }
		virtual int GetOutputLatency() { return _dsBufferSize; }
		virtual int GetWritePos();
		virtual int GetPlayPos();
//		int virtual GetMaxLatencyInSamples() { return settings().sampleSize() * _dsBufferSize; }
		virtual void Configure();
		virtual bool Initialized() { return _initialized; }
		virtual bool Configured() { return _configured; }
	protected:
		void ReadConfig();
		void WriteConfig();
		void Error(const WCHAR msg[]);
		static DWORD WINAPI PollerThread(void* pDirectSound);
		//static void TimerCallback(UINT uTimerID, UINT uMsg, DWORD pDirectSound, DWORD dw1, DWORD dw2);
		void DoBlocks();
		bool WantsMoreBlocks();
		void DoBlocksRecording(PortCapt& port);
		bool WantsMoreBlocksRecording(PortCapt& port);
		bool Start();
		bool Stop();

	private:
		bool _initialized;
		bool _configured;
		static AudioDriverInfo _info;

		bool _running;
		bool _playing;
		bool _threadRun;
		//static AudioDriverEvent _event;
		//CCriticalSection _lock;

		GUID device_guid;
		bool _exclusive;
		bool _dither;
		int _bytesPerSample;
		int _bufferSize;
		int _numBuffers;

		int _dsBufferSize;
		int _runningBufSize;
		int _lowMark;
		int _highMark;
		int _buffersToDo;

		std::vector<PortEnums> _capEnums;
		std::vector<PortCapt> _capPorts;
		std::vector<int> _portMapping;
		LPDIRECTSOUND8 _pDs;
		LPDIRECTSOUNDBUFFER8 _pBuffer;
		void* _callbackContext;
		AUDIODRIVERWORKFN _pCallback;


		void quantize(float *pin, int *piout, int c);
};

}}
#endif
