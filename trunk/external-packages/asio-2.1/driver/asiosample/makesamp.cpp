/* -*- mode:c++, indent-tabs-mode:t -*- */
#include "asiosmpl.h"

AsioDriver *getDriver()
{
	return new AsioSample();
}
