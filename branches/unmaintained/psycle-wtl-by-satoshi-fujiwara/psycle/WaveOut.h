#ifndef _WAVEOUT_H

#define _WAVEOUT_H
/** @file 
 *  @brief waveout stuff based on buzz code
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
#include <mmsystem.h>
#include "AudioDriver.h"

#define MAX_WAVEOUT_BLOCKS		8

class WaveOut : public AudioDriver
{
public:
	explicit WaveOut(iController *pController);
	~WaveOut();
	void Initialize(
		const HWND hwnd,
		const AUDIODRIVERWORKFN pCallback,
		const void* context);
	void Reset();
	const bool Enable(const bool e);	
	const int WritePos();
	const int PlayPos();
	void Configure();
	const bool Initialized() { return m_Initialized; };
	const bool Configured() { return m_Configured; };
	AudioDriverInfo* const GetInfo() { return &m_Info; };
	const int NumBuffers( void ){ return m_NumBlocks; };
	const int BufferSize( void ){ return m_BlockSize; };

private:
	class CBlock
	{
	public:
		HANDLE Handle;
		byte *pData;
		WAVEHDR *pHeader;
		HANDLE HeaderHandle;
		bool Prepared;
	};

	bool m_Initialized;
	bool m_Configured;
	static AudioDriverInfo m_Info;
	static AudioDriverEvent m_Event;
	static ATL::CComAutoCriticalSection m_Lock;

	HWAVEOUT m_Handle;
	int m_DeviceID;
	int m_NumBlocks;
	int m_BlockSize;
	int m_CurrentBlock;
	int m_WritePos;
	int m_PollSleep;
	int m_Dither;
	bool m_bRunning;
	bool m_bStopPolling;
	CBlock m_Blocks[MAX_WAVEOUT_BLOCKS];
	void* m_CallbackContext;
	AUDIODRIVERWORKFN m_pCallback;

	static void PollerThread(void *pWaveOut);
	void ReadConfig();
	void WriteConfig();
	void Error(TCHAR const *msg);
	void DoBlocks();
	bool Start();
	bool Stop();
};

#endif