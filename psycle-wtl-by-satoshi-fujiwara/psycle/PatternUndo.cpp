/** @file 
 *  @brief implementation of the Pattern Undo class
 *  @author S.F.
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.2 $
 */
#include "stdafx.h"
// STL
#include <algorithm>
#include <cctype>
#include <boost/format.hpp>
#include <sstream>

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#include "resource.h"
#define rdtsc __asm __emit 0fh __asm __emit 031h
#define cpuid __asm __emit 0fh __asm __emit 0a2h
#include <memory>
#include "sfhelper.h"
#include "UndoRedo.h"
#include "NewPatternView.h"
#include "PatternUndo.h"
#include "Registry.h"
#include "configuration.h"
#include "FileXM.h"
//#include "FileIT.h"
//#include "ChildView.h"
#include "Bitmap.cpp"

//#include "Dsp.h"
//#include "Filter.h"
#include "MenuXP.h"

PatternUndo::PatternUndo
(DefaultPatternView *ptrView,const int pattern,const int x,const int y,const int tracks,
	const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter)
{
	// fill data
	unsigned char* ptrData = new unsigned char[tracks * lines * EVENT_SIZE * sizeof(char)];
	
	Song& _Song(const_cast<Song&>(Controller::Instance().Song()));

	int _tracks = tracks;

	if (tracks + x > _Song.SongTracks())
	{
		_tracks = _Song.SongTracks() - x;
	}

	const int nl = _Song.PatternLines(pattern);
	
	int _lines = lines;

	if (lines + y > nl)
	{
		_lines = nl - y;
	}
//		PatternUndo* pNew = new PatternUndo
//		(this,pData,pattern,x,y,tracks,lines,edittrack,editline,editcol,seqpos,counter);
	pView(ptrView);
	Pattern(pattern);
	pData(ptrData);
	X(x);
	Y(y);
	Tracks(_tracks);
	Lines(_lines);
	EditTrack(editTrack);
	EditLine(editLine);
	EditCol(editCol);
	SeqPos(seqPos);
	Counter(counter);

	for (int t = x; t < x + _tracks; t++)
	{
		for (int l = y; l < y + _lines; l++)
		{
			unsigned char *offset_source = ptrView->_ptrackline(pattern,t,l);
			memcpy(ptrData,offset_source,EVENT_SIZE);
			ptrData += EVENT_SIZE;
		}
	}
//SetTitleBarText();
}

void PatternUndo::Execute()
{
	DefaultPatternView * _pview = const_cast<DefaultPatternView*>(pView());

	// TODO: AddRedo ‚ð View‘¤‚ÉŽ‚Á‚Ä‚¢‚­
	_pview->AddRedo(
		new PatternUndo(const_cast<DefaultPatternView*>(pView()),
		Pattern(),X(),Y(),Tracks(),	Lines(),
		_pview->EditCursor().Track(),
		_pview->EditCursor().Line(),
		_pview->EditCursor().Column(),
		SeqPos(),
		Counter())
	);
		// do undo
	unsigned char* _pData = const_cast<BYTE*>(pData());

		for (int t = X();t < X() + Tracks(); t++)
		{
			for (int l = Y();l < Y() + Lines(); l++)
			{
				unsigned char *offset_source = _pview->_ptrackline(Pattern(),t,l);
				memcpy(offset_source,_pData,EVENT_SIZE * sizeof(char));
				_pData += EVENT_SIZE * sizeof(char);
			}
		}
		// set up cursor
		_pview->EditCursor().Track(EditTrack());
		_pview->EditCursor().Line(EditLine());
		_pview->EditCursor().Column(EditCol());

		if (SeqPos() == _pview->EditPosition())
		{
			// display changes
			_pview->NewPatternDraw(X(),X() + Tracks(),Y(),Y() + Lines());
			_pview->Repaint(DMData);
		}
		else
		{
			_pview->EditPosition(SeqPos());
			Controller::Instance().UpdatePlayOrder(true);
			_pview->Repaint(DMPattern);
		}

		this->UndoController()->PopUndo();
		delete this;
}

PatternRedo::PatternRedo
(DefaultPatternView *ptrView,const int pattern,const int x,const int y,const int tracks,
	const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter)
	: PatternUndo(ptrView,pattern,x,y,tracks,lines,editTrack,editLine,editCol,seqPos,counter)
{
	;
}

void PatternRedo::Execute()
{
	DefaultPatternView& _pview(*(const_cast<DefaultPatternView*>(pView())));
	CCursor& _editcur(_pview.EditCursor());

	_pview.AddUndo(
		new PatternUndo(const_cast<DefaultPatternView*>(pView()),
			Pattern(),X(),Y(),Tracks(),Lines(),
			_editcur.Track(),
			_editcur.Line(),
			_editcur.Column(),
			SeqPos(),
			Counter()
		),
		false
	);

	//AddUndo(pRedoList->pattern,pRedoList->x,pRedoList->y,pRedoList->tracks,pRedoList->lines,EditCursor.track,EditCursor.line,EditCursor.col,pRedoList->seqpos,FALSE,pRedoList->counter);
	// do redo
	unsigned char* pData = m_pData;

	for (int t = X(); t < X() + Tracks(); t++)
	{
		for (int l = Y(); l < Y() + Lines(); l++)
		{
			unsigned char *offset_source = _pview._ptrackline(Pattern(),t,l);
			memcpy(offset_source,pData,EVENT_SIZE * sizeof(char));
			pData += EVENT_SIZE * sizeof(char);
		}
	}

	// set up cursor
	_editcur.Track(EditTrack());
	_editcur.Line(EditLine());
	_editcur.Column(EditCol());

	if (SeqPos() == _pview.EditPosition())
	{
		// display changes
		_pview.NewPatternDraw(X(),X() + Tracks(),Y(),Y() + Lines());
		_pview.Repaint(DMData);
	}
	else
	{
		_pview.EditPosition(SeqPos());
		Controller::Instance().UpdatePlayOrder(true);
		_pview.Repaint(DMPattern);
		
	}
	
	UndoController()->PopRedo();
	
	delete this;
}

// ------------------------------------------------
// LengthUndo Class
// ------------------------------------------------

LengthUndo::LengthUndo(
DefaultPatternView *ptrView,const int pattern,
			const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter)
{
	pView(ptrView);
	Pattern(pattern);
	pData(NULL);
	X(0);
	Y(0);
	Tracks(0);
	Lines(lines);
	EditTrack(editTrack);
	EditLine(editLine);
	EditCol(editCol);
	SeqPos(seqPos);
	Counter(counter);
	// TODO: SetTitleBarText ‚ð‚Ç‚¤‚·‚é‚©Œˆ‚ß‚é
	//SetTitleBarText();
}

void LengthUndo::Execute()
{	
	DefaultPatternView& _view(*m_pView);
	CCursor& _editcur(_view.EditCursor());
	Song& _song(Controller::Instance().Song());
	
	_view.AddRedo(
		new LengthRedo(m_pView,Pattern(), //_view.PatternLines(Pattern()),
			_song.PatternLines(Pattern()),
			_editcur.Track(),
			_editcur.Line(),
			_editcur.Column(),
			SeqPos(),
			Counter())
	);

	// do undo
	(const_cast<Song&>(_view.Song())).PatternLines(m_Pattern,m_Lines);
	
	// set up cursor
	_view.EditCursor().Track(m_EditTrack);
	_view.EditCursor().Line(m_EditLine);
	_view.EditCursor().Column(m_EditCol);

	if (SeqPos() != _view.EditPosition())
	{
		_view.EditPosition(SeqPos());
		Controller::Instance().UpdatePlayOrder(true);
	}
	// display changes
	_view.Repaint(DMPattern);
	this->UndoController()->PopUndo();
	delete this;
}

// ------------------------------------------------
// LengthRedo Class
// ------------------------------------------------

LengthRedo::LengthRedo(
DefaultPatternView *ptrView,const int pattern,
			const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter)
	: LengthUndo(ptrView,pattern,lines,editTrack,editLine,editCol,seqPos,counter)
{
	;
//SetTitleBarText();
}

void LengthRedo::Execute()
{
	DefaultPatternView& _view(*m_pView);
	CCursor& _editcur(_view.EditCursor());

	_view.AddUndo(
		new LengthUndo(m_pView,Pattern(),
			_view.Song().PatternLines(Pattern()),
			_editcur.Track(),
			_editcur.Line(),
			_editcur.Column(),
			SeqPos(),
			Counter()
		),
		false
	);

	// do undo
	_view.Song().PatternLines(Pattern(),Lines());
	// set up cursor
	_editcur.Track(EditTrack());
	_editcur.Line(EditLine());
	_editcur.Column(EditCol());

	if (SeqPos() != _view.EditPosition())
	{
		 _view.EditPosition(SeqPos());
		 Controller::Instance().UpdatePlayOrder(true);
	}

	// display changes
	_view.Repaint(DMPattern);
	UndoController()->PopRedo();
	delete this;
}

// ------------------------------------------------
// SequencerUndo Class
// ------------------------------------------------

SequencerUndo::SequencerUndo(
DefaultPatternView *ptrView,
			const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter)
{
	DefaultPatternView& _view(*ptrView);
	CCursor& _editcur(_view.EditCursor());
	Song _song(Controller::Instance().Song());

	// fill data
	// count used patterns
	pData(new BYTE[MAX_SONG_POSITIONS]);

	BYTE *_pdata = const_cast<BYTE*>(pData());

	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		*(_pdata + i)  = _song.PlayOrder(i);
	}

	pView(ptrView);
	Pattern(0);
	pData(NULL);
	X(0);
	Y(0);
	Tracks(0);
	Lines(lines);
	EditTrack(editTrack);
	EditLine(editLine);
	EditCol(editCol);
	SeqPos(seqPos);
	Counter(counter);
	// TODO: SetTitleBarText ‚ð‚Ç‚¤‚·‚é‚©Œˆ‚ß‚é
	//SetTitleBarText();
}

void SequencerUndo::Execute()
{
	DefaultPatternView& _view(*m_pView);
	CCursor& _editcur(_view.EditCursor());
	Song _song(Controller::Instance().Song());

	_view.AddRedo(
		new SequencerRedo(m_pView,
			_song.PlayLength(),
			_editcur.Track(),
			_editcur.Line(),
			_editcur.Column(),
			_view.EditPosition(),
			Counter()
		)
	);
	
	// do undo
	for(int i = 0; i <  MAX_SONG_POSITIONS; i++)
	{
		_song.PlayOrder(i,*(pData() + i));
	}

	//memcpy(m_pSong->PlayOrder(), pUndoList->pData, MAX_SONG_POSITIONS * sizeof(char));
	_song.PlayLength(Lines());
	// set up cursor
	_editcur.Track(EditTrack());
	_editcur.Line(EditLine());
	_editcur.Column(EditCol());
	_view.EditPosition(SeqPos());

	Controller::Instance().UpdatePlayOrder(true);
	Controller::Instance().UpdateSequencer();

//	m_pMainFrame->UpdateSequencer();

	// display changes
	_view.Repaint(DMPattern);

	UndoController()->PopUndo();
	delete this;
}

// ------------------------------------------------
// SequencerRedo Class 
// ------------------------------------------------
SequencerRedo::SequencerRedo(
DefaultPatternView *ptrView,
const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter) :
 SequencerUndo(ptrView,lines,editTrack,editLine,editCol,seqPos,counter)
{

}

void SequencerRedo::Execute(){

	DefaultPatternView& _view(*m_pView);
	CCursor& _editcur(_view.EditCursor());
	Song _song(Controller::Instance().Song());

	_view.AddUndo(
		new SequencerUndo(m_pView,
			_song.PlayLength(),
			_editcur.Track(),
			_editcur.Line(),
			_editcur.Column(),
			_view.EditPosition(),
			Counter()
		),
		false
	);

	for(int i = 0; i < MAX_SONG_POSITIONS; i++){
		_song.PlayOrder(i,*(pData() + i));
	}

	//memcpy(m_pSong->m_PlayOrder, pRedoList->pData, MAX_SONG_POSITIONS*sizeof(char));
	_song.PlayLength(Lines());
	// set up cursor
	_editcur.Track(EditTrack());
	_editcur.Line(EditLine());
	_editcur.Column(EditCol());
	_view.EditPosition(SeqPos());

	Controller::Instance().UpdatePlayOrder(true);
	Controller::Instance().UpdateSequencer();

	_view.Repaint(DMPattern);
	UndoController()->PopRedo();
	delete this;
}



// ------------------------------------------------
// SongUndo Class 
// ------------------------------------------------

SongUndo::SongUndo(DefaultPatternView *ptrView,const int editTrack, const int editLine, const int editCol, const int seqPos, const int counter)
{
	DefaultPatternView& _view(*ptrView);
	CCursor& _editcur(_view.EditCursor());
	Song _song(Controller::Instance().Song());


	unsigned char count = 0;
	for (unsigned char i = 0; i < MAX_PATTERNS; i++)
	{
		if (_song.pPatternData(i))
		{
			count++;
		}
	}

	pData(new BYTE[MAX_SONG_POSITIONS + sizeof(count) + MAX_PATTERNS + count * MULTIPLY2]);
	unsigned char *pWrite = const_cast<BYTE *>(pData());

	for(int i = 0;i < MAX_SONG_POSITIONS;i++){
		*(pWrite + i) = _song.PlayOrder(i);
	}
	//memcpy(pWrite, m_pSong->m_PlayOrder, MAX_SONG_POSITIONS*sizeof(char));
	pWrite += MAX_SONG_POSITIONS * sizeof(BYTE);

	memcpy(pWrite, &count, sizeof(count));
	
	pWrite += sizeof(count);

	for (i = 0; i < MAX_PATTERNS; i++)
	{
		if (_song.pPatternData(i))
		{
			memcpy(pWrite, &i, sizeof(i));
			pWrite += sizeof(i);
			
			memcpy(pWrite, _song.pPatternData(i), MULTIPLY2);
			pWrite += MULTIPLY2;
		}
	}

	pView(ptrView);
	Pattern(0);
	pData(NULL);
	X(0);
	Y(0);
	Tracks(0);
	Lines(0);
	EditTrack(editTrack);
	EditLine(editLine);
	EditCol(editCol);
	SeqPos(seqPos);
	Counter(counter);

}

void SongUndo::Execute()
{
	DefaultPatternView& _view(*m_pView);
	CCursor& _editcur(_view.EditCursor());
	Song _song(Controller::Instance().Song());
	_view.AddRedo(
		new SongRedo(m_pView,
			_editcur.Track(),
			_editcur.Line(),
			_editcur.Column(),
			_view.EditPosition(),
			Counter()
		)
	);

	// do undo
	BYTE * _pdata = const_cast<BYTE*>(pData());
	
	for(int i = 0; i < MAX_SONG_POSITIONS; i++){
		_song.PlayOrder(i,*(_pdata + 1));
	}

	//memcpy(m_pSong->m_PlayOrder, pData, MAX_SONG_POSITIONS*sizeof(char));
	_pdata += MAX_SONG_POSITIONS;
	unsigned char count = *_pdata;
	_pdata += sizeof(count);
	
	for (int i = 0; i < count; i++)
	{
		unsigned char index = *_pdata;
		_pdata += sizeof(index);
		unsigned char* pWrite = _song._ppattern(index);

		memcpy(pWrite,_pdata,MULTIPLY2);
		_pdata += MULTIPLY2;
	}

	_song.PlayLength(Lines());
	// set up cursor
	_editcur.Track(EditTrack());
	_editcur.Line(EditLine());
	_editcur.Column(EditCol());
	_view.EditPosition(SeqPos());

	Controller::Instance().UpdatePlayOrder(true);
	Controller::Instance().UpdateSequencer();
	// display changes
	_view.Repaint(DMPattern);

	UndoController()->PopRedo();
	delete this;				
}

SongRedo::SongRedo(DefaultPatternView *ptrView,const int editTrack, const int editLine, const int editCol, const int seqPos, const int counter)
: SongUndo(ptrView,editTrack,editLine,editCol,seqPos,counter)
{

}

void SongRedo::Execute()
{
	DefaultPatternView& _view(*m_pView);
	CCursor& _editcur(_view.EditCursor());
	Song _song(Controller::Instance().Song());

	_view.AddUndo(
		new SongUndo(m_pView,
			_editcur.Track(),
			_editcur.Line(),
			_editcur.Column(),
			_view.EditPosition(),
			Counter()
		)
	);

	BYTE* _pdata = const_cast<BYTE*>(pData());
	
	for(int i = 0; i < MAX_SONG_POSITIONS; i++)
	{
		_song.PlayOrder(i,*(_pdata + i));
	}

	//memcpy(m_pSong->m_PlayOrder, pData, MAX_SONG_POSITIONS*sizeof(char));
	_pdata += MAX_SONG_POSITIONS;
	unsigned char count = *_pdata;
	_pdata += sizeof(count);
	for (int i = 0; i < count; i++)
	{
		unsigned char index = *_pdata;
		_pdata += sizeof(index);
		unsigned char* pWrite = _song._ppattern(index);

		memcpy(pWrite,_pdata,MULTIPLY2);
		_pdata += MULTIPLY2;
	}
	
	_song.PlayLength(Lines());
	// set up cursor
	_editcur.Track(EditTrack());
	_editcur.Line(EditLine());
	_editcur.Column(EditCol());
	_view.EditPosition(SeqPos());

	Controller::Instance().UpdatePlayOrder(true);
	Controller::Instance().UpdateSequencer();
	// display changes
	_view.Repaint(DMPattern);

	UndoController()->PopRedo();
	delete this;				
}

