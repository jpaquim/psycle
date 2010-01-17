// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net

#ifndef PSYCLE__AUDIODRIVERS__ASIO_INTERFACE__INCLUDED
#define PSYCLE__AUDIODRIVERS__ASIO_INTERFACE__INCLUDED
#pragma once

#include "audiodriver.h"
#include <universalis/stdlib/condition.hpp>
#include <asiodrivers.h>
#include <asio.h>
#include <map>
#include <windows.h>
#include <diversalis/compiler.hpp>
#if defined DIVERSALIS__COMPILER__FEATURE__AUTO_LINK
	#pragma comment(lib, "asio")
#endif

namespace psycle { namespace audiodrivers {

unsigned int const MAX_ASIO_DRIVERS = 32;
unsigned int const MAX_ASIO_OUTPUTS = 128;

class AsioUiInterface {
	public:
		AsioUiInterface::AsioUiInterface() {}
		virtual ~AsioUiInterface() {}

		virtual int DoModal(class ASIOInterface* asio) = 0;
		virtual void SetValues(int device_id, int sample_rate, int buffer_size) = 0;
		virtual void GetValues(int& device_id, int& sample_rate, int& buffer_size) = 0;
		virtual void WriteConfig(int device_id_guid, int sample_rate, int buffer_size) = 0;
		virtual void ReadConfig(int& device_id, int& sample_rate, int& buffer_size) = 0;
		virtual void Error(std::string const & msg) = 0;
};

/// output device interface implemented by asio.
class ASIOInterface : public AudioDriver {
	public:
		class PortEnum
		{
			public:
				PortEnum()
				:
					_idx(0)
				{
					_info.channel = 0;
					_info.isInput = 0;
					_info.isActive = 0;
					_info.channelGroup = 0;
					_info.type = 0;
					std::memset(_info.name, 0, sizeof _info.name);
				}

				PortEnum(int idx,ASIOChannelInfo info)
				:
					_idx(idx)
				{
					_info.channel = info.channel;
					_info.isInput = info.isInput;
					_info.isActive = info.isActive;
					_info.channelGroup = info.channelGroup;
					_info.type = info.type;
					std::strcpy(_info.name, info.name);
				}
				
				std::string GetName();
			public:
				int _idx;
				ASIOChannelInfo _info;
		};

		class DriverEnum {
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

		class PortOut {
			public:
				PortOut():driver(0),port(0){};
				DriverEnum* driver;
				PortEnum* port;
		};

		class PortCapt : public PortOut {
			public:
				PortCapt():PortOut(),pleft(0),pright(0),machinepos(0){};
				float *pleft;
				float *pright;
				int machinepos;
		};

		class AsioStereoBuffer {
			public:
				AsioStereoBuffer() {
					pleft[0] = 0;
					pleft[1] = 0;
					pright[0] = 0;
					pright[1] = 0;
					_sampletype = 0;
				}
				
				AsioStereoBuffer(void** left,void**right,ASIOSampleType stype) {
					pleft[0] = left[0];
					pleft[1] = left[1];
					pright[0] = right[0];
					pright[1] = right[1];
					_sampletype=stype;
				}
				
				ASIOSampleType _sampletype;
				
				AsioStereoBuffer operator=(AsioStereoBuffer& buf) {
					_sampletype = buf._sampletype;
					pleft[0] = buf.pleft[0];
					pleft[1] = buf.pleft[1];
					pright[0] = buf.pright[0];
					pright[1] = buf.pright[1];
					return *this;
				}
				
				void *pleft[2];
				void *pright[2];
		};

	public:
		ASIOInterface(AsioUiInterface * ui = 0);
		~ASIOInterface() throw();

		/*override*/ AudioDriverInfo info() const;
		/*override*/ void Configure();

		/*override*/ int GetInputLatency() { return _inlatency; }
		/*override*/ int GetOutputLatency() { return _outlatency; }

		DriverEnum GetDriverFromidx(int driverID);
		void ControlPanel(int driverID);
		std::vector<DriverEnum> _drivEnum;
		static int _ASIObufferSize;

	protected:
		/*override*/ void do_open();
		/*override*/ void do_start();
		/*override*/ void do_stop();
		/*override*/ void do_close();

	private:
		int GetBufferSize();
		void GetCapturePorts(std::vector<std::string>&ports);
		void AddCapturePort(int idx);
		void RemoveCapturePort(int idx);
		void GetReadBuffers(int idx, float **pleft, float **pright,int numsamples);
		int GetWritePos();
		int GetPlayPos();		
		PortOut GetOutPortFromidx(int driverID);
		int GetidxFromOutPort(PortOut&port);		

		static void bufferSwitch(long index, ASIOBool processNow);
		static ASIOInterface * instance_;
		static ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);
		static void sampleRateChanged(ASIOSampleRate sRate);
		static long asioMessages(long selector, long value, void* message, double* opt);

		static AsioStereoBuffer *ASIObuffers;
		static bool _firstrun;
		static bool _supportsOutputReady;

		void ReadConfig();
		void WriteConfig();

		ASIOCallbacks asioCallbacks;

		//int drivercount;
		//char szFullName[MAX_ASIO_OUTPUTS][160];

		// callback prototypes
		#if 0
		int driverindex[MAX_ASIO_OUTPUTS];
		int outputindex[MAX_ASIO_OUTPUTS];
		long minSamples[MAX_ASIO_DRIVERS];
		long maxSamples[MAX_ASIO_DRIVERS];
		long prefSamples[MAX_ASIO_DRIVERS];
		long Granularity[MAX_ASIO_DRIVERS];
		int currentSamples[MAX_ASIO_DRIVERS];
		#endif

		mutex mutex_;
		typedef class scoped_lock<mutex> scoped_lock;
		condition<scoped_lock> condition_;

		bool _configured;
		bool _running;
		//int _driverID;
		AsioDrivers asioDrivers;
		long _inlatency;
		long _outlatency;


		static PortOut _selectedout;
		static std::vector<PortCapt> _selectedins;
		std::vector<int> _portMapping;

		AsioUiInterface* ui_;
		ASIOBufferInfo* info_;
};
	
}}
#endif
