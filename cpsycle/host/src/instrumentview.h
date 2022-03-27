/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(INSTRUMENTVIEW_H)
#define INSTRUMENTVIEW_H

/* host */
#include "envelopeview.h"
#include "instrumentsbox.h"
#include "instrumentampview.h"
#include "instrumentfilterview.h"
#include "instrumentgenview.h"
#include "instrumentpanview.h"
#include "instrumentpitchview.h"
#include "workspace.h"
/* ui */
#include <uitabbar.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Instrument Editor */

struct InstrumentView;

typedef struct VirtualGeneratorsBox {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_CheckBox active;
	psy_ui_ComboBox generators;
	psy_ui_Label on;
	psy_ui_ComboBox samplers;
	/* references */
	Workspace* workspace;
} VirtualGeneratorsBox;

void virtualgeneratorbox_init(VirtualGeneratorsBox*, psy_ui_Component* parent,
	Workspace*);
void virtualgeneratorbox_updatesamplers(VirtualGeneratorsBox*);
void virtualgeneratorbox_updategenerators(VirtualGeneratorsBox*);
void virtualgeneratorbox_update(VirtualGeneratorsBox*);

/* InstrumentPredefsBar */
typedef struct InstrumentPredefsBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label predefs;	
	psy_ui_Button predef_1;
	psy_ui_Button predef_2;
	psy_ui_Button predef_3;
	psy_ui_Button predef_4;
	psy_ui_Button predef_5;
	psy_ui_Button predef_6;
	/* references */
	psy_audio_Instrument* instrument;
	struct InstrumentView* view;
	Workspace* workspace;
} InstrumentPredefsBar;

void instrumentpredefsbar_init(InstrumentPredefsBar*, psy_ui_Component* parent,
	psy_audio_Instrument* instrument, struct InstrumentView* view, Workspace*);

/* InstrumentHeaderView */
typedef struct InstrumentHeaderView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label namelabel;
	psy_ui_TextArea nameedit;	
	psy_ui_Button prevbutton;
	psy_ui_Button nextbutton;
	psy_ui_Component more;
	VirtualGeneratorsBox virtualgenerators;
	InstrumentPredefsBar predefs;
	/* references */
	struct InstrumentView* view; /* todo remove view reference */
	psy_audio_Instrument* instrument;
	psy_audio_Instruments* instruments;
} InstrumentHeaderView;

void instrumentheaderview_init(InstrumentHeaderView*, psy_ui_Component* parent,
	psy_audio_Instruments*, struct InstrumentView*, Workspace*);
void instrumentheaderview_setinstrument(InstrumentHeaderView*,
	psy_audio_Instrument*);

typedef struct InstrumentViewButtons {
	// inherits
	psy_ui_Component component;
	// ui elements	
	psy_ui_Button create;
	psy_ui_Button load;
	psy_ui_Button save;	
	psy_ui_Button duplicate;
	psy_ui_Button del;
} InstrumentViewButtons;

void instrumentviewbuttons_init(InstrumentViewButtons*,
	psy_ui_Component* parent, Workspace*);

/* InstrumentViewBar */
typedef struct InstrumentsViewBar {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label status;
	/* references */
	Workspace* workspace;
} InstrumentsViewBar;

void instrumentsviewbar_init(InstrumentsViewBar*, psy_ui_Component* parent, Workspace*);
void instrumentsviewbar_settext(InstrumentsViewBar*, const char* text);

INLINE psy_ui_Component* instrumentsviewbar_base(InstrumentsViewBar* self)
{
	return &self->component;
}

// InstrumentView
typedef struct InstrumentView {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Component viewtabbar;
	psy_ui_Notebook notebook;
	psy_ui_TabBar tabbar;
	psy_ui_Component left;
	psy_ui_Label label;
	InstrumentViewButtons buttons;
	InstrumentsBox instrumentsbox;
	psy_ui_Notebook clientnotebook;
	psy_ui_Label empty;
	psy_ui_Component client;
	InstrumentHeaderView header;
	InstrumentGeneralView general;
	InstrumentVolumeView volume;
	InstrumentPanView pan;
	InstrumentFilterView filter;
	InstrumentPitchView pitch;
	// references
	InstrumentsViewBar* statusbar;
	psy_audio_Player* player;
	Workspace* workspace;
} InstrumentView;

void instrumentview_init(InstrumentView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

INLINE void instrumentsview_setstatusbar(InstrumentView* self,
	InstrumentsViewBar* statusbar)
{
	self->statusbar = statusbar;
}

INLINE psy_ui_Component* instrumentview_base(InstrumentView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* INSTRUMENTVIEW_H */
