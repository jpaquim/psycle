#ifndef PYSCLE__CORE__PCH__INCLUDED
#define PYSCLE__CORE__PCH__INCLUDED

#pragma once

///\todo qmake is currently not configured to create/use pre-compiled headers
#if 0 // #ifndef PSYCLE__CORE__NO_PCH

#ifdef _WIN32
#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR 5
#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR 0
#define DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH 0
#include <diversalis/operating_system.hpp> // sets winapi version so must come before <windows.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cfloat>
#include <cmath>
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

#include "signalslib.h"

#endif
#endif
