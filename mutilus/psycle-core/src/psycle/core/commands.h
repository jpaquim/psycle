#pragma once

#include "pattern.h"

namespace psy
{
	namespace core
	{
		namespace commands
		{
			enum command {
				c0 = 0,
				b9 = 119,
				release = 120,
				tweak,
				tweak_effect, ///< old. for compatibility only.
				midi_cc,
				tweak_slide,
				// maintain these two as the last ones
				invalid,
				empty = 255
			};
		}
	}
}
