///\file
///\brief interface file for psycle::host::AudioDriver.
#pragma once
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
			AudioDriver();
			virtual ~AudioDriver() {};
			virtual void Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void* context) {};
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