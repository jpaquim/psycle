///\file
///\brief interface file for psycle::host::AudioDriver.
#pragma once
#include "Global.hpp"
#include <MMReg.h>

namespace psycle
{
	namespace host
	{
		typedef float* (*AUDIODRIVERWORKFN)(void* context, int numSamples);
		typedef enum {
			no_mode = -1,
			mono_mix = 0,
			mono_left,
			mono_right,
			stereo
		} channel_mode;
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
			virtual bool Enable(bool e) { return false; }
			virtual bool Enabled() { return false; }
			virtual void GetPlaybackPorts(std::vector<std::string> &ports) { ports.resize(0); }
			virtual void GetCapturePorts(std::vector<std::string> &ports) { ports.resize(0); }
			virtual void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples) { pleft=0; pright=0; return; }
			virtual bool AddCapturePort(int idx){ return false; };
			virtual bool RemoveCapturePort(int idx){ return false; }
			virtual std::uint32_t GetWritePosInSamples() { return 0; }
			virtual std::uint32_t GetPlayPosInSamples() { return 0; }
			virtual int GetNumBuffers() { return _numBlocks; }
			//size of each buffer
			virtual int GetBufferBytes() { return _blockSizeBytes; }
			//size of each buffer, for a whole sample (counting all channels)
			virtual int GetBufferSamples() { return _blockSizeBytes/GetSampleSizeBytes(); }
			//Size of a whole sample (counting all channels)
			virtual int GetSampleSizeBytes() { return (_channelMode==stereo)?(_sampleBits/4):(_sampleBits/8); }
			//Size of a mono sample in bits. If validBits=32 floats are assumed!
			virtual int GetSampleBits() { return _sampleBits; }
			//Amount of bits valid inside a mono sample. (left aligned. i.e. lower bits unused). If validBits=32 floats are assumed!
			virtual int GetSampleValidBits() { return _sampleValidBits; }
			virtual int GetSamplesPerSec() { return _samplesPerSec; }
			virtual channel_mode GetChannelMode() { return _channelMode; }
			virtual std::uint32_t GetInputLatencyMs() { return GetInputLatencySamples()*0.001f / _samplesPerSec; }
			virtual std::uint32_t GetInputLatencySamples() { return 0; }
			virtual std::uint32_t GetOutputLatencyMs() { return GetOutputLatencySamples()*0.001f / _samplesPerSec; }
			virtual std::uint32_t GetOutputLatencySamples() { return 0; }
			virtual void Configure(void) {};
			virtual bool Initialized(void) { return true; }
			virtual bool Configured(void) { return true; }
			virtual void RefreshAvailablePorts() {}
			virtual AudioDriverInfo* GetInfo() =0;
			static void PrepareWaveFormat(WAVEFORMATEXTENSIBLE& wf, int channels, int sampleRate, int bits, int validBits);
			static void Quantize16(float *pin, int *piout, int c);
			static void Quantize16WithDither(float *pin, int *piout, int c);
			static void Quantize24in32Bit(float *pin, int *piout, int c);
			static void DeQuantize16AndDeinterlace(short int *pin, float *poutleft,float *poutright,int c);
			static void DeQuantize32AndDeinterlace(int *pin, float *poutleft,float *poutright,int c);
			static void DeinterlaceFloat(float *pin, float *poutleft,float *poutright,int c);
		protected:
			int _numBlocks;
			int _blockSizeBytes;
			int _samplesPerSec;
			int _sampleBits; // for each mono sample. (i.e. 24bits packed inside a 32bit integer) If validBits=32 floats are assumed!
			int _sampleValidBits; // for each mono sample. (i.e. 24bits packed inside a 32bit integer) If validBits=32 floats are assumed!
			channel_mode _channelMode;
		};

		class SilentDriver: public AudioDriver
		{
		public:
			virtual AudioDriverInfo* GetInfo() { return &_info; }
		protected:
			static AudioDriverInfo _info;
		};

	}
}