// notes: asio drivers will tell us their preferred settings with : ASIOGetBufferSize

#include "stdafx.h"
#include "ASIOInterface.h"
#include "resource.h"
#include "Registry.h"
#include "ASIOConfig.h"
#include "Configuration.h"
#include "MidiInput.h"
#include "ASIO\\asiosys.h"
#include "ASIO\\asio.h"
#include "ASIO\\asiodrivers.h"

#include "helpers.h"


#define ALLOW_NON_ASIO


void bufferSwitch(long index, ASIOBool processNow);
ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow);
void sampleRateChanged(ASIOSampleRate sRate);
long asioMessages(long selector, long value, void* message, double* opt);

void * ASIObuffers[2][2];

ASIOCallbacks asioCallbacks;
ASIOSampleType asioSample;
AUDIODRIVERWORKFN	_pASIOCallback;
void* _pASIOcallbackContext;
int _ASIObufferSize;
bool asiopostOutput;

AudioDriverInfo ASIOInterface::_info = { "ASIO 2.0 Output" };
CCriticalSection ASIOInterface::_lock;

ASIOInterface::ASIOInterface(void)
{
	_initialized = false;
	_configured = false;
	_running = false;
	_pASIOCallback = NULL;

	char* pNameBuf[64];
	for (int i = 0; i < 64; i++)
	{
		pNameBuf[i] = szNameBuf[i];
	}
	int drivers = asioDrivers.getDriverNames((char**)pNameBuf,64);

	drivercount = 0;

	ASIODriverInfo driverInfo;
	driverInfo.sysRef = NULL;

	for (int i = 0; i < drivers; i++)
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

				TRACE(szNameBuf[i]);

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
}

ASIOInterface::~ASIOInterface(void)
{
	if (_initialized) 
	{
		Reset();
	}
//	ASIOExit();
	asioDrivers.removeCurrentDriver();
}

void ASIOInterface::Error(char const *msg)
{
	MessageBox(NULL, msg, "ASIO Driver", MB_ICONWARNING);
}

void ASIOInterface::Initialize(
	HWND hwnd,
	AUDIODRIVERWORKFN pCallback,
	void* context)
{
	_pASIOcallbackContext = context;
	_pASIOCallback = pCallback;
	_running = false;
	ReadConfig();
	_initialized = true;

}

void ASIOInterface::ReadConfig()
{
	bool configured;
	DWORD type;
	DWORD numData;
	Registry reg;

	// Default configuration
	//
	_samplesPerSec=44100;
	_driverID=0;
	_ASIObufferSize = 1024;
	_channelmode = 3; // always stereo
	_bitDepth = 16; // asio don't care about bit depth

	if (reg.OpenRootKey(HKEY_CURRENT_USER, CONFIG_ROOT_KEY) != ERROR_SUCCESS)
	{
		return;
	}
	if (reg.OpenKey("ASIOOut") != ERROR_SUCCESS)
	{
		return;
	}
	configured = true;
	numData = sizeof(_ASIObufferSize);
	configured &= (reg.QueryValue("BufferSize", &type, (BYTE*)&_ASIObufferSize, &numData) == ERROR_SUCCESS);
	numData = sizeof(_driverID);
	configured &= (reg.QueryValue("DriverID", &type, (BYTE*)&_driverID, &numData) == ERROR_SUCCESS);
	numData = sizeof(_samplesPerSec);
	configured &= (reg.QueryValue("SamplesPerSec", &type, (BYTE*)&_samplesPerSec, &numData) == ERROR_SUCCESS);
	reg.CloseKey();
	reg.CloseRootKey();
	_configured = configured;

	currentSamples[_driverID]=_ASIObufferSize;
}

void ASIOInterface::WriteConfig()
{
	Registry reg;
	if (reg.OpenRootKey(HKEY_CURRENT_USER, CONFIG_ROOT_KEY) != ERROR_SUCCESS)
	{
		Error("Unable to write configuration to the registry");
		return;
	}
	if (reg.OpenKey("ASIOOut") != ERROR_SUCCESS)
	{
		if (reg.CreateKey("ASIOOut") != ERROR_SUCCESS)
		{
			Error("Unable to write configuration to the registry");
			return;
		}
	}
	reg.SetValue("BufferSize", REG_DWORD, (BYTE*)&_ASIObufferSize, sizeof(_ASIObufferSize));
	reg.SetValue("DriverID", REG_DWORD, (BYTE*)&_driverID, sizeof(_driverID));
	reg.SetValue("SamplesPerSec", REG_DWORD, (BYTE*)&_samplesPerSec, sizeof(_samplesPerSec));
	reg.CloseKey();
	reg.CloseRootKey();
}

void ASIOInterface::Configure()
{
	if (!_configured)
	{
		ReadConfig();
	}
	
	CASIOConfig dlg;
	dlg.pASIO = this;
	dlg.m_bufferSize = _ASIObufferSize;
	dlg.m_driverIndex = _driverID;
	dlg.m_sampleRate = _samplesPerSec;

	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	
	int oldbs = _ASIObufferSize;
	int oldvid = _driverID;
	int oldsps = _samplesPerSec;

	if (_initialized)
	{
		Stop();
	}

	_ASIObufferSize = dlg.m_bufferSize;
	_driverID = dlg.m_driverIndex;
	_samplesPerSec = dlg.m_sampleRate;

	_configured = true;

	if (_initialized)
	{
		if (Start())
		{
			WriteConfig();
		}
		else
		{
			_ASIObufferSize = oldbs;
			_driverID = oldvid;
			_samplesPerSec = oldsps;

			Start();
		}
	}
	else
	{
		WriteConfig();
	}
}

bool ASIOInterface::Start()
{
	CSingleLock lock(&_lock, TRUE);
	if (_running)
	{
		return true;
	}

	if (_pASIOCallback == NULL)
	{
		_running = false;
		return false;
	}

	// BEGIN -  Code 

	asioDrivers.removeCurrentDriver();

	ASIODriverInfo driverInfo;
	driverInfo.sysRef = NULL;

	if (!asioDrivers.loadDriver(szNameBuf[driverindex[_driverID]]))
	{
		_running = false;
		return false;
	}

	// initialize the driver
	if (ASIOInit(&driverInfo) != ASE_OK)
	{
//		ASIOExit();
		asioDrivers.removeCurrentDriver();
		_running = false;
		return false;
	}

	if (ASIOSetSampleRate(_samplesPerSec) != ASE_OK)
	{
		_samplesPerSec = 44100;
		if (ASIOSetSampleRate(_samplesPerSec) != ASE_OK)
		{

	//		ASIOExit();
			asioDrivers.removeCurrentDriver();
			_running = false;
			return false;
		}
	}

	if(ASIOOutputReady() == ASE_OK)
		asiopostOutput = true;
	else
		asiopostOutput = false;

	// set up the asioCallback structure and create the ASIO data buffer
	asioCallbacks.bufferSwitch = &bufferSwitch;
	asioCallbacks.sampleRateDidChange = &sampleRateChanged;
	asioCallbacks.asioMessage = &asioMessages;
	asioCallbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfo;

	// fill the bufferInfos from the start without a gap
	ASIOBufferInfo info[2];

	// prepare outputs
	info[0].isInput = ASIOFalse;
	info[0].channelNum = outputindex[_driverID];
	info[0].buffers[0] = info->buffers[1] = 0;

	info[1].isInput = ASIOFalse;
	info[1].channelNum = outputindex[_driverID]+1;
	info[1].buffers[0] = info->buffers[1] = 0;

	// create and activate buffers
		
	if (_ASIObufferSize < minSamples[driverindex[_driverID]])
	{
		_ASIObufferSize = prefSamples[driverindex[_driverID]];
	}
	else if (_ASIObufferSize > maxSamples[driverindex[_driverID]])
	{
		_ASIObufferSize = prefSamples[driverindex[_driverID]];
	}
	if (ASIOCreateBuffers(info,2,_ASIObufferSize,&asioCallbacks) != ASE_OK)
	{
//		ASIOExit();
		asioDrivers.removeCurrentDriver();
		_running = false;
		return false;
	}

	currentSamples[_driverID]=_ASIObufferSize;

	ASIObuffers[0][0] = info[0].buffers[0];
	ASIObuffers[0][1] = info[0].buffers[1];

	ASIObuffers[1][0] = info[1].buffers[0];
	ASIObuffers[1][1] = info[1].buffers[1];

	ASIOChannelInfo channelInfo;
	channelInfo.isInput = FALSE;
	channelInfo.channel = outputindex[_driverID];
	if (ASIOGetChannelInfo(&channelInfo) != ASE_OK)
	{
		ASIODisposeBuffers();
//		ASIOExit();
		asioDrivers.removeCurrentDriver();
		_running = false;
		return false;
	}

	asioSample = channelInfo.type;

	if (ASIOStart() != ASE_OK)
	{
		ASIODisposeBuffers();
//		ASIOExit();
		asioDrivers.removeCurrentDriver();
		_running = false;
		return false;
	}

	// END -  CODE


	_running = true;
	CMidiInput::Instance()->ReSync();	// MIDI IMPLEMENTATION
	return true;
}

bool ASIOInterface::Stop()
{
	CSingleLock lock(&_lock, TRUE);
	if (!_running)
	{
		return true;
	}
	_running = false;

	ASIOStop();
	ASIODisposeBuffers();
//	ASIOExit();
	asioDrivers.removeCurrentDriver();

	return true;
}

void ASIOInterface::Reset()
{
	Stop();
}

bool ASIOInterface::Enable(bool e)
{
	return e ? Start() : Stop();
}

int ASIOInterface::GetWritePos()
{
	// Not yet implemted
	if (!_running)
	{
		return 0;
	}
	return GetPlayPos();
}

int ASIOInterface::GetPlayPos()
{
	// Not yet implemted
	if (!_running)
	{
		return 0;
	}
	int playPos = 0;//int(Pa_StreamTime(stream));

	return playPos;
}

int ASIOInterface::GetBufferSize()
{ 
	return _ASIObufferSize; 
};

void ASIOInterface::ControlPanel(int driverID)
{
	if (driverindex[_driverID] != driverindex[driverID])
	{
		if (_running)
		{
			Stop();
			// load it
			if (asioDrivers.loadDriver(szNameBuf[driverindex[driverID]]))
			{
				ASIOControlPanel(); //you might want to check wether the ASIOControlPanel() can open
				asioDrivers.removeCurrentDriver();
			}

			Start();
		}
		else if (asioDrivers.loadDriver(szNameBuf[driverindex[driverID]]))
		{
			ASIOControlPanel(); //you might want to check wether the ASIOControlPanel() can open
			asioDrivers.removeCurrentDriver();
		}
	}

	else if (_running)
	{
		ASIOControlPanel(); //you might want to check wether the ASIOControlPanel() can open
	}
	else
	{
		// load it
		if (asioDrivers.loadDriver(szNameBuf[driverindex[driverID]]))
		{
			ASIOControlPanel(); //you might want to check wether the ASIOControlPanel() can open
			asioDrivers.removeCurrentDriver();
		}
	}
}

//----------------------------------------------------------------------------------
// conversion from 64 bit ASIOSample/ASIOTimeStamp to double float
#if NATIVE_INT64
	#define ASIO64toDouble(a)  (a)
#else
	const double twoRaisedTo32 = 4294967296.;
	#define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)
#endif

#define SwapLong(v) ((((v)>>24)&0xFF)|(((v)>>8)&0xFF00)|(((v)&0xFF00)<<8)|(((v)&0xFF)<<24)) ;   
#define SwapShort(v) ((((v)>>8)&0xFF)|(((v)&0xFF)<<8)) ;        

ASIOTime *bufferSwitchTimeInfo(ASIOTime *timeInfo, long index, ASIOBool processNow)
{	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.
	float *pBuf = _pASIOCallback(_pASIOcallbackContext, _ASIObufferSize);

	int i;
	switch (asioSample)
	{
	case ASIOSTInt16LSB:
		{
			WORD* outl;
			WORD* outr;
			outl = (WORD*)ASIObuffers[0][index];
			outr = (WORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = f2iclip16(*pBuf++);
				*outr++ = f2iclip16(*pBuf++);
			}
		}
		break;
	case ASIOSTInt24LSB:		// used for 20 bits as well
		{
			char* outl;
			char* outr;
			outl = (char*)ASIObuffers[0][index];
			outr = (char*)ASIObuffers[1][index];
			int t;
			char* pt = (char*)&t;

			for (i = 0; i < _ASIObufferSize; i++)
			{
				t = f2iclip24((*pBuf++)*256.0f);
				*outl++ = pt[0];
				*outl++ = pt[1];
				*outl++ = pt[2];

				t = f2iclip24((*pBuf++)*256.0f);
				*outr++ = pt[0];
				*outr++ = pt[1];
				*outr++ = pt[2];
			}
		}

		break;
	case ASIOSTInt32LSB:
		{
			DWORD* outl;
			DWORD* outr;
			outl = (DWORD*)ASIObuffers[0][index];
			outr = (DWORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = f2iclip32((*pBuf++)*65536.0f);
				*outr++ = f2iclip32((*pBuf++)*65536.0f);
			}
		}
		break;
	case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
		{
			float* outl;
			float* outr;
			outl = (float*)ASIObuffers[0][index];
			outr = (float*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = ((*pBuf++)/32768.0f);
				*outr++ = ((*pBuf++)/32768.0f);
			}
		}
		break;
	case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
		{
			double* outl;
			double* outr;
			outl = (double*)ASIObuffers[0][index];
			outr = (double*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = ((*pBuf++)/32768.0);
				*outr++ = ((*pBuf++)/32768.0);
			}
		}
		break;

		// these are used for 32 bit data buffer, with different alignment of the data inside
		// 32 bit PCI bus systems can more easily used with these
	case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
		{
			DWORD* outl;
			DWORD* outr;
			outl = (DWORD*)ASIObuffers[0][index];
			outr = (DWORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = f2iclip16(*pBuf++);
				*outr++ = f2iclip16(*pBuf++);
			}
		}
		break;

	case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
		{
			DWORD* outl;
			DWORD* outr;
			outl = (DWORD*)ASIObuffers[0][index];
			outr = (DWORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = f2iclip18((*pBuf++)*4.0f);
				*outr++ = f2iclip18((*pBuf++)*4.0f);
			}
		}
		break;
	case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
		{
			DWORD* outl;
			DWORD* outr;
			outl = (DWORD*)ASIObuffers[0][index];
			outr = (DWORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = f2iclip20((*pBuf++)*16.0f);
				*outr++ = f2iclip20((*pBuf++)*16.0f);
			}
		}
		break;
	case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
		{
			DWORD* outl;
			DWORD* outr;
			outl = (DWORD*)ASIObuffers[0][index];
			outr = (DWORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = f2iclip24((*pBuf++)*256.0f);
				*outr++ = f2iclip24((*pBuf++)*256.0f);
			}
		}
		break;

	case ASIOSTInt16MSB:
		{
			WORD* outl;
			WORD* outr;
			outl = (WORD*)ASIObuffers[0][index];
			outr = (WORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = SwapShort(f2iclip16(*pBuf++));
				*outr++ = SwapShort(f2iclip16(*pBuf++));
			}
		}
		break;
	case ASIOSTInt24MSB:		// used for 20 bits as well
		{
			char* outl;
			char* outr;
			outl = (char*)ASIObuffers[0][index];
			outr = (char*)ASIObuffers[1][index];
			int t;
			char* pt = (char*)&t;

			for (i = 0; i < _ASIObufferSize; i++)
			{
				t = f2iclip24((*pBuf++)*256.0f);
				*outl++ = pt[2];
				*outl++ = pt[1];
				*outl++ = pt[0];

				t = f2iclip24((*pBuf++)*256.0f);
				*outr++ = pt[2];
				*outr++ = pt[1];
				*outr++ = pt[0];
			}
		}
		break;
	case ASIOSTInt32MSB:
		{
			DWORD* outl;
			DWORD* outr;
			outl = (DWORD*)ASIObuffers[0][index];
			outr = (DWORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = SwapLong(f2iclip32((*pBuf++)*65536.0f));
				*outr++ = SwapLong(f2iclip32((*pBuf++)*65536.0f));
			}
		}
		break;
	case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
		{
			DWORD* outl;
			DWORD* outr;
			outl = (DWORD*)ASIObuffers[0][index];
			outr = (DWORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = SwapLong(f2iclip16(*pBuf++));
				*outr++ = SwapLong(f2iclip16(*pBuf++));
			}
		}
		break;
	case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
		{
			DWORD* outl;
			DWORD* outr;
			outl = (DWORD*)ASIObuffers[0][index];
			outr = (DWORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = SwapLong(f2iclip18((*pBuf++)*4.0f));
				*outr++ = SwapLong(f2iclip18((*pBuf++)*4.0f));
			}
		}
		break;
	case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
		{
			DWORD* outl;
			DWORD* outr;
			outl = (DWORD*)ASIObuffers[0][index];
			outr = (DWORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = SwapLong(f2iclip20((*pBuf++)*16.0f));
				*outr++ = SwapLong(f2iclip20((*pBuf++)*16.0f));
			}
		}
		break;

	case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
		{
			DWORD* outl;
			DWORD* outr;
			outl = (DWORD*)ASIObuffers[0][index];
			outr = (DWORD*)ASIObuffers[1][index];

			for (i = 0; i < _ASIObufferSize; i++)
			{
				*outl++ = SwapLong(f2iclip24((*pBuf++)*256.0f));
				*outr++ = SwapLong(f2iclip24((*pBuf++)*256.0f));
			}
		}
		break;
	case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
		memset (ASIObuffers[0][index], 0, _ASIObufferSize * 4);
		memset (ASIObuffers[1][index], 0, _ASIObufferSize * 4);
		break;
	case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
		memset (ASIObuffers[0][index], 0, _ASIObufferSize * 8);
		memset (ASIObuffers[1][index], 0, _ASIObufferSize * 8);
		break;
	}

	// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
	if (asiopostOutput)
	{
		ASIOOutputReady();
	}

	return 0L;
}

//----------------------------------------------------------------------------------
void bufferSwitch(long index, ASIOBool processNow)
{	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.

	// as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
	// though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
	ASIOTime  timeInfo;
	memset (&timeInfo, 0, sizeof (timeInfo));

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if(ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	bufferSwitchTimeInfo (&timeInfo, index, processNow);
}


//----------------------------------------------------------------------------------
void sampleRateChanged(ASIOSampleRate sRate)
{
	// do whatever you need to do if the sample rate changed
	// usually this only happens during external sync.
	// Audio processing is not stopped by the driver, actual sample rate
	// might not have even changed, maybe only the sample rate status of an
	// AES/EBU or S/PDIF digital input at the audio device.
	// You might have to update time/sample related conversion routines, etc.
}

//----------------------------------------------------------------------------------
long asioMessages(long selector, long value, void* message, double* opt)
{
	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;
	switch(selector)
	{
		case kAsioSelectorSupported:
			if(value == kAsioResetRequest
			|| value == kAsioEngineVersion
			|| value == kAsioResyncRequest
			|| value == kAsioLatenciesChanged
			// the following three were added for ASIO 2.0, you don't necessarily have to support them
			|| value == kAsioSupportsTimeInfo
			|| value == kAsioSupportsTimeCode
			|| value == kAsioSupportsInputMonitor)
				ret = 1L;
			break;
		case kAsioResetRequest:
			// defer the task and perform the reset of the driver during the next "safe" situation
			// You cannot reset the driver right now, as this code is called from the driver.
			// Reset the driver is done by completely destruct is. I.e. ASIOStop(), ASIODisposeBuffers(), Destruction
			// Afterwards you initialize the driver again.
//			asioDriverInfo.stopped;  // In this sample the processing will just stop
			
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
			ret = 2L;
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

