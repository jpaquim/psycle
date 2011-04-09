/** @file 
 *  @brief SequenceBar dialog
 *  $Date: 2010-08-15 18:18:35 +0200 (dg., 15 ag 2010) $
 *  $Revision: 9831 $
 */

#include "SequenceBar.hpp"
#include "InputHandler.hpp"
#include "PsycleConfig.hpp"
#include "MainFrm.hpp"
#include "ChildView.hpp"
#include "Song.hpp"
#include "Player.hpp"

namespace psycle{ namespace host{

IMPLEMENT_DYNAMIC(SequenceBar, CDialogBar)

	SequenceBar::SequenceBar()
	{
		seqcopybufferlength = 0;
	}

	SequenceBar::~SequenceBar()
	{
	}

	void SequenceBar::DoDataExchange(CDataExchange* pDX)
	{
		CDialogBar::DoDataExchange(pDX);
		DDX_Control(pDX, IDC_LENGTH, m_duration);
		DDX_Control(pDX, IDC_SEQ3, m_seqLen);
		DDX_Control(pDX, IDC_SEQLIST, m_sequence);
		DDX_Control(pDX, IDC_FOLLOW, m_follow);
		DDX_Control(pDX, IDC_RECORD_NOTEOFF, m_noteoffs);
		DDX_Control(pDX, IDC_RECORD_TWEAKS, m_tweaks);
		DDX_Control(pDX, IDC_SHOWPATTERNAME, m_patNames);
		DDX_Control(pDX, IDC_MULTICHANNEL_AUDITION, m_multiChannel);
		DDX_Control(pDX, IDC_NOTESTOEFFECTS, m_notesToEffects);
		DDX_Control(pDX, IDC_MOVECURSORPASTE, m_moveWhenPaste);
	}

	//Message Maps are defined in CMainFrame, since this isn't a window, but a DialogBar.
	BEGIN_MESSAGE_MAP(SequenceBar, CDialogBar)
		ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
	END_MESSAGE_MAP()

	void SequenceBar::InitializeValues(CMainFrame* frame, CChildView* view, Song* song)
	{
		m_pParentMain = frame;
		m_pWndView = view;
		m_pSong = song;
	}

	// SequenceBar message handlers

	LONG SequenceBar::OnInitDialog ( UINT wParam, LONG lParam)
	{
		BOOL bRet = HandleInitDialog(wParam, lParam);

		if (!UpdateData(FALSE))
		{
		   TRACE0("Warning: UpdateData failed during dialog init.\n");
		}

		//m_sequence.SubclassDlgItem(IDC_SEQLIST, this );

		m_multiChannel.SetCheck(Global::psycleconf().inputHandler().bMultiKey?1:0);
		m_moveWhenPaste.SetCheck(Global::psycleconf().inputHandler().bMoveCursorPaste?1:0);
		m_patNames.SetCheck(Global::psycleconf()._bShowPatternNames?1:0);
		m_noteoffs.SetCheck(Global::psycleconf().inputHandler()._RecordNoteoff?1:0);
		m_tweaks.SetCheck(Global::psycleconf().inputHandler()._RecordTweaks?1:0);
		m_notesToEffects.SetCheck(Global::psycleconf().inputHandler()._notesToEffects?1:0);
		m_follow.SetCheck(Global::psycleconf()._followSong?1:0);

		bplus.LoadMappedBitmap(IDB_PLUS,0);
		bminus.LoadMappedBitmap(IDB_MINUS,0);
		bplusplus.LoadMappedBitmap(IDB_PLUSPLUS,0);
		bminusminus.LoadMappedBitmap(IDB_MINUSMINUS,0);
		bless.LoadMappedBitmap(IDB_LESS,0);
		bmore.LoadMappedBitmap(IDB_MORE,0);

		((CButton*)GetDlgItem(IDC_INCSHORT))->SetBitmap((HBITMAP)bplus);
		((CButton*)GetDlgItem(IDC_INCLONG))->SetBitmap((HBITMAP)bplusplus);
		((CButton*)GetDlgItem(IDC_DECSHORT))->SetBitmap((HBITMAP)bminus);
		((CButton*)GetDlgItem(IDC_DECLONG))->SetBitmap((HBITMAP)bminusminus);
		((CButton*)GetDlgItem(IDC_DECLEN))->SetBitmap((HBITMAP)bless);
		((CButton*)GetDlgItem(IDC_INCLEN))->SetBitmap((HBITMAP)bmore);
		
		UpdatePlayOrder(true);
		UpdateSequencer();

		return bRet;
	}

	void SequenceBar::OnSelchangeSeqlist() 
	{
		int maxitems=m_sequence.GetCount();
		int const ep=m_sequence.GetCurSel();
		if(m_pWndView->editPosition<0) m_pWndView->editPosition = 0; // DAN FIXME
		int const cpid=m_pSong->playOrder[m_pWndView->editPosition];

		memset(m_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
		for (int c=0;c<maxitems;c++) 
		{
			if ( m_sequence.GetSel(c) != 0) m_pSong->playOrderSel[c]=true;
		}
		
		if((ep!=m_pWndView->editPosition))// && ( m_sequence.GetSelCount() == 1))
		{
			if ((Global::pPlayer->_playing) && (Global::psycleconf()._followSong))
			{
				bool b = Global::pPlayer->_playBlock;
				Global::pPlayer->Start(ep,0,false);
				Global::pPlayer->_playBlock = b;
			}
			m_pWndView->editPosition=ep;
			m_pWndView->prevEditPosition=ep;
			UpdatePlayOrder(false);
			
			if(cpid!=m_pSong->playOrder[ep])
			{
				m_pWndView->Repaint(draw_modes::pattern);
				if (Global::pPlayer->_playing) {
					m_pWndView->Repaint(draw_modes::playback);
				}
			}		
		}
		m_pParentMain->StatusBarIdle();
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnDblclkSeqlist() 
	{
		int const ep=m_sequence.GetCurSel();
		if (Global::pPlayer->_playing)
		{
			bool b = Global::pPlayer->_playBlock;
			Global::pPlayer->Start(ep,0);
			Global::pPlayer->_playBlock = b;
		}
		else
		{
			Global::pPlayer->Start(ep,0);
		}
		m_pWndView->editPosition=ep;
		m_pWndView->SetFocus();
	}
	void SequenceBar::OnBnClickedIncshort()
	{
		int indexes[MAX_SONG_POSITIONS];
		m_pWndView->AddUndoSequence(m_pSong->playLength,m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);

		int const num= m_sequence.GetSelCount();
		m_sequence.GetSelItems(MAX_SONG_POSITIONS,indexes);

		for (int i = 0; i < num; i++)
		{
			if(m_pSong->playOrder[indexes[i]]<(MAX_PATTERNS-1))
			{
				m_pSong->playOrder[indexes[i]]++;
			}
		}
		UpdatePlayOrder(false);
		UpdateSequencer();
		((CButton*)GetDlgItem(IDC_INCSHORT))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->Repaint(draw_modes::pattern);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedDecshort()
	{
		int indexes[MAX_SONG_POSITIONS];
		m_pWndView->AddUndoSequence(m_pSong->playLength,m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);

		int const num= m_sequence.GetSelCount();
		m_sequence.GetSelItems(MAX_SONG_POSITIONS,indexes);

		for (int i = 0; i < num; i++)
		{
			if(m_pSong->playOrder[indexes[i]]>0)
			{
				m_pSong->playOrder[indexes[i]]--;
			}
		}
		UpdatePlayOrder(false);
		UpdateSequencer();
		((CButton*)GetDlgItem(IDC_DECSHORT))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->Repaint(draw_modes::pattern);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedInclong()
	{
		int indexes[MAX_SONG_POSITIONS];
		m_pWndView->AddUndoSequence(m_pSong->playLength,m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);

		int const num= m_sequence.GetSelCount();
		m_sequence.GetSelItems(MAX_SONG_POSITIONS,indexes);

		for (int i = 0; i < num; i++)
		{
			if(m_pSong->playOrder[indexes[i]]<(MAX_PATTERNS-16))
			{
				m_pSong->playOrder[indexes[i]]+=16;			
			}
			else
			{
				m_pSong->playOrder[indexes[i]]=(MAX_PATTERNS-1);
			}
		}
		UpdatePlayOrder(false);
		UpdateSequencer();
		((CButton*)GetDlgItem(IDC_INCLONG))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->Repaint(draw_modes::pattern);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedDeclong()
	{
		int indexes[MAX_SONG_POSITIONS];
		m_pWndView->AddUndoSequence(m_pSong->playLength,m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);

		int const num= m_sequence.GetSelCount();
		m_sequence.GetSelItems(MAX_SONG_POSITIONS,indexes);

		for (int i = 0; i < num; i++)
		{
			if(m_pSong->playOrder[indexes[i]]>=16)
			{
				m_pSong->playOrder[indexes[i]]-=16;			
			}
			else
			{
				m_pSong->playOrder[indexes[i]]=0;
			}
		}
		UpdatePlayOrder(false);
		UpdateSequencer();
		((CButton*)GetDlgItem(IDC_DECLONG))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->Repaint(draw_modes::pattern);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedSeqnew()
	{
		if(m_pSong->playLength<(MAX_SONG_POSITIONS-1))
		{
			m_pWndView->AddUndoSequence(m_pSong->playLength,m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);
			++m_pSong->playLength;

			m_pWndView->editPosition++;
			int const pop=m_pWndView->editPosition;
			for(int c=(m_pSong->playLength-1);c>=pop;c--)
			{
				m_pSong->playOrder[c]=m_pSong->playOrder[c-1];
			}
			m_pSong->playOrder[m_pWndView->editPosition]=m_pSong->GetBlankPatternUnused();
			
			if ( m_pSong->playOrder[m_pWndView->editPosition]>= MAX_PATTERNS )
			{
				m_pSong->playOrder[m_pWndView->editPosition]=MAX_PATTERNS-1;
			}

			m_pSong->AllocNewPattern(m_pSong->playOrder[m_pWndView->editPosition],"",
				Global::configuration().GetDefaultPatLines(),FALSE);

			UpdatePlayOrder(true);
			UpdateSequencer(m_pWndView->editPosition);

			m_pWndView->Repaint(draw_modes::pattern);
		}
		((CButton*)GetDlgItem(IDC_SEQNEW))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedSeqduplicate()
	{
		int selcount = m_sequence.GetSelCount();
		if (selcount == 0) return;
		if ( m_pSong->playLength+selcount >= MAX_SONG_POSITIONS)
		{
			MessageBox("Cannot clone the pattern(s). The maximum sequence length would be exceeded.","Clone Patterns");
			m_pWndView->SetFocus();
			return;
		}
		m_pWndView->AddUndoSequence(m_pSong->playLength,m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);
		// Moves all patterns after the selection, to make space.
		int* litems = new int[selcount];
		m_sequence.GetSelItems(selcount,litems);
		for(int i(m_pSong->playLength-1) ; i >= litems[selcount-1] ;--i)
		{
			m_pSong->playOrder[i+selcount]=m_pSong->playOrder[i];
		}
		m_pSong->playLength+=selcount;

		for(int i(0) ; i < selcount ; ++i)
		{
			int newpat = -1;
			// This for loop is in order to clone sequences like: 00 00 01 01 and avoid duplication of same patterns.
			for (int j(0); j < i; ++j)
			{
				if (m_pSong->playOrder[litems[0]+j] == m_pSong->playOrder[litems[0]+i])
				{
					newpat=m_pSong->playOrder[litems[selcount-1]+j+1];
				}
			}
			if (newpat == -1 ) 
			{
				newpat = m_pSong->GetBlankPatternUnused();
				if (newpat < MAX_PATTERNS-1)
				{
					int oldpat = m_pSong->playOrder[litems[i]];
					m_pSong->AllocNewPattern(newpat,m_pSong->patternName[oldpat],m_pSong->patternLines[oldpat],FALSE);
					memcpy(m_pSong->_ppattern(newpat),m_pSong->_ppattern(oldpat),MULTIPLY2);
				}
				else 
				{
					newpat=0;
				}
			}
			m_pSong->playOrder[litems[selcount-1]+i+1]=newpat;
		}
		m_pWndView->editPosition=litems[selcount-1]+1;
		UpdatePlayOrder(true);
		UpdateSequencer(m_pWndView->editPosition);
		((CButton*)GetDlgItem(IDC_SEQDUPLICATE))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->Repaint(draw_modes::pattern);

		delete [] litems; litems = 0;
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedSeqins()
	{
		if(m_pSong->playLength<(MAX_SONG_POSITIONS-1))
		{
			m_pWndView->AddUndoSequence(m_pSong->playLength,m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);
			++m_pSong->playLength;

			m_pWndView->editPosition++;
			int const pop=m_pWndView->editPosition;
			for(int c=(m_pSong->playLength-1);c>=pop;c--)
			{
				m_pSong->playOrder[c]=m_pSong->playOrder[c-1];
			}

			UpdatePlayOrder(true);
			UpdateSequencer(m_pWndView->editPosition);

			m_pWndView->Repaint(draw_modes::pattern);
		}
		((CButton*)GetDlgItem(IDC_SEQINS))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedSeqdelete()
	{
		int indexes[MAX_SONG_POSITIONS];
		m_pWndView->AddUndoSequence(m_pSong->playLength,m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);

		int const num= m_sequence.GetSelCount();
		m_sequence.GetSelItems(MAX_SONG_POSITIONS,indexes);

		// our list can be in any order so we must be careful
		int smallest = indexes[0]; // we need a good place to put the cursor when we are done, above the topmost selection seems most intuitive
		for (int i=0; i < num; i++)
		{
			int c;
			for(c = indexes[i] ; c < m_pSong->playLength - 1 ; ++c)
			{
				m_pSong->playOrder[c]=m_pSong->playOrder[c+1];
			}
			m_pSong->playOrder[c]=0;
			m_pSong->playLength--;
			if (m_pSong->playLength <= 0)
			{
				m_pSong->playLength =1;
			}
			for(int j(i + 1) ; j < num ; ++j)
			{
				if (indexes[j] > indexes[i])
				{
					indexes[j]--;
				}
			}
			if (indexes[i] < smallest)
			{
				smallest = indexes[i];
			}
		}
		m_pWndView->editPosition = smallest-1;

		if (m_pWndView->editPosition<0)
		{
			m_pWndView->editPosition = 0;
		}
		else if (m_pWndView->editPosition>=m_pSong->playLength)
		{
			m_pWndView->editPosition=m_pSong->playLength-1;
		}

		UpdatePlayOrder(true);
		UpdateSequencer(m_pWndView->editPosition);
		((CButton*)GetDlgItem(IDC_SEQDELETE))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->Repaint(draw_modes::pattern);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedSeqcut()
	{
		OnBnClickedSeqcopy();
		OnBnClickedSeqdelete();
		((CButton*)GetDlgItem(IDC_SEQCUT))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
	}

	void SequenceBar::OnBnClickedSeqcopy()
	{
		seqcopybufferlength= m_sequence.GetSelCount();
		m_sequence.GetSelItems(MAX_SONG_POSITIONS,seqcopybuffer);

		// sort our table so we can paste it in a sensible manner later
		for (int i=0; i < seqcopybufferlength; i++)
		{
			for (int j=i+1; j < seqcopybufferlength; j++)
			{
				if (seqcopybuffer[j] < seqcopybuffer[i])
				{
					int k = seqcopybuffer[i];
					seqcopybuffer[i] = seqcopybuffer[j];
					seqcopybuffer[j] = k;
				}
			}
			// convert to actual index
			seqcopybuffer[i] = m_pSong->playOrder[seqcopybuffer[i]];
		}
		((CButton*)GetDlgItem(IDC_SEQCOPY))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedSeqpaste()
	{
		if (seqcopybufferlength > 0)
		{
			if(m_pSong->playLength<(MAX_SONG_POSITIONS-1))
			{
				m_pWndView->AddUndoSequence(m_pSong->playLength,m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);

				// we will do this in a loop to easily handle an error if we run out of space

				// our list can be in any order so we must be careful
				int pastedcount = 0;
				for (int i=0; i < seqcopybufferlength; i++)
				{
					if(m_pSong->playLength<(MAX_SONG_POSITIONS-1))
					{
						++m_pSong->playLength;

						m_pWndView->editPosition++;
						pastedcount++;
						int c;
						for(c = m_pSong->playLength - 1 ; c >= m_pWndView->editPosition ; --c)
						{
							m_pSong->playOrder[c]=m_pSong->playOrder[c-1];
						}
						m_pSong->playOrder[c+1] = seqcopybuffer[i];
					}
				}

				if (pastedcount>0)
				{
					UpdatePlayOrder(true);
					for(int i(m_pWndView->editPosition + 1 - pastedcount) ; i < m_pWndView->editPosition ; ++i)
					{
						m_pSong->playOrderSel[i] = true;
					}
					UpdateSequencer(m_pWndView->editPosition);
					m_pWndView->Repaint(draw_modes::pattern);

				}
			}
		}
		((CButton*)GetDlgItem(IDC_SEQPASTE))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedSeqclr()
	{
		if (MessageBox("Do you really want to clear the sequence and pattern data?","Sequencer",MB_YESNO) == IDYES)
		{
			m_pWndView->AddUndoSong(m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);
			{
				CExclusiveLock lock(&m_pSong->semaphore, 2, true);
				// clear sequence
				for(int c=0;c<MAX_SONG_POSITIONS;c++)
				{
					m_pSong->playOrder[c]=0;
				}
				// clear pattern data
				m_pSong->DeleteAllPatterns();
				// init a pattern for #0
				m_pSong->_ppattern(0);

				m_pWndView->editPosition=0;
				m_pSong->playLength=1;
			}
			UpdatePlayOrder(true);
			UpdateSequencer();
			m_pWndView->Repaint(draw_modes::pattern);
		}
		((CButton*)GetDlgItem(IDC_SEQCLR))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedSeqsrt()
	{
		m_pWndView->AddUndoSong(m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);
		unsigned char oldtonew[MAX_PATTERNS];
		unsigned char newtoold[MAX_PATTERNS];
		memset(oldtonew,255,MAX_PATTERNS*sizeof(char));
		memset(newtoold,255,MAX_PATTERNS*sizeof(char));

		if (Global::pPlayer->_playing)
		{
			Global::pPlayer->Stop();
		}


	// Part one, Read patterns from sequence and assign them a new ordered number.
		unsigned char freep=0;
		for ( int i=0 ; i<m_pSong->playLength ; i++ )
		{
			const unsigned char cp=m_pSong->playOrder[i];
			if ( oldtonew[cp] == 255 ) // else, we have processed it already
			{
				oldtonew[cp]=freep;
				newtoold[freep]=cp;
				freep++;
			}
		}
	// Part one and a half. End filling the order numbers.
		for(int i(0) ; i < MAX_PATTERNS ; ++i)
		{
			if ( oldtonew[i] == 255 )
			{
				oldtonew[i] = freep;
				newtoold[freep] = i;
				freep++;
			}
		}
	// Part two. Sort Patterns. Take first "invalid" out, and start putting patterns in their place.
	//			 When we have to put the first read one back, do it and find next candidate.

		int patl; // first one is initial one, next one is temp one
		char patn[32]; // ""
		unsigned char * pData; // ""


		int idx=0;
		int idx2=0;
		for(int i(0) ; i < MAX_PATTERNS ; ++i)
		{
			if ( newtoold[i] != i ) // check if this place belongs to another pattern
			{
				pData = m_pSong->ppPatternData[i];
				memcpy(&patl,&m_pSong->patternLines[i],sizeof(int));
				memcpy(patn,&m_pSong->patternName[i],sizeof(char)*32);

				idx = i;
				while ( newtoold[idx] != i ) // Start moving patterns while it is not the stored one.
				{
					idx2 = newtoold[idx]; // get pattern that goes here and move.

					m_pSong->ppPatternData[idx] = m_pSong->ppPatternData[idx2];
					memcpy(&m_pSong->patternLines[idx],&m_pSong->patternLines[idx2],sizeof(int));
					memcpy(&m_pSong->patternName[idx],&m_pSong->patternName[idx2],sizeof(char)*32);
					
					newtoold[idx]=idx; // and indicate that this pattern has been corrected.
					idx = idx2;
				}

				// Put pattern back.
				m_pSong->ppPatternData[idx] = pData;
				memcpy(&m_pSong->patternLines[idx],&patl,sizeof(int));
				memcpy(m_pSong->patternName[idx],patn,sizeof(char)*32);

				newtoold[idx]=idx; // and indicate that this pattern has been corrected.
			}
		}
	// Part three. Update the sequence

		for(int i(0) ; i < m_pSong->playLength ; ++i)
		{
			m_pSong->playOrder[i]=oldtonew[m_pSong->playOrder[i]];
		}

	// Part four. All the needed things.

		seqcopybufferlength = 0;
		UpdateSequencer();
		((CButton*)GetDlgItem(IDC_SEQSRT))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->Repaint(draw_modes::pattern);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedDeclen()
	{
		m_pWndView->AddUndoSequence(m_pSong->playLength,m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);
		if(m_pSong->playLength>1)
		{
			--m_pSong->playLength;
			m_pSong->playOrder[m_pSong->playLength]=0;
			UpdatePlayOrder(false);
			UpdateSequencer();
		}
		((CButton*)GetDlgItem(IDC_DECLEN))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedInclen()
	{
		m_pWndView->AddUndoSequence(m_pSong->playLength,m_pWndView->editcur.track,m_pWndView->editcur.line,m_pWndView->editcur.col,m_pWndView->editPosition);
		if(m_pSong->playLength<(MAX_SONG_POSITIONS-1))
		{
			++m_pSong->playLength;
			UpdatePlayOrder(false);
			UpdateSequencer();
		}
		((CButton*)GetDlgItem(IDC_INCLEN))->ModifyStyle(BS_DEFPUSHBUTTON, 0);
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedFollow()
	{
		Global::psycleconf()._followSong = m_follow.GetCheck()?true:false;

		if ( Global::psycleconf()._followSong )
		{
			if  ( Global::pPlayer->_playing )
			{
				m_pWndView->ChordModeOffs = 0;
				m_pWndView->bScrollDetatch=false;
				if (m_sequence.GetCurSel() != Global::pPlayer->_playPosition)
				{
					m_sequence.SelItemRange(false,0,m_sequence.GetCount()-1);
					m_sequence.SetSel(Global::pPlayer->_playPosition,true);
				}
				if ( m_pWndView->editPosition  != Global::pPlayer->_playPosition )
				{
					m_pWndView->editPosition=Global::pPlayer->_playPosition;
					m_pWndView->Repaint(draw_modes::pattern);
				}
				int top = Global::pPlayer->_playPosition - 0xC;
				if (top < 0) top = 0;
				m_sequence.SetTopIndex(top);
			}
			else
			{
				m_sequence.SelItemRange(false,0,m_sequence.GetCount()-1);
				for (int i=0;i<MAX_SONG_POSITIONS;i++ )
				{
					if (m_pSong->playOrderSel[i]) m_sequence.SetSel(i,true);
				}
				int top = m_pWndView->editPosition - 0xC;
				if (top < 0) top = 0;
				m_sequence.SetTopIndex(top);
			}
		}
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedRecordNoteoff()
	{
		if ( m_noteoffs.GetCheck() ) Global::psycleconf().inputHandler()._RecordNoteoff=true;
		else Global::psycleconf().inputHandler()._RecordNoteoff=false;
		m_pWndView->SetFocus();}

	void SequenceBar::OnBnClickedRecordTweaks()
	{
		if ( m_tweaks.GetCheck() ) Global::psycleconf().inputHandler()._RecordTweaks=true;
		else Global::psycleconf().inputHandler()._RecordTweaks=false;
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedShowpattername()
	{
		Global::psycleconf()._bShowPatternNames=m_patNames.GetCheck();
		
		/*

		//trying to set the size of the sequencer bar... how to do this!?

		CRect borders;
		GetWindowRect(&borders);
		TRACE("borders.right = %i", borders.right);
		if (Global::psycleconf()._bShowPatternNames)
		{
		   //SetBorders(borders.left, borders.top, 6, borders.bottom);
		}
		else
		{
			//SetBorders(borders.left, borders.top, 3, borders.bottom);
		}
		*/

		UpdateSequencer();
		m_sequence.SetSel(Global::pPlayer->_playPosition,true);

		int top = ((Global::pPlayer->_playing)?Global::pPlayer->_playPosition:m_pWndView->editPosition) - 0xC;
		if (top < 0) top = 0;
		m_sequence.SetTopIndex(top);
		m_pWndView->SetFocus();
	}
	void SequenceBar::OnBnClickedMultichannelAudition()
	{
		Global::psycleconf().inputHandler().bMultiKey = !Global::psycleconf().inputHandler().bMultiKey;
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedNotestoeffects()
	{
		if ( m_notesToEffects.GetCheck() ) Global::psycleconf().inputHandler()._notesToEffects=true;
		else Global::psycleconf().inputHandler()._notesToEffects=false;
		m_pWndView->SetFocus();
	}

	void SequenceBar::OnBnClickedMovecursorpaste()
	{
		Global::psycleconf().inputHandler().bMoveCursorPaste = !Global::psycleconf().inputHandler().bMoveCursorPaste;
		m_pWndView->SetFocus();
	}

	void SequenceBar::UpdateSequencer(int selectedpos)
	{
		char buf[16];

		int top = m_sequence.GetTopIndex();
		m_sequence.ResetContent();
		
		if (Global::psycleconf()._bShowPatternNames)
		{
			for(int n=0;n<m_pSong->playLength;n++)
			{
				sprintf(buf,"%.2X:%s",n,m_pSong->patternName[m_pSong->playOrder[n]]);
				m_sequence.AddString(buf);
			}
		}
		else
		{
			for(int n=0;n<m_pSong->playLength;n++)
			{
				sprintf(buf,"%.2X: %.2X",n,m_pSong->playOrder[n]);
				m_sequence.AddString(buf);
			}
		}
		
		m_sequence.SelItemRange(false,0,m_sequence.GetCount()-1);
		for (int i=0; i<MAX_SONG_POSITIONS;i++)
		{
			if ( m_pSong->playOrderSel[i]) m_sequence.SetSel(i,true);
		}
		if (selectedpos >= 0)
		{
			m_sequence.SetSel(selectedpos);
			top = selectedpos - 0xC;
			if (top < 0) top = 0;
		}
		m_sequence.SetTopIndex(top);
		m_pParentMain->StatusBarIdle();
	}




	void SequenceBar::UpdatePlayOrder(bool mode)
	{

		int ll = m_pSong->playLength;

		char buffer[16];

	// Update Labels
		
		sprintf(buffer,"%.2X",ll);
		m_seqLen.SetWindowText(buffer);

		// take ff and fe commands into account

		float songLength = 0;
		int bpm = m_pSong->BeatsPerMin();
		int tpb = m_pSong->LinesPerBeat();
		for (int i=0; i <ll; i++)
		{
			int pattern = m_pSong->playOrder[i];
			// this should parse each line for ffxx commands if you want it to be truly accurate
			unsigned char* const plineOffset = m_pSong->_ppattern(pattern);
			for (int l = 0; l < m_pSong->patternLines[pattern]*MULTIPLY; l+=MULTIPLY)
			{
				for (int t = 0; t < m_pSong->SONGTRACKS*EVENT_SIZE; t+=EVENT_SIZE)
				{
					PatternEntry* pEntry = (PatternEntry*)(plineOffset+l+t);
					switch (pEntry->_cmd)
					{
					case 0xFF:
						if ( pEntry->_parameter != 0 && pEntry->_note < 121 || pEntry->_note == 255)
						{
							bpm=pEntry->_parameter;//+0x20; // ***** proposed change to ffxx command to allow more useable range since the tempo bar only uses this range anyway...
						}
						break;
						
					case 0xFE:
						if ( pEntry->_parameter != 0 && pEntry->_note < 121 || pEntry->_note == 255)
						{
							tpb=pEntry->_parameter;
						}
						break;
					}
				}
				songLength += (60.0f/(bpm * tpb));
			}
		}
		
		sprintf(buffer, "%02d:%02d", ((int)songLength) / 60, ((int)songLength) % 60);
		m_duration.SetWindowText(buffer);
		
		// Update sequencer line
		
		if (mode)
		{
			const int ls=m_pWndView->editPosition;
			const int le=m_pSong->playOrder[ls];
			m_sequence.DeleteString(ls);

			if (Global::psycleconf()._bShowPatternNames)
				sprintf(buffer,"%.2X:%s",ls,m_pSong->patternName[le]);
			else
				sprintf(buffer,"%.2X: %.2X",ls,le);
			m_sequence.InsertString(ls,buffer);
			// Update sequencer selection	
			m_sequence.SelItemRange(false,0,m_sequence.GetCount()-1);
			m_sequence.SetSel(ls,true);
			int top = ls - 0xC;
			if (top < 0) top = 0;
			m_sequence.SetTopIndex(top);
			memset(m_pSong->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
			m_pSong->playOrderSel[ls] = true;
		}
		else
		{
			int top = m_sequence.GetTopIndex();
			m_sequence.SelItemRange(false,0,m_sequence.GetCount()-1);
			for (int i=0;i<MAX_SONG_POSITIONS;i++ )
			{
				if (m_pSong->playOrderSel[i]) m_sequence.SetSel(i,true);
			}
			m_sequence.SetTopIndex(top);
		}
	}

}}
