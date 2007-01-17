/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#include "statusbar.h"
#include "statusmodel.h"
#include "alignlayout.h"
#include "customstatusitem.h"
#include "app.h"
#include "config.h"
#include "textstatusitem.h"

namespace ngrs {

  NStatusBar::NStatusBar()
    : Panel(), statusModel_(0)
  {
    setLayout( AlignLayout() );
    setAlign( nAlBottom );

    setSkin( App::config()->skin("stat_bar_bg") );

    add(  new TextStatusItem(), nAlClient );
  }


  NStatusBar::~NStatusBar()
  {
  }

  void NStatusBar::add( CustomStatusItem * component )
  {
    add( component, nAlRight );
  }

  void NStatusBar::add( CustomStatusItem * component, int align )
  {
    Panel::add(component,align);
    statusItems_.push_back( component );
  }

  void NStatusBar::add( VisualComponent * component, int align )
  {
    Panel::add( component, align );
  }

  void NStatusBar::setModel( CustomStatusModel & model ) {
    statusModel_ = &model;
    statusModel_->changed.connect( this, &NStatusBar::onModelDataChange );
  }

  CustomStatusModel* NStatusBar::model() const {
    return statusModel_;
  }

  void NStatusBar::onModelDataChange( const CustomStatusModel & sender, unsigned int index ) {
    std::vector<CustomStatusItem*>::iterator it = statusItems_.begin();
    for ( ; it < statusItems_.end(); it++ ) {
      if ( index == (*it)->modelIndex() ) {
        (*it)->setText( sender.text( index ) );
        (*it)->repaint();
      }
    } 
  }

}
