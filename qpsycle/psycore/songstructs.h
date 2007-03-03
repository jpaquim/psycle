#pragma once  // based on Revision 2078
#include "pattern.h"
namespace psy
{
	namespace core
	{
	}
	namespace notecommands
	{
		enum notecommands {
			c0 = 0,
			b9 = 119,
			release = 120,
			tweak,
			tweakeffect, //old. for compatibility only.
			midicc,
			tweakslide,
			//maintain these two as the last ones
			invalid,
			empty = 255
		};
	}
}
