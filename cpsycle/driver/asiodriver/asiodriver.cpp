// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

// linking
#include "../../detail/prefix.h"
#include "../audiodriversettings.h"
#include "avrt.h"
#include <quantize.h>
#include <operations.h>
#include <rint.h>
#include <rintclip.h>

#include "../audiodriver.h"
#include "../../detail/psydef.h"

#include <windows.h>
#include <stdio.h>
#include <hashtbl.h>

#include <ks.h>
#include <ksmedia.h>

#if !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS  /* avoid warnings about ISO C functions */
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <asiodrivers.h>
#include <asio.h>

#include "../../detail/portable.h"

#define MAX_ASIO_DRIVERS 32
#define MAX_ASIO_OUTPUTS 128
#define ASIO_VERSION 2L

#undef KSDATAFORMAT_SUBTYPE_PCM
const GUID KSDATAFORMAT_SUBTYPE_PCM = { 0x00000001, 0x0000, 0x0010,
{0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

#undef KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 00000003, 0x0000, 0x0010,
{0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71} };

#define PSY_AUDIODRIVER_ASIO_GUID 0x0001

class ASIOInterface {
public:
	class DriverEnum;
	class PortEnum
	{
	public:
		PortEnum() :_idx(0)
		{
			_info.channel = 0;			_info.isInput = 0;
			_info.isActive = 0;			_info.channelGroup = 0;
			_info.type = 0;				memset(_info.name, 0, sizeof(_info.name));
		}
		PortEnum(int idx, ASIOChannelInfo info) :_idx(idx)
		{
			_info.channel = info.channel;		_info.isInput = info.isInput;
			_info.isActive = info.isActive;	_info.channelGroup = info.channelGroup;
			_info.type = info.type;			strcpy(_info.name, info.name);
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
		DriverEnum() :minSamples(2048), maxSamples(2048), prefSamples(2048), granularity(0) {};
		DriverEnum(std::string name) :_name(name) {};
		~DriverEnum() {};
		void AddInPort(PortEnum& port) { _portin.push_back(port); }
		void AddOutPort(PortEnum& port) { _portout.push_back(port); }
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
		PortOut() :driver(0), port(0) {};
		DriverEnum* driver;
		PortEnum* port;
	};
	class PortCapt : public PortOut
	{
	public:
		PortCapt() :PortOut(), pleft(0), pright(0), machinepos(0) {};
		float* pleft;
		float* pright;
		int machinepos;
	};
	class AsioStereoBuffer
	{
	public:
		AsioStereoBuffer()
		{
			pleft[0] = 0;	pleft[1] = 0;	pright[0] = 0;	pright[1] = 0; _sampletype = 0;
		}
		AsioStereoBuffer(void** left, void** right, ASIOSampleType stype)
		{
			pleft[0] = left[0]; pleft[1] = left[1]; pright[0] = right[0]; pright[1] = right[1];
			_sampletype = stype;
		}
		ASIOSampleType _sampletype;
		AsioStereoBuffer operator=(AsioStereoBuffer& buf)
		{
			_sampletype = buf._sampletype;
			pleft[0] = buf.pleft[0];
			pleft[1] = buf.pleft[1];
			pright[0] = buf.pright[0];
			pright[1] = buf.pright[1];
			return *this;
		}
		void* pleft[2];
		void* pright[2];
	};

	ASIOInterface();
	virtual ~ASIOInterface();

	void Initialize(AUDIODRIVERWORKFN pCallback, void* context);
	bool Enable(bool e);
	void Reset();
	bool Initialized() const { return _initialized; };
	bool Enabled() const { return _running; };
	void Configure();
	void RefreshAvailablePorts();
	void GetPlaybackPorts(std::vector<std::string>& ports) const;
	void GetCapturePorts(std::vector<std::string>& ports) const;
	bool AddCapturePort(uintptr_t idx);
	bool RemoveCapturePort(uintptr_t idx);
	void GetReadBuffers(uintptr_t idx, float** pleft, float** pright, int numsamples);
	uint32_t GetWritePosInSamples() const;
	uint32_t GetPlayPosInSamples();
	inline uint32_t GetInputLatencySamples() const { return _inlatency; }
	inline uint32_t GetOutputLatencySamples() const { return _outlatency; }

	DriverEnum GetDriverFromidx(uintptr_t driverID) const;
	PortOut GetOutPortFromidx(uintptr_t driverID);
	int GetidxFromOutPort(PortOut& port) const;
	void ControlPanel(uintptr_t driverID);

	static bool SupportsAsio();
	static void bufferSwitch(long index, ASIOBool processNow);
	static ASIOTime* bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow);
	static void sampleRateChanged(ASIOSampleRate sRate);
	static long asioMessages(long selector, long value, void* message, double* opt);

	HWND m_hWnd;
	static psy_AudioDriverSettings settings_;
protected:
	void Error(const char msg[]);
	bool Start();
	bool Stop();

	static AUDIODRIVERWORKFN _pCallback;
	static void* _pCallbackContext;

	ASIOCallbacks asioCallbacks;

private:
	bool _initialized;
	bool _running;
	long _inlatency;
	long _outlatency;
	std::vector<DriverEnum> drivEnum_;

	
	uintptr_t driverID;
	static AsioDrivers asioDrivers;

	
	// static ::CCriticalSection _lock;
	static PortOut _selectedout;
	static std::vector<PortCapt> _selectedins;
	std::vector<int> _portMapping;
	static AsioStereoBuffer* ASIObuffers;
	static bool _firstrun;
	static bool _supportsOutputReady;
	static uint32_t writePos;
	static uint32_t m_wrapControl;
};

AsioDrivers ASIOInterface::asioDrivers;
ASIOInterface::AsioStereoBuffer* ASIOInterface::ASIObuffers(0);
bool ASIOInterface::_firstrun(true);
bool ASIOInterface::_supportsOutputReady(false);
ASIOInterface::PortOut ASIOInterface::_selectedout;
std::vector<ASIOInterface::PortCapt> ASIOInterface::_selectedins;
uint32_t ASIOInterface::writePos(0);
uint32_t ASIOInterface::m_wrapControl(0);
psy_AudioDriverSettings ASIOInterface::settings_;

AUDIODRIVERWORKFN ASIOInterface::_pCallback(0);
void* ASIOInterface::_pCallbackContext(0);

std::string ASIOInterface::PortEnum::GetName() const
{
	std::string fullname = _info.name;
	switch (_info.type)
	{
	case ASIOSTInt16LSB:
	case ASIOSTInt32LSB16:		// 32 bit data with 16 bit alignment
	case ASIOSTInt16MSB:
	case ASIOSTInt32MSB16:		// 32 bit data with 16 bit alignment
		fullname = fullname + " : 16 bit";
		break;
	case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
	case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
		fullname = fullname + " : 18 bit";
		break;

	case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
	case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
		fullname = fullname + " : 20 bit";
		break;
	case ASIOSTInt24LSB:		// used for 20 bits as well
	case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
	case ASIOSTInt24MSB:		// used for 20 bits as well
	case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
		fullname = fullname + " : 24 bit";
		break;
	case ASIOSTInt32LSB:
	case ASIOSTInt32MSB:
		fullname = fullname + ": 32 bit";
		break;
	case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
	case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, Big Endian architecture
		fullname = fullname + ": 32 bit float";
		break;
	case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
	case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, Big Endian architecture
		fullname = fullname + ": 64 bit float";
		break;
	default:
		fullname = fullname + ": unsupported!";
		break;
	}
	return fullname;
}

ASIOInterface::ASIOInterface()
{
	_initialized = false;
	_running = false;
	_firstrun = true;
	_pCallback = 0;
	psy_audiodriversettings_init(&settings_);
	psy_audiodriversettings_setvalidbitdepth(&settings_, 32);
}

ASIOInterface::~ASIOInterface()
{
	if (_initialized) Reset();
	// ASIOExit();
	asioDrivers.removeCurrentDriver();
}

void ASIOInterface::Configure()
{
	/*CASIOConfig dlg;
	dlg.pASIO = this;
	dlg.m_driverIndex = settings_->driverID;
	dlg.m_sampleRate = settings_->samplesPerSec();
	dlg.m_bufferSize = settings_->blockFrames();
	if (dlg.DoModal() != IDOK) return;
	Enable(false);
	PortOut port = GetOutPortFromidx(dlg.m_driverIndex);
	bool supported = port.port->IsFormatSupported(port.driver, dlg.m_sampleRate);
	if (!supported) {
		Error("The Format selected is not supported. Keeping the previous configuration");
		return;
	}
	_selectedout = port;
	settings_->driverID = dlg.m_driverIndex;
	settings_->setSamplesPerSec(dlg.m_sampleRate);
	settings_->setBlockFrames(dlg.m_bufferSize);
	Enable(true);*/
}

bool ASIOInterface::Enable(bool e)
{
	return e ? Start() : Stop();
}

uint32_t ASIOInterface::GetPlayPosInSamples()
{
	if (!_running) return 0;
	return writePos - GetOutputLatencySamples();
}

uint32_t ASIOInterface::GetWritePosInSamples() const
{
	if (!_running) return 0;
	return writePos;
}

void ASIOInterface::GetPlaybackPorts(std::vector<std::string>& ports) const
{
	ports.resize(0);
	for (unsigned int j = 0; j < drivEnum_.size(); ++j) {
		for (unsigned int i = 0; i < drivEnum_[j]._portout.size(); ++i) {
			std::ostringstream stream;
			stream << drivEnum_[j]._name << " "
				<< drivEnum_[j]._portout[i].GetName();
			ports.push_back(stream.str());
		}
	}
}

void ASIOInterface::GetCapturePorts(std::vector<std::string>& ports) const
{
	ports.resize(0);
	DriverEnum* driver = _selectedout.driver;
	if (!driver) return;
	for (unsigned int i = 0; i < driver->_portin.size(); i++) ports.push_back(driver->_portin[i].GetName());
}
bool ASIOInterface::AddCapturePort(uintptr_t idx)
{
	bool isplaying = _running;
	DriverEnum* driver = _selectedout.driver;
	if (idx >= driver->_portin.size()) return false;
	if (idx < _portMapping.size() && _portMapping[idx] != -1) return true;
	if (isplaying) Stop();
	PortCapt port;
	port.driver = driver;
	port.port = &driver->_portin[idx];
	_selectedins.push_back(port);
	if (_portMapping.size() <= idx) {
		int oldsize = _portMapping.size();
		_portMapping.resize(idx + 1);
		for (uintptr_t i = oldsize; i < _portMapping.size(); i++) _portMapping[i] = -1;
	}
	_portMapping[idx] = (int)(_selectedins.size() - 1);
	if (isplaying) return Start();

	return true;
}
bool ASIOInterface::RemoveCapturePort(uintptr_t idx)
{
	bool isplaying = _running;
	uintptr_t maxSize = 0;
	std::vector<PortCapt> newports;
	DriverEnum* driver = _selectedout.driver;
	if (idx >= driver->_portin.size() ||
		idx >= _portMapping.size() || _portMapping[idx] == -1) return false;

	if (isplaying) Stop();
	for (unsigned int i = 0; i < _portMapping.size(); ++i)
	{
		if (i != idx && _portMapping[i] != -1) {
			maxSize = i + 1;
			newports.push_back(_selectedins[_portMapping[i]]);
			_portMapping[i] = (int)(newports.size() - 1);
		}
	}
	_portMapping[idx] = -1;
	if (maxSize < _portMapping.size()) _portMapping.resize(maxSize);
	_selectedins = newports;
	if (isplaying) Start();
	return true;
}
void ASIOInterface::GetReadBuffers(uintptr_t idx, float** pleft, float** pright, int numsamples)
{
	if (!_running || idx >= _portMapping.size() || _portMapping[idx] == -1)
	{
		*pleft = 0;
		*pright = 0;
		return;
	}
	int mpos = _selectedins[_portMapping[idx]].machinepos;
	*pleft = _selectedins[_portMapping[idx]].pleft + mpos;
	*pright = _selectedins[_portMapping[idx]].pright + mpos;
	_selectedins[_portMapping[idx]].machinepos += numsamples;
}
ASIOInterface::DriverEnum ASIOInterface::GetDriverFromidx(uintptr_t driverID) const
{
	int counter = 0;
	for (uintptr_t i(0); i < drivEnum_.size(); ++i)
	{
		if (driverID < counter + drivEnum_[i]._portout.size())
		{
			return drivEnum_[i];
		}
		counter += (int)(drivEnum_[i]._portout.size());
	}
	DriverEnum driver;
	return driver;
}
ASIOInterface::PortOut ASIOInterface::GetOutPortFromidx(uintptr_t driverID)
{
	PortOut port;
	int counter = 0;
	for (unsigned int i(0); i < drivEnum_.size(); ++i)
	{
		if (driverID < counter + drivEnum_[i]._portout.size())
		{
			port.driver = &drivEnum_[i];
			port.port = &drivEnum_[i]._portout[driverID - counter];
			return port;
		}
		counter += (int)(drivEnum_[i]._portout.size());
	}
	return port;
}
int ASIOInterface::GetidxFromOutPort(PortOut& port) const
{
	int counter = 0;
	for (unsigned int i(0); i < drivEnum_.size(); ++i)
	{
		if (&drivEnum_[i] == port.driver)
		{
			return counter + (port.port->_idx / 2);

		}
		counter += (int)(drivEnum_[i]._portout.size());
	}
	return 0;
}

bool ASIOInterface::SupportsAsio()
{
	char szNameBuf[MAX_ASIO_DRIVERS][33];
	char* pNameBuf[MAX_ASIO_DRIVERS];
	for (int i(0); i < MAX_ASIO_DRIVERS; ++i) pNameBuf[i] = szNameBuf[i];
	return asioDrivers.getDriverNames((char**)pNameBuf, MAX_ASIO_DRIVERS);
}

void ASIOInterface::Initialize(AUDIODRIVERWORKFN pcallback, void* context)
{
	_pCallbackContext = context;
	_pCallback = pcallback;
	_running = false;
	RefreshAvailablePorts();
	_initialized = true;
}

void ASIOInterface::RefreshAvailablePorts()
{
	bool isPlaying = _running;
	if (isPlaying) Stop();
	char szNameBuf[MAX_ASIO_DRIVERS][33];
	char* pNameBuf[MAX_ASIO_DRIVERS];
	for (int i(0); i < MAX_ASIO_DRIVERS; ++i) pNameBuf[i] = szNameBuf[i];
	int drivers = asioDrivers.getDriverNames((char**)pNameBuf, MAX_ASIO_DRIVERS);
	drivEnum_.resize(0);
	ASIODriverInfo driverInfo;
	for (int i(0); i < drivers; ++i)
	{
#if !defined ALLOW_NON_ASIO
		if (std::strcmp("ASIO DirectX Driver", szNameBuf[i]) == 0) continue;
		if (std::strcmp("ASIO DirectX Full Duplex Driver", szNameBuf[i]) == 0) continue;
		if (std::strcmp("ASIO Multimedia Driver", szNameBuf[i]) == 0) continue;
#endif
		memset(&driverInfo, 0, sizeof(ASIODriverInfo));
		driverInfo.asioVersion = ASIO_VERSION;
		driverInfo.sysRef = m_hWnd;
		if (asioDrivers.loadDriver(szNameBuf[i]))
		{
			// initialize the driver
			if (ASIOInit(&driverInfo) == ASE_OK)
			{
				DriverEnum driver(szNameBuf[i]);
				// TRACE("%s\n", szNameBuf[i]);
				long in, out;
				if (ASIOGetChannels(&in, &out) == ASE_OK)
				{
					// += 2 because we pair them in stereo
					for (int j(0); j < out; j += 2)
					{
						ASIOChannelInfo channelInfo;
						channelInfo.isInput = ASIOFalse;
						channelInfo.channel = j;
						ASIOGetChannelInfo(&channelInfo);
						PortEnum port(j, channelInfo);
						driver.AddOutPort(port);
					}
					// += 2 because we pair them in stereo
					for (int j(0); j < in; j += 2)
					{
						ASIOChannelInfo channelInfo;
						channelInfo.isInput = ASIOTrue;
						channelInfo.channel = j;
						ASIOGetChannelInfo(&channelInfo);
						PortEnum port(j, channelInfo);
						driver.AddInPort(port);
					}
				}
				ASIOGetBufferSize(&driver.minSamples, &driver.maxSamples, &driver.prefSamples, &driver.granularity);
				drivEnum_.push_back(driver);
			}
			asioDrivers.removeCurrentDriver();
		}
	}
	if (isPlaying) Start();
}

void ASIOInterface::Reset()
{
	Stop();
}

bool ASIOInterface::Start()
{
	// CSingleLock lock(&_lock, TRUE);
	_firstrun = true;
	if (_running) return true;
	if (_pCallback == 0)
	{
		_running = false;
		return false;
	}
	// BEGIN -  Code 
	asioDrivers.removeCurrentDriver();

	_selectedout = GetOutPortFromidx(driverID);
	if (!_selectedout.driver) _selectedout = GetOutPortFromidx(0);
	if (_selectedout.driver) {
		if (psy_audiodriversettings_blockframes(&settings_) < (uintptr_t) _selectedout.driver->minSamples)
			psy_audiodriversettings_setblockframes(&settings_, _selectedout.driver->prefSamples);
		else if (psy_audiodriversettings_blockframes(&settings_) > (uintptr_t) _selectedout.driver->maxSamples)
			psy_audiodriversettings_setblockframes(&settings_, _selectedout.driver->prefSamples);
	}
	if (!_selectedout.driver) {
		_running = false;
		return false;
	}
	char bla[128]; strcpy(bla, _selectedout.driver->_name.c_str());	
	if (!asioDrivers.loadDriver(bla))
	{
		_running = false;
		return false;
	}
	// initialize the driver
	ASIODriverInfo driverInfo;
	memset(&driverInfo, 0, sizeof(ASIODriverInfo));
	driverInfo.asioVersion = ASIO_VERSION;
	driverInfo.sysRef = m_hWnd;
	if (ASIOInit(&driverInfo) != ASE_OK)
	{
		//ASIOExit();
		asioDrivers.removeCurrentDriver();
		_running = false;
		return false;
	}
	if (ASIOSetSampleRate(psy_audiodriversettings_samplespersec(&settings_)) != ASE_OK)
	{
		psy_audiodriversettings_setsamplespersec(&settings_, 44100);
		if (ASIOSetSampleRate(psy_audiodriversettings_samplespersec(&settings_)) != ASE_OK)
		{
			//ASIOExit();
			asioDrivers.removeCurrentDriver();
			_running = false;
			return false;
		}
	}
	if (ASIOOutputReady() == ASE_OK) _supportsOutputReady = true;
	else _supportsOutputReady = false;
	// set up the asioCallback structure and create the ASIO data buffer
	asioCallbacks.bufferSwitch = &bufferSwitch;
	asioCallbacks.sampleRateDidChange = &sampleRateChanged;
	asioCallbacks.asioMessage = &asioMessages;
	asioCallbacks.bufferSwitchTimeInfo = &bufferSwitchTimeInfo;
	//////////////////////////////////////////////////////////////////////////
	// Create the buffers to play and record.
	int numbuffers = (int)(1 + _selectedins.size()) * 2;
	ASIOBufferInfo* info = new ASIOBufferInfo[numbuffers];
	int counter = 0;
	for (unsigned int i(0); i < _selectedins.size(); ++i)
	{
		info[counter].isInput = info[counter + 1].isInput = _selectedins[i].port->_info.isInput;
		info[counter].channelNum = _selectedins[i].port->_idx;
		info[counter + 1].channelNum = _selectedins[i].port->_idx + 1;
		info[counter].buffers[0] = info[counter].buffers[1] = info[counter + 1].buffers[0] = info[counter + 1].buffers[1] = 0;
		counter += 2;
	}
	info[counter].isInput = info[counter + 1].isInput = _selectedout.port->_info.isInput;
	info[counter].channelNum = _selectedout.port->_idx;
	info[counter + 1].channelNum = _selectedout.port->_idx + 1;
	info[counter].buffers[0] = info[counter].buffers[1] = info[counter + 1].buffers[0] = info[counter + 1].buffers[1] = 0;
	// create and activate buffers
	if (ASIOCreateBuffers(info, numbuffers, psy_audiodriversettings_blockframes(&settings_), &asioCallbacks) != ASE_OK)
	{
		//ASIOExit();
		asioDrivers.removeCurrentDriver();
		_running = false;
		return false;
	}
	ASIObuffers = new AsioStereoBuffer[_selectedins.size() + 1];
	counter = 0;
	unsigned int i(0);
	for (; i < _selectedins.size(); ++i)
	{
		AsioStereoBuffer buffer(info[counter].buffers, info[counter + 1].buffers, _selectedins[i].port->_info.type);
		ASIObuffers[i] = buffer;
		// 2* is a safety measure (Haven't been able to dig out why it crashes if it is exactly the size)
		_selectedins[i].pleft = (float*) dsp.memory_alloc(2 * psy_audiodriversettings_blockframes(&settings_), sizeof(float));
		_selectedins[i].pright = (float*) dsp.memory_alloc(2 * psy_audiodriversettings_blockframes(&settings_), sizeof(float));
		counter += 2;
	}
	AsioStereoBuffer buffer(info[counter].buffers, info[counter + 1].buffers, _selectedout.port->_info.type);
	ASIObuffers[i] = buffer;

	ASIOGetLatencies(&_inlatency, &_outlatency);
	if (ASIOStart() != ASE_OK)
	{
		ASIODisposeBuffers();
		ASIOExit();
		asioDrivers.removeCurrentDriver();
		_running = false;
		return false;
	}
	// END -  CODE
	_running = true;
	writePos = 0;
	m_wrapControl = 0;
	// PsycleGlobal::midi().ReSync(); // MIDI IMPLEMENTATION
	delete[] info;
	return true;
}

bool ASIOInterface::Stop()
{
	//CSingleLock lock(&_lock, TRUE);
	if (!_running) return true;
	_running = false;
	ASIOStop();
	ASIODisposeBuffers();
	for (unsigned int i(0); i < _selectedins.size(); ++i)
	{
		dsp.memory_dealloc(_selectedins[i].pleft);
		dsp.memory_dealloc(_selectedins[i].pright);
	}

	delete[] ASIObuffers;
	// ASIOExit();
	asioDrivers.removeCurrentDriver();
	return true;
}

void ASIOInterface::ControlPanel(uintptr_t driverID)
{
	PortOut pout = GetOutPortFromidx(driverID);
	DriverEnum* newdriver = pout.driver;
	if (_selectedout.driver == newdriver && _running)
	{
		ASIOControlPanel(); //you might want to check wether the ASIOControlPanel() can open
	}
	else
	{
		bool isPlaying = _running;
		if (isPlaying) Stop();
		// load it
		if (asioDrivers.loadDriver(const_cast<char*>(newdriver->_name.c_str())))
		{
			ASIOControlPanel(); // you might want to check wether the ASIOControlPanel() can open
			asioDrivers.removeCurrentDriver();
		}
		if (isPlaying) Start();
	}
}

#define SwapDouble(v) SwapLongLong((long long)(v))
#define SwapFloat(v) SwapLong(long(v))
#define SwapLongLong(v) ((((v)>>56)&0xFF)|(((v)>>40)&0xFF00)|(((v)>>24)&0xFF0000)|(((v)>>8)&0xFF000000) \
						       | (((v)&0xFF)<<56)|(((v)&0xFF00)<<40)|(((v)&0xFF0000)<<24)|(((v)&0xFF000000)<<8))
#define SwapLong(v) ((((v)>>24)&0xFF)|(((v)>>8)&0xFF00)|(((v)&0xFF00)<<8)|(((v)&0xFF)<<24)) 
#define SwapShort(v) ((((v)>>8)&0xFF)|(((v)&0xFF)<<8))

//ADVICE: Remember that psycle uses the range +32768.f to -32768.f. All conversions are done relative to this.
ASIOTime* ASIOInterface::bufferSwitchTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow)
{
	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization.
	if (_firstrun)
	{
		// universalis::cpu::exceptions::install_handler_in_thread();
		_firstrun = false;
	}
	writePos = timeInfo->timeInfo.samplePosition.lo;
	if (timeInfo->timeInfo.samplePosition.hi != m_wrapControl) {
		m_wrapControl = timeInfo->timeInfo.samplePosition.hi;
		// PsycleGlobal::midi().ReSync();	// MIDI IMPLEMENTATION
	}

	const unsigned int _ASIObufferSamples = psy_audiodriversettings_blockframes(&settings_);
	//////////////////////////////////////////////////////////////////////////
	// psy_audio_Inputs
	unsigned int counter(0);
	for (; counter < _selectedins.size(); ++counter)
	{
		ASIOSampleType dtype = _selectedins[counter].port->_info.type;
		uintptr_t i(0);
		switch (dtype)
		{
		case ASIOSTInt16LSB:
		{
			short* inl;
			short* inr;
			inl = (short*)ASIObuffers[counter].pleft[index];
			inr = (short*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
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
				_selectedins[counter].pleft[i] = t * 0.00390625f;

				pt[0] = *inr++;
				pt[1] = *inr++;
				pt[2] = *inr++;
				_selectedins[counter].pright[i] = t * 0.00390625f;

			}
		}
		break;
		case ASIOSTInt32LSB:
		{
			long* inl;
			long* inr;
			inl = (long*)ASIObuffers[counter].pleft[index];
			inr = (long*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				_selectedins[counter].pleft[i] = (float)((*inl) * 0.0000152587890625);
				_selectedins[counter].pright[i] = (float)((*inr) * 0.0000152587890625);
			}
		}
		break;
		case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
		{
			dsp.movmul(static_cast<float*>(ASIObuffers[counter].pleft[index]), _selectedins[counter].pleft, _ASIObufferSamples, 32768.0f);
			dsp.movmul(static_cast<float*>(ASIObuffers[counter].pright[index]), _selectedins[counter].pright, _ASIObufferSamples, 32768.0f);
		}
		break;
		case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
		{
			double* inl;
			double* inr;
			inl = (double*)ASIObuffers[counter].pleft[index];
			inr = (double*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				_selectedins[counter].pleft[i] = (float)((*inl) * 32768.0);
				_selectedins[counter].pright[i] = (float)((*inr) * 32768.0);
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
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				_selectedins[counter].pleft[i] = (float) *inl;
				_selectedins[counter].pright[i] = (float) *inr;
			}
		}
		break;
		case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment (right aligned)
		{
			long* inl;
			long* inr;
			inl = (long*)ASIObuffers[counter].pleft[index];
			inr = (long*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				_selectedins[counter].pleft[i] = (*inl) * 0.25f;
				_selectedins[counter].pright[i] = (*inr) * 0.25f;
			}
		}
		break;
		case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment (right aligned)
		{
			long* inl;
			long* inr;
			inl = (long*)ASIObuffers[counter].pleft[index];
			inr = (long*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				_selectedins[counter].pleft[i] = (*inl) * 0.0625f;
				_selectedins[counter].pright[i] = (*inr) * 0.0625f;
			}
		}
		break;
		case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment (right aligned)
		{
			long* inl;
			long* inr;
			inl = (long*)ASIObuffers[counter].pleft[index];
			inr = (long*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				_selectedins[counter].pleft[i] = (*inl) * 0.00390625f;
				_selectedins[counter].pright[i] = (*inr) * 0.00390625f;
			}
		}
		break;
		case ASIOSTInt16MSB:
		{
			short* inl;
			short* inr;
			inl = (short*)ASIObuffers[counter].pleft[index];
			inr = (short*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				_selectedins[counter].pleft[i] = (float) SwapShort(*inl);
				_selectedins[counter].pright[i] = (float) SwapShort(*inr);
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
				_selectedins[counter].pleft[i] = t * 0.00390625f;

				pt[2] = *inr++;
				pt[1] = *inr++;
				pt[0] = *inr++;
				_selectedins[counter].pright[i] = t * 0.00390625f;

			}
		}
		break;
		case ASIOSTInt32MSB:
		{
			long* inl;
			long* inr;
			inl = (long*)ASIObuffers[counter].pleft[index];
			inr = (long*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				long val = SwapLong(*inl++);
				_selectedins[counter].pleft[i] = (float)(val * 0.0000152587890625);
				val = SwapLong(*inr++);
				_selectedins[counter].pright[i] = (float)(val * 0.0000152587890625);
			}
		}
		break;
		case ASIOSTInt32MSB16:		// 32 bit data with 16 bit alignment (right aligned)
		{
			long* inl;
			long* inr;
			inl = (long*)ASIObuffers[counter].pleft[index];
			inr = (long*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				_selectedins[counter].pleft[i] = (float) SwapLong(*inl);
				_selectedins[counter].pright[i] = (float) SwapLong(*inr);
			}
		}
		break;
		case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment (right aligned)
		{
			long* inl;
			long* inr;
			inl = (long*)ASIObuffers[counter].pleft[index];
			inr = (long*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				long val = SwapLong(*inl);
				_selectedins[counter].pleft[i] = val * 0.25f;
				val = SwapLong(*inr);
				_selectedins[counter].pright[i] = val * 0.25f;
			}
		}
		break;
		case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment (right aligned)
		{
			long* inl;
			long* inr;
			inl = (long*)ASIObuffers[counter].pleft[index];
			inr = (long*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				long val = SwapLong(*inl);
				_selectedins[counter].pleft[i] = val * 0.0625f;
				val = SwapLong(*inr);
				_selectedins[counter].pright[i] = val * 0.0625f;
			}
		}
		break;
		case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment (right aligned)
		{
			long* inl;
			long* inr;
			inl = (long*)ASIObuffers[counter].pleft[index];
			inr = (long*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				long val = SwapLong(*inl);
				_selectedins[counter].pleft[i] = val * 0.00390625f;
				val = SwapLong(*inr);
				_selectedins[counter].pright[i] = val * 0.00390625f;
			}
		}
		break;
		case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on PowerPC implementation
		{
			float* inl;
			float* inr;
			inl = (float*)ASIObuffers[counter].pleft[index];
			inr = (float*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				_selectedins[counter].pleft[i] = (float) SwapFloat(*inl);
				_selectedins[counter].pright[i] = (float) SwapFloat(*inr);
			}
		}
		break;
		case ASIOSTFloat64MSB: 		// IEEE 754 64 bit float, as found on PowerPC implementation
		{
			double* inl;
			double* inr;
			inl = (double*)ASIObuffers[counter].pleft[index];
			inr = (double*)ASIObuffers[counter].pright[index];
			for (i = 0; i < _ASIObufferSamples; i++, inl++, inr++)
			{
				_selectedins[counter].pleft[i] = (float) SwapDouble(*inl);
				_selectedins[counter].pright[i] = (float) SwapDouble(*inr);
			}
		}
		break;
		}
		_selectedins[counter].machinepos = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// Outputs
	int numSamples;
	numSamples = _ASIObufferSamples;
	int hostisplaying;

	float* pBuf =
		_pCallback(
			_pCallbackContext, &numSamples, &hostisplaying);

	switch (_selectedout.port->_info.type)
	{
	case ASIOSTInt16LSB:
	{
		int16_t* outl;
		int16_t* outr;
		outl = (int16_t*)ASIObuffers[counter].pleft[index];
		outr = (int16_t*)ASIObuffers[counter].pright[index];
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			*outl++ = psy_dsp_rint16_clip(*pBuf++);
			*outr++ = psy_dsp_rint16_clip(*pBuf++);
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
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			t = psy_dsp_rint32_clip_bits((*pBuf++) * 256.0f, 24);
			*outl++ = pt[0];
			*outl++ = pt[1];
			*outl++ = pt[2];

			t = psy_dsp_rint32_clip_bits((*pBuf++) * 256.0f, 24);
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
		int const maxv((1u << ((sizeof(int32_t) << 3) - 1)) - 100);
		int const minv(-maxv - 1);
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			//*outl++ = psy_dsp_rint32_float(psy_dsp_clip((float) minv, (*pBuf++) * 65536.0f, (float) maxv));
			//*outr++ = psy_dsp_rint32_float(psy_dsp_clip((float) minv, (*pBuf++) * 65536.0f, (float) maxv));
			*outl++ = psy_dsp_rint((*pBuf++) * 65536.0f);
			*outr++ = psy_dsp_rint((*pBuf++) * 65536.0f);
		}
	}
	break;
	case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
	{
		float* outl;
		float* outr;
		outl = (float*)ASIObuffers[counter].pleft[index];
		outr = (float*)ASIObuffers[counter].pright[index];
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
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
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
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
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			*outl++ = (int32_t)(*pBuf++);  //  rint_clip<int32_t, 16>
			*outr++ = (int32_t)(*pBuf++);  //  rint_clip<int32_t, 16>
		}
	}
	break;
	case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment (right aligned)
	{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			*outl++ = (int32_t)((*pBuf++) * 4.0f); // rint_clip<int32_t, 18>
			*outr++ = (int32_t)((*pBuf++) * 4.0f); // rint_clip<int32_t, 18>
		}
	}
	break;
	case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment (right aligned)
	{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			*outl++ = (int32_t)((*pBuf++) * 16.0f); // rint_clip<int32_t, 20>
			*outr++ = (int32_t)((*pBuf++) * 16.0f); //  rint_clip<int32_t, 20>
		}
	}
	break;
	case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment (right aligned)
	{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			*outl++ = (int32_t)((*pBuf++) * 256.0f); // rint_clip<int32_t, 24>
			*outr++ = (int32_t)((*pBuf++) * 256.0f); // rint_clip<int32_t, 24>
		}
	}
	break;
	case ASIOSTInt16MSB:
	{
		int16_t* outl;
		int16_t* outr;
		outl = (int16_t*)ASIObuffers[counter].pleft[index];
		outr = (int16_t*)ASIObuffers[counter].pright[index];
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			*outl++ = SwapShort(psy_dsp_rint16_clip(*pBuf++));
			*outr++ = SwapShort(psy_dsp_rint16_clip(*pBuf++));
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
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			t = (int)((*pBuf++) * 256.0f); // rint_clip<int, 24>
			*outl++ = pt[2];
			*outl++ = pt[1];
			*outl++ = pt[0];

			t = (int)((*pBuf++) * 256.0f);
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
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			//See the LSB case above.
			int const maxv((1u << ((sizeof(int32_t) << 3) - 1)) - 100);
			int const minv(-maxv - 1);
			// psycle::helpers::math::rint<int32_t>(psycle::helpers::math::clip
			*outl++ = SwapLong((int32_t)((float(minv), (*pBuf++) * 65536.0f, float(maxv))));
			*outr++ = SwapLong((int32_t)((float(minv), (*pBuf++) * 65536.0f, float(maxv))));
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
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			// rint_clip<int32_t, 16>
			*outl++ = SwapLong(((int32_t)(*pBuf++)));
			*outr++ = SwapLong(((int32_t)(*pBuf++)));
		}
	}
	break;
	case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment (right aligned)
	{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			// rint_clip<int32_t, 18
			*outl++ = SwapLong((int32_t)((*pBuf++) * 4.0f));
			*outr++ = SwapLong((int32_t)((*pBuf++) * 4.0f));
		}
	}
	break;
	case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment (right aligned)
	{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			//rint_clip<int32_t, 20>
			*outl++ = SwapLong((int32_t)((*pBuf++) * 16.0f));
			*outr++ = SwapLong((int32_t)((*pBuf++) * 16.0f));
		}
	}
	break;
	case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment (right aligned)
	{
		int32_t* outl;
		int32_t* outr;
		outl = (int32_t*)ASIObuffers[counter].pleft[index];
		outr = (int32_t*)ASIObuffers[counter].pright[index];
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			// rint_clip<int32_t, 24>
			*outl++ = SwapLong((int32_t)((*pBuf++) * 256.0f));
			*outr++ = SwapLong((int32_t)((*pBuf++) * 256.0f));
		}
	}
	break;
	case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on PowerPC implementation
	{
		float* outl;
		float* outr;
		outl = (float*)ASIObuffers[counter].pleft[index];
		outr = (float*)ASIObuffers[counter].pright[index];
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			*outl++ = (float) SwapFloat((*pBuf++) * 0.00030517578125);
			*outr++ = (float) SwapFloat((*pBuf++) * 0.00030517578125);
		}
	}
	break;
	case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on PowerPC implementation
	{
		double* outl;
		double* outr;
		outl = (double*)ASIObuffers[counter].pleft[index];
		outr = (double*)ASIObuffers[counter].pright[index];
		for (uintptr_t i = 0; i < _ASIObufferSamples; ++i) {
			*outl++ = SwapDouble((*pBuf++) * 0.00030517578125);
			*outr++ = SwapDouble((*pBuf++) * 0.00030517578125);
		}
	}
	break;
	}
	// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
	if (_supportsOutputReady) ASIOOutputReady();
	return 0;
}

void ASIOInterface::bufferSwitch(long index, ASIOBool processNow)
{
	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.

	// as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
	// though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
	ASIOTime  timeInfo;
	std::memset(&timeInfo, 0, sizeof timeInfo);

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if (ASIOGetSamplePosition(&timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK)
		timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;

	bufferSwitchTimeInfo(&timeInfo, index, processNow);
}

void ASIOInterface::sampleRateChanged(ASIOSampleRate sRate)
{
	// do whatever you need to do if the sample rate changed
	// usually this only happens during external sync.
	// Audio processing is not stopped by the driver, actual sample rate
	// might not have even changed, maybe only the sample rate status of an
	// AES/EBU or S/PDIF digital input at the audio device.
	// You might have to update time/sample related conversion routines, etc.
	psy_audiodriversettings_setsamplespersec(&settings_, sRate);
}

long ASIOInterface::asioMessages(long selector, long value, void* message, double* opt)
{
	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;
	switch (selector)
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

typedef struct {
	psy_AudioDriver driver;
	// psy_AudioDriverSettings settings;
	psy_Property* configuration;
	HWND m_hWnd;
	int _dither;
	unsigned int pollSleep_;

	int _initialized;
	int _configured;
	// controls if the driver is supposed to be running or not
	int _running;
	// informs the real state of the DSound buffer (see the control of buffer play in DoBlocks())
	int _playing;
	// Controls if we want the thread to be running or not
	int _threadRun;
	
	psy_List* _playEnums;
	psy_List* _capEnums;
	psy_List* _capPorts;
	psy_Table _portMapping; // <int, int>
	
	HANDLE hEvent;
	int (*error)(int, const char*);
	ASIOInterface* asioif;
} AsioDriver;

static int driver_init(psy_AudioDriver*);
static int driver_open(psy_AudioDriver*);
static int driver_close(psy_AudioDriver*);
static int driver_dispose(psy_AudioDriver*);
static void driver_configure(psy_AudioDriver*, const psy_Property*);
static const psy_Property* driver_configuration(const psy_AudioDriver*);
static psy_dsp_big_hz_t driver_samplerate(psy_AudioDriver*);
static const char* capturename(psy_AudioDriver*, int index);
static int numcaptures(psy_AudioDriver*);
static const char* playbackname(psy_AudioDriver*, int index);
static int numplaybacks(psy_AudioDriver*);
static int addcaptureport(AsioDriver* self, int idx);
static int removecaptureport(AsioDriver* self, int idx);
static bool start(AsioDriver*);
static bool stop(AsioDriver*);
static void driver_deallocate(psy_AudioDriver*);
static void init_properties(psy_AudioDriver*);
static void readbuffers(AsioDriver* self, int idx,
	float** left, float** right, uintptr_t numsamples);
static const psy_AudioDriverInfo* driver_info(psy_AudioDriver*);
static uintptr_t playposinsamples(psy_AudioDriver*);

static psy_AudioDriverVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.open = driver_open;
		vtable.deallocate = driver_deallocate;
		vtable.open = driver_open;
		vtable.close = driver_close;
		vtable.dispose = driver_dispose;
		vtable.configure = driver_configure;
		vtable.configuration = driver_configuration;
		vtable.samplerate = driver_samplerate;
		vtable.addcapture = (psy_audiodriver_fp_addcapture)addcaptureport;
		vtable.removecapture = (psy_audiodriver_fp_removecapture)removecaptureport;
		vtable.readbuffers = (psy_audiodriver_fp_readbuffers)readbuffers;
		vtable.capturename = (psy_audiodriver_fp_capturename)capturename;
		vtable.numcaptures = (psy_audiodriver_fp_numcaptures)numcaptures;
		vtable.playbackname = (psy_audiodriver_fp_playbackname)playbackname;
		vtable.numplaybacks = (psy_audiodriver_fp_numplaybacks)numplaybacks;
		vtable.playposinsamples = playposinsamples;
		vtable.info = (psy_audiodriver_fp_info)driver_info;
		vtable_initialized = 1;
	}
}

int on_error(int err, const char* msg)
{
	MessageBox(0, msg, "Windows WaveOut MME driver", MB_OK | MB_ICONERROR);
	return 0;
}

EXPORT psy_AudioDriverInfo const* __cdecl GetPsycleDriverInfo(void)
{
	static psy_AudioDriverInfo info;

	info.guid = PSY_AUDIODRIVER_ASIO_GUID;
	info.Flags = 0;
	info.Name = "Asio Audio Driver";
	info.ShortName = "Asio";
	info.Version = 0;
	return &info;
}

EXPORT psy_AudioDriver* __cdecl driver_create(void)
{
	AsioDriver* asio;

	asio = (AsioDriver*)malloc(sizeof(AsioDriver));
	if (asio != NULL) {		
		driver_init(&asio->driver);
		return &asio->driver;
	}
	return NULL;
}

void driver_deallocate(psy_AudioDriver* driver)
{
	driver_dispose(driver);
	free(driver);
}

int driver_init(psy_AudioDriver* driver)
{
	AsioDriver* self = (AsioDriver*) driver;

	memset(self, 0, sizeof(AsioDriver));
	vtable_init();
	self->driver.vtable = &vtable;
	self->error = on_error;
#ifdef PSYCLE_USE_SSE
	psy_dsp_sse2_init(&dsp);
#else
	psy_dsp_noopt_init(&dsp);
#endif
	SetupAVRT();
	self->asioif = new ASIOInterface();
	init_properties(&self->driver);
	return 0;
}

int driver_dispose(psy_AudioDriver* driver)
{
	AsioDriver* self = (AsioDriver*)driver;
	delete self->asioif;
	self->asioif = 0;
	psy_property_deallocate(self->configuration);
	self->configuration = NULL;
	CloseAVRT();
	return 0;
}

void init_properties(psy_AudioDriver* driver)
{
	AsioDriver* self = (AsioDriver*)driver;
	char key[256];
	psy_Property* property;
	psy_Property* devices;
	psy_Property* indevices;

	psy_snprintf(key, 256, "asio-guid-%d", PSY_AUDIODRIVER_ASIO_GUID);
	self->configuration = psy_property_preventtranslate(psy_property_settext(
		psy_property_allocinit_key(key), "Asio 2_2"));
	psy_property_sethint(psy_property_append_int(self->configuration,
		"guid", PSY_AUDIODRIVER_ASIO_GUID, 0, 0),
		PSY_PROPERTY_HINT_HIDE);
	psy_property_settext(
		psy_property_setreadonly(
			psy_property_append_str(self->configuration, "name", "Asio 2_2 Driver"),
			TRUE),
		"Name");
	psy_property_setreadonly(
		psy_property_append_str(self->configuration, "vendor", "Psycledelics"),
		TRUE);
	psy_property_setreadonly(
		psy_property_append_str(self->configuration, "version", "1.0"),
		TRUE);
	property = psy_property_append_choice(self->configuration, "device", -1);
	psy_property_append_int(self->configuration, "bitdepth",
		psy_audiodriversettings_bitdepth(&ASIOInterface::settings_), 0, 32);
	psy_property_append_int(self->configuration, "samplerate",
		(intptr_t)psy_audiodriversettings_samplespersec(
			&ASIOInterface::settings_), 0, 0);
	psy_property_append_int(self->configuration, "dither", 0, 0, 1);
	psy_property_settext(
		psy_property_append_int(self->configuration, "numbuf",
			psy_audiodriversettings_blockcount(&ASIOInterface::settings_), 1, 8),
		"Buffer Number");
	psy_property_settext(
		psy_property_append_int(self->configuration, "numsamples",
			psy_audiodriversettings_blockframes(&ASIOInterface::settings_),
			64, 8193),
		"Buffer Samples");
	devices = psy_property_settext(
		psy_property_append_choice(self->configuration, "device", 0),
		"Output Device");
	indevices = psy_property_settext(
		psy_property_append_choice(self->configuration, "indevice", 0),
		"Standard Input Device(Select different in Recorder)");
}

void driver_configure(psy_AudioDriver* driver, const psy_Property* config)
{
	AsioDriver* self;
	psy_Property* property;

	self = (AsioDriver*)driver;
	if (config) {
		psy_property_sync(self->configuration, config);
	}
	property = psy_property_at(self->configuration, "bitdepth",
		PSY_PROPERTY_TYPE_INTEGER);
	if (property) {
		psy_audiodriversettings_setvalidbitdepth(&ASIOInterface::settings_,
			psy_property_item_int(property));
	}
	property = psy_property_at(self->configuration, "samplerate",
		PSY_PROPERTY_TYPE_INTEGER);
	if (property) {
		psy_audiodriversettings_setsamplespersec(&ASIOInterface::settings_,
			(psy_dsp_big_hz_t)psy_property_item_int(property));
	}
	property = psy_property_at(self->configuration, "numbuf",
		PSY_PROPERTY_TYPE_INTEGER);
	if (property) {
		psy_audiodriversettings_setblockcount(&ASIOInterface::settings_,
			psy_property_item_int(property));
	}
	property = psy_property_at(self->configuration, "numsamples",
		PSY_PROPERTY_TYPE_INTEGER);
	if (property) {
		psy_audiodriversettings_setblockframes(&ASIOInterface::settings_,
			psy_property_item_int(property));
	}	
}

psy_dsp_big_hz_t driver_samplerate(psy_AudioDriver* self)
{
	return psy_audiodriversettings_samplespersec(&ASIOInterface::settings_);
}

int driver_open(psy_AudioDriver* driver)
{
	int status;
	AsioDriver* self = (AsioDriver*)driver;
	self->asioif->m_hWnd = (HWND)driver->handle;
	self->asioif->Initialize(driver->callback, driver->callbackcontext);
	status = self->asioif->Enable(true);
	return status;
}

int driver_close(psy_AudioDriver* driver)
{
	int status;
	AsioDriver* self = (AsioDriver*)driver;
	status = self->asioif->Enable(false);
	return status;
}

void readbuffers(AsioDriver* self, int idx, float** left, float** right,
	uintptr_t numsamples)
{
	self->asioif->GetReadBuffers(idx, left, right, (int)numsamples);
}

const char* capturename(psy_AudioDriver* driver, int index)
{
	AsioDriver* self = (AsioDriver*)driver;
	
	std::vector<std::string> ports;
	self->asioif->GetCapturePorts(ports);
	return ports[index].c_str();
}

int numcaptures(psy_AudioDriver* driver)
{
	AsioDriver* self = (AsioDriver*)driver;
	std::vector<std::string> ports;
	self->asioif->GetCapturePorts(ports);
	return ports.size();
}

const char* playbackname(psy_AudioDriver* driver, int index)
{
	AsioDriver* self = (AsioDriver*)driver;

	std::vector<std::string> ports;
	self->asioif->GetPlaybackPorts(ports);
	return ports[index].c_str();
}

int numplaybacks(psy_AudioDriver* driver)
{
	AsioDriver* self = (AsioDriver*)driver;
	std::vector<std::string> ports;
	self->asioif->GetPlaybackPorts(ports);
	return ports.size();
}

int removecaptureport(AsioDriver* self, int idx)
{
	return self->asioif->RemoveCapturePort(idx);
}

int addcaptureport(AsioDriver* self, int idx)
{
	return self->asioif->AddCapturePort(idx);
}

const psy_AudioDriverInfo* driver_info(psy_AudioDriver* self)
{
	return GetPsycleDriverInfo();
}

const psy_Property* driver_configuration(const psy_AudioDriver* driver)
{
	AsioDriver* self = (AsioDriver*)driver;

	return self->configuration;
}

uintptr_t playposinsamples(psy_AudioDriver* driver)
{
	AsioDriver* self = (AsioDriver*)driver;

	return self->asioif->GetPlayPosInSamples();
}
