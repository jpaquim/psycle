///\interface psycle::host::UIGlobal
#pragma once
#include <psycle/host/detail/project.hpp>
#include <psycle/engine/global.hpp>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		class InputHandler;
		class CachedDllFinder;

		class Global
		{
			public:
				Global();
				~Global() throw();

				///\todo use singleton pattern instead of static public vars
				//Global static inline & singleton() { static Global instance; return instance; }

				InputHandler     static inline & input_handler() throw() { return *pInputHandler; }
				// For other implementations, you might wish to use Mapped or simply DllFinder instead of the Cached one.
				CachedDllFinder  static inline & dllfinder()	 throw() { return *pDllFinder; }

			PSYCLE__PRIVATE:// shouldn't be static either
				static InputHandler* pInputHandler;

		};

		namespace loggers
		{
			/// very low level, debug, flooding output.
			///\see levels::trace
			void trace(const std::string & string) throw();
			/// normal, informative output.
			///\see levels::info
			void info(const std::string & string) throw();
			/// warnings.
			///\see levels::warning
			void warning(const std::string & string) throw();
			/// exceptions thrown from software, via "throw some_exception;".
			///\see levels::exception
			void exception(const std::string & string) throw();
			/// exceptions thrown from cpu/os.
			/// They are translated into c++ exceptions, see operating_system::exceptions::translated.
			///\see levels::crash
			void crash(const std::string & string) throw();
			/// levels of importance.
			namespace levels
			{
				enum level
				{
					trace, ///< very low level, debug, flooding output.
					info, ///< normal, informative output.
					warning, ///< warnings.
					exception, ///< exceptions thrown from software, via "throw some_exception;".
					crash ///< exceptions thrown from cpu/os. They are translated into c++ exceptions, see operating_system::exceptions::translated.
				};
			}
		}
	}
}
