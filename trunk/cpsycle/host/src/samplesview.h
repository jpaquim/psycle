/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(SAMPLESVIEW_H)
#define SAMPLESVIEW_H

/* host */
#include <uitabbar.h>
#include "samplesbox.h"
#include "sampleeditor.h"
#include "wavebox.h"
#include "workspace.h"
/* dsp */
#include <notestab.h>
/* ui */
#include <uibutton.h>
#include <uicombobox.h>
#include <uitextarea.h>
#include <uilabel.h>
#include <uinotebook.h>
#include <uislider.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SamplesView */

struct SamplesView;

/* SamplesHeaderView */
typedef struct SamplesHeaderView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label namelabel;
	psy_ui_TextArea nameedit;
	psy_ui_Button prevbutton;
	psy_ui_Button nextbutton;
	psy_ui_Label srlabel;
	psy_ui_TextArea sredit;
	psy_ui_Label numsamplesheaderlabel;
	psy_ui_Label numsampleslabel;
	psy_ui_Label channellabel;
	/* references */
	struct SamplesView* view;
	psy_audio_Sample* sample;
	psy_audio_Instruments* instruments;
} SamplesHeaderView;

void samplesheaderview_init(SamplesHeaderView*, psy_ui_Component* parent,
	psy_audio_Instruments*, struct SamplesView*, Workspace*);

void samplesheaderview_setsample(SamplesHeaderView*, psy_audio_Sample*);

INLINE psy_ui_Component* samplesheaderview_base(SamplesHeaderView* self)
{
	return &self->component;
}

/* SamplesGeneralView */
typedef struct SamplesGeneralView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Slider defaultvolume;
	psy_ui_Slider globalvolume;
	psy_ui_Slider panposition;
	psy_ui_Slider samplednote; 
	psy_ui_Slider pitchfinetune;	
	psy_dsp_NotesTabMode notes_tab_mode;
	/* references */
	psy_audio_Sample* sample;
} SamplesGeneralView;

void samplesgeneralview_init(SamplesGeneralView*, psy_ui_Component* parent,
	Workspace*);

INLINE psy_ui_Component* samplesgeneralview_base(SamplesGeneralView* self)
{
	return &self->component;
}

/* SamplesVibratoView */
typedef struct SamplesVibratoView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component header;
	psy_ui_Label waveformheaderlabel;
	psy_ui_ComboBox waveformbox;
	psy_ui_Slider attack;
	psy_ui_Slider speed;
	psy_ui_Slider depth;
	/* references */
	psy_audio_Player* player;
	psy_audio_Sample* sample;
} SamplesVibratoView;

void samplesvibratoview_init(SamplesVibratoView*, psy_ui_Component* parent,
	psy_audio_Player* player);

INLINE psy_ui_Component* samplesvibratoview_base(SamplesVibratoView* self)
{
	return &self->component;
}

struct SamplesView;

/* SamplesLoopView */
typedef struct SamplesLoopView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component cont;
	psy_ui_Label loopheaderlabel;
	psy_ui_ComboBox loopdir;
	psy_ui_Label loopstartlabel;
	psy_ui_TextArea loopstartedit;
	psy_ui_Label loopendlabel;
	psy_ui_TextArea loopendedit;
	psy_ui_Component sustain;
	psy_ui_Label sustainloopheaderlabel;
	psy_ui_ComboBox sustainloopdir;
	psy_ui_Label sustainloopstartlabel;
	psy_ui_TextArea sustainloopstartedit;
	psy_ui_Label sustainloopendlabel;
	psy_ui_TextArea sustainloopendedit;	
	psy_audio_SampleLoop currloop;
	psy_audio_SampleLoop currsustainloop;
	/* references */
	struct SamplesView* view;
	psy_audio_Sample* sample;
} SamplesLoopView;

void samplesloopview_init(SamplesLoopView*, psy_ui_Component* parent,
	struct SamplesView*);

INLINE psy_ui_Component* samplesloopview_base(SamplesLoopView* self)
{
	return &self->component;
}

/* SamplesViewButtons */
typedef struct SamplesViewButtons {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Button load;
	psy_ui_Button save;
	psy_ui_Button duplicate;
	psy_ui_Button del;
} SamplesViewButtons;

void samplesviewbuttons_init(SamplesViewButtons*, psy_ui_Component* parent,
	Workspace*);

/* SamplesSongImportView */
typedef struct SamplesSongImportView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Component header;
	psy_ui_Label label;
	psy_ui_Label songname;
	psy_ui_Button browse;
	SamplesBox samplesbox;
	psy_ui_Component bar;
	psy_ui_Button add;
	WaveBox samplebox;
	psy_Property song_load;	
	/* references */
	psy_audio_Song* source;
	struct SamplesView* view;
	Workspace* workspace;	
} SamplesSongImportView;

void samplessongimportview_init(SamplesSongImportView*, psy_ui_Component* parent,
	struct SamplesView* view, Workspace*);

INLINE psy_ui_Component* samplessongimportview_base(SamplesSongImportView* self)
{
	return &self->component;
}

/* SamplesView */
typedef struct SamplesView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_TabBar clienttabbar;
	psy_ui_Notebook clientnotebook;
	psy_ui_Component mainview;
	psy_ui_Component importview;
	psy_ui_Notebook notebook;
	SamplesBox samplesbox;
	psy_ui_Component left;
	SamplesViewButtons buttons;	
	psy_ui_Component right;
	psy_ui_Component client;
	SamplesSongImportView songimport;
	SampleEditor sampleeditor;
	SamplesHeaderView header;
	psy_ui_TabBar tabbar;
	SamplesGeneralView general;
	SamplesVibratoView vibrato;
	psy_ui_Component loop;
	SamplesLoopView waveloop;	
	WaveBox wavebox;
	psy_Property sample_load;
	/* references */
	Workspace* workspace;
} SamplesView;

void samplesview_init(SamplesView*, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace*);

void samplesview_setsample(SamplesView*, psy_audio_SampleIndex);
void samplesview_connectstatusbar(SamplesView*);

INLINE psy_ui_Component* samplesview_base(SamplesView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* SAMPLESVIEW_H */
