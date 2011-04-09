///\file
///\brief interface file for psycle::host::AudioDriver.
#pragma once
#include "Global.hpp"
#include <MMReg.h>

namespace psycle
{
	namespace host
	{
		class ConfigStorage;
		class AudioDriver;

		class AudioDriverEvent : public CEvent
		{
		public:
			AudioDriverEvent() : CEvent(FALSE, TRUE) {}
		};

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


	/// holds the info about sample rate, bit depth, etc
	class AudioDriverSettings {
	public:
		AudioDriverSettings();
		AudioDriverSettings(const AudioDriverSettings& othersettings);
		AudioDriverSettings& operator=(const AudioDriverSettings& othersettings);
		virtual bool operator!=(AudioDriverSettings const &);
		virtual bool operator==(AudioDriverSettings const & other) { return !((*this) != other); }

		virtual void SetDefaultSettings(bool include_others=true);
		virtual void Load(ConfigStorage &) = 0;
		virtual void Save(ConfigStorage &) = 0;

		virtual AudioDriverInfo& GetInfo() = 0;
		virtual AudioDriver* NewDriver() = 0;

	///\name getter/setter for sample rate
	///\{
		public:
			inline unsigned int samplesPerSec() const { return samplesPerSec_; }
			void setSamplesPerSec(unsigned int value) { samplesPerSec_ = value; }
		private:
			unsigned int samplesPerSec_;
	///\}

	///\name getter/setter for sample bit depth, per-channel.. bitDepth is the container size. validBitDepth is the actual number of bits used.
	/// Valid values:  8/8, 16/16 , 32/24 (int) and 32/32 (float)
	///\{
		public:
			inline bool dither() const { return dither_; }
			inline unsigned int validBitDepth() const { return validBitDepth_; }
			inline unsigned int bitDepth() const { return bitDepth_; }
			/// getter for number of bytes per sample (counting all channels).
			inline unsigned int frameBytes() const { return frameBytes_; }
			void setValidBitDepth(unsigned int value) {
				validBitDepth_ = value;
				bitDepth_ = (value == 24) ? 32 : value;
				frameBytes_ = (channelMode_ == stereo) ? (bitDepth_ >> 2) : (bitDepth_ >> 3);
			}
			void setDither(bool dither) { dither_ = dither; }
		private:
			bool dither_;
			unsigned int validBitDepth_;
			unsigned int bitDepth_;
			unsigned int frameBytes_;
	///\}

	///\name getter/setter for channel mode
	///\{
		public:
			inline unsigned int numChannels() const { return (channelMode_ == stereo) ? 2 : 1; }
			inline channel_mode channelMode() const { return channelMode_; }
			void setChannelMode(channel_mode value) {
				channelMode_ = value;
				frameBytes_ = (channelMode_ == stereo) ? (bitDepth_ >> 2) : (bitDepth_ >> 3);
			}
		private:
			channel_mode channelMode_;
	///\}

	///\name getter/setter for the audio block size (in samples comprising all channels)
	///\{
		public:
			inline unsigned int blockFrames() const { return blockFrames_; }
			void setBlockFrames(unsigned int value) {
				blockFrames_ = value;
				assert(blockFrames_ < MAX_SAMPLES_WORKFN);
			}
			///\name getter/setter for the audio block size (in bytes)
			inline unsigned int blockBytes() const { return blockFrames_ * frameBytes_; }
			void setBlockBytes(unsigned int value) { setBlockFrames(value / frameBytes_);	}
			/// getter for the whole buffer size (in bytes).
			inline unsigned int totalBufferBytes() const { return blockFrames_ * frameBytes_ * blockCount_; }
		private:
			unsigned int blockFrames_;
	///\}

	///\name getter/setter for number of blocks.
	///\{
		public:
			inline unsigned int blockCount() const { return blockCount_; }
			void setBlockCount(unsigned int value) { blockCount_ = value; }
		private:
			unsigned int blockCount_;
	///\}
	};

		/// output device interface.
		class AudioDriver
		{
		protected:
			AudioDriver(){}
		public:
			virtual ~AudioDriver() {};

			virtual AudioDriverSettings& settings() = 0;

			//Sets the callback and initializes the info (like which ports there are)
			virtual void Initialize(AUDIODRIVERWORKFN pCallback, void* context) {};
			//Frees all resources related to this audio driver.
			virtual void Reset(void) {};
			//Enables or disables the playback/recording with this driver.
			virtual bool Enable(bool e) { return false; }
			//Initalize has been called.
			virtual bool Initialized(void) { return true; }
			//Is enabled?
			virtual bool Enabled() { return false; }
			//Launches the configuration dialog
			virtual void Configure(void) {};
			//Refreshes the playback and capture port lists.
			virtual void RefreshAvailablePorts() {}
			virtual void GetPlaybackPorts(std::vector<std::string> &ports) { ports.resize(0); }
			virtual void GetCapturePorts(std::vector<std::string> &ports) { ports.resize(0); }
			virtual void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples) { pleft=0; pright=0; return; }
			virtual bool AddCapturePort(int idx){ return false; };
			virtual bool RemoveCapturePort(int idx){ return false; }
			virtual std::uint32_t GetWritePosInSamples() { return 0; }
			virtual std::uint32_t GetPlayPosInSamples() { return 0; }
			virtual std::uint32_t GetInputLatencyMs() { return GetInputLatencySamples()*0.001f / settings().samplesPerSec(); }
			virtual std::uint32_t GetInputLatencySamples() { return 0; }
			virtual std::uint32_t GetOutputLatencyMs() { return GetOutputLatencySamples()*0.001f / settings().samplesPerSec(); }
			virtual std::uint32_t GetOutputLatencySamples() { return 0; }

			//amount of buffers.
			int GetNumBuffers() { return settings().blockCount(); }
			//size of each buffer
			int GetBufferBytes() { return settings().blockBytes(); }
			//size of each buffer, for a whole sample (counting all channels)
			int GetBufferSamples()  { return settings().blockFrames(); }
			//Size of a whole sample (counting all channels)
			int GetSampleSizeBytes()  { return settings().frameBytes(); }
			//Size of a mono sample in bits. If validBits=32 floats are assumed!
			int GetSampleBits() { return settings().bitDepth(); }
			//Amount of bits valid inside a mono sample. (left aligned. i.e. lower bits unused). If validBits=32 floats are assumed!
			int GetSampleValidBits() { return settings().validBitDepth(); }
			int GetSamplesPerSec() { return settings().samplesPerSec(); }
			channel_mode GetChannelMode() { return settings().channelMode(); }

			static void PrepareWaveFormat(WAVEFORMATEXTENSIBLE& wf, int channels, int sampleRate, int bits, int validBits);
			static void Quantize16(float *pin, int *piout, int c);
			static void Quantize16WithDither(float *pin, int *piout, int c);
			static void Quantize24in32Bit(float *pin, int *piout, int c);
			static void DeQuantize16AndDeinterlace(short int *pin, float *poutleft,float *poutright,int c);
			static void DeQuantize32AndDeinterlace(int *pin, float *poutleft,float *poutright,int c);
			static void DeinterlaceFloat(float *pin, float *poutleft,float *poutright,int c);
		};

		class SilentSettings : public AudioDriverSettings
		{
		public:
			SilentSettings(){ SetDefaultSettings(); };
			SilentSettings(const SilentSettings& othersettings):AudioDriverSettings(othersettings){};
			virtual AudioDriver* NewDriver();
			virtual AudioDriverInfo& GetInfo() { return info_; };

			virtual void Load(ConfigStorage &) {};
			virtual void Save(ConfigStorage &) {};
		private:
			static AudioDriverInfo info_;
		};

		class SilentDriver: public AudioDriver
		{
		public:
			SilentDriver(SilentSettings* settings):settings_(settings) {}
			virtual AudioDriverSettings& settings() { return *settings_; };
		protected:
			SilentSettings* settings_;
		};

	}
}