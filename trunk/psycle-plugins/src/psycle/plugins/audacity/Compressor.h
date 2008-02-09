/* -*- mode:c++, indent-tabs-mode:t -*- */
/**********************************************************************

	Audacity: A Digital Audio Editor

	Compressor.h

	Dominic Mazzoni

**********************************************************************/

#ifndef __AUDACITY_EFFECT_COMPRESSOR__
#define __AUDACITY_EFFECT_COMPRESSOR__

#define MAX_SAMPLES 3000

class EffectCompressor {
	
public:
	
	EffectCompressor();
	~EffectCompressor();

	void setSampleRate(double rate);
	void setRatio(double ratio);
	void setThreshold(double threshold);
	void setAttack(double attack);
	void setDecay(double decay);
	void setGainDB();
	void setGain(bool gain);
	float BufferIn(float *samples, int num);
	void BufferOut(float *samples, int num);
	void Process(int len);

	private:
	inline void setAttackFactor();
	inline void setDecayFactor();

	double AvgCircle(float x);
	void Follow(float x, double *outEnv, int maxBack);
	float DoCompression(float x, double env);
	
	double    mAttackTime;
	double    mThresholdDB;
	double    mRatio;
	bool      mUseGain;
	
	double				 mCurRate;
	double    mDecayTime;
	double    mGainDB;
	double    mAttackFactor;
	double    mDecayFactor;
	double    mFloor;
	double    mThreshold;
	double    mGain;
	double    mRMSSum;
	int       mCircleSize;
	int       mCirclePos;
	double   *mCircle;
	double   *mLevelCircle;
	double    mLastLevel;

	float    *buffer;
};


#endif

