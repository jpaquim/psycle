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
#include "ngridlayout.h"
#include "configuration.h"
#include "patternview.h"
#include "nframeborder.h"

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
}

void SequencerBar::init( )
{
  NFrameBorder* br = new NFrameBorder();
    br->setOval();
    br->setLineCount(2,4,4);
    
  setBorder(br);

  setLayout(new NListLayout());
  setWidth(90);

  patternView_ = 0;

  NApp::config()->setSkin(&skin_,"seqbar");

  seqPanel_ = new NPanel();
  seqPanel_->setWidth(90);
  add(seqPanel_);

  NPanel* btnBar = new NPanel();
    NGridLayout* gr = new NGridLayout();
    gr->setVgap(5);
    gr->setHgap(5);
    btnBar->setLayout(gr);
     btnBar->add( incshort_     = new NButton( new NImage(Global::pConfig()->iconPath+"plus.xpm")));
     btnBar->add( decshort_     = new NButton( new NImage(Global::pConfig()->iconPath+"minus.xpm")));
     btnBar->add( inclong_      = new NButton( new NImage(Global::pConfig()->iconPath+"plus1.xpm")));
     btnBar->add( declong_      = new NButton( new NImage(Global::pConfig()->iconPath+"minus1.xpm")));
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
    inclong_->setFlat(false);
    declong_->setFlat(false);
    seqnew_->setFlat(false);
    seqnew_->clicked.connect(this,&SequencerBar::onSeqNew);
    seqduplicate_->setFlat(false);
    seqins_->setFlat(false);
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
  btnBar->setLeft(seqList_->left()+seqList_->width()+2);
  seqPanel_->setWidth(btnBar->left()+btnBar->width()+2);
  seqPanel_->add(seqList_);
  

  NPanel* lenPanel = new NPanel();
   lenPanel->setLayout(new NFlowLayout(nAlLeft,5,0));
   lenPanel->add( new NLabel("Len"));
   lenPanel->add( declen_ = new NButton( new NImage(Global::pConfig()->iconPath+"less.xpm"),40,10) );
   lenPanel->add( new NLabel("00"));
    lenPanel->add( inclem_ = new NButton( new NImage(Global::pConfig()->iconPath+"more.xpm"),40,10) );
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

void SequencerBar::updateSequencer( )
{
  seqList_->removeChilds();

  char buf[20];
  for(int n=0;n<Global::pSong()->playLength;n++)
  {
     sprintf(buf,"%.2X: %.2X",n,Global::pSong()->playOrder[n]);
     seqList_->add(new NItem(buf));
  }
}

NListBox * SequencerBar::seqList( )
{
  return seqList_;
}

void SequencerBar::onIncShort( NButtonEvent * ev )
{
  //m_wndView.AddUndoSequence(pSong()->playLength,m_wndView.editcur.track,m_wndView.editcur.line,m_wndView.editcur.col,m_wndView.editPosition);

  int const num = seqList_->selIndex();
  std::vector<int> indexes = seqList_->selIndexList();

  for (int i = 0; i < num; i++) {
    if(Global::pSong()->playOrder[indexes[i]]<(MAX_PATTERNS-1)) {
      Global::pSong()->playOrder[indexes[i]]++;
    }
  }
  updateSequencer();
  seqList_->repaint();
  //UpdatePlayOrder(false);
  //m_wndView.Repaint(DMPattern);
  //m_wndView.SetFocus();*/
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

   //  UpdatePlayOrder(true);
      updateSequencer();
      seqList_->repaint();
   //  m_wndView.Repaint(DMPattern);
   }
//   m_wndView.SetFocus();
}

void SequencerBar::onSelChangeSeqList( NItemEvent * sender )
{
  if (patternView_!=0) {
    int maxitems = seqList()->itemCount();
    int const ep = seqList()->selIndex();

    memset(Global::pSong()->playOrderSel,0,MAX_SONG_POSITIONS*sizeof(bool));

    if((ep!=patternView_->editPosition()))// && ( cc->GetSelCount() == 1))
    {
      patternView_->setEditPosition(ep);
      patternView_->repaint();
    }
  }
}

void SequencerBar::updatePlayOrder(bool mode)
{
 // todo
}


