#pragma once
#include <psycle/host/AudioDriver.h>
#include <asio/asiodrivers.h>
#include <asio/asio.h>
#pragma comment(lib, "asio")
///\file
///\brief interface file for psycle::host::ASIOInterface.
namespace psycle
{
	namespace host
	{
		#define MAX_ASIO_DRIVERS 32
		#define MAX_ASIO_OUTPUTS 128

		/// output device interface implemented by asio.
		class ASIOInterface : public AudioDriver
		{
		public:
			ASIOInterface();
			virtual ~ASIOInterface() throw();
			virtual void Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void* context);
			virtual void Configure();
			inline virtual bool Initialized() { return _initialized; };
			inline virtual bool Configured() { return _configured; };
			virtual int GetBufferSize();
			inline virtual AudioDriverInfo* GetInfo() { return &_info; };
			virtual void Reset();
			virtual bool Enable(bool e);
			virtual int GetWritePos();
			virtual int GetPlayPos();
			void ControlPanel(int driverID);

			int drivercount;
			char szFullName[MAX_ASIO_OUTPUTS][160];

		// callback prototypes
			int driverindex[MAX_ASIO_OUTPUTS];
			int outputindex[MAX_ASIO_OUTPUTS];
			long minSamples[MAX_ASIO_DRIVERS];
			long maxSamples[MAX_ASIO_DRIVERS];
			long prefSamples[MAX_ASIO_DRIVERS];
			long Granularity[MAX_ASIO_DRIVERS];
			int currentSamples[MAX_ASIO_DRIVERS];

		private:
			void Error(const char msg[]);
			void ReadConfig();
			void WriteConfig();
			bool Start();
			bool Stop();
			static AudioDriverInfo _info;
			static ::CCriticalSection _lock;
			bool _initialized;
			bool _configured;
			bool _running;
			int _driverID;
			char szNameBuf[MAX_ASIO_DRIVERS][64];
			asio::AsioDrivers asioDrivers;
		};
	}
}