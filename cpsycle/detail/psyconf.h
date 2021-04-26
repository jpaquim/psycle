// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(psy_CONF_H)
#define psy_CONF_H

#include "cpu.h"
#include "os.h"

#if defined DIVERSALIS__OS__MICROSOFT && defined(DIVERSALIS__CPU__X86__SSE)
	#define PSYCLE_USE_SSE	/* enable support for SSE */
#endif

#define PSYCLE_TK_WIN32 1
#define PSYCLE_TK_CURSES 2
#define PSYCLE_TK_XT 3
#define PSYCLE_TK_GTK 4

#if defined(DIVERSALIS__OS__UNIX)
#define PSYCLE_USE_TK PSYCLE_TK_XT
#elif defined(DIVERSALIS__OS__MICROSOFT)
#define PSYCLE_USE_TK PSYCLE_TK_WIN32
#define PSY_USE_PLATFORM_CHECKBOX
#define PSY_USE_PLATFORM_LISTBOX
#define PSYCLE_USE_PLATFORM_FILEOPEN
#else
#define PSYCLE_USE_TK PSYCLE_TK_CURSES  
#endif

#if defined(_MSC_VER) && _MSC_VER > 1200
#define PSYCLE_APP_DIR "C:\\Program Files (x86)\\Psycle Modular Music Studio"
#define PSYCLE_APP64_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#define PSYCLE_USER_DIR "C:\\Program Files (x86)\\Psycle Modular Music Studio"
#define PSYCLE_USER64_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#else
#define PSYCLE_APP_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#define PSYCLE_APP64_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#define PSYCLE_USER_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#define PSYCLE_USER64_DIR "C:\\Program Files\\Psycle Modular Music Studio"
#endif

#define PSYCLE_SONGS_DEFAULT_DIR PSYCLE_USER_DIR "\\Songs"
#define PSYCLE_SAMPLES_DEFAULT_DIR PSYCLE_USER_DIR "\\Samples"
#define PSYCLE_PLUGINS32_DEFAULT_DIR PSYCLE_USER_DIR "\\PsyclePlugins"
#define PSYCLE_PLUGINS64_DEFAULT_DIR PSYCLE_USER64_DIR "\\PsyclePlugins"
#define PSYCLE_LUASCRIPTS_DEFAULT_DIR PSYCLE_USER_DIR "\\LuaScripts"
#define PSYCLE_VSTS32_DEFAULT_DIR PSYCLE_USER_DIR "\\VstPlugins"
#define PSYCLE_VSTS64_DEFAULT_DIR PSYCLE_USER_DIR "\\Vst64Plugins"
#if defined __unix__ || defined __APPLE__
#define PSYCLE_LADSPAS_DEFAULT_DIR "/usr/lib/ladspa/"
#else
#define PSYCLE_LADSPAS_DEFAULT_DIR "C:\\Program Files (x86)\\Audacity\\Plug-Ins"
#endif
#define PSYCLE_SKINS_DEFAULT_DIR PSYCLE_USER_DIR "\\Skins"
#define PSYCLE_DOC_DEFAULT_DIR PSYCLE_APP_DIR "\\Docs"
#define PSYCLE_USERPRESETS_DEFAULT_DIR PSYCLE_APP_DIR "\\Presets"

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
// #define PSYCLE_DEFAULT_LANG_ES
// #define PSYCLE_DEFAULT_LANG_EN

// outcomment this to create the english language dictionary and an empty one
// only with keys. It will be saved in the user dir. It needs then to be copied
// to the host dir

// #define PSYCLE_MAKE_DEFAULT_LANG

/* #define PSYCLE_LOG_WORKEVENTS */

#define PSYCLE_DEFAULT_LOAD_EXTENSION "PSY"
#define PSYCLE_DEFAULT_SAVE_EXTENSION "PSY"

#define PSYCLE_USE_PSY3
#define PSYCLE_USE_PSY2
#define PSYCLE_USE_XM

// #define PSYCLE_TIMEBAR_OLD

#endif /* psy_CONF_H */
