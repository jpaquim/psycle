#include <stdafx.h>
//#include <project.h>
#define OPERATING_SYSTEM__LOGGER
#include <operating_system/logger.h>
#include <operating_system/exceptions/code_description.h>
#if defined OPERATING_SYSTEM__MICROSOFT
#include <io.h>
#include <fcntl.h>
/*
#if !defined WINVER
/// mswin2k
#define WINVER 0x0500
#endif
#if !defined _WIN32_WINNT
/// mswin2k
#define _WIN32_WINNT 0x0500
#endif
*/
#endif
namespace operating_system
{
	logger::logger(const int & threshold_level, std::ostream & ostream) : threshold_level_(threshold_level), ostream_(ostream) {}
	logger logger::default_logger_(logger::default_threshold_level(), std::cout);


	bool console::got_a_console_window_=false;
	console::console()
	{
		got_a_console_window_=false;
	}

	console::~console()
	{
		close();
	}

	void console::close()
	{
#if defined OPERATING_SYSTEM__MICROSOFT
		if(got_a_console_window_)
			FreeConsole();
		got_a_console_window_=false;
#endif
	}

	void console::open() throw(exception)
	{
		assert(!got_a_console_window_);
		close();

#if !defined OPERATING_SYSTEM__MICROSOFT
		// nothing to do when the operating system is not microsoft's
#else
		{
			::HANDLE output_handle=0;
			if(!AllocConsole() || !(output_handle = ::GetStdHandle(STD_OUTPUT_HANDLE))) {
				std::ostringstream s;
				s << "could not allocate a console at the operating system layer: " << operating_system::exceptions::code_description();
				throw operating_system::exception(s.str());
			}
			got_a_console_window_ = true;

			// change buffer size
			{
				::CONSOLE_SCREEN_BUFFER_INFO buffer;
				::GetConsoleScreenBufferInfo(output_handle, &buffer);
				const int width(256), height(1024);
				if(buffer.dwSize.X < width) buffer.dwSize.X = width;
				if(buffer.dwSize.Y < height) buffer.dwSize.Y = height;
				if(!(got_a_console_window_ = ::SetConsoleScreenBufferSize(output_handle, buffer.dwSize)))
				{
					::GetConsoleScreenBufferInfo(output_handle, &buffer);
					const int width(80), height(50); // on non nt systems, we can only have such a ridiculous size!
					if(buffer.dwSize.X < width) buffer.dwSize.X = width;
					if(buffer.dwSize.Y < height) buffer.dwSize.Y = height;
					if(!(got_a_console_window_ = ::SetConsoleScreenBufferSize(output_handle, buffer.dwSize)))
					{
						// huh? giving up.
					}
				}
			}

			// cursor
			{
				::CONSOLE_CURSOR_INFO cursor;
				::GetConsoleCursorInfo(output_handle, &cursor); 
				cursor.dwSize = 100;
				cursor.bVisible = true;
				::SetConsoleCursorInfo(output_handle, &cursor);
			}
		}
#endif
	}

	void console::log(int level, const std::string & string)
	{
		if(!got_a_console_window_)
			return;
		::HANDLE output_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if(!output_handle)
			return;
		unsigned short attributes;
		switch(level)
		{
		case ::psycle::host::loggers::levels::trace:
			attributes=FOREGROUND_BLUE|FOREGROUND_INTENSITY;
			break;
		case ::psycle::host::loggers::levels::info:
			attributes=FOREGROUND_GREEN|FOREGROUND_INTENSITY;
			break;
		case ::psycle::host::loggers::levels::exception:
			attributes=FOREGROUND_RED|FOREGROUND_INTENSITY;
			break;
		case ::psycle::host::loggers::levels::crash:
			attributes=BACKGROUND_RED;
			break;
		default:
			attributes|=FOREGROUND_BLUE|FOREGROUND_RED|FOREGROUND_INTENSITY;
		};

		DWORD length=string.length();
		::SetConsoleTextAttribute(output_handle, attributes);
		::WriteConsole(output_handle,string.c_str(),length,&length,0);
		::WriteConsole(output_handle,"\n",1,&length,0);
	}
}
