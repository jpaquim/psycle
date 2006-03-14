// PCSignalProcessor.cpp: implementation of the PCSignalProcessor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PsyClean.h"
#include "PCSignalProcessor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PCSignalProcessor::PCSignalProcessor()
{
	queue_size=0;
	buffer_size=1024;
}

PCSignalProcessor::~PCSignalProcessor()
{

}

void PCSignalProcessor::Load(void *data)
{

}

void PCSignalProcessor::Save(void *data)
{

}

int PCSignalProcessor::SaveDataSize()
{

}

void PCSignalProcessor::AddMidiEvent(PCMidiEvent& midievent)
{
	if (queue_size+1 < MAX_EVENTS ) midievents[queue_size++]=midievent;
}
