#include <psycle/plugin_interface.hpp>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <cmath>

const int fftlen = 2048;
const int iir = 6;
const int inbuflen = fftlen * iir * 2;
float optimal[iir+1] = { 0, 0.50f, 0.40f, 0.32f, 0.30f, 0.28f, 0.24f };
const double BexPI = psycle::plugin_interface::pi / 2;

CMachineParameter const paraFreq = 
{ 
	"Phase Shift",
	"Phase Shift",
	1,
	512,
	MPF_STATE,
	256,
};

CMachineParameter const paraDiff = 
{ 
	"Differentiator",
	"Differentiator",
	0,
	512,
	MPF_STATE,
	0,
};

CMachineParameter const paraAmount = 
{ 
	"LFO / Freq",
	"LFO / Freq",
	0,
	512,
	MPF_STATE,
	384,
};

CMachineParameter const paraMode = 
{ 
	"Mode",
	"Mode",
	1,
	iir,
	MPF_STATE,
	1,
};

CMachineParameter const paraDry = 
{ 
	"Dry / Wet",
	"Dry / Wet",
	0,
	512,
	MPF_STATE,
	390,
};

CMachineParameter const paraRefresh = 
{ 
	"Lfo/Freq Speed",
	"Lfo/Freq Speed",
	1,
	32,
	MPF_STATE,
	8,
};

CMachineParameter const *pParameters[] = 
{ 
	// global
	&paraFreq,
	&paraDry,
	&paraRefresh,
	&paraMode,
	&paraAmount,
	&paraDiff,
};

int const pFreq = 0;
int const pDry = 1;
int const pRefresh = 2;
int const pMode = 3;
int const pAmount = 4;
int const pDiff = 5;
int const numVals = 6;

CMachineInfo const bexphase_info = 
{
	MI_VERSION,				
	EFFECT,
	numVals,
	pParameters,
	"DocBexter'S PhaZaR",
	"BexPhase!",
	"Simon Bucher",
	"About",
	3
};


class bexphase : public CMachineInterface
{
	public:
		bexphase();
		virtual ~bexphase();

		virtual void Init();
		virtual void SequencerTick();
		virtual void Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks);
		virtual bool DescribeValue(char* txt,int const param, int const value);
		virtual void Command();
		virtual void ParameterTweak(int par, int val);
		virtual void Stop();

	private:

		float inbufr[inbuflen], inbufl[inbuflen];
		float buffer[fftlen], altbuffer[fftlen];
		long int inpoint, counter;
		long int x, y, z;
		float help_r, help_l, dry, wet, freq, shift, amntfreq, amntlfo;
		int buflen, shiftcount, last_dir;
		float dlay_r, dlay_l, shift_r, shift_l, diff, undiff;
};

PSYCLE__PLUGIN__INSTANCIATOR(bexphase, bexphase_info)

bexphase::bexphase()
{
	Vals = new int[numVals];
	inpoint = 0;
	counter = 0;
	shift = 0; shiftcount = 0;
	for ( y = 0; y < inbuflen; y++ ) { inbufr[y] = 0; inbufl[y] = 0; }
	for ( x = 0; x < fftlen; x++ ) { buffer[x] = 0; altbuffer[x] = 0; }
	dry = optimal[0]; wet = 1 - dry;
	freq = 1; amntfreq = 0.75f; amntlfo = 0.25f;
	buflen = 0; last_dir = 0; dlay_r = 0; dlay_l = 0;
	shift_r = 0; shift_l = 0; diff = 0; undiff = 1;
}

bexphase::~bexphase()
{
	delete Vals;
}
	
void bexphase::Init()
{
};

void bexphase::SequencerTick()
{
	#if 0
	if ( ++counter >= Vals[pRefresh] )
	{
		if ( (inpoint - fftlen) < 0 ) y = inbuflen - ( fftlen - inpoint ); else y = inpoint - fftlen;
		for ( x = 0; x < fftlen; x++ ) { if ( (y+x) == inbuflen ) y = x * (-1); buffer[x] = inbufr[x+y]; }
		rfftw_one( forward, buffer, altbuffer );
		y = 1;
		for ( x = fftlen/64; x < fftlen/2; x++ )
		{
			if ( altbuffer[x] < 0 ) buffer[x] = altbuffer[x] * (-1); else buffer[x] = altbuffer[x];
			if ( buffer[x] > buffer[y] ) y = x;
		}
		shift_r = ( fftlen / (float)y ) * amntfreq;
		if ( last_dir == 0 ) shift_r += (fftlen/(3+iir)) * amntlfo;
		shift_r -= dlay_r; shift_r /= buflen;
		
		if ( (inpoint - fftlen) < 0 ) y = inbuflen - ( fftlen - inpoint ); else y = inpoint - fftlen;
		for ( x = 0; x < fftlen; x++ ) { if ( (y+x) == inbuflen ) y = x * (-1); buffer[x] = inbufl[x+y]; }
		rfftw_one( forward, buffer, altbuffer );
		y = 1;
		for ( x = fftlen/64; x < fftlen/2; x++ )
		{
			if ( altbuffer[x] < 0 ) buffer[x] = altbuffer[x] * (-1); else buffer[x] = altbuffer[x];
			if ( buffer[x] > buffer[y] ) y = x;
		}
		shift_l = ( fftlen / (float)y ) * amntfreq;
		if ( last_dir == 0 ) { shift_l += (fftlen/(3+iir)) * amntlfo; last_dir = 1; }
		else last_dir = 0;
		shift_l -= dlay_l; shift_l /= buflen;

		counter = 0;
	}
	#endif
}

void bexphase::Command()
{
	pCB->MessBox("original author: docbexter <docbexter@web.de> ; maintained by the psycledelics", ";)", 0);
}

void bexphase::ParameterTweak(int par, int val)
{
	if ( par == pRefresh ) { buflen = val * pCB->GetTickLength(); counter = val; last_dir = 1; }
	if ( par == pFreq ) { shift = (float)(val/443.396f); shift *= shift; shift = ((shift/(float)Vals[3]) - freq)/256.0f; shiftcount = 256; }
	if ( par == pDry ) 
	{ 
		if ( val < 384 )
		{
			optimal[0] = val / 384.0f;
			wet = optimal[0] * optimal[1];
		}
		else
		{
			if ( val < 448 ) wet = optimal[1];
			else wet = 1;
		}
		dry = 1 - wet;
	}
	if ( par == pMode ) { freq *= Vals[pMode] / (float)val; if ( wet == optimal[1] ) { wet = optimal[val]; dry = 1 - wet; }; }
	if ( par == pAmount ) { amntfreq = val / 512.0f; amntlfo = 1 - amntfreq; }
	if ( par == pDiff ) { diff = val / 512.0f; undiff = 1 - diff; }
	Vals[par] = val;
}

void bexphase::Stop() 
{
	shift_r = 0;
	shift_l = 0;
	shift = 0;
}

void bexphase::Work(float *psamplesleft, float *psamplesright , int numsamples, int tracks)
{
	do
	{
		inbufr[inpoint] = *psamplesright; 
		inbufl[inpoint] = *psamplesleft;

		if ( shiftcount != 0 ) { freq += shift; --shiftcount; }
		help_r = (dlay_r+=shift_r) * freq; 
		help_l = (dlay_l+=shift_l) * freq;

		for ( x = 1; x <= Vals[pMode]; x++ )
		{
			if ( (y = inpoint - (int)(help_r * x)) < 0 ) y += inbuflen;
			if ( (z = inpoint - (int)(help_l * x)) < 0 ) z += inbuflen;
			*psamplesright = (*psamplesright * dry) + (inbufr[y] * wet);
			*psamplesleft = (*psamplesleft * dry) + (inbufl[z] * wet);
		}

		*psamplesright = ((inbufr[inpoint] - *psamplesright)*diff)+(*psamplesright*undiff);
		++psamplesright;
		*psamplesleft = ((inbufl[inpoint] - *psamplesleft)*diff)+(*psamplesleft*undiff);
		++psamplesleft;
		if ( ++inpoint == inbuflen ) inpoint = 0;
	}
	while(--numsamples);
}

bool bexphase::DescribeValue(char* txt,int const param, int const value)
{
	if ( param == pMode )
	{
		switch( value )
		{
			case 1 : *txt++='S'; *txt++='i'; *txt++='n'; *txt++='g'; *txt++='l'; *txt++='e'; *txt++=' '; *txt++='M'; *txt++='o'; *txt++='d'; *txt++='e'; *txt='\0'; break;
			case 2 : *txt++='D'; *txt++='o'; *txt++='u'; *txt++='b'; *txt++='l'; *txt++='e'; *txt++=' '; *txt++='M'; *txt++='o'; *txt++='d'; *txt++='e'; *txt='\0'; break; break;
			case 3 : *txt++='T'; *txt++='r'; *txt++='i'; *txt++='p'; *txt++='l'; *txt++='e'; *txt++=' '; *txt++='M'; *txt++='o'; *txt++='d'; *txt++='e'; *txt='\0'; break; break;
			case 4 : *txt++='Q'; *txt++='u'; *txt++='a'; *txt++='d'; *txt++=' '; *txt++='M'; *txt++='o'; *txt++='d'; *txt++='e'; *txt='\0'; break; break;
			default:
				sprintf( txt,"%.00f",(float)value );
				txt++;
				if ( value >= 10 ) txt++;
				*txt++='x'; *txt++=' '; *txt++='M'; *txt++='o'; *txt++='d'; *txt++='e'; *txt='\0';
		}
		return true;
	}
	if ( param == pRefresh )
	{
		if ( value <= 16 )
		{
			*txt++='T'; *txt++='i'; *txt++='c'; *txt++='k'; *txt++=' '; *txt++='x';
			sprintf( txt,"%.00f",(float)value );
		}
		else
		{
			help_l = buflen / 44.1f;
			sprintf( txt,"%.02f",help_l );
			txt+=4;
			if ( help_l > 10 ) txt++;
			if ( help_l > 100 ) txt++;
			*txt++=' '; *txt++='m'; *txt++='s'; *txt++='\0';
		}
		return true;
	}
	if ( param == pFreq ) 
	{ 
		sprintf( txt,"%.02f",freq * 90 ); 
		txt+=4;
		if ( (freq*90) >= 10 ) txt++;
		if ( (freq*90) >= 100 ) txt++;
		*txt++=' '; *txt++='°'; *txt='\0';
		return true; 
	}
	if ( param == pDiff ) 
	{ 
		sprintf( txt,"%.00f",diff*100  ); 
		txt++;
		if ( (diff*100) >= 10 ) txt++;
		if ( (diff*100) >= 100 ) txt++;
		*txt++=' '; *txt++='%'; *txt='\0';
		return true; 
	}
	if ( param == pDry ) 
	{ 
		if ( value < 384 ) sprintf( txt,"%.02f",optimal[0] ); 
		else
			{
			if ( value < 448 ) { *txt++='1'; *txt++='0'; *txt++='0'; *txt++='%'; *txt++=' '; *txt++='E'; *txt++='f'; *txt++='f'; *txt++='e'; *txt++='c'; *txt++='t'; *txt='\0';				}
			else { *txt++='1'; *txt++='0'; *txt++='0'; *txt++='%'; *txt++=' '; *txt++='W'; *txt++='e'; *txt++='t'; *txt='\0'; }
			}
		return true; 
	}
	if ( param == pAmount ) 
	{ 
		sprintf( txt,"%.02f",amntlfo ); 
		txt[4] = ' '; txt[5] = '/'; txt[6] = ' ';
		sprintf( &txt[7],"%.02f",amntfreq ); 
		return true; 
	}
	return false;
}
