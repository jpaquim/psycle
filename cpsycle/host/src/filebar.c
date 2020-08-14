// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "filebar.h"

#include <songio.h>

#include <uiopendialog.h>
#include <uisavedialog.h>

static void filebar_updatetext(FileBar*, Translator* translator);
static void filebar_onlanguagechanged(FileBar*, Translator* sender);
static void filebar_initalign(FileBar*);
static void filebar_onnewsong(FileBar*, psy_ui_Component* sender);
static void filebar_onloadsong(FileBar*, psy_ui_Component* sender);
static void filebar_onsavesong(FileBar*, psy_ui_Component* sender);

void filebar_init(FileBar* self, psy_ui_Component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(filebar_base(self), parent);	
	psy_ui_component_enablealign(filebar_base(self));
	psy_ui_component_setalignexpand(filebar_base(self),
		psy_ui_HORIZONTALEXPAND);
	psy_ui_button_init(&self->recentbutton, filebar_base(self));
	psy_ui_button_seticon(&self->recentbutton, psy_ui_ICON_MORE);
	psy_ui_label_init(&self->header, filebar_base(self));
	psy_ui_button_init(&self->newbutton, filebar_base(self));
	psy_signal_connect(&self->newbutton.signal_clicked, self,
		filebar_onnewsong);
	psy_ui_button_init(&self->loadbutton, filebar_base(self));
#ifdef PSYCLE_USE_PLATFORM_FILEOPEN
	psy_signal_connect(&self->loadbutton.signal_clicked, self,
		filebar_onloadsong);
#endif
	psy_ui_button_init(&self->savebutton, filebar_base(self));
	psy_signal_connect(&self->savebutton.signal_clicked, self,
		filebar_onsavesong);
	psy_ui_button_init(&self->renderbutton, filebar_base(self));	
	filebar_initalign(self);
	psy_signal_connect(&self->workspace->signal_languagechanged, self,
		filebar_onlanguagechanged);
	filebar_updatetext(self, &workspace->translator);
}

void filebar_updatetext(FileBar* self, Translator* translator)
{
	psy_ui_label_settext(&self->header,
		translator_translate(translator, "file.song"));
	psy_ui_button_settext(&self->newbutton,
		translator_translate(translator, "file.new"));
	psy_ui_button_settext(&self->loadbutton,
		translator_translate(translator, "file.load"));
	psy_ui_button_settext(&self->savebutton,
		translator_translate(translator, "file.save"));
	psy_ui_button_settext(&self->renderbutton,
		translator_translate(translator, "file.render"));
}

void filebar_onlanguagechanged(FileBar* self, Translator* sender)
{
	filebar_updatetext(self, sender);
}

void filebar_initalign(FileBar* self)
{	
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(filebar_base(self), psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
}

void filebar_onnewsong(FileBar* self, psy_ui_Component* sender)
{
	if (workspace_songmodified(self->workspace)) {
		workspace_selectview(self->workspace, TABPAGE_CHECKUNSAVED, 0, CHECKUNSAVE_NEW);
	} else {
		workspace_newsong(self->workspace);
	}
}

void filebar_onloadsong(FileBar* self, psy_ui_Component* sender)
{	
	if (workspace_songmodified(self->workspace)) {
		workspace_selectview(self->workspace, TABPAGE_CHECKUNSAVED, 0, CHECKUNSAVE_LOAD);
	} else {
		psy_ui_OpenDialog dialog;		

		psy_ui_opendialog_init_all(&dialog, 0, "Load Song",
			psy_audio_songfile_loadfilter(), "PSY",
			workspace_songs_directory(self->workspace));
		if (psy_ui_opendialog_execute(&dialog)) {
			workspace_loadsong(self->workspace,
				psy_ui_opendialog_filename(&dialog),
				workspace_playsongafterload(self->workspace));
		}
		psy_ui_opendialog_dispose(&dialog);
	}
}

void filebar_onsavesong(FileBar* self, psy_ui_Component* sender)
{	
	psy_ui_SaveDialog dialog;

	psy_ui_savedialog_init_all(&dialog, 0, "Save Song",
		psy_audio_songfile_loadfilter(), "PSY",
		workspace_songs_directory(self->workspace));
	if (psy_ui_savedialog_execute(&dialog)) {
		workspace_savesong(self->workspace,
			psy_ui_savedialog_filename(&dialog));
	}
	psy_ui_savedialog_dispose(&dialog);
}
