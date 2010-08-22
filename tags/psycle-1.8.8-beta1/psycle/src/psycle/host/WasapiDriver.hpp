///\file
///\brief interface file for psycle::host::AudioDriver.
#pragma once
#include "Global.hpp"
#include "AudioDriver.hpp"
namespace psycle
{
	namespace host
	{
		/// output device interface.
		class WasapiDriver : public AudioDriver
		{
		public:
			WasapiDriver();
			virtual ~WasapiDriver();
			virtual void Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void* context);
			virtual void Reset(void);
			virtual bool Enable(bool e);
			virtual bool Enabled();
			virtual void GetCapturePorts(std::vector<std::string> &ports);
			virtual void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples);
			virtual bool AddCapturePort(int idx);
			virtual bool RemoveCapturePort(int idx);
			virtual int GetWritePos();
			virtual int GetPlayPos();
			virtual int GetInputLatency();
			virtual int GetOutputLatency();
			virtual void Configure(void);
			virtual bool Initialized(void);
			virtual bool Configured(void);
			virtual void RefreshAvailablePorts();
			virtual AudioDriverInfo* GetInfo() { return &_info; }
		private:
			static AudioDriverInfo _info;
		};
	}
}