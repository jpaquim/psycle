#pragma once
#if defined XPSYCLE__CONFIGURATION
	#include <xpsycle/microsoft_direct_sound_conditional_build.h>
#endif
#if !defined XPSYCLE__NO_MICROSOFT_DIRECT_SOUND
#include "audiodriver.h"
#include <dsound.h>
#if defined __unix__
	#include <pthread.h>
#endif
namespace psycle
	{
		namespace host
		{
			class MicrosoftDirectSoundOut : public AudioDriver
			{
			public:
				MicrosoftDirectSoundOut();
				virtual ~MicrosoftDirectSoundOut();
				virtual MicrosoftDirectSoundOut* clone()  const; // Uses the copy constructor

				virtual AudioDriverInfo info() const;

				void Initialize(AUDIODRIVERWORKFN pCallback, void * context );
				virtual bool Initialized() { return _initialized; };
				virtual bool Enable(bool);
				virtual void configure();

/*				virtual void Reset();
				virtual int GetWritePos();
				virtual int GetPlayPos();
				int virtual GetMaxLatencyInSamples() { return GetSampleSize() * _dsBufferSize; }
				virtual bool Configured() { return _configured; };
*/
			private:
				bool _initialized;
//				bool _configured;
				static AudioDriverInfo _info;
				bool _threadRunning;
				bool _playing;

//				HWND _hwnd;
//				MMRESULT _timer;
//				bool _timerActive;
//				static AudioDriverEvent _event;
//				CCriticalSection _lock;

				GUID device_guid;
				bool _exclusive;
				bool _dither;
				int _bufferSize;
				int _numBuffers;

				int _dsBufferSize;
				int _currentOffset;
				int _lowMark;
				int _highMark;
				int _buffersToDo;

				LPDIRECTSOUND _pDs;
				LPDIRECTSOUNDBUFFER _pBuffer;
				void* _callbackContext;
				AUDIODRIVERWORKFN _pCallback;
				pthread_t threadid;

				bool Start();
				bool Stop();
				static int audioOutThread(void* pDirectSound);
				void DoBlocks();
/*				void ReadConfig();
				void WriteConfig();
*/				void Error(const char msg[]);



			};
	}
}
#endif // !defined XPSYCLE__NO_MICROSOFT_DIRECT_SOUND
