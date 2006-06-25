///\file
///\brief implementation file for psycle::host::Global.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include <psycle/host/engine/global.hpp>
#include <psycle/host/engine/dsp.hpp>
#include <psycle/host/engine/song.hpp>
#include <psycle/host/engine/player.hpp>
#include <psycle/host/engine/dllfinder.hpp>
#include <psycle/host/configuration.hpp>
#include <operating_system/logger.hpp>

#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <windows.h>
	#include <comdef.h>
	#include <wbemidl.h>
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma comment(lib, "wbemuuid")
	#endif
#endif

namespace psycle
{
	namespace host
	{
		namespace
		{
			cpu::cycles_type GetNaiveCPUFreq()
			{
				cpu::cycles_type before(cpu::cycles());
				::Sleep(1000); ///\todo wastes one second to startup :-(
				return cpu::cycles() - before;
			}

			/// CPU Frequency setup
			/// redone by kSh
			/// based on the WMI; doesn't work on Win9x but falls back to the old method then
			cpu::cycles_type GetWMICPUFreq()
			{
				#if !defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					#error "sorry"
				#endif

				HRESULT hres;
				hres = CoInitializeEx(0, COINIT_MULTITHREADED);
				if(FAILED(hres)) return GetNaiveCPUFreq();
				hres = CoInitializeSecurity
					(
						NULL, -1, NULL,	NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
						RPC_C_IMP_LEVEL_IMPERSONATE, NULL,	EOAC_NONE, NULL
					);
				if(FAILED(hres))
				{
					CoUninitialize();
					return GetNaiveCPUFreq();
				}
				IWbemLocator * pLoc(0);
				hres = CoCreateInstance
					(
						CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
						IID_IWbemLocator, (LPVOID *) &pLoc
					);
				if(FAILED(hres))
				{
					CoUninitialize();
					return GetNaiveCPUFreq();
				}
				IWbemServices * pSvc(0);
				hres = pLoc->ConnectServer
					(
						_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0,
						NULL, 0, 0, &pSvc
					);
				if(FAILED(hres))
				{
					pLoc->Release();
					CoUninitialize();
					return GetNaiveCPUFreq();
				}
				hres = CoSetProxyBlanket
					(
						pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE,
						NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
						NULL, EOAC_NONE
					);
				if(FAILED(hres))
				{
					pSvc->Release();
					pLoc->Release();
					CoUninitialize();
					return GetNaiveCPUFreq();
				}
				IEnumWbemClassObject * pEnumerator(0);
				hres = pSvc->ExecQuery
					(
						bstr_t("WQL"),
						bstr_t("SELECT * FROM Win32_Processor"),
						WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
						NULL, &pEnumerator
					);
				if(FAILED(hres))
				{
					pSvc->Release();
					pLoc->Release();
					CoUninitialize();
					return GetNaiveCPUFreq();
				}
				IWbemClassObject * pclsObj(0);
				ULONG uReturn(0);
				cpu::cycles_type result(0);
				while(pEnumerator)
				{
					HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
					if(!uReturn) break;
					VARIANT vtProp;
					VariantInit(&vtProp);
					hr = pclsObj->Get(L"CurrentClockSpeed", 0, &vtProp, 0, 0);
					if(!FAILED(hr)) result = cpu::cycles_type(1000000) * vtProp.intVal; ///\todo this gives a 1MHz accuracy only.
					VariantClear(&vtProp);
				}
				if(result <= 0) return GetNaiveCPUFreq();
				pSvc->Release();
				pLoc->Release();
				pEnumerator->Release();
				pclsObj->Release();
				CoUninitialize();
				return result;
			}

			cpu::cycles_type GetCPUFreq()
			{
				return GetWMICPUFreq();
			}
		}

		Song *            Global::_pSong(0);
		Player *          Global::pPlayer(0);
		dsp::Resampler *  Global::pResampler(0);
		Configuration *   Global::pConfig(0);
		cpu::cycles_type  Global::cpu_frequency_(0 /*GetCPUFreq()*/);
		InputHandler *    Global::pInputHandler(0);
		DllFinder * Global::pDllFinder(0);
			
		Global::Global()
		{
			#ifndef NDEBUG
				operating_system::console::open();
			#endif
			loggers::trace("Global::Global() ...");
			_pSong = new Song;
			pPlayer = new Player(*_pSong); // [bohan] afaik song is never deleted/recreated from the gui, so we don't even have to care about updating the player's reference.
			pConfig = new Configuration;
			pResampler = new dsp::Cubic;
			pResampler->SetQuality(dsp::R_LINEAR);
			pInputHandler = new InputHandler;
			pDllFinder = new CachedDllFinder;
			cpu_frequency_ = GetCPUFreq();
			loggers::trace("Global::Global() ... initialized static objects.");
		}

		Global::~Global()
		{
			delete pConfig;
			delete pDllFinder;
			#ifndef NDEBUG
				operating_system::console::close();
			#endif
		}
	}
}
