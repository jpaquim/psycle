//============================================================================
//
//				DLineA (AllPass Interpolating Delay Line)
//
//				druttis@darkface.pp.se
//
//============================================================================
#include "DLineA.h"
//============================================================================
//				Constructor
//============================================================================
DLineA::DLineA()
{
	inputs = 0;
	length = 0;
}
//============================================================================
//				Destructor
//============================================================================
DLineA::~DLineA()
{
	if (inputs)
		delete inputs;
}
//============================================================================
//				Init
//============================================================================
void DLineA::Init()
{
	Init(2047);
}
//============================================================================
//				Init
//============================================================================
void DLineA::Init(int maxLength)
{
	length = maxLength + 1;
	if (inputs)
		delete inputs;
	inputs = new float[length];
	Clear();
	inPoint = 0;
	outPoint = length >> 1;
}
//============================================================================
//				Clear
//============================================================================
void DLineA::Clear()
{
	for (int i = 0; i < length; i++)
		inputs[i] = 0.0f;
	lastIn = 0.0f;
	lastOutput = 0.0f;
}
//============================================================================
//				SetDelay
//============================================================================
void DLineA::SetDelay(float lag)
{
	float outPointer;
	if (lag >= length) {
		outPointer = inPoint + 1.0f;
	} else if (lag < 0.1f) {
		outPointer = inPoint + 0.8999999999f;
	} else {
		outPointer = inPoint - lag + 1.0f;
	}
	while (outPointer < 0)
		outPointer += length;
	outPoint = (int) outPointer;
	alpha = 1.0f + (float) outPoint - outPointer;
	while (alpha < 0.1f) {
		outPoint++;
		while (outPoint >= length)
			outPoint -= length;
		alpha += 1.0f;
	}
	coeff = (1.0f - alpha) / (1.0f + alpha);
}
//============================================================================
//				GetDelay
//============================================================================
float DLineA::GetDelay()
{
	return delay;
}
