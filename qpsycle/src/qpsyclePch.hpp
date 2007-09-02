#pragma once

#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR 5
#define DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR 0
#define DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH 0
#include <diversalis/operating_system.hpp> // sets winapi version so must be before <windows.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <algorithm>
#include <cassert>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
