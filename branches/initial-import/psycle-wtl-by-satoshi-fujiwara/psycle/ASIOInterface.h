#pragma once
/** @file
 *  @brief header file
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.5 $
 */


#include "AudioDriver.h"
#include "ASIO\\asiodrivers.h"
#include "ASIO\\asio.h"



class ASIOInterface : public AudioDriver
{
public:
	static const UINT NAME_LEN = 32;
	static const UINT FULLNAME_LEN = 160;
	static const UINT MAX_ASIO_DRIVERS = 32;
	static const UINT MAX_ASIO_OUTPUTS = 128;

	explicit ASIOInterface(iController *pController);

	~ASIOInterface(void);
	void Initialize(
		const HWND hwnd,
		const AUDIODRIVERWORKFN pCallback,
		const void* context);
	void Configure();
	const bool IsInitialized() const { return _initialized; };
	const bool IsConfigured() const { return _configured; };
	const int BufferSize() const;
	AudioDriverInfo* const GetInfo(){ return &_info; };
	void Reset();
	const bool Enable(const bool e);
	const int WritePos() const;
	const int PlayPos() const;
	
	void ControlPanel(const int driverID);

private:

	int drivercount;
	TCHAR szFullName[MAX_ASIO_OUTPUTS][FULLNAME_LEN];

// callback prototypes

	int driverindex[MAX_ASIO_OUTPUTS];
	int outputindex[MAX_ASIO_OUTPUTS];

	long minSamples[MAX_ASIO_DRIVERS];
	long maxSamples[MAX_ASIO_DRIVERS];
	long prefSamples[MAX_ASIO_DRIVERS];
	long Granularity[MAX_ASIO_DRIVERS];

	int currentSamples[MAX_ASIO_DRIVERS];


	void Error(TCHAR const *msg);
	void ReadConfig();
	void WriteConfig();
	
	const bool Start();
	
	const bool Stop();

	static AudioDriverInfo		_info;
	static ATL::CComAutoCriticalSection		_lock;

	bool	_initialized;
	bool	_configured;
	bool	_running;

	int		_driverID;

	TCHAR szNameBuf[MAX_ASIO_DRIVERS][NAME_LEN];
	
	AsioDrivers asioDrivers;
};

