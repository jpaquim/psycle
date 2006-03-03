///\file
///\brief interface file for psycle::host::ASIOInterface.
#pragma once
#include "AudioDriver.hpp"
#include <asio/asiodrivers.h>
#include <asio/asio.h>
#pragma comment(lib, "asio")
namespace psycle
{
	namespace host
	{

		/// output device interface implemented by asio.
		class ASIOInterface : public AudioDriver
		{
		public:
			int const static MAX_ASIO_DRIVERS = 32;
			int const static MAX_ASIO_OUTPUTS = 128;

		public:
			ASIOInterface();
			virtual ~ASIOInterface() throw();
			virtual void Initialize(HWND hwnd, WorkFunction pCallback, void* context);
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
