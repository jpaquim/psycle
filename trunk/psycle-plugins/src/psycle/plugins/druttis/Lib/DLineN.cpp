//============================================================================
//
//				DLineN (Non-Interpolating Delay Line)
//
//				druttis@darkface.pp.se
//
//============================================================================
#include <packageneric/pre-compiled.private.hpp>
#include "DLineN.h"
//============================================================================
//				Constructor
//============================================================================
DLineN::DLineN()
{
	inputs = 0;
	length = 0;
}
//============================================================================
//				Destructor
//============================================================================
DLineN::~DLineN()
{
	if (inputs)
		delete inputs;
}
//============================================================================
//				Init
//============================================================================
void DLineN::Init()
{
	Init(2047);
}
//============================================================================
//				Init
//============================================================================
void DLineN::Init(int maxLength)
{
	length = maxLength + 1;
	if (inputs)
		delete inputs;
	inputs = new float[length * sizeof(float)];
	Clear();
	inPoint = 0;
	outPoint = length >> 1;
	delay = (float) outPoint;
}
//============================================================================
//				Clear
//============================================================================
void DLineN::Clear()
{
	for (int i = 0; i < length; i++)
		inputs[i] = 0.0f;
	lastOutput = 0.0f;
}
//============================================================================
//				SetDelay
//============================================================================
void DLineN::SetDelay(float lag)
{
	if (lag >= length) {
		outPoint = inPoint + 1;
		delay = (float) (length - 1);
	} else {
		outPoint = inPoint - (int) lag;
		delay = lag;
	}
	while (outPoint < 0)
		outPoint += length;
}
//============================================================================
//				GetDelay
//============================================================================
float DLineN::GetDelay()
{
	return delay;
}
