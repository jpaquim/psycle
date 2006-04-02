#include <cmath>

//////////////////////////////////////////////////////////////////////
// dw resonator

#define PI			3.141592653589793238
#define TWO_PI		6.283185307189586476
#define PI_EIGHTHS	0.392699081699
#define PI_FOURTHS	0.785398163397
#define PI_THIRDS	1.0471975512
#define PI_HALVES	1.57079632679

#define FILT_MIN_FREQ	15
#define FILT_MIN_GAIN	0.125	//-18 dB
#define FILT_MAX_GAIN	8.0		//18  dB
#define FILT_MIN_BW		0.01
#define FILT_MAX_BW		PI_THIRDS

#define FILT_MIN_SLOPE	0.01
#define FILT_MAX_SLOPE	1

#define FILT_MAX_CHANS	2

#define FILT_MIN_Q		0.7071
#define FILT_MAX_Q		0.98

#define NUM_MODES 4


enum filtmode
{
	nop=0,
	eq_parametric=1,
	eq_hishelf=2,
	eq_loshelf=3
};

class dwfilter
{
public:
	dwfilter();
	dwfilter(int samprate);
	virtual ~dwfilter();
	virtual void CoefUpdate();

	virtual float Process(const float xn, const int chan=0);

	virtual bool SetMode(int _mode)			{if(_mode<0 || _mode>=NUM_MODES)	return false;
												mode = (filtmode)_mode;	CoefUpdate();		return true;	}

	virtual bool SetFreq(double _freq)		{if(_freq>=0.0 && _freq<=nyquist)
												{freq = _freq;	CoefUpdate();	return true; } 
											else return false; }

	virtual bool SetGain(double _gain)		{if(_gain>=FILT_MIN_GAIN && _gain<=FILT_MAX_GAIN)
												{gain = _gain;	CoefUpdate();	return true; }
											else return false; }

	virtual bool SetBW(double _bw)			{if(_bw>=FILT_MIN_BW && _bw<=FILT_MAX_BW)
												{bandwidth = _bw;		CoefUpdate();	return true; }
											else return false; }

	virtual bool SetSlope(double _slope)	{if(_slope>=FILT_MIN_SLOPE && _slope<=FILT_MAX_SLOPE)
												{slope = _slope;	CoefUpdate();	return true; }
											else return false; }

	virtual bool SetQ(double _q)			{if(_q>=FILT_MIN_Q && _q<=FILT_MAX_Q)
												{q = _q;		CoefUpdate();		return true;	}
											else return false;	}


	virtual bool SetSampleRate(int samprate)	{if(samprate>=22050 && samprate<=192000)		/* who knows, maybe some day... */
													{nyquist = samprate/2.0;	CoefUpdate();	return true; }
												else return false; }

	virtual double GetFreq()	{ return freq;		}
	virtual double GetGain()	{ return gain;		}
	virtual double GetBW()		{ return bandwidth;	}
	virtual double GetSlope()	{ return slope;		}
	virtual double GetQ()		{ return q;			}
	virtual int GetMode()	{ return (int)mode;		}


protected:

	virtual void zeroize();

	filtmode mode;

	double freq;
	double gain;
	double bandwidth;
	double q;

	int nyquist;

	double coefa0, coefa1, coefa2;
	double coefb0, coefb1, coefb2;

	float z0[FILT_MAX_CHANS], z1[FILT_MAX_CHANS], z2[FILT_MAX_CHANS];

	// intermediate vars for parametric
	double bwtan;
	double costheta;
	double thetac;

	// intermediate vars for shelves
	double rho;
	double beta;
	double sintheta;
	double costhetaRho;
	double sinthetaBeta;
	double slope;
};


dwfilter::dwfilter()
{
	zeroize();

	nyquist=22050;		//seems like a safe guess
}

dwfilter::dwfilter(int samprate)
{
	zeroize();

	if(samprate>=22050 && samprate<=192000)
		nyquist=samprate/2;
	else
		nyquist=22050;
}

void dwfilter::zeroize()
{
	beta=0.0;
	sintheta=0.0;
	costhetaRho=0.0;
	sinthetaBeta=0.0;
	thetac = 0;
	bwtan = 0;
	costheta = 0;
	sintheta = 0;
	coefa0 = 1.0;
	coefa1 = 0.0;
	coefa2 = 0.0;
	coefb1 = 0.0;
	coefb2 = 0.0;
	gain=1.0;
	freq=0.0;
	bandwidth=0.0;
	mode=nop;

	for(int i=0;i<FILT_MAX_CHANS;++i)
		z0[i] = z1[i] = z2[i] = 0;

	slope=1.0f;				// for now..
}


dwfilter::~dwfilter()
{
}

void dwfilter::CoefUpdate()
{
	switch(mode)
	{
	case eq_parametric:	thetac = freq/(float)nyquist * PI;
						costheta = cosf(thetac);
						bwtan = tanf(bandwidth/2.0f);
						coefa0 = (float)(1.0f + gain * bwtan) / (float)(1.0f + bwtan);
						coefa1 = coefb1 = (float) (-2.0f * costheta) / (float) (1.0f + bwtan);
						coefa2 = (float)(1.0f - gain * bwtan) / (float)(1.0f + bwtan);
						coefb2 = (float)(1.0f - bwtan) / (float)(1.0f + bwtan);
						break;

	case eq_loshelf:	thetac = freq/(float)nyquist * PI;
						rho = sqrt(gain);
						costheta = cosf(thetac);
						sintheta = sinf(thetac);
						//slope = 1.0f;
						beta = sqrt((rho*rho + 1)/slope - ((rho-1)*(rho-1)));
						costhetaRho = costheta * rho;
						sinthetaBeta = sintheta * beta;

						coefb0 =        ((rho+1) + costhetaRho - costheta + sinthetaBeta); 

						coefa0 = (rho * ((rho+1) - costhetaRho + costheta + sinthetaBeta) / coefb0);
						coefa1 = (2*rho*((rho-1) - costhetaRho - costheta				) / coefb0);
						coefa2 = (rho * ((rho+1) - costhetaRho + costheta - sinthetaBeta) / coefb0);
						coefb1 = ( -2 * ((rho-1) + costhetaRho + costheta				) / coefb0);
						coefb2 =       (((rho+1) + costhetaRho - costheta - sinthetaBeta) / coefb0);
						break;

	case eq_hishelf:	thetac = freq/(float)nyquist * PI;
						rho = sqrt(gain);
						costheta = cosf(thetac);
						sintheta = sinf(thetac);
						//slope = 1.0f;
						beta = sqrt((rho*rho + 1)/slope - ((rho-1)*(rho-1)));
						costhetaRho = costheta * rho;
						sinthetaBeta = sintheta * beta;

						coefb0 =       ((rho+1) - costhetaRho + costheta + sinthetaBeta );

						coefa0 = ( rho * ((rho+1) + costhetaRho - costheta + sinthetaBeta) / coefb0);
						coefa1 = (-2*rho*((rho-1) + costhetaRho + costheta				 ) / coefb0);
						coefa2 = ( rho * ((rho+1) + costhetaRho - costheta - sinthetaBeta) / coefb0);
						coefb1 = (   2 * ((rho-1) - costhetaRho - costheta				 ) / coefb0);
						coefb2 = (       ((rho+1) - costhetaRho + costheta - sinthetaBeta) / coefb0);
						break;

	}

}



float dwfilter::Process(const float xn, const int chan)
{
	float yn=0.0;

	if (chan<0 || chan>=FILT_MAX_CHANS)
		return 0.0;

	switch(mode)
	{
	case eq_hishelf:
	case eq_loshelf:
	case eq_parametric:		z0[chan] =	  xn
										- coefb1 * z1[chan]
										- coefb2 * z2[chan];

							yn =		  coefa0 * z0[chan]
										+ coefa1 * z1[chan]
										+ coefa2 * z2[chan];
							break;

	}

	z2[chan] = z1[chan];
	z1[chan] = z0[chan];
	return yn;

}
