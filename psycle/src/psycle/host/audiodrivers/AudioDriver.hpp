///\interface psycle::host::AudioDriver
#pragma once
#include <psycle/host/detail/project.hpp>

#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(push)
	#endif
	#include <afxmt.h> // because of CEvent
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(pop)
	#endif
#else
	#error "sorry"
#endif

namespace psycle
{
	namespace host
	{
		class AudioDriverInfo
		{
			public:
				char const * _psName;
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
				typedef float * (*WorkFunction) (void * context, int & numSamples);

			public:
				AudioDriver();
				virtual ~AudioDriver() {};
				virtual void Initialize(HWND hwnd, WorkFunction pCallback, void* context) {};
				virtual void Reset(void) {};
				virtual bool Enable(bool e) { return false; };	
				virtual int GetWritePos() { return 0; };
				virtual int GetPlayPos() { return 0; };
				virtual int GetNumBuffers() { return _numBlocks; };
				virtual int GetBufferSize() { return _blockSize; };
				virtual int GetBufferSamples() { return _blockSize/GetSampleSize(); };
				virtual int GetSampleSize() { return (_channelmode==3)?(_bitDepth/4):(_bitDepth/8); };
				virtual void Configure(void) {};
				virtual bool Initialized(void) { return true; };
				virtual bool Configured(void) { return true; };
				virtual AudioDriverInfo* GetInfo() { return &_info; };
				static void QuantizeWithDither(float *pin, int *piout, int c);
				static void Quantize(float *pin, int *piout, int c);
			public:
				int _numBlocks;
				int _blockSize;
				int _samplesPerSec;
				int _channelmode;
				int _bitDepth;
			protected:
				static AudioDriverInfo _info;
		};
	}
}
