#include <stdafx.h>
#include <global.h>
#include <Song.h>
#include <Player.h>
#include <Dsp.h>
#include <Configuration.h>
#include "LoggingWindow.h"
#include <operating_system/logger.h>
#if !defined _WINAMP_PLUGIN_
	#include <InputHandler.h>
#endif
///\file
///\brief implementation file for psycle::host::Global.
namespace psycle
{
	namespace host
	{
		Song * Global::_pSong(0);
		Player * Global::pPlayer(0);
		Resampler * Global::pResampler(0);
		Configuration * Global::pConfig(0);
		CLoggingWindow * Global::pLogWindow(0);
		#if !defined _WINAMP_PLUGIN_
			unsigned int Global::_cpuHz;
			InputHandler * Global::pInputHandler(0);
		#endif
			
		Global::Global()
		{
			#ifndef NDEBUG
			operating_system::console::open();
			#endif
			_pSong = new Song;
			pPlayer = new Player;
			pConfig = new Configuration;
			pResampler = new Cubic;
			pResampler->SetQuality(RESAMPLE_LINEAR);
			#if !defined _WINAMP_PLUGIN_
				pInputHandler = new InputHandler;
			#endif
		}

		Global::~Global()
		{
			zapObject(_pSong);
			zapObject(pPlayer);
			zapObject(pResampler);
			zapObject(pConfig);
			zapObject(pLogWindow);
			#if !defined _WINAMP_PLUGIN_
				zapObject(pInputHandler);
			#endif
			#ifndef NDEBUG
				operating_system::console::close();
			#endif
		}



		#if defined _WINAMP_PLUGIN_
			bool FindFileinDir(char *dllname,CString &path)
			{
				CFileFind finder;
				int loop = finder.FindFile(path + "\\*"); // check for subfolders.
				while(loop) 
				{						
					loop = finder.FindNextFile();
					if(finder.IsDirectory() && !finder.IsDots())
					{
						CString filepath = finder.GetFilePath();
						if(FindFileinDir(dllname,filepath))
						{
							path = filepath;
							return true;
						}
					}
				}
				finder.Close();
				if(finder.FindFile(path + "\\" + dllname)) // not found in subdirectories, lets see if it's here
				{
					finder.Close();
					path= (path + "\\") + dllname;
					return true;
				}
				finder.Close();
				return false;
			}
		#endif

			
			
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////
		// logger


		namespace
		{
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
				static logger & create_default_logger() throw(...);
				static std::ostream & default_logger_ostream() throw();
			};
		}

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
			//boost::mutex::scoped_lock lock(mutex()); // scope outside the try-catch statement so that it is freed in all cases if something goes wrong.
			try
			{
				// could reuse the default implementation, but here we've redefined it all
				//operating_system::logger::operator()(level, string);
				if((*this)(level))
				{
						ostream() << string;
						ostream().flush();
						operating_system::console::log(level,string);
						if(Global::pLogWindow) Global::pLogWindow->AddEntry(level, string + '\n'); // hmm, nasty end-of-lines ;p we should clearly specify where it should be added
				}
			}
			catch(...)
			{
				// oh dear!
				// fallback to std::cerr
				std::cerr << "logger crashed" << std::endl;
			}
		}

		logger & logger::create_default_logger() throw(...)
		{
			std::string module_directory;
			{
				char module_file_name[MAX_PATH];
				::GetModuleFileName(0, module_file_name, sizeof module_file_name);
				module_directory = module_file_name;
				module_directory = module_directory.substr(0, module_directory.rfind('\\'));
			}
			return *new logger(loggers::levels::trace, default_logger_ostream());
		}

		logger & logger::default_logger() throw()
		{
			try
			{
				static logger instance = create_default_logger();
				return instance;
			}
			catch(...)
			{
				std::cerr << "could not create logger" << std::endl;
				return *new logger(loggers::levels::trace, std::cout);
			}
		}

		std::ostream & logger::default_logger_ostream() throw()
		{
			try
			{
				class once
				{
				public:
					// <bohan> hmm, someone told me it is disallowed to have static functions in local classes, although msvc7.1 seems to allow it.
					//static std::ostream & instanciate() throw(...)
					std::ostream & instanciate() throw(...) // <bohan> hmm, someone told me it is disallowed to have static functions in local classes.
					{
						std::string module_directory;
						{
							char module_file_name[1 << 10];
							::GetModuleFileName(0, module_file_name, sizeof module_file_name);
							module_directory = module_file_name;
							module_directory = module_directory.substr(0, module_directory.rfind('\\'));
						}
						// this overwrites the file if it already exists.
						return *new std::ofstream((module_directory + "/output.log.txt").c_str());
					}
				};

				// <bohan> hmm, someone told me it is disallowed to have static functions in local classes.
				//static std::ofstream instance = once::instanciate();

				static once once;
				static std::ostream & instance = once.instanciate();

				return instance;
			}
			catch(...)
			{
				std::cerr << "could not create logger output stream" << std::endl;
				return std::cout;
			}
		}
	}
}
