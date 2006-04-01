/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
 *   natti@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "sequencerbar.h"
#include "global.h"
#include "song.h"
#include "nlabel.h"
#include "nitem.h"
#include "configuration.h"
#include "patternview.h"
#include "defaultbitmaps.h"
#include "player.h"

#include <napp.h>
#include <nconfig.h>

SequencerBar::SequencerBar()
 : NPanel()
{
  init();
}

SequencerBar::SequencerBar( PatternView * patternView )
{
  init();
  patternView_ = patternView;
}

SequencerBar::~SequencerBar()
{
  delete frBorder;
  delete listLayout;
  delete gridLayout;
}

void SequencerBar::init( )
{
  frBorder = new NFrameBorder();
    frBorder->setOval();
    frBorder->setLineCount(2,4,4);
  setBorder(frBorder);

  listLayout = new NListLayout();
  setLayout(listLayout);
  setWidth(90);

  patternView_ = 0;

  NApp::config()->setSkin(&skin_,"seqbar");

  seqPanel_ = new NPanel();
  seqPanel_->setWidth(90);
  add(seqPanel_);

  NPanel* btnBar = new NPanel();
    gridLayout = new NGridLayout();
    gridLayout->setVgap(5);
    gridLayout->setHgap(5);
    btnBar->setLayout(gridLayout);

    NImage* img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->plus()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "plus.xpm");
    btnBar->add( incshort_     = new NButton(img));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->minus()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "minus.xpm");
    btnBar->add( decshort_     = new NButton( img));


    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->plus1()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "plus1.xpm");
    btnBar->add( inclong_     = new NButton( img));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->minus1()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "minus1.xpm");
    btnBar->add( declong_     = new NButton( img));

    btnBar->add( seqnew_       = new NButton("New"));
    btnBar->add( seqduplicate_ = new NButton("Close"));
    btnBar->add( seqins_       = new NButton("Ins"));
    btnBar->add( seqcut_       = new NButton("Del"));
    btnBar->add( seqcopy_      = new NButton("Cut"));
    btnBar->add( seqpaste_     = new NButton("Copy"));
    btnBar->add( seqdelete_    = new NButton("Paste"));
    btnBar->add( seqclr_       = new NButton("Clear"));
    btnBar->add( seqsrt_       = new NButton("Sort"));

    incshort_->setFlat(false);
    incshort_->clicked.connect(this,&SequencerBar::onIncShort);
    decshort_->setFlat(false);
    decshort_->clicked.connect(this,&SequencerBar::onDecShort);
    inclong_->setFlat(false);
    inclong_->clicked.connect(this,&SequencerBar::onIncLong);
    declong_->setFlat(false);
    declong_->clicked.connect(this,&SequencerBar::onDecLong);
    seqnew_->setFlat(false);
    seqnew_->clicked.connect(this,&SequencerBar::onSeqNew);
    seqduplicate_->setFlat(false);
    seqins_->setFlat(false);
    seqins_->clicked.connect(this,&SequencerBar::onSeqIns);
    seqcut_->setFlat(false);
    seqcopy_->setFlat(false);
    seqpaste_->setFlat(false);
    seqdelete_->setFlat(false);
    seqclr_->setFlat(false);
    seqsrt_->setFlat(false);

    btnBar->setLeft(50);
    btnBar->setHeight(btnBar->preferredHeight());
    btnBar->setWidth(50);
  seqPanel_->add(btnBar);
  seqPanel_->setHeight(btnBar->preferredHeight());


  seqList_ = new NListBox();
    seqList_->setHScrollBarPolicy(nNoneVisible);
    seqList_->setHeight(btnBar->preferredHeight());
    NFontMetrics metrics;
    metrics.setFont(font());
    seqList_->setWidth(metrics.textWidth("11: 11")+20);
    seqList_->itemSelected.connect(this,&SequencerBar::onSelChangeSeqList);
  btnBar->setLeft(seqList_->left()+seqList_->width()+2);
    seqPanel_->setWidth(btnBar->left()+btnBar->width()+2);
  seqPanel_->add(seqList_);
  

  NPanel* lenPanel = new NPanel();
   lenPanel->setLayout(new NFlowLayout(nAlLeft,5,0));
   lenPanel->add( new NLabel("Len"));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->less()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "less.xpm");
    lenPanel->add( declong_     = new NButton( img,40,10));

    lenPanel->add( new NLabel("00"));

    img = new NImage();
    if (Global::pConfig()->iconPath=="") img->setSharedBitmap(&Global::pBitmaps()->more()); else
                                         img->loadFromFile(Global::pConfig()->iconPath+ "more.xpm");
    lenPanel->add( declong_     = new NButton( img,40,10));
   lenPanel->resize();
  add(lenPanel);

  NPanel* lengthPanel = new NPanel();
    lengthPanel->setLayout(new NFlowLayout(nAlLeft,5,0));
    lengthPanel->add(new NLabel("Length"));
    lengthPanel->add(new NLabel("00:07"));
  add(lengthPanel);

  NPanel* checkPanel = new NPanel();
    checkPanel->setLayout(new NListLayout());
    checkPanel->add( follow_                = new NCheckBox("Follow song"));
    checkPanel->add( multichannel_audition_ = new NCheckBox("Multichannel\nAudition"));
    checkPanel->add( record_noteoff_        = new NCheckBox("Record NoteOffs"));
    checkPanel->add( record_tweaks_         = new NCheckBox("Record Tewaks"));
    checkPanel->add( notestoeffects_        = new NCheckBox("Allow Notes\nto Effects"));
    checkPanel->add( movecirsorpaste_       = new NCheckBox("Move Cursor\nWhen Paste"));
  add(checkPanel);

  resize();

  seqList()->itemSelected.connect(this,&SequencerBar::onSelChangeSeqList);
}

void SequencerBar::setPatternView( PatternView * patternView )
{
  patternView_ = patternView;
}

void SequencerBar::updateSequencer()
{
  char buf[16];

  //int top = cc->GetTopIndex();
  seqList_->removeChilds();  // delete all items of the ListBox


  for(int n=0;n<Global::pSong()->playLength;n++)
  {
    sprintf(buf,"%.2X: %.2X",n,Global::pSong()->playOrder[n]);
    seqList_->add(new NItem(buf));
  }

  //cc->SelItemRange(false,0,cc->GetCount()-1);
  for (int i=0; i<MAX_SONG_POSITIONS;i++)
  {
      if ( Global::pSong()->playOrderSel[i]) seqList_->setIndex(i);
  }

}

NListBox * SequencerBar::seqList( )
{
  return seqList_;
}

void SequencerBar::onIncShort( NButtonEvent * ev )
{
  //m_wndView.AddUndoSequence(pSong()->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);

  std::vector<int> sel = seqList_->selIndexList();

  for (std::vector<int>::iterator it = sel.begin(); it < sel.end(); it++) {
    int i = *it;
    if(Global::pSong()->playOrder[i]<(MAX_PATTERNS-1)) {
      Global::pSong()->playOrder[i]++;
    }
  }

  updatePlayOrder(false);
  updateSequencer();
  seqList_->repaint();
  patternView_->repaint();
  //m_wndView.SetFocus();*/
}

void SequencerBar::onIncLong( NButtonEvent * ev )
{
  std::vector<int> sel = seqList_->selIndexList();

  for (std::vector<int>::iterator it = sel.begin(); it < sel.end(); it++) {
    int i = *it;
    if(Global::pSong()->playOrder[i]<(MAX_PATTERNS-16)) {
      Global::pSong()->playOrder[i]+=16;
    }
  }

  updatePlayOrder(false);
  updateSequencer();
  seqList_->repaint();
  patternView_->repaint();
}

void SequencerBar::onDecLong( NButtonEvent * ev )
{
  std::vector<int> sel = seqList_->selIndexList();

  for (std::vector<int>::iterator it = sel.begin(); it < sel.end(); it++) {
    int i = *it;
    if(Global::pSong()->playOrder[i]>=16) {
      Global::pSong()->playOrder[i]-=16;
    }
  }

  updatePlayOrder(false);
  updateSequencer();
  seqList_->repaint();
  patternView_->repaint();
}



void SequencerBar::onDecShort( NButtonEvent * ev )
{
  std::vector<int> sel = seqList_->selIndexList();

  for (std::vector<int>::iterator it = sel.begin(); it < sel.end(); it++) {
    int i = *it;
    if(Global::pSong()->playOrder[i]>0) {
      Global::pSong()->playOrder[i]--;
    }
  }

  updatePlayOrder(false);
  updateSequencer();
  seqList_->repaint();
  patternView_->repaint();
}


void SequencerBar::onSeqNew( NButtonEvent * ev )
{
  int editPosition = 0;

  if (patternView_!=0) {
      patternView_->setEditPosition(patternView_->editPosition()+1);
      editPosition = patternView_->editPosition();
  }

  if(Global::pSong()->playLength<(MAX_SONG_POSITIONS-1)) {
     //m_wndView.AddUndoSequence(pSong()->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);
       ++Global::pSong()->playLength;

       int const pop = editPosition;
       for(int c=(Global::pSong()->playLength-1);c >= pop;c--) {
         Global::pSong()->playOrder[c]=Global::pSong()->playOrder[c-1];
       }
       Global::pSong()->playOrder[editPosition]=Global::pSong()->GetBlankPatternUnused();

       if ( Global::pSong()->playOrder[editPosition]>= MAX_PATTERNS ) {
        Global::pSong()->playOrder[editPosition]=MAX_PATTERNS-1;
      }

      Global::pSong()->AllocNewPattern(Global::pSong()->playOrder[editPosition],"",Global::pConfig()->defaultPatLines,false);

      updatePlayOrder(true);
      updateSequencer();
      seqList_->repaint();
      patternView_->repaint();
   }
//   m_wndView.SetFocus();
}



void SequencerBar::updatePlayOrder(bool mode)
{
/*  CStatic *ll_l=(CStatic *)m_wndSeq.GetDlgItem(IDC_SEQ3);
  CListBox *pls=(CListBox*)m_wndSeq.GetDlgItem(IDC_SEQLIST);
  CStatic *pLength = (CStatic*)m_wndSeq.GetDlgItem(IDC_LENGTH);*/

  int ll = Global::pSong()->playLength;
  char buffer[16];

  // Update Labels

  sprintf(buffer,"%.2X",ll);
  //ll_l->SetWindowText(buffer);

  // take ff and fe commands into account

  float songLength = 0;
  int bpm = Global::pSong()->BeatsPerMin();
  int tpb = Global::pSong()->LinesPerBeat();

  for (int i=0; i <ll; i++)
  {
      int pattern = Global::pSong()->playOrder[i];
      // this should parse each line for ffxx commands if you want it to be truly accurate
      unsigned char* const plineOffset = Global::pSong()->_ppattern(pattern);

      for (int l = 0; l < Global::pSong()->patternLines[pattern]*MULTIPLY; l+=MULTIPLY) {
        for (int t = 0; t < Global::pSong()->SONGTRACKS*EVENT_SIZE; t+=EVENT_SIZE) {
           PatternEntry* pEntry = (PatternEntry*)(plineOffset+l+t);
           switch (pEntry->_cmd) {
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

  sprintf(buffer, "%02d:%02d", (int)(songLength / 60), (int)(songLength) % 60);
  //pLength->SetWindowText(buffer);

  // Update sequencer line

  if (mode) {
    const int ls= patternView_->editPosition();
    const int le=Global::pSong()->playOrder[ls];
//    pls->DeleteString(ls);
    sprintf(buffer,"%.2X: %.2X",ls,le);
    seqList_->insert(new NItem(buffer),ls);
    // Update sequencer selection   
  //  pls->SelItemRange(false,0,pls->GetCount()-1);
    seqList_->setIndex(ls);
    memset(Global::pSong()->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));
    Global::pSong()->playOrderSel[ls] = true;
  } else {
//    pls->SelItemRange(false,0,pls->GetCount()-1);
    for (int i=0;i<MAX_SONG_POSITIONS;i++ )
    {
        if (Global::pSong()->playOrderSel[i]) {
          seqList_->setIndex(i);//pls->SetSel(i,true);
        }
    }
  }
}

void SequencerBar::onSelChangeSeqList( NItemEvent * sender )
{
  int maxitems= seqList_->itemCount();

  int const ep=seqList_->selIndex();

  int const cpid=Global::pSong()->playOrder[patternView_->editPosition()];

  // clear selection to 0

  memset(Global::pSong()->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));

  // set new selection from listbox selection of the SequencerList
  std::vector<int> sel = seqList_->selIndexList();

  for (std::vector<int>::iterator it = sel.begin(); it < sel.end(); it++) {
    int c = *it;
    Global::pSong()->playOrderSel[c]=1;
  }

  if((ep!=patternView_->editPosition())) {
      if ((Global::pPlayer()->_playing) && (Global::pConfig()->_followSong)) {
         bool b = Global::pPlayer()->_playBlock;
         Global::pPlayer()->Start(ep,0);
         Global::pPlayer()->_playBlock = b;
      }
      patternView_->setEditPosition(ep);
      patternView_->setPrevEditPosition(ep);
      updatePlayOrder(false);

      if(cpid!=Global::pSong()->playOrder[ep]) {
        patternView_->repaint();
        if (Global::pPlayer()->_playing) {
          //patternViewm_wndView.Repaint(DMPlayback); ??
        }
      }
  }
  //StatusBarIdle();
  //m_wndView.SetFocus();
}

void SequencerBar::onSeqIns( NButtonEvent * ev )
{
  if(Global::pSong()->playLength<(MAX_SONG_POSITIONS-1)) {
     //m_wndView.AddUndoSequence(_pSong->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);
     ++Global::pSong()->playLength;

     patternView_->setEditPosition(patternView_->editPosition()+1);
     int const pop=patternView_->editPosition();
     for(int c=(Global::pSong()->playLength-1);c>=pop;c--) {
        Global::pSong()->playOrder[c]=Global::pSong()->playOrder[c-1];
     }

     updatePlayOrder(true);
     updateSequencer();
     seqList_->repaint();

     patternView_->repaint();
  }
  //m_wndView.SetFocus();
}




