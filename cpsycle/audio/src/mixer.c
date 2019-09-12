#include "mixer.h"

static void dispose(Mixer* self);
static int mode(Mixer* self) { return MACHMODE_FX; }

void mixer_init(Mixer* self)
{
	machine_init(&self->machine);
	self->machine.dispose = dispose;
	self->machine.mode = mode;
	buffer_init(&self->machine.inputs, 2);
	buffer_init(&self->machine.outputs, 2);	
}

void dispose(Mixer* self)
{		
	buffer_dispose(&self->machine.inputs);
	buffer_dispose(&self->machine.outputs);
	machine_dispose(&self->machine);
}

