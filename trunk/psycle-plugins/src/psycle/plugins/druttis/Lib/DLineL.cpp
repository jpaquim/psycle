// -*- mode:c++; indent-tabs-mode:t -*-
//============================================================================
//
//				DLineL (Linear Interpolating Delay Line)
//
//				druttis@darkface.pp.se
//
//============================================================================
#include "DLineL.h"
//============================================================================
//				Constructor
//============================================================================
DLineL::DLineL()
{
	inputs = 0;
	length = 0;
}
//============================================================================
//				Destructor
//============================================================================
DLineL::~DLineL()
{
	if (inputs)
		delete inputs;
}
//============================================================================
//				Init
//============================================================================
void DLineL::Init()
{
	Init(2047);
}
//============================================================================
//				Init
//============================================================================
void DLineL::Init(int maxLength)
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
void DLineL::Clear()
{
	for (int i = 0; i < length; i++)
		inputs[i] = 0.0f;
	lastOutput = 0.0f;
}
//============================================================================
//				SetDelay
//============================================================================
void DLineL::SetDelay(float lag)
{
	float outPointer;
	if (lag >= length) {
		outPointer = (float) inPoint + 1.0f;
		delay = (float) length - 1.0f;
	} else {
		outPointer = (float) inPoint - lag;
		delay = lag;
	}
	while (outPointer < 0)
		outPointer += length;
	outPoint = (int) outPointer;
	alpha = outPointer - outPoint;
	omAlpha = 1.0f - alpha;
}
//============================================================================
//				GetDelay
//============================================================================
float DLineL::GetDelay()
{
	return delay;
}
