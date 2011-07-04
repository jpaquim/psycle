//////////////////////////////////////////////////////////////////////
// Machine Presets

void psyGear::SetPreset(int mm,int preset)
{					
	switch(mm)
	{
	case 7:	/* Flanger presets */

		switch(preset)
		{
		case 0:	/* Default */
			delayTimeL=75;
			filterLfoamp=246;
			filterLfospeed=3748;
			delayFeedbackL=65;
			filterLfophase=32;
			delayFeedbackR=49;
			outDry=256;
			outWet=256;
		break;
		
		case 1:	/* Chorus */
			delayTimeL=227;
			filterLfoamp=246;
			filterLfospeed=23543;
			delayFeedbackL=28;
			filterLfophase=112;
			delayFeedbackR=-24;
			outDry=236;
			outWet=256;
		break;
		
		case 2:	/* Chorus 2*/
			delayTimeL=325;
			filterLfoamp=100;
			filterLfospeed=12973;
			delayFeedbackL=64;
			filterLfophase=124;
			delayFeedbackR=65;
			outDry=144;
			outWet=256;
		break;
		
		}

		// Updating flanger
		UpdateFlanger();
	break;
	}
}