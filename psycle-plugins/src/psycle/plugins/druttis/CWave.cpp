// -*- mode:c++; indent-tabs-mode:t -*-
//============================================================================
//
//				CWave.cpp
//
//============================================================================
#include <string.h>
#include "CWave.h"
//============================================================================
//				Constructor
//============================================================================
CWave::CWave()
{
	m_pname = NULL;
	m_psamples = NULL;
	m_nsamples = 0;
	m_mask = 0;
}
//============================================================================
//				Destructor
//============================================================================
CWave::~CWave()
{
	//				Name
	if (m_pname) {
		delete[] m_pname;
		m_pname = NULL;
	}
	//				Samples
	if (m_psamples) {
		delete[] m_psamples;
		m_psamples = NULL;
	}
	//				Length
	m_nsamples = 0;
}
//============================================================================
//				Initialize
//============================================================================
bool CWave::Init(char *pname, float *psamples, int nsamples)
{
	//				Name
	if (!pname)
		return false;
	if (m_pname)
		delete[] m_pname;
	m_pname = new char[strlen(pname) + 1];
	strcpy(m_pname, pname);
	//				Samples
	if (!psamples)
		return false;
	if (m_psamples)
		delete[] m_psamples;
	m_psamples = new float[nsamples];
	memcpy(m_psamples, psamples, sizeof(float) * nsamples);
	//				Length
	m_nsamples = nsamples;
	m_mask = nsamples - 1;
	if (((nsamples & m_mask) != 0) || (nsamples & 1))
		return false;
	//				Ok!
	return true;
}
//============================================================================
//				GetName
//============================================================================
char *CWave::GetName()
{
	return m_pname;
}
//============================================================================
//				GetNumberOfSamples
//============================================================================
int CWave::GetNumberOfSamples()
{
	return m_nsamples;
}
