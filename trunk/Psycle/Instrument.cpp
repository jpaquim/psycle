////////////////////////////////////
//
////////////////////////////////////

#include "stdafx.h"


#include "Instrument.h"

bool Instrument::Empty()
{
	for (int i = 0; i < MAX_WAVES; i++)
	{
		if (waveLength[i] > 0)
		{
			return FALSE;
		}
	}
	return TRUE;
}
