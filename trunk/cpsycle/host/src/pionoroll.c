/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "pianoroll.h"
/* local */
#include "trackergridstate.h"
#include "patterncmds.h"
#include "cmdsnotes.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/trace.h"

#define PIANOROLL_REFRESHRATE 50
#define CMD_ENTER 1100

static void set_cmd(psy_Property* cmds, uintptr_t cmd, uint32_t keycode,
	bool shift, bool ctrl, const char* key, const char* shorttext);

/* PianoBar */

/* implenentation */
void pianobar_init(PianoBar* self, psy_ui_Component* parent)
{
	assert(self);
		
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 0.0));	
	psy_ui_label_init_text(&self->beats, pianobar_base(self),
		"patternview.beats");
	psy_ui_component_set_margin(&self->component,
		psy_ui_margin_make_em(0.0, 1.0, 0.0, 1.0));
	zoombox_init(&self->zoombox_beatwidth, pianobar_base(self));
	psy_ui_label_init_text(&self->keys, pianobar_base(self),
		"patternview.keyboard");
	zoombox_init(&self->zoombox_keyheight, &self->component);
	psy_ui_button_init_text(&self->edit_mode, pianobar_base(self),
		"Edit");
	psy_ui_button_highlight(&self->edit_mode);
	psy_ui_button_init_text(&self->select_mode, pianobar_base(self),
		"Select");
	psy_ui_label_init_text(&self->tracks, pianobar_base(self),
		"patternview.showtracks");
	psy_ui_button_init_text(&self->tracks_all, pianobar_base(self),
		"patternview.all");
	psy_ui_button_highlight(&self->tracks_all);
	psy_ui_button_init_text(&self->track_curr, pianobar_base(self),
		"patternview.current");
	psy_ui_button_init_text(&self->tracks_active, pianobar_base(self),
		"patternview.active");	
}

/* Pianoroll */

/* protoypes */
static void pianoroll_on_timer(Pianoroll*, uintptr_t timerid);
static void pianoroll_on_play_line_changed(Pianoroll*, Workspace* sender);
static void pianoroll_on_lpb_changed(Pianoroll*, psy_audio_Player* sender,
	uintptr_t lpb);
static void pianoroll_on_align(Pianoroll*);
static void pianoroll_on_mouse_down(Pianoroll*, psy_ui_MouseEvent*);
static void pianoroll_on_grid_scroll(Pianoroll*, psy_ui_Component* sender);
static void pianoroll_on_beat_width_changed(Pianoroll*, ZoomBox* sender);
static void pianoroll_on_key_height_changed(Pianoroll*, ZoomBox* sender);
static void pianoroll_on_keytype_changed(Pianoroll*, psy_ui_Component* sender,
	intptr_t index);
static void pianoroll_on_edit_mode(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_on_select_mode(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_on_display_all_tracks(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_on_display_current_track(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_on_display_active_tracks(Pianoroll*, psy_ui_Button* sender);
static void pianoroll_update_track_display_buttons(Pianoroll*);
static void pianoroll_on_cursor_changed(Pianoroll*, psy_audio_Sequence* sender);
static void pianoroll_on_song_changed(Pianoroll*, Workspace* sender);
static void pianoroll_on_configure(Pianoroll*, PatternViewConfig* sender,
	psy_Property*);

/* vtable */
static psy_ui_ComponentVtable pianoroll_vtable;
static bool pianoroll_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* pianoroll_vtable_init(Pianoroll* self)
{
	assert(self);

	if (!pianoroll_vtable_initialized) {
		pianoroll_vtable = *(self->component.vtable);
		pianoroll_vtable.onalign =
			(psy_ui_fp_component_event)
			pianoroll_on_align;
		pianoroll_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			pianoroll_on_mouse_down;		
		pianoroll_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			pianoroll_on_timer;
		pianoroll_vtable_initialized = TRUE;
	}
	return &pianoroll_vtable;
}
/* implenentation */
void pianoroll_init(Pianoroll* self, psy_ui_Component* parent,
	PatternViewState* pvstate, Workspace* workspace)
{
	assert(self);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_vtable(&self->component, pianoroll_vtable_init(self));
	psy_ui_component_set_style_type(&self->component,
		STYLE_PV_TRACK_VIEW);
	psy_ui_component_set_style_type_select(&self->component,
		STYLE_PV_TRACK_VIEW_SELECT);
	self->workspace = workspace;	
	self->opcount = 0;
	self->center_key = TRUE;
	/* shared states */
	keyboardstate_init(&self->keyboardstate);
	pianogridstate_init(&self->gridstate, pvstate);	
	/* bar */
	pianobar_init(&self->bar, &self->component);
	psy_ui_component_set_align(&self->bar.component, psy_ui_ALIGN_BOTTOM);
	psy_signal_connect(&self->bar.zoombox_beatwidth.signal_changed, self,
		pianoroll_on_beat_width_changed);
	psy_signal_connect(&self->bar.zoombox_keyheight.signal_changed, self,
		pianoroll_on_key_height_changed);
	psy_ui_component_set_align(&self->bar.zoombox_keyheight.component,
		psy_ui_ALIGN_LEFT);
	psy_signal_connect(&self->bar.edit_mode.signal_clicked, self,
		pianoroll_on_edit_mode);
	psy_signal_connect(&self->bar.select_mode.signal_clicked, self,
		pianoroll_on_select_mode);
	psy_signal_connect(&self->bar.tracks_all.signal_clicked, self,
		pianoroll_on_display_all_tracks);
	psy_signal_connect(&self->bar.track_curr.signal_clicked, self,
		pianoroll_on_display_current_track);
	psy_signal_connect(&self->bar.tracks_active.signal_clicked, self,
		pianoroll_on_display_active_tracks);
	/* left area (keyboardheader, keyboard) */
	psy_ui_component_init(&self->left, &self->component, NULL);
	psy_ui_component_set_align(&self->left, psy_ui_ALIGN_LEFT);
	
	/* top area (beatruler) */
	psy_ui_component_init(&self->top, &self->component, NULL);
	psy_ui_component_set_align(&self->top, psy_ui_ALIGN_TOP);
	psy_ui_component_setpreferredheight(&self->top,
		psy_ui_value_make_eh(1.0));
	pianoruler_init(&self->header, &self->top, &self->gridstate);
	psy_ui_component_set_align(pianoruler_base(&self->header),
		psy_ui_ALIGN_FIXED);
	/* client area (event grid) */
	pianogrid_init(&self->grid, &self->component, &self->keyboardstate,
		&self->gridstate, &workspace->inputhandler, self->workspace);
	psy_ui_scroller_init(&self->scroller, &self->component, NULL, NULL);
	psy_ui_scroller_set_client(&self->scroller, pianogrid_base(&self->grid));
	psy_ui_component_set_align(&self->scroller.component, psy_ui_ALIGN_CLIENT);	
	psy_ui_component_set_align(&self->grid.component, psy_ui_ALIGN_FIXED);
	/* Keyboard */
	psy_ui_component_init(&self->keyboardpane, &self->left, NULL);
	psy_ui_component_set_align(&self->keyboardpane, psy_ui_ALIGN_CLIENT);
	pianokeyboard_init(&self->keyboard, &self->keyboardpane,
		&self->keyboardstate, &self->workspace->player,
		&self->grid.component);
	psy_ui_combobox_init(&self->keytype, &self->left);
	psy_ui_combobox_set_char_number(&self->keytype, 6);
	psy_ui_combobox_add_text(&self->keytype, "Keys");
	psy_ui_combobox_add_text(&self->keytype, "Notes");
	psy_ui_combobox_add_text(&self->keytype, "Drums");
	psy_ui_combobox_select(&self->keytype, 0);
	psy_ui_component_setpreferredheight(&self->keytype.component,
		psy_ui_value_make_eh(1.0));
	psy_ui_component_hide(&self->keytype.expand.component);
	psy_ui_component_set_align(&self->keytype.component,
		psy_ui_ALIGN_TOP);
	psy_signal_connect(&self->keytype.signal_selchanged, self,
		pianoroll_on_keytype_changed);
	psy_ui_component_set_align(&self->keyboard.component,
		psy_ui_ALIGN_HCLIENT);
	/* connect signals */
	psy_signal_connect(&pianogrid_base(&self->grid)->signal_scrolled, self,
		pianoroll_on_grid_scroll);
	psy_signal_connect(&self->workspace->song->sequence.signal_cursorchanged,
		self, pianoroll_on_cursor_changed);
	psy_signal_connect(&self->workspace->signal_play_line_changed, self,
		pianoroll_on_play_line_changed);
	psy_signal_connect(&workspace->signal_songchanged, self,
		pianoroll_on_song_changed);	
	psy_signal_connect(&workspace->player.signal_lpbchanged, self,
		pianoroll_on_lpb_changed);	
	psy_ui_component_start_timer(&self->component, 0, PIANOROLL_REFRESHRATE);
	/* configuration */
	psy_signal_connect(&workspace->config.patview.signal_changed, self,
		pianoroll_on_configure);
	keyboardstate_update_metrics(&self->keyboardstate,
		psy_ui_component_textmetric(&self->component));
}

void pianoroll_scroll_to_order(Pianoroll* self)
{
	assert(self);
		
	if (!self->grid.preventscrollleft) {
		pianogrid_onpatternchange(&self->grid);
		psy_ui_component_set_scroll(pianogrid_base(&self->grid),
			psy_ui_point_make(psy_ui_value_zero(),
				psy_ui_component_scroll_top(pianogrid_base(&self->grid))));
		psy_ui_component_set_scroll(pianoruler_base(&self->header),
			psy_ui_point_zero());
		pianoroll_update_scroll(self);
	}
}

void pianoroll_scroll_to_key(Pianoroll* self, uint8_t key)
{
	psy_ui_component_set_scroll_top(pianogrid_base(&self->grid),
		psy_ui_value_make_px(
			self->keyboardstate.keyheightpx *
			(self->keyboardstate.keymax - key)));
}

void pianoroll_on_play_line_changed(Pianoroll* self, Workspace* sender)
{
	assert(self);

	if (!psy_ui_component_draw_visible(pianoroll_base(self))) {
		return;
	}	
	pianogrid_invalidate_lines(&self->grid,
		self->workspace->host_sequencer_time.lastplaycursor.linecache,
		self->workspace->host_sequencer_time.currplaycursor.linecache);	
}

void pianoroll_on_timer(Pianoroll* self, uintptr_t timerid)
{
	assert(self);

	if (patternviewstate_pattern(self->gridstate.pv)) {
		if (patternviewstate_pattern(self->gridstate.pv) &&
				patternviewstate_pattern(self->gridstate.pv)->opcount != self->opcount) {
			psy_ui_component_invalidate(pianogrid_base(&self->grid));
		}
		self->opcount = patternviewstate_pattern(self->gridstate.pv)
			? patternviewstate_pattern(self->gridstate.pv)->opcount
			: 0;
	}
	pianokeyboard_idle(&self->keyboard);
}

void pianoroll_on_align(Pianoroll* self)
{
	assert(self);
	
	pianoroll_update_scroll(self);
	if (self->center_key && psy_ui_component_draw_visible(&self->component)) {
		self->center_key = FALSE;
		pianoroll_scroll_to_key(self, 84);
	}
}

void pianoroll_on_mouse_down(Pianoroll* self, psy_ui_MouseEvent* ev)
{
	assert(self);

	psy_ui_component_set_focus(pianoroll_base(self));
}

void pianoroll_update_scroll(Pianoroll* self)
{	
	assert(self);	
	
	psy_ui_component_setscrollstep(pianogrid_base(&self->grid),
		psy_ui_size_make_px(
			pianogridstate_steppx(&self->gridstate),
			self->keyboardstate.keyheightpx));
	psy_ui_component_set_scroll_step_height(pianokeyboard_base(&self->keyboard),
		psy_ui_value_make_px(self->keyboardstate.keyheightpx));
	psy_ui_component_set_scroll_step_width(pianoruler_base(&self->header),
		psy_ui_value_make_px(pianogridstate_steppx(&self->gridstate)));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
	psy_ui_component_invalidate(pianoruler_base(&self->header));
	psy_ui_component_invalidate(pianokeyboard_base(&self->keyboard));
}

void pianoroll_on_lpb_changed(Pianoroll* self, psy_audio_Player* sender,
	uintptr_t lpb)
{
	assert(self);
	
	psy_ui_component_set_scroll_step_width(pianogrid_base(&self->grid),
		psy_ui_value_make_px(pianogridstate_steppx(&self->gridstate)));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_set_scroll_left(pianoruler_base(&self->header),
		psy_ui_component_scroll_left(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(pianoruler_base(&self->header));	
	psy_ui_component_invalidate(pianogrid_base(&self->grid));
}

void pianoroll_on_cursor_changed(Pianoroll* self, psy_audio_Sequence* sender)
{	
	assert(self);
	
	pianoroll_scroll_to_order(self);
	pianogrid_update_cursor(&self->grid, sender->cursor);	
}

void pianoroll_on_song_changed(Pianoroll* self, Workspace* workspace)
{	
	if (workspace->song) {
		psy_signal_connect(
			&self->workspace->song->sequence.signal_cursorchanged, self,
			pianoroll_on_cursor_changed);
	}
	psy_audio_sequencecursor_init(&self->grid.old_cursor);
}

void pianoroll_on_grid_scroll(Pianoroll* self, psy_ui_Component* sender)
{
	assert(self);

	if (psy_ui_component_scroll_left_px(pianogrid_base(&self->grid)) !=
		psy_ui_component_scroll_left_px(pianoruler_base(&self->header))) {
		psy_ui_component_set_scroll_left(pianoruler_base(&self->header),
			psy_ui_component_scroll_left(pianogrid_base(&self->grid)));		
	}
	if (psy_ui_component_scroll_top_px(pianogrid_base(&self->grid)) !=
			psy_ui_component_scroll_top_px(&self->keyboard.component)) {
		self->keyboard.component.blitscroll = TRUE;
		psy_ui_component_set_scroll_top(&self->keyboard.component,
			psy_ui_component_scroll_top(pianogrid_base(&self->grid)));
		self->keyboard.component.blitscroll = FALSE;
	}
}

void pianoroll_on_beat_width_changed(Pianoroll* self, ZoomBox* sender)
{
	assert(self);

	pianogridstate_setzoom(&self->gridstate, zoombox_rate(sender));
	psy_ui_component_set_scroll_step_width(pianogrid_base(&self->grid),
		psy_ui_value_make_px(pianogridstate_steppx(&self->gridstate)));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_set_scroll_left(pianoruler_base(&self->header),
		psy_ui_component_scroll_left(pianogrid_base(&self->grid)));
	psy_ui_component_align(&self->top);
	psy_ui_component_invalidate(pianoruler_base(&self->header));
	psy_ui_component_update(pianoruler_base(&self->header));
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_invalidate(&self->scroller.pane);
}

void pianoroll_on_key_height_changed(Pianoroll* self, ZoomBox* sender)
{
	assert(self);

	self->keyboardstate.keyheight = psy_ui_mul_value_real(
		self->keyboardstate.defaultkeyheight, zoombox_rate(sender));
	keyboardstate_update_metrics(&self->keyboardstate,
		psy_ui_component_textmetric(&self->component));
	psy_ui_component_set_scroll_step_height(pianogrid_base(&self->grid),
		psy_ui_value_make_px(self->keyboardstate.keyheightpx));
	psy_ui_component_set_scroll_step_height(pianokeyboard_base(&self->keyboard),
		psy_ui_value_make_px(self->keyboardstate.keyheightpx));
	psy_ui_component_updateoverflow(pianogrid_base(&self->grid));
	psy_ui_component_set_scroll_top(&self->keyboard.component,
		psy_ui_component_scroll_top(pianogrid_base(&self->grid)));
	psy_ui_component_invalidate(&self->keyboard.component);
	psy_ui_component_update(&self->keyboard.component);	
	psy_ui_component_align(&self->scroller.pane);
	psy_ui_component_invalidate(&self->scroller.pane);
}

void pianoroll_on_keytype_changed(Pianoroll* self, psy_ui_Component* sender, intptr_t index)
{
	assert(self);

	if (index >= 0 && index < KEYBOARDTYPE_NUM) {
		pianokeyboard_set_keyboard_type(&self->keyboard, (KeyboardType)index);
		psy_ui_component_align(pianoroll_base(self));
	}	
}

void pianoroll_on_edit_mode(Pianoroll* self, psy_ui_Button* sender)
{
	self->grid.edit_mode = TRUE;
	psy_ui_button_highlight(sender);
	psy_ui_button_disable_highlight(&self->bar.select_mode);
}

void pianoroll_on_select_mode(Pianoroll* self, psy_ui_Button* sender)
{
	self->grid.edit_mode = FALSE;
	psy_ui_button_highlight(sender);
	psy_ui_button_disable_highlight(&self->bar.edit_mode);
}

void pianoroll_on_display_all_tracks(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid, PIANOROLL_TRACK_DISPLAY_ALL);
	pianoroll_update_track_display_buttons(self);
}

void pianoroll_on_display_current_track(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid, PIANOROLL_TRACK_DISPLAY_CURRENT);
	pianoroll_update_track_display_buttons(self);
}

void pianoroll_on_display_active_tracks(Pianoroll* self, psy_ui_Button* sender)
{
	assert(self);

	pianogrid_settrackdisplay(&self->grid, PIANOROLL_TRACK_DISPLAY_ACTIVE);
	pianoroll_update_track_display_buttons(self);
}

void pianoroll_update_track_display_buttons(Pianoroll* self)
{
	assert(self);

	psy_ui_button_disable_highlight(&self->bar.tracks_all);
	psy_ui_button_disable_highlight(&self->bar.track_curr);
	psy_ui_button_disable_highlight(&self->bar.tracks_active);
	switch (pianogrid_trackdisplay(&self->grid)) {
		case PIANOROLL_TRACK_DISPLAY_ALL:
			psy_ui_button_highlight(&self->bar.tracks_all);			
			break;
		case PIANOROLL_TRACK_DISPLAY_CURRENT:
			psy_ui_button_highlight(&self->bar.track_curr);
			break;
		case PIANOROLL_TRACK_DISPLAY_ACTIVE:			
			psy_ui_button_highlight(&self->bar.tracks_active);
			break;
		default:			
			break;
	}	
}

void pianoroll_make_cmds(psy_Property* parent)
{
	psy_Property* cmds;

	assert(parent);

	cmds = psy_property_settext(psy_property_append_section(parent,
		"pianoroll"), "Pianoroll");
	set_cmd(cmds, CMD_NAVUP,
		psy_ui_KEY_UP, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navup", "up");	
	set_cmd(cmds, CMD_NAVDOWN,
		psy_ui_KEY_DOWN, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navdown", "down");
	set_cmd(cmds, CMD_NAVLEFT,
		psy_ui_KEY_LEFT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navleft", "left");
	set_cmd(cmds, CMD_NAVRIGHT,
		psy_ui_KEY_RIGHT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navright", "right");
	set_cmd(cmds, CMD_NAVPAGEUP,
		psy_ui_KEY_PRIOR, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navpageup", "pageup");
	set_cmd(cmds, CMD_NAVPAGEDOWN,
		psy_ui_KEY_NEXT, psy_SHIFT_OFF, psy_CTRL_OFF,
		"navpagedown", "pagedown");
	set_cmd(cmds, CMD_NAVPAGEUPKEYBOARD,
		psy_ui_KEY_PRIOR, psy_SHIFT_ON, psy_CTRL_OFF,
		"navpageupkbd", "pageup kbd");
	set_cmd(cmds, CMD_NAVPAGEDOWNKEYBOARD,
		psy_ui_KEY_NEXT, psy_SHIFT_ON, psy_CTRL_OFF,
		"navpagedownkbd", "pagedown kbd");
	set_cmd(cmds, CMD_ENTER,
		psy_ui_KEY_SPACE, psy_SHIFT_OFF, psy_CTRL_OFF,
		"enter", "enter");
	set_cmd(cmds, CMD_ROWCLEAR,
		psy_ui_KEY_DELETE, psy_SHIFT_OFF, psy_CTRL_OFF,
		"rowclear", "clr row");
	set_cmd(cmds, CMD_BLOCKSTART,
		psy_ui_KEY_B, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockstart", "sel start");
	set_cmd(cmds, CMD_BLOCKEND,
		psy_ui_KEY_E, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockend", "sel end");
	set_cmd(cmds, CMD_BLOCKUNMARK,
		psy_ui_KEY_U, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockunmark", "unmark");
	set_cmd(cmds, CMD_BLOCKCUT,
		psy_ui_KEY_X, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcut", "cut");
	set_cmd(cmds, CMD_BLOCKCOPY,
		psy_ui_KEY_C, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockcopy", "copy");
	set_cmd(cmds, CMD_BLOCKPASTE,
		psy_ui_KEY_V, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockpaste", "paste");
	set_cmd(cmds, CMD_BLOCKMIX,
		psy_ui_KEY_M, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockmix", "mix");
	set_cmd(cmds, CMD_BLOCKDELETE,
		0, 0, 0,
		"blockdelete", "blkdel");

	set_cmd(cmds, CMD_SELECTALL,
		psy_ui_KEY_A, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectall", "sel all");
	set_cmd(cmds, CMD_SELECTBAR,
		psy_ui_KEY_K, psy_SHIFT_OFF, psy_CTRL_ON,
		"selectbar", "sel bar");

	set_cmd(cmds, CMD_BLOCKUNMARK,
		psy_ui_KEY_U, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockunmark", "unmark");
	set_cmd(cmds, CMD_BLOCKSTART,
		psy_ui_KEY_B, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockstart", "sel start");
	set_cmd(cmds, CMD_BLOCKEND,
		psy_ui_KEY_E, psy_SHIFT_OFF, psy_CTRL_ON,
		"blockend", "sel end");
}

/*
** Defines a property with shortcut defaults for the keyboard driver
** key		: cmd id used by the trackerview
** text		: "cmds.key" language dictionary key used by the translator
** shorttext: short description for the keyboard help view
** value	: encoded key shortcut (keycode/shift/ctrl)
*/
void set_cmd(psy_Property* cmds, uintptr_t cmd, uint32_t keycode, bool shift,
	bool ctrl, const char* key, const char* shorttext)
{
	char text[256];

	assert(cmds);

	psy_snprintf(text, 256, "cmds.%s", key);
	psy_property_sethint(psy_property_settext(psy_property_setshorttext(
		psy_property_set_id(psy_property_append_int(cmds, key,
			psy_audio_encodeinput(keycode, shift, ctrl, 0, 0), 0, 0),
			cmd), shorttext), text), PSY_PROPERTY_HINT_SHORTCUT);
}

void pianoroll_on_configure(Pianoroll* self, PatternViewConfig* config,
	psy_Property* property)
{
	if (patternviewconfig_is_smooth_scrolling(config)) {
		psy_ui_scroller_scroll_smooth(&self->scroller);
	} else {
		psy_ui_scroller_scroll_fast(&self->scroller);
	}
}
