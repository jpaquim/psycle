/** @file
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */
//////////////////////////////////////////////////////////////////////
// Keyboard Handler sourcecode

void CPsycleWTLView::KeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
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
	else if ((nChar == 16) && ((nFlags & 0xC000) == 0xC000) && ChordModeOffs)
	{
		// shift is up, abort chord mode
		editcur.line = m_ChordModeLine;
		editcur.track = m_ChordModeTrack;
		ChordModeOffs = 0;
		AdvanceLine(patStep,Global::pConfig->_wrapAround,true);
//		m_pMainFrame->StatusBarIdle();
//		Repaint(DMCursor);
	}
}

void CPsycleWTLView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// undo code not required, enter note handles it
	KeyUp(nChar, nRepCnt, nFlags);
	//SetMsgHandled(FALSE);
}

void CPsycleWTLView::KeyDown(UINT nChar, UINT nRepCnt, UINT nFlags )
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
				SetMsgHandled(FALSE);
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

void CPsycleWTLView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags )
{
	KeyDown(nChar, nRepCnt, nFlags);
	SetMsgHandled(TRUE);
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

void CPsycleWTLView::MidiPatternNote(int outnote, int velocity)
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

void CPsycleWTLView::MidiPatternTweak(int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

	// build entry
	PatternEntry entry;
	entry._mach = m_pSong->SeqBus();
	entry._cmd = (value>>8)&255;
	entry._parameter = value&255;
	entry._inst = command;
	entry._note = cdefTweakM;

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset; 

		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{		
				// play it
				Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

				// play
				if (pMachine)
				{
					pMachine->Tick(editcur.track,&entry);
				}
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *pentry = (PatternEntry*) toffset;
		if (pentry->_note >= 120)
		{
			if ((pentry->_mach != entry._mach) 
				|| (pentry->_cmd != entry._cmd)
				|| (pentry->_parameter != entry._parameter) 
				|| (pentry->_inst != entry._inst) 
				|| ((pentry->_note != cdefTweakM) && (pentry->_note != cdefTweakE) && (pentry->_note != cdefTweakS)))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				pentry->_mach = entry._mach;
				pentry->_cmd = entry._cmd;
				pentry->_parameter = entry._parameter;
				pentry->_inst = entry._inst;
				pentry->_note = entry._note;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
//	else
	{
		// play it
		Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

		if (pMachine)
		{
			// play
			pMachine->Tick(editcur.track,&entry);
		}
	}
}

void CPsycleWTLView::MidiPatternTweakSlide(int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

	// build entry
	PatternEntry entry;
	entry._mach = m_pSong->SeqBus();
	entry._cmd = (value>>8)&255;
	entry._parameter = value&255;
	entry._inst = command;
	entry._note = cdefTweakS;

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset; 

		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{		
				Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *pentry = (PatternEntry*) toffset;
		if (pentry->_note >= 120)
		{
			if ((pentry->_mach != entry._mach) 
				|| (pentry->_cmd != entry._cmd)
				|| (pentry->_parameter != entry._parameter) 
				|| (pentry->_inst != entry._inst) 
				|| ((pentry->_note != cdefTweakM) && (pentry->_note != cdefTweakE) && (pentry->_note != cdefTweakS)))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				pentry->_mach = entry._mach;
				pentry->_cmd = entry._cmd;
				pentry->_parameter = entry._parameter;
				pentry->_inst = entry._inst;
				pentry->_note = entry._note;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
//	else
	{
		Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

		if (pMachine)
		{
			// play
			pMachine->Tick(editcur.track,&entry);
		}
	}
}

void CPsycleWTLView::MidiPatternCommand(int command, int value)
{
	// UNDO CODE MIDI PATTERN
	if (value < 0) value = (0x80-value);// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xff) value = 0xff; // no else incase of neg overflow

	// build entry
	PatternEntry entry;
	entry._mach = m_pSong->SeqBus();
	entry._inst = m_pSong->AuxcolSelected();
	entry._cmd = command;
	entry._parameter = value;
	entry._note = 255;

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset; 

		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{		
				Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *pentry = (PatternEntry*) toffset;
		if ((pentry->_mach != entry._mach) 
			|| (pentry->_inst != entry._inst) 
			|| (pentry->_cmd != entry._cmd) 
			|| (pentry->_parameter != entry._parameter))
		{
			AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
			pentry->_mach = entry._mach;
			pentry->_cmd = entry._cmd;
			pentry->_parameter = entry._parameter;
			pentry->_inst = entry._inst;

			NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
			Repaint(DMData);
		}
	}
//	else
	{
		// play it
		Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

		if (pMachine)
		{
			// play
			pMachine->Tick(editcur.track,&entry);
		}
	}
}

void CPsycleWTLView::MidiPatternMidiCommand(int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

	PatternEntry entry;
	entry._mach = m_pSong->SeqBus();
	entry._cmd = (value>>8)&0xFF;
	entry._parameter = value&0xFF;
	entry._inst = command;
	entry._note = cdefMIDICC;

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset; 

		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{		
				Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *pentry = (PatternEntry*) toffset;
		if (pentry->_note >= 120)
		{
			if ((pentry->_mach != entry._mach) 
				|| (pentry->_cmd != entry._cmd) 
				|| (pentry->_parameter != entry._parameter) 
				|| (pentry->_inst != entry._inst) 
				|| (pentry->_note != cdefMIDICC))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				pentry->_mach = entry._mach;
				pentry->_cmd = entry._cmd;
				pentry->_parameter = entry._parameter;
				pentry->_inst = entry._inst;
				pentry->_note = entry._note;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
//	else
	{
		Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

		if (pMachine)
		{
			// play
			pMachine->Tick(editcur.track,&entry);
		}
	}
}

void CPsycleWTLView::MidiPatternInstrument(int value)
{
	// UNDO CODE MIDI PATTERN
	if (value < 0) value = (0x80-value);// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xff) value = 0xff; // no else incase of neg overflow

	// build entry
	PatternEntry entry;
	entry._mach = m_pSong->SeqBus();
	entry._inst = value;
	entry._cmd = 255;
	entry._parameter = 255;
	entry._note = 255;
	entry._volume = 0;
	entry._volcmd = 0;

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset; 

		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{		
				Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			line = editcur.line;
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *pentry = (PatternEntry*) toffset;
		if ((pentry->_mach != entry._mach) 
			|| (pentry->_inst != entry._inst))
		{
			AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
			pentry->_mach = entry._mach;
			pentry->_inst = entry._inst;

			NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
			Repaint(DMData);
		}
	}
//	else
	{
		Machine* pMachine = m_pSong->pMachine(m_pSong->SeqBus());

		if (pMachine)
		{
			// play
			pMachine->Tick(editcur.track,&entry);
		}
	}
}

void CPsycleWTLView::MousePatternTweak(int machine, int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset;
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{	
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			toffset = _ptrackline(ps);
			line = editcur.line;
		}

		// build entry
		PatternEntry *entry = (PatternEntry*) toffset;
		if (entry->_note >= 120)
		{
			if ((entry->_mach != machine) || (entry->_cmd != ((value>>8)&255)) || (entry->_parameter != (value&255)) || (entry->_inst != command) || ((entry->_note != cdefTweakM) && (entry->_note != cdefTweakE) && (entry->_note != cdefTweakS)))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				entry->_mach = machine;
				entry->_cmd = (value>>8)&255;
				entry->_parameter = value&255;
				entry->_inst = command;
				entry->_note = cdefTweakM;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
}

void CPsycleWTLView::MousePatternTweakSlide(int machine, int command, int value)
{
	// UNDO CODE MIDI PATTERN TWEAK
	if (value < 0) value = 0x8000-value;// according to doc psycle uses this weird negative format, but in reality there are no negatives for tweaks..
	if (value > 0xffff) value = 0xffff;// no else incase of neg overflow
	if(viewMode == VMPattern && bEditMode)
	{ 
		// write effect
		const int ps = _ps();
		int line = Global::pPlayer->_lineCounter;
		unsigned char * toffset;
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			if(m_pSong->TrackArmedCount())
			{
				SelectNextTrack();
			}
			else if (!Global::pConfig->_RecordUnarmed)
			{	
				return;
			}
			toffset = _ptrack(ps)+(line*MULTIPLY);
		}
		else
		{
			toffset = _ptrackline(ps);
			line = editcur.line;
		}

		// build entry
		PatternEntry *entry = (PatternEntry*) toffset;
		if (entry->_note >= 120)
		{
			if ((entry->_mach != machine) || (entry->_cmd != ((value>>8)&255)) || (entry->_parameter != (value&255)) || (entry->_inst != command) || ((entry->_note != cdefTweakM) && (entry->_note != cdefTweakE) && (entry->_note != cdefTweakS)))
			{
				AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				entry->_mach = machine;
				entry->_cmd = (value>>8)&255;
				entry->_parameter = value&255;
				entry->_inst = command;
				entry->_note = cdefTweakS;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
				Repaint(DMData);
			}
		}
	}
}


void CPsycleWTLView::EnterNote(int note, int velocity, bool bTranspose)
{
	int line;

	// UNDO CODE ENTER NOTE
	const int ps = _ps();
	unsigned char * toffset;
	
	if (note < 0 || note > cdefTweakS ) return;

	// octave offset
	if(note<120)
	{
		if(bTranspose)
			note+=m_pSong->CurrentOctave() * 12;

		if (note > 119) 
			note = 119;
	}
	
	// realtime note entering
	if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
	{
		if(m_pSong->TrackArmedCount())
		{
			if (velocity == 0)
			{
				for (int i = 0; i < m_pSong->SongTracks(); i++)
				{
					if (m_pSong->IsTrackArmed(i))
					{
						if (Global::pInputHandler->notetrack[i] == note)
						{
							editcur.track = i;
							break;
						}
					}
				}
				if (i == m_pSong->SongTracks())
				{
					Global::pInputHandler->StopNote(note,false);
					return;
				}
			}
			else
			{
				SelectNextTrack();
			}
		}
		else if (!Global::pConfig->_RecordUnarmed)
		{
			// build entry
			PatternEntry entry;
			entry._note = note;
			entry._mach = m_pSong->SeqBus();

			if ( note < 120)
			{
				if (Global::pConfig->_RecordTweaks && Global::pConfig->_midiRecordVel )
				{
					// command
					// entry._cmd = Global::pConfig->_midiCommandVel;
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
					entry._volume = par;
					entry._volcmd = PatternCmd::VELOCITY;//CMD::VOLUME
				} else {
					int v = Global::pConfig->_midiFromVel + 
										(((Global::pConfig->_midiToVel - Global::pConfig->_midiFromVel) * 64)/127);
					if (v > 255) 
					{
						v = 255;
					}
					else if (v < 0) 
					{
						v = 0;
					}
					entry._volume = v;
					entry._volcmd = PatternCmd::VELOCITY;//CMD::VOLUME
				}
			}

			if (note>120)
			{
				entry._inst = m_pSong->AuxcolSelected();
			}

			Machine *tmac = m_pSong->pMachine(m_pSong->SeqBus());
			if (tmac)
			{
				if (tmac->_type == MACH_SAMPLER)
				{
					entry._inst = m_pSong->AuxcolSelected();
				}
				else if (tmac->_type == MACH_VST) // entry->_inst is the MIDI channel for VSTi's
				{
					entry._inst = m_pSong->AuxcolSelected();
				}
				
				if ( note < 120)
				{
					tmac->Tick(editcur.track, &entry);
				}
			}
			Global::pInputHandler->notetrack[editcur.track]=note;
			return;
		}
		line = Global::pPlayer->_lineCounter;
		toffset = _ptrack(ps)+(line*MULTIPLY);
		ChordModeOffs = 0;
	}
	else 
	{
		if ((GetKeyState(VK_SHIFT)<0) && (note != cdefTweakM) && (note != cdefTweakE) && (note != cdefTweakS) && (note != cdefMIDICC))
		{
			if (ChordModeOffs == 0)
			{
				m_ChordModeLine = editcur.line;
				m_ChordModeTrack = editcur.track;
			}
			editcur.track = (m_ChordModeTrack+ChordModeOffs)%m_pSong->SongTracks();
			editcur.line = line = m_ChordModeLine;
			toffset = _ptrackline(ps, editcur.track, line);
			ChordModeOffs++;
		}
		else
		{
			if (ChordModeOffs) // this should never happen because the shift check should catch it... but..
			{					// ok pooplog, now it REALLY shouldn't happen (now that the shift check works)
				editcur.line = m_ChordModeLine;
				editcur.track = m_ChordModeTrack;
				ChordModeOffs = 0;
				AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
			}
			line = editcur.line;
			toffset = _ptrackline(ps);
		}
	}

	// build entry
	PatternEntry *entry = (PatternEntry*) toffset;
	if (velocity==0)
	{
		Global::pInputHandler->StopNote(note,false);
		if (entry->_note == note)
		{
			return;
		}
		note = 120;
	}
	AddUndo(ps,editcur.track,line,1,1,editcur.track,line,editcur.col,editPosition);
	entry->_note = note;
	entry->_mach = m_pSong->SeqBus();

	if ( note < 120)
	{
		if (Global::pConfig->_RecordTweaks && Global::pConfig->_midiRecordVel)
		{
			// command
			//entry->_cmd = Global::pConfig->_midiCommandVel;
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
			//entry->_parameter = par;
			entry->_volume = par;
			entry->_volcmd = PatternCmd::VELOCITY;
		} else {
			int v = Global::pConfig->_midiFromVel + 
								(((Global::pConfig->_midiToVel - Global::pConfig->_midiFromVel) * 64)/127);
			if (v > 255) 
			{
				v = 255;
			}
			else if (v < 0) 
			{
				v = 0;
			}
			entry->_volume = v;
			entry->_volcmd = PatternCmd::VELOCITY;
		}
	}

	if (note>120)
	{
		entry->_inst = m_pSong->AuxcolSelected();
	}

	Machine *tmac = m_pSong->pMachine(m_pSong->SeqBus());
	if (tmac)
	{
		if (tmac->_type == MACH_SAMPLER)
		{
			entry->_inst = m_pSong->AuxcolSelected();
		}
		else if (tmac->_type == MACH_VST) // entry->_inst is the MIDI channel for VSTi's
		{
			entry->_inst = m_pSong->AuxcolSelected();
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

void CPsycleWTLView::EnterNoteoffAny()
{
	if (viewMode == VMPattern)
	{
		// UNDO CODE ENTER NOTE
		const int ps = _ps();
		unsigned char * toffset;
		
		// realtime note entering
		if (Global::pPlayer->_playing&&Global::pConfig->_followSong)
		{
			toffset = _ptrack(ps)+(Global::pPlayer->_lineCounter*MULTIPLY);
		}
		else
		{
			toffset = _ptrackline(ps);
		}

		// build entry
		PatternEntry *entry = (PatternEntry*) toffset;
		AddUndo(ps,editcur.track,editcur.line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
		entry->_note = 120;

		Global::pInputHandler->notetrack[editcur.track]=120;

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

bool CPsycleWTLView::MSBPut(int nChar)
{
	// UNDO CODE MSB PUT
	// init
	const int ps = _ps();
	unsigned char * toffset = _ptrackline(ps) + (editcur.col+1)/2;

	int oldValue = *toffset;	
	int sValue = -1;

	if	(	nChar>='0'		&&	nChar<='9')			{ sValue = nChar - '0'; }
	else if(nChar>=VK_NUMPAD0&&nChar<=VK_NUMPAD9)	{ sValue = nChar - VK_NUMPAD0; }
	else if(nChar>='A'		&&	nChar<='F')			{ sValue = nChar - 'A' + 10; }
	else											{ return false; }

	if (editcur.col < 5 && oldValue == 255)	{ oldValue = 0; }

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
		case 9:
		case 11:
		case 10:
			NextCol(false,false);
			break;
		case 8:
		case 12:
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
	NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
	Repaint(DMData);
	return true;
}

void CPsycleWTLView::ClearCurr() // delete content at Cursor pos.
{
	// UNDO CODE CLEAR
	const int ps = _ps();
	unsigned char * offset = _ptrack(ps);
	unsigned char * toffset = _ptrackline(ps);

	AddUndo(ps,editcur.track,editcur.line,1,1,editcur.track,editcur.line,editcur.col,editPosition);

	// &&&&& hardcoded # of bytes per event
	if ( editcur.col == 0 )
	{
		memset(offset+(editcur.line*MULTIPLY),255,3*sizeof(char));
		memset(offset+(editcur.line*MULTIPLY)+3,0,3*sizeof(char));
	}
	else if (editcur.col < 5 )	{	*(toffset+(editcur.col+1)/2)= 255; }
	else						{	*(toffset+(editcur.col+1)/2)= 0; }

	NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);

	AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
	Global::pInputHandler->bDoingSelection = false;
	ChordModeOffs = 0;
	bScrollDetatch=false;
	Repaint(DMData);
}

void CPsycleWTLView::DeleteCurr()
{
	// UNDO CODE DELETE
	const int ps = _ps();
	unsigned char * offset = _ptrack(ps);
	int patlines = m_pSong->PatternLines(ps);

	if ( Global::pInputHandler->bFT2DelBehaviour )
	{
		if(editcur.line==0)
			return;
		else
			editcur.line--;
	}

	AddUndo(ps,editcur.track,editcur.line,1,patlines-editcur.line,editcur.track,editcur.line,editcur.col,editPosition);

	for (int i=editcur.line; i < patlines-1; i++)
		memcpy(offset+(i*MULTIPLY), offset+((i+1)*MULTIPLY), EVENT_SIZE);

	//unsigned char blank[5]={255,255,255,0,0};
	memcpy(offset+(i*MULTIPLY),BLANK_EVENT,EVENT_SIZE);

	NewPatternDraw(editcur.track,editcur.track,editcur.line,patlines-1);

	Global::pInputHandler->bDoingSelection = false;
	ChordModeOffs = 0;
	bScrollDetatch=false;
	Repaint(DMData);
}

void CPsycleWTLView::InsertCurr()
{
	// UNDO CODE INSERT
	const int ps = _ps();
	unsigned char * offset = _ptrack(ps);
	int patlines = m_pSong->PatternLines(ps);

	AddUndo(ps,editcur.track,editcur.line,1,patlines-editcur.line,editcur.track,editcur.line,editcur.col,editPosition);

	for (int i=patlines-1; i > editcur.line; i--)
		memcpy(offset+(i*MULTIPLY), offset+((i-1)*MULTIPLY), EVENT_SIZE);

	//unsigned char blank[5]={255,255,255,0,0};
	memcpy(offset+(i*MULTIPLY),BLANK_EVENT,EVENT_SIZE * sizeof(char));

	NewPatternDraw(editcur.track,editcur.track,editcur.line,patlines-1);

	Global::pInputHandler->bDoingSelection = false;
	ChordModeOffs = 0;
	bScrollDetatch=false;
	Repaint(DMData);
}



//////////////////////////////////////////////////////////////////////
// Plays entire row of the pattern

void CPsycleWTLView::PlayCurrentRow(void)
{
	if (Global::pConfig->_followSong)
	{
		bScrollDetatch=false;
	}
	PatternEntry* pEntry = (PatternEntry*)_ptrackline(_ps(),0,editcur.line);

	for (int i = 0; i < m_pSong->SongTracks();i++)
	{
		if (pEntry->_mach < MAX_MACHINES && !m_pSong->IsTrackMuted(i))
		{
			Machine *pMachine = m_pSong->pMachine(pEntry->_mach);
			if (pMachine)
			{
				if ( !pMachine->_mute)	
				{
					pMachine->Tick(i, pEntry);
				}
			}
		}
		pEntry++;
	}
}

void CPsycleWTLView::PlayCurrentNote(void)
{
	if (Global::pConfig->_followSong)
	{
		bScrollDetatch=false;
	}

	PatternEntry* pEntry = (PatternEntry*)_ptrackline();
	if (pEntry->_mach < MAX_MACHINES)
	{
		Machine *pMachine = m_pSong->pMachine(pEntry->_mach);
		if (pMachine)
		{
			if ( !pMachine->_mute)	
			{
				pMachine->Tick(editcur.track, pEntry);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////
// Cursor Moving Functions
void CPsycleWTLView::PrevCol(bool wrap,bool updateDisplay)
{
	if(--editcur.col < EDIT_COLUMNS_START)
	{
		editcur.col = EDIT_COLUMNS_END;
		m_pMainFrame->StatusBarIdle();
		if (editcur.track == 0)
		{
			if ( wrap ) 
				editcur.track = m_pSong->SongTracks() - 1;
			else 
				editcur.col = EDIT_COLUMNS_START;
		}
		else 
			--editcur.track;
	}
	if (updateDisplay) 
	{
		Repaint(DMCursor);
	}
}

void CPsycleWTLView::NextCol(bool wrap,bool updateDisplay)
{
	if (++editcur.col > EDIT_COLUMNS_END)
	{
		editcur.col = 0;
		m_pMainFrame->StatusBarIdle();
		if (editcur.track == m_pSong->SongTracks() - 1)
		{
			if ( wrap ) 
				editcur.track = 0;
			else 
				editcur.col=EDIT_COLUMNS_END;
		}
		else 
			++editcur.track;
	}
	if (updateDisplay) 
	{
		Repaint(DMCursor);
	}
}

void CPsycleWTLView::PrevLine(int x, bool wrap,bool updateDisplay)
{
	const int nl = m_pSong->PatternLines(_ps());

	editcur.line -= x;

	if(editcur.line<EDIT_COLUMNS_START)
	{
		if(wrap)
		{ 
			editcur.line = nl + editcur.line % nl; 
		}
		else	
		{ 
			editcur.line = EDIT_COLUMNS_START;	
		}
	}
	m_pMainFrame->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}

void CPsycleWTLView::AdvanceLine(int x,bool wrap,bool updateDisplay)
{
	const int nl = m_pSong->PatternLines(_ps());

	if ( x >= 0)	
	{
		editcur.line += x;
	}
	else
	{
		editcur.track+=1;
		if (editcur.track >= m_pSong->SongTracks())
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

	m_pMainFrame->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}

void CPsycleWTLView::AdvanceTrack(int x,bool wrap,bool updateDisplay)
{
	editcur.track+=x;
	editcur.col=0;
	
	if(editcur.track>= m_pSong->SongTracks())
	{
		if ( wrap ) editcur.track=0;
		else editcur.track = m_pSong->SongTracks() - 1;
	}
	
	m_pMainFrame->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}

void CPsycleWTLView::PrevTrack(int x,bool wrap,bool updateDisplay)
{
	editcur.track-=x;
	editcur.col=0;
	
	if(editcur.track<0)
	{
		if (wrap) editcur.track=m_pSong->SongTracks() - 1;
		else editcur.track=0;
	}
	
	m_pMainFrame->StatusBarIdle();
	if (updateDisplay) Repaint(DMCursor);
}


//////////////////////////////////////////////////////////////////////
// Pattern Modifier functions ( Copy&paste , Transpose, ... )

void CPsycleWTLView::patCut()
{
	if(viewMode == VMPattern)
	{
		// UNDO CODE PATT CUT
		const int ps = _ps();
		unsigned char *soffset = _ppattern(ps);
		//unsigned char blank[5]={255,255,255,0,0};

		m_PatBufferLines = m_pSong->PatternLines(ps);
		AddUndo(ps,0,0,MAX_TRACKS,m_PatBufferLines,editcur.track,editcur.line,editcur.col,editPosition);

		int length = m_PatBufferLines*EVENT_SIZE*MAX_TRACKS;
		
		memcpy(m_PatBufferData,soffset,length);
		for	(int c=0; c<length; c+=EVENT_SIZE)
		{
			memcpy(soffset,BLANK_EVENT,EVENT_SIZE);
			soffset+=EVENT_SIZE;
		}
		m_PatBufferCopy = true;

		NewPatternDraw(0,m_pSong->SongTracks(),0,m_PatBufferLines - 1);
		Repaint(DMData);
	}
}

void CPsycleWTLView::patCopy()
{
	if(viewMode == VMPattern)
	{
		const int ps = _ps();
		unsigned char *soffset = _ppattern(ps);
		
		m_PatBufferLines = m_pSong->PatternLines(ps);
		int length = m_PatBufferLines * EVENT_SIZE * MAX_TRACKS;
		
		memcpy(m_PatBufferData,soffset,length);
		
		m_PatBufferCopy = true;
	}
}

void CPsycleWTLView::patPaste()
{
	// UNDO CODE PATT PASTE
	if(m_PatBufferCopy && viewMode == VMPattern)
	{
		const int ps = _ps();
		unsigned char *soffset = _ppattern(ps);
		// **************** funky shit goin on here yo with the pattern resize or some shit
		AddUndo(ps,0,0,MAX_TRACKS,m_pSong->PatternLines(ps),editcur.track,editcur.line,editcur.col,editPosition);
		if ( m_PatBufferLines != m_pSong->PatternLines(ps) )
		{
			AddUndoLength(ps,m_pSong->PatternLines(ps),editcur.track,editcur.line,editcur.col,editPosition);
			m_pSong->AllocNewPattern(ps,_T(""),m_PatBufferLines,false);
		}
		
		memcpy(soffset,m_PatBufferData,m_PatBufferLines * EVENT_SIZE * MAX_TRACKS);
		Repaint(DMPattern);
	}
}

void CPsycleWTLView::patMixPaste()
{
	// UNDO CODE PATT PASTE
	if(m_PatBufferCopy && viewMode == VMPattern)
	{
		const int ps = _ps();
		unsigned char* offset_target = _ppattern(ps);
		unsigned char* offset_source = m_PatBufferData;
		// **************** funky shit goin on here yo with the pattern resize or some shit
		AddUndo(ps,0,0,MAX_TRACKS,m_pSong->PatternLines(ps),editcur.track,editcur.line,editcur.col,editPosition);
		if ( m_PatBufferLines != m_pSong->PatternLines(ps) )
		{
			AddUndoLength(ps,m_pSong->PatternLines(ps),editcur.track,editcur.line,editcur.col,editPosition);
			m_pSong->AllocNewPattern(ps,_T(""),m_PatBufferLines,false);
		}

		for (int i = 0; i < MAX_TRACKS*m_PatBufferLines; i++)
		{
			if (*offset_target == 0xFF) *(offset_target)=*offset_source;
			if (*(offset_target+1)== 0xFF) *(offset_target+1)=*(offset_source+1);
			if (*(offset_target+2)== 0xFF) *(offset_target+2)=*(offset_source+2);
			if (*(offset_target+3)== 0) *(offset_target+3)=*(offset_source+3);
			if (*(offset_target+4)== 0) *(offset_target+4)=*(offset_source+4);
			if (*(offset_target+5)== 0) *(offset_target+5)=*(offset_source+5);
			if (*(offset_target+6)== 0) *(offset_target+6)=*(offset_source+6);
			offset_target+= EVENT_SIZE;
			offset_source+= EVENT_SIZE;
		}
		
		Repaint(DMPattern);
	}
}

void CPsycleWTLView::patDelete()
{
	if(viewMode == VMPattern)
	{
		// UNDO CODE PATT CUT
		const int ps = _ps();
		unsigned char *soffset = _ppattern(ps);
//		unsigned char blank[5]={255,255,255,0,0};

		m_PatBufferLines = m_pSong->PatternLines(ps);
		AddUndo(ps,0,0,MAX_TRACKS,m_PatBufferLines,editcur.track,editcur.line,editcur.col,editPosition);

		int length = m_PatBufferLines*EVENT_SIZE*MAX_TRACKS;
		
		for	(int c=0; c<length; c+=EVENT_SIZE)
		{
			memcpy(soffset,BLANK_EVENT,EVENT_SIZE);
			soffset+=EVENT_SIZE;
		}

		NewPatternDraw(0,m_pSong->SongTracks(),0,m_PatBufferLines-1);
		Repaint(DMData);
	}
}

void CPsycleWTLView::patTranspose(int trp)
{
	// UNDO CODE PATT TRANSPOSE
	const int ps = _ps();
	unsigned char *soffset = _ppattern(ps);

	if(viewMode == VMPattern)
	{
		int pLines = m_pSong->PatternLines(ps);
		int length = pLines * EVENT_SIZE * MAX_TRACKS;

		AddUndo(ps,0,0,MAX_TRACKS,pLines,editcur.track,editcur.line,editcur.col,editPosition);

		for	(int c=editcur.line*EVENT_SIZE*MAX_TRACKS;c<length;c+=EVENT_SIZE)
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
		NewPatternDraw(0,m_pSong->SongTracks(),editcur.line,pLines-1);

		Repaint(DMData);
	}
}

void CPsycleWTLView::StartBlock(int track,int line, int col)
{
	m_BlockSelection.start.track=track;
	m_BlockSelection.start.line=line;
	m_IniSelection = m_BlockSelection.start;

	if ( blockSelected )
	{
		if(m_BlockSelection.end.line<m_BlockSelection.start.line)
			m_BlockSelection.end.line=m_BlockSelection.start.line;
		
		if(m_BlockSelection.end.track<m_BlockSelection.start.track)
			m_BlockSelection.end.track=m_BlockSelection.start.track;
	}
	else
	{
		m_BlockSelection.end.line=line;
		m_BlockSelection.end.track=track;
	}
	blockSelected=true;

	Repaint(DMSelection);
}
void CPsycleWTLView::ChangeBlock(int track,int line, int col)
{
	if ( blockSelected )
	{
		if ( track > m_IniSelection.track )
		{
			m_BlockSelection.start.track = m_IniSelection.track;
			m_BlockSelection.end.track = track;
		}
		else
		{
			m_BlockSelection.start.track = track;
			m_BlockSelection.end.track = m_IniSelection.track;
		}
		if ( line > m_IniSelection.line )
		{
			m_BlockSelection.start.line = m_IniSelection.line;
			m_BlockSelection.end.line = line;
		}
		else
		{
			m_BlockSelection.start.line = line;
			m_BlockSelection.end.line = m_IniSelection.line;
		}
	}
	else
	{
		m_BlockSelection.start.track=track;
		m_BlockSelection.start.line=line;
		m_BlockSelection.end.track=track;
		m_BlockSelection.end.line=line;
		m_IniSelection = m_BlockSelection.start;
	}
	blockSelected=true;

	Repaint(DMSelection);
}

void CPsycleWTLView::EndBlock(int track,int line, int col)
{
	m_BlockSelection.end.track=track;
	m_BlockSelection.end.line=line;
	
	if ( blockSelected )
	{
		ATLTRACE(_T("%i,%i"),m_BlockSelection.end.line,m_BlockSelection.start.line);
		if(m_BlockSelection.end.line<m_BlockSelection.start.line)
		{
			int tmp = m_BlockSelection.start.line;
			m_BlockSelection.start.line=m_BlockSelection.end.line;
			m_BlockSelection.end.line=tmp;
//			m_BlockSelection.end.line=m_BlockSelection.start.line;
		}
		
		ATLTRACE(_T("%i,%i"),m_BlockSelection.end.track,m_BlockSelection.start.track);
		if(m_BlockSelection.end.track<m_BlockSelection.start.track)
		{
			int tmp = m_BlockSelection.start.track;
			m_BlockSelection.start.track=m_BlockSelection.end.track;
			m_BlockSelection.end.track=tmp;
//			m_BlockSelection.end.track=m_BlockSelection.start.track;
		}
		
	}
	else
	{
		m_BlockSelection.start.track=track;
		m_BlockSelection.start.line=line;
		m_IniSelection = m_BlockSelection.start;
	}
	blockSelected=true;

	Repaint(DMSelection);
}

void CPsycleWTLView::BlockUnmark()
{
	blockSelected=false;

	Repaint(DMSelection);
}

void CPsycleWTLView::CopyBlock(bool cutit)
{
	// UNDO CODE HERE CUT
	if(blockSelected)
	{
		isBlockCopied=true;
		blockNTracks=(m_BlockSelection.end.track-m_BlockSelection.start.track)+1;
		blockNLines=(m_BlockSelection.end.line-m_BlockSelection.start.line)+1;
		
		int ps=m_pSong->PlayOrder(editPosition);
		
		int ls=0;
		int ts=0;
		//unsigned char blank[5]={255,255,255,0,0};

		if (cutit)
		{
			AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);
		}
		for (int t=m_BlockSelection.start.track;t<m_BlockSelection.end.track+1;t++)
		{
			ls=0;
			for (int l=m_BlockSelection.start.line;l<m_BlockSelection.end.line+1;l++)
			{
				unsigned char *offset_target=m_BlockBufferData+(ts*EVENT_SIZE+ls*MULTIPLY);				
				unsigned char *offset_source=_ptrackline(ps,t,l);
				
				memcpy(offset_target,offset_source,EVENT_SIZE);
				
				if(cutit)
					memcpy(offset_source,BLANK_EVENT,EVENT_SIZE);
				
				++ls;
			}
			++ts;
		}
		if(cutit)
		{
			NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);

			Repaint(DMData);
		}
	}
}

void CPsycleWTLView::DeleteBlock()
{
	// UNDO CODE HERE CUT
	if(blockSelected)
	{
		int ps=m_pSong->PlayOrder(editPosition);
		
		//unsigned char blank[5]={255,255,255,0,0};

		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);
		for (int t=m_BlockSelection.start.track;t<m_BlockSelection.end.track+1;t++)
		{
			for (int l=m_BlockSelection.start.line;l<m_BlockSelection.end.line+1;l++)
			{
				memcpy(_ptrackline(ps,t,l),BLANK_EVENT,EVENT_SIZE);
			}
		}
		NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);
		Repaint(DMData);
	}
}

void CPsycleWTLView::PasteBlock(int tx,int lx,bool mix)
{
	// UNDO CODE PASTE AND MIX PASTE
	if(isBlockCopied)
	{
		int ps=m_pSong->PlayOrder(editPosition);
		const int nl = m_pSong->PatternLines(ps);

		AddUndo(ps,tx,lx,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);

		int ls=0;
		int ts=0;
		
		for (int t=tx;t<tx+blockNTracks;t++)
		{
			ls=0;
			for (int l=lx;l<lx+blockNLines;l++)
			{
				if(l<nl && t<m_pSong->SongTracks())
				{
					unsigned char* offset_source=m_BlockBufferData+(ts*EVENT_SIZE+ls*MULTIPLY);
					unsigned char* offset_target=_ptrackline(ps,t,l);
					if ( mix )
					{
						if (*offset_target == 0xFF) *(offset_target)=*offset_source;
						if (*(offset_target+1)== 0xFF) *(offset_target+1)=*(offset_source+1);
						if (*(offset_target+2)== 0xFF) *(offset_target+2)=*(offset_source+2);
						if (*(offset_target+3)== 0) *(offset_target+3)=*(offset_source+3);
						if (*(offset_target+4)== 0) *(offset_target+4)=*(offset_source+4);
						if (*(offset_target+5)== 0) *(offset_target+5)=*(offset_source+5);
						if (*(offset_target+6)== 0) *(offset_target+6)=*(offset_source+6);

					}
					else
					{
						memcpy(offset_target,offset_source,EVENT_SIZE);
					}
				}
				++ls;
			}
			++ts;
		}
		
		if (lx+blockNLines < nl ) editcur.line = lx+blockNLines;
		else editcur.line = nl-1;

		bScrollDetatch=false;
		NewPatternDraw(tx,tx+blockNTracks-1,lx,lx+blockNLines-1);
		Repaint(DMData);
	}
	
}

void CPsycleWTLView::SaveBlock(FILE* file)
{

	int ps = _ps();
	int nlines = m_pSong->PatternLines(ps);
	int _songTracks = m_pSong->SongTracks();
	fwrite(&_songTracks, sizeof(int), 1, file);
	fwrite(&nlines, sizeof(int), 1, file);

	for (int t=0;t<m_pSong->SongTracks();t++)
	{
		for (int l=0;l<nlines;l++)
		{
			unsigned char* offset_source=_ptrackline(ps,t,l);
			
			fwrite(offset_source,sizeof(char),EVENT_SIZE,file);
		}
	}
}

void CPsycleWTLView::LoadBlock(FILE* file)
{
	int nt, nl;
	fread(&nt,sizeof(int),1,file);
	fread(&nl,sizeof(int),1,file);

	if ((nt > 0) && (nl > 0))
	{

		int ps = _ps();
		int nlines = m_pSong->PatternLines(ps);
		AddUndo(ps,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
		if (nlines != nl)
		{
			AddUndoLength(ps,nlines,editcur.track,editcur.line,editcur.col,editPosition);
			m_pSong->PatternLines(ps,nl);
		}

		for (int t=0;t<nt;t++)
		{
			for (int l=0;l<nl;l++)
			{
				if(l<MAX_LINES && t<MAX_TRACKS)
				{
					unsigned char* offset_target=_ptrackline(ps,t,l);
					fread(offset_target,sizeof(char),EVENT_SIZE,file);
				}
			}
		}
		//unsigned char blank[5]={255,255,255,0,0};

		for (t = nt; t < MAX_TRACKS;t++)
		{
			for (int l = nl; l < MAX_LINES; l++)
			{
				unsigned char* offset_target=_ptrackline(ps,t,l);
				memcpy(offset_target,BLANK_EVENT,EVENT_SIZE);
			}
		}
		Repaint(DMPattern);
	}
}

void CPsycleWTLView::DoubleLength()
{
	// UNDO CODE DOUBLE LENGTH
	unsigned char *toffset;
	//unsigned char blank[5]={255,255,255,0,0};
	int st, et, sl, el,nl;

	int ps = _ps();
	if ( blockSelected )
	{
///////////////////////////////////////////////////////// Add ROW
		st=m_BlockSelection.start.track;		
		et=m_BlockSelection.end.track+1;
		sl=m_BlockSelection.start.line;			
		nl=((m_BlockSelection.end.line-sl)/2)+1;
		el=m_BlockSelection.end.line;
		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,nl*2,editcur.track,editcur.line,editcur.col,editPosition);
	}
	else 
	{
		st = 0;		
		et = m_pSong->SongTracks();		
		sl = 0;
		nl = m_pSong->PatternLines(ps) / 2;	
		el = m_pSong->PatternLines(ps) - 1;
		AddUndo(ps,0,0,MAX_TRACKS,el+1,editcur.track,editcur.line,editcur.col,editPosition);
	}

	for (int t=st;t<et;t++)
	{
		toffset=_ptrack(ps,t);
		memcpy(toffset+el*MULTIPLY,BLANK_EVENT,EVENT_SIZE);
		for (int l=nl-1;l>0;l--)
		{
			memcpy(toffset+(sl+l*2)*MULTIPLY,toffset+(sl+l)*MULTIPLY,EVENT_SIZE);
			memcpy(toffset+(sl+(l*2)-1)*MULTIPLY,BLANK_EVENT,EVENT_SIZE);
		}
	}

	NewPatternDraw(st,et,sl,el);
	Repaint(DMData);
}

void CPsycleWTLView::HalveLength()
{
	// UNDO CODE HALF LENGTH
	unsigned char *toffset;
	int st, et, sl, el,nl;
	int ps = _ps();
	//unsigned char blank[5]={255,255,255,0,0};

	if ( blockSelected )
	{
///////////////////////////////////////////////////////// Add ROW
		st=m_BlockSelection.start.track;	
		et=m_BlockSelection.end.track+1;
		sl=m_BlockSelection.start.line;		
		nl=m_BlockSelection.end.line-sl+1;
		el=nl/2;
		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,nl,editcur.track,editcur.line,editcur.col,editPosition);
	}
	else 
	{
		st=0;	
		et=m_pSong->SongTracks();		
		sl=0;
		nl=m_pSong->PatternLines(ps);	
		el=m_pSong->PatternLines(ps) / 2;
		AddUndo(ps,0,0,MAX_TRACKS,nl,editcur.track,editcur.line,editcur.col,editPosition);
	}
	
	for (int t=st;t<et;t++)
	{
		toffset=_ptrack(ps,t);
		for (int l=1;l<el;l++)
		{
			memcpy(toffset+(l+sl)*MULTIPLY,toffset+((l*2)+sl)*MULTIPLY,EVENT_SIZE);
		}
		while (l < nl)
		{
			memcpy(toffset+((l+sl)*MULTIPLY),BLANK_EVENT,EVENT_SIZE);
			l++;
		}
	}

	NewPatternDraw(st,et,sl,nl+sl);
	Repaint(DMData);
}


void CPsycleWTLView::BlockTranspose(int trp)
{
	// UNDO CODE TRANSPOSE
	if ( blockSelected == true ) 
	{
		int ps = _ps();

		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,m_BlockSelection.end.line-m_BlockSelection.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

		for (int t=m_BlockSelection.start.track;t<m_BlockSelection.end.track+1;t++)
		{
			for (int l=m_BlockSelection.start.line;l<m_BlockSelection.end.line+1;l++)
			{
				unsigned char *toffset=_ptrackline(ps,t,l);
				
				int not=*(toffset);
			
				if(not<120)
				{
					not+=trp;
					if(not<0)not=0;
					if(not>119)not=119;
					*toffset=static_cast<unsigned char>(not);
				}
			}
		}
		NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);
		Repaint(DMData);
	}
}


void CPsycleWTLView::BlockGenChange(int x)
{
	// UNDO CODE BLOCK GENERATOR CHANGE
	if ( blockSelected == true ) 
	{
		int ps = _ps();
		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,m_BlockSelection.end.line-m_BlockSelection.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

		for (int t=m_BlockSelection.start.track;t<m_BlockSelection.end.track+1;t++)
		{
			for (int l=m_BlockSelection.start.line;l<m_BlockSelection.end.line+1;l++)
			{
				unsigned char *toffset=_ptrackline(ps,t,l)+2;
				
				unsigned char gen=*(toffset);
				
				if ( gen != 255 )
				{
					gen=x;
					if(gen<0)gen=0;
					if(gen>=MAX_MACHINES)gen=MAX_MACHINES-1;
					*toffset=gen;
				}
			}
		}
		NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);
		Repaint(DMData);
	}
}

void CPsycleWTLView::BlockInsChange(int x)
{
	// UNDO CODE BLOCK INS CHANGE
	if ( blockSelected == true ) 
	{
		const int ps=_ps();

		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,m_BlockSelection.end.line-m_BlockSelection.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

		for (int t=m_BlockSelection.start.track;t<m_BlockSelection.end.track+1;t++)
		{
			for (int l=m_BlockSelection.start.line;l<m_BlockSelection.end.line+1;l++)
			{
				unsigned char *toffset=_ptrackline(ps,t,l)+1;
				unsigned char ins=*(toffset);
			
				if (ins != 255 )
				{
					ins=x;
					if(ins<0)ins=0;
					if(ins>255)ins=255;
					*toffset=ins;
				}
			}
		}
		NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);
		Repaint(DMData);
	}
}

void CPsycleWTLView::BlockParamInterpolate()
{
	// UNDO CODE BLOCK INTERPOLATE
	if (blockSelected)
	{
		const int ps = _ps();
		///////////////////////////////////////////////////////// Add ROW
		unsigned char *toffset=_ppattern(ps);
		
		AddUndo(ps,m_BlockSelection.start.track,m_BlockSelection.start.line,m_BlockSelection.end.track-m_BlockSelection.start.track+1,m_BlockSelection.end.line-m_BlockSelection.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);
		
		const int initvalue = 
			*(toffset+m_BlockSelection.start.track*EVENT_SIZE+m_BlockSelection.start.line*MULTIPLY+3) * 0x100 +
			*(toffset+m_BlockSelection.start.track*EVENT_SIZE+m_BlockSelection.start.line*MULTIPLY+4);
		const int endvalue =
			*(toffset+m_BlockSelection.start.track*EVENT_SIZE+m_BlockSelection.end.line*MULTIPLY+3) * 0x100 +
			*(toffset+m_BlockSelection.start.track*EVENT_SIZE+m_BlockSelection.end.line*MULTIPLY+4);
		const float addvalue = (float)(endvalue -initvalue)/(m_BlockSelection.end.line-m_BlockSelection.start.line);
		const unsigned char comd = *(toffset+m_BlockSelection.start.track * EVENT_SIZE + m_BlockSelection.start.line*MULTIPLY+3);
		int displace2=(m_BlockSelection.start.track*EVENT_SIZE)+((m_BlockSelection.start.line+1)*MULTIPLY);
		
		if ( toffset[displace2] == cdefTweakM || toffset[displace2] == cdefTweakE || toffset[displace2] == cdefTweakS)
		{
			unsigned char note = toffset[displace2];
			unsigned char aux = toffset[displace2+1];
			unsigned char mac = toffset[displace2+2];
			unsigned char vel = toffset[displace2+6];
			unsigned char volcmd = toffset[displace2+5];


			for (int l=m_BlockSelection.start.line+1;l<m_BlockSelection.end.line;l++)
			{
				toffset[displace2]=note;
				toffset[displace2+1]=aux;
				toffset[displace2+2]=mac;
				int val=f2i(initvalue+addvalue*(l-m_BlockSelection.start.line));
				toffset[displace2+3]=static_cast<unsigned char>(val/0x100);
				toffset[displace2+4]=static_cast<unsigned char>(val%0x100);
				toffset[displace2+5]=volcmd;
				toffset[displace2+6]=vel;
				displace2+=MULTIPLY;
			}
			toffset[displace2]=note;
			toffset[displace2+1]=aux;
			toffset[displace2+2]=mac;
			toffset[displace2+6]=vel;
			toffset[displace2+5]=volcmd;
		}
		else
		{
			for (int l=m_BlockSelection.start.line+1;l<m_BlockSelection.end.line;l++)
			{
				int val=f2i(initvalue+addvalue*(l-m_BlockSelection.start.line));
				toffset[displace2+3]=static_cast<unsigned char>(val/0x100);
				toffset[displace2+4]=static_cast<unsigned char>(val%0x100);
				displace2+=MULTIPLY;
			}
		}
		NewPatternDraw(m_BlockSelection.start.track,m_BlockSelection.end.track,m_BlockSelection.start.line,m_BlockSelection.end.line);
		Repaint(DMData);
	}
}


void CPsycleWTLView::IncCurPattern()
{
	if(m_pSong->PlayOrder(editPosition) < (MAX_PATTERNS-1))
	{
		AddUndoSequence(m_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition);
		m_pSong->PlayOrder(m_pSong->PlayOrder(editPosition) + 1);
		m_pMainFrame->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}


void CPsycleWTLView::DecCurPattern()
{
	if(m_pSong->PlayOrder(editPosition) > 0)
	{
		AddUndoSequence(m_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition);
		m_pSong->PlayOrder(editPosition,m_pSong->PlayOrder(editPosition) - 1);
		m_pMainFrame->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}

void CPsycleWTLView::DecPosition()
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
			Global::pPlayer->Start(m_pSong->PlayLength() - 1,0);
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
//			editPosition = m_pSong->playLength-1;
			editPosition = 0;
		}
		
		for(int i = 0;i < MAX_SONG_POSITIONS;i++)
		{
			m_pSong->PlayOrderSel(editPosition,false);
		}
		//memset(m_pSong->PlayOrderSel(),0,MAX_SONG_POSITIONS*sizeof(bool));
		
		m_pSong->PlayOrderSel(editPosition,true);

		m_pMainFrame->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}

void CPsycleWTLView::IncPosition(bool bRepeat)
{
//	case cdefPlaySkipAhead:
	if (Global::pPlayer->_playing && Global::pConfig->_followSong)
	{
		if (Global::pPlayer->_playPosition < m_pSong->PlayLength() - 1)
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
		if(editPosition < m_pSong->PlayLength() - 1)
		{
			++editPosition;
		}
		else if (!bRepeat) // This prevents adding patterns when only trying to reach the end.
		{
//			editPosition = 0;
			++editPosition;
			AddUndoSequence(m_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition-1);
			int const ep=m_pSong->GetBlankPatternUnused();
			m_pSong->PlayLength(editPosition + 1);
			m_pSong->PlayOrder(editPosition,ep);
			if (m_pSong->PatternLines(editPosition) != Global::pConfig->defaultPatLines)
			{
				AddUndoLength(editPosition,m_pSong->PatternLines(editPosition),editcur.track,editcur.line,editcur.col,editPosition);
				m_pSong->PatternLines(editPosition,Global::pConfig->defaultPatLines);
			}
			
			m_pMainFrame->UpdateSequencer();
		}
		
		for(int i = 0;i < MAX_SONG_POSITIONS;i++)
		{
			m_pSong->PlayOrderSel(i,0);
		}
		//memset(m_pSong->m_PlayOrderSel,0,MAX_SONG_POSITIONS * sizeof(bool));
		m_pSong->PlayOrderSel(editPosition,true);

		m_pMainFrame->UpdatePlayOrder(true);
		Repaint(DMPattern);
	}
}

void CPsycleWTLView::SelectMachineUnderCursor()
{
	unsigned char *toffset=_ptrackline();

	PatternEntry *entry = (PatternEntry*) toffset;

	if ( entry->_mach < MAX_BUSES*2 ) m_pSong->SeqBus(entry->_mach);
	m_pMainFrame->ChangeGen(m_pSong->SeqBus());
	if ( entry->_inst != 255 ) m_pSong->AuxcolSelected(entry->_inst);
	m_pMainFrame->ChangeIns(m_pSong->AuxcolSelected());

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// undo/redo code
////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPsycleWTLView::AddMacViewUndo()
{
	// i have not written the undo code yet for machine and instruments
	// however, for now it at least tracks changes for save/new/open/close warnings
	UndoMacCounter++;
	SetTitleBarText();
}

void CPsycleWTLView::AddUndo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pUndoList;
	pUndoList = pNew;

	// fill data
	unsigned char* pData = new unsigned char[tracks*lines*EVENT_SIZE*sizeof(char)];
	pNew->pData = pData;
	pNew->pattern = pattern;
	pNew->x = x;
	pNew->y = y;
	if (tracks+x > m_pSong->SongTracks())
	{
		tracks = m_pSong->SongTracks() - x;
	}
	pNew->tracks = tracks;
	const int nl = m_pSong->PatternLines(pattern);
	if (lines+y > nl)
	{
		lines = nl-y;
	}
	pNew->lines = lines;
	pNew->type = UNDO_PATTERN;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;

	for (int t=x;t<x+tracks;t++)
	{
		for (int l=y;l<y+lines;l++)
		{
			unsigned char *offset_source=_ptrackline(pattern,t,l);
			
			memcpy(pData,offset_source,EVENT_SIZE);
			pData+=EVENT_SIZE;
		}
	}
	if (bWipeRedo)
	{
		KillRedo();
		m_UndoCounter++;
		pNew->counter = m_UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void CPsycleWTLView::AddRedo(int pattern, int x, int y, int tracks, int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pRedoList;
	pRedoList = pNew;
	// fill data
	unsigned char* pData = new unsigned char[tracks*lines*EVENT_SIZE*sizeof(char)];
	pNew->pData = pData;
	pNew->pattern = pattern;
	pNew->x = x;
	pNew->y = y;
	if (tracks+x > m_pSong->SongTracks())
	{
		tracks = m_pSong->SongTracks() - x;
	}
	pNew->tracks = tracks;
	const int nl = m_pSong->PatternLines(pattern);
	if (lines+y > nl)
	{
		lines = nl-y;
	}
	pNew->tracks = tracks;
	pNew->lines = lines;
	pNew->type = UNDO_PATTERN;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;
	pNew->counter = counter;

	for (int t=x;t<x+tracks;t++)
	{
		for (int l=y;l<y+lines;l++)
		{
			unsigned char *offset_source=_ptrackline(pattern,t,l);
			
			memcpy(pData,offset_source,EVENT_SIZE);
			pData+=EVENT_SIZE;
		}
	}
}

void CPsycleWTLView::AddUndoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
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
		m_UndoCounter++;
		pNew->counter = m_UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void CPsycleWTLView::AddRedoLength(int pattern, int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
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

void CPsycleWTLView::AddUndoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pUndoList;
	pUndoList = pNew;
	// fill data
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS];
	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		*(pNew->pData + i)  = m_pSong->PlayOrder(i);
	}
	//memcpy(pNew->pData, m_pSong->m_PlayOrder, MAX_SONG_POSITIONS*sizeof(char));
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
		m_UndoCounter++;
		pNew->counter = m_UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void CPsycleWTLView::AddUndoSong(int edittrack, int editline, int editcol, int seqpos, BOOL bWipeRedo, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pUndoList;
	pUndoList = pNew;
	// fill data
	// count used patterns
	unsigned char count = 0;
	for (unsigned char i = 0; i < MAX_PATTERNS; i++)
	{
		if (m_pSong->pPatternData(i))
		{
			count++;
		}
	}
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS+sizeof(count)+MAX_PATTERNS+count*MULTIPLY2];
	unsigned char *pWrite=pNew->pData;
	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		*(pWrite + i) = m_pSong->PlayOrder(i);
	}
	//memcpy(pWrite, m_pSong->m_PlayOrder, MAX_SONG_POSITIONS*sizeof(char));
	pWrite += MAX_SONG_POSITIONS * sizeof(char);

	memcpy(pWrite, &count, sizeof(count));
	
	pWrite += sizeof(count);

	for (i = 0; i < MAX_PATTERNS; i++)
	{
		if (m_pSong->pPatternData(i))
		{
			memcpy(pWrite, &i, sizeof(i));
			pWrite += sizeof(i);
			
			memcpy(pWrite, m_pSong->pPatternData(i), MULTIPLY2);
			pWrite += MULTIPLY2;
		}
	}

	pNew->pattern = 0;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = m_pSong->PlayLength();
	pNew->type = UNDO_SONG;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;

	if (bWipeRedo)
	{
		KillRedo();
		m_UndoCounter++;
		pNew->counter = m_UndoCounter;
	}
	else
	{
		pNew->counter = counter;
	}
	SetTitleBarText();
}

void CPsycleWTLView::AddRedoSong(int edittrack, int editline, int editcol, int seqpos, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pRedoList;
	pRedoList = pNew;
	// fill data
	// count used patterns
	unsigned char count = 0;
	for (unsigned char i = 0; i < MAX_PATTERNS; i++)
	{
		if (m_pSong->pPatternData(i))
		{
			count++;
		}
	}
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS+sizeof(count)+MAX_PATTERNS+count*MULTIPLY2];
	unsigned char *pWrite=pNew->pData;
	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		*(pWrite + i) = m_pSong->PlayOrder(i);
	}
	//memcpy(pWrite, m_pSong->m_PlayOrder, MAX_SONG_POSITIONS*sizeof(char));
	pWrite+=MAX_SONG_POSITIONS*sizeof(char);

	memcpy(pWrite, &count, sizeof(count));
	pWrite+=sizeof(count);

	for (i = 0; i < MAX_PATTERNS; i++)
	{
		if (m_pSong->pPatternData(i))
		{
			memcpy(pWrite, &i, sizeof(i));
			pWrite+=sizeof(i);
			memcpy(pWrite, m_pSong->pPatternData(i), MULTIPLY2);
			pWrite+=MULTIPLY2;
		}
	}

	pNew->pattern = 0;
	pNew->x = 0;
	pNew->y = 0;
	pNew->tracks = 0;
	pNew->lines = m_pSong->PlayLength();
	pNew->type = UNDO_SONG;
	pNew->edittrack = edittrack;
	pNew->editline = editline;
	pNew->editcol = editcol;
	pNew->seqpos = seqpos;
	pNew->counter = counter;
}

void CPsycleWTLView::AddRedoSequence(int lines, int edittrack, int editline, int editcol, int seqpos, int counter)
{
	SPatternUndo* pNew = new SPatternUndo;
	pNew->pPrev = pRedoList;
	pRedoList = pNew;
	// fill data
	pNew->pData = new unsigned char[MAX_SONG_POSITIONS];

	//memcpy(pNew->pData, m_pSong->m_PlayOrder, MAX_SONG_POSITIONS*sizeof(char));
	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		*(pNew->pData + i) = m_pSong->PlayOrder(i);
	}
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

void CPsycleWTLView::OnEditUndo() 
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
				unsigned char* pData = pUndoList->pData;

				for (int t=pUndoList->x;t<pUndoList->x+pUndoList->tracks;t++)
				{
					for (int l=pUndoList->y;l<pUndoList->y+pUndoList->lines;l++)
					{
						unsigned char *offset_source=_ptrackline(pUndoList->pattern,t,l);
						
						memcpy(offset_source,pData,EVENT_SIZE*sizeof(char));
						pData+=EVENT_SIZE*sizeof(char);
					}
				}
				// set up cursor
				editcur.track = pUndoList->edittrack;
				editcur.line = pUndoList->editline;
				editcur.col = pUndoList->editcol;
				if (pUndoList->seqpos == editPosition)
				{
					// display changes
					NewPatternDraw(pUndoList->x,pUndoList->x+pUndoList->tracks,pUndoList->y,pUndoList->y+pUndoList->lines);
					Repaint(DMData);
				}
				else
				{
					editPosition = pUndoList->seqpos;
					m_pMainFrame->UpdatePlayOrder(true);
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
				AddRedoLength(pUndoList->pattern,m_pSong->PatternLines(pUndoList->pattern),editcur.track,editcur.line,editcur.col,pUndoList->seqpos,pUndoList->counter);
				// do undo
				m_pSong->PatternLines(pUndoList->pattern,pUndoList->lines);
				// set up cursor
				editcur.track = pUndoList->edittrack;
				editcur.line = pUndoList->editline;
				editcur.col = pUndoList->editcol;
				if (pUndoList->seqpos != editPosition)
				{
					editPosition = pUndoList->seqpos;
					m_pMainFrame->UpdatePlayOrder(true);
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
			{
				AddRedoSequence(m_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition,pUndoList->counter);
				// do undo
				for(int i = 0;i <  MAX_SONG_POSITIONS;i++){
					m_pSong->PlayOrder(i,*(pUndoList->pData + i));
				}
				//memcpy(m_pSong->PlayOrder(), pUndoList->pData, MAX_SONG_POSITIONS * sizeof(char));
				m_pSong->PlayLength(pUndoList->lines);
				// set up cursor
				editcur.track = pUndoList->edittrack;
				editcur.line = pUndoList->editline;
				editcur.col = pUndoList->editcol;
				editPosition = pUndoList->seqpos;
				m_pMainFrame->UpdatePlayOrder(true);
				m_pMainFrame->UpdateSequencer();
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
		case UNDO_SONG:
			{
				AddRedoSong(editcur.track,editcur.line,editcur.col,editPosition,pUndoList->counter);
				// do undo
				unsigned char * pData = pUndoList->pData;
				for(int i = 0;i < MAX_SONG_POSITIONS;i++){
					m_pSong->PlayOrder(i,*(pData + 1));
				}
				//memcpy(m_pSong->m_PlayOrder, pData, MAX_SONG_POSITIONS*sizeof(char));
				pData += MAX_SONG_POSITIONS;
				unsigned char count = *pData;
				pData += sizeof(count);
				for (int i = 0; i < count; i++)
				{
					unsigned char index = *pData;
					pData += sizeof(index);
					unsigned char* pWrite = _ppattern(index);

					memcpy(pWrite,pData,MULTIPLY2);
					pData+= MULTIPLY2;
				}
				m_pSong->PlayLength(pUndoList->lines);
				// set up cursor
				editcur.track = pUndoList->edittrack;
				editcur.line = pUndoList->editline;
				editcur.col = pUndoList->editcol;
				editPosition = pUndoList->seqpos;
				m_pMainFrame->UpdatePlayOrder(true);
				m_pMainFrame->UpdateSequencer();
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

		}
		SetTitleBarText();
	}
}


void CPsycleWTLView::OnEditRedo() 
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
				unsigned char* pData = pRedoList->pData;

				for (int t=pRedoList->x;t<pRedoList->x+pRedoList->tracks;t++)
				{
					for (int l=pRedoList->y;l<pRedoList->y+pRedoList->lines;l++)
					{
						unsigned char *offset_source=_ptrackline(pRedoList->pattern,t,l);

						
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
					NewPatternDraw(pRedoList->x,pRedoList->x+pRedoList->tracks,pRedoList->y,pRedoList->y+pRedoList->lines);
					Repaint(DMData);
				}
				else
				{
					editPosition = pRedoList->seqpos;
					m_pMainFrame->UpdatePlayOrder(true);
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
				AddUndoLength(pRedoList->pattern,m_pSong->PatternLines(pUndoList->pattern),editcur.track,editcur.line,editcur.col,pRedoList->seqpos,FALSE,pRedoList->counter);
				// do undo
				m_pSong->PatternLines(pRedoList->pattern,pRedoList->lines);
				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				if (pRedoList->seqpos != editPosition)
				{
					editPosition = pRedoList->seqpos;
					m_pMainFrame->UpdatePlayOrder(true);
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
			{
				AddUndoSequence(m_pSong->PlayLength(),editcur.track,editcur.line,editcur.col,editPosition,FALSE,pRedoList->counter);
				// do undo
				for(int i = 0;i < MAX_SONG_POSITIONS;i++){
					m_pSong->PlayOrder(i,*(pRedoList->pData + i));
				}
				//memcpy(m_pSong->m_PlayOrder, pRedoList->pData, MAX_SONG_POSITIONS*sizeof(char));
				m_pSong->PlayLength(pRedoList->lines);
				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				editPosition = pRedoList->seqpos;
				m_pMainFrame->UpdatePlayOrder(true);
				m_pMainFrame->UpdateSequencer();
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
		case UNDO_SONG:
			{
				AddUndoSong(editcur.track,editcur.line,editcur.col,editPosition,FALSE,pRedoList->counter);
				// do undo
				unsigned char * pData = pRedoList->pData;
				for(int i = 0;i < MAX_SONG_POSITIONS;i++){
					m_pSong->PlayOrder(i,*(pData + i));
				}

				//memcpy(m_pSong->m_PlayOrder, pData, MAX_SONG_POSITIONS*sizeof(char));
				pData += MAX_SONG_POSITIONS;
				unsigned char count = *pData;
				pData += sizeof(count);
				for (int i = 0; i < count; i++)
				{
					unsigned char index = *pData;
					pData += sizeof(index);
					unsigned char* pWrite = _ppattern(index);

					memcpy(pWrite,pData,MULTIPLY2);
					pData+= MULTIPLY2;
				}

				// set up cursor
				editcur.track = pRedoList->edittrack;
				editcur.line = pRedoList->editline;
				editcur.col = pRedoList->editcol;
				editPosition = pRedoList->seqpos;
				m_pMainFrame->UpdatePlayOrder(true);
				m_pMainFrame->UpdateSequencer();
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
		}
		SetTitleBarText();
	}
}

void CPsycleWTLView::KillRedo()
{
	while (pRedoList)
	{
		SPatternUndo* pTemp = pRedoList->pPrev;
		delete (pRedoList->pData);
		delete (pRedoList);
		pRedoList = pTemp;
	}
}

void CPsycleWTLView::KillUndo()
{
	while (pUndoList)
	{
		SPatternUndo* pTemp = pUndoList->pPrev;
		delete (pUndoList->pData);
		delete (pUndoList);
		pUndoList = pTemp;
	}
	m_UndoCounter = 0;
	m_UndoSaved = 0;

	UndoMacCounter=0;
	UndoMacSaved=0;

//	SetTitleBarText();
}


void CPsycleWTLView::SelectNextTrack()
{
	if(++editcur.track >= m_pSong->SongTracks())
		editcur.track=0;
	while(m_pSong->IsTrackArmed(editcur.track) == 0)
	{
		if(++editcur.track >= m_pSong->SongTracks())
			editcur.track=0;
	}
	editcur.col = 0;
}
