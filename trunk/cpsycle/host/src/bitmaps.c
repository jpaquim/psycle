/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* host */
#include "./resources/resource.h"
/* ui */
#include <uiapp.h>


/* implementation */
void register_bitmaps(psy_ui_App* app, const char* app_bmp_path)
{
	psy_ui_app_set_bmp_path(app, app_bmp_path);	
	psy_ui_app_add_app_bmp(app, IDB_MACHINESKIN, "machine_skin.bmp");
	psy_ui_app_add_app_bmp(app, IDB_PARAMKNOB, "TbMainKnob.bmp");
	psy_ui_app_add_app_bmp(app, IDB_ABOUT, "splash_screen.bmp");
	psy_ui_app_add_app_bmp(app, IDB_HEADERSKIN, "pattern_header_skin.bmp");
	psy_ui_app_add_app_bmp(app, IDB_MIXERSKIN, "mixer_skin.bmp");
	psy_ui_app_add_app_bmp(app, IDI_PSYCLEICON, "");
	psy_ui_app_add_app_bmp(app, IDI_MACPARAM, "");
	psy_ui_app_add_app_bmp(app, IDB_KBD, "kbd.bmp");
	psy_ui_app_add_app_bmp(app, IDB_TERM, "term.bmp");
	psy_ui_app_add_app_bmp(app, IDB_CONF_GENERAL, "");
	psy_ui_app_add_app_bmp(app, IDB_PATTERNEDITOR, "pattern-editor.bmp");
	psy_ui_app_add_app_bmp(app, IDB_HELP, "help.bmp");
	psy_ui_app_add_app_bmp(app, IDB_ARROW_BACK_CIRCLE_OUTLINE, "");
	psy_ui_app_add_app_bmp(app, IDB_ARROW_FORWARD_CIRCLE_OUTLINE, "");
	psy_ui_app_add_app_bmp(app, IDB_ARROW_BACK_DARK, "arrow-back-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_ARROW_FORWARD_DARK, "arrow-forward-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_SETTINGS_DARK, "settings-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_PLAY_DARK, "play-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_STOP_DARK, "stop-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_SAVE_DARK, "save-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_OPEN_DARK, "open-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_NEW_DARK, "new-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_PULSE_DARK, "pulse-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_EARTH_DARK, "earth-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_UNDO_DARK, "undo-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_REDO_DARK, "redo-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_NOTES_DARK, "notes-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_MACHINES_DARK, "machines-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_NEWMACHINE_DARK, "newmachine-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_MATRIX_DARK, "matrix-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_WIRES_DARK, "wires-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_HEART_DARK, "heart-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_TRAY_DARK, "tray-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_HEART_FULL_DARK, "heart-full-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_LOOP_DARK, "loop_dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_VSPLIT_DARK, "vsplit_dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_POWER_DARK, "power_dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_SEARCH_DARK, "search-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_EXPAND_DARK, "expand-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_BGMAIN_1, "");
	psy_ui_app_add_app_bmp(app, IDB_BGMAIN, "bgmain.bmp");
	psy_ui_app_add_app_bmp(app, IDB_BGTOP, "bgtop.bmp");
	psy_ui_app_add_app_bmp(app, IDB_ARROW_FORWARD_LIGHT, "arrow-forward-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_ARROW_BACK_LIGHT, "arrow-back-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_MACHINES_LIGHT, "machines-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_NOTES_LIGHT, "notes-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_PLAY_LIGHT, "play-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_OPEN_LIGHT, "open-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_SAVE_LIGHT, "save-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_EARTH_LIGHT, "earth-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_PULSE_LIGHT, "pulse-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_UNDO_LIGHT, "undo-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_REDO_LIGHT, "redo-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_LOOP_LIGHT, "loop_light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_STOP_LIGHT, "stop-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_SETTINGS_LIGHT, "settings-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_VSPLIT_LIGHT, "vsplit_light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_EXPAND_LIGHT, "expand-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_WIRES_LIGHT, "wires-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_MATRIX_LIGHT, "matrix-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_NEWMACHINE_LIGHT, "newmachine-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_EXIT_LIGHT, "exit-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_EXIT_DARK, "exit-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_ARROW_DOWN_DARK, "arrow-down-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_ARROW_UP_DARK, "arrow-up-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_SEQ_DEL_TRACK, "seq-del-track.bmp");
	psy_ui_app_add_app_bmp(app, IDB_SEQ_ADD_TRACK, "seq-add-track.bmp");
	psy_ui_app_add_app_bmp(app, IDB_FLOAT_DARK, "float-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_BITMAP1, "");
	psy_ui_app_add_app_bmp(app, IDB_FLOAT_LIGHT, "float-light,bmp");
	psy_ui_app_add_app_bmp(app, IDB_FOLDER_DARK, "folder-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_FOLDER_LIGHT, "folder-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_TRAIL_SIGN_DARK, "trail-sign-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_TRAIL_SIGN_LIGHT, "trail-sign-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_IMAGE_DARK, "image-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_IMAGE_LIGHT, "image-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_KEYPAD_DARK, "keypad-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_KEYPAD_LIGHT, "keypad-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_METRONOME_DARK, "metronome-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_METRONOME_LIGHT, "metronome-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_TOGGLE_DARK, "toggle-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_TOGGLE_LIGHT, "toggle-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_OPTIONS_DARK, "options-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_OPTIONS_LIGHT, "options-light.bmp");
	psy_ui_app_add_app_bmp(app, IDB_STEP_DARK, "step-dark.bmp");
	psy_ui_app_add_app_bmp(app, IDB_BITMAP3, "");
	psy_ui_app_add_app_bmp(app, IDB_STEP_LIGHT, "step-light.bmp");
}
