///\file
///\brief keyboard handler for psycle::host::CChildView, private header
#pragma once
#include <psycle/host/detail/project.hpp>
namespace psycle { namespace host {		
		
		void CChildView::KeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
		{
			// undo code not required, enter note handles it
			CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);	
			if (cmd.GetType() == CT_Note)
			{
				const int outnote = cmd.GetNote();
				if(viewMode == view_modes::pattern && bEditMode && Global::pPlayer->_playing && Global::psycleconf()._followSong && Global::psycleconf().inputHandler()._RecordNoteoff)
				{ 
					EnterNote(outnote,255,0,true);	// note end
				}
				else
				{
					Global::pInputHandler->StopNote(outnote);
				}
			}
			else if ((nChar == 16) && ((nFlags & 0xC000) == 0xC000) && ChordModeOffs)
			{
				// shift is up, abort chord mode
				editcur.line = ChordModeLine;
				editcur.track = ChordModeTrack;
				ChordModeOffs = 0;
				AdvanceLine(patStep,Global::psycleconf().inputHandler()._wrapAround,true);
		//		pParentMain->StatusBarIdle();
		//		Repaint(draw_modes::cursor);
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
			// undo code not required, enternote and msbput handle it
			BOOL bRepeat = nFlags&0x4000;

			if(viewMode == view_modes::pattern && bEditMode)
			{
				if (!(Global::pPlayer->_playing && Global::psycleconf()._followSong && bRepeat))
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
			/*else if (viewMode == view_modes::sequence && bEditMode)
			{
				bool success;
				// add data
				success = Global::pInputHandler->EnterDataSeq(nChar,nFlags);
				if ( success )
				{
					CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
					return;
				}
			}*/
			else
			{
				ChordModeOffs = 0;
			}

			// get command
			CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);

			if(cmd.IsValid())
			{
				if((cmd.GetType() == CT_Immediate) ||
				(cmd.GetType() == CT_Editor && viewMode == view_modes::pattern) ) 
				{			
					Global::pInputHandler->PerformCmd(cmd,bRepeat);
				}
				else if (cmd.GetType() == CT_Note && viewMode != view_modes::sequence)
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
		// PARAMETERS     : int outnote - note to insert/stop . int velocity - velocity of the note, or zero if noteoff
		// RETURNS		  : <void>
		// 

		void CChildView::MidiPatternNote(int outnote,int macidx, int instidx, int velocity)
		{
			Machine* mac = NULL;
			if(macidx >=0 && macidx <MAX_BUSES)
			{
				mac = Global::_pSong->_pMachine[macidx];
			}
			// undo code not required, enter note handles it
			if(viewMode == view_modes::pattern && bEditMode)
			{ 
				// add note
				if(velocity > 0 || 
					(Global::psycleconf().inputHandler()._RecordNoteoff && Global::pPlayer->_playing && Global::psycleconf()._followSong))
				{
					EnterNote(outnote,/*macidx,*/ instidx,velocity,false);
				}
				else
				{
					Global::pInputHandler->StopNote(outnote,instidx,false, mac);	// note end
				}			
			}
			else 
			{
				// play note
				if(velocity>0)
					Global::pInputHandler->PlayNote(outnote,instidx,velocity,false,mac);
				else
					Global::pInputHandler->StopNote(outnote,instidx,false,mac);
			}
		}

		void CChildView::MidiPatternTweak(int command, int value)
		{
			// UNDO CODE MIDI PATTERN TWEAK
			if (value < 0) value = 0;
			if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

			// build entry
			PatternEntry entry;
			entry._mach = _pSong->seqBus;
			entry._cmd = (value>>8)&255;
			entry._parameter = value&255;
			entry._inst = command;
			entry._note = notecommands::tweak;

			if(viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
				const int ps = _ps();
				int line = Global::pPlayer->_lineCounter;
				unsigned char * toffset; 

				if (Global::pPlayer->_playing&&Global::psycleconf()._followSong)
				{
					if(_pSong->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::psycleconf().inputHandler()._RecordUnarmed)
					{		
						// play it
						Machine* pMachine = _pSong->_pMachine[_pSong->seqBus];

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
				if (pentry->_note >= notecommands::release)
				{
					if ((pentry->_mach != entry._mach) 
						|| (pentry->_cmd != entry._cmd)
						|| (pentry->_parameter != entry._parameter) 
						|| (pentry->_inst != entry._inst) 
						|| ((pentry->_note != notecommands::tweak) && (pentry->_note != notecommands::tweakeffect) && (pentry->_note != notecommands::tweakslide)))
					{
						Global::pInputHandler->AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
						pentry->_mach = entry._mach;
						pentry->_cmd = entry._cmd;
						pentry->_parameter = entry._parameter;
						pentry->_inst = entry._inst;
						pentry->_note = entry._note;

						NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
						Repaint(draw_modes::data);
					}
				}
			}
		//	else
			{
				// play it
				Machine* pMachine = _pSong->_pMachine[_pSong->seqBus];

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
			}
		}

		void CChildView::MidiPatternTweakSlide(int command, int value)
		{
			// UNDO CODE MIDI PATTERN TWEAK
			if (value < 0) value = 0;
			if (value > 0xffff) value = 0xffff;// no else incase of neg overflow

			// build entry
			PatternEntry entry;
			entry._mach = _pSong->seqBus;
			entry._cmd = (value>>8)&255;
			entry._parameter = value&255;
			entry._inst = command;
			entry._note = notecommands::tweakslide;

			if(viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
				const int ps = _ps();
				int line = Global::pPlayer->_lineCounter;
				unsigned char * toffset; 

				if (Global::pPlayer->_playing&&Global::psycleconf()._followSong)
				{
					if(_pSong->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::psycleconf().inputHandler()._RecordUnarmed)
					{		
						Machine* pMachine = _pSong->_pMachine[_pSong->seqBus];

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
				if (pentry->_note >= notecommands::release)
				{
					if ((pentry->_mach != entry._mach) 
						|| (pentry->_cmd != entry._cmd)
						|| (pentry->_parameter != entry._parameter) 
						|| (pentry->_inst != entry._inst) 
						|| ((pentry->_note != notecommands::tweak) && (pentry->_note != notecommands::tweakeffect) && (pentry->_note != notecommands::tweakslide)))
					{
						Global::pInputHandler->AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
						pentry->_mach = entry._mach;
						pentry->_cmd = entry._cmd;
						pentry->_parameter = entry._parameter;
						pentry->_inst = entry._inst;
						pentry->_note = entry._note;

						NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
						Repaint(draw_modes::data);
					}
				}
			}
		//	else
			{
				Machine* pMachine = _pSong->_pMachine[_pSong->seqBus];

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
			}
		}

		void CChildView::MidiPatternCommand(int command, int value)
		{
			// UNDO CODE MIDI PATTERN
			if (value < 0) value = 0;
			if (value > 0xffff) value = 0xffff;

			// build entry
			PatternEntry entry;
			entry._mach = _pSong->seqBus;
			entry._inst = _pSong->auxcolSelected;
			entry._cmd = command;
			entry._parameter = value;
			entry._note = notecommands::empty;

			if(viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
				const int ps = _ps();
				int line = Global::pPlayer->_lineCounter;
				unsigned char * toffset; 

				if (Global::pPlayer->_playing&&Global::psycleconf()._followSong)
				{
					if(_pSong->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::psycleconf().inputHandler()._RecordUnarmed)
					{		
						Machine* pMachine = _pSong->_pMachine[_pSong->seqBus];

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
					Global::pInputHandler->AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
					pentry->_mach = entry._mach;
					pentry->_cmd = entry._cmd;
					pentry->_parameter = entry._parameter;
					pentry->_inst = entry._inst;

					NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
					Repaint(draw_modes::data);
				}
			}
		//	else
			{
				// play it
				Machine* pMachine = _pSong->_pMachine[_pSong->seqBus];

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
			}
		}

		void CChildView::MidiPatternMidiCommand(int command, int value)
		{
			// UNDO CODE MIDI PATTERN TWEAK
			PatternEntry entry;
			entry._mach = _pSong->seqBus;
			entry._cmd = (value&0xFF00)>>8;
			entry._parameter = value&0xFF;
			entry._inst = command;
			entry._note = notecommands::midicc;

			if(viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
				const int ps = _ps();
				int line = Global::pPlayer->_lineCounter;
				unsigned char * toffset; 

				if (Global::pPlayer->_playing&&Global::psycleconf()._followSong)
				{
					if(_pSong->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::psycleconf().inputHandler()._RecordUnarmed)
					{		
						Machine* pMachine = _pSong->_pMachine[_pSong->seqBus];

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
				if (pentry->_note >= notecommands::release)
				{
					if ((pentry->_mach != entry._mach) 
						|| (pentry->_cmd != entry._cmd) 
						|| (pentry->_parameter != entry._parameter) 
						|| (pentry->_inst != entry._inst) 
						|| (pentry->_note != notecommands::midicc))
					{
						Global::pInputHandler->AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
						pentry->_mach = entry._mach;
						pentry->_cmd = entry._cmd;
						pentry->_parameter = entry._parameter;
						pentry->_inst = entry._inst;
						pentry->_note = entry._note;

						NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
						Repaint(draw_modes::data);
					}
				}
			}
		//	else
			{
				Machine* pMachine = _pSong->_pMachine[_pSong->seqBus];

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
			}
		}

		void CChildView::MidiPatternInstrument(int value)
		{
			if (value < 0) value = 0;
			if (value >= MAX_INSTRUMENTS) value = MAX_INSTRUMENTS-1;
			// build entry
			PatternEntry entry;
			entry._mach = _pSong->seqBus;
			entry._inst = value;
			entry._cmd = 255;
			entry._parameter = 255;
			entry._note = notecommands::empty;

			if(viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
				const int ps = _ps();
				int line = Global::pPlayer->_lineCounter;
				unsigned char * toffset; 

				if (Global::pPlayer->_playing&&Global::psycleconf()._followSong)
				{
					if(_pSong->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::psycleconf().inputHandler()._RecordUnarmed)
					{		
						Machine* pMachine = _pSong->_pMachine[_pSong->seqBus];

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
					Global::pInputHandler->AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
					pentry->_mach = entry._mach;
					pentry->_inst = entry._inst;

					NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
					Repaint(draw_modes::data);
				}
			}
		//	else
			{
				Machine* pMachine = _pSong->_pMachine[_pSong->seqBus];

				if (pMachine)
				{
					// play
					pMachine->Tick(editcur.track,&entry);
				}
			}
		}

		void CChildView::MousePatternTweak(int machine, int command, int value)
		{
			if (value < 0) value = 0;
			if (value > 0xffff) value = 0xffff;

			if(viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
				const int ps = _ps();
				int line = Global::pPlayer->_lineCounter;
				unsigned char * toffset;
				if (Global::pPlayer->_playing&&Global::psycleconf()._followSong)
				{
					if(_pSong->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::psycleconf().inputHandler()._RecordUnarmed)
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
				if (entry->_note >= notecommands::release)
				{
					if ((entry->_mach != machine) || (entry->_cmd != ((value>>8)&255)) || (entry->_parameter != (value&255)) || (entry->_inst != command) || ((entry->_note != notecommands::tweak) && (entry->_note != notecommands::tweakeffect) && (entry->_note != notecommands::tweakslide)))
					{
						Global::pInputHandler->AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
						entry->_mach = machine;
						entry->_cmd = (value>>8)&255;
						entry->_parameter = value&255;
						entry->_inst = command;
						entry->_note = notecommands::tweak;

						NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
						Repaint(draw_modes::data);
					}
				}
			}
		}

		void CChildView::MousePatternTweakSlide(int machine, int command, int value)
		{
			if (value < 0) value = 0;
			if (value > 0xffff) value = 0xffff;
			if(viewMode == view_modes::pattern && bEditMode)
			{ 
				// write effect
				const int ps = _ps();
				int line = Global::pPlayer->_lineCounter;
				unsigned char * toffset;
				if (Global::pPlayer->_playing&&Global::psycleconf()._followSong)
				{
					if(_pSong->_trackArmedCount)
					{
						SelectNextTrack();
					}
					else if (!Global::psycleconf().inputHandler()._RecordUnarmed)
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
				if (entry->_note >= notecommands::release)
				{
					if ((entry->_mach != machine) || (entry->_cmd != ((value>>8)&255)) || (entry->_parameter != (value&255)) || (entry->_inst != command) || ((entry->_note != notecommands::tweak) && (entry->_note != notecommands::tweakeffect) && (entry->_note != notecommands::tweakslide)))
					{
						Global::pInputHandler->AddUndo(ps,editcur.track,line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
						entry->_mach = machine;
						entry->_cmd = (value>>8)&255;
						entry->_parameter = value&255;
						entry->_inst = command;
						entry->_note = notecommands::tweakslide;

						NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
						Repaint(draw_modes::data);
					}
				}
			}
		}

		// note:  note to insert or stop. If it is notecommands::release it means that 
		//		the noteoff key has been pressed.
		// instr: value for aux column. if instr = 255 then use the auxcolselected.
		// velocity: volume to use. If velocity is zero, then do a noteoff.
		// bTranspose: transpose the note with the currentOctave.
		void CChildView::EnterNote(int note, int instr, int velocity, bool bTranspose)
		{
			int line;
			const int ps = _ps();
			unsigned char * toffset;

			int instNo;
			if (instr < 255) instNo = instr;
			else instNo = _pSong->auxcolSelected;
			
			if (note < 0 || note >= notecommands::invalid ) return;

			// octave offset
			if(note<notecommands::release)
			{
				if(bTranspose)
					note+=_pSong->currentOctave*12;

				if (note > 119) 
					note = 119;
			}
			
			// realtime note entering
			if (Global::pPlayer->_playing&&Global::psycleconf()._followSong)
			{
				// If there is at least one track selected for recording, select the proper track
				if(_pSong->_trackArmedCount)
				{
					if (velocity == 0)
					{
						int i;
						for (i = 0; i < _pSong->SONGTRACKS; i++)
						{
							if (_pSong->_trackArmed[i])
							{
								if (Global::pInputHandler->notetrack[i] == note && Global::pInputHandler->instrtrack[i] == instNo)
								{
									editcur.track = i;
									break;
								}
							}
						}
						/// if not found, stop and leave
						if (i == _pSong->SONGTRACKS)
						{
							Global::pInputHandler->StopNote(note,instNo,false);
							return;
						}
					}
					else
					{
						SelectNextTrack();
					}
				}
				//If the FT2 recording behaviour is not selected, do not record, but play the note.
				else if (!Global::psycleconf().inputHandler()._RecordUnarmed)
				{
					// build entry
					PatternEntry entry;
					entry._note = note;
					entry._mach = _pSong->seqBus;
					if (velocity==0)
						note = notecommands::release;

					if ( _pSong->seqBus < MAX_MACHINES && _pSong->_pMachine[_pSong->seqBus] != 0 ) 
					{
						// if the current machine is a sampler, check 
						// if current sample is locked to a machine.
						// if so, switch entry._mach to that machine number
						if (((Machine*)_pSong->_pMachine[_pSong->seqBus])->_type == MACH_SAMPLER)
						{
							if ((_pSong->_pInstrument[instNo]->_lock_instrument_to_machine != -1)
								&& (_pSong->_pInstrument[instNo]->_LOCKINST == true))
							{
								entry._mach = _pSong->_pInstrument[instNo]->_lock_instrument_to_machine;
							}
						}
					}

					if ( note < notecommands::release && Global::psycleconf().inputHandler()._RecordTweaks && velocity < 127)
					{
						if (Global::psycleconf().midi().raw())
						{
							entry._cmd = 0x0c;
							entry._parameter = velocity*2;
						}
						else if (Global::psycleconf().midi().velocity().record())
						{
							// command
							entry._cmd = Global::psycleconf().midi().velocity().command();
							int par = Global::psycleconf().midi().velocity().from() + (Global::psycleconf().midi().velocity().to() - Global::psycleconf().midi().velocity().from()) * velocity / 127;
							if (par > 255) 
							{
								par = 255;
							}
							else if (par < 0) 
							{
								par = 0;
							}
							entry._parameter = par;
						}
					}

					Machine *tmac = _pSong->_pMachine[entry._mach];
					if (tmac)
					{
						// if the current machine is a sampler, check 
						// if current sample is locked to a machine.
						// if so, switch entry._mach to that machine number
						if (tmac->_type == MACH_SAMPLER)
						{
							if ((_pSong->_pInstrument[instNo]->_lock_instrument_to_machine != -1)
								&& (_pSong->_pInstrument[instNo]->_LOCKINST == true))
							{
								entry._mach = _pSong->_pInstrument[instNo]->_lock_instrument_to_machine;
								tmac = _pSong->_pMachine[entry._mach];
								if (!tmac) return;
							}
						}
						if (note > notecommands::release || tmac->NeedsAuxColumn())
						{
							entry._inst = instNo;
						}

						if ( note <= notecommands::release)
						{
							tmac->Tick(editcur.track, &entry);
						}
					}
					Global::pInputHandler->notetrack[editcur.track]=note;
					Global::pInputHandler->instrtrack[editcur.track]=instNo;
					return;
				}
				line = Global::pPlayer->_lineCounter;
				toffset = _ptrack(ps)+(line*MULTIPLY);
				ChordModeOffs = 0;
			}
			else 
			{
				if ((GetKeyState(VK_SHIFT)<0) && (note != notecommands::tweak) && (note != notecommands::tweakeffect) && (note != notecommands::tweakslide) && (note != notecommands::midicc))
				{
					if (ChordModeOffs == 0)
					{
						ChordModeLine = editcur.line;
						ChordModeTrack = editcur.track;
					}
					editcur.track = (ChordModeTrack+ChordModeOffs)%_pSong->SONGTRACKS;
					editcur.line = line = ChordModeLine;
					toffset = _ptrackline(ps, editcur.track, line);
					ChordModeOffs++;
				}
				else
				{
					if (ChordModeOffs) // this should never happen because the shift check should catch it... but..
					{					// ok pooplog, now it REALLY shouldn't happen (now that the shift check works)
						editcur.line = ChordModeLine;
						editcur.track = ChordModeTrack;
						ChordModeOffs = 0;
						AdvanceLine(patStep,Global::psycleconf().inputHandler()._wrapAround,false);
					}
					line = editcur.line;
					toffset = _ptrackline(ps);
				}
			}

			// build the entry to add it to the pattern
			PatternEntry *entry = (PatternEntry*) toffset;
			if (velocity==0)
			{
				//This prevents writing a noteoff over its own noteon.
				if (entry->_note == note && entry->_inst == instNo)
				{
					Global::pInputHandler->StopNote(note,instNo,false);
					return;
				}
				note = notecommands::release;
			}
			Global::pInputHandler->AddUndo(ps,editcur.track,line,1,1,editcur.track,line,editcur.col,editPosition);
			entry->_note = note;
			entry->_mach = _pSong->seqBus;

			if (note>notecommands::release)
			{
				entry->_inst = _pSong->auxcolSelected;
			}
			else
			{
				if ( _pSong->seqBus < MAX_MACHINES && _pSong->_pMachine[_pSong->seqBus] != 0 ) 
				{
					// if the current machine is a sampler, check 
					// if current sample is locked to a machine.
					// if so, switch entry._mach to that machine number
					if (((Machine*)_pSong->_pMachine[_pSong->seqBus])->_type == MACH_SAMPLER)
					{
						if ((_pSong->_pInstrument[instNo]->_lock_instrument_to_machine != -1)
							&& (_pSong->_pInstrument[instNo]->_LOCKINST == true))
						{
							entry->_mach = _pSong->_pInstrument[instNo]->_lock_instrument_to_machine;
						}
					}
				}

				if ( note < notecommands::release && Global::psycleconf().inputHandler()._RecordTweaks && velocity < 127)
				{
					if (Global::psycleconf().midi().raw())
					{
						entry->_cmd = 0x0c;
						entry->_parameter = velocity * 2;
					}
					else if (Global::psycleconf().midi().velocity().record())
					{
						// command
						entry->_cmd = Global::psycleconf().midi().velocity().command();
						int par = Global::psycleconf().midi().velocity().from() + (Global::psycleconf().midi().velocity().to() - Global::psycleconf().midi().velocity().from()) * velocity / 127;
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
				Machine *tmac = _pSong->_pMachine[entry->_mach];
				if (tmac)
				{
					if (tmac->NeedsAuxColumn())
					{
						entry->_inst = instNo;
					}
					tmac->Tick(editcur.track, entry);
				}
			}

			Global::pInputHandler->notetrack[editcur.track]=note;
			Global::pInputHandler->instrtrack[editcur.track]=instNo;
			NewPatternDraw(editcur.track,editcur.track,line,line);
			if (!(Global::pPlayer->_playing&&Global::psycleconf()._followSong))
			{
				if (ChordModeOffs)
				{
					AdvanceLine(-1,Global::psycleconf().inputHandler()._wrapAround,false); //Advance track?
				}
				else
				{
					AdvanceLine(patStep,Global::psycleconf().inputHandler()._wrapAround,false);
				}
			}

			bScrollDetatch=false;
			Global::pInputHandler->bDoingSelection = false;
			Repaint(draw_modes::data);
		}

		void CChildView::EnterNoteoffAny()
		{
			if (viewMode == view_modes::pattern)
			{
				const int ps = _ps();
				unsigned char * toffset;
				
				// realtime note entering
				if (Global::pPlayer->_playing&&Global::psycleconf()._followSong)
				{
					toffset = _ptrack(ps)+(Global::pPlayer->_lineCounter*MULTIPLY);
				}
				else
				{
					toffset = _ptrackline(ps);
				}

				// build entry
				PatternEntry *entry = (PatternEntry*) toffset;
				Global::pInputHandler->AddUndo(ps,editcur.track,editcur.line,1,1,editcur.track,editcur.line,editcur.col,editPosition);
				entry->_note = notecommands::release;

				Global::pInputHandler->notetrack[editcur.track]=notecommands::release;
				Global::pInputHandler->instrtrack[editcur.track]=255;

				NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);

				if (!(Global::pPlayer->_playing&&Global::psycleconf()._followSong))
				{
					AdvanceLine(patStep,Global::psycleconf().inputHandler()._wrapAround,false);
				}

				bScrollDetatch=false;
				Global::pInputHandler->bDoingSelection = false;
				Repaint(draw_modes::data);
			}
		}

		bool CChildView::MSBPut(int nChar)
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

			Global::pInputHandler->AddUndo(ps,editcur.track,editcur.line,1,1,editcur.track,editcur.line,editcur.col,editPosition);

			switch ((editcur.col+1)%2)
			{
			case 0:	
				*toffset = (oldValue&0xF)+(sValue<<4); 
				break;
			
			case 1:	
				*toffset = (oldValue&0xF0)+(sValue); 
				break;
			}

			if (Global::psycleconf().inputHandler()._cursorAlwaysDown)
			{
				AdvanceLine(patStep,Global::psycleconf().inputHandler()._wrapAround,false);
			}
			else
			{
				switch (editcur.col)
				{
				case 0:
					AdvanceLine(patStep,Global::psycleconf().inputHandler()._wrapAround,false);
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
					AdvanceLine(patStep,Global::psycleconf().inputHandler()._wrapAround,false);
					break;
				}
			}
			bScrollDetatch=false;
			Global::pInputHandler->bDoingSelection = false;
			NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);
			Repaint(draw_modes::data);
			return true;
		}

		void CChildView::ClearCurr() // delete content at Cursor pos.
		{
			// UNDO CODE CLEAR
			const int ps = _ps();
			unsigned char * offset = _ptrack(ps);
			unsigned char * toffset = _ptrackline(ps);

			Global::pInputHandler->AddUndo(ps,editcur.track,editcur.line,1,1,editcur.track,editcur.line,editcur.col,editPosition);

			// &&&&& hardcoded # of bytes per event
			if ( editcur.col == 0 )
			{
				memset(offset+(editcur.line*MULTIPLY),255,3*sizeof(char));
				memset(offset+(editcur.line*MULTIPLY)+3,0,2*sizeof(char));
			}
			else if (editcur.col < 5 )	{	*(toffset+(editcur.col+1)/2)= 255; }
			else						{	*(toffset+(editcur.col+1)/2)= 0; }

			NewPatternDraw(editcur.track,editcur.track,editcur.line,editcur.line);

			AdvanceLine(patStep,Global::psycleconf().inputHandler()._wrapAround,false);
			Global::pInputHandler->bDoingSelection = false;
			ChordModeOffs = 0;
			bScrollDetatch=false;
			Repaint(draw_modes::data);
		}

		void CChildView::DeleteCurr()
		{
			// UNDO CODE DELETE
			const int ps = _ps();
			unsigned char * offset = _ptrack(ps);
			int patlines = _pSong->patternLines[ps];

			if ( Global::psycleconf().inputHandler().bFT2DelBehaviour )
			{
				if(editcur.line==0)
					return;
				else
					editcur.line--;
			}

			Global::pInputHandler->AddUndo(ps,editcur.track,editcur.line,1,patlines-editcur.line,editcur.track,editcur.line,editcur.col,editPosition);

			int i;
			for (i=editcur.line; i < patlines-1; i++)
				memcpy(offset+(i*MULTIPLY), offset+((i+1)*MULTIPLY), EVENT_SIZE);

			PatternEntry blank;
			memcpy(offset+(i*MULTIPLY),&blank,EVENT_SIZE);

			NewPatternDraw(editcur.track,editcur.track,editcur.line,patlines-1);

			Global::pInputHandler->bDoingSelection = false;
			ChordModeOffs = 0;
			bScrollDetatch=false;
			Repaint(draw_modes::data);
		}

		void CChildView::InsertCurr()
		{
			// UNDO CODE INSERT
			const int ps = _ps();
			unsigned char * offset = _ptrack(ps);
			int patlines = _pSong->patternLines[ps];

			Global::pInputHandler->AddUndo(ps,editcur.track,editcur.line,1,patlines-editcur.line,editcur.track,editcur.line,editcur.col,editPosition);

			int i;
			for (i=patlines-1; i > editcur.line; i--)
				memcpy(offset+(i*MULTIPLY), offset+((i-1)*MULTIPLY), EVENT_SIZE);

			PatternEntry blank;
			memcpy(offset+(i*MULTIPLY),&blank,EVENT_SIZE);

			NewPatternDraw(editcur.track,editcur.track,editcur.line,patlines-1);

			Global::pInputHandler->bDoingSelection = false;
			ChordModeOffs = 0;
			bScrollDetatch=false;
			Repaint(draw_modes::data);
		}



		//////////////////////////////////////////////////////////////////////
		// Plays entire row of the pattern

		void CChildView::PlayCurrentRow(void)
		{
			if (Global::psycleconf()._followSong)
			{
				bScrollDetatch=false;
			}
			PatternEntry* pEntry = (PatternEntry*)_ptrackline(_ps(),0,editcur.line);

			for (int i=0; i<_pSong->SONGTRACKS;i++)
			{
				if (pEntry->_mach < MAX_MACHINES && !_pSong->_trackMuted[i])
				{
					Machine *pMachine = _pSong->_pMachine[pEntry->_mach];
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

		void CChildView::PlayCurrentNote(void)
		{
			if (Global::psycleconf()._followSong)
			{
				bScrollDetatch=false;
			}

			PatternEntry* pEntry = (PatternEntry*)_ptrackline();
			if (pEntry->_mach < MAX_MACHINES)
			{
				Machine *pMachine = _pSong->_pMachine[pEntry->_mach];
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
		void CChildView::PrevCol(bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			CChildView::blockSelectBarState = 1;

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
				Repaint(draw_modes::cursor);
			}
		}

		void CChildView::NextCol(bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			CChildView::blockSelectBarState = 1;

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
				Repaint(draw_modes::cursor);
			}
		}

		void CChildView::PrevLine(int x, bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			CChildView::blockSelectBarState = 1;

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
			if (updateDisplay) Repaint(draw_modes::cursor);
		}

		void CChildView::AdvanceLine(int x,bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			CChildView::blockSelectBarState = 1;

			const int nl = _pSong->patternLines[_ps()];

			// <sampler> a bit recoded. 
			if (x<0) //kind of trick used to advance track (related to chord mode).
			{
				editcur.track+=1;
				if (editcur.track >= _pSong->SONGTRACKS)
				{
					editcur.track=0;
					editcur.line+=1;
				}
			}
			else //advance corresponding rows
			{
				if (wrap)
				{
					editcur.line += x;
					editcur.line = editcur.line % nl; 
				}
				else
				{
					if (editcur.line + x < nl) editcur.line = editcur.line + x;
					else editcur.line = nl -1; //as it has worked always.
				}
				
			}

			pParentMain->StatusBarIdle();
			if (updateDisplay) Repaint(draw_modes::cursor);
		}

		void CChildView::AdvanceTrack(int x,bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			CChildView::blockSelectBarState = 1;

			editcur.track+=x;
			editcur.col=0;
			
			if(editcur.track>= _pSong->SONGTRACKS)
			{
				if ( wrap ) editcur.track=0;
				else editcur.track=_pSong->SONGTRACKS-1;
			}
			
			pParentMain->StatusBarIdle();
			if (updateDisplay) Repaint(draw_modes::cursor);
		}

		void CChildView::PrevTrack(int x,bool wrap,bool updateDisplay)
		{
			//reinitialise the select bar state
			CChildView::blockSelectBarState = 1;

			editcur.track-=x;
			editcur.col=0;
			
			if(editcur.track<0)
			{
				if (wrap) editcur.track=_pSong->SONGTRACKS-1;
				else editcur.track=0;
			}
			
			pParentMain->StatusBarIdle();
			if (updateDisplay) Repaint(draw_modes::cursor);
		}


		//////////////////////////////////////////////////////////////////////
		// Pattern Modifier functions ( Copy&paste , Transpose, ... )

		void CChildView::patCut()
		{
			if(viewMode == view_modes::pattern)
			{
				// UNDO CODE PATT CUT
				const int ps = _ps();
				unsigned char *soffset = _ppattern(ps);
				PatternEntry blank;

				patBufferLines = _pSong->patternLines[ps];
				Global::pInputHandler->AddUndo(ps,0,0,MAX_TRACKS,patBufferLines,editcur.track,editcur.line,editcur.col,editPosition);

				int length = patBufferLines*EVENT_SIZE*MAX_TRACKS;
				
				memcpy(patBufferData,soffset,length);
				for	(int c=0; c<length; c+=EVENT_SIZE)
				{
					memcpy(soffset,&blank,EVENT_SIZE);
					soffset+=EVENT_SIZE;
				}
				patBufferCopy = true;

				NewPatternDraw(0,_pSong->SONGTRACKS,0,patBufferLines-1);
				Repaint(draw_modes::data);
			}
		}

		void CChildView::patCopy()
		{
			if(viewMode == view_modes::pattern)
			{
				const int ps = _ps();
				unsigned char *soffset = _ppattern(ps);
				
				patBufferLines=_pSong->patternLines[ps];
				int length=patBufferLines*EVENT_SIZE*MAX_TRACKS;
				
				memcpy(patBufferData,soffset,length);
				
				patBufferCopy=true;
			}
		}

		void CChildView::patPaste()
		{
			// UNDO CODE PATT PASTE
			if(patBufferCopy && viewMode == view_modes::pattern)
			{
				const int ps = _ps();
				unsigned char *soffset = _ppattern(ps);
				// **************** funky shit goin on here yo with the pattern resize or some shit
				Global::pInputHandler->AddUndo(ps,0,0,MAX_TRACKS,_pSong->patternLines[ps],editcur.track,editcur.line,editcur.col,editPosition);
				if ( patBufferLines != _pSong->patternLines[ps] )
				{
					Global::pInputHandler->AddUndoLength(ps,_pSong->patternLines[ps],editcur.track,editcur.line,editcur.col,editPosition);
					_pSong->AllocNewPattern(ps,"",patBufferLines,false);
				}
				memcpy(soffset,patBufferData,patBufferLines*EVENT_SIZE*MAX_TRACKS);

				Repaint(draw_modes::pattern);
			}
		}

		void CChildView::patMixPaste()
		{
			// UNDO CODE PATT PASTE
			if(patBufferCopy && viewMode == view_modes::pattern)
			{
				const int ps = _ps();
				unsigned char* offset_target = _ppattern(ps);
				unsigned char* offset_source = patBufferData;
				// **************** funky shit goin on here yo with the pattern resize or some shit
				Global::pInputHandler->AddUndo(ps,0,0,MAX_TRACKS,_pSong->patternLines[ps],editcur.track,editcur.line,editcur.col,editPosition);
				if ( patBufferLines != _pSong->patternLines[ps] )
				{
					Global::pInputHandler->AddUndoLength(ps,_pSong->patternLines[ps],editcur.track,editcur.line,editcur.col,editPosition);
					_pSong->AllocNewPattern(ps,"",patBufferLines,false);
				}

				for (int i = 0; i < MAX_TRACKS*patBufferLines; i++)
				{
					if (*offset_target == 0xFF) *(offset_target)=*offset_source;
					if (*(offset_target+1)== 0xFF) *(offset_target+1)=*(offset_source+1);
					if (*(offset_target+2)== 0xFF) *(offset_target+2)=*(offset_source+2);
					if (*(offset_target+3)== 0) *(offset_target+3)=*(offset_source+3);
					if (*(offset_target+4)== 0) *(offset_target+4)=*(offset_source+4);
					offset_target+= EVENT_SIZE;
					offset_source+= EVENT_SIZE;
				}
				
				Repaint(draw_modes::pattern);
			}
		}

		void CChildView::patDelete()
		{
			if(viewMode == view_modes::pattern)
			{
				// UNDO CODE PATT CUT
				const int ps = _ps();
				unsigned char *soffset = _ppattern(ps);
				PatternEntry blank;

				patBufferLines = _pSong->patternLines[ps];
				Global::pInputHandler->AddUndo(ps,0,0,MAX_TRACKS,patBufferLines,editcur.track,editcur.line,editcur.col,editPosition);

				int length = patBufferLines*EVENT_SIZE*MAX_TRACKS;
				
				for	(int c=0; c<length; c+=EVENT_SIZE)
				{
					memcpy(soffset,&blank,EVENT_SIZE);
					soffset+=EVENT_SIZE;
				}

				NewPatternDraw(0,_pSong->SONGTRACKS,0,patBufferLines-1);
				Repaint(draw_modes::data);
			}
		}

		void CChildView::patTranspose(int trp)
		{
			// UNDO CODE PATT TRANSPOSE
			const int ps = _ps();
			unsigned char *soffset = _ppattern(ps);

			if(viewMode == view_modes::pattern)
			{
				int pLines=_pSong->patternLines[ps];
				int length=pLines*EVENT_SIZE*MAX_TRACKS;

				Global::pInputHandler->AddUndo(ps,0,0,MAX_TRACKS,pLines,editcur.track,editcur.line,editcur.col,editPosition);

				for	(int c=editcur.line*EVENT_SIZE*MAX_TRACKS;c<length;c+=EVENT_SIZE)
				{
					int note=*(soffset+c);
					
					if(note<notecommands::release)
					{
						note+=trp;
						if(note<0) note=0; else if(note>119) note=119;
						soffset[c]=static_cast<unsigned char>(note);
					}
				}
				NewPatternDraw(0,_pSong->SONGTRACKS,editcur.line,pLines-1);

				Repaint(draw_modes::data);
			}
		}

		void CChildView::StartBlock(int track,int line, int col)
		{

			//reinitialise the select bar state
			CChildView::blockSelectBarState = 1;

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

			Repaint(draw_modes::selection);
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

			Repaint(draw_modes::selection);
		}

		void CChildView::EndBlock(int track,int line, int col)
		{
			blockSel.end.track=track;
			blockSel.end.line=line;
			
			if ( blockSelected )
			{
				TRACE("%i,%i",blockSel.end.line,blockSel.start.line);
				if(blockSel.end.line<blockSel.start.line)
				{
					int tmp = blockSel.start.line;
					blockSel.start.line=blockSel.end.line;
					blockSel.end.line=tmp;
		//			blockSel.end.line=blockSel.start.line;
				}
				
				TRACE("%i,%i",blockSel.end.track,blockSel.start.track);
				if(blockSel.end.track<blockSel.start.track)
				{
					int tmp = blockSel.start.track;
					blockSel.start.track=blockSel.end.track;
					blockSel.end.track=tmp;
		//			blockSel.end.track=blockSel.start.track;
				}
				
			}
			else
			{
				blockSel.start.track=track;
				blockSel.start.line=line;
				iniSelec = blockSel.start;
			}
			blockSelected=true;

			Repaint(draw_modes::selection);
		}

		void CChildView::BlockUnmark()
		{
			blockSelected=false;

			//reinitialise the select bar state
			CChildView::blockSelectBarState = 1;

			Repaint(draw_modes::selection);
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
				
				int ls=0;
				int ts=0;
				PatternEntry blank;

				if (cutit)
				{
					Global::pInputHandler->AddUndo(ps,blockSel.start.track,blockSel.start.line,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);
				}
				for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
				{
					ls=0;
					for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
					{
						unsigned char *offset_target=blockBufferData+(ts*EVENT_SIZE+ls*MULTIPLY);				
						unsigned char *offset_source=_ptrackline(ps,t,l);
						
						memcpy(offset_target,offset_source,EVENT_SIZE);
						
						if(cutit)
							memcpy(offset_source,&blank,EVENT_SIZE);
						
						++ls;
					}
					++ts;
				}
				if(cutit)
				{
					NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);

					Repaint(draw_modes::data);
				}
			}
		}

		void CChildView::DeleteBlock()
		{
			if(blockSelected)
			{
				int ps=_pSong->playOrder[editPosition];
				
				PatternEntry blank;

				// UNDO CODE HERE CUT
				Global::pInputHandler->AddUndo(ps,blockSel.start.track,blockSel.start.line,blockNTracks,blockNLines,editcur.track,editcur.line,editcur.col,editPosition);
				for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
				{
					for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
					{
						memcpy(_ptrackline(ps,t,l),&blank,EVENT_SIZE);
					}
				}
				NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
				Repaint(draw_modes::data);
			}
		}

		void CChildView::PasteBlock(int tx,int lx,bool mix,bool save)
		{
			if(isBlockCopied)
			{
				int ps=_pSong->playOrder[editPosition];
				int nl = _pSong->patternLines[ps];

				// UNDO CODE PASTE AND MIX PASTE
				if (save) Global::pInputHandler->AddUndo(ps,tx,lx,blockNTracks,nl,editcur.track,editcur.line,editcur.col,editPosition);

				int ls=0;
				int ts=0;
				
				//added by sampler. There is a problem. The paste action can be undo but the lines are not reverted back.
				if (blockNLines > nl) 
					if (MessageBox("Do you want to autoincrease this pattern lines?","Block doesn't fit in current pattern",MB_YESNO) == IDYES)
					{
						_pSong->patternLines[ps] = blockNLines;
						nl = blockNLines;
					}
				//end of added by sampler

				for (int t=tx;t<tx+blockNTracks && t<_pSong->SONGTRACKS;t++)
				{
					ls=0;
					for (int l=lx;l<lx+blockNLines && l<nl;l++)
						{
							unsigned char* offset_source=blockBufferData+(ts*EVENT_SIZE+ls*MULTIPLY);
							unsigned char* offset_target=_ptrackline(ps,t,l);
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
								memcpy(offset_target,offset_source,EVENT_SIZE);
							}
						++ls;
					}
					++ts;
				}
				
				if (Global::psycleconf().inputHandler().bMoveCursorPaste)
				{
					if (lx+blockNLines < nl ) editcur.line = lx+blockNLines;
					else editcur.line = nl-1;
				}

				bScrollDetatch=false;
				NewPatternDraw(tx,tx+blockNTracks-1,lx,lx+blockNLines-1);
				Repaint(draw_modes::data);
			}
		}

		void CChildView::SwitchBlock(int destt, int destl)
		{
			if(blockSelected || isBlockCopied)// With shift+arrows, moving the cursor unselects the block, so in this case it is a three step
			{									// operation: select, copy, switch, instead of select, switch.
				int ps=_pSong->playOrder[editPosition];
				int nl = _pSong->patternLines[ps];
				bool bSwapTracks = false;
				bool bSwapLines = false;
				int ls=0;
				int ts=0;
				//Where to start reading and writing to free the destination area.
				int startRT=destt;
				int startRL=destl;
				int startWT=blockLastOrigin.start.track;
				int startWL=blockLastOrigin.start.line;
				PatternEntry blank;

				// Copy block(1) if not done already.
				if (blockSelected) CopyBlock(false);
				int stopT=destt+blockNTracks;
				int stopL=destl+blockNLines;

				// We backup the data of the whole block.
				Global::pInputHandler->AddUndo(ps,0,0,_pSong->SONGTRACKS,nl,editcur.track,editcur.line,editcur.col,editPosition);

				// Do the blocks overlap? Then take care of moving the appropiate data.
				if (abs(blockLastOrigin.start.track-destt) < blockNTracks	&& abs(blockLastOrigin.start.line-destl) < blockNLines )
				{
					if 	( blockLastOrigin.start.track != destt )  //Is the origin and destination track different?
					{
						// ok, then we need to exchange some of the tracks.
						bSwapTracks = true;

						// If the switch moves to the left, exchange the start of the destination block
						if ( blockLastOrigin.start.track > destt)
						{
							startRT=destt;
							startWT=destt+blockNTracks;
							stopT=blockLastOrigin.start.track;
						}
						else // else, exchange the end of the destination block.
						{
							startRT=blockLastOrigin.start.track+blockNTracks;
							startWT=blockLastOrigin.start.track;
							stopT=destt+blockNTracks;
						}
					}
					if ( blockLastOrigin.start.line != destl )  //Is the origin and destination line different?
					{
						// ok, then we need to exchange some of the lines.
						bSwapLines = true;

						// do we have to swap both, tracks and lines?
						if (bSwapTracks)
						{
							// We have a situation like this :  ( - don't care , O origin D destination Q overlap.)
							// The swap cannot be clean, as in maintaining the structure.
							//--OOOO   >>>>> --1234  >>>> --BCDE
							//DDQQOO   >>>>> 56789A  >>>> 123456
							//DDDD--   >>>>> BCDE--  >>>> 789A--

							int startRT2, startRL2, startWT2, startWL2, stopT2, stopL2;
							if ( blockLastOrigin.start.line > destl)
							{
								startRL2=destl;
								startWL2=destl+blockNLines;
								stopL2=blockLastOrigin.start.line;
								startRL=blockLastOrigin.start.line;
								startWL=blockLastOrigin.start.line;
								stopL=destl+blockNLines;
							}
							else
							{
								startRL2=blockLastOrigin.start.line+blockNLines;
								startWL2=blockLastOrigin.start.line;
								stopL2=destl+blockNLines;
								startRL=destl;
								startWL=destl;
								stopL=blockLastOrigin.start.line+blockNLines;
							}
							if ( blockLastOrigin.start.track > destt)
							{
								startRT2=destt;
								startWT2=blockLastOrigin.start.track;
								stopT2=destt+blockNTracks;
							}
							else
							{
								startRT2=destt;
								startWT2=blockLastOrigin.start.track;
								stopT2=destt+blockNTracks;
							}
							// We exchange just the lines here. The loop outside will exchange the tracks.
							ts = startWT2;
							for (int t=startRT2;t<stopT2 && t<_pSong->SONGTRACKS && ts<_pSong->SONGTRACKS;t++)
							{
								ls=startWL2;
								for (int l=startRL2;l<stopL2 && l<nl && ls <nl;l++)
								{
									unsigned char *offset_target=_ptrackline(ps,ts,ls);
									unsigned char *offset_source=_ptrackline(ps,t,l);

									memcpy(offset_target,offset_source,EVENT_SIZE);

									++ls;
								}
								++ts;
							}
						}
						else
						{
							// If the switch moves to the top, exchange the start of the destination block
							if ( blockLastOrigin.start.line > destl)
							{
								startRL=destl;
								startWL=destl+blockNLines;
								stopL=blockLastOrigin.start.line;
							}
							else
							{
								startRL=blockLastOrigin.start.line+blockNLines;
								startWL=blockLastOrigin.start.line;
								stopL=destl+blockNLines;
							}
						}
					}
					// There is nothing to Swap. blocks are the same.
					if (!bSwapTracks && !bSwapLines) return;
				}
			
				// do Swap "inplace".
				ts = startWT;
				for (int t=startRT;t<stopT && t<_pSong->SONGTRACKS && ts <_pSong->SONGTRACKS;t++)
				{
					ls=startWL;
					for (int l=startRL;l<stopL && l<nl && ls<nl;l++)
					{
						unsigned char *offset_target=_ptrackline(ps,ts,ls);
						unsigned char *offset_source=_ptrackline(ps,t,l);

						memcpy(offset_target,offset_source,EVENT_SIZE);

						++ls;
					}
					++ts;
				}

				// Finally, paste the Original selected block on the freed space.
				PasteBlock(destt, destl, false,false);
				
				NewPatternDraw(0,_pSong->SONGTRACKS-1,0,nl-1);
				Repaint(draw_modes::data);
			}
		}


		void CChildView::SaveBlock(FILE* file)
		{

			int ps = _ps();
			int nlines = _pSong->patternLines[ps];

			fwrite(&_pSong->SONGTRACKS, sizeof(int), 1, file);
			fwrite(&nlines, sizeof(int), 1, file);

			for (int t=0;t<_pSong->SONGTRACKS;t++)
			{
				for (int l=0;l<nlines;l++)
				{
					unsigned char* offset_source=_ptrackline(ps,t,l);
					
					fwrite(offset_source,sizeof(char),EVENT_SIZE,file);
				}
			}
		}

		void CChildView::LoadBlock(FILE* file)
		{
			int nt, nl;
			fread(&nt,sizeof(int),1,file);
			fread(&nl,sizeof(int),1,file);

			if ((nt > 0) && (nl > 0))
			{

				int ps = _ps();
				int nlines = _pSong->patternLines[ps];
				Global::pInputHandler->AddUndo(ps,0,0,MAX_TRACKS,nlines,editcur.track,editcur.line,editcur.col,editPosition);
				if (nlines != nl)
				{
					Global::pInputHandler->AddUndoLength(ps,nlines,editcur.track,editcur.line,editcur.col,editPosition);
					_pSong->patternLines[ps] = nl;
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
				PatternEntry blank;

				for (int t = nt; t < MAX_TRACKS;t++)
				{
					for (int l = nl; l < MAX_LINES; l++)
					{
						unsigned char* offset_target=_ptrackline(ps,t,l);
						memcpy(offset_target,&blank,EVENT_SIZE);
					}
				}
				Repaint(draw_modes::pattern);
			}
		}

		void CChildView::DoubleLength()
		{
			// UNDO CODE DOUBLE LENGTH
			unsigned char *toffset;
			PatternEntry blank;
			int st, et, sl, el,nl;

			int ps = _ps();
			if ( blockSelected )
			{
		///////////////////////////////////////////////////////// Add ROW
				st=blockSel.start.track;		
				et=blockSel.end.track+1;
				sl=blockSel.start.line;			
				nl=((blockSel.end.line-sl)/2)+1;
				el=blockSel.end.line;
				Global::pInputHandler->AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,nl*2,editcur.track,editcur.line,editcur.col,editPosition);
			}
			else 
			{
				st=0;		
				et=_pSong->SONGTRACKS;		
				sl=0;
				nl= _pSong->patternLines[ps]/2;	
				el=_pSong->patternLines[ps]-1;
				Global::pInputHandler->AddUndo(ps,0,0,MAX_TRACKS,el+1,editcur.track,editcur.line,editcur.col,editPosition);
			}

			for (int t=st;t<et;t++)
			{
				toffset=_ptrack(ps,t);
				memcpy(toffset+el*MULTIPLY,&blank,EVENT_SIZE);
				for (int l=nl-1;l>0;l--)
				{
					memcpy(toffset+(sl+l*2)*MULTIPLY,toffset+(sl+l)*MULTIPLY,EVENT_SIZE);
					memcpy(toffset+(sl+(l*2)-1)*MULTIPLY,&blank,EVENT_SIZE);
				}
			}

			NewPatternDraw(st,et,sl,el);
			Repaint(draw_modes::data);
		}

		void CChildView::HalveLength()
		{
			// UNDO CODE HALF LENGTH
			unsigned char *toffset;
			int st, et, sl, el,nl;
			int ps = _ps();
			PatternEntry blank;

			if ( blockSelected )
			{
		///////////////////////////////////////////////////////// Add ROW
				st=blockSel.start.track;	
				et=blockSel.end.track+1;
				sl=blockSel.start.line;		
				nl=blockSel.end.line-sl+1;
				el=nl/2;
				Global::pInputHandler->AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,nl,editcur.track,editcur.line,editcur.col,editPosition);
			}
			else 
			{
				st=0;	
				et=_pSong->SONGTRACKS;		
				sl=0;
				nl=_pSong->patternLines[ps];	
				el=_pSong->patternLines[ps]/2;
				Global::pInputHandler->AddUndo(ps,0,0,MAX_TRACKS,nl,editcur.track,editcur.line,editcur.col,editPosition);
			}
			
			for (int t=st;t<et;t++)
			{
				toffset=_ptrack(ps,t);
				int l;
				for (l=1;l<el;l++)
				{
					memcpy(toffset+(l+sl)*MULTIPLY,toffset+((l*2)+sl)*MULTIPLY,EVENT_SIZE);
				}
				while (l < nl)
				{
					memcpy(toffset+((l+sl)*MULTIPLY),&blank,EVENT_SIZE);
					l++;
				}
			}

			NewPatternDraw(st,et,sl,nl+sl);
			Repaint(draw_modes::data);
		}


		void CChildView::BlockTranspose(int trp)
		{
			// UNDO CODE TRANSPOSE
			if ( blockSelected == true ) 
			{
				int ps = _ps();

				Global::pInputHandler->AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,blockSel.end.line-blockSel.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

				for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
				{
					for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
					{
						unsigned char *toffset=_ptrackline(ps,t,l);
						
						int note=*(toffset);
					
						if(note<notecommands::release)
						{
							note+=trp;
							if(note<0) note=0; else if(note>119) note=119;
							*toffset=static_cast<unsigned char>(note);
						}
					}
				}
				NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
				Repaint(draw_modes::data);
			}
		}


		void CChildView::BlockGenChange(int x)
		{
			// UNDO CODE BLOCK GENERATOR CHANGE
			if ( blockSelected == true ) 
			{
				int ps = _ps();
				Global::pInputHandler->AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,blockSel.end.line-blockSel.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

				for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
				{
					for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
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
				NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
				Repaint(draw_modes::data);
			}
		}

		void CChildView::BlockInsChange(int x)
		{
			// UNDO CODE BLOCK INS CHANGE
			if ( blockSelected == true ) 
			{
				const int ps=_ps();

				Global::pInputHandler->AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,blockSel.end.line-blockSel.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);

				for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
				{
					for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
					{
						unsigned char *toffset=_ptrackline(ps,t,l);
						unsigned char ins=*(toffset+1);
						unsigned char mac=*(toffset+2);
					
						if (mac != 255 )
						{
							ins=x;
							if(ins<0)ins=0;
							if(ins>255)ins=255;
							*(toffset+1)=ins;
						}
					}
				}
				NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
				Repaint(draw_modes::data);
			}
		}

		void CChildView::BlockParamInterpolate(int *points, int twktype)
		{
			if (blockSelected)
			{
				const int ps = _ps();
				///////////////////////////////////////////////////////// Add ROW
				unsigned char *toffset=_ppattern(ps);
				
				Global::pInputHandler->AddUndo(ps,blockSel.start.track,blockSel.start.line,blockSel.end.track-blockSel.start.track+1,blockSel.end.line-blockSel.start.line+1,editcur.track,editcur.line,editcur.col,editPosition);
				
				const int initvalue = 
					*(toffset+blockSel.start.track*EVENT_SIZE+blockSel.start.line*MULTIPLY+3) * 0x100 +
					*(toffset+blockSel.start.track*EVENT_SIZE+blockSel.start.line*MULTIPLY+4);
				const int endvalue =
					*(toffset+blockSel.start.track*EVENT_SIZE+blockSel.end.line*MULTIPLY+3) * 0x100 +
					*(toffset+blockSel.start.track*EVENT_SIZE+blockSel.end.line*MULTIPLY+4);
				const float addvalue = float(endvalue - initvalue) / (blockSel.end.line - blockSel.start.line);
				const int firstrow = (blockSel.start.track*EVENT_SIZE)+(blockSel.start.line*MULTIPLY);
				int displace = firstrow;
				
				if ( toffset[firstrow] == notecommands::tweak || toffset[firstrow] == notecommands::tweakeffect || toffset[firstrow] == notecommands::tweakslide || toffset[firstrow] == notecommands::midicc || twktype != notecommands::empty)
				{
					unsigned char note = (twktype != notecommands::empty)?twktype:toffset[firstrow];
					unsigned char aux = (twktype != notecommands::empty)?Global::_pSong->auxcolSelected:toffset[firstrow+1];
					unsigned char mac = (twktype != notecommands::empty)?Global::_pSong->seqBus:toffset[firstrow+2];
					for (int l=blockSel.start.line;l<=blockSel.end.line;l++)
					{
						toffset[displace]=note;
						toffset[displace+1]=aux;
						toffset[displace+2]=mac;
						int val= (points)? points[l-blockSel.start.line]: /* round toward zero */ static_cast<int>(initvalue+addvalue*(l-blockSel.start.line));
						if ( val == -1 ) continue;
						toffset[displace+3]=static_cast<unsigned char>(val/0x100);
						toffset[displace+4]=static_cast<unsigned char>(val%0x100);
						displace+=MULTIPLY;
					}
				}
				else
				{
					unsigned char mac = toffset[firstrow+2];
					for (int l=blockSel.start.line;l<=blockSel.end.line;l++)
					{
						int val = (points)? points[l-blockSel.start.line]: /* round toward zero */ static_cast<int>(initvalue+addvalue*(l-blockSel.start.line));
						if ( val == -1 ) continue;
						toffset[displace+2]=mac;
						toffset[displace+3]=static_cast<unsigned char>(val/0x100);
						toffset[displace+4]=static_cast<unsigned char>(val%0x100);
						displace+=MULTIPLY;
					}
				}
				NewPatternDraw(blockSel.start.track,blockSel.end.track,blockSel.start.line,blockSel.end.line);
				Repaint(draw_modes::data);
			}
		}


		void CChildView::IncCurPattern()
		{
			if(_pSong->playOrder[editPosition]<(MAX_PATTERNS-1))
			{
				Global::pInputHandler->AddUndoSequence(_pSong->playLength,editcur.track,editcur.line,editcur.col,editPosition);
				++_pSong->playOrder[editPosition];
				pParentMain->UpdatePlayOrder(true);
				Repaint(draw_modes::pattern);
			}
		}


		void CChildView::DecCurPattern()
		{
			if(_pSong->playOrder[editPosition]>0)
			{
				Global::pInputHandler->AddUndoSequence(_pSong->playLength,editcur.track,editcur.line,editcur.col,editPosition);
				--_pSong->playOrder[editPosition];
				pParentMain->UpdatePlayOrder(true);
				Repaint(draw_modes::pattern);
			}
		}

		void CChildView::DecPosition()
		{
		//	case cdefPlaySkipBack:
			if (Global::pPlayer->_playing && Global::psycleconf()._followSong)
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
				Repaint(draw_modes::pattern);
				if (Global::pPlayer->_playing) {
					Repaint(draw_modes::playback);
				}
			}
		}

		void CChildView::IncPosition(bool bRepeat)
		{
		//	case cdefPlaySkipAhead:
			if (Global::pPlayer->_playing && Global::psycleconf()._followSong)
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
					if ( _pSong->playLength+1 > MAX_SONG_POSITIONS) return;

					Global::pInputHandler->AddUndoSequence(_pSong->playLength,editcur.track,editcur.line,editcur.col,editPosition);
					int patternum=_pSong->GetBlankPatternUnused();
					if ( patternum>= MAX_PATTERNS )
					{
						patternum=MAX_PATTERNS-1;
					}
					else 
					{
						_pSong->AllocNewPattern(patternum,"",Global::psycleconf().GetDefaultPatLines(),false);
					}
			
					++_pSong->playLength;
					++editPosition;
					_pSong->playOrder[editPosition]=patternum;
					
					pParentMain->UpdateSequencer();
				}

				memset(_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
				_pSong->playOrderSel[editPosition]=true;

				pParentMain->UpdatePlayOrder(true);
				Repaint(draw_modes::pattern);
				if (Global::pPlayer->_playing) {
					Repaint(draw_modes::playback);
				}
			}
		}

		void CChildView::SelectMachineUnderCursor()
		{
			unsigned char *toffset=_ptrackline();

			PatternEntry *entry = (PatternEntry*) toffset;

			if ( entry->_mach < MAX_BUSES*2 ) _pSong->seqBus = entry->_mach;
			pParentMain->ChangeGen(_pSong->seqBus);
			if ( entry->_inst != 255 ) _pSong->auxcolSelected = entry->_inst;
			pParentMain->ChangeIns(_pSong->auxcolSelected);

		}


		void CChildView::OnEditUndo() 
		{
			SPatternUndo * pUndoList = Global::pInputHandler->pUndoList;
			if (pUndoList)
			{
				switch (pUndoList->type)
				{
				case UNDO_PATTERN:
					if(viewMode == view_modes::pattern)// && bEditMode)
					{
						Global::pInputHandler->AddRedo(pUndoList->pattern,pUndoList->x,pUndoList->y,pUndoList->tracks,pUndoList->lines,editcur.track,editcur.line,editcur.col,pUndoList->seqpos,pUndoList->counter);
						// do undo
						unsigned char* pData = pUndoList->pData;

						for (int t=pUndoList->x;t<pUndoList->x+pUndoList->tracks;t++)
						{
							for (int l=pUndoList->y;l<pUndoList->y+pUndoList->lines;l++)
							{
								unsigned char *offset_source=_ptrackline(pUndoList->pattern,t,l);
								
								memcpy(offset_source,pData,EVENT_SIZE);
								pData+=EVENT_SIZE;
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
							Repaint(draw_modes::data);
						}
						else
						{
							editPosition = pUndoList->seqpos;
							pParentMain->UpdatePlayOrder(true);
							Repaint(draw_modes::pattern);
							
						}
						// delete undo from list
						SPatternUndo* pTemp = pUndoList->pPrev;
						delete (pUndoList->pData);
						delete (pUndoList);
						pUndoList = pTemp;
					}
					break;
				case UNDO_LENGTH:
					if(viewMode == view_modes::pattern)// && bEditMode)
					{
						Global::pInputHandler->AddRedoLength(pUndoList->pattern,_pSong->patternLines[pUndoList->pattern],editcur.track,editcur.line,editcur.col,pUndoList->seqpos,pUndoList->counter);
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
						Repaint(draw_modes::pattern);
						
						// delete undo from list
						SPatternUndo* pTemp = pUndoList->pPrev;
						delete (pUndoList->pData);
						delete (pUndoList);
						pUndoList = pTemp;
						break;
					}
				case UNDO_SEQUENCE:
					Global::pInputHandler->AddRedoSequence(_pSong->playLength,editcur.track,editcur.line,editcur.col,editPosition,pUndoList->counter);
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
					Repaint(draw_modes::pattern);
					
					// delete undo from list
					{
						SPatternUndo* pTemp = pUndoList->pPrev;
						delete (pUndoList->pData);
						delete (pUndoList);
						pUndoList = pTemp;
					}
					break;
				case UNDO_SONG:
					Global::pInputHandler->AddRedoSong(editcur.track,editcur.line,editcur.col,editPosition,pUndoList->counter);
					// do undo
					unsigned char * pData = pUndoList->pData;
					memcpy(_pSong->playOrder, pData, MAX_SONG_POSITIONS*sizeof(char));
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
					_pSong->playLength = pUndoList->lines;
					// set up cursor
					editcur.track = pUndoList->edittrack;
					editcur.line = pUndoList->editline;
					editcur.col = pUndoList->editcol;
					editPosition = pUndoList->seqpos;
					pParentMain->UpdatePlayOrder(true);
					pParentMain->UpdateSequencer();
					// display changes
					Repaint(draw_modes::pattern);
					
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
			SPatternUndo * pRedoList = Global::pInputHandler->pRedoList;
			if (pRedoList)
			{
				switch (pRedoList->type)
				{
				case UNDO_PATTERN:
					if(viewMode == view_modes::pattern)// && bEditMode)
					{
						Global::pInputHandler->AddUndo(pRedoList->pattern,pRedoList->x,pRedoList->y,pRedoList->tracks,pRedoList->lines,editcur.track,editcur.line,editcur.col,pRedoList->seqpos,false,pRedoList->counter);
						// do redo
						unsigned char* pData = pRedoList->pData;

						for (int t=pRedoList->x;t<pRedoList->x+pRedoList->tracks;t++)
						{
							for (int l=pRedoList->y;l<pRedoList->y+pRedoList->lines;l++)
							{
								unsigned char *offset_source=_ptrackline(pRedoList->pattern,t,l);

								
								memcpy(offset_source,pData,EVENT_SIZE);
								pData+=EVENT_SIZE;
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
							Repaint(draw_modes::data);
						}
						else
						{
							editPosition = pRedoList->seqpos;
							pParentMain->UpdatePlayOrder(true);
							Repaint(draw_modes::pattern);
							
						}
						// delete redo from list
						SPatternUndo* pTemp = pRedoList->pPrev;
						delete (pRedoList->pData);
						delete (pRedoList);
						pRedoList = pTemp;
					}
					break;
				case UNDO_LENGTH:
					if(viewMode == view_modes::pattern)// && bEditMode)
					{
						Global::pInputHandler->AddUndoLength(pRedoList->pattern,_pSong->patternLines[pRedoList->pattern],editcur.track,editcur.line,editcur.col,pRedoList->seqpos,false,pRedoList->counter);
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
						Repaint(draw_modes::pattern);
						
						// delete redo from list
						SPatternUndo* pTemp = pRedoList->pPrev;
						delete (pRedoList->pData);
						delete (pRedoList);
						pRedoList = pTemp;
						break;
					}
				case UNDO_SEQUENCE:
					Global::pInputHandler->AddUndoSequence(_pSong->playLength,editcur.track,editcur.line,editcur.col,editPosition,false,pRedoList->counter);
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
					Repaint(draw_modes::pattern);
					
					{
						// delete redo from list
						SPatternUndo* pTemp = pRedoList->pPrev;
						delete (pRedoList->pData);
						delete (pRedoList);
						pRedoList = pTemp;
					}
					break;
				case UNDO_SONG:
					Global::pInputHandler->AddUndoSong(editcur.track,editcur.line,editcur.col,editPosition,false,pRedoList->counter);
					// do undo
					unsigned char * pData = pRedoList->pData;
					memcpy(_pSong->playOrder, pData, MAX_SONG_POSITIONS*sizeof(char));
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
					pParentMain->UpdatePlayOrder(true);
					pParentMain->UpdateSequencer();
					// display changes
					Repaint(draw_modes::pattern);
					
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

		void CChildView::SelectNextTrack()
		{
			int i;
			for (i = editcur.track+1; i < _pSong->SONGTRACKS; i++)
			{
				if (_pSong->_trackArmed[i])
				{
					if (Global::pInputHandler->notetrack[i] == notecommands::release)
					{
						break;
					}
				}
			}
			if (i >= _pSong->SONGTRACKS)
			{
				for (i = 0; i <= editcur.track; i++)
				{
					if (_pSong->_trackArmed[i])
					{
						if (Global::pInputHandler->notetrack[i] == notecommands::release)
						{
							break;
						}
					}
				}
			}
			editcur.track = i;
			while(_pSong->_trackArmed[editcur.track] == 0)
			{
				if(++editcur.track >= _pSong->SONGTRACKS)
					editcur.track=0;
			}
			editcur.col = 0;
		}

}}
