/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once

class CLowpass  
{
public:
	CLowpass();
	virtual ~CLowpass() throw();
	inline float Process(float i,float c);

private:
	float o1;
};

inline float CLowpass::Process(float i,float c)
{
	float output= o1 + c * (i-o1);
	o1=output;

	return output;
}
