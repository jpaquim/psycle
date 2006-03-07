
// Note: there's no autoconf configuration script, so we hardcode the options.

#pragma once
#include <psycle/host/detail/configuration.hpp>

#define CONFIGURATION__OPTION__ENABLE__PRE_COMPILATION

/// [bohan] this is needed with boost version 1.33.1, but i think it's a bug in boost's autolinking.
#define BOOST_THREAD_USE_DLL
