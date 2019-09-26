#include "mixer.h"

#define TRUE 1
#define FALSE 0

static void mixer_dispose(Mixer* self);
static int mixer_mode(Mixer* self) { return MACHMODE_FX; }
static void mixer_seqtick(Mixer* self, int channel, const PatternEvent* event);
static unsigned int numinputs(Mixer*);
static unsigned int numoutputs(Mixer*);

static char* _psName;

void mixerinputchannel_init(MixerInputChannel* self)
{
	self->volume_=1.0f;
	self->panning_=0.5f;
	self->drymix_=1.0f;
	self->mute_= FALSE;
	self->dryonly_= FALSE;
	self->wetonly_= FALSE;		
}

void mixerreturnchannel_init(MixerReturnChannel* self)
{
	self->mastersend_= TRUE;
	self->volume_ = 1.0f;
	self->panning_ = 0.5f;
	self->mute_ = FALSE;	
}

void mixermasterchannel_init(MixerMasterChannel* self)
{
	self->volume_ = 1.0f;
	self->drywetmix_ = 0.5f;
	self->gain_ = 1.0f;
}

void mixer_init(Mixer* self, MachineCallback callback)
{
	machine_init(&self->machine, callback);
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
	self->machine.dispose = mixer_dispose;
	self->machine.seqtick = mixer_seqtick;
	self->machine.mode = mixer_mode;
	self->solocolumn_ = -1;	
}

void mixer_dispose(Mixer* self)
{		
	machine_dispose(&self->machine);
}

void mixer_seqtick(Mixer* self, int channel, const PatternEvent* event)
{	
	if(event->note == NOTECOMMANDS_TWEAK)
	{
		int nv = (event->cmd<<8)+event->parameter;
		int param =  event->inst; // translate_param(event->inst);

		// if(param < GetNumParams()) {
		//	SetParameter(param,nv);
		// }
	}
	else if(event->note == NOTECOMMANDS_TWEAKSLIDE)
	{
		//\todo: Tweaks and tweak slides should not be a per-machine thing, but rather be player centric.
		// doing simply "tweak" for now..
		int nv = (event->cmd<<8)+event->parameter;
		int param = event->inst; // translate_param(event->inst);

		//if(param < GetNumParams()) {
		//	SetParameter(param,nv);
		//}
	}
}


unsigned int numinputs(Mixer* self)
{
	return 2;
}

unsigned int numoutputs(Mixer* self)
{
	return 2;
}