///\file
///\brief implementation file for psycle::host::Global.
#include <project.private.hpp>
//#include "global.hpp" //(already i n project.private.hpp)
#include "Dsp.hpp"
#include "Song.hpp"
#include "Player.hpp"
#include "Configuration.hpp"
#include <operating_system/logger.hpp>
#include "InputHandler.hpp"
#include "vsthost24.hpp"
namespace psycle
{
	namespace host
	{
		Song * Global::_pSong(0);
		Player * Global::pPlayer(0);
		dsp::Resampler * Global::pResampler(0);
		Configuration * Global::pConfig(0);
		cpu::cycles_type Global::_cpuHz(1);
		InputHandler * Global::pInputHandler(0);
		vst::host *Global::pVstHost(0);

		Global::Global()
		{
			#ifndef NDEBUG
				operating_system::console::open();
			#endif
			_pSong = new Song;
			pPlayer = new Player;
			pConfig = new Configuration;
			pResampler = new dsp::Cubic;
			pResampler->SetQuality(dsp::R_LINEAR);
			pInputHandler = new InputHandler;
			pVstHost = new vst::host;
		}

		Global::~Global()
		{
			zapObject(_pSong);
			zapObject(pPlayer);
			zapObject(pResampler);
			zapObject(pConfig);
			zapObject(pInputHandler);
			zapObject(pVstHost);
			#ifndef NDEBUG
				operating_system::console::close();
			#endif
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
				std::string module_directory;
				{
					char module_file_name[1 << 10];
					::GetModuleFileName(0, module_file_name, sizeof module_file_name);
					module_directory = module_file_name;
					module_directory = module_directory.substr(0, module_directory.rfind('\\'));
				}
				static std::ostream* instanced = new std::ofstream((module_directory + "/psycle.log.txt").c_str());
				return *instanced;
			}
			catch(...)
			{
				std::cerr << "could not create logger output stream ; will use standard log output instead" << std::endl;
				return std::clog;
			}
		}
	}
}
