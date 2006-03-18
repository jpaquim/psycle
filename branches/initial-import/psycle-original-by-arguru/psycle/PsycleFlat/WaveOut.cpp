// waveout stuff based on buzz code
#include "mmsystem.h"
#include "AudioDriver.h"
#include <process.h>

#define MAX_WAVEOUT_BLOCKS		8
#define BYTES_PER_SAMPLE		4	// 2 * 16bits

class CBlock
{
public:
	HANDLE Handle;
	byte *pData;
	WAVEHDR *pHeader;
	HANDLE HeaderHandle;
	bool Prepared;
};

class ad : public CAudioDriver
{
public:
	ad();
	virtual ~ad();
	virtual void Initialize(dword hwnd, float *(*pcallback)(int &numsamples));
	virtual void Reset();
	virtual bool Enable(bool e);	
	virtual int GetWritePos();
	virtual int GetPlayPos();
	virtual void Configure();

	void DoBlocks();

	bool Start();	
	bool Stop();

	private:

	void ReadConfig();
	void WriteConfig();
	void Error(char const *msg);

public:
	HWAVEOUT Handle;
	int DeviceID;
	int numBlocks;
	int BlockSize;
	int CurrentBlock;
	int WritePos;
	int PollSleep;
	int Dither;
	CBlock Blocks[MAX_WAVEOUT_BLOCKS];
	float *(*pCallback)(int &numsamples);
	bool Initialized;
	bool Running;
	bool StopPolling;

};

static void __cdecl PollerThread(void *poo)
{
	ad *pad = (ad *)poo;

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

	pad->StopPolling = false;
	
	while(!pad->StopPolling)
	{
		pad->DoBlocks();
		Sleep(pad->PollSleep);
	}

	pad->StopPolling = false;

	_endthread();
}

ad::ad()
{
	Initialized = false;
}


ad::~ad()
{
	Reset();
}

void ad::Error(char const *msg)
{
	MessageBox(NULL, msg, "WaveOut driver", MB_OK);
}

bool ad::Start()
{
	if (Running)
		return true;

	WAVEFORMATEX format;
	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = (Flags & ADF_STEREO) ? 2 : 1;
	format.wBitsPerSample = 16;
	format.nSamplesPerSec = SamplesPerSec;
	format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
	format.cbSize = 0;

	if (waveOutOpen(&Handle, DeviceID, &format, NULL, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
	{
		Error("waveOutOpen() failed");
		return false;
	}

	CurrentBlock = 0;
	WritePos = 0;

	// allocate blocks
	for (CBlock *pBlock = Blocks; pBlock < Blocks + numBlocks; pBlock++)
	{
		pBlock->Handle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, BlockSize);
		pBlock->pData = (byte *)GlobalLock(pBlock->Handle);
	}

	// allocate block headers
	for (pBlock = Blocks; pBlock < Blocks + numBlocks; pBlock++)
	{
		pBlock->HeaderHandle = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, sizeof(WAVEHDR));
		pBlock->pHeader = (WAVEHDR *)GlobalLock(pBlock->HeaderHandle);

		WAVEHDR *ph = pBlock->pHeader;
		ph->lpData = (char *)pBlock->pData;
		ph->dwBufferLength = BlockSize;
		ph->dwFlags = WHDR_DONE;
		ph->dwLoops = 0;

		pBlock->Prepared = false;
	}
	
	HANDLE h = (HANDLE)_beginthread(PollerThread, 0, this);

	/*
	BOOL ret = SetThreadPriority(h, THREAD_PRIORITY_ABOVE_NORMAL);
	assert(ret);
*/	

	Running = true;
	return true;
}

bool ad::Stop()
{
	if (!Running)
		return true;

	StopPolling = true;
	while(StopPolling)
		Sleep(PollSleep);

	if (waveOutReset(Handle) != MMSYSERR_NOERROR)
	{
		Error("waveOutReset() failed");
		return false;
	}

	while(1)
	{
		bool alldone = true;

		for (CBlock *pBlock = Blocks; pBlock < Blocks + numBlocks; pBlock++)
		{
			if ((pBlock->pHeader->dwFlags & WHDR_DONE) == 0)
				alldone = false;
		}
	
		if (alldone)
			break;

		Sleep(20);
	}

	for (CBlock *pBlock = Blocks; pBlock < Blocks + numBlocks; pBlock++)
	{
		if (pBlock->Prepared)
		{
			if (waveOutUnprepareHeader(Handle, pBlock->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			{
				Error("waveOutUnprepareHeader() failed");
			}
		}
	}

	if (waveOutClose(Handle) != MMSYSERR_NOERROR)
	{
		Error("waveOutClose() failed");
		return false;
	}

	for (pBlock = Blocks; pBlock < Blocks + numBlocks; pBlock++)
	{
		GlobalUnlock(pBlock->Handle);
		GlobalFree(pBlock->Handle);
		GlobalUnlock(pBlock->HeaderHandle);
		GlobalFree(pBlock->HeaderHandle);
	}
	
	Running = false;
	return true;
}

#define SHORT_MIN	-32768
#define SHORT_MAX	32767

// returns random value between 0 and 1
// i got the magic numbers from csound so they should be ok but 
// I haven't checked them myself
inline double frand()
{
	static long stat = 0x16BA2118;
	stat = (stat * 1103515245 + 12345) & 0x7fffffff;
	return (double)stat * (1.0 / 0x7fffffff);
}

static void QuantizeWithDither(float *pin, int *piout, int c)
{
	double const d2i = (1.5 * (1 << 26) * (1 << 26));
	
	do
	{
		double res = ((double)pin[1] + frand()) + d2i;

		int r = *(int *)&res;

		if (r < SHORT_MIN)
			r = SHORT_MIN;
		else if (r > SHORT_MAX)
			r = SHORT_MAX;

		res = ((double)pin[0] + frand()) + d2i;
		int l = *(int *)&res;

		if (l < SHORT_MIN)
			l = SHORT_MIN;
		else if (l > SHORT_MAX)
			l = SHORT_MAX;

		*piout++ = (r << 16) | (word)l;
		pin += 2;
	} while(--c);
}

static void Quantize(float *pin, int *piout, int c)
{
	double const d2i = (1.5 * (1 << 26) * (1 << 26));
	
	do
	{
		double res = ((double)pin[1]) + d2i;

		int r = *(int *)&res;

		if (r < SHORT_MIN)
			r = SHORT_MIN;
		else if (r > SHORT_MAX)
			r = SHORT_MAX;

		res = ((double)pin[0]) + d2i;
		int l = *(int *)&res;

		if (l < SHORT_MIN)
			l = SHORT_MIN;
		else if (l > SHORT_MAX)
			l = SHORT_MAX;

		*piout++ = (r << 16) | (word)l;
		pin += 2;
	} while(--c);
}

void ad::DoBlocks()
{
	CBlock *pb = Blocks + CurrentBlock;

	while(pb->pHeader->dwFlags & WHDR_DONE)
	{
		if (pb->Prepared)
		{
			if (waveOutUnprepareHeader(Handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
				Error("waveOutUnprepareHeader() failed");

			pb->Prepared = false;
		}

		int *pout = (int *)pb->pData;
		int bs = BlockSize / BYTES_PER_SAMPLE;
		
		do
		{
			int n = bs;
			float *pbuf = pCallback(n);
			if (Dither)
				QuantizeWithDither(pbuf, pout, n);
			else
				Quantize(pbuf, pout, n);
			pout += n;
			bs -= n;
		} while(bs > 0);
		

		WritePos += BlockSize/BYTES_PER_SAMPLE;

		pb->pHeader->dwFlags = 0;
		pb->pHeader->lpData = (char *)pb->pData;
		pb->pHeader->dwBufferLength = BlockSize;
		pb->pHeader->dwFlags = 0;
		pb->pHeader->dwLoops = 0;

		if (waveOutPrepareHeader(Handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			Error("waveOutPrepareHeader() failed");

		pb->Prepared = true;

		if (waveOutWrite(Handle, pb->pHeader, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
			Error("waveOutWrite() failed");

		pb++;
		if (pb== Blocks + numBlocks)
			pb = Blocks;

	}

	CurrentBlock = pb - Blocks;
}

void ad::ReadConfig()
{
	SamplesPerSec=44100;
	DeviceID=0;
	numBlocks = 4;
	BlockSize = 8192;
	PollSleep = 20;
	Dither = 0;
	Flags = ADF_STEREO;
}

void ad::WriteConfig()
{
/*
	WriteProfileInt("SamplesPerSec", SamplesPerSec);
	WriteProfileInt("DeviceID", DeviceID);
	WriteProfileInt("numBlocks", numBlocks);
	WriteProfileInt("BlockSize", BlockSize);
	WriteProfileInt("PollSleep", PollSleep);
	WriteProfileInt("Dither", Dither);
*/
}


void ad::Initialize(dword hwnd, float *(*pcallback)(int &numsamples))
{
	pCallback = pcallback;
	Running = false;
	ReadConfig();

	Initialized = true;
	Start();
}

void ad::Reset()
{
	Stop();
}

void ad::Configure()
{
	CADriverDialog dlg;
	dlg.m_BufNum = numBlocks;
	dlg.m_BufSize = BlockSize;
	dlg.m_Device = DeviceID;
	dlg.m_Dither = Dither;
	dlg.m_SampleRate = SamplesPerSec;

	//{
	//	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if (dlg.DoModal() != IDOK)
	return;
	//}
	
	int oldnb = numBlocks;
	int oldbs = BlockSize;
	int olddid = DeviceID;
	int olddither = Dither;
	int oldsps = SamplesPerSec;

	if (Initialized)Stop();

	numBlocks = dlg.m_BufNum;
	BlockSize = dlg.m_BufSize;
	DeviceID = dlg.m_Device;
	Dither = dlg.m_Dither;
	SamplesPerSec = dlg.m_SampleRate;

	if (Initialized)
	{
		if (Start())
		{
		WriteConfig();
		}
		else
		{
			numBlocks = oldnb;
			BlockSize = oldbs;
			DeviceID = olddid;
			Dither = olddither;
			SamplesPerSec = oldsps;

			Start();
		}
	}
	else
	{
		WriteConfig();
	}
	
}

int ad::GetPlayPos()
{
	if (!Running)
		return 0;

	MMTIME time;
	time.wType = TIME_SAMPLES;

	if (waveOutGetPosition(Handle, &time, sizeof(MMTIME)) != MMSYSERR_NOERROR)
		Error("waveOutGetPosition() failed");

	if (time.wType != TIME_SAMPLES)
		Error("waveOutGetPosition() doesn't support TIME_SAMPLES");

	return time.u.sample & ((1 << 23) - 1);

}

int ad::GetWritePos()
{
	if (!Running)
		return 0;	

	return WritePos & ((1 << 23) - 1);
}

bool ad::Enable(bool e)
{
	if (e)
		return Start();
	else
		return Stop();
}
/*
CAudioDriverInfo info = { 
#ifdef _DEBUG
	"Windows Waveform Audio (debug build)"
#else
	"Windows Waveform Audio"
#endif
};

extern "C"
{
__declspec(dllexport) CAudioDriver * __cdecl NewAD()
{
	return new ad;
}

__declspec(dllexport) CAudioDriverInfo const * __cdecl GetADInfo()
{
	return &info;
}
}

CWinApp App;	
*/