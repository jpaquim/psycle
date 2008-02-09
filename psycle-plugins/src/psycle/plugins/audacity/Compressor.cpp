/* -*- mode:c++, indent-tabs-mode:t -*- */
/**********************************************************************

	Audacity: A Digital Audio Editor

	Compressor.cpp

	Dominic Mazzoni

	Steve Jolly made it inherit from EffectSimpleMono.
	GUI added and implementation improved by Dominic Mazzoni, 5/11/2003.

**********************************************************************/
#include <packageneric/pre-compiled.private.hpp>
#include <cmath>

#include "Compressor.h"

EffectCompressor::EffectCompressor()
{
	mFloor = 0.001;
	mGainDB = 0.0;

	this->setRatio(2.0);
	this->setThreshold(-12.0);
	this->setGain(true);
	this->setSampleRate(44100.0);
	this->setAttack(0.2);
	this->setDecay(1.0);

	buffer = new float[MAX_SAMPLES];

	mCircleSize = 100;

	mCircle = new double[mCircleSize];
	mLevelCircle = new double[mCircleSize];
	for(int j=0; j<mCircleSize; j++) {
		mCircle[j] = 0.0;
		mLevelCircle[j] = mFloor;
	}

	mCirclePos = 0;
	mRMSSum = 0.0;
	mLastLevel = 0.0;
}

EffectCompressor::~EffectCompressor()
{
	if (mCircle)
		delete[] mCircle;
	if (mLevelCircle)
		delete[] mLevelCircle;
	if (buffer)
		delete[] buffer;
}

void EffectCompressor::setSampleRate(double rate)
{
	mCurRate = rate;
	this->setAttackFactor();
	this->setDecayFactor();
}

void EffectCompressor::setRatio(double ratio)
{
	mRatio = ratio;
	if(mRatio<1.0)mRatio=1.0;
	
	this->setGainDB();
}

void EffectCompressor::setThreshold(double threshold)
{
	mThresholdDB = threshold;
	mThreshold = std::pow(10.0, mThresholdDB/10); // factor of 10 because it's power
}

void EffectCompressor::setAttack(double attack)
{
	mAttackTime = attack;
	this->setAttackFactor();
}

inline void EffectCompressor::setAttackFactor()
{
	mAttackFactor = std::exp(-log(mFloor) / (mCurRate * mAttackTime + 0.5));
}

void EffectCompressor::setDecay(double decay)
{
	mDecayTime = decay;
	this->setDecayFactor();
}

inline void EffectCompressor::setDecayFactor()
{
	mDecayFactor = std::exp(log(mFloor) / (mCurRate * mDecayTime + 0.5));
}

void EffectCompressor::setGainDB()
{
	mGainDB = ((mThresholdDB*-0.7) * (1 - 1/mRatio));
	if (mGainDB < 0)
		mGainDB = 0;
}

void EffectCompressor::setGain(bool gain)
{
	mUseGain = gain;
	if (mUseGain)
		mGain = std::pow(10.0, mGainDB/20); // factor of 20 because it's amplitude
	else
		mGain = 1.0;
}

float EffectCompressor::BufferIn(float *samples, int num)
{
	float s = 0;
	for(int i=0;i<num;i++){
		

		s+=buffer[i]= *samples * 0.000030517578125f; // -1 < buffer[i] < 1
		++samples;
	}
	return s;
}

void EffectCompressor::BufferOut(float *samples, int num)
{
	for(int i=0;i<num;i++){
		*samples = buffer[i]* 32767.f; // 
		++samples;
	}
}

void EffectCompressor::Process(int len)
{
	double *follow = new double[len];
	int i;

	// This makes sure that the initial value is well-chosen
	if (mLastLevel == 0.0) {
		int preSeed = mCircleSize;
		if (preSeed > len)
			preSeed = len;
		for(i=0; i<preSeed; i++)
			this->AvgCircle(buffer[i]);
	}

	for (i = 0; i < len; i++) {
		this->Follow(buffer[i], &follow[i], i);
	}

	for (i = 0; i < len; i++) {
	buffer[i] = this->DoCompression(buffer[i], follow[i]);

	}

	delete[] follow;
}

double EffectCompressor::AvgCircle(float value)
{
	float level;

	// Calculate current level from root-mean-squared of
	// circular buffer ("RMS")
	mRMSSum -= mCircle[mCirclePos];
	mCircle[mCirclePos] = value*value;
	mRMSSum += mCircle[mCirclePos];
	level = std::sqrt(mRMSSum/mCircleSize);
	// NaN and Den remover :
	unsigned int corrected_sample = *((unsigned int*)&level);
	unsigned int exponent = corrected_sample & 0x7F800000;
	corrected_sample *= ((exponent < 0x7F800000) & (exponent > 0));
	level = *((float*)&corrected_sample);

	mLevelCircle[mCirclePos] = level;
	mCirclePos = (mCirclePos+1)%mCircleSize;   

#if 0 // Peak instead of RMS
	int j;
	level = 0.0;
	for(j=0; j<mCircleSize; j++)
		if (mCircle[j] > level)
			level = mCircle[j];
#endif

	return double(level);
}

void EffectCompressor::Follow(float x, double *outEnv, int maxBack)
{
	/*

	"Follow"ing algorithm by Roger B. Dannenberg, taken from
	Nyquist.  His description follows.  -DMM

	Description: this is a sophisticated envelope follower.
	The input is an envelope, e.g. something produced with
	the AVG function. The purpose of this function is to
	generate a smooth envelope that is generally not less
	than the input signal. In other words, we want to "ride"
	the peaks of the signal with a smooth function. The 
	algorithm is as follows: keep a current output value
	(called the "value"). The value is allowed to increase
	by at most rise_factor and decrease by at most fall_factor.
	Therefore, the next value should be between
	value * rise_factor and value * fall_factor. If the input
	is in this range, then the next value is simply the input.
	If the input is less than value * fall_factor, then the
	next value is just value * fall_factor, which will be greater
	than the input signal. If the input is greater than value *
	rise_factor, then we compute a rising envelope that meets
	the input value by working bacwards in time, changing the
	previous values to input / rise_factor, input / rise_factor^2,
	input / rise_factor^3, etc. until this new envelope intersects
	the previously computed values. There is only a limited buffer
	in which we can work backwards, so if the new envelope does not
	intersect the old one, then make yet another pass, this time
	from the oldest buffered value forward, increasing on each 
	sample by rise_factor to produce a maximal envelope. This will 
	still be less than the input.
	
	The value has a lower limit of floor to make sure value has a 
	reasonable positive value from which to begin an attack.
	*/

	double level = AvgCircle(x);
	double high = mLastLevel * mAttackFactor;
	double low = mLastLevel * mDecayFactor;

	if (low < mFloor)
		low = mFloor;

	if (level < low)
		*outEnv = low;
	else if (level < high)
		*outEnv = level;
	else {
		// Backtrack
		double attackInverse = 1.0 / mAttackFactor;
		double temp = level * attackInverse;

		int backtrack = 50;
		if (backtrack > maxBack)
			backtrack = maxBack;

		double *ptr = &outEnv[-1];
		int i;
		bool ok = false;
		for(i=0; i<backtrack-2; i++) {
			if (*ptr < temp) {
			*ptr-- = temp;
			temp *= attackInverse;
			}
			else {
			ok = true;
			break;
			}   
		}

		if (!ok && backtrack>1 && (*ptr < temp)) {
			temp = *ptr;
			for (i = 0; i < backtrack-1; i++) {
			ptr++;
			temp *= mAttackFactor;
			*ptr = temp;
			}
		}
		else
			*outEnv = level;
	}

	mLastLevel = *outEnv;
}

float EffectCompressor::DoCompression(float value, double env)
{
	float mult;
	float out;

	if (env > mThreshold)
		mult = mGain * std::pow(mThreshold/env, 1.0/mRatio);
	else
		mult = mGain;

	out = value * mult;

	if (out > 1.0)
		out = 1.0;

	if (out < -1.0)
		out = -1.0;

	return out;
}
