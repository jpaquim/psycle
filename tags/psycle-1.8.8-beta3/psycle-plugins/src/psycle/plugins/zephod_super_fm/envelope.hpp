#pragma once

#define ENV_ATT 1
#define ENV_DEC 2
#define ENV_SUS 3
#define ENV_REL 4
#define ENV_NONE 99
#define ENV_CLICK 5

class envelope  
{
public:
	envelope();
	virtual ~envelope();

	inline float res(void);
	void reset();
	void attack(int newv);
	void decay(int newv);
	void sustain(int newv);
	void sustainv(float newv);
	void release(int newv);
	void stop();
	void noteoff();

public:
	int a,d,s,r;
	int envstate;
	float envvol;
	float susvol;
	float envcoef;
	int suscounter;

};

// Envelope get function inline

float envelope::res(void)
{
	if (envstate!=ENV_NONE)
	{
		// Attack
		if(envstate==ENV_ATT)
		{
			envvol+=envcoef;
			
			if(envvol>1.0f)
			{
				envvol=1.0f;
				envstate=ENV_DEC;
				envcoef=(1.0f-susvol)/(float)d;
			}
		}

		// Decay
		if(envstate==ENV_DEC)
		{
			envvol-=envcoef;
			
			if(envvol<susvol)
			{
				envvol=susvol;
				envstate=ENV_SUS;
				suscounter=0;
			}
		}

		// Sustain
		if(envstate==ENV_SUS)
		{
			suscounter++;
			
			if(suscounter>s)
			{
				envstate=ENV_REL;
				envcoef=envvol/(float)r;
			}
		}

		// Release
		if(envstate==ENV_REL)
		{
			envvol-=envcoef;

			if(envvol<0)
			{
				envvol=0;
				envstate=ENV_NONE;
			}
		}


		if(envstate==ENV_CLICK)
		{
			envvol-=envcoef;

			if(envvol<0)
			{
				envvol=0;
				envstate=ENV_ATT;
				envcoef=1.0f/(float)a;
			}
		}

	return envvol;
	}
	else 
	return 0;

}
