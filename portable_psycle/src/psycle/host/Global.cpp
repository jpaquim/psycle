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



		#if defined _WINAMP_PLUGIN_ // <bohan> why???
			bool FindFileinDir(/* const */ char name[], CString & path)
			{
				return find_file_in_dir_loose(name, path);
			}
			bool find_file_in_dir_loose(/* const */ char original_name[], CString & path)
			{
				if(find_file_in_dir_exact(original_name, path)) return true;
				// <bohan> grrrr... i know the following code won't compile, but anyway, it's only used for winamp... why???
				{
					std::string string = original_name;
					for(std::string::iterator i(string.begin()), i != string.end(), ++i) if(*i == ' ') *i = '-';
					if(find_file_in_dir_exact(string.c_str(), path)) return true;
				}
				{
					std::string string = original_name;
					for(std::string::iterator i(string.begin()), i != string.end(), ++i) if(*i == ' ') *i = '_';
					if(find_file_in_dir_exact(string.c_str(), path)) return true;
				}
				return false;

			}
			bool find_file_in_dir_exact(/* const */ char name[], CString & path)
			{
				CFileFind finder;
				int loop = finder.FindFile(path + "\\*"); // check for subfolders.
				while(loop) 
				{						
					loop = finder.FindNextFile();
					if(finder.IsDirectory() && !finder.IsDots())
					{
						CString filepath = finder.GetFilePath();
						if(FindFileinDir(name, filepath))
						{
							path = filepath;
							return true;
						}
					}
				}
				finder.Close();
				if(finder.FindFile(path + "\\" + name)) // not found in subdirectories, lets see if it's here
				{
					finder.Close();
					path= (path + "\\") + name;
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
					ostream() << string << std::endl;
					ostream().flush();
					operating_system::console::log(level, string);

					// turned off for now because it hangs when called from the
					// processing thread. Perhaps due to dependency on the main
					// thread's event loop? Perhaps it needs it's own event loop?

					//if(Global::pLogWindow) Global::pLogWindow->AddEntry(level, string);
					
				}
			}
			catch(...)
			{
				// oh dear!
				// fallback to std::cerr
				std::cerr << "logger crashed" << std::endl;
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
				static logger instance(loggers::levels::trace, std::cout);
				try
				{
					instance(loggers::levels::exception, "could not create logger");
				}
				catch(...)
				{
					std::cerr << "could not create fallback logger" << std::endl;
				}
				return instance;
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
					std::ostream & instanciate() throw(...)
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
