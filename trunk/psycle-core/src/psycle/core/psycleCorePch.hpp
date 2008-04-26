// -*- mode:c++; indent-tabs-mode:t -*-

#ifdef _WIN32
#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR 5
#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR 0
#define DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH 0
#include <diversalis/operating_system.hpp> // sets winapi version so must come before <windows.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined __cplusplus
// Standard headers
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>


#include <diversalis/compiler.hpp>

//psycle-core headers
#include "commands.h"
#include "constants.h"
#include "datacompression.h"
//#include "dither.h"
//#include "dsp.h"
#include "file.h"
//#include "fileio.h"
#include "ladspa.h"
#include "machinekey.hpp"
#include "mersennetwister.h"
#include "playertimeinfo.h"
#include "plugininfo.h"
#include "plugin_interface.h"
#include "riff.h"
#include "signalslib.h"
#include "timesignature.h"
#include "zipreader.h"
#include "zipwriter.h"
//#include "zipwriterstream.h"
#endif
