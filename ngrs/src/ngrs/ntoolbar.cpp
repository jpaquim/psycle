/***************************************************************************
 *   Copyright (C) 2005, 2006 by Stefan Nattkemper   *
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
#include "ntoolbar.h"
#include "nbevelborder.h"
#include "napp.h"
#include "nconfig.h"
#include "nbutton.h"
#include "npopupmenu.h"
#include "nlabel.h"

/* XPM */
const char * icon_more_xpm[] = {
"11 11 3 1",
" 	c #FFFFFF",
".	c None",
"*	c #000001",
"...........",
"*******....",
".  *.  *...",
"*.  *.  *..",
".*.  *.  *.",
"..*.  *.  *",
".*.  *.  *.",
"*.  *.  *..",
".  *.  *...",
"*******....",
"..........."};

NToolBar::NToolBar()
 : NTogglePanel()
{
  spacer_ = new NPanel();
    spacer_->setWidth(5);
    spacer_->setTransparent(true);
  add(spacer_);
  
  NBitmap bmp( icon_more_xpm );
  NImage * img = new NImage( bmp );
  img->setPreferredSize( bmp.width() + 5, bmp.height() + 5 );  
  moreBtn_ = new NButton( img );
    moreBtn_->clicked.connect( this, &NToolBar::onMoreBtnClicked );
  add( moreBtn_ );
  
  popup_ = new NPopupWindow();   
  add( popup_ );

  skin_ = NApp::config()->skin("toolbar");
}


NToolBar::~NToolBar()
{
}

void NToolBar::paint( NGraphics * g )
{
  g->fillGradient( 0, 0, 5, clientHeight(), NColor( background().red() + 50, background().green() + 50, background().blue() +50 ), NColor(background().red() + 20,background().green() + 20, background().blue()+20),nVertical );
  g->fillGradient( 5, 0, 4, clientHeight(),NColor( background().red() + 20, background().green() + 20, background().blue() +20 ), NColor(background().red() - 40,background().green() - 40, background().blue()-40),nVertical);
}

void NToolBar::add( NRuntime * component )
{
  NPanel::add(component);
}

NButton* NToolBar::add( NButton * button )
{
  NTogglePanel::add( button );
  return button;
}

void NToolBar::add( NVisualComponent * comp )
{
  NPanel::add(comp);
}

void NToolBar::doAlign() {

  int hgap_ = 3;
  int vgap_ = 0;
  
  int xp = hgap_;
  int yp = vgap_;
  int ymax = 2*vgap_;
  bool moreFlag = false;
  
  std::vector<NVisualComponent*> swapVisuals;
  std::vector<NVisualComponent*>::iterator itr; 
  for ( itr = popup_->pane()->begin(); itr != popup_->pane()->end(); itr++ ) {
    swapVisuals.push_back( *itr );
  }
  
  popup_->pane()->erase( popup_->pane()->begin(), popup_->pane()->end() );
     
  for ( itr = swapVisuals.begin(); itr != swapVisuals.end(); itr++ ) {
    add( *itr ); 
  }
  swapVisuals.clear();

  std::vector<NVisualComponent*>::iterator moreItr = end();
  for ( itr = begin(); itr < end(); itr++ ) {
    NVisualComponent* visualChild = *itr;
    if ( visualChild->visible() && visualChild != moreBtn_ ) {   
      if (xp + visualChild->preferredWidth() <= clientWidth() ) 
      {
          visualChild->setPosition( xp, yp, visualChild->preferredWidth(), visualChild->preferredHeight() );
          xp = xp + visualChild->preferredWidth() + hgap_;
          if ( ymax < visualChild->preferredHeight() ) ymax = visualChild->preferredHeight();
      } else
      {       
        moreItr = itr;
        moreFlag = true;
        break;
      }
    }      
  }

  for ( itr = begin(); itr < end() ; itr++ ) {
    NVisualComponent* visual = *itr;
    if ( visual-> visible() ) {    
      visual->setTop( yp + ( ymax - visual->preferredHeight() ) / 2 );    
    }
  }
  
  if ( moreFlag ) {
    moreBtn_->setLeft( clientWidth() - moreBtn_->preferredWidth() - hgap_ );
    moreBtn_->setWidth( moreBtn_->preferredWidth() );
    moreBtn_->setHeight( moreBtn_->preferredHeight() );
    moreBtn_->setVisible( true );
      
    for ( itr = moreItr; itr != end(); itr++ ) {
       swapVisuals.push_back( *itr );
    }
    erase( moreItr, end() );
    
    popup_->pane()->erase( popup_->pane()->begin(), popup_->pane()->end() );
    for ( itr = swapVisuals.begin(); itr != swapVisuals.end(); itr++ ) {
      popup_->pane()->add( *itr, nAlTop );
    }
    
  } else 
  moreBtn_->setVisible( false );
}     

void NToolBar::resize( )
{
  NTogglePanel::resize();
  doAlign();
}

int NToolBar::preferredWidth( ) const
{
  int hgap_ = 3;
  int xp = hgap_;

  std::vector<NVisualComponent*>::const_iterator itr   = visualComponents().begin();  
  for ( ; itr < visualComponents().end(); itr++) {
    NVisualComponent* visualChild = *itr;
    if ( visualChild->visible() ) {      
      xp = xp + visualChild->preferredWidth() + hgap_;         
    }
  }
 
  itr   = popup_->pane()->visualComponents().begin();
  for ( ; itr < popup_->pane()->visualComponents().end(); itr++) {
    NVisualComponent* visualChild = *itr;
    if ( visualChild->visible() )
    {         
      xp = xp + visualChild->preferredWidth() + hgap_;         
    }
  }
 
  return xp + spacing().left() + spacing().right() + borderLeft() + borderRight();
}

int NToolBar::preferredHeight( ) const
{
  int ymax = 0;
    
  std::vector<NVisualComponent*>::const_iterator itr   = visualComponents().begin( );
  for ( ; itr < visualComponents( ).end( ) ; itr++ ) {
    NVisualComponent* visual = *itr;
    if ( visual-> visible( ) ) {    
       if ( ymax < visual->preferredHeight( ) ) 
          ymax = visual->preferredHeight();  
    }
  }

  return ymax + spacing().top() + spacing().bottom() + borderTop() + borderBottom();
}

void NToolBar::drawChildren( NGraphics* g, const NRegion & repaintArea, NVisualComponent* sender ) {
  int hgap_ = 3;

  std::vector< NVisualComponent* >::const_iterator itr = visualComponents().begin();
  for ( ;itr < visualComponents().end(); itr++ ) {
      NVisualComponent* visual = *itr;
      if ( visual != moreBtn_ ) {
     //   if ( visual->left() + visual->preferredWidth()  > clientWidth() ) break;
        visual->draw( g, repaintArea, sender );
      }
  }
  
  moreBtn_->draw( g, repaintArea, sender );
}

NVisualComponent* NToolBar::checkChildrenEvent( NGraphics* g, int absX, int absY ) {
  NVisualComponent* found = 0;
  
  // check first moreBtn
  if ( moreBtn_->visible() ) {
      found = checkChildEvent( moreBtn_, g, absX, absY );
      if ( found ) return found;
  }
  
  std::vector<NVisualComponent*>::const_reverse_iterator rev_it = visualComponents().rbegin();
  for ( ; rev_it != visualComponents().rend(); ++rev_it ) {
      found = checkChildEvent( *rev_it, g, absX, absY );
      if ( found ) break;
  } 
  return found;
}     

void NToolBar::onMoreBtnClicked( NButtonEvent* ev ) {
       
  if ( !popup_->visible() ) {
    int winLeft = window()->left();
    int winTop  = window()->top();
    popup_->setPosition( winLeft + moreBtn_->absoluteLeft(), winTop + moreBtn_->absoluteTop() + moreBtn_->height() ,100, 100 );
    popup_->setVisible( true );
  } else {
    popup_->setVisible( false );     
  }
}     
