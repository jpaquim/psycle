///\file
///\brief implementation file for psycle::host::Global.
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/host/uiglobal.hpp>
#include <psycle/host/uiconfiguration.hpp>
#include <psycle/host/InputHandler.hpp>
#include <psycle/host/cacheddllfinder.hpp>
#include <legacy/operating_system/logger.hpp>

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

		InputHandler *    UIGlobal::pInputHandler(0);
			
		UIGlobal::UIGlobal()
		{
			#ifndef NDEBUG
				operating_system::console::open();
			#endif
			loggers::trace("Global::Global() ...");
			pInputHandler = new InputHandler;
			loggers::trace("Global::Global() ... initialized static objects.");
		}

		UIGlobal::~UIGlobal()
		{
			delete pInputHandler;
			#ifndef NDEBUG
				operating_system::console::close();
			#endif
		}

		cpu::cycles_type UIGlobal::CalculateCPUFreq()
		{
			return GetWMICPUFreq();
		}

		/// CPU Frequency setup
		/// redone by kSh
		/// based on the WMI; doesn't work on Win9x but falls back to the old method then
		cpu::cycles_type UIGlobal::GetWMICPUFreq()
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
		
			
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		// logger

		/// Psycle's reimplementation of operating_system::logger that also outputs to the logging window.
		/// This is just a reimplementation, it is better documented in the base class itself.
		class logger : public operating_system::logger
		{
		public:
			static logger & default_logger() throw();
			logger(const int & threshold_level, std::ostream & ostream) : operating_system::logger(threshold_level, ostream) {}
			const bool operator()(const int & level) const throw() { return operating_system::logger::operator()(level); }
			/*redefine*/ void operator()(const int & level, const std::string & string) throw();
		private:
			static std::ostream & default_logger_ostream() throw();
		};

		namespace loggers
		{
			void trace(const std::string & string) throw()
			{
				logger::default_logger()(levels::trace, string);
			}

			void info(const std::string & string) throw()
			{
				logger::default_logger()(levels::info, string);
			}

			void warning(const std::string & string) throw()
			{
				logger::default_logger()(levels::warning, string);
			}

			void exception(const std::string & string) throw()
			{
				logger::default_logger()(levels::exception, string);
			}

			void crash(const std::string & string) throw()
			{
				logger::default_logger()(levels::crash, string);
			}
		}

		void logger::operator()(const int & level, const std::string & string) throw()
		{
			boost::mutex::scoped_lock lock(mutex());
			try
			{
				// could reuse the default implementation, but here we've redefined it all
				//operating_system::logger::operator()(level, string);
				if((*this)(level))
				{
					ostream() << "=== ";
					switch(level)
					{
						case loggers::levels::trace: ostream() << "trace"; break;
						case loggers::levels::info: ostream() << "information"; break;
						case loggers::levels::warning: ostream() << "warning"; break;
						case loggers::levels::exception: ostream() << "exception"; break;
						case loggers::levels::crash: ostream() << "crash"; break;
						default: ostream() << "default"; break;
					}
					ostream() << " ===" << std::endl;
					ostream() << string << std::endl;
					ostream().flush();
					operating_system::console::log(level, string);
				}
			}
			catch(...)
			{
				// oh dear!
				// output and error message to std::cerr and
				// fallback to std::clog
				std::cerr << "logger crashed" << std::endl;
				std::clog << "logger: " << level << ": " << string << std::endl;
			}
		}

		logger & logger::default_logger() throw()
		{
			try
			{
				static logger instance(loggers::levels::trace, default_logger_ostream());
				return instance;
			}
			catch(...)
			{
				try
				{
					static logger instance(loggers::levels::trace, std::cout);
					instance(loggers::levels::exception, "could not create logger");
					return instance;
				}
				catch(...)
				{
					std::cerr << "could not create fallback logger" << std::endl;
					//\todo: what to return?
				}
			}
		}

		std::ostream & logger::default_logger_ostream() throw()
		{
			try
			{
				class once
				{
				public:
					static std::ostream & instanciate()
					{
						std::string module_directory;
						{
							char module_file_name[1 << 10];
							::GetModuleFileName(0, module_file_name, sizeof module_file_name);
							module_directory = module_file_name;
							module_directory = module_directory.substr(0, module_directory.rfind('\\'));
						}
						return *new std::ofstream((module_directory + "/psycle.log.txt").c_str());
					}
				};
				static std::ostream & instance(once::instanciate());
				return instance;
			}
			catch(...)
			{
				std::cerr << "could not create logger output stream ; will use standard log output instead" << std::endl;
				return std::clog;
			}
		}
	}
}
