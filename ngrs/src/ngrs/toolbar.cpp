/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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
#include "toolbar.h"
#include "bevelborder.h"
#include "app.h"
#include "config.h"
#include "button.h"
#include "popupmenu.h"
#include "label.h"
#include "app.h"
#include "config.h"

namespace ngrs {

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
    "..........."}
  ;

  ToolBar::ToolBar()
    : TogglePanel()
  { 
    ident_ = 5;  // space between toolbar start and first component              

    Bitmap bmp( icon_more_xpm );
    Image * img = new Image( bmp );
    img->setPreferredSize( bmp.width() + 5, bmp.height() + 5 );  
    moreBtn_ = new Button( img );
    moreBtn_->clicked.connect( this, &ToolBar::onMoreBtnClicked );
    add( moreBtn_ );

    popup_ = new PopupWindow();
    popup_->pane()->setSkin( App::config()->skin("popup_menu_bg") );
    add( popup_ );

    skin_ = App::config()->skin("toolbar");
  }


  ToolBar::~ToolBar()
  {
  }

  void ToolBar::paint( Graphics& g )
  {
  }

  void ToolBar::add( Runtime * component )
  {
    Panel::add(component);
  }

  Button* ToolBar::add( Button * button )
  {
    TogglePanel::add( button );
    return button;
  }

  void ToolBar::add( VisualComponent * comp )
  {
    Panel::add( comp );
  }

  void ToolBar::doAlign( ) {
    int hgap_ = 3;
    int vgap_ = 0;

    int xp = hgap_ + ident_;
    int yp = vgap_;
    int ymax = 2*vgap_;
    bool moreFlag = false;

    std::vector<VisualComponent*> swapVisuals;
    std::vector<VisualComponent*>::iterator itr; 
    int i = 0;
    for ( itr = popup_->pane()->vcBegin(); itr != popup_->pane()->vcEnd(); itr++ ) {
      swapVisuals.push_back( *itr );
      i++;
    }

    popup_->pane()->eraseAll();

    for ( itr = swapVisuals.begin(); itr != swapVisuals.end(); itr++ ) {
      (*itr)->setAlign( nAlLeft );
      add( *itr );
    }
    swapVisuals.clear();

    std::vector<VisualComponent*>::iterator moreItr = vcEnd();
    for ( itr = vcBegin(); itr < vcEnd(); itr++ ) {
      VisualComponent* visualChild = *itr;
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

    for ( itr = vcBegin(); itr < vcEnd() ; itr++ ) {
      VisualComponent* visual = *itr;
      if ( visual-> visible() ) {    
        visual->setTop( yp + ( ymax - visual->preferredHeight() ) / 2 );    
      }
    }

    if ( moreFlag ) {
      moreBtn_->setLeft( clientWidth() - moreBtn_->preferredWidth() - hgap_ );
      moreBtn_->setWidth( moreBtn_->preferredWidth() );
      moreBtn_->setHeight( moreBtn_->preferredHeight() );
      moreBtn_->setVisible( true );

      for ( itr = moreItr; itr != vcEnd(); itr++ ) {
        swapVisuals.push_back( *itr );
      }

      if ( moreItr != vcEnd() )
        erase( moreItr, vcEnd() );

      popup_->pane()->eraseAll();
      for ( itr = swapVisuals.begin(); itr != swapVisuals.end(); itr++ ) {
        popup_->pane()->add( *itr, nAlTop );
      }

    } else 
      moreBtn_->setVisible( false );

  }     

  void ToolBar::resize( )
  {
    TogglePanel::resize();
    doAlign();
  }

  int ToolBar::preferredWidth( ) const
  {
    int hgap_ = 3;
    int xp = hgap_ + ident_;

    std::vector<VisualComponent*>::const_iterator itr   = visualComponents().begin();  
    for ( ; itr < visualComponents().end(); itr++) {
      VisualComponent* visualChild = *itr;
      if ( visualChild->visible() ) {      
        xp = xp + visualChild->preferredWidth() + hgap_;         
      }
    }

    itr   = popup_->pane()->visualComponents().begin();
    for ( ; itr < popup_->pane()->visualComponents().end(); itr++) {
      VisualComponent* visualChild = *itr;
      if ( visualChild->visible() )
      {         
        xp = xp + visualChild->preferredWidth() + hgap_;         
      }
    }

    return xp + spacing().left() + spacing().right() + borderLeft() + borderRight();
  }

  int ToolBar::preferredHeight( ) const
  {
    int ymax = 0;

    std::vector<VisualComponent*>::const_iterator itr   = visualComponents().begin( );
    for ( ; itr < visualComponents( ).end( ) ; itr++ ) {
      VisualComponent* visual = *itr;
      if ( visual-> visible( ) ) {    
        if ( ymax < visual->preferredHeight( ) ) 
          ymax = visual->preferredHeight();  
      }
    }

    return ymax + spacing().top() + spacing().bottom() + borderTop() + borderBottom();
  }

  void ToolBar::drawChildren( Graphics& g, const ngrs::Region & repaintArea, VisualComponent* sender ) {
    int hgap_ = 3;

    std::vector< VisualComponent* >::const_iterator itr = visualComponents().begin();
    for ( ;itr < visualComponents().end(); itr++ ) {
      VisualComponent* visual = *itr;
      if ( visual != moreBtn_ ) {
        //   if ( visual->left() + visual->preferredWidth()  > clientWidth() ) break;
        visual->draw( g, repaintArea, sender );
      }
    }

    moreBtn_->draw( g, repaintArea, sender );
  }

  VisualComponent* ToolBar::checkChildrenEvent( Graphics& g, int absX, int absY ) {
    VisualComponent* found = 0;

    // check first moreBtn
    if ( moreBtn_->visible() ) {
      found = checkChildEvent( moreBtn_, g, absX, absY );
      if ( found ) return found;
    }

    std::vector<VisualComponent*>::const_reverse_iterator rev_it = visualComponents().rbegin();
    for ( ; rev_it != visualComponents().rend(); ++rev_it ) {
      found = checkChildEvent( *rev_it, g, absX, absY );
      if ( found ) break;
    } 
    return found;
  }     

  void ToolBar::onMoreBtnClicked( ButtonEvent* ev ) {

    if ( !popup_->visible() ) {
      int winLeft = window()->left();
      int winTop  = window()->top();
      popup_->setPosition( winLeft + moreBtn_->absoluteLeft(), winTop + moreBtn_->absoluteTop() + moreBtn_->height() ,100, 100 );
      popup_->setVisible( true );
    } else {
      popup_->setVisible( false );     
    }
  }     

}
