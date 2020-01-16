// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(psy_CONF_H)
#define psy_CONF_H

#include "cpu.h"

#if defined(DIVERSALIS__CPU__X86__SSE)
#define PSYCLE_USE_SSE	/* enable support for SSE */
#endif

#define PSYCLE_APP_DIR "C:\\Programme\\Psycle"
#define PSYCLE_USER_DIR "C:\\Programme\\Psycle"

#define PSYCLE_SONGS_DEFAULT_DIR PSYCLE_USER_DIR "\\Songs"
#define PSYCLE_SAMPLES_DEFAULT_DIR PSYCLE_USER_DIR "\\Samples"
#define PSYCLE_PLUGINS_DEFAULT_DIR PSYCLE_USER_DIR "\\PsyclePlugins"
#define PSYCLE_LUASCRIPTS_DEFAULT_DIR PSYCLE_USER_DIR "\\LuaScripts"
#define PSYCLE_VSTS32_DEFAULT_DIR PSYCLE_USER_DIR "\\VstPlugins"
#define PSYCLE_VSTS64_DEFAULT_DIR PSYCLE_USER_DIR "\\Vst64Plugins"
#define PSYCLE_SKINS_DEFAULT_DIR PSYCLE_USER_DIR "\\Skins"
#define PSYCLE_DOC_DEFAULT_DIR PSYCLE_APP_DIR "\\Docs"

#endif psy_CONF_H
