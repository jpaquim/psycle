#include <stdafx.h>
//#include <project.h>
#define OPERATING_SYSTEM__LOGGER
#include <operating_system/logger.h>
#include <operating_system/exceptions/code_description.h>

logger::logger(const int & threshold_level, std::ostream & ostream) : threshold_level_(threshold_level), ostream_(ostream) {}
logger logger::default_logger_(logger::default_threshold_level(), std::cout);

namespace operating_system
{

	console::~console() throw()
	{
		#if defined OPERATING_SYSTEM__MICROSOFT
			if(got_a_console_window_)
			{
				//std::cout << "there is no parent process owning this terminal console window, so, it will close when this process terminates." << std::endl;
				//std::cout << "press enter to terminate and close this window ..." << std::endl;
				std::cout << "press enter to terminate ..." << std::endl;
				std::cin.get();
			}
		#endif
	}

	console::console() throw(exception)
	{
		got_a_console_window_ = false;
		#if !defined OPERATING_SYSTEM__MICROSOFT
			// nothing to do when the operating system is not microsoft's
		#elif 1
		{
			::HANDLE buffer = ::GetStdHandle(STD_OUTPUT_HANDLE);
			unsigned short attributes = BACKGROUND_BLUE | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
			::SetConsoleTextAttribute(buffer, attributes);
			::COORD coord = {0, 0};
			const int width(256), height(1024);
			unsigned long int length;
			::FillConsoleOutputAttribute(buffer, attributes, width * height, coord, &length);
			::CONSOLE_SCREEN_BUFFER_INFO buffer_info;
			::GetConsoleScreenBufferInfo(buffer, &buffer_info);
			if(buffer_info.dwSize.X < width) buffer_info.dwSize.X = width;
			if(buffer_info.dwSize.Y < height) buffer_info.dwSize.Y = height;
			got_a_console_window_ = ::SetConsoleScreenBufferSize(buffer, buffer_info.dwSize);
			if(!got_a_console_window_)
			{
				// no real console, maybe we output to a file, so nevermind
				//throw operating_system::exception("this console is not big enough ! please use ibm windows (aka nt)");
			}
		}
		#else
		{
			// ok, the follow code looks completly weird,
			// but that's actually the "simplest" way one can allocate a console in a gui application the microsoft way.
			bool allocated(false);
			try
			{
				////////////////////////////////////////////////////////////////////
				// allocates a new console window if we don't have one attached yet

#if 0
					if(!::GetConsoleWindow())
					{
						if(!::AllocConsole())
						{
							std::ostringstream s;
							s << "could not allocate a console window: " << operating_system::exceptions::code_description();
							throw operating_system::exception(s.str());
						}
						allocated = true;
					}
#endif

//				::HANDLE console(::CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, 0, CONSOLE_TEXTMODE_BUFFER, 0));

				/////////////////////////////////////////////////////////////////////////
				// allocates the standard file streams for the operating sytem i/o layer

				// standard output file stream
				::HANDLE operating_system_output(::GetStdHandle(STD_OUTPUT_HANDLE));
				if(!operating_system_output) // if we don't have a console attached yet, allocates a console for the operating sytem i/o layer
				{
					if(!::AllocConsole())
					{
						std::ostringstream s;
						s << "could not allocate a console at the operating system layer: " << operating_system::exceptions::code_description();
						throw operating_system::exception(s.str());
					}
					allocated = true;
					operating_system_output = ::GetStdHandle(STD_OUTPUT_HANDLE);
				}
				if(!operating_system_output)
				{
						std::ostringstream s;
						s << "could not allocate a standard output file stream at the operating system layer: " << operating_system::exceptions::code_description();
						throw exception(s.str());
				}

				// standard error file stream
				::HANDLE operating_system_error(::GetStdHandle(STD_ERROR_HANDLE));
				if(!operating_system_error)
				{
						std::ostringstream s;
						s << "could not allocate a standard error file stream at the operating system layer: " << operating_system::exceptions::code_description();
//						throw Exception(s.str());
				}

				// standard input file stream
				::HANDLE operating_system_input(::GetStdHandle(STD_INPUT_HANDLE));
				if(!operating_system_input)
				{
						std::ostringstream s;
						s << "could not allocate a standard input file stream at the operating system layer: " << operating_system::exceptions::code_description();
//						throw Exception(s.str());
				}
				
				/////////////////////////////////////////////////////////////////
				// allocates the standard file streams for the runtime i/o layer

				// standard output file stream
				if(operating_system_output)
				{
					int file_descriptor(::_open_osfhandle(/* microsoft messed the type definition of handles, we *must* hard cast! */ reinterpret_cast<intptr_t>(operating_system_output), _O_TEXT /* opens file in text (translated) mode */));
					if(file_descriptor == -1)
					{
							std::ostringstream s;
							s << "could not allocate a standard output file descriptor at the runtime layer: " << operating_system::exceptions::code_description();
							throw exception(s.str());
					}
					else
					{
						const FILE * const file(::_fdopen(file_descriptor, "w"));
						if(!file)
						{
								std::ostringstream s;
								s << "could not open the standard output file stream at the runtime layer: " << operating_system::exceptions::code_description();
								throw exception(s.str());
						}
						*stdout = *file;
					}
					if(::setvbuf(stdout, 0, _IONBF, 0))
					{
							std::ostringstream s;
							s << "could not set a buffer for the standard output file stream at the runtime layer: " << operating_system::exceptions::code_description();
							throw exception(s.str());
					}
				}

				// standard error file stream
				if(operating_system_error)
				{
					int file_descriptor(::_open_osfhandle(/* microsoft messed the type definition of handles, we *must* hard cast! */ reinterpret_cast<intptr_t>(operating_system_error), _O_TEXT /* opens file in text (translated) mode */));
					if(file_descriptor == -1)
					{
							std::ostringstream s;
							s << "could not allocate a standard error file descriptor at the runtime layer: " << operating_system::exceptions::code_description();
							throw exception(s.str());
					}
					const FILE * const file(::_fdopen(file_descriptor, "w"));
					if(!file)
					{
							std::ostringstream s;
							s << "could not open the standard error file stream at the runtime layer: " << operating_system::exceptions::code_description();
							throw exception(s.str());
					}
					*stderr = *file;
					if(::setvbuf(stderr, 0, _IONBF, 0))
					{
							std::ostringstream s;
							s << "could not set a buffer for the standard error file stream at the runtime layer: " << operating_system::exceptions::code_description();
							throw exception(s.str());
					}
				}

				// standard input file stream
				if(operating_system_input)
				{
					int file_descriptor(::_open_osfhandle(/* microsoft messed the type definition of handles, we *must* hard cast! */ reinterpret_cast<intptr_t>(operating_system_input), _O_TEXT /* opens file in text (translated) mode */));
					if(file_descriptor == -1)
					{
							std::ostringstream s;
							s << "could not allocate a standard input file descriptor at the runtime layer: " << operating_system::exceptions::code_description();
							throw exception(s.str());
					}
					const FILE * const file(::_fdopen(file_descriptor, "r"));
					if(!file)
					{
							std::ostringstream s;
							s << "could not open the standard input file stream at the runtime layer: " << operating_system::exceptions::code_description();
							throw exception(s.str());
					}
					*stdin = *file;
					if(::setvbuf(stdin, 0, _IONBF, 0))
					{
							std::ostringstream s;
							s << "could not set a buffer for the standard input file stream at the runtime layer: " << operating_system::exceptions::code_description();
							throw exception(s.str());
					}
				}

				/////////////////////////////////////////////////////////////////////////
				// allocates the standard file streams for the standard i/o stream layer
	
				std::ios::sync_with_stdio(); // makes cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well

				//////////////////////////////////
				// sets console window properties

				if(::HANDLE console = operating_system_output) // ::GetStdHandle(STD_OUTPUT_HANDLE)
				{
					// colors
					unsigned short attributes = BACKGROUND_BLUE | FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
					::SetConsoleTextAttribute(console, attributes);

					// buffer size
					::CONSOLE_SCREEN_BUFFER_INFO buffer;
					::GetConsoleScreenBufferInfo(console, &buffer);
					{
						const int width(buffer.dwSize.X), height(buffer.dwSize.Y);
						::COORD coord = {0, 0};
						unsigned long int length;
						::FillConsoleOutputAttribute(console, attributes, width * height, coord, &length);
					}
					const int width(256), height(1024);
					if(buffer.dwSize.X < width) buffer.dwSize.X = width;
					if(buffer.dwSize.Y < height) buffer.dwSize.Y = height;
					if(!::SetConsoleScreenBufferSize(console, buffer.dwSize))
					{
						::GetConsoleScreenBufferInfo(console, &buffer);
						const int width(80), height(50); // on non nt systems, we can only have such a ridiculous size!
						if(buffer.dwSize.X < width) buffer.dwSize.X = width;
						if(buffer.dwSize.Y < height) buffer.dwSize.Y = height;
						if(!::SetConsoleScreenBufferSize(console, buffer.dwSize))
						{
							// huh? giving up.
						}
					}

					// cursor
					::CONSOLE_CURSOR_INFO cursor;
					::GetConsoleCursorInfo(console, &cursor); 
					cursor.dwSize = 100;
					cursor.bVisible = true;
					::SetConsoleCursorInfo(console, &cursor);
				}
			}
			catch(const std::exception & e)
			{
//				std::freopen("conout$", "w", stdout);
//				std::ios::sync_with_stdio(); // makes cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
				std::ostringstream types; types << typeid(*this).name() << ", threw " << typeid(e).name();
				{
					std::ostringstream s; s << types.str() << ": " << e.what();
					std::printf(s.str().c_str());
				}
				{
					std::ostringstream title; title << "error in " << types.str();
					std::ostringstream message; message << "could not allocate a console!" << std::endl << types.str() << std::endl << e.what();
					::MessageBox(0, message.str().c_str(), title.str().c_str(), MB_OK | MB_ICONWARNING);
				}
				throw;
			}
			if(allocated) std::cout << "console allocated" << std::endl;
		}
		#endif
	}
}
