//////////////////////////////////////////////////////////////////////
//
//	Filter.cpp
//
//	druttis@darkface.pp.se
//
//////////////////////////////////////////////////////////////////////
#include <project.private.hpp>
#include "Filter.h"
//////////////////////////////////////////////////////////////////////
//
//	Filter constructor
//
//////////////////////////////////////////////////////////////////////
Filter::Filter()
{
	Reset();
}
//////////////////////////////////////////////////////////////////////
//
//	Filter destructor
//
//////////////////////////////////////////////////////////////////////
Filter::~Filter()
{
}
//////////////////////////////////////////////////////////////////////
//
//	Reset
//
//////////////////////////////////////////////////////////////////////
void Filter::Reset()
{
	ca = 0.0f;
	cb = 0.0f;
	cc = 0.0f;
	b0 = 0.0f;
	b1 = 0.0f;
	b2 = 0.0f;
	b3 = 0.0f;
	b4 = 0.0f;
	b5 = 0.0f;
	b6 = 0.0f;
	b7 = 0.0f;
}
