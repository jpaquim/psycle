///\interface psycle::host::AudioDriver
#pragma once
#include <psycle/engine/detail/project.hpp>

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
				AudioDriverEvent() : CEvent(false, true) {}
		};

		/// output device interface.
		class AudioDriver
		{
			public:
				typedef float * (*WorkFunction) (void * context, int & numSamples);

			public:
				virtual AudioDriverInfo* GetInfo() { return &_info; }
				AudioDriver();
				virtual ~AudioDriver() throw() {}
				virtual void Initialize(HWND, WorkFunction, void*)= 0;
				virtual void Reset() = 0;
				virtual bool Enable(bool) = 0;
				virtual int GetWritePos() = 0;
				virtual int GetPlayPos() = 0;
				int virtual GetMaxLatencyInSamples() = 0;
				virtual void Configure() = 0;
				virtual bool Initialized() = 0;
				virtual bool Configured() = 0;
				static void QuantizeWithDither(float *pin, int *piout, int c);
				static void Quantize(float *pin, int *piout, int c);
				int GetSampleSize() { return _channelmode == 3 ? _bitDepth / 4 : _bitDepth / 8; }
			PSYCLE__PRIVATE:
				int _blockSize;
				int _samplesPerSec;
				int _bitDepth;
				int _channelmode;
			protected: // whuo!?
				/**/static/**/ AudioDriverInfo _info;
		};

		class NullOutput : public AudioDriver
		{
			public:
				~NullOutput() throw() {}
				virtual void Initialize(HWND hwnd, WorkFunction, void*) {}
				virtual void Reset() {}
				virtual bool Enable(bool) { return true; }
				virtual int GetWritePos() { return 0; }
				virtual int GetPlayPos() { return 0; }
				int virtual GetMaxLatencyInSamples() { return 0; }
				virtual void Configure() {}
				virtual bool Initialized() { return true; }
				virtual bool Configured() { return true; }
		};
	}
}
