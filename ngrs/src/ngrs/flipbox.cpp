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
#include "flipbox.h"
#include "flipbar.h"
#include "alignlayout.h"
#include "label.h"

namespace ngrs {

  NFlipBox::NFlipBox()
    : Panel()
  {
    setLayout(AlignLayout());

    flipBar_ = new FlipBar();
    flipBar_->change.connect(this,&NFlipBox::onFlipChange);
    add (flipBar_  , nAlTop);

    pane_ = new Panel();
    pane_->setLayout(AlignLayout());
    add( pane_ , nAlClient);

  }


  NFlipBox::~NFlipBox()
  {
  }

  int NFlipBox::preferredHeight( ) const
  {
    if (flipBar_->expanded())
      return flipBar_->preferredHeight() + pane_->preferredHeight();
    else
      return flipBar_->preferredHeight();
  }

  //int NFlipBox::preferredWidth( ) const
  //{
  //  return flipBar_->preferredWidth() + (!pane_ ? 0 : pane_->preferredWidth());
  //}

  void NFlipBox::onFlipChange( FlipBar * sender )
  {
    setHeight(preferredHeight());
    ((VisualComponent*)parent())->resize();
    ((VisualComponent*)parent())->repaint();
  }

  Panel* NFlipBox::pane()
  {
    return pane_;
  }

  Panel * NFlipBox::header( )
  {
    return flipBar_->header();
  }

  int NFlipBox::flipperWidth( ) const
  {
    return flipBar_->flipperWidth();
  }

  void NFlipBox::setExpanded( bool on )
  {
    flipBar_->setExpanded(on);
  }

  bool NFlipBox::expanded() const {
    return flipBar_->expanded();
  }

  void NFlipBox::setStyle( int style ) {
    style_ = style; 
  }

  int NFlipBox::style() const {
    return style_;
  }

  void NFlipBox::paint( Graphics& g ) {

    if ( style() & nFlipBoxLine ) {

      Pen oldPen = g.pen();
      g.setForeground( Color( 200, 200, 200 ) );
      Pen pen;
      pen.setLineStyle( nLineOnOffDash );
      g.setPen( pen );	  
      int xoff = flipBar_->flipperWidth() / 2;

      g.drawLine( xoff, flipBar_->header()->height() / 2, xoff, height() );	  
      g.drawLine( xoff, flipBar_->header()->height() / 2, flipBar_->header()->left(), flipBar_->header()->height() / 2 );

      g.setPen( oldPen );
    }
  }

}
