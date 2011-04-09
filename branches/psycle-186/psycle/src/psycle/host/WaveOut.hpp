///\file
///\brief interface file for psycle::host::WaveOut.
#pragma once
#include "Global.hpp"
#include "AudioDriver.hpp"
#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(push)
	#pragma warning(disable:4201) // nonstandard extension used : nameless struct/union
#endif

#include <mmsystem.h>

#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "winmm")
#endif

#if defined DIVERSALIS__COMPILER__MICROSOFT
	#pragma warning(pop)
#endif

#include <map>
namespace psycle
{
	namespace host
	{
		#define MAX_WAVEOUT_BLOCKS 8

		class WaveOutSettings : public AudioDriverSettings
		{
		public:
			WaveOutSettings();
			WaveOutSettings(const WaveOutSettings& othersettings);
			WaveOutSettings& operator=(const WaveOutSettings& othersettings);
			virtual bool operator!=(WaveOutSettings const &);
			virtual bool operator==(WaveOutSettings const & other) { return !((*this) != other); }
			virtual AudioDriver* NewDriver();
			virtual AudioDriverInfo& GetInfo() { return info_; }

			virtual void SetDefaultSettings();
			virtual void Load(ConfigStorage &);
			virtual void Save(ConfigStorage &);

			unsigned int deviceID_;
			unsigned int pollSleep_;
		private:
			static AudioDriverInfo info_;
		};

		/// output device interface implemented by mme.
		class WaveOut : public AudioDriver
		{
		public:
			class CBlock
			{
			public:
				HANDLE Handle;
				byte *pData;
				WAVEHDR *pHeader;
				HANDLE HeaderHandle;
				bool Prepared;
			};
			class PortEnums
			{
			public:
				PortEnums() {};
				PortEnums(std::string _pname, int _idx):portname(_pname),idx(_idx){}
				bool IsFormatSupported(WAVEFORMATEXTENSIBLE& pwfx, bool isInput);
				std::string portname;
				int idx;
			};
			class PortCapt
			{
			public:
				PortCapt():pleft(0),pright(0),_handle(0),_idx(0),_machinepos(0) {};

				HWAVEIN _handle;
				int _idx;
				byte *pData;
				WAVEHDR *pHeader;
				HANDLE HeaderHandle;
				CBlock _blocks[MAX_WAVEOUT_BLOCKS];
				bool Prepared;
				float *pleft;
				float *pright;
				int _machinepos;
			};

		public:
			WaveOut(WaveOutSettings* settings);
			virtual ~WaveOut() throw();
			virtual AudioDriverSettings& settings() { return *settings_; }

			virtual void Initialize(AUDIODRIVERWORKFN pCallback, void * context);
			virtual bool Enable(bool e);
			virtual void Reset();
			virtual bool Initialized() { return _initialized; }
			virtual bool Enabled() { return _running; }
			virtual void Configure();
			virtual void RefreshAvailablePorts();
			virtual void GetPlaybackPorts(std::vector<std::string> &ports);
			virtual void GetCapturePorts(std::vector<std::string> &ports);
			virtual bool AddCapturePort(int idx);
			virtual bool RemoveCapturePort(int idx);
			virtual bool CreateCapturePort(PortCapt &port);
			virtual void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples);

			virtual std::uint32_t GetInputLatencySamples() { return settings().blockCount() * settings().blockFrames(); }
			virtual std::uint32_t GetOutputLatencySamples() { return settings().blockCount() * settings().blockFrames(); }
			virtual std::uint32_t GetWritePosInSamples();
			virtual std::uint32_t GetPlayPosInSamples();
			MMRESULT IsFormatSupported(LPWAVEFORMATEX pwfx, UINT uDeviceID);
			static void PollerThread(void *pWaveOut);
		protected:
			void EnumeratePlaybackPorts();
			void EnumerateCapturePorts();
			void ReadConfig();
			void WriteConfig();
			void Error(const char msg[]);
			void DoBlocks();
			bool WantsMoreBlocks();
			void DoBlocksRecording(PortCapt& port);
			bool Start();
			bool Stop();

		private:
			bool _initialized;
			WaveOutSettings* settings_;
			static AudioDriverEvent _event;
//			static CCriticalSection _lock;

			HWAVEOUT _handle;
			int _currentBlock;
			std::uint32_t _writePos;
			/// number of "wraparounds" to compensate the WaveOutGetPosition() call.
			int m_readPosWraps;
			/// helper variable to detect the previous wraps.
			int m_lastPlayPos;

			bool _running;
			bool _stopPolling;
			CBlock _blocks[MAX_WAVEOUT_BLOCKS];
			void* _callbackContext;
			AUDIODRIVERWORKFN _pCallback;

			std::vector<PortEnums> _playEnums;
			std::vector<PortEnums> _capEnums;
			std::vector<PortCapt> _capPorts;
			std::vector<int> _portMapping;

		};
	}
}
