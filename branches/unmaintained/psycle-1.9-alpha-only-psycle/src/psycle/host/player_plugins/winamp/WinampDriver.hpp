///\file
///\brief interface file for psycle::host::WinampDriver.
#pragma once
#include "../../AudioDriver.hpp"
namespace psycle
{
	namespace host
	{
		class Player;
		typedef float* (*AUDIODRIVERWORKFN)(void* context, int numSamples);

		/// output device interface.
		class WinampDriver : public AudioDriver
		{
		public:
			WinampDriver();
			virtual ~WinampDriver() {};
			virtual void Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void* context);
			virtual void Reset(void) {};
			virtual bool Enable(bool e);
			virtual bool Enabled() { return false; };
			virtual void Pause(bool e);
			virtual bool Paused() { return paused; }
			virtual void Configure(void) {};
			virtual bool Initialized(void) { return true; };
			virtual bool Configured(void) { return true; };
			virtual int GetOutputLatency() { return outputlatency; }
			static void Quantize(float *pin, short *piout, int c);
		protected:
			void* context;
			static DWORD WINAPI __stdcall PlayThread(void *b);

			static AudioDriverInfo _info;
			
			int outputlatency;
			int killDecodeThread;
			HANDLE thread_handle;
			bool initialized;
			int paused;
			int worked;

		};
	}
}