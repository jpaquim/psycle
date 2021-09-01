/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(psy_PSYCONF_H)
#define psy_PSYCONF_H

#if defined __unix__
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#endif

#include "cpu.h"
#include "os.h"

#if defined DIVERSALIS__OS__MICROSOFT && defined(DIVERSALIS__CPU__X86__SSE)
	#define PSYCLE_USE_SSE	/* enable support for SSE */
#endif

#define PSYCLE_TK_WIN32 1
#define PSYCLE_TK_X11 3

#if defined(DIVERSALIS__OS__UNIX)
#define PSYCLE_USE_TK PSYCLE_TK_X11
#elif defined(DIVERSALIS__OS__MICROSOFT)
#define PSYCLE_USE_TK PSYCLE_TK_WIN32
#define PSY_USE_PLATFORM_CHECKBOX
#define PSY_USE_PLATFORM_LISTBOX
#define PSY_USE_PLATFORM_EDIT
#endif

#if defined(DIVERSALIS__OS__UNIX)
#define PSYCLE_RES_DIR "./resources"
#define PSYCLE_APP_DIR "../../../cpsycle"
#define PSYCLE_APP64_DIR "../../../cpsycle"
#define PSYCLE_USER_DIR "../../../cpsycle"
#define PSYCLE_USER64_DIR "../../../cpsycle"
#elif defined(_MSC_VER) && _MSC_VER > 1200
#define PSYCLE_RES_DIR ""
#define PSYCLE_APP_DIR "C:\\Program Files (x86)\\Psycle Modular Music Studio"
#define PSYCLE_APP64_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#define PSYCLE_USER_DIR "C:\\Program Files (x86)\\Psycle Modular Music Studio"
#define PSYCLE_USER64_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#else
#define PSYCLE_RES_DIR ""
#define PSYCLE_APP_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#define PSYCLE_APP64_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#define PSYCLE_USER_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#define PSYCLE_USER64_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#endif

#if defined(DIVERSALIS__OS__MICROSOFT)
#define PSYCLE_SONGS_DEFAULT_DIR PSYCLE_USER_DIR "\\Songs"
#define PSYCLE_SAMPLES_DEFAULT_DIR PSYCLE_USER_DIR "\\Samples"
#define PSYCLE_PLUGINS32_DEFAULT_DIR PSYCLE_USER_DIR "\\PsyclePlugins"
#define PSYCLE_PLUGINS64_DEFAULT_DIR PSYCLE_USER64_DIR "\\PsyclePlugins"
#define PSYCLE_LUASCRIPTS_DEFAULT_DIR PSYCLE_USER_DIR "\\LuaScripts"
#define PSYCLE_VSTS32_DEFAULT_DIR PSYCLE_USER_DIR "\\VstPlugins"
#define PSYCLE_VSTS64_DEFAULT_DIR PSYCLE_USER_DIR "\\Vst64Plugins"
#define PSYCLE_LADSPAS_DEFAULT_DIR "C:\\Program Files (x86)\\Audacity\\Plug-Ins"
#define PSYCLE_SKINS_DEFAULT_DIR PSYCLE_USER_DIR "\\Skins"
#define PSYCLE_DOC_DEFAULT_DIR PSYCLE_APP_DIR "\\Docs"
#define PSYCLE_USERPRESETS_DEFAULT_DIR PSYCLE_APP_DIR "\\Presets"
#else
#define PSYCLE_SONGS_DEFAULT_DIR PSYCLE_USER_DIR "/songs"
#define PSYCLE_SAMPLES_DEFAULT_DIR PSYCLE_USER_DIR "/samples"
#define PSYCLE_PLUGINS32_DEFAULT_DIR PSYCLE_USER_DIR "/plugins/build"
#define PSYCLE_PLUGINS64_DEFAULT_DIR PSYCLE_USER64_DIR "/plugins/build"
#define PSYCLE_LUASCRIPTS_DEFAULT_DIR PSYCLE_USER_DIR "/luascripts"
#define PSYCLE_VSTS32_DEFAULT_DIR PSYCLE_USER_DIR "/vst32"
#define PSYCLE_VSTS64_DEFAULT_DIR PSYCLE_USER_DIR "/vst64"
#define PSYCLE_LADSPAS_DEFAULT_DIR "/usr/lib/ladspa/"
#define PSYCLE_SKINS_DEFAULT_DIR PSYCLE_USER_DIR "/Skins"
#define PSYCLE_DOC_DEFAULT_DIR PSYCLE_APP_DIR "/docs"
#define PSYCLE_USERPRESETS_DEFAULT_DIR PSYCLE_APP_DIR "/Presets"
#endif

#if defined(_MSC_VER) && _MSC_VER > 1200
#define PSYCLE_DEFAULT_FONT "Tahoma;-16"
#define PSYCLE_DEFAULT_PATTERN_FONT "Verdana;-16"
#else
#define PSYCLE_DEFAULT_FONT "tahoma;-12"
#define PSYCLE_DEFAULT_PATTERN_FONT "verdana;-12"
#endif

#define PSYCLE_UNTITLED "Untitled.psy"
#define PSYCLE_INI "psycle.ini"
#define PSYCLE_RECENT_SONG_INI "psycle-recentsongs.ini"
#define PSYCLE_DARKSTYLES_INI "psycle-darkstyles.ini"
#define PSYCLE_LIGHTSTYLES_INI "psycle-lightstyles.ini"

#define PSYCLE_HELPFILES "readme.txt keys.txt tweaking.txt whatsnew.txt"

#define PSYCLE_USE_MACHINEPROXY

#define PSYCLE_DEFAULT_LANG_USER
/* #define PSYCLE_DEFAULT_LANG_ES */
/* #define PSYCLE_DEFAULT_LANG_EN */

/* outcomment the next define to create the english language dictionary and an
   empty one only with keys. It will be saved in the user dir. It needs then to
   be copied to the host dir */

/* #define PSYCLE_MAKE_DEFAULT_LANG */

/* #define PSYCLE_LOG_WORKEVENTS */

#define PSYCLE_DEFAULT_LOAD_EXTENSION "PSY"
#define PSYCLE_DEFAULT_SAVE_EXTENSION "PSY"

#define PSYCLE_USE_PSY3
#define PSYCLE_USE_PSY2
#define PSYCLE_USE_XM

/* #define PSYCLE_TIMEBAR_OLD */

#endif /* psy_PSYCONF_H */
