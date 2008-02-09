/* -*- mode:c++, indent-tabs-mode:t -*- */
#include <packageneric/pre-compiled.private.hpp>
#include "drum.hpp"

Drum::Drum()
{
	AmpEnvStage=ST_NONOTE;
	Started=false;
}

Drum::~Drum()
{

}

void Drum::NoteOn(int note, DrumPars* pars)
{
	double pow2=pow(2.0,(note-48)/12.0);
	Drummy.setIncSpeed(pars->StartSpeed*pow2);
	Thump.setSampleRate(pars->samplerate);
	Thump.setHz(pars->ThumpFreq*pow2);
	IncSpeed=pars->IncSpeed*pow2;

	Spos=0;
	Slength=pars->SLength;
	DecLength=(int)(pars->DecLength);
	ThumpLength=pars->ThumpLength;

	DecMode=pars->DecMode;
	if ( DecMode == 1 ) DecConstant=1.5/DecLength;
	else if ( DecMode == 2 ) DecConstant=1.5*DecLength;

	if (pars->AttackPos==0) {
		SinVol=pars->SinVol; VolChangePos=pars->DecayPos; SinInc=(-1)*pars->DecayDec;
	}
	else {
		SinVol=0; VolChangePos=pars->AttackPos; SinInc=pars->AttackInc;
	}
	VolDecay=pars->DecayPos;

	VolDecayDec=(-1)*pars->DecayDec;
	VolSustainDec=(-1)*pars->SustainDec;

	ThumpVol=pars->ThumpVol;
	ThumpDec=pars->ThumpDec;

	OutVol = pars->OutVol;
	Drummy.setEnvPos(0);
	Thump.setEnvPos(0);
	AmpEnvStage=ST_NOTESUSTAIN;

}

void Drum::NoteOff()
{
	if ( AmpEnvStage == ST_NOTESUSTAIN )
	{
		AmpEnvStage=ST_NOTERELEASE;
		VolDec=OutVol/256.0;
	}
}
