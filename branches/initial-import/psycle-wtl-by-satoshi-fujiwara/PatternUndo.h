/** @file PaternUndo.h 
 *  @brief interface of the CPsycleWTLView class
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.2 $
 */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class AbstractPatternUndo : public SF::IUndo
{
public:

	const BYTE * pData(){return  m_pData;};
	void pData( BYTE * const p){m_pData = p;};
	
	const int Pattern(){return m_Pattern;};
	void Pattern(const int value){m_Pattern = value;};

	const int X(){ return m_X;};
	void X(const int value){m_X = value;};

	const int Y(){ return m_Y;};
	void Y(const int value){m_Y = value;};

	const int Tracks(){ return m_Tracks;};
	void Tracks(const int value){m_Tracks = value;};

	const int Lines(){ return m_Lines;};
	void Lines(const int value){m_Lines = value;};
	
	const int EditTrack(){ return m_EditTrack;};
	void EditTrack(const int value){m_EditTrack = value;};

	const int EditLine(){ return m_EditLine;};
	void EditLine(const int value){m_EditLine = value;};

	const int EditCol(){ return m_EditCol;};
	void EditCol(const int value){m_EditCol = value;};

	const int SeqPos(){ return m_SeqPos;};
	void SeqPos(const int value){m_SeqPos = value;};

	const int Counter(){ return m_Counter;};
	void Counter(const int value){m_Counter = value;};

	const DefaultPatternView* pView(){return m_pView;};
	void pView(const DefaultPatternView * p){m_pView = const_cast<DefaultPatternView*>(p);};

	void UndoController(SF::UndoController * const p){m_pUndoController = p;};
	SF::UndoController * const UndoController(){return m_pUndoController;};

protected:
	SF::UndoController * m_pUndoController;
//	int type;
//	SPatternUndo* pPrev;
	BYTE* m_pData;
	int m_Pattern;
	int m_X;
	int m_Y;
	int	m_Tracks;
	int	m_Lines;
	// store positional data plz
	int m_EditTrack;
	int m_EditLine;
	int m_EditCol;
	int m_SeqPos;
	// counter for tracking, works like ID
	int m_Counter;

	DefaultPatternView *m_pView;

};

class PatternUndo : public AbstractPatternUndo
{
public:
	explicit PatternUndo(DefaultPatternView *ptrView,const int pattern,const int x,const int y,const int tracks,
			const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter);

	~PatternUndo(){
		if(pData() != NULL){
			delete [] pData();
			pData(NULL);
		}
	};
	virtual void Execute();
};

class PatternRedo : public PatternUndo
{
	explicit PatternRedo(DefaultPatternView *ptrView,const int pattern,const int x,const int y,const int tracks,
			const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter);
	~PatternRedo(){
		if(pData() != NULL){
			delete [] pData();
			pData(NULL);
		}
	};
	virtual void Execute();
};

class LengthUndo : public AbstractPatternUndo
{
public:
	explicit LengthUndo(DefaultPatternView *ptrView,const int pattern,
			const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter);
	~LengthUndo()
	{
		;
	};
	void Execute();
};

class LengthRedo : public LengthUndo
{
	public:
	explicit LengthRedo(DefaultPatternView *ptrView,const int pattern,
			const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter);
	~LengthRedo()
	{
		;
	};
	void Execute();

};

class SequencerUndo : public AbstractPatternUndo
{
public:
	explicit SequencerUndo(DefaultPatternView *ptrView,
			const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter);
	~SequencerUndo(){
		if(pData() != NULL){
			delete [] pData();
			pData(NULL);
		}
	};

	void Execute();
};

class SequencerRedo : public SequencerUndo
{
public:
	explicit SequencerRedo(DefaultPatternView *ptrView,
			const int lines,const int editTrack,const int editLine,const int editCol,const int seqPos,const int counter);
	~SequencerRedo(){
		if(pData() != NULL){
			delete [] pData();
			pData(NULL);
		};
	};

	void Execute();
};

class SongUndo : public AbstractPatternUndo
{
public:
	explicit SongUndo(DefaultPatternView *ptrView,const int editTrack, const int editLine, const int editCol, const int seqPos, const int counter);
	~SongUndo(){
		if(pData() != NULL){
			delete [] pData();
			pData(NULL);
		};
	};

	void Execute();
};

class SongRedo : public SongUndo
{
public:
	explicit SongRedo(DefaultPatternView *ptrView,const int editTrack, const int editLine, const int editCol, const int seqPos, const int counter);
	~SongRedo(){
		if(pData() != NULL){
			delete [] pData();
			pData(NULL);
		};
	};
	void Execute();
};

