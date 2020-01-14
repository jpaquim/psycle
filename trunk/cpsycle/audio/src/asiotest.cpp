// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

/*
#include "../../detail/prefix.h"

#include "asiotest.h"
#include <operations.h>

#include "../../driver/driver.h"

#define ASIO_VERSION 2L

#include <string>
#include <vector>
#include <map>

#include "ASIO\\asiodrivers.h"
#include "ASIO\\asio.h"
#include "ASIO\\asiosys.h"
#include "ASIO\\asio.h"
#include "ASIO\\asiodrivers.h"

#include <stdio.h>

#define MAX_ASIO_DRIVERS 32
#define MAX_ASIO_OUTPUTS 128

static void driver_free(Driver*);
static int driver_init(Driver*);
static void driver_connect(Driver*, void* context, AUDIODRIVERWORKFN callback,
	void* handle);
static int driver_open(Driver*);
static void driver_configure(Driver*, psy_Properties*);
static int driver_close(Driver*);
static int driver_dispose(Driver*);
static unsigned int samplerate(Driver*);

static void init_properties(Driver* driver);

static char szFullName[MAX_ASIO_OUTPUTS][160];
static char szNameBuf[MAX_ASIO_DRIVERS][64];
static int drivercount;
static int driverindex[MAX_ASIO_OUTPUTS];
static int outputindex[MAX_ASIO_OUTPUTS];
static long minSamples[MAX_ASIO_DRIVERS];
static long maxSamples[MAX_ASIO_DRIVERS];
static long prefSamples[MAX_ASIO_DRIVERS];
static long Granularity[MAX_ASIO_DRIVERS];

static AsioDrivers asioDrivers;
static int currentSamples[MAX_ASIO_DRIVERS];

static int _initialized;
static int _running;
static long _inlatency;
static long _outlatency;
// static AsioStereoBuffer *ASIObuffers;
static int _firstrun;
static int _supportsOutputReady;
static uint32_t writePos;
static uint32_t m_wrapControl;

class DriverEnum;
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
	std::string GetName() const;
	bool IsFormatSupported(DriverEnum* driver, int samplerate) const;
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

static std::vector<PortCapt> _selectedins;
static AsioStereoBuffer *ASIObuffers;
static PortOut _selectedout;

typedef struct {
	Driver driver;
	int	_driverID;
	HANDLE hEvent;
} AsioDriver;

Driver* create_asio_driver(void)
{
	int drivers;		
	ASIODriverInfo driverInfo;	
	char* pNameBuf[64];
	int i;

	for (i = 0; i < 64; i++)
	{
		pNameBuf[i] = szNameBuf[i];
	}
	drivers = asioDrivers.getDriverNames((char**)pNameBuf,64);
	for (i = 0; i < drivers; i++)
	{
#ifndef ALLOW_NON_ASIO
		if (strcmp("ASIO DirectX Driver",szNameBuf[i])==0)
		{
			continue;
		}
		if (strcmp("ASIO DirectX Full Duplex Driver",szNameBuf[i])==0)
		{
			continue;
		}
		if (strcmp("ASIO Multimedia Driver",szNameBuf[i])==0)
		{
			continue;
		}
#endif
		if (asioDrivers.loadDriver(szNameBuf[i]))
		{

			// initialize the driver
			if (ASIOInit(&driverInfo) == ASE_OK)
			{

				// TRACE(szNameBuf[i]);

				long in,out;

				if (ASIOGetChannels(&in,&out) == ASE_OK)
				{
					for (int j = 0; j < out; j+=2)
					{
						driverindex[drivercount]=i;
						outputindex[drivercount]=j;

						ASIOChannelInfo channelInfo;
						channelInfo.isInput = FALSE;
						channelInfo.channel = j;
						ASIOGetChannelInfo(&channelInfo);
						sprintf(szFullName[drivercount],"%s : %s",szNameBuf[i],channelInfo.name);

						switch (channelInfo.type)
						{
						case ASIOSTInt16LSB:
						case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
						case ASIOSTInt16MSB:
						case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
							strcat(szFullName[drivercount]," : 16 bit");
							break;
						case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
						case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
							strcat(szFullName[drivercount]," : 18 bit");
							break;

						case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
						case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
							strcat(szFullName[drivercount]," : 20 bit");
							break;
						case ASIOSTInt24LSB:		// used for 20 bits as well
						case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
						case ASIOSTInt24MSB:		// used for 20 bits as well
						case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
							strcat(szFullName[drivercount]," : 24 bit");
							break;
						case ASIOSTInt32LSB:
						case ASIOSTInt32MSB:
							strcat(szFullName[drivercount],": 32 bit");
							break;
						case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
							strcat(szFullName[drivercount],": 32 bit float");
							break;
						case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
							strcat(szFullName[drivercount],": 64 bit float");
							break;
						case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
						case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
							strcat(szFullName[drivercount]," : unsupported MSB float bitorder");
							continue;
							break;
						}
						drivercount++;
					}
				}

				if (ASIOGetBufferSize(&minSamples[i], &maxSamples[i], &prefSamples[i], &Granularity[i]) != ASE_OK)
				{
					minSamples[i] = maxSamples[i] = prefSamples[i] = 2048;
					Granularity[i] = 0;
				}
				currentSamples[i] = prefSamples[i];
			}
			asioDrivers.removeCurrentDriver();
		}
	}
	
	AsioDriver* asio = (AsioDriver*) malloc(sizeof(AsioDriver));
	if (asio) {
		memset(asio, 0, sizeof(AsioDriver));
		asio->driver.open = driver_open;
		asio->driver.free = driver_free;
		asio->driver.connect = driver_connect;
		asio->driver.open = driver_open;
		asio->driver.close = driver_close;
		asio->driver.dispose = driver_dispose;
		asio->driver.configure = driver_configure;
		asio->driver.samplerate = samplerate;
		asio->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		driver_init(&asio->driver);
		return &asio->driver;
	} else {
		return 0;
	}	
}

void driver_free(Driver* driver)
{
	free(driver);
}

int driver_init(Driver* driver)
{
	return 0;
}

int driver_dispose(Driver* driver)
{
	//if (_initialized) 
	//{
	//	Reset();
	//}
//	ASIOExit();
	asioDrivers.removeCurrentDriver();
	properties_free(driver->properties);
	driver->properties = 0;
	return 0;
}

void driver_connect(Driver* driver, void* context, AUDIODRIVERWORKFN callback,
	void* handle)
{
	driver->_pCallback = callback;
	driver->_callbackContext = context;
}

int driver_open(Driver* driver)
{
	return 0;
}

int driver_close(Driver* driver)
{
	return 0;
}

void driver_configure(Driver* driver, psy_Properties* config)
{

}

unsigned int samplerate(Driver* self)
{
	return 44100;
}

void init_properties(Driver* self)
{		
	self->properties = psy_properties_create();
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "name", "Silent Driver"),
		PSY_PROPERTY_HINT_READONLY);
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "vendor", "Psycedelics"),
		PSY_PROPERTY_HINT_READONLY);
	psy_properties_sethint(
		psy_properties_append_string(self->properties, "version", "1.0"),
		PSY_PROPERTY_HINT_READONLY);
}


#define SwapDouble(v) SwapLongLong((long long)(v))
#define SwapFloat(v) SwapLong(long(v))
#define SwapLongLong(v) ((((v)>>56)&0xFF)|(((v)>>40)&0xFF00)|(((v)>>24)&0xFF0000)|(((v)>>8)&0xFF000000) \
				       | (((v)&0xFF)<<56)|(((v)&0xFF00)<<40)|(((v)&0xFF0000)<<24)|(((v)&0xFF000000)<<8))
#define SwapLong(v) ((((v)>>24)&0xFF)|(((v)>>8)&0xFF00)|(((v)&0xFF00)<<8)|(((v)&0xFF)<<24)) 
#define SwapShort(v) ((((v)>>8)&0xFF)|(((v)&0xFF)<<8))

//ADVICE: Remember that psycle uses the range +32768.f to -32768.f. All conversions are done relative to this.
ASIOTime bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow)
{
	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization.
	if(_firstrun)
	{
// 		universalis::cpu::exceptions::install_handler_in_thread();
		_firstrun = false;
	}
	writePos = timeInfo->timeInfo.samplePosition.lo;
	if (timeInfo->timeInfo.samplePosition.hi != m_wrapControl) {
		m_wrapControl = timeInfo->timeInfo.samplePosition.hi;
//		PsycleGlobal::midi().ReSync();	// MIDI IMPLEMENTATION
	}

	const unsigned int _ASIObufferSamples = 1024; // settings_->blockFrames();
	//////////////////////////////////////////////////////////////////////////
	// Inputs
	unsigned int counter(0);
	for (; counter< _selectedins.size(); ++counter)
	{
		ASIOSampleType dtype =_selectedins[counter].port->_info.type;
		int i(0);
		switch (dtype)
		{
		case ASIOSTInt16LSB:
			{
				short* inl;
				short* inr;
				inl = (short*)ASIObuffers[counter].pleft[index];
				inr = (short*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					_selectedins[counter].pleft[i] = *inl;
					_selectedins[counter].pright[i] = *inr;
				}
			}
			break;
		case ASIOSTInt24LSB:		// used for 20 bits as well
			{
				char* inl;
				char* inr;
				inl = (char*)ASIObuffers[counter].pleft[index];
				inr = (char*)ASIObuffers[counter].pright[index];
				int t;
				char* pt = (char*)&t;
				for (i = 0; i < _ASIObufferSamples; i++)
				{
					pt[0] = *inl++;
					pt[1] = *inl++;
					pt[2] = *inl++;
					_selectedins[counter].pleft[i] = t*0.00390625f;

					pt[0] = *inr++;
					pt[1] = *inr++;
					pt[2] = *inr++;
					_selectedins[counter].pright[i] = t*0.00390625f;

				}
			}
			break;
		case ASIOSTInt32LSB:
			{
				long* inl;
				long* inr;
				inl = (long*)ASIObuffers[counter].pleft[index];
				inr = (long*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					_selectedins[counter].pleft[i] = (*inl)*0.0000152587890625;
					_selectedins[counter].pright[i] = (*inr)*0.0000152587890625;
				}
			}
			break;
		case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
			{
				dsp.movmul(static_cast<float*>(ASIObuffers[counter].pleft[index]),_selectedins[counter].pleft,_ASIObufferSamples,32768.0f);
				dsp.movmul(static_cast<float*>(ASIObuffers[counter].pright[index]),_selectedins[counter].pright,_ASIObufferSamples,32768.0f);
			}
			break;
		case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
			{
				double* inl;
				double* inr;
				inl = (double*)ASIObuffers[counter].pleft[index];
				inr = (double*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					_selectedins[counter].pleft[i] = (*inl)*32768.0;
					_selectedins[counter].pright[i] = (*inr)*32768.0;
				}
			}
			break;
			// these are used for 32 bit data buffer, with different alignment of the data inside
			// 32 bit PCI bus systems can more easily used with these
		case ASIOSTInt32LSB16:		// 32 bit data with 16 bit alignment (right aligned)
			{
				long* inl;
				long* inr;
				inl = (long*)ASIObuffers[counter].pleft[index];
				inr = (long*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					_selectedins[counter].pleft[i] = *inl;
					_selectedins[counter].pright[i] = *inr;
				}
			}
			break;
		case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment (right aligned)
			{
				long* inl;
				long* inr;
				inl = (long*)ASIObuffers[counter].pleft[index];
				inr = (long*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					_selectedins[counter].pleft[i] = (*inl)*0.25f;
					_selectedins[counter].pright[i] = (*inr)*0.25f;
				}
			}
			break;
		case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment (right aligned)
			{
				long* inl;
				long* inr;
				inl = (long*)ASIObuffers[counter].pleft[index];
				inr = (long*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					_selectedins[counter].pleft[i] = (*inl)*0.0625f;
					_selectedins[counter].pright[i] = (*inr)*0.0625f;
				}
			}
			break;
		case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment (right aligned)
			{
				long* inl;
				long* inr;
				inl = (long*)ASIObuffers[counter].pleft[index];
				inr = (long*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					_selectedins[counter].pleft[i] = (*inl)*0.00390625f;
					_selectedins[counter].pright[i] = (*inr)*0.00390625f;
				}
			}
			break;
		case ASIOSTInt16MSB:
			{
				short* inl;
				short* inr;
				inl = (short*)ASIObuffers[counter].pleft[index];
				inr = (short*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					_selectedins[counter].pleft[i] = SwapShort(*inl);
					_selectedins[counter].pright[i] = SwapShort(*inr);
				}
			}
			break;
		case ASIOSTInt24MSB:		// used for 20 bits as well
			{
				char* inl;
				char* inr;
				inl = (char*)ASIObuffers[counter].pleft[index];
				inr = (char*)ASIObuffers[counter].pright[index];
				int t;
				char* pt = (char*)&t;
				for (i = 0; i < _ASIObufferSamples; i++)
				{
					pt[2] = *inl++;
					pt[1] = *inl++;
					pt[0] = *inl++;
					_selectedins[counter].pleft[i] = t*0.00390625f;

					pt[2] = *inr++;
					pt[1] = *inr++;
					pt[0] = *inr++;
					_selectedins[counter].pright[i] = t*0.00390625f;

				}
			}
			break;
		case ASIOSTInt32MSB:
			{
				long* inl;
				long* inr;
				inl = (long*)ASIObuffers[counter].pleft[index];
				inr = (long*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					long val = SwapLong(*inl++);
					_selectedins[counter].pleft[i] = val *0.0000152587890625;
					val = SwapLong(*inr++);
					_selectedins[counter].pright[i] = val *0.0000152587890625;
				}
			}
			break;
		case ASIOSTInt32MSB16:		// 32 bit data with 16 bit alignment (right aligned)
			{
				long* inl;
				long* inr;
				inl = (long*)ASIObuffers[counter].pleft[index];
				inr = (long*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					_selectedins[counter].pleft[i] = SwapLong(*inl);
					_selectedins[counter].pright[i] = SwapLong(*inr);
				}
			}
			break;
		case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment (right aligned)
			{
				long* inl;
				long* inr;
				inl = (long*)ASIObuffers[counter].pleft[index];
				inr = (long*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					long val = SwapLong(*inl);
					_selectedins[counter].pleft[i] = val*0.25f;
					val = SwapLong(*inr);
					_selectedins[counter].pright[i] = val*0.25f;
				}
			}
			break;
		case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment (right aligned)
			{
				long* inl;
				long* inr;
				inl = (long*)ASIObuffers[counter].pleft[index];
				inr = (long*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					long val = SwapLong(*inl);
					_selectedins[counter].pleft[i] = val*0.0625f;
					val = SwapLong(*inr);
					_selectedins[counter].pright[i] = val*0.0625f;
				}
			}
			break;
		case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment (right aligned)
			{
				long* inl;
				long* inr;
				inl = (long*)ASIObuffers[counter].pleft[index];
				inr = (long*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					long val = SwapLong(*inl);
					_selectedins[counter].pleft[i] = val*0.00390625f;
					val = SwapLong(*inr);
					_selectedins[counter].pright[i] = val*0.00390625f;
				}
			}
			break;
		case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on PowerPC implementation
			{
				float* inl;
				float* inr;
				inl = (float*)ASIObuffers[counter].pleft[index];
				inr = (float*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
					///  _selectedins[counter].pleft[i] = SwapFloat(*inl);
					/// _selectedins[counter].pright[i] = SwapFloat(*inr);
				}
			}
			break;
		case ASIOSTFloat64MSB: 		// IEEE 754 64 bit float, as found on PowerPC implementation
			{
				double* inl;
				double* inr;
				inl = (double*)ASIObuffers[counter].pleft[index];
				inr = (double*)ASIObuffers[counter].pright[index];
				for (i = 0; i < _ASIObufferSamples; i++,inl++,inr++)
				{
///					_selectedins[counter].pleft[i] = SwapDouble(*inl);
///					_selectedins[counter].pright[i] = SwapDouble(*inr);
				}
			}
			break;
		}
		_selectedins[counter].machinepos=0;
	}

	//////////////////////////////////////////////////////////////////////////
	// Outputs
	float *pBuf = 0; /// _pCallback(_pCallbackContext, _ASIObufferSamples);
	switch (_selectedout.port->_info.type)
	{
	case ASIOSTInt16LSB:
		{
		int16_t* outl;
		int16_t* outr;
		outl = (int16_t*)ASIObuffers[counter].pleft[index];
		outr = (int16_t*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = rint_clip<int16_t>(*pBuf++);
///			*outr++ = rint_clip<int16_t>(*pBuf++);
			}
		}
		break;
	case ASIOSTInt24LSB:		// used for 20 bits as well
		{
			char* outl;
			char* outr;
			outl = (char*)ASIObuffers[counter].pleft[index];
			outr = (char*)ASIObuffers[counter].pright[index];
			int t;
			char* pt = (char*)&t;
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///		t = rint_clip<int, 24>((*pBuf++) * 256.0f);
				*outl++ = pt[0];
				*outl++ = pt[1];
				*outl++ = pt[2];

///			t = rint_clip<int, 24>((*pBuf++) * 256.0f);
				*outr++ = pt[0];
				*outr++ = pt[1];
				*outr++ = pt[2];
			}
		}

		break;
	case ASIOSTInt32LSB:
		{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		// Don't really know why, but the -100 is what made the clipping work correctly.
		////
		int const max = 0; //((1u << ((sizeof(int32_t) << 3) - 1)) - 100);
		int const min = 0; //(-max - 1);
		for(int i = 0; i < _ASIObufferSamples; ++i) {
//			*outl++ = psycle::helpers::math::rint<int32_t>(psycle::helpers::math::clip(float(min), (*pBuf++) * 65536.0f, float(max)));
//			*outr++ = psycle::helpers::math::rint<int32_t>(psycle::helpers::math::clip(float(min), (*pBuf++) * 65536.0f, float(max)));
			//*outl++ = rint_clip<int32_t>((*pBuf++) * 65536.0f);
			//*outr++ = rint_clip<int32_t>((*pBuf++) * 65536.0f);
			}
		}
		break;
	case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
		{
			float* outl;
			float* outr;
			outl = (float*)ASIObuffers[counter].pleft[index];
			outr = (float*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
			*outl++ = (*pBuf++) / 32768.0f;
			*outr++ = (*pBuf++) / 32768.0f;
			}
		}
		break;
	case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
		{
			double* outl;
			double* outr;
			outl = (double*)ASIObuffers[counter].pleft[index];
			outr = (double*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
			*outl++ = (*pBuf++) / 32768.0;
			*outr++ = (*pBuf++) / 32768.0;
			}
		}
		break;
		// these are used for 32 bit data buffer, with different alignment of the data inside
		// 32 bit PCI bus systems can more easily used with these
case ASIOSTInt32LSB16: // 32 bit data with 16 bit alignment (right aligned)
		{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = rint_clip<int32_t, 16>(*pBuf++);
///			*outr++ = rint_clip<int32_t, 16>(*pBuf++);
			}
		}
		break;
	case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment (right aligned)
		{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = rint_clip<int32_t, 18>((*pBuf++) * 4.0f);
///			*outr++ = rint_clip<int32_t, 18>((*pBuf++) * 4.0f);
			}
		}
		break;
	case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment (right aligned)
		{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = rint_clip<int32_t, 20>((*pBuf++) * 16.0f);
///			*outr++ = rint_clip<int32_t, 20>((*pBuf++) * 16.0f);
			}
		}
		break;
	case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment (right aligned)
		{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = rint_clip<int32_t, 24>((*pBuf++) * 256.0f);
///			*outr++ = rint_clip<int32_t, 24>((*pBuf++) * 256.0f);
			}
		}
		break;
	case ASIOSTInt16MSB:
		{
		int16_t* outl;
		int16_t* outr;
		outl = (int16_t*)ASIObuffers[counter].pleft[index];
		outr = (int16_t*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = SwapShort(rint_clip<int16_t>(*pBuf++));
///			*outr++ = SwapShort(rint_clip<int16_t>(*pBuf++));
			}
		}
		break;
	case ASIOSTInt24MSB:		// used for 20 bits as well
		{
			char* outl;
			char* outr;
			outl = (char*)ASIObuffers[counter].pleft[index];
			outr = (char*)ASIObuffers[counter].pright[index];
			int t;
			char* pt = (char*)&t;
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			t = rint_clip<int, 24>((*pBuf++) * 256.0f);
				*outl++ = pt[2];
				*outl++ = pt[1];
				*outl++ = pt[0];

///			t = rint_clip<int, 24>((*pBuf++) * 256.0f);
				*outr++ = pt[2];
				*outr++ = pt[1];
				*outr++ = pt[0];
			}
		}
		break;
	case ASIOSTInt32MSB:
		{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
			//See the LSB case above.
			int const max = 0; // ((1u << ((sizeof(int32_t) << 3) - 1)) - 100);
			int const min = 0; // (-max - 1);
///			*outl++ = SwapLong(psycle::helpers::math::rint<int32_t>(psycle::helpers::math::clip(float(min), (*pBuf++) * 65536.0f, float(max))));
///			*outr++ = SwapLong(psycle::helpers::math::rint<int32_t>(psycle::helpers::math::clip(float(min), (*pBuf++) * 65536.0f, float(max))));
			//*outl++ = SwapLong(rint_clip<int32_t>((*pBuf++) * 65536.0f));
			//*outr++ = SwapLong(rint_clip<int32_t>((*pBuf++) * 65536.0f));
			}
		}
		break;
	case ASIOSTInt32MSB16:		// 32 bit data with 16 bit alignment (right aligned)
		{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = SwapLong((rint_clip<int32_t, 16>(*pBuf++)));
///			*outr++ = SwapLong((rint_clip<int32_t, 16>(*pBuf++)));
			}
		}
		break;
	case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment (right aligned)
		{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = SwapLong((rint_clip<int32_t, 18>((*pBuf++) * 4.0f)));
///			*outr++ = SwapLong((rint_clip<int32_t, 18>((*pBuf++) * 4.0f)));
			}
		}
		break;
	case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment (right aligned)
		{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = SwapLong((rint_clip<int32_t, 20>((*pBuf++) * 16.0f)));
///			*outr++ = SwapLong((rint_clip<int32_t, 20>((*pBuf++) * 16.0f)));
			}
		}
		break;
	case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment (right aligned)
		{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = SwapLong((rint_clip<int32_t, 24>((*pBuf++) * 256.0f)));
///			*outr++ = SwapLong((rint_clip<int32_t, 24>((*pBuf++) * 256.0f)));
			}
		}
		break;
	case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on PowerPC implementation
		{
		float* outl;
		float* outr;
		outl = (float*)ASIObuffers[counter].pleft[index];
		outr = (float*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = SwapFloat((*pBuf++) * 0.00030517578125);
///			*outr++ = SwapFloat((*pBuf++) * 0.00030517578125);
			}
		}
		break;
	case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on PowerPC implementation
		{
		double* outl;
		double* outr;
		outl = (double*)ASIObuffers[counter].pleft[index];
		outr = (double*)ASIObuffers[counter].pright[index];
		for(int i = 0; i < _ASIObufferSamples; ++i) {
///			*outl++ = SwapDouble((*pBuf++) * 0.00030517578125);
///			*outr++ = SwapDouble((*pBuf++) * 0.00030517578125);
			}
		}
		break;
	}
	// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
	if(_supportsOutputReady) ASIOOutputReady();
	ASIOTime rv;
	memset(&rv, 0, sizeof(ASIOTime));	
	return rv;
}

void ASIOInterface_bufferSwitch(long index, ASIOBool processNow)
{
	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.

	// as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
	// though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
	ASIOTime  timeInfo;
	memset(&timeInfo, 0, sizeof timeInfo);

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if(ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	bufferSwitchTimeInfo (&timeInfo, index, processNow);
}


void ASIOInterface_sampleRateChanged(ASIOSampleRate sRate)
{
	// do whatever you need to do if the sample rate changed
	// usually this only happens during external sync.
	// Audio processing is not stopped by the driver, actual sample rate
	// might not have even changed, maybe only the sample rate status of an
	// AES/EBU or S/PDIF digital input at the audio device.
	// You might have to update time/sample related conversion routines, etc.
///	settings_->setSamplesPerSec(sRate);
}

long ASIOInterface_asioMessages(long selector, long value, void* message, double* opt)
{
	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;
	switch(selector)
	{
		case kAsioSelectorSupported:
			if
				(
					value == kAsioResetRequest ||
					value == kAsioEngineVersion ||
					value == kAsioResyncRequest ||
					value == kAsioLatenciesChanged ||
					// the following three were added for ASIO 2.0, you don't necessarily have to support them
					value == kAsioSupportsTimeInfo ||
					value == kAsioSupportsTimeCode ||
					value == kAsioSupportsInputMonitor
				)
				ret = 1L;
			break;
		case kAsioResetRequest:
			// defer the task and perform the reset of the driver during the next "safe" situation
			// You cannot reset the driver right now, as this code is called from the driver.
			// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
			// Afterwards you initialize the driver again.
			//asioDriverInfo.stopped;  // In this sample the processing will just stop
			ret = 1L;
			break;
		case kAsioResyncRequest:
			// This informs the application, that the driver encountered some non fatal data loss.
			// It is used for synchronization purposes of different media.
			// Added mainly to work around the Win16Mutex problems in Windows 95/98 with the
			// Windows Multimedia system, which could loose data because the Mutex was hold too long
			// by another thread.
			// However a driver can issue it in other situations, too.
			ret = 1L;
			break;
		case kAsioLatenciesChanged:
			// This will inform the host application that the drivers were latencies changed.
			// Beware, it this does not mean that the buffer sizes have changed!
			// You might need to update internal delay data.
			ret = 1L;
			break;
		case kAsioEngineVersion:
			// return the supported ASIO version of the host application
			// If a host applications does not implement this selector, ASIO 1.0 is assumed
			// by the driver
			ret = ASIO_VERSION;
			break;
		case kAsioSupportsTimeInfo:
			// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback
			// is supported.
			// For compatibility with ASIO 1.0 drivers the host application should always support
			// the "old" bufferSwitch method, too.
			ret = 1;
			break;
		case kAsioSupportsTimeCode:
			// informs the driver wether application is interested in time code info.
			// If an application does not need to know about time code, the driver has less work
			// to do.
			ret = 0;
			break;
	}
	return ret;
}

*/