//////////////////////////////////////////////////////////////////////
// Keyboard Handler sourcecode

void CChildView::KeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// undo code not required, enter note handles it
	CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);	
	if (cmd.GetType() == CT_Note)
	{
		const int outnote = cmd.GetNote();
		if(viewMode == VMPattern && bEditMode && Global::pPlayer->_playing && Global::pConfig->_followSong && Global::pConfig->_RecordNoteoff)
		{ 
			EnterNote(outnote,0,true);	// note end
		}
		else
		{
			Global::pInputHandler->StopNote(outnote);
		}
	}
	else if ((nChar == 16) && (nFlags & 49152) && ChordModeOffs)
	{
		// shift is up, abort chord mode
		editcur.line = ChordModeLine;
		editcur.track = ChordModeTrack;
		ChordModeOffs = 0;
		AdvanceLine(patStep,Global::pConfig->_wrapAround,true);
//		pParentMain->StatusBarIdle();
//		Repaint(DMCursor);
	}
}

void CChildView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// undo code not required, enter note handles it
	KeyUp(nChar, nRepCnt, nFlags);
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CChildView::KeyDown(UINT nChar, UINT nRepCnt, UINT nFlags )
{
	// undo code not required, enter not and msbput handle it
	BOOL bRepeat = nFlags&0x4000;

	if(viewMode == VMPattern && bEditMode)
	{
		if (!(Global::pPlayer->_playing && Global::pConfig->_followSong && bRepeat))
		{
			bool success;
			// add data
			success = Global::pInputHandler->EnterData(nChar,nFlags);

			if ( success )
			{
				CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
				return;
			}
		}
	}
	else
	{
		ChordModeOffs = 0;
	}

	// get command
	CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);

	if(cmd.IsValid())
	{
		if((cmd.GetType() == CT_Immediate) ||
		   (cmd.GetType() == CT_Editor && viewMode == VMPattern) ) 
		{			
			Global::pInputHandler->PerformCmd(cmd,bRepeat);
		}
		else if (cmd.GetType() == CT_Note )
		{
			if(!bRepeat) 
			{	
				const int outnote = cmd.GetNote();
				// play note
				Global::pInputHandler->PlayNote(outnote); 
			}
		}
	}
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags )
{
	KeyDown(nChar, nRepCnt, nFlags);
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);	
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MidiPatternNote
//
// DESCRIPTION	  : Called by the MIDI input interface to insert pattern notes
// PARAMETERS     : int outnote - note to insert . int velocity - velocity of the note
// RETURNS		  : <void>
// 

//
// Mark!!!!! Please, check if the following function is ok. I have some doubts about the
// NoteOff. And check if "if(outnote >= 0 && outnote <= 120)" is necessary.
//

void CChildView::MidiPatternNote(int outnote, int velocity)
{
	// undo code not required, enter note handles it
/*	if(outnote >= 0 && outnote <= 120)  // I really believe this is not necessary.
	{									// outnote <= 120 is checked before calling this function
										// and outnote CAN NOT be negative since it's taken from
										//	(dwParam1 & 0xFF00) >>8;
	*/
		if(viewMode == VMPattern && bEditMode)
		{ 
			// add note
			if(velocity > 0 && outnote != 120)
			{
				EnterNote(outnote,velocity,false);
			}
			else
			{
				if(Global::pConfig->_RecordNoteoff && Global::pPlayer->_playing && Global::pConfig->_followSong)
				{
					EnterNote(outnote,0,false);	// note end
				}
				else
				{
					Global::pInputHandler->StopNote(outnote,false);	// note end
				}
			}			
		}
		else 
		{
			// play note
			if(velocity>0)
				Global::pInputHandler->PlayNote(outnote,velocity,false);
			else
				Global::pInputHandler->StopNote(outnote,false);
		}
//	}
}

void CChildView::MidiPatternTweak(int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow
	if(viewMode == VMPattern && bEditMode)
	{ 
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(_pSong->_trackArmedCount)
			{
				SelectNextTrack();
			}
		}
		// write effect
		int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * offset = _offset(ps);
		unsigned char * toffset = _toffset(ps);
		
		// realtime note entering
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			toffset = offset+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
		}

		// build entry
		PatternEntry *entry = (PatternEntry*) toffset;
		if (entry->_note >= 120)
		{
			if ((entry->_mach != _pSong->seqBus) || (entry->_cmd != ((value>>8)&255)) || (entry->_parameter != (value&255)) || (entry->_inst != command) || (entry->_note != 121))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				entry->_mach = _pSong->seqBus;
				entry->_cmd = (value>>8)&255;
				entry->_parameter = value&255;
				entry->_inst = command;
				entry->_note = 121;

//				drawTrackStart=editcur.track;
//				drawTrackEnd=editcur.track;
//				drawLineStart=editcur.line;
//				drawLineEnd=editcur.line;
				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
//	else
	{
		// build entry
		PatternEntry entry;
		entry._mach = _pSong->seqBus;
		entry._cmd = (value>>8)&255;
		entry._parameter = value&255;
		entry._inst = command;
		entry._note = 121;

		// play it
		int mgn;
		Machine* pMachine;
		if ( _pSong->seqBus < MAX_BUSES )
			mgn = _pSong->busMachine[_pSong->seqBus];
		else
			mgn = _pSong->busEffect[(_pSong->seqBus & (MAX_BUSES-1))];

		if (mgn < MAX_MACHINES && _pSong->_machineActive[mgn])
			pMachine = _pSong->_pMachines[mgn];
		else return;

		// play
		pMachine->Tick(0,&entry);
	}
}


void CChildView::MidiPatternCommand(int command, int value)
{
	// UNDO CODE MIDI PATTERN
	if (value < 0) value = (0x80-value);// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xff) value = 0xff; // no else incase of neg overflow
	if(viewMode == VMPattern && bEditMode)
	{ 
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(_pSong->_trackArmedCount)
			{
				SelectNextTrack();
			}
		}
		// write effect
		int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * offset = _offset(ps);
		unsigned char * toffset = _toffset(ps);
		
		// realtime note entering
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			toffset = offset+(line*MULTIPLY);
		}
		else
		{
			line = editcur.track;
		}

		// build entry
		PatternEntry *entry = (PatternEntry*) toffset;
		if ((entry->_mach != _pSong->seqBus) || (entry->_inst != _pSong->auxcolSelected) || (entry->_cmd != command) || (entry->_parameter != value))
		{
			AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
			entry->_mach = _pSong->seqBus;
			entry->_inst = _pSong->auxcolSelected;
			entry->_cmd = command;
			entry->_parameter = value;

//			drawTrackStart=editcur.track;
//			drawTrackEnd=editcur.track;
//			drawLineStart=editcur.line;
//			drawLineEnd=editcur.line;
			NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
			Repaint(DMData);
		}
	}
//	else
	{
		// build entry
		PatternEntry entry;
		entry._mach = _pSong->seqBus;
		entry._inst = _pSong->auxcolSelected;
		entry._cmd = command;
		entry._parameter = value;
		entry._note = 255;

		// play it
		int mgn;
		Machine* pMachine;
		if (_pSong->seqBus < MAX_BUSES)
			mgn = _pSong->busMachine[_pSong->seqBus];
		else
			mgn = _pSong->busEffect[(_pSong->seqBus & (MAX_BUSES-1))];

		if (mgn < MAX_MACHINES && _pSong->_machineActive[mgn])
			pMachine = _pSong->_pMachines[mgn];
		else return;

		// play
		pMachine->Tick(0,&entry);
	}
}

void CChildView::MousePatternTweak(int machine, int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow
	if(viewMode == VMPattern && bEditMode)
	{ 
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(_pSong->_trackArmedCount)
			{
				SelectNextTrack();
			}
		}
		// write effect
		int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * offset = _offset(ps);
		unsigned char * toffset = _toffset(ps);
		
		// realtime note entering
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			toffset = offset+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
		}

		// build entry
		PatternEntry *entry = (PatternEntry*) toffset;
		if (entry->_note >= 120)
		{
			if ((entry->_mach != machine) || (entry->_cmd != ((value>>8)&255)) || (entry->_parameter != (value&255)) || (entry->_inst != command) || (entry->_note != 121))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				entry->_mach = machine;
				entry->_cmd = (value>>8)&255;
				entry->_parameter = value&255;
				entry->_inst = command;
				entry->_note = 121;

//				drawTrackStart=editcur.track;
//				drawTrackEnd=editcur.track;
//				drawLineStart=editcur.line;
//				drawLineEnd=editcur.line;
				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
}


void CChildView::EnterNote(int note, int velocity, bool bTranspose)
{
	int line;

	// UNDO CODE ENTER NOTE
	int ps = _ps();
	unsigned char * offset; 
	unsigned char * toffset;
	
	if (note < 0 || note > 123 ) return;

	// octave offset
	if(note<120)
	{
		if(bTranspose)
			note+=_pSong->currentOctave*12;

		if (note > 119) 
			note = 119;
	}
	
	// realtime note entering
	if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
	{
		if(_pSong->_trackArmedCount)
		{
			if (velocity == 0)
			{
				for (int i = 0; i < _pSong->SONGTRACKS; i++)
				{
					if (_pSong->_trackArmed[i])
					{
						if (Global::pInputHandler->notetrack[i] == note)
						{
							editcur.track = i;
							break;
						}
					}
				}
				Global::pInputHandler->StopNote(note,false);
				if (i == _pSong->SONGTRACKS)
				{
					return;
				}
			}
			else
			{
				SelectNextTrack();
			}
		}
		line = Global::pPlayer->_lineCounter;
		offset = _offset(ps);
		toffset = offset+(line*MULTIPLY);
		ChordModeOffs = 0;
	}
	else 
	{
		if ( GetKeyState(VK_SHIFT)<0) 
		{
			if (ChordModeOffs == 0)
			{
				ChordModeLine = editcur.line;
				ChordModeTrack = editcur.track;
			}
			editcur.track = (ChordModeTrack+ChordModeOffs)%_pSong->SONGTRACKS;
			editcur.line = line = ChordModeLine;
			offset = _offset(ps);
			toffset = _toffset(ps, editcur.track, line);
			ChordModeOffs++;
		}
		else
		{
			if (ChordModeOffs) // this should never happen because the shift check should catch it... but..
			{					// ok pooplog, now it REALLY shouldn't happen (now that the shift check works)
				editcur.line = ChordModeLine;
				editcur.track = ChordModeTrack;
				ChordModeOffs = 0;
				AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
			}
			line = editcur.line;
			offset = _offset(ps);
			toffset = _toffset(ps);
		}
	}

	// build entry
	PatternEntry *entry = (PatternEntry*) toffset;
	if (velocity==0)
	{
		if (entry->_note == note)
		{
			return;
		}
		note = 120;
	}
	AddUndo(ps,editcur.track,line,1,1,editcur.track,line,editcur.col,editPosition);
	entry->_note = note;
	entry->_mach = _pSong->seqBus;

	if ( note < 120)
	{
		if (Global::pConfig->_RecordTweaks)
		{
			if (Global::pConfig->_midiRecordVel)
			{
				// command
				entry->_cmd = Global::pConfig->_midiCommandVel;
				int par = Global::pConfig->_midiFromVel + 
									(((Global::pConfig->_midiToVel - Global::pConfig->_midiFromVel) * velocity)/127);
				if (par > 255) 
				{
					par = 255;
				}
				else if (par < 0) 
				{
					par = 0;
				}
				entry->_parameter = par;
			}
		}
	}

	int mgn;
	if (_pSong->seqBus & MAX_BUSES) // If it is an effect
	{
		mgn = _pSong->busEffect[(_pSong->seqBus & (MAX_BUSES-1))];
	}
	else
	{
		mgn = _pSong->busMachine[_pSong->seqBus];
	}

	if (note>120)
	{
		entry->_inst = _pSong->auxcolSelected;
	}

	if (mgn < MAX_MACHINES && _pSong->_machineActive[mgn])
	{
		Machine *tmac = _pSong->_pMachines[mgn];

		if (tmac->_type == MACH_SAMPLER)
		{
			entry->_inst = _pSong->auxcolSelected;
		}
		if (tmac->_type == MACH_VST) // entry->_inst is the MIDI channel for VSTi's
		{
			entry->_inst = _pSong->auxcolSelected;
		}
		
		if ( note < 120)
		{
			tmac->Tick(editcur.track, entry);
		}
	}

	Global::pInputHandler->notetrack[editcur.track]=note;
	NewPatternDraw(editcur.track,editcur.track,line,line);
	if (!(Global::pPlayer->_playing&&Global::pConfig->_followSong))
	{
		if (ChordModeOffs)
		{
			AdvanceLine(-1,Global::pConfig->_wrapAround,false);
		}
		else
		{
			AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
		}
	}

	bScrollDetatch=false;
	Global::pInputHandler->bDoingSelection = false;
	Repaint(DMData);
}

void CChildView::EnterNoteoffAny()
{
	if (viewMode == VMPattern)
	{
		// UNDO CODE ENTER NOTE
		int ps = _ps();
		unsigned char * offset; 
		unsigned char * toffset;
		
		// realtime note entering
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			offset = _offset(ps);
			toffset = offset+(Global::pPlayer->_lineCounter*MULTIPLY);
		}
		else
		{
			offset = _offset(ps);
			toffset = _toffset(ps);
		}

		// build entry
		PatternEntry *entry = (PatternEntry*) toffset;
		AddUndo(ps,editcur.track,editcur.line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
		entry->_note = 120;

		Global::pInputHandler->notetrack[editcur.track]=120;

	//	drawTrackStart=editcur.track;
	//	drawTrackEnd=editcur.track;
	//	drawLineStart=editcur.line;
	//	drawLineEnd=editcur.line;
		NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);

		if (!(Global::pPlayer->_playing&&Global::pConfig->_followSong))
		{
			AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
		}

		bScrollDetatch=false;
		Global::pInputHandler->bDoingSelection = false;
		Repaint(DMData);
	}
}

bool CChildView::MSBPut(int nChar)
{
	// UNDO CODE MSB PUT
	// init
	int ps = _ps();
	unsigned char * toffset = _toffset(ps) + (editcur.col+1)/2;

	int oldValue = *toffset;	
	int sValue = -1;

	if	(	nChar>='0'		&&	nChar<='9')			{ sValue = nChar - '0'; }
	else if(nChar>=VK_NUMPAD0&&nChar<=VK_NUMPAD9)	{ sValue = nChar - VK_NUMPAD0; }
	else if(nChar>='A'		&&	nChar<='F')			{ sValue = nChar - 'A' + 10; }
	else											{ return false; }

	if (editcur.col < 5 && oldValue == 255)	{ oldValue = 0; }

//	drawTrackStart=editcur.track;
//	drawTrackEnd=editcur.track;
//	drawLineStart=editcur.line;
//	drawLineEnd=editcur.line;

	AddUndo(ps,editcur.track,editcur.line,1,1,editcur.track,editcur.line,editcur.col,editPosition);

	switch ((editcur.col+1)%2)
	{
	case 0:	
		*toffset = (oldValue&0xF)+(sValue<<4); 
		break;
	
	case 1:	
		*toffset = (oldValue&0xF0)+(sValue); 
		break;
	}

	if (Global::pConfig->_cursorAlwaysDown)
	{
		AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
	}
	else
	{
		switch (editcur.col)
		{
		case 0:
			AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
			break;
		case 1:
		case 3:
		case 5:
		case 6:
		case 7:
			NextCol(false,false);
			break;
		case 8:
			PrevCol(false,false);
			PrevCol(false,false);
		case 2:
		case 4:
			PrevCol(false,false);
			AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
			break;
		}
	}
	bScrollDetatch=false;
	Global::pInputHandler->bDoingSelection = false;
	Repaint(DMCursor);
	return true;
}

void CChildView::ClearCurr() // delete content at Cursor pos.
{
	// UNDO CODE CLEAR
	int ps = _ps();
	unsigned char * offset = _offset(ps);
	unsigned char * toffset = _toffset(ps);

	AddUndo(ps,editcur.track,editcur.line,1,1,editcur.track,editcur.line,editcur.col,editPosition);

	if ( editcur.col == 0 )
	{
		memset(offset+(editcur.line*MULTIPLY),255,3*sizeof(char));
		memset(offset+(editcur.line*MULTIPLY)+3,0,2*sizeof(char));
	}
	else if (editcur.col < 5 )	{	*(toffset+(editcur.col+1)/2)= 255; }
	else						{	*(toffset+(editcur.col+1)/2)= 0; }

//	drawTrackStart=editcur.track;
//	drawTrackEnd=editcur.track;
//	drawLineStart=editcur.line;
//	drawLineEnd=editcur.line;
	NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);

	AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
	Global::pInputHandler->bDoingSelection = false;
	ChordModeOffs = 0;
	bScrollDetatch=false;
	Repaint(DMData);
}

void CChildView::DeleteCurr()
{
	// UNDO CODE DELETE
	int ps = _ps();
	unsigned char * offset = _offset(ps);
	int patlines = _pSong->patternLines[ps];

	if ( Global::pInputHandler->bFT2DelBehaviour )
	{
		if(editcur.line==0)
			return;
		else
			editcur.line--;
	}

	AddUndo(ps,editcur.track,editcur.line,1,patlines-editcur.line,editcur.track,editcur.line,editcur.col,editPosition);

	for (int i=editcur.line; i < patlines-1; i++)
		memcpy(offset+(i*MULTIPLY), offset+((i+1)*MULTIPLY), 5);

	unsigned char blank[5]={255,255,255,0,0};
	memcpy(offset+(i*MULTIPLY),blank,5*sizeof(char));

//	drawTrackStart=editcur.track;
//	drawTrackEnd=editcur.track;
//	drawLineStart=editcur.line;
//	drawLineEnd=patlines-1;
	NewPatternDraw(editcur.track,editcur.track,editcur.line,patlines-1);

	Global::pInputHandler->bDoingSelection = false;
	ChordModeOffs = 0;
	bScrollDetatch=false;
	Repaint(DMData);
}

void CChildView::InsertCurr()
{
	// UNDO CODE INSERT
	int ps = _ps();
	unsigned char * offset = _offset(ps);
	int patlines = _pSong->patternLines[ps];

	AddUndo(ps,editcur.track,editcur.line,1,patlines-editcur.line,editcur.track,editcur.line,editcur.col,editPosition);

	for (int i=patlines-1; i > editcur.line; i--)
		memcpy(offset+(i*MULTIPLY), offset+((i-1)*MULTIPLY), 5);

	unsigned char blank[5]={255,255,255,0,0};
	memcpy(offset+(i*MULTIPLY),blank,5*sizeof(char));

//	drawTrackStart=editcur.track;
//	drawTrackEnd=editcur.track;
//	drawLineStart=editcur.line;
//	drawLineEnd=patlines-1;
	NewPatternDraw(editcur.track,editcur.track,editcur.line,patlines-1);

	Global::pInputHandler->bDoingSelection = false;
	ChordModeOffs = 0;
	bScrollDetatch=false;
	Repaint(DMData);
}



//////////////////////////////////////////////////////////////////////
// Plays entire row of the pattern

void CChildView::PlayCurrentRow(void)
{
	if (Global::pConfig->_followSong)
	{
		bScrollDetatch=false;
	}
	int displace=(_pSong->playOrder[editPosition]*MULTIPLY2) +
				(editcur.line*MULTIPLY);

	for (int i=0; i<_pSong->SONGTRACKS;i++)
	{
		PatternEntry* pEntry = (PatternEntry*)(_pSong->pPatternData+displace);
		if (pEntry->_mach != 255 && !_pSong->_trackMuted[i])
		{
			int mIndex;
			if ( pEntry->_mach & MAX_BUSES ) mIndex = _pSong->busEffect[(pEntry->_mach & (MAX_BUSES-1))];
			else if ( pEntry->_note == 122 ) mIndex = _pSong->busEffect[(pEntry->_mach &(MAX_BUSES-1))];
			else mIndex = _pSong->busMachine[(pEntry->_mach & (MAX_BUSES-1))];
			
			if (mIndex < MAX_MACHINES && _pSong->_machineActive[mIndex])
			{
				Machine *pMachine = _pSong->_pMachines[mIndex];
				
				if ( !pMachine->_mute)	pMachine->Tick(i, pEntry);
			}
		}
		displace+=5;
	}
}

void CChildView::PlayCurrentNote(void)
{
	if (Global::pConfig->_followSong)
	{
		bScrollDetatch=false;
	}
	const int displace=	(_pSong->playOrder[editPosition]*MULTIPLY2) +
						(editcur.line*MULTIPLY) + 
						(editcur.track*5);

	PatternEntry* pEntry = (PatternEntry*)(_pSong->pPatternData+displace);
	if (pEntry->_mach != 255)
	{
		int mIndex;
		if ( pEntry->_mach & MAX_BUSES ) mIndex = _pSong->busEffect[(pEntry->_mach & (MAX_BUSES-1))];
		else if ( pEntry->_note == 122 ) mIndex = _pSong->busEffect[(pEntry->_mach &(MAX_BUSES-1))];
		else mIndex = _pSong->busMachine[(pEntry->_mach & (MAX_BUSES-1))];
		
		if (mIndex < MAX_MACHINES && _pSong->_machineActive[mIndex])
		{
			Machine *pMachine = _pSong->_pMachines[mIndex];
			
			if ( !pMachine->_mute)	pMachine->Tick(editcur.track, pEntry);
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Cursor Moving Functions
void CChildView::PrevCol(bool wrap,bool updateDisplay)
{
	if(--editcur.col < 0)
	{
		editcur.col=8;
		pParentMain->StatusBarIdle();
		if (editcur.track == 0)
		{
			if ( wrap ) 
				editcur.track = _pSong->SONGTRACKS-1;
			else 
				editcur.col=0;
		}
		else 
			--editcur.track;
	}
	if (updateDisplay) 
	{
		Repaint(DMCursor);
	}
}

void CChildView::NextCol(bool wrap,bool updateDisplay)
{
	if (++editcur.col > 8)
	{
		editcur.col = 0;
		pParentMain->StatusBarIdle();
		if (editcur.track == _pSong->SONGTRACKS-1)
		{
			if ( wrap ) 
				editcur.track = 0;
			else 
				editcur.col=8;
		}
		else 
			++editcur.track;
	}
	if (updateDisplay) 
	{
		Repaint(DMCursor);
	}
}

void CChildView::PrevLine(int x, bool wrap,bool updateDisplay)
{
	const int nl = _pSong->patternLines[_ps()];

	editcur.line -= x;

	if(editcur.line<0)
	{
		if(wrap)
		{ 
			editcur.line = nl + editcur.line % nl; 
		}
		else	
		{ 
			editcur.line = 0;	
		}
	}
	pParentMain->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}

void CChildView::AdvanceLine(int x,bool wrap,bool updateDisplay)
{
	const int nl = _pSong->patternLines[_ps()];

	if ( x >= 0)	
	{
		editcur.line += x;
	}
	else
	{
		editcur.track+=1;
		if (editcur.track >= _pSong->SONGTRACKS)
		{
			editcur.track=0;
			editcur.line+=1;
		}
	}

	if (editcur.line >= nl)
	{
		if(wrap){ editcur.line = editcur.line % nl; }
		else	{ editcur.line = nl-1; }
	}

	pParentMain->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}

void CChildView::AdvanceTrack(int x,bool wrap,bool updateDisplay)
{
	editcur.track+=x;
	editcur.col=0;
	
	if(editcur.track>= _pSong->SONGTRACKS)
	{
		if ( wrap ) editcur.track=0;
		else editcur.track=_pSong->SONGTRACKS-1;
	}
	
	pParentMain->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}

void CChildView::PrevTrack(int x,bool wrap,bool updateDisplay)
{
	editcur.track-=x;
	editcur.col=0;
	
	if(editcur.track<0)
	{
		if (wrap) editcur.track=_pSong->SONGTRACKS-1;
		else editcur.track=0;
	}
	
	pParentMain->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}


//////////////////////////////////////////////////////////////////////
// Pattern Modifier functions ( Copy&paste , Transpose, ... )

void CChildView::patCut()
{
	if(viewMode == VMPattern)
	{
		// UNDO CODE PATT CUT
		const int ps = _ps();
		unsigned char *soffset = _pSong->pPatternData + (ps*MULTIPLY2);
		unsigned char blank[5]={255,255,255,0,0};

		patBufferLines = _pSong->patternLines[ps];
		AddUndo(ps,0,0,MAX_TRACKS,patBufferLines,editcur.track,editcur.line,editcur.col,editPosition);

		int length = patBufferLines*5*MAX_TRACKS;
		
		memcpy(patBufferData,soffset,length);
		for	(int c=0; c<length; c+=5)
		{
			memcpy(soffset,blank,sizeof(char)*5);
			soffset+=5;
		}
		patBufferCopy = true;

//		drawTrackStart=0;
//		drawTrackEnd=_pSong->SONGTRACKS;
//		drawLineStart=0;
//		drawLineEnd=patBufferLines-1;
		NewPatternDraw(0,_pSong->SONGTRACKS,0,patBufferLines-1);
		Repaint(DMData);
	}
}

void CChildView::patCopy()
{
	if(viewMode == VMPattern)
	{
		const int ps = _ps();
		unsigned char *soffset = _pSong->pPatternData + (ps*MULTIPLY2);
		
		patBufferLines=_pSong->patternLines[ps];
		int length=patBufferLines*5*MAX_TRACKS;
		
		memcpy(patBufferData,soffset,length);
		
		patBufferCopy=true;
	}
}

void CChildView::patPaste()
{
	// UNDO CODE PATT PASTE
	if(patBufferCopy && viewMode == VMPattern)
	{
		const int ps = _ps();
		unsigned char *soffset = _pSong->pPatternData + (ps*MULTIPLY2);
		// **************** funky shit goin on here yo with the pattern resize or some shit
		AddUndo(ps,0,0,MAX_TRACKS,_pSong->patternLines[ps],editcur.track,editcur.line,editcur.col,editPosition);
		if ( patBufferLines != _pSong->patternLines[_ps()] )
		{
			AddUndoLength(ps,_pSong->patternLines[ps],editcur.track,editcur.line,editcur.col,editPosition);
			_pSong->AllocNewPattern(_ps(),"",patBufferLines,false);
		}
		memcpy(soffset,patBufferData,patBufferLines*5*MAX_TRACKS);
		
//		drawTrackStart=0;
//		drawTrackEnd=_pSong->SONGTRACKS;
//		drawLineStart=0;
//		drawLineEnd=patBufferLines-1;
		NewPatternDraw(0,_pSong->SONGTRACKS,0,patBufferLines-1);
		Repaint(DMData);
	}
}

void CChildView::patMixPaste()
{
	// UNDO CODE PATT PASTE
	if(patBufferCopy && viewMode == VMPattern)
	{
		const int ps = _ps();
		unsigned char* offset_target = _pSong->pPatternData + (ps*MULTIPLY2);
		unsigned char* offset_source = patBufferData;
		// **************** funky shit goin on here yo with the pattern resize or some shit
		AddUndo(ps,0,0,MAX_TRACKS,_pSong->patternLines[ps],editcur.track,editcur.line,editcur.col,editPosition);
		if ( patBufferLines != _pSong->patternLines[_ps()] )
		{
			AddUndoLength(ps,_pSong->patternLines[ps],editcur.track,editcur.line,editcur.col,editPosition);
			_pSong->AllocNewPattern(_ps(),"",patBufferLines,false);
		}

		for (int i = 0; i < MAX_TRACKS*patBufferLines; i++)
		{
			if (*offset_target == 0xFF) *(offset_target)=*offset_source;
			if (*(offset_target+1)== 0xFF) *(offset_target+1)=*(offset_source+1);
			if (*(offset_target+2)== 0xFF) *(offset_target+2)=*(offset_source+2);
			if (*(offset_target+3)== 0) *(offset_target+3)=*(offset_source+3);
			if (*(offset_target+4)== 0) *(offset_target+4)=*(offset_source+4);
			offset_target+= 5;
			offset_source+= 5;
		}
		
//		drawTrackStart=0;
//		drawTrackEnd=_pSong->SONGTRACKS;
//		drawLineStart=0;
//		drawLineEnd=patBufferLines-1;
		NewPatternDraw(0,_pSong->SONGTRACKS,0,patBufferLines-1);
		Repaint(DMData);
	}
}

void CChildView::patDelete()
{
	if(viewMode == VMPattern)
	{
		// UNDO CODE PATT CUT
		const int ps = _ps();
		unsigned char *soffset = _pSong->pPatternData + (ps*MULTIPLY2);
		unsigned char blank[5]={255,255,255,0,0};

		patBufferLines = _pSong->patternLines[ps];
		AddUndo(ps,0,0,MAX_TRACKS,patBufferLines,editcur.track,editcur.line,editcur.col,editPosition);

		int length = patBufferLines*5*MAX_TRACKS;
		
		for	(int c=0; c<length; c+=5)
		{
			memcpy(soffset,blank,sizeof(char)*5);
			soffset+=5;
		}

		NewPatternDraw(0,_pSong->SONGTRACKS,0,patBufferLines-1);
		Repaint(DMData);
	}
}

void CChildView::patTranspose(int trp)
{
	// UNDO CODE PATT TRANSPOSE
	const int ps = _ps();
	unsigned char *soffset = _pSong->pPatternData + (ps*MULTIPLY2);

	if(viewMode == VMPattern)
	{
		int pLines=_pSong->patternLines[ps];
		int length=pLines*5*MAX_TRACKS;

		AddUndo(ps,0,0,MAX_TRACKS,pLines,editcur.track,editcur.line,editcur.col,editPosition);

		for	(int c=editcur.line*5*MAX_TRACKS;c<length;c+=5)
		{
			int not=*(soffset+c);
			
			if(not<120)
			{
				not+=trp;
				if(not<0)not=0;
				if(not>119)not=119;
				soffset[c]=static_cast<unsigned char>(not);
			}
		}
//		drawTrackStart=0;
//		drawTrackEnd=_pSong->SONGTRACKS;
//		drawLineStart=editcur.line;
//		drawLineEnd=pLines-1;
		NewPatternDraw(0,_pSong->SONGTRACKS,editcur.line,pLines-1);

		Repaint(DMData);
	}
}

void CChildView::StartBlock(int track,int line, int col)
{
	blockSel.start.track=track;
	blockSel.start.line=line;
	iniSelec = blockSel.start;

	if ( blockSelected )
	{
		if(blockSel.end.line<blockSel.start.line)
			blockSel.end.line=blockSel.start.line;
		
		if(blockSel.end.track<blockSel.start.track)
			blockSel.end.track=blockSel.start.track;
	}
	else
	{
		blockSel.end.line=line;
		blockSel.end.track=track;
	}
	blockSelected=true;

	Repaint(DMSelection);
}
void CChildView::ChangeBlock(int track,int line, int col)
{
	if ( blockSelected )
	{
		if ( track > iniSelec.track )
		{
			blockSel.start.track = iniSelec.track;
			blockSel.end.track = track;
		}
		else
		{
			blockSel.start.track = track;
			blockSel.end.track = iniSelec.track;
		}
		if ( line > iniSelec.line )
		{
			blockSel.start.line = iniSelec.line;
			blockSel.end.line = line;
		}
		else
		{
			blockSel.start.line = line;
			blockSel.end.line = iniSelec.line;
		}
	}
	else
	{
		blockSel.start.track=track;
		blockSel.start.line=line;
		blockSel.end.track=track;
		blockSel.end.line=line;
		iniSelec = blockSel.start;
	}
	blockSelected=true;

	Repaint(DMSelection);
}

void CChildView::EndBlock(int track,int line, int col)
{
	blockSel.end.track=track;
	blockSel.end.line=line;
	
	if ( blockSelected )
	{
		TRACE("%i,%i",blockSel.end.line,blockSel.start.line);
		if(blockSel.end.line<blockSel.start.line)
			blockSel.end.line=blockSel.start.line;
	
		TRACE("%i,%i",blockSel.end.track,blockSel.start.track);
		if(blockSel.end.track<blockSel.start.track)
			blockSel.end.track=blockSel.start.track;
	
	}
	else
	{
		blockSel.start.track=track;
		blockSel.start.line=line;
		iniSelec = blockSel.start;
	}
	blockSelected=true;

	Repaint(DMSelection);
}

void CChildView::BlockUnmark()
{
	blockSelected=false;

	Repaint(DMSelection);
}

void CChildView::CopyBlock(bool cutit)
{
	// UNDO CODE HERE CUT
	if(blockSelected)
	{
		isBlockCopied=true;
		blockNTracks=(blockSel.end.track-blockSel.start.track)+1;
		blockNLines=(blockSel.end.line-blockSel.start.line)+1;
		
		int ps=_pSong->playOrder[editPosition];
		int displace=ps*MULTIPLY2;
		
		int ls=0;
		int ts=0;
		unsigned char blank[5]={255,255,255,0,0};

		if (cutit)
		{
			AddUndo(ps,blockSel.start.track,blockSel.start.line,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);
		}
		for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
		{
			ls=0;
			for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
			{
				int const displace2=t*5+l*MULTIPLY;
				int const displace3=ts*5+ls*MULTIPLY;
				
				unsigned char *offset_source=_pSong->pPatternData+displace+displace2;				
				unsigned char *offset_target=blockBufferData+displace3;				
				
				memcpy(offset_target,offset_source,5*sizeof(char));
				
				if(cutit)
					memcpy(offset_source,blank,5*sizeof(char));
				
				++ls;
			}
			++ts;
		}
		if(cutit)
		{
	//		drawTrackStart=blockSel.start.track;
	//		drawTrackEnd=blockSel.end.track;
	//		drawLineStart=blockSel.start.line;
	//		drawLineEnd=blockSel.end.line;
			NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);

			Repaint(DMData);
		}
	}
}

void CChildView::DeleteBlock()
{
	// UNDO CODE HERE CUT
	if(blockSelected)
	{
		int ps=_pSong->playOrder[editPosition];
		int displace=ps*MULTIPLY2;
		
		int ls=0;
		int ts=0;
		unsigned char blank[5]={255,255,255,0,0};

		AddUndo(ps,blockSel.start.track,blockSel.start.line,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);
		for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
		{
			ls=0;
			for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
			{
				int const displace2=t*5+l*MULTIPLY;
				int const displace3=ts*5+ls*MULTIPLY;
				
				unsigned char *offset_source=_pSong->pPatternData+displace+displace2;				
				
				memcpy(offset_source,blank,5*sizeof(char));
				
				++ls;
			}
			++ts;
		}
		NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
		Repaint(DMData);
	}
}

void CChildView::PasteBlock(int tx,int lx,bool mix)
{
	// UNDO CODE PASTE AND MIX PASTE
	if(isBlockCopied)
	{
		int ps=_pSong->playOrder[editPosition];
		const int nl = _pSong->patternLines[ps];

		AddUndo(ps,tx,lx,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);

		int displace=ps*MULTIPLY2;
		
		int ls=0;
		int ts=0;
		
		for (int t=tx;t<tx+blockNTracks;t++)
		{
			ls=0;
			for (int l=lx;l<lx+blockNLines;l++)
			{
				if(l<nl && t<_pSong->SONGTRACKS)
				{
					int const displace2=t*5+l*MULTIPLY;
					int const displace3=ts*5+ls*MULTIPLY;
					unsigned char* offset_source=blockBufferData+displace3;
					unsigned char* offset_target=_pSong->pPatternData+displace+displace2;
					if ( mix )
					{
						if (*offset_target == 0xFF) *(offset_target)=*offset_source;
						if (*(offset_target+1)== 0xFF) *(offset_target+1)=*(offset_source+1);
						if (*(offset_target+2)== 0xFF) *(offset_target+2)=*(offset_source+2);
						if (*(offset_target+3)== 0) *(offset_target+3)=*(offset_source+3);
						if (*(offset_target+4)== 0) *(offset_target+4)=*(offset_source+4);
					}
					else
					{
						memcpy(offset_target,offset_source,5*sizeof(char));
					}
				}
				
				++ls;
			}
			++ts;
		}
		
		if (lx+blockNLines < nl ) editcur.line = lx+blockNLines;
		else editcur.line = nl-1;

//		drawTrackStart=tx;
//		drawTrackEnd=tx+blockNTracks-1;
//		drawLineStart=lx;
//		drawLineEnd=lx+blockNLines-1;
		bScrollDetatch=false;
		NewPatternDraw(tx,tx+blockNTracks-1,lx,lx+blockNLines-1);
		Repaint(DMData);
	}
	
}

void CChildView::SaveBlock(FILE* file)
{
	if(isBlockCopied)
	{
		fwrite(&blockNTracks, sizeof(int), 1, file);
		fwrite(&blockNLines, sizeof(int), 1, file);

		for (int t=0;t<blockNTracks;t++)
		{
			for (int l=0;l<blockNLines;l++)
			{
				int const displace2=t*5+l*MULTIPLY;
				
				unsigned char *offset_source=blockBufferData+displace2;
				
				fwrite(offset_source,sizeof(char),5,file);

			}
		}
	}
}
void CChildView::LoadBlock(FILE* file)
{
	fread(&blockNTracks,sizeof(int),1,file);
	fread(&blockNLines,sizeof(int),1,file);

	int patNum = _pSong->playOrder[editPosition];
	int nlines = _pSong->patternLines[patNum];
	AddUndo(patNum,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
	AddUndoLength(patNum,nlines,editcur.track,editcur.line,editcur.col,editPosition);

	for (int t=0;t<blockNTracks;t++)
	{
		for (int l=0;l<blockNLines;l++)
		{
			if(l<MAX_LINES && t<MAX_TRACKS)
			{
				int const displace2=t*5+l*MULTIPLY;
				unsigned char* offset_target=blockBufferData+displace2;
			
				fread(offset_target,sizeof(char),5,file);
			}
		}
	}
	isBlockCopied=true;		
}

void CChildView::DoubleLength()
{
	// UNDO CODE DOUBLE LENGTH
	const int displace=_pSong->playOrder[editPosition];
	unsigned char *toffset;
	unsigned char blank[5]={255,255,255,0,0};
	int st, et, sl, el,nl;

	if ( blockSelected )
	{
///////////////////////////////////////////////////////// Add ROW
		st=blockSel.start.track;		
		et=blockSel.end.track+1;
		sl=blockSel.start.line;			
		nl=((blockSel.end.line-sl)/2)+1;
		el=blockSel.end.line;
		AddUndo(_ps(),blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,nl*2,editcur.track,editcur.line,editcur.col,editPosition);
	}
	else 
	{
		st=0;		
		et=_pSong->SONGTRACKS;		
		sl=0;
		nl= _pSong->patternLines[displace]/2;	
		el=_pSong->patternLines[displace]-1;
		AddUndo(_ps(),0,0,MAX_TRACKS,el+1,editcur.track,editcur.line,editcur.col,editPosition);
	}

	for (int t=st;t<et;t++)
	{
		toffset=_pSong->pPatternData+displace*MULTIPLY2+t*5;
		memcpy(toffset+el*MULTIPLY,blank,5);
		for (int l=nl-1;l>0;l--)
		{
			memcpy(toffset+(sl+l*2)*MULTIPLY,toffset+(sl+l)*MULTIPLY,5);
			memcpy(toffset+(sl+(l*2)-1)*MULTIPLY,blank,5);
		}
	}

//	drawTrackStart=st;
//	drawTrackEnd=et;
//	drawLineStart=sl;
//	drawLineEnd=el;
	NewPatternDraw(st,et,sl,el);
	Repaint(DMData);
}

void CChildView::HalveLength()
{
	// UNDO CODE HALF LENGTH
	const int displace=_pSong->playOrder[editPosition];
	unsigned char *toffset;
	int st, et, sl, el,nl;
	unsigned char blank[5]={255,255,255,0,0};

	if ( blockSelected )
	{
///////////////////////////////////////////////////////// Add ROW
		st=blockSel.start.track;	
		et=blockSel.end.track+1;
		sl=blockSel.start.line;		
		nl=blockSel.end.line-sl+1;
		el=nl/2;
		AddUndo(_ps(),blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,nl,editcur.track,editcur.line,editcur.col,editPosition);
	}
	else 
	{
		st=0;	
		et=_pSong->SONGTRACKS;		
		sl=0;
		nl=_pSong->patternLines[displace];	
		el=_pSong->patternLines[displace]/2;
		AddUndo(_ps(),0,0,MAX_TRACKS,nl,editcur.track,editcur.line,editcur.col,editPosition);
	}
	
	for (int t=st;t<et;t++)
	{
		toffset=_pSong->pPatternData+displace*MULTIPLY2+t*5;
		for (int l=1;l<el;l++)
		{
			memcpy(toffset+(l+sl)*MULTIPLY,toffset+((l*2)+sl)*MULTIPLY,5);
		}
		while (l < nl)
		{
			memcpy(toffset+((l+sl)*MULTIPLY),blank,5);
			l++;
		}
	}

//	drawTrackStart=st;
//	drawTrackEnd=et;
//	drawLineStart=sl;
//	drawLineEnd=nl+sl;
	NewPatternDraw(st,et,sl,nl+sl);
	Repaint(DMData);
}


void CChildView::BlockTranspose(int trp)
{
	// UNDO CODE TRANSPOSE
	if ( blockSelected == true ) 
	{
		const int displace=_pSong->playOrder[editPosition]*MULTIPLY2;
		unsigned char *toffset=_pSong->pPatternData+displace;

		AddUndo(_ps(),blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,blockSel.end.line-blockSel.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

		for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
		{
			for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
			{
				const int displace2=t*5+l*MULTIPLY;
				
				int not=*(toffset+displace2);
			
				if(not<120)
				{
					not+=trp;
					if(not<0)not=0;
					if(not>119)not=119;
					toffset[displace2]=static_cast<unsigned char>(not);
				}
			}
		}
//		drawTrackStart=blockSel.start.track;
//		drawTrackEnd=blockSel.end.track;
//		drawLineStart=blockSel.start.line;
//		drawLineEnd=blockSel.end.line;
		NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
		Repaint(DMData);
	}
}


void CChildView::BlockGenChange(int x)
{
	// UNDO CODE BLOCK GENERATOR CHANGE
	if ( blockSelected == true ) 
	{
		const int displace=_pSong->playOrder[editPosition]*MULTIPLY2;
		unsigned char *toffset=_pSong->pPatternData+displace;

		AddUndo(_ps(),blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,blockSel.end.line-blockSel.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

		for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
		{
			for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
			{
				const int displace2=t*5+l*MULTIPLY+2;
				
				unsigned char gen=*(toffset+displace2);
				
				if ( gen != 255 )
				{
					gen=x;
					if(gen<0)gen=0;
					if(gen>=MAX_MACHINES)gen=MAX_MACHINES-1;
					toffset[displace2]=gen;
				}
			}
		}
//		drawTrackStart=blockSel.start.track;
//		drawTrackEnd=blockSel.end.track;
//		drawLineStart=blockSel.start.line;
//		drawLineEnd=blockSel.end.line;
		NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
		Repaint(DMData);
	}
}

void CChildView::BlockInsChange(int x)
{
	// UNDO CODE BLOCK INS CHANGE
	if ( blockSelected == true ) 
	{
		const int displace=_pSong->playOrder[editPosition]*MULTIPLY2;
		unsigned char *toffset=_pSong->pPatternData+displace;

		AddUndo(_ps(),blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,blockSel.end.line-blockSel.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

		for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
		{
			for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
			{
				const int displace2=t*5+l*MULTIPLY+1;
				
				unsigned char ins=*(toffset+displace2);
			
				if (ins != 255 )
				{
					ins=x;
					if(ins<0)ins=0;
					if(ins>255)ins=255;
					toffset[displace2]=ins;
				}
			}
		}
//		drawTrackStart=blockSel.start.track;
//		drawTrackEnd=blockSel.end.track;
//		drawLineStart=blockSel.start.line;
//		drawLineEnd=blockSel.end.line;
		NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
		Repaint(DMData);
	}
}

void CChildView::BlockParamInterpolate()
{
	// UNDO CODE BLOCK INTERPOLATE
	if (blockSelected)
	{
		///////////////////////////////////////////////////////// Add ROW
		const int displace=_pSong->playOrder[editPosition]*MULTIPLY2;
		unsigned char *toffset=_pSong->pPatternData+displace;
		
		AddUndo(_ps(),blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,blockSel.end.line-blockSel.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);
		
		const int initvalue = 
			*(toffset+blockSel.start.track*5+blockSel.start.line*MULTIPLY+3) * 0x100 +
			*(toffset+blockSel.start.track*5+blockSel.start.line*MULTIPLY+4);
		const int endvalue =
			*(toffset+blockSel.start.track*5+blockSel.end.line*MULTIPLY+3) * 0x100 +
			*(toffset+blockSel.start.track*5+blockSel.end.line*MULTIPLY+4);
		const float addvalue = (float)(endvalue -initvalue)/(blockSel.end.line-blockSel.start.line);
		const unsigned char comd = *(toffset+blockSel.start.track*5+blockSel.start.line*MULTIPLY+3);
		int displace2=(blockSel.start.track*5)+((blockSel.start.line)*MULTIPLY);
		
		if ( toffset[displace2] == 121 || toffset[displace2] == 122)
		{
			unsigned char note = toffset[displace2];
			unsigned char aux = toffset[displace2+1];
			unsigned char mac = toffset[displace2+2];
			displace2+=MULTIPLY;
			for (int l=blockSel.start.line+1;l<blockSel.end.line;l++)
			{
				toffset[displace2]=note;
				toffset[displace2+1]=aux;
				toffset[displace2+2]=mac;
				int val=f2i(initvalue+addvalue*(l-blockSel.start.line));
				toffset[displace2+3]=static_cast<unsigned char>(val/0x100);
				toffset[displace2+4]=static_cast<unsigned char>(val%0x100);
				displace2+=MULTIPLY;
			}
			toffset[displace2]=note;
			toffset[displace2+1]=aux;
			toffset[displace2+2]=mac;
		}
		else
		{
			displace2+=MULTIPLY;
			for (int l=blockSel.start.line+1;l<blockSel.end.line;l++)
			{
				int val=f2i(initvalue+addvalue*(l-blockSel.start.line));
				toffset[displace2+3]=static_cast<unsigned char>(val/0x100);
				toffset[displace2+4]=static_cast<unsigned char>(val%0x100);
				displace2+=MULTIPLY;
			}
		}
//		drawTrackStart=blockSel.start.track;
//		drawTrackEnd=blockSel.end.track;
//		drawLineStart=blockSel.start.line;
//		drawLineEnd=blockSel.end.line;
		NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
		Repaint(DMData);
	}
}


void CChildView::IncCurPattern()
{
	if(_pSong->playOrder[editPosition]<(MAX_PATTERNS-1))
	{
		AddUndoSequence(_pSong->playLength,editcur.track,editcur.line,editcur.col,editPosition);
		++_pSong->playOrder[editPosition];
		pParentMain->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}


void CChildView::DecCurPattern()
{
	if(_pSong->playOrder[editPosition]>0)
	{
		AddUndoSequence(_pSong->playLength,editcur.track,editcur.line,editcur.col,editPosition);
		--_pSong->playOrder[editPosition];
		pParentMain->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}

void CChildView::DecPosition()
{
//	case cdefPlaySkipBack:
	if (Global::pPlayer->_playing && Global::pConfig->_followSong)
	{
		if (Global::pPlayer->_playPosition > 0 )
		{
			bool b = Global::pPlayer->_playBlock;
			Global::pPlayer->Start(Global::pPlayer->_playPosition-1,0);
			Global::pPlayer->_playBlock = b;
		}
		else
		{
			bool b = Global::pPlayer->_playBlock;
			Global::pPlayer->Start(_pSong->playLength-1,0);
			Global::pPlayer->_playBlock = b;
		}
	}
	else
	{
		if(editPosition>0)
		{
			--editPosition;
		}
		else
		{
//			editPosition = _pSong->playLength-1;
			editPosition = 0;
		}
		
		memset(_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
		_pSong->playOrderSel[editPosition]=true;

		pParentMain->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}

void CChildView::IncPosition(bool bRepeat)
{
//	case cdefPlaySkipAhead:
	if (Global::pPlayer->_playing && Global::pConfig->_followSong)
	{
		if (Global::pPlayer->_playPosition < _pSong->playLength-1)
		{
			bool b = Global::pPlayer->_playBlock;
			Global::pPlayer->Start(Global::pPlayer->_playPosition+1,0);
			Global::pPlayer->_playBlock = b;
		}
		else
		{
			bool b = Global::pPlayer->_playBlock;
			Global::pPlayer->Start(0,0);
			Global::pPlayer->_playBlock = b;
		}
	}
	else 
	{
		if(editPosition < _pSong->playLength-1)
		{
			++editPosition;
		}
		else if (!bRepeat) // This prevents adding patterns when only trying to reach the end.
		{
//			editPosition = 0;
			++editPosition;
			AddUndoSequence(_pSong->playLength,editcur.track,editcur.line,editcur.col,editPosition-1);
			int const ep=_pSong->GetBlankPatternUnused();
			_pSong->playLength=editPosition+1;
			_pSong->playOrder[editPosition]=ep;
			pParentMain->UpdateSequencer();
		}

		memset(_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
		_pSong->playOrderSel[editPosition]=true;

		pParentMain->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}

void CChildView::SelectMachineUnderCursor()
{
	const int displace=_pSong->playOrder[editPosition]*MULTIPLY2;
	const int displace2 = editcur.track*5+editcur.line*MULTIPLY;
	unsigned char *toffset=_pSong->pPatternData+displace+displace2;

	PatternEntry *entry = (PatternEntry*) toffset;

	if ( entry->_mach < MAX_BUSES*2 ) _pSong->seqBus = entry->_mach;
	pParentMain->ChangeGen(_pSong->seqBus);
	if ( entry->_inst != 255 ) _pSong->auxcolSelected = entry->_inst;
	pParentMain->ChangeIns(_pSong->auxcolSelected);

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// undo/redo code
////////////////////////////////////////////////////////////////////////////////////////////////////////

void CChildView::AddUndo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pUndoList;
	pUndoList = pNew;

	// fill data
	unsigned char* pData = new unsigned char[tracks*lines*5*sizeof(char)];
	pNew->pData = pData;
	pNew->pattern = pattern;
	pNew->x = x;
	pNew->y = y;
	pNew->tracks = tracks;
	pNew->lines = lines;
	pNew->type = UNDO_PATTERN;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;

	int displace=pattern*MULTIPLY2;

	for (int t=x;t<x+tracks;t++)
	{
		for (int l=y;l<y+lines;l++)
		{
			int const displace2=t*5+l*MULTIPLY;
			
			unsigned char *offset_source=_pSong->pPatternData+displace+displace2;
			
			memcpy(pData,offset_source,5*sizeof(char));
			pData+=5*sizeof(char);
		}
	}
	if (bWipeRedo)
	{
		KillRedo();
		UndoCounter++;
		pNew->counter = UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void CChildView::AddRedo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pRedoList;
	pRedoList = pNew;
	// fill data
	unsigned char* pData = new unsigned char[tracks*lines*5*sizeof(char)];
	pNew->pData = pData;
	pNew->pattern = pattern;
	pNew->x = x;
	pNew->y = y;
	pNew->tracks = tracks;
	pNew->lines = lines;
	pNew->type = UNDO_PATTERN;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;
	pNew->counter = counter;

	int displace=pattern*MULTIPLY2;

	for (int t=x;t<x+tracks;t++)
	{
		for (int l=y;l<y+lines;l++)
		{
			int const displace2=t*5+l*MULTIPLY;
			
			unsigned char *offset_source=_pSong->pPatternData+displace+displace2;
			
			memcpy(pData,offset_source,5*sizeof(char));
			pData+=5*sizeof(char);
		}
	}
}

void CChildView::AddUndoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pUndoList;
	pUndoList = pNew;
	// fill data
	pNew->pData = NULL;
	pNew->pattern = pattern;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = lines;
	pNew->type = UNDO_LENGTH;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;

	if (bWipeRedo)
	{
		KillRedo();
		UndoCounter++;
		pNew->counter = UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void CChildView::AddRedoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pRedoList;
	pRedoList = pNew;
	// fill data
	pNew->pData = NULL;
	pNew->pattern = pattern;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = lines;
	pNew->type = UNDO_LENGTH;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;
	pNew->counter = counter;
}

void CChildView::AddUndoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pUndoList;
	pUndoList = pNew;
	// fill data
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS];
	memcpy(pNew->pData, _pSong->playOrder, MAX_SONG_POSITIONS*sizeof(char));
	pNew->pattern = 0;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = lines;
	pNew->type = UNDO_SEQUENCE;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;

	if (bWipeRedo)
	{
		KillRedo();
		UndoCounter++;
		pNew->counter = UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void CChildView::AddUndoSong(int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pUndoList;
	pUndoList = pNew;
	// fill data
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS+MAX_PATTERN_BUFFER_LEN];
	memcpy(pNew->pData, _pSong->playOrder, MAX_SONG_POSITIONS*sizeof(char));
	memcpy(pNew->pData+MAX_SONG_POSITIONS,_pSong->pPatternData,MAX_PATTERN_BUFFER_LEN*sizeof(char));
	pNew->pattern = 0;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = _pSong->playLength;
	pNew->type = UNDO_SONG;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;

	if (bWipeRedo)
	{
		KillRedo();
		UndoCounter++;
		pNew->counter = UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void CChildView::AddRedoSong(int edittrack, int editline, int editcol, int seqpos, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pRedoList;
	pRedoList = pNew;
	// fill data
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS+MAX_PATTERN_BUFFER_LEN];
	memcpy(pNew->pData, _pSong->playOrder, MAX_SONG_POSITIONS*sizeof(char));
	memcpy(pNew->pData+MAX_SONG_POSITIONS,_pSong->pPatternData,MAX_PATTERN_BUFFER_LEN*sizeof(char));
	pNew->pattern = 0;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = _pSong->playLength;
	pNew->type = UNDO_SONG;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;
	pNew->counter = counter;
}

void CChildView::AddRedoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pRedoList;
	pRedoList = pNew;
	// fill data
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS];
	memcpy(pNew->pData, _pSong->playOrder, MAX_SONG_POSITIONS*sizeof(char));
	pNew->pattern = 0;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = lines;
	pNew->type = UNDO_SEQUENCE;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;
	pNew->counter = counter;
}

void CChildView::OnEditUndo() 
{
	// TODO: Add your command handler code here
	if (pUndoList)
	{
		switch (pUndoList->type)
		{
		case UNDO_PATTERN:
			if(viewMode == VMPattern)// && bEditMode)
			{
				AddRedo(pUndoList->pattern,pUndoList->x,pUndoList->y,pUndoList->tracks,pUndoList->lines,editcur.track,editcur.line,editcur.col,pUndoList->seqpos,pUndoList->counter);
				// do undo
				int displace=pUndoList->pattern*MULTIPLY2;
				unsigned char* pData = pUndoList->pData;

				for (int t=pUndoList->x;t<pUndoList->x+pUndoList->tracks;t++)
				{
					for (int l=pUndoList->y;l<pUndoList->y+pUndoList->lines;l++)
					{
						int const displace2=t*5+l*MULTIPLY;
						
						unsigned char *offset_source=_pSong->pPatternData+displace+displace2;
						
						memcpy(offset_source,pData,5*sizeof(char));
						pData+=5*sizeof(char);
					}
				}
				// set up cursor
				editcur.track = pUndoList->edittrack;
				editcur.line = pUndoList->editline;
				editcur.col = pUndoList->editcol;
				if (pUndoList->seqpos == editPosition)
				{
					// display changes
//					drawTrackStart=pUndoList->x;
//					drawTrackEnd=pUndoList->x+pUndoList->tracks;
//					drawLineStart=pUndoList->y;
//					drawLineEnd=pUndoList->y+pUndoList->lines;
					NewPatternDraw(pUndoList->x,pUndoList->x+pUndoList->tracks,pUndoList->y,pUndoList->y+pUndoList->lines);
					Repaint(DMData);
				}
				else
				{
					editPosition = pUndoList->seqpos;
					pParentMain->UpdatePlayOrder(true);
					Repaint(DMPattern);
					
				}
				// delete undo from list
				SPatternUndo* pTemp = pUndoList->pPrev;
				delete (pUndoList->pData);
				delete (pUndoList);
				pUndoList = pTemp;
			}
			break;
		case UNDO_LENGTH:
			if(viewMode == VMPattern)// && bEditMode)
			{
				AddRedoLength(pUndoList->pattern,_pSong->patternLines[pUndoList->pattern],editcur.track,editcur.line,editcur.col,pUndoList->seqpos,pUndoList->counter);
				// do undo
				_pSong->patternLines[pUndoList->pattern]=pUndoList->lines;
				// set up cursor
				editcur.track = pUndoList->edittrack;
				editcur.line = pUndoList->editline;
				editcur.col = pUndoList->editcol;
				if (pUndoList->seqpos != editPosition)
				{
					editPosition = pUndoList->seqpos;
					pParentMain->UpdatePlayOrder(true);
				}
				// display changes
				Repaint(DMPattern);
				
				// delete undo from list
				SPatternUndo* pTemp = pUndoList->pPrev;
				delete (pUndoList->pData);
				delete (pUndoList);
				pUndoList = pTemp;
				break;
			}
		case UNDO_SEQUENCE:
			AddRedoSequence(_pSong->playLength,editcur.track,editcur.line,editcur.col,editPosition,pUndoList->counter);
			// do undo
			memcpy(_pSong->playOrder, pUndoList->pData, MAX_SONG_POSITIONS*sizeof(char));
			_pSong->playLength = pUndoList->lines;
			// set up cursor
			editcur.track = pUndoList->edittrack;
			editcur.line = pUndoList->editline;
			editcur.col = pUndoList->editcol;
			editPosition = pUndoList->seqpos;
			pParentMain->UpdatePlayOrder(true);
			pParentMain->UpdateSequencer();
			// display changes
			Repaint(DMPattern);
			
			// delete undo from list
			{
				SPatternUndo* pTemp = pUndoList->pPrev;
				delete (pUndoList->pData);
				delete (pUndoList);
				pUndoList = pTemp;
			}
			break;
		case UNDO_SONG:
			AddRedoSong(editcur.track,editcur.line,editcur.col,editPosition,pUndoList->counter);
			// do undo
			memcpy(_pSong->playOrder, pUndoList->pData, MAX_SONG_POSITIONS*sizeof(char));
			memcpy(_pSong->pPatternData,pUndoList->pData+MAX_SONG_POSITIONS,MAX_PATTERN_BUFFER_LEN*sizeof(char));
			_pSong->playLength = pUndoList->lines;
			// set up cursor
			editcur.track = pUndoList->edittrack;
			editcur.line = pUndoList->editline;
			editcur.col = pUndoList->editcol;
			editPosition = pUndoList->seqpos;
			pParentMain->UpdatePlayOrder(true);
			pParentMain->UpdateSequencer();
			// display changes
			Repaint(DMPattern);
			
			// delete undo from list
			{
				SPatternUndo* pTemp = pUndoList->pPrev;
				delete (pUndoList->pData);
				delete (pUndoList);
				pUndoList = pTemp;
			}
			break;

		}
		SetTitleBarText();
	}
}


void CChildView::OnEditRedo() 
{
	// TODO: Add your command handler code here
	if (pRedoList)
	{
		switch (pRedoList->type)
		{
		case UNDO_PATTERN:
			if(viewMode == VMPattern)// && bEditMode)
			{
				AddUndo(pRedoList->pattern,pRedoList->x,pRedoList->y,pRedoList->tracks,pRedoList->lines,editcur.track,editcur.line,editcur.col,pRedoList->seqpos,FALSE,pRedoList->counter);
				// do redo
				int displace=pRedoList->pattern*MULTIPLY2;
				unsigned char* pData = pRedoList->pData;

				for (int t=pRedoList->x;t<pRedoList->x+pRedoList->tracks;t++)
				{
					for (int l=pRedoList->y;l<pRedoList->y+pRedoList->lines;l++)
					{
						int const displace2=t*5+l*MULTIPLY;
						
						unsigned char *offset_source=_pSong->pPatternData+displace+displace2;
						
						memcpy(offset_source,pData,5*sizeof(char));
						pData+=5*sizeof(char);
					}
				}
				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				if (pRedoList->seqpos == editPosition)
				{
					// display changes
//					drawTrackStart=pRedoList->x;
//					drawTrackEnd=pRedoList->x+pRedoList->tracks;
//					drawLineStart=pRedoList->y;
//					drawLineEnd=pRedoList->y+pRedoList->lines;
					NewPatternDraw(pRedoList->x,pRedoList->x+pRedoList->tracks,pRedoList->y,pRedoList->y+pRedoList->lines);
					Repaint(DMData);
				}
				else
				{
					editPosition = pRedoList->seqpos;
					pParentMain->UpdatePlayOrder(true);
					Repaint(DMPattern);
					
				}
				// delete redo from list
				SPatternUndo* pTemp = pRedoList->pPrev;
				delete (pRedoList->pData);
				delete (pRedoList);
				pRedoList = pTemp;
			}
			break;
		case UNDO_LENGTH:
			if(viewMode == VMPattern)// && bEditMode)
			{
				AddUndoLength(pRedoList->pattern,_pSong->patternLines[pUndoList->pattern],editcur.track,editcur.line,editcur.col,pRedoList->seqpos,FALSE,pRedoList->counter);
				// do undo
				_pSong->patternLines[pRedoList->pattern]=pRedoList->lines;
				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				if (pRedoList->seqpos != editPosition)
				{
					editPosition = pRedoList->seqpos;
					pParentMain->UpdatePlayOrder(true);
				}
				// display changes
				Repaint(DMPattern);
				
				// delete redo from list
				SPatternUndo* pTemp = pRedoList->pPrev;
				delete (pRedoList->pData);
				delete (pRedoList);
				pRedoList = pTemp;
				break;
			}
		case UNDO_SEQUENCE:
			AddUndoSequence(_pSong->playLength,editcur.track,editcur.line,editcur.col,editPosition,FALSE,pRedoList->counter);
			// do undo
			memcpy(_pSong->playOrder, pRedoList->pData, MAX_SONG_POSITIONS*sizeof(char));
			_pSong->playLength = pRedoList->lines;
			// set up cursor
			editcur.track = pRedoList->edittrack;
			editcur.line = pRedoList->editline;
			editcur.col = pRedoList->editcol;
			editPosition = pRedoList->seqpos;
			pParentMain->UpdatePlayOrder(true);
			pParentMain->UpdateSequencer();
			// display changes
			Repaint(DMPattern);
			
			{
				// delete redo from list
				SPatternUndo* pTemp = pRedoList->pPrev;
				delete (pRedoList->pData);
				delete (pRedoList);
				pRedoList = pTemp;
			}
			break;
		case UNDO_SONG:
			AddUndoSong(editcur.track,editcur.line,editcur.col,editPosition,FALSE,pRedoList->counter);
			// do undo
			memcpy(_pSong->playOrder, pRedoList->pData, MAX_SONG_POSITIONS*sizeof(char));
			memcpy(_pSong->pPatternData,pRedoList->pData+MAX_SONG_POSITIONS,MAX_PATTERN_BUFFER_LEN*sizeof(char));
			_pSong->playLength = pRedoList->lines;
			// set up cursor
			editcur.track = pRedoList->edittrack;
			editcur.line = pRedoList->editline;
			editcur.col = pRedoList->editcol;
			editPosition = pRedoList->seqpos;
			pParentMain->UpdatePlayOrder(true);
			pParentMain->UpdateSequencer();
			// display changes
			Repaint(DMPattern);
			
			{
				// delete redo from list
				SPatternUndo* pTemp = pRedoList->pPrev;
				delete (pRedoList->pData);
				delete (pRedoList);
				pRedoList = pTemp;
			}
			break;
		}
		SetTitleBarText();
	}
}

void CChildView::KillRedo()
{
	while (pRedoList)
	{
		SPatternUndo* pTemp = pRedoList->pPrev;
		delete (pRedoList->pData);
		delete (pRedoList);
		pRedoList = pTemp;
	}
}

void CChildView::KillUndo()
{
	while (pUndoList)
	{
		SPatternUndo* pTemp = pUndoList->pPrev;
		delete (pUndoList->pData);
		delete (pUndoList);
		pUndoList = pTemp;
	}
	UndoCounter = 0;
	UndoSaved = 0;
//	SetTitleBarText();
}


void CChildView::SelectNextTrack()
{
	if(++editcur.track >= _pSong->SONGTRACKS)
		editcur.track=0;
	while(_pSong->_trackArmed[editcur.track] == 0)
	{
		if(++editcur.track >= _pSong->SONGTRACKS)
			editcur.track=0;
	}
	editcur.col = 0;
}
