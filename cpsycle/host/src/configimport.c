/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "configimport.h"
/* file */
#include <inireader.h>
/* audio */
#include <cmdsnotes.h>

/* prototypes*/
static void psycleconfigimport_on_read(PsycleConfigImport*, psy_IniReader* sender,
	const char* key, const char* value);
static void psycleconfigimport_read_cmdset(PsycleConfigImport*,
	const char* strkey, const char* strval);

static int _httoi(const char* value)
{
	return (int)strtol(value, 0, 16);
}


/* implementation */
void psycleconfigimport_init(PsycleConfigImport* self, PsycleConfig* config)
{
	self->config = config;
}

void psycleconfigimport_dispose(PsycleConfigImport* self)
{

}

int psycleconfigimport_read(PsycleConfigImport* self, const char* path)
{	
	int success;
	psy_EventDriver* driver;
	psy_Property* cmds;
	psy_IniReader inireader;	

	driver = psy_audio_player_kbddriver(self->config->audio.player);
	if (driver) {				
		cmds = psy_property_clone(psy_eventdriver_configuration(driver));
		self->notes = psy_property_at(cmds, "cmds.notes", PSY_PROPERTY_TYPE_NONE);
	} else {
		cmds = NULL;
		self->notes = NULL;
	}
	psy_inireader_init(&inireader);
	psy_signal_connect(&inireader.signal_read, self,
		psycleconfigimport_on_read);
	success = inireader_load(&inireader, path);
	if (cmds) {
		psy_eventdriver_configure(driver, cmds);
		psy_property_deallocate(cmds);
	}
	psy_inireader_dispose(&inireader);	
	return success;
}

void psycleconfigimport_on_read(PsycleConfigImport* self, psy_IniReader* sender,
	const char* key, const char* strval)
{
	if (!key || !strval) {
		return;
	}
	/* [configuration] */	
	if (strcmp(key, "FollowSong") == 0) {
		if (strcmp(strval, "1") == 0) {
			keyboardmiscconfig_follow_song(&self->config->misc);
		} else {
			keyboardmiscconfig_stop_follow_song(&self->config->misc);
		}
		return;
	}
	/* [PatternVisual] */
	if (strcmp(key, "pvc_background") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_background_colour_left(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_background2") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_background_colour_right(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_row4beat") == 0) {
			int value;

			value = _httoi(strval);
			patternviewconfig_set_row4beat_colour_left(&self->config->patview,
				psy_ui_colour_make(value));
			return;
	} else if (strcmp(key, "pvc_row4beat2") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_row4beat_colour_right(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_rowbeat") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_rowbeat_colour_left(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_rowbeat2") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_rowbeat_colour_right(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_row") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_row_colour_left(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_row2") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_row_colour_right(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_font") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_font_colour_left(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_font2") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_font_colour_right(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_fontPlay") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_font_play_colour_left(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_fontPlay2") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_font_play_colour_right(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_fontSel") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_font_sel_colour_left(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_fontSel2") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_font_sel_colour_right(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_selection") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_selection_colour_left(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_selection2") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_selection_colour_right(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_playbar") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_playbar_colour_left(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "pvc_playbar2") == 0) {
		int value;

		value = _httoi(strval);
		patternviewconfig_set_playbar_colour_right(&self->config->patview,
			psy_ui_colour_make(value));
		return;
	} else if (strcmp(key, "DisplayLineNumbers") == 0) {
		if (strcmp(strval, "1") == 0) {
			patternviewconfig_display_line_numbers(&self->config->patview);
		} else {
			patternviewconfig_hide_line_numbers(&self->config->patview);
		}
		return;
	}
	if (strcmp(key, "DisplayLineNumbersHex") == 0) {
		if (strcmp(strval, "1") == 0) {
			patternviewconfig_display_line_numbers_in_hex(&self->config->patview);
		} else {
			patternviewconfig_display_line_numbers_in_dec(&self->config->patview);
		}
		return;
	}
	if (strcmp(key, "DisplayLineNumbersCursor") == 0) {
		if (strcmp(strval, "1") == 0) {
			patternviewconfig_display_line_numbers_cursor(&self->config->patview);
		} else {
			patternviewconfig_hide_line_numbers_cursor(&self->config->patview);
		}
		return;
	}
	/* [InputHandling] */
	if (strcmp(key, "bCtrlPlay") == 0) {
		if (strcmp(strval, "1") == 0) {
			keyboardmiscconfig_enable_playstartwithrctrl(&self->config->misc);
		} else {
			keyboardmiscconfig_disable_playstartwithrctrl(&self->config->misc);
		}
		return;
	}
	if (strcmp(key, "bFT2HomeBehaviour") == 0) {
		if (strcmp(strval, "1") == 0) {
			keyboardmiscconfig_enable_ft2home(&self->config->misc);
		} else {
			keyboardmiscconfig_disable_ft2home(&self->config->misc);
		}
		return;
	}
	if (strcmp(key, "bFT2DelBehaviour") == 0) {
		if (strcmp(strval, "1") == 0) {
			keyboardmiscconfig_enable_ft2delete(&self->config->misc);
		} else {
			keyboardmiscconfig_disable_ft2delete(&self->config->misc);
		}
		return;
	}
	if (strcmp(key, "wrapAround") == 0) {
		if (strcmp(strval, "1") == 0) {
			patternviewconfig_enable_wrap_around(&self->config->patview);
		} else {
			patternviewconfig_disable_wrap_around(&self->config->patview);
		}
		return;
	}	
	/* [MachineVisual] */
	if (strcmp(key, "mv_colour") == 0) {
		int value;

		value = _httoi(strval);
		machineviewconfig_set_background_colour(&self->config->macview,
			psy_ui_colour_make(value));
		return;
	}
	psycleconfigimport_read_cmdset(self, key, strval);
}

void psycleconfigimport_read_cmdset(PsycleConfigImport* self,
	const char* strkey, const char* strval)
{
	if (self->notes && strstr(strkey, "CmdSet") != NULL) {		
		int value;
		int key;
		int mod;
		const char* kbd_key;
		
		value = atoi(strval);
		mod = value >> 8;
		key = value & 0xFF;
		if (strcmp(strkey, "CmdSet:0000") == 0) {
			kbd_key = "cmd_note_c_0";			
		} else if (strcmp(strkey, "CmdSet:0001") == 0) {
			kbd_key = "cmd_note_cs0";		
		} else if (strcmp(strkey, "CmdSet:0002") == 0) {
			kbd_key = "cmd_note_d_0";		
		} else if (strcmp(strkey, "CmdSet:0003") == 0) {
			kbd_key = "cmd_note_ds0";
		} else if (strcmp(strkey, "CmdSet:0004") == 0) {
			kbd_key = "cmd_note_e_0";
		} else if (strcmp(strkey, "CmdSet:0005") == 0) {
			kbd_key = "cmd_note_f_0";
		} else if (strcmp(strkey, "CmdSet:0006") == 0) {
			kbd_key = "cmd_note_fs0";
		} else if (strcmp(strkey, "CmdSet:0007") == 0) {
			kbd_key = "cmd_note_g_0";
		} else if (strcmp(strkey, "CmdSet:0008") == 0) {
			kbd_key = "cmd_note_gs0";
		} else if (strcmp(strkey, "CmdSet:0009") == 0) {
			kbd_key = "cmd_note_a_0";
		} else if (strcmp(strkey, "CmdSet:0010") == 0) {
			kbd_key = "cmd_note_as0";
		} else if (strcmp(strkey, "CmdSet:0011") == 0) {
			kbd_key = "cmd_note_b_0";
		} else if (strcmp(strkey, "CmdSet:0012") == 0) {
			kbd_key = "cmd_note_c_1";
		} else if (strcmp(strkey, "CmdSet:0013") == 0) {
			kbd_key = "cmd_note_cs1";
		} else if (strcmp(strkey, "CmdSet:0014") == 0) {
			kbd_key = "cmd_note_d_1";
		} else if (strcmp(strkey, "CmdSet:0015") == 0) {
			kbd_key = "cmd_note_ds1";
		} else if (strcmp(strkey, "CmdSet:0016") == 0) {
			kbd_key = "cmd_note_e_1";
		} else if (strcmp(strkey, "CmdSet:0017") == 0) {
			kbd_key = "cmd_note_f_1";
		} else if (strcmp(strkey, "CmdSet:0018") == 0) {
			kbd_key = "cmd_note_fs1";		
		} else if (strcmp(strkey, "CmdSet:0019") == 0) {
			kbd_key = "cmd_note_g_1";
		} else if (strcmp(strkey, "CmdSet:0020") == 0) {
			kbd_key = "cmd_note_gs1";
		} else if (strcmp(strkey, "CmdSet:0021") == 0) {
			kbd_key = "cmd_note_a_1";
		} else if (strcmp(strkey, "CmdSet:0022") == 0) {
			kbd_key = "cmd_note_as1";
		} else if (strcmp(strkey, "CmdSet:0023") == 0) {
			kbd_key = "cmd_note_b_1";
		} else if (strcmp(strkey, "CmdSet:0024") == 0) {
			kbd_key = "cmd_note_c_2";
		} else if (strcmp(strkey, "CmdSet:0025") == 0) {
			kbd_key = "cmd_note_cs2";
		} else if (strcmp(strkey, "CmdSet:0026") == 0) {
			kbd_key = "cmd_note_d_2";
		} else if (strcmp(strkey, "CmdSet:0027") == 0) {
			kbd_key = "cmd_note_ds2";
		} else if (strcmp(strkey, "CmdSet:0028") == 0) {
			kbd_key = "cmd_note_e_2";
		} else {
			kbd_key = NULL;
		}
		if (kbd_key) {						
			psy_property_set_int(self->notes, kbd_key,
				psy_audio_encodeinput(key, 0, 0, 0, 0));			
		}		
	}
}
