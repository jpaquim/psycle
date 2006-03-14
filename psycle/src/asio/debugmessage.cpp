//#include <project.h>
#include <stdafx.h>
#define ASIO
#include <asio/asiosys.h>
#if DEBUG
	#if MAC
		#include <TextUtils.h>
		namespace asio
		{
			void DEBUGGERMESSAGE(char *string)
			{
				c2pstr(string);
				DebugStr((unsigned char *)string);
			}
		}
	#else
		#error debugmessage
	#endif
#endif
