#pragma once
///\file
///\brief interface file for psycle::host::AudioDriver.
namespace psycle
{
	namespace host
	{
		typedef float* (*AUDIODRIVERWORKFN)(void* context, int& numSamples);

		class AudioDriverInfo
		{
		public:
			char const *_psName;
		};

		class AudioDriverEvent : public CEvent
		{
		public:
			AudioDriverEvent() : CEvent(FALSE, TRUE) {}
		};

		/// output device interface.
		class AudioDriver
		{
		public:
			AudioDriver() { _samplesPerSec = 44100; _bitDepth = 16; _channelmode = 3;}
			virtual ~AudioDriver() {};
			virtual void Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void* context) {};
			virtual void Reset(void) {};
			virtual bool Enable(bool e) { return false; };	
			virtual int GetWritePos() { return 0; };
			virtual int GetPlayPos() { return 0; };
			virtual int GetNumBuffers() { return 0; };
			virtual int GetBufferSize() { return 0; };
			virtual void Configure(void) {};
			virtual bool Initialized(void) { return true; };
			virtual bool Configured(void) { return true; };
			virtual AudioDriverInfo* GetInfo() { return &_info; };
			static void QuantizeWithDither(float *pin, int *piout, int c);
			static void Quantize(float *pin, int *piout, int c);
		public:
			int _samplesPerSec;
			int _channelmode;
			int _bitDepth;
		protected:
			static AudioDriverInfo _info;
		};
	}
}