#pragma once

// Lowpass.h: interface for the CLowpass class.
//
//////////////////////////////////////////////////////////////////////

class CLowpass  
{
public:
	CLowpass();
	virtual ~CLowpass();
	float Process(float i,float c);

private:
	float o1;
};
