#include "stdafx.h"
#include "PortAudioASIO.h"
#include "resource.h"
#include "Registry.h"
#include "PortAudioASIOConfig.h"
#include "Configuration.h"
#include "MidiInput.h"

AudioDriverInfo PortAudioASIO::_info = { "PortAudio ASIO Driver" };
CCriticalSection PortAudioASIO::_lock;

PortAudioASIO::PortAudioASIO(void)
{
	_initialized = false;
	_configured = false;
	_running = false;
	_pCallback = NULL;
}

PortAudioASIO::~PortAudioASIO(void)
{
	if (_initialized) Reset();
}

void PortAudioASIO::Error(char const *msg)
{
	MessageBox(NULL, msg, "PortAudio ASIO Driver", MB_ICONWARNING);
}

void PortAudioASIO::Initialize(
	HWND hwnd,
	AUDIODRIVERWORKFN pCallback,
	void* context)
{
	_callbackContext = context;
	_pCallback = pCallback;
	_running = false;
	ReadConfig();
	_initialized = true;

}

void PortAudioASIO::ReadConfig()
{
	bool configured;
	DWORD type;
	DWORD numData;
	Registry reg;

	// Default configuration
	//
	_samplesPerSec=44100;
	_deviceID=0;
	_numBlocks = 2;
	_blockSize = 512;
	_dither = 0;
	_channelmode = 3;
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
	numData = sizeof(_numBlocks);
	configured &= (reg.QueryValue("NumBlocks", &type, (BYTE*)&_numBlocks, &numData) == ERROR_SUCCESS);
	numData = sizeof(_blockSize);
	configured &= (reg.QueryValue("BlockSize", &type, (BYTE*)&_blockSize, &numData) == ERROR_SUCCESS);
	numData = sizeof(_deviceID);
	configured &= (reg.QueryValue("DeviceID", &type, (BYTE*)&_deviceID, &numData) == ERROR_SUCCESS);
	numData = sizeof(_dither);
	configured &= (reg.QueryValue("Dither", &type, (BYTE*)&_dither, &numData) == ERROR_SUCCESS);
	numData = sizeof(_samplesPerSec);
	configured &= (reg.QueryValue("SamplesPerSec", &type, (BYTE*)&_samplesPerSec, &numData) == ERROR_SUCCESS);
	reg.CloseKey();
	reg.CloseRootKey();
	_configured = configured;
}

void PortAudioASIO::WriteConfig()
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
	reg.SetValue("NumBlocks", REG_DWORD, (BYTE*)&_numBlocks, sizeof(_numBlocks));
	reg.SetValue("BlockSize", REG_DWORD, (BYTE*)&_blockSize, sizeof(_blockSize));
	reg.SetValue("DeviceID", REG_DWORD, (BYTE*)&_deviceID, sizeof(_deviceID));
	reg.SetValue("Dither", REG_DWORD, (BYTE*)&_dither, sizeof(_dither));
	reg.SetValue("SamplesPerSec", REG_DWORD, (BYTE*)&_samplesPerSec, sizeof(_samplesPerSec));
	reg.CloseKey();
	reg.CloseRootKey();
}

void PortAudioASIO::Configure()
{
	ReadConfig();

	CPortAudioASIOConfig dlg;
	dlg.m_numBuffers = _numBlocks;
	dlg.m_bufferSize = _blockSize;
	dlg.m_deviceIndex = _deviceID;
	dlg.m_dither = _dither;
	dlg.m_sampleRate = _samplesPerSec;

	if (dlg.DoModal() != IDOK)
	{
		return;
	}
	
	int oldnb = _numBlocks;
	int oldbs = _blockSize;
	int olddid = _deviceID;
	int olddither = _dither;
	int oldsps = _samplesPerSec;

	if (_initialized)
	{
		Stop();
	}

	_numBlocks = dlg.m_numBuffers;
	_blockSize = dlg.m_bufferSize;
	_deviceID = dlg.m_deviceIndex;
	_dither = dlg.m_dither;
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
			_numBlocks = oldnb;
			_blockSize = oldbs;
			_deviceID = olddid;
			_dither = olddither;
			_samplesPerSec = oldsps;

			Start();
		}
	}
	else
	{
		WriteConfig();
	}
	
}

bool PortAudioASIO::Start()
{
	CSingleLock lock(&_lock, TRUE);
	if (_running)
	{
		return true;
	}

	if (_pCallback == NULL)
	{
		return false;
	}

	// BEGIN - PortAudio Code 
	err = Pa_Initialize();
	if( err != paNoError )
		Error("ASIO failed initialize");
	
	err = Pa_OpenStream(
			&stream,								/* passes back stream pointer */
			paNoDevice,								/* input device */
			0,										/* no input channels */
			paFloat32,								/* input sample format */
			NULL,									/* input driver info */
			_deviceID,								/* output device */
			2,										/* no output channels */
			paFloat32,								/* output sample format */
			NULL,									/* output driver info */
			_samplesPerSec,							/* sample rate */
            _blockSize,								/* frames per buffer */
			_numBlocks,								/* number of buffers */
			(_dither ? paNoFlag : paDitherOff),		/* stream flags */
            paCallback,								/* specify our custom callback */
            this);									/* pass our data through to callback */
	if( err != paNoError )
		Error("ASIO failed open stream");
	
	err = Pa_StartStream( stream );
	if( err != paNoError )
		Error("ASIO failed start stream");
	// END - PortAudio CODE

	_running = true;
	CMidiInput::Instance()->ReSync();	// MIDI IMPLEMENTATION
	return true;
}

bool PortAudioASIO::Stop()
{
	CSingleLock lock(&_lock, TRUE);
	if (!_running)
	{
		return true;
	}

	// BEGIN - PortAudio Code
	err = Pa_StopStream( stream );
	if( err != paNoError )
		Error("ASIO failed stop stream");

	err = Pa_CloseStream( stream );
		if( err != paNoError )
			Error("ASIO failed close stream");

	err = Pa_Terminate();
	// END - PortAudio Code

	_running = false;
	return true;
}

void PortAudioASIO::Reset()
{
	if (_running)Stop();
}

bool PortAudioASIO::Enable(bool e)
{
	return e ? Start() : Stop();
}

int PortAudioASIO::GetWritePos()
{
	// Not yet implemted
	if (!_running)
	{
		return 0;
	}

	
	return GetPlayPos();


}

int PortAudioASIO::GetPlayPos()
{
	// Not yet implemted
	if (!_running)
	{
		return 0;
	}
	int playPos = int(Pa_StreamTime(stream));

	return playPos;
}

int PortAudioASIO::paCallback(void *inputBuffer, void *outputBuffer,
                     unsigned long framesPerBuffer,
                     PaTimestamp outTime, void *userData )
{
	PortAudioASIO *pThis = (PortAudioASIO *) userData;

	int	nr_of_samples = framesPerBuffer;
	
	float *out = (float *) outputBuffer;

	do
	{
		int n = nr_of_samples;

		float *pBuf = pThis->_pCallback(pThis->_callbackContext, n);

		for(int i=0; i < nr_of_samples; i++ )
		{
			*out++ = (float) ((float)*pBuf++/(float)32768);
			*out++ = (float) ((float)*pBuf++/(float)32768);
		}

		nr_of_samples -= n;
	}
	while(nr_of_samples > 0);

	return 0;
}




