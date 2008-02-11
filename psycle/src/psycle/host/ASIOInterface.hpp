/* -*- mode:c++, indent-tabs-mode:t -*- */
///\file
///\brief interface file for psycle::host::ASIOInterface.
#pragma once
#include "AudioDriver.hpp"
#include <asiodrivers.h>
#include <asio.h>
#include <map>
#pragma comment(lib, "asio")
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
			class PortEnum
			{
			public:
				PortEnum():_idx(0)
				{
					_info.channel=0;			_info.isInput=0;
					_info.isActive=0;			_info.channelGroup=0;
					_info.type=0;				memset(_info.name,0,sizeof(_info.name));
				}
				PortEnum(int idx,ASIOChannelInfo info):_idx(idx)
				{
					_info.channel=info.channel;		_info.isInput=info.isInput;
					_info.isActive=info.isActive;	_info.channelGroup=info.channelGroup;
					_info.type=info.type;			strcpy(_info.name,info.name);
				}
				std::string GetName();
			public:
				int _idx;
				ASIOChannelInfo _info;
			};
			class DriverEnum
			{
			public:
				DriverEnum():minSamples(2048),maxSamples(2048),prefSamples(2048),granularity(0){};
				DriverEnum(std::string name):_name(name){};
				~DriverEnum(){};
				void AddInPort(PortEnum &port) { _portin.push_back(port); }
				void AddOutPort(PortEnum &port) { _portout.push_back(port); }
			public:
				std::string _name;
				std::vector<PortEnum> _portout;
				std::vector<PortEnum> _portin;
				long minSamples;
				long maxSamples;
				long prefSamples;
				long granularity;
			};
			class PortOut
			{
			public:
				PortOut():driver(0),port(0){};
				DriverEnum* driver;
				PortEnum* port;
			};
			class PortCapt : public PortOut
			{
			public:
				PortCapt():PortOut(),pleft(0),pright(0),machinepos(0){};
				float *pleft;
				float *pright;
				int machinepos;
			};
			class AsioStereoBuffer
			{
			public:
				AsioStereoBuffer()
				{	pleft[0]=0;	pleft[1]=0;	pright[0]=0;	pright[1]=0;_sampletype=0;	}
				AsioStereoBuffer(void** left,void**right,ASIOSampleType stype)
				{
					pleft[0]=left[0];pleft[1]=left[1];pright[0]=right[0];pright[1]=right[1];
					_sampletype=stype;
				}
				ASIOSampleType _sampletype;
				AsioStereoBuffer operator=(AsioStereoBuffer& buf)
				{
					_sampletype=buf._sampletype;
					pleft[0]=buf.pleft[0];
					pleft[1]=buf.pleft[1];
					pright[0]=buf.pright[0];
					pright[1]=buf.pright[1];
					return *this;
				}
				void *pleft[2];
				void *pright[2];
			};

		public:
			ASIOInterface();
			virtual ~ASIOInterface() throw();
			virtual void Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void* context);
			virtual void Configure();
			inline virtual bool Initialized() { return _initialized; };
			inline virtual bool Configured() { return _configured; };
			virtual bool Enabled() { return _running; };
			virtual int GetBufferSize();
			virtual void GetCapturePorts(std::vector<std::string>&ports);
			virtual bool AddCapturePort(int idx);
			virtual bool RemoveCapturePort(int idx);
			virtual void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples);
			inline virtual AudioDriverInfo* GetInfo() { return &_info; };
			virtual void Reset();
			virtual bool Enable(bool e);
			virtual int GetWritePos();
			virtual int GetPlayPos();
			DriverEnum GetDriverFromidx(int driverID);
			PortOut GetOutPortFromidx(int driverID);
			int GetidxFromOutPort(PortOut&port);
			void ControlPanel(int driverID);
			virtual int GetInputLatency() { return _inlatency; }
			virtual int GetOutputLatency() { return _outlatency; }

			static void bufferSwitch(long index, ASIOBool processNow);
			static ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);
			static void sampleRateChanged(ASIOSampleRate sRate);
			static long asioMessages(long selector, long value, void* message, double* opt);

			std::vector<DriverEnum> _drivEnum;

			static int _ASIObufferSize;
			static AsioStereoBuffer *ASIObuffers;
			static bool _firstrun;
			static bool _supportsOutputReady;

		protected:
			void Error(const char msg[]);
			void ReadConfig();
			void WriteConfig();
			bool Start();
			bool Stop();

			static AUDIODRIVERWORKFN _pCallback;
			static void* _pCallbackContext;

			ASIOCallbacks asioCallbacks;



//			int drivercount;
//			char szFullName[MAX_ASIO_OUTPUTS][160];

		// callback prototypes
/*
			int driverindex[MAX_ASIO_OUTPUTS];
			int outputindex[MAX_ASIO_OUTPUTS];
			long minSamples[MAX_ASIO_DRIVERS];
			long maxSamples[MAX_ASIO_DRIVERS];
			long prefSamples[MAX_ASIO_DRIVERS];
			long Granularity[MAX_ASIO_DRIVERS];
			int currentSamples[MAX_ASIO_DRIVERS];
*/
		private:
			static AudioDriverInfo _info;
			static ::CCriticalSection _lock;
			bool _initialized;
			bool _configured;
			bool _running;
//			int _driverID;
			AsioDrivers asioDrivers;
			long _inlatency;
			long _outlatency;


			static PortOut _selectedout;
			static std::vector<PortCapt> _selectedins;
			std::vector<int> _portMapping;

		};
	}
}
