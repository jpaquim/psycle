///\file
///\brief implementation file for psycle::host::Global.
#include <project.private.hpp>
#include "global.hpp"
#include "Song.hpp"
#include "Player.hpp"
#include "Dsp.hpp"
#include "Configuration.hpp"
#include <operating_system/logger.hpp>
#include "InputHandler.hpp"
namespace psycle
{
	namespace host
	{
		Song * Global::_pSong(0);
		Player * Global::pPlayer(0);
		Resampler * Global::pResampler(0);
		Configuration * Global::pConfig(0);
		CLoggingWindow * Global::pLogWindow(0);
		unsigned int Global::_cpuHz;
		InputHandler * Global::pInputHandler(0);
			
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
			pInputHandler = new InputHandler;
		}

		Global::~Global()
		{
			zapObject(_pSong);
			zapObject(pPlayer);
			zapObject(pResampler);
			zapObject(pConfig);
			zapObject(pInputHandler);
			#ifndef NDEBUG
				operating_system::console::close();
				zapObject(pLogWindow);
			#endif
		}



		
			
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
			boost::mutex::scoped_lock lock(mutex()); // scope outside the try-catch statement so that it is freed in all cases if something goes wrong.
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
					case loggers::levels::info: ostream() << "info"; break;
					case loggers::levels::exception: ostream() << "exception"; break;
					case loggers::levels::crash: ostream() << "crash"; break;
					default: ostream() << "default"; break;
					}
					ostream() << " ===" << std::endl;
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
				std::cerr << "logger: " << level << ": " << string;
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
						return *new std::ofstream((module_directory + "/psycle.log.txt").c_str());
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
