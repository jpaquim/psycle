/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(PRESETSVIEW_H)
#define PRESETSVIEW_H

/* host */
#include "titlebar.h"
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uicheckbox.h>
#include <uicombobox.h>
/* audio */
#include <preset.h>

#ifdef __cplusplus
extern "C" {
#endif

struct MachineFrame;

typedef struct PresetsView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	TitleBar title_bar;
	psy_ui_Component client;
	psy_ui_ComboBox programbox;	
	psy_ui_Component buttons;	
	psy_ui_Button save;
	psy_ui_Button del;
	psy_ui_Button import;
	psy_ui_Button export_preset;
	psy_ui_CheckBox preview;
	psy_ui_Component bottom;
	psy_ui_Button use;
	psy_ui_Button close;
	psy_Property presets_load;
	psy_Property presets_export;
	uintptr_t mac_id;
	psy_audio_Preset ini_preset;
	bool preset_changed;
	char* presets_file_name;
	/* references */
	Workspace* workspace;
} PresetsView;

void presetsview_init(PresetsView*, psy_ui_Component* parent, Workspace*);

void presetsview_set_mac_id(PresetsView*, uintptr_t mac_id);

#ifdef __cplusplus
}
#endif

#endif /* PRESETSVIEW_H */
