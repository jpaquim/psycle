/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "linesperbeatbar.h"
/* host */
#include "styles.h"
/* ui */
#include <uiapp.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void linesperbeatbar_on_less(LinesPerBeatBar*,
	psy_ui_Component* sender);
static void linesperbeatbar_on_more(LinesPerBeatBar*,
	psy_ui_Component* sender);
static void linesperbeatbar_update(LinesPerBeatBar*);

/* implementation */
void linesperbeatbar_init(LinesPerBeatBar* self, psy_ui_Component* parent,
	psy_audio_Player* player)
{		
	assert(self);	
	assert(player);

	psy_ui_component_init(linesperbeatbar_base(self), parent, NULL);	
	self->player = player;
	/* prevent line wrap */
	psy_ui_component_set_align_expand(linesperbeatbar_base(self),
		psy_ui_HEXPAND);
	psy_ui_component_set_default_align(linesperbeatbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	/* lpb description label */
	psy_ui_label_init_text(&self->desc, linesperbeatbar_base(self),
		"lpb.lines-per-beat");	
	/* less button */
	psy_ui_button_init_connect(&self->less, linesperbeatbar_base(self),
		self, linesperbeatbar_on_less);
	psy_ui_button_set_icon(&self->less, psy_ui_ICON_LESS);
	/* lpb number label */
	valueui_init(&self->number, linesperbeatbar_base(self), NULL, 0,
		&player->lpb_param.machineparam);
	psy_ui_component_set_preferred_width(valueui_base(&self->number),
		psy_ui_value_make_ew(4.0));
	psy_ui_component_set_style_type(psy_ui_label_base(&self->number.label),
		STYLE_LPB_NUMLABEL);	
	/* more button */
	psy_ui_button_init_connect(&self->more, linesperbeatbar_base(self),
		self, linesperbeatbar_on_more);
	psy_ui_button_set_icon(&self->more, psy_ui_ICON_MORE);	
}

void linesperbeatbar_on_less(LinesPerBeatBar* self,
	psy_ui_Component* sender)
{
	assert(self);

	if (psy_audio_player_lpb(self->player) > 1) {
		psy_audio_player_set_lpb(self->player,
			psy_audio_player_lpb(self->player) - 1);
	}
}

void linesperbeatbar_on_more(LinesPerBeatBar* self,
	psy_ui_Component* sender)
{
	assert(self);

	psy_audio_player_set_lpb(self->player,
		psy_audio_player_lpb(self->player) + 1);
}
