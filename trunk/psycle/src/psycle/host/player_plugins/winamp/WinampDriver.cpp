///\file
///\brief implementation file for psycle::host::WinampDriver.
#include <psycle/project.private.hpp>
#include "WinampDriver.hpp"
#include "../../Player.hpp"
#include "../../Helpers.hpp"
#include <winamp-2/in2.h>

extern In_Module mod;

namespace psycle
{
	namespace host
	{
		#define WA_STREAM_SIZE 576
		// post this to the main window at end of file (after playback has stopped)
		#define WM_WA_PSY_EOF WM_USER+2
		short stream_buffer[WA_STREAM_SIZE*4]; // stereo, and double size (for special dsp plugins)

		AudioDriverInfo WinampDriver::_info = { "Winamp" };

		WinampDriver::WinampDriver()
			:context(NULL)
			,killDecodeThread(0)
			,thread_handle(INVALID_HANDLE_VALUE)
			,initialized(false)
			,paused(false)
			,worked(false)
		{}
		
		void WinampDriver::Initialize(HWND hwnd, AUDIODRIVERWORKFN pCallback, void* context)
		{
			hwnd; // unused var
			pCallback; // unused var
			this->context = context;
		}
		
		void WinampDriver::Pause(bool e)
		{
			if (e) 
			{
				paused=true; mod.outMod->Pause(1);
			}
			else
			{
				worked=false;
				paused=false; mod.outMod->Pause(0);
			}
		}
		
		bool WinampDriver::Enable(bool e)
		{
			if ( e )
			{
				outputlatency = mod.outMod->Open(_samplesPerSec,2,16, -1,-1);
				if (outputlatency < 0)
				{
					return false;
				}
				mod.outMod->SetVolume(-666);

				unsigned long tmp;
				killDecodeThread=0;
				paused=0; worked=false;
				memset(stream_buffer,0,sizeof(stream_buffer));
				thread_handle = (HANDLE) CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) PlayThread,(void *) this,0,&tmp);
			}
			else
			{
				if (thread_handle != INVALID_HANDLE_VALUE)
				{
					killDecodeThread=1;
					if (WaitForSingleObject(thread_handle,INFINITE) == WAIT_TIMEOUT)
					{
						MessageBox(mod.hMainWindow,"error asking thread to die!\n","error killing decode thread",0);
						TerminateThread(thread_handle,0);
					}
					CloseHandle(thread_handle);
					thread_handle = INVALID_HANDLE_VALUE;
				}
				mod.outMod->Close();
			}
			return true;
		}

		DWORD WINAPI __stdcall WinampDriver::PlayThread(void *b)
		{
			WinampDriver& wadriver = *reinterpret_cast<WinampDriver*>(b);

			float *float_buffer;
			Player* pPlayer = static_cast<Player*>(wadriver.context);
			int samprate = wadriver._samplesPerSec;
			//int smp2 = samprate * 0.001;
			int plug_stream_size = WA_STREAM_SIZE;
			::SetThreadPriority(::GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

			while (!wadriver.killDecodeThread)
			{
				if ( !wadriver.worked)
				{
					if (pPlayer->_playing)
					{
						//int bmp = pPlayer->bpm;
						float_buffer = pPlayer->Work(pPlayer,plug_stream_size);
						Quantize(float_buffer,stream_buffer,plug_stream_size*2);
						//if ( bmp != pPlayer->bpm ) mod.SetInfo(pPlayer->bpm,smp2,2,1);
						wadriver.worked=true;
					}
					else
					{
						mod.outMod->CanWrite();
						if (!mod.outMod->IsPlaying())
						{
							mod.outMod->Close();
							PostMessage(mod.hMainWindow,WM_WA_PSY_EOF,0,0);
							return 0;
						}
						Sleep(10);
					}
				}
				else if (mod.outMod->CanWrite() >= (plug_stream_size<<(mod.dsp_isactive()?3:2)))
				{
					int t;
					if (mod.dsp_isactive()) t=mod.dsp_dosamples(stream_buffer,plug_stream_size,16,2,samprate)*4;
					else t=plug_stream_size*4;

					int s=mod.outMod->GetWrittenTime();
					mod.SAAddPCMData((char*)stream_buffer,2,16,s);
					mod.VSAAddPCMData((char*)stream_buffer,2,16,s);

					mod.outMod->Write((char*)stream_buffer,t);
					wadriver.worked=false;
				}
				else Sleep(20);
			}
			return 0;
		}

		void WinampDriver::Quantize(float *pin, short *piout, int c)
		{
			float* inb =pin;
			short* outb =piout;
			for (int i=0; i<c; i++)
			{
				if ( *inb > 32767.0f) *outb = 32767;
				else if ( *inb < -32767.0f ) *outb = -32768;
				else *outb = psycle::helpers::math::truncated(*inb);
				*inb++; outb++;
			}
		}

	}
}
