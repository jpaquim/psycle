// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(CMDPROPERTIES_H)
#define CMDPROPERTIES_H

#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// aim: Creates properties, that are mapping cmd enum ids (cmdsgeneral.h) to
//      a input key and containing additional information for the SettingView
//      and 'psycle.ini' configuration file. The mapping consits of
//      key: enum cmd int
//      encodedinput: keycode associated with the command
//      text: name that is displayed in the SettingsView
//      shorttext: keyboardbox help text
//      Changes to the default can be made with the Settingsview and are stored
//      by the Workspace in the 'psycle.ini' configuration file.
//
// Example:
//  psy_property_settext(psy_property_setshorttext(
//	  psy_property_setid(psy_property_append_int(general, "cmd_terminal",
//    psy_audio_encodeinput(psy_ui_KEY_F12, 0, 0, 0), 0, 0), CMD_IMM_TERMINAL),
//    "term"), "Terminal");


psy_Property* cmdproperties_create(void);

#ifdef __cplusplus
}
#endif

#endif
