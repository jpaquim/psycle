//////////////////////////////////////////////////////////////////////
// Keyboard Handler sourcecode

void CChildView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);	
	const int outnote = cmd.GetNote();
	Global::pInputHandler->StopNote(outnote);
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags )
{
	if(viewMode == VMPattern && bEditMode)
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

	BOOL bRepeat = nFlags&0x4000;

	// get command
	CmdDef cmd = Global::pInputHandler->KeyToCmd(nChar,nFlags);

	if(cmd.IsValid())
	{
		if((cmd.GetType() == CT_Immediate) ||
		   (cmd.GetType() == CT_Editor && viewMode == VMPattern) ) 
		{			
			Global::pInputHandler->PerformCmd(cmd);
		}
		else if (cmd.GetType() == CT_Note )
		{
			const int outnote = cmd.GetNote();
			// play note
			if(!bRepeat) {	Global::pInputHandler->PlayNote(outnote); }
		}
	}
	
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
				if(Global::pConfig->_midiRecordNoteoff)
				{
					EnterNote(120);	// note end
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

void CChildView::EnterNote(int note, int velocity, bool bTranspose)
{
	int ps = _ps();
	unsigned char * offset = _offset(ps);
	unsigned char * toffset = _toffset(ps);
	
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
	if (Global::pPlayer->_playing&&_followSong)
	{
		if(_previousTicks)
		{
			if(++editcur.track >= _pSong->SONGTRACKS)
				editcur.track=0;
		}
		_previousTicks++;

		offset = _offset(ps);
		toffset = offset+(Global::pPlayer->_lineCounter*MULTIPLY);
	}

	// build entry
	PatternEntry *entry = (PatternEntry*) toffset;
	entry->_note = note;
	entry->_mach = _pSong->seqBus;
	entry->_inst = _pSong->auxcolSelected;

	int mgn;
	if ( (_pSong->seqBus & MAX_BUSES) ) // If it is an effect
		mgn = _pSong->busEffect[(_pSong->seqBus & (MAX_BUSES-1))];
	else
		mgn = _pSong->busMachine[_pSong->seqBus];

	if (mgn != 255)
	{
		Machine *tmac = Global::_pSong->_pMachines[mgn];
		if ( tmac->_type == MACH_SAMPLER || tmac->_type == MACH_VST)
		{
			if( entry->_cmd==0 || entry->_cmd==0x0C )
			{
				if(velocity==127)
				{
					entry->_cmd = 0;
					entry->_parameter = 0;
				}
				else
				{
					entry->_cmd = 0x0C;
					entry->_parameter = velocity*2;
				}
			}					
		}

		if ( note < 120)
		{
			Global::pInputHandler->notetrack[editcur.track]=note;
			tmac->Tick(editcur.track, entry);
		}
	}

/*	if(note>=0)
	{
		// octave offset
		if(note<120)
		{
			if(bTranspose)
				note+=_pSong->currentOctave*12;

			if (note > 119) 
				note = 119;
		}

		// realtime note entering
		if (Global::pPlayer->_playing&&_followSong)
		{
			if(_previousTicks)
			{
				if(++editcur.track >= _pSong->SONGTRACKS)
					editcur.track=0;
			}
			_previousTicks++;

			offset = _offset(ps);
			toffset = offset+(Global::pPlayer->_lineCounter*MULTIPLY);
		}
		
	
		// build entry
		PatternEntry *entry = (PatternEntry*) toffset;
		entry->_note = note;
		entry->_mach = _pSong->seqBus;
		
		// insert
		int mgn;
		if ( note == 122 )  mgn = _pSong->busEffect[_pSong->seqBus];
		else mgn = _pSong->busMachine[_pSong->seqBus];
		if (mgn != 255)
		{
			Machine *tmac = Global::_pSong->_pMachines[mgn];
			
			if ( note == 121 || note == 122 )
			{
				entry->_inst = _pSong->instSelected;
			}
			else if ( tmac->_type == MACH_SAMPLER || tmac->_type == MACH_VST)
			{
				entry->_inst = _pSong->instSelected;
				if( entry->_cmd==0 || entry->_cmd==0x0C )
				{
					if(velocity==127)
					{
						entry->_cmd = 0;
						entry->_parameter = 0;
					}
					else
					{
						entry->_cmd = 0x0C;
						entry->_parameter = velocity*2;
					}
				}					
			}
			else 
			{
				entry->_inst = 255;
			}

			Global::pInputHandler->notetrack[editcur.track]=note;
			tmac->Tick(editcur.track, entry);
		}
	}
*/
	drawTrackStart=editcur.track;
	drawTrackEnd=editcur.track;
	drawLineStart=editcur.line;
	drawLineEnd=editcur.line;

	if ( GetKeyState(VK_SHIFT)<0) 
		AdvanceLine(0,Global::pConfig->_wrapAround,false);
	else
		AdvanceLine(patStep,Global::pConfig->_wrapAround,false);

	Repaint(DMDataChange);
}

bool CChildView::MSBPut(int nChar)
{
	// init
	const unsigned char ps = _ps();
	unsigned char * toffset = _toffset(ps) + (editcur.col+1)/2;

	int oldValue = *toffset;	
	int sValue = -1;

	if	(	nChar>='0'		&&	nChar<='9')			{ sValue = nChar - '0'; }
	else if(nChar>=VK_NUMPAD0&&nChar<=VK_NUMPAD9)	{ sValue = nChar - VK_NUMPAD0; }
	else if(nChar>='A'		&&	nChar<='F')			{ sValue = nChar - 'A' + 10; }
	else											{ return false; }

	if (editcur.col < 5 && oldValue == 255)	{ oldValue = 0; }

	drawTrackStart=editcur.track;
	drawTrackEnd=editcur.track;
	drawLineStart=editcur.line;
	drawLineEnd=editcur.line;

	switch ((editcur.col+1)%2)
	{
	case 0:	*toffset = (oldValue&0xF)+(sValue<<4); 
			if (Global::pConfig->_cursorAlwaysDown)
				AdvanceLine(patStep,Global::pConfig->_wrapAround,false);
			else
				NextCol(false,false);

			break;
	
	case 1:	*toffset = (oldValue&0xF0)+(sValue); 
			if (!Global::pConfig->_cursorAlwaysDown && editcur.col != 0)
				PrevCol(false,false);
			AdvanceLine(patStep,Global::pConfig->_wrapAround,false);

			break;
	}

	Repaint(DMDataChange);
	return true;
}

void CChildView::ClearCurr() // delete content at Cursor pos.
{
	int ps = _ps();
	unsigned char * offset = _offset(ps);
	unsigned char * toffset = _toffset(ps);

	if ( editcur.col == 0 )
	{
		memset(offset+(editcur.line*MULTIPLY),255,3*sizeof(char));
		memset(offset+(editcur.line*MULTIPLY)+3,0,2*sizeof(char));
	}
	else if (editcur.col < 5 )	{	*(toffset+(editcur.col+1)/2)= 255; }
	else						{	*(toffset+(editcur.col+1)/2)= 0; }

	drawTrackStart=editcur.track;
	drawTrackEnd=editcur.track;
	drawLineStart=editcur.line;
	drawLineEnd=editcur.line;

	AdvanceLine(patStep,Global::pConfig->_wrapAround,false);

	Repaint(DMDataChange);
}

void CChildView::DeleteCurr()
{
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

	for (int i=editcur.line; i < patlines-1; i++)
		memcpy(offset+(i*MULTIPLY), offset+((i+1)*MULTIPLY), 5);

	unsigned char blank[5]={255,255,255,0,0};
	memcpy(offset+(i*MULTIPLY),blank,5*sizeof(char));

	drawTrackStart=editcur.track;
	drawTrackEnd=editcur.track;
	drawLineStart=editcur.line;
	drawLineEnd=patlines-1;

	Repaint(DMDataChange);
}

void CChildView::InsertCurr()
{
	int ps = _ps();
	unsigned char * offset = _offset(ps);
	int patlines = _pSong->patternLines[ps];

	for (int i=patlines-1; i > editcur.line; i--)
		memcpy(offset+(i*MULTIPLY), offset+((i-1)*MULTIPLY), 5);

	unsigned char blank[5]={255,255,255,0,0};
	memcpy(offset+(i*MULTIPLY),blank,5*sizeof(char));

	drawTrackStart=editcur.track;
	drawTrackEnd=editcur.track;
	drawLineStart=editcur.line;
	drawLineEnd=patlines-1;

	Repaint(DMDataChange);
}



//////////////////////////////////////////////////////////////////////
// Plays entire row of the pattern

void CChildView::PlayCurrentRow(void)
{
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
	if (updateDisplay) Repaint(DMCursorMove);
}

void CChildView::NextCol(bool wrap,bool updateDisplay)
{
	if (++editcur.col > 8)
	{
		editcur.col = 0;
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
	if (updateDisplay) Repaint(DMCursorMove);
}

void CChildView::PrevLine(int x, bool wrap,bool updateDisplay)
{
	const int ps = _pSong->playOrder[editPosition];
	const int nl = _pSong->patternLines[ps];

	editcur.line -= x;

	if(editcur.line<0)
	{
		if(wrap){ editcur.line = nl + editcur.line % nl; }
		else	{ editcur.line = 0;	}
	}
	if (updateDisplay) Repaint(DMCursorMove);
}

void CChildView::AdvanceLine(int x,bool wrap,bool updateDisplay)
{
	const int ps = _pSong->playOrder[editPosition];
	const int nl = _pSong->patternLines[ps];

	if ( x != 0)	editcur.line += x;
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

	if (updateDisplay) Repaint(DMCursorMove);
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
	
	if (updateDisplay) Repaint(DMCursorMove);
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
	
	if (updateDisplay) Repaint(DMCursorMove);
}

//////////////////////////////////////////////////////////////////////
// Function that shift the current editing octave

void CChildView::ShiftOctave(int x)
{

	_pSong->currentOctave += x;
	if ( _pSong->currentOctave < 0 )	 { _pSong->currentOctave = 0; }
	else if ( _pSong->currentOctave > 8 ){ _pSong->currentOctave = 8; }

	Repaint(DMPatternHeader);
}

//////////////////////////////////////////////////////////////////////
// Pattern Modifier functions ( Copy&paste , Transpose, ... )

void CChildView::patCut()
{
	const int ps = _ps();
	unsigned char *soffset = _pSong->pPatternData + (ps*MULTIPLY2);
	unsigned char blank[5]={255,255,255,0,0};
	
	patBufferLines = _pSong->patternLines[ps];
	int length = patBufferLines*5*MAX_TRACKS;
	
	memcpy(patBufferData,soffset,length);
	for	(int c=0; c<length; c+=5)
	{
		memcpy(soffset,blank,sizeof(char)*5);
		soffset+=5;
	}
	patBufferCopy = true;

	Repaint();
}

void CChildView::patCopy()
{
	const int ps = _ps();
	unsigned char *soffset = _pSong->pPatternData + (ps*MULTIPLY2);
	
	patBufferLines=_pSong->patternLines[ps];
	int length=patBufferLines*5*MAX_TRACKS;
	
	memcpy(patBufferData,soffset,length);
	
	patBufferCopy=true;

	Repaint(DMCursorMove);
}

void CChildView::patPaste()
{
	const int ps = _ps();
	unsigned char *soffset = _pSong->pPatternData + (ps*MULTIPLY2);

	if(patBufferCopy && viewMode == VMPattern)
	{
		if ( patBufferLines != _pSong->patternLines[_ps()] )
		{
			_pSong->AllocNewPattern(_ps(),"",patBufferLines,false);
		}
		memcpy(soffset,patBufferData,patBufferLines*5*MAX_TRACKS);
		
		Repaint();
	}
}

void CChildView::patTranspose(int trp)
{
	const int ps = _ps();
	unsigned char *soffset = _pSong->pPatternData + (ps*MULTIPLY2);

	if(viewMode == VMPattern)
	{
		int pLines=_pSong->patternLines[ps];
		int length=pLines*5*MAX_TRACKS;
		
		for	(int c=editcur.line*5*MAX_TRACKS;c<length;c+=5)
		{
			int not=*(soffset+c);
			
			if(not<120)
			{
				not+=trp;
				if(not<0)not=0;
				if(not>119)not=119;
				*(soffset+c)=unsigned char(not);
			}
		}
		drawTrackStart=0;
		drawTrackEnd=_pSong->SONGTRACKS;
		drawLineStart=editcur.line;
		drawLineEnd=pLines-1;

		Repaint(DMDataChange);
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
	}
	
	if(cutit)
	{
		drawTrackStart=blockSel.start.track;
		drawTrackEnd=blockSel.end.track;
		drawLineStart=blockSel.start.line;
		drawLineEnd=blockSel.end.line;
		Repaint(DMDataChange);
	}
}

void CChildView::PasteBlock(int tx,int lx,bool mix)
{
	if(isBlockCopied)
	{
		int ps=_pSong->playOrder[editPosition];
		const int nl = _pSong->patternLines[ps];
		int displace=ps*MULTIPLY2;
		
		int ls=0;
		int ts=0;
		
		for (int t=tx;t<tx+blockNTracks;t++)
		{
			ls=0;
			for (int l=lx;l<lx+blockNLines;l++)
			{
				if(l<nl && t<Global::_pSong->SONGTRACKS)
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

		drawTrackStart=tx;
		drawTrackEnd=tx+blockNTracks-1;
		drawLineStart=lx;
		drawLineEnd=lx+blockNLines-1;
		Repaint(DMDataChange);
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
	const int displace=_pSong->playOrder[editPosition];
	unsigned char *toffset;
	unsigned char blank[5]={255,255,255,0,0};
	int st, et, sl, el,nl;

	if ( blockSelected )
	{
///////////////////////////////////////////////////////// Add ROW
		st=blockSel.start.track;		et=blockSel.end.track+1;
		sl=blockSel.start.line;			nl=blockSel.end.line-sl+1;
		el=sl+nl*2-1;
	}
	else 
	{
		st=0;		et=_pSong->SONGTRACKS;		sl=0;
		nl= _pSong->patternLines[displace]/2;	el=_pSong->patternLines[displace]-1;
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

	drawTrackStart=st;
	drawTrackEnd=et;
	drawLineStart=sl;
	drawLineEnd=el;
	Repaint(DMDataChange);
}

void CChildView::HalveLength()
{
	const int displace=_pSong->playOrder[editPosition];
	unsigned char *toffset;
	int st, et, sl, el,nl;
	unsigned char blank[5]={255,255,255,0,0};

	if ( blockSelected )
	{
///////////////////////////////////////////////////////// Add ROW
		st=blockSel.start.track;	et=blockSel.end.track+1;
		sl=blockSel.start.line;		nl=blockSel.end.line-sl+1;
		el=(blockSel.end.line-sl+1)/2;
	}
	else 
	{
		st=0;	et=_pSong->SONGTRACKS;		sl=0;
		nl=_pSong->patternLines[displace];	el=_pSong->patternLines[displace]/2;
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

	drawTrackStart=st;
	drawTrackEnd=et;
	drawLineStart=sl;
	drawLineEnd=el;
	Repaint(DMDataChange);
}


void CChildView::BlockTranspose(int trp)
{
	if ( blockSelected == true ) {

		const int displace=_pSong->playOrder[editPosition]*MULTIPLY2;
		unsigned char *toffset=_pSong->pPatternData+displace;

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
					*(toffset+displace2)=unsigned char(not);
				}
			}
		}
		drawTrackStart=blockSel.start.track;
		drawTrackEnd=blockSel.end.track;
		drawLineStart=blockSel.start.line;
		drawLineEnd=blockSel.end.line;
		Repaint(DMDataChange);
	}
}


void CChildView::BlockGenChange(int x)
{
	if ( blockSelected == true ) {

		const int displace=_pSong->playOrder[editPosition]*MULTIPLY2;
		unsigned char *toffset=_pSong->pPatternData+displace;

		for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
		{
			for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
			{
				const int displace2=t*5+l*MULTIPLY;
				
				int gen=*(toffset+displace2+2);
				
				if ( gen != 255 )
				{
					gen=x;
					if(gen<0)gen=0;
					if(gen>63)gen=63;
					*(toffset+displace2+2)=unsigned char(gen);
				}
			}
		}
		drawTrackStart=blockSel.start.track;
		drawTrackEnd=blockSel.end.track;
		drawLineStart=blockSel.start.line;
		drawLineEnd=blockSel.end.line;
		Repaint(DMDataChange);
	}
}

void CChildView::BlockInsChange(int x)
{
	if ( blockSelected == true ) {

		const int displace=_pSong->playOrder[editPosition]*MULTIPLY2;
		unsigned char *toffset=_pSong->pPatternData+displace;

		for (int t=blockSel.start.track;t<blockSel.end.track+1;t++)
		{
			for (int l=blockSel.start.line;l<blockSel.end.line+1;l++)
			{
				const int displace2=t*5+l*MULTIPLY;
				
				int ins=*(toffset+displace2+1);
			
				if (ins != 255 )
				{
					ins=x;
					if(ins<0)ins=0;
					if(ins>255)ins=255;
					*(toffset+displace2+1)=unsigned char(ins);
				}
			}
		}
		drawTrackStart=blockSel.start.track;
		drawTrackEnd=blockSel.end.track;
		drawLineStart=blockSel.start.line;
		drawLineEnd=blockSel.end.line;
		Repaint(DMDataChange);
	}
}

void CChildView::BlockParamInterpolate()
{
	if (blockSelected)
	{
	///////////////////////////////////////////////////////// Add ROW
		const int displace=_pSong->playOrder[editPosition]*MULTIPLY2;
		unsigned char *toffset=_pSong->pPatternData+displace;
		
		const int initvalue = 
			*(toffset+blockSel.start.track*5+blockSel.start.line*MULTIPLY+3) * 0x100 +
			*(toffset+blockSel.start.track*5+blockSel.start.line*MULTIPLY+4);
		const int endvalue =
			*(toffset+blockSel.start.track*5+blockSel.end.line*MULTIPLY+3) * 0x100 +
			*(toffset+blockSel.start.track*5+blockSel.end.line*MULTIPLY+4);
		const float addvalue = (float)(endvalue -initvalue)/(blockSel.end.line-blockSel.start.line);
		const unsigned char comd = *(toffset+blockSel.start.track*5+blockSel.start.line*MULTIPLY+3);

		for (int l=blockSel.start.line+1;l<blockSel.end.line;l++)
		{
			const int displace2=blockSel.start.track*5+l*MULTIPLY;
			
			const int val=(int)(initvalue+addvalue*(l-blockSel.start.line));
			*(toffset+displace2+3)=unsigned char(val/0x100);
			*(toffset+displace2+4)=unsigned char(val%0x100);
		}
		drawTrackStart=blockSel.start.track;
		drawTrackEnd=blockSel.end.track;
		drawLineStart=blockSel.start.line;
		drawLineEnd=blockSel.end.line;
		Repaint(DMDataChange);
	}
}



void CChildView::IncCurPattern()
{
	if(_pSong->playOrder[editPosition]<(MAX_PATTERNS-1))
	{
		++_pSong->playOrder[editPosition];
		pParentMain->UpdatePlayOrder(true);
		Repaint(DMPatternChange);
	}
}


void CChildView::DecCurPattern()
{
	if(_pSong->playOrder[editPosition]>0)
	{
		--_pSong->playOrder[editPosition];
		pParentMain->UpdatePlayOrder(true);
		Repaint(DMPatternChange);
	}
}

void CChildView::DecPosition()
{
	if(editPosition>0)
	{
		--editPosition;
		
		memset(_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
		_pSong->playOrderSel[editPosition]=true;

		pParentMain->UpdatePlayOrder(false);
		Repaint(DMPatternChange);
	}
}

void CChildView::IncPosition()
{
	if(editPosition<(MAX_SONG_POSITIONS-1))
	{
		++editPosition;
		if ( editPosition >= _pSong->playLength )
		{
			int const ep=_pSong->GetNumPatternsUsed();
			_pSong->playLength=editPosition+1;
			_pSong->playOrder[editPosition]=ep;
		}

		memset(_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
		_pSong->playOrderSel[editPosition]=true;

		pParentMain->UpdatePlayOrder(false);
		Repaint(DMPatternChange);
	}
}

void CChildView::SelectMachineUnderCursor()
{
	const int displace=_pSong->playOrder[editPosition]*MULTIPLY2;
	const int displace2 = editcur.track*5+editcur.line*MULTIPLY;
	unsigned char *toffset=_pSong->pPatternData+displace+displace2;

	PatternEntry *entry = (PatternEntry*) toffset;

	if ( entry->_mach < MAX_BUSES*2 ) _pSong->seqBus = entry->_mach;
	pParentMain->UpdateComboGen();
	if ( entry->_inst != 255 ) _pSong->auxcolSelected = entry->_inst;
	pParentMain->UpdateComboIns();

}
