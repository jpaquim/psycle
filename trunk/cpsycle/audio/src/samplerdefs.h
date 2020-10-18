// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_SAMPLERDEFS_H
#define psy_audio_SAMPLERDEFS_H

// container
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Version for the sampler machine data.
// The instruments and sample bank versions are saved with the song chunk
// versioning
#define XMSAMPLER_VERSION 0x00010002
// Version zero was the development version (no format freeze). Version one is
// the published one.
#define XMSAMPLER_VERSION_ONE 0x00010000
// Version Sampler PS1
#define SAMPLERVERSION 0x00000003

#define SAMPLERHELP\
	"Track Commands :""\n"\
		"	01xx : Portamento Up(Fx : fine, Ex : Extra fine)""\n"\
		"	02xx : Portamento Down(Fx : fine, Ex : Extra fine)""\n"\
		"	03xx : Tone Portamento""\n"\
		"	04xy : Vibrato with speed y and depth x""\n"\
		"	05xx : Continue Portamento and Volume Slide with speed xx""\n"\
		"	06xx : Continue Vibrato and Volume Slide with speed xx""\n"\
		"	07xx : Tremolo""\n"\
		"	08xx : Pan. 0800 Left 08FF right""\n"\
		"	09xx: Panning slide x0 Left, 0x Right""\n"\
		"	0Axx: Channel Volume, 00 = Min, 40 = Max""\n"\
		"	0Bxx : Channel VolSlide x0 Up(xF fine), 0x Down(Fx Fine)""\n"\
		"	0Cxx: Volume(0C80 : 100 %)""\n"\
		"	0Dxx : Volume Slide x0 Up(xF fine), 0x Down(Fx Fine)""\n"\
		"	0Exy: Extended(see below).""\n"\
		"	0Fxx : Filter.""\n"\
		"	10xy : Arpeggio with note, note + x and note + y""\n"\
		"	11xy : Retrig note after y ticks""\n"\
		"	14xx : Fine Vibrato with speed y and depth x""\n"\
		"	17xy : Tremor Effect(ontime x, offtime y)""\n"\
		"	18xx : Panbrello""\n"\
		"	19xx : Set Envelope position(in ticks)""\n"\
		"	1Cxx : Global Volume, 00 = Min, 80 = Max""\n"\
		"	1Dxx : Global Volume Slide x0 Up(xF fine), 0x Down(Fx Fine)""\n"\
		"	1Exx: Send xx to volume colum(see below)""\n"\
		"	9xxx : Sample Offset x * 256"\
		"""\n"\
		"Extended Commands :""\n"\
		"	30 / 1 : Glissando mode Off / on""\n"\
		"	4x : Vibrato Wave""\n"\
		"	5x : Panbrello Wave""\n"\
		"	7x : Tremolo Wave""\n"\
		"	Waves : 0 : Sinus, 1 : Square""\n"\
		"	2 : Ramp Up, 3 : Ramp Down, 4 : Random""\n"\
		"	8x : Panning""\n"\
		"	90 : Surround Off""\n"\
		"	91 : Surround On""\n"\
		"	9E : Play Forward""\n"\
		"	9F : Play Backward""\n"\
		"	Cx : Delay NoteCut by x ticks""\n"\
		"	Dx : Delay New Note by x ticks""\n"\
		"	E0 : Notecut background notes""\n"\
		"	E1 : Noteoff background notes""\n"\
		"	E2 : NoteFade background notes""\n"\
		"	E3 : Set NNA NoteCut for this voice""\n"\
		"	E4 : Set NNA NoteContinue for this voice""\n"\
		"	E5 : Set NNA Noteoff for this voice""\n"\
		"	E6 : Set NNA NoteFade for this channel""\n"\
		"	E7 / 8 : Disable / Enable Volume Envelope""\n"\
		"	E9 / A : Disable / Enable Pan Envelope""\n"\
		"	EB / C : Disable / Enable Pitch / Filter Envelope""\n"\
		"	Fx : Set Filter Mode.""\n"\
		"	""\n"\
		"Volume Column : ""\n"\
		"	00..3F : Set volume to x * 2""\n"\
		"	4x : Volume slide up""\n"\
		"	5x : Volume slide down""\n"\
		"	6x : Fine Volslide up""\n"\
		"	7x : Fine Volslide down""\n"\
		"	8x : Panning(0:Left, F : Right)""\n"\
		"	9x : PanSlide Left""\n"\
		"	Ax : PanSlide Right""\n"\
		"	Bx : Vibrato""\n"\
		"	Cx : TonePorta""\n"\
		"	Dx : Pitch slide up""\n"\
		"	Ex : Pitch slide down""\n"


#define SAMPLER_DEFAULT_POLYPHONY	8
	
/*
	* = remembers its last value when called with param 00.
	t = slides/changes each tick. (or is applied in a specific tick != 0 )
	p = persistent ( a new note doesn't reset it )
	n = they need to appear next to a note.
*/
#define	XM_SAMPLER_CMD_NONE                 0x00	
#define XM_SAMPLER_CMD_PORTAMENTO_UP        0x01 // Portamento Up , Fine porta (01Fx), and Extra fine porta (01Ex)	(*t)
#define XM_SAMPLER_CMD_PORTAMENTO_DOWN      0x02 // Portamento Down, Fine porta (02Fx), and Extra fine porta (02Ex) (*t)
#define XM_SAMPLER_CMD_PORTA2NOTE           0x03 // Tone Portamento						(*tn)
#define XM_SAMPLER_CMD_VIBRATO              0x04 // Do Vibrato							(*t)
#define XM_SAMPLER_CMD_TONEPORTAVOL         0x05 // Tone Portament & Volume Slide		(*t)
#define XM_SAMPLER_CMD_VIBRATOVOL           0x06 // Vibrato & Volume Slide				(*t)
#define XM_SAMPLER_CMD_TREMOLO              0x07 // Tremolo							(*t)
#define XM_SAMPLER_CMD_PANNING              0x08 // Set Panning Position				(p)
#define XM_SAMPLER_CMD_PANNINGSLIDE         0x09 // Panning slide						(*t)
#define XM_SAMPLER_CMD_SET_CHANNEL_VOLUME   0x0A // Set channel's volume				(p)
#define XM_SAMPLER_CMD_CHANNEL_VOLUMESLIDE  0x0B // channel Volume Slide up (0By0) down (0B0x), Fine slide up(0BFy) down(0BxF)	 (*tp)
#define XM_SAMPLER_CMD_VOLUME               0x0C // Set Volume
#define XM_SAMPLER_CMD_VOLUMESLIDE          0x0D // Volume Slide up (0Dy0), down (0D0x), Fine slide up(0DyF), down(0DFy)	 (*t)
#define XM_SAMPLER_CMD_FINESLIDEUP          0x0F // Part of the value that indicates it is a fine slide up
#define XM_SAMPLER_CMD_FINESLIDEDOWN        0xF0 // Part of the value that indicates it is a fine slide down
#define XM_SAMPLER_CMD_EXTENDED             0x0E // Extend Command
#define XM_SAMPLER_CMD_MIDI_MACRO           0x0F // Impulse Tracker MIDI macro			(p)
#define XM_SAMPLER_CMD_ARPEGGIO             0x10 // Arpeggio							(*t)
#define XM_SAMPLER_CMD_RETRIG               0x11 // Retrigger Note						(*t)
#define XM_SAMPLER_CMD_FINE_VIBRATO         0x14 // Vibrato 4 times finer				(*t)
#define XM_SAMPLER_CMD_TREMOR               0x17 // Tremor								(*t)
#define XM_SAMPLER_CMD_PANBRELLO            0x18 // Panbrello							(*t)
#define XM_SAMPLER_CMD_SET_ENV_POSITION     0x19 // Set Envelope Position
#define XM_SAMPLER_CMD_SET_GLOBAL_VOLUME    0x1C // Sets Global Volume
#define XM_SAMPLER_CMD_GLOBAL_VOLUME_SLIDE  0x1D // Slides Global Volume				(*t)
#define XM_SAMPLER_CMD_SENDTOVOLUME         0x1E // Interprets this as a volume command	()
#define XM_SAMPLER_CMD_OFFSET               0x90 // Set Sample Offset  , note!: 0x9yyy ! not 0x90yy (*n)		PORTAMENTO_UP = 0x01,// Portamento Up , Fine porta (01Fx), and Extra fine porta (01Ex)	(*t)

#define XM_SAMPLER_CMD_E_GLISSANDO_TYPE     0x30 // E3     Set gliss control           (p)
#define XM_SAMPLER_CMD_E_VIBRATO_WAVE       0x40 // E4     Set vibrato control         (p)
#define XM_SAMPLER_CMD_E_PANBRELLO_WAVE     0x50 //                                    (p)
//0x60
#define XM_SAMPLER_CMD_E_TREMOLO_WAVE       0x70 // E7     Set tremolo control         (p)
#define XM_SAMPLER_CMD_E_SET_PAN            0x80 //                                    (p)
#define XM_SAMPLER_CMD_E9	                 0x90
//0xA0,
//0xB0,
#define XM_SAMPLER_CMD_E_DELAYED_NOTECUT    0xC0 // EC     Note cut                    (t)
#define XM_SAMPLER_CMD_E_NOTE_DELAY         0xD0 // ED     Note delay                  (tn)
#define XM_SAMPLER_CMD_EE                   0xE0
#define XM_SAMPLER_CMD_E_SET_MIDI_MACRO     0xF0 //                                    (p)

#define	XM_SAMPLER_CMD_E9_SURROUND_OFF      0x00 //									(p)
#define	XM_SAMPLER_CMD_E9_SURROUND_ON       0x01 //									(p)
#define	XM_SAMPLER_CMD_E9_REVERB_OFF        0x08 //									(p)
#define	XM_SAMPLER_CMD_E9_REVERB_FORCE      0x09 //									(p)
#define	XM_SAMPLER_CMD_E9_STANDARD_SURROUND 0x0A //									(p)
#define	XM_SAMPLER_CMD_E9_QUAD_SURROUND     0x0B // (p)Select quad surround mode: this allows you to pan in the rear channels, especially useful for 4-speakers playback. Note that S9A and S9B do not activate the surround for the current channel, it is a global setting that will affect the behavior of the surround for all channels. You can enable or disable the surround for individual channels by using the S90 and S91 effects. In quad surround mode, the channel surround will stay active until explicitely disabled by a S90 effect
#define	XM_SAMPLER_CMD_E9_GLOBAL_FILTER     0x0C // (p)Select global filter mode (IT compatibility). This is the default, when resonant filters are enabled with a Zxx effect, they will stay active until explicitely disabled by setting the cutoff frequency to the maximum (Z7F), and the resonance to the minimum (Z80).
#define	XM_SAMPLER_CMD_E9_LOCAL_FILTER      0x0D // (p)Select local filter mode (MPT beta compatibility): when this mode is selected, the resonant filter will only affect the current note. It will be deactivated when a new note is being played.
#define	XM_SAMPLER_CMD_E9_PLAY_FORWARD      0x0E // Play forward. You may use this to temporarily force the direction of a bidirectional loop to go forward.
#define	XM_SAMPLER_CMD_E9_PLAY_BACKWARD     0x0F // Play backward. The current instrument will be played backwards, or it will temporarily set the direction of a loop to go backward. 


#define	XM_SAMPLER_CMD_VOL_VOLUME0			0x00  // 0x00..0x0F (63)  ||
#define	XM_SAMPLER_CMD_VOL_VOLUME1			0x10  // 0x10..0x1F (63)  || All are the same command.
#define	XM_SAMPLER_CMD_VOL_VOLUME2			0x20  // 0x20..0x2F (63)  ||
#define	XM_SAMPLER_CMD_VOL_VOLUME3			0x30  // 0x30..0x3F (63)  ||
#define	XM_SAMPLER_CMD_VOL_VOLSLIDEUP		0x40  // 0x40..0x4F (16)
#define	XM_SAMPLER_CMD_VOL_VOLSLIDEDOWN	0x50  // 0x50..0x5F (16)
#define	XM_SAMPLER_CMD_VOL_FINEVOLSLIDEUP	0x60  // 0x60..0x6F (16)
#define	XM_SAMPLER_CMD_VOL_FINEVOLSLIDEDOWN 0x70 // 0x70..0x7F (16)
#define	XM_SAMPLER_CMD_VOL_PANNING			0x80  // 0x80..0x8F (16)
#define	XM_SAMPLER_CMD_VOL_PANSLIDELEFT	0x90  // 0x90..0x9F (16)
#define	XM_SAMPLER_CMD_VOL_PANSLIDERIGHT	0xA0  // 0xA0..0xAF (16)
#define	XM_SAMPLER_CMD_VOL_VIBRATO			0xB0  // 0xB0..0xBF (16) Linked to Vibrato Vy = 4xy 
#define	XM_SAMPLER_CMD_VOL_TONEPORTAMENTO	0xC0  // 0xC0..0xCF (16) Linked to Porta2Note 
#define	XM_SAMPLER_CMD_VOL_PITCH_SLIDE_UP	0xD0  // 0xD0..0xDF (16)
#define	XM_SAMPLER_CMD_VOL_PITCH_SLIDE_DOWN 0xE0 // 0xE0..0xEF (16)
											   // 0xFF -> Blank.
	
	
#define ISSLIDEUP(val) !((val)&0x0F)
#define ISSLIDEDOWN(val) !((val)&0xF0)
#define ISFINESLIDEUP(val) (((val)&0x0F)==XM_SAMPLER_CMD_FINESLIDEUP)
#define ISFINESLIDEDOWN(val) (((val)&0xF0)==XM_SAMPLER_CMD_FINESLIDEDOWN)
#define GETSLIDEUPVAL(val) (((val)&0xF0)>>4)
#define GETSLIDEDOWNVAL(val) ((val)&0x0F)

#define XM_SAMPLER_EFFECT_VIBRATO  0x00000001
#define XM_SAMPLER_EFFECT_PITCHSLIDE  0x00000002
#define XM_SAMPLER_EFFECT_CHANNELVOLSLIDE  0x00000004
#define XM_SAMPLER_EFFECT_SLIDE2NOTE  0x00000008
#define XM_SAMPLER_EFFECT_VOLUMESLIDE  0x00000010
#define XM_SAMPLER_EFFECT_PANSLIDE  0x00000020
#define XM_SAMPLER_EFFECT_TREMOLO  0x00000040
#define XM_SAMPLER_EFFECT_ARPEGGIO  0x00000080
#define XM_SAMPLER_EFFECT_NOTECUT  0x00000100
#define XM_SAMPLER_EFFECT_PANBRELLO  0x00000200
#define XM_SAMPLER_EFFECT_RETRIG  0x00000400
#define XM_SAMPLER_EFFECT_TREMOR  0x00000800
#define XM_SAMPLER_EFFECT_NOTEDELAY  0x00001000
#define XM_SAMPLER_EFFECT_GLOBALVOLSLIDE  0x00002000
#define XM_SAMPLER_EFFECT_PORTAMENTO  0x00004000

typedef enum psy_audio_XMSamplerCmdMode {
	// *= remembers its last value when called with param 00.
	psy_audio_SAMPLERCMDMODE_MEM0 = 1,
	// t = slides / changes each tick. (or is applied in a specific tick != 0)
	psy_audio_SAMPLERCMDMODE_TICK = 2,
	// p = persistent(a new note doesn't reset it )	
	psy_audio_SAMPLERCMDMODE_PERS = 4,
	// n = they need to appear next to a note.
	psy_audio_SAMPLERCMDMODE_NEXT = 8  
} psy_audio_XMSamplerCmdMode;

typedef struct SamplerCmd {
	int id;
	int patternid;
	int mode;
} psy_audio_XMSamplerCmd;

void psy_audio_xmsamplercmd_init_all(psy_audio_XMSamplerCmd*,
	int id, int patternid, int mask);
void psy_audio_xmsamplercmd_dispose(psy_audio_XMSamplerCmd*);

INLINE psy_audio_XMSamplerCmd psy_audio_xmsamplercmd_make(int id, int patternid, int mask)
{
	psy_audio_XMSamplerCmd rv;

	rv.id = id;
	rv.patternid = patternid;
	rv.mode = mask;
	return rv;
}

INLINE int psy_audio_xmsamplercmd_id(psy_audio_XMSamplerCmd* self)
{
	return self->id;
}

INLINE bool psy_audio_xmsamplercmd_hasticktime(const psy_audio_XMSamplerCmd* self)
{
	return ((self->mode & psy_audio_SAMPLERCMDMODE_TICK)
		== psy_audio_SAMPLERCMDMODE_TICK);
}

psy_audio_XMSamplerCmd* psy_audio_xmsamplercmd_alloc(void);
psy_audio_XMSamplerCmd* psy_audio_xmsamplercmd_allocinit_all(int id,
	int patternid, int mask);

typedef enum {
	INTERPOL_NONE = 0,
	INTERPOL_LINEAR = 1,
	INTERPOL_SPLINE = 2
} InterpolationType;

typedef enum {
	psy_audio_PANNING_LINEAR = 0,
	psy_audio_PANNING_TWOWAY = 1,
	psy_audio_PANNING_EQUALPOWER = 2
} psy_audio_XMSamplerPanningMode;

typedef struct ZxxMacro {
	int32_t mode;
	int32_t value;
} ZxxMacro;

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_SAMPLERDEFS_H */
