/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "nstatusbar.h"
#include "nstatusmodel.h"
#include "nalignlayout.h"
#include "ncustomstatusitem.h"
#include "napp.h"
#include "nconfig.h"
#include "ntextstatusitem.h"


NStatusBar::NStatusBar()
 : NPanel(), statusModel_(0)
{
  setLayout( NAlignLayout() );
  setAlign( nAlBottom );

  setSkin( NApp::config()->skin("stat_bar_bg") );

  add(  new NTextStatusItem(), nAlClient );
}


NStatusBar::~NStatusBar()
{
}

void NStatusBar::add( NCustomStatusItem * component )
{
  add( component, nAlRight );
}

void NStatusBar::add( NCustomStatusItem * component, int align )
{
  NPanel::add(component,align);
  statusItems_.push_back( component );
}

void NStatusBar::add( NVisualComponent * component, int align )
{
  NPanel::add( component, align );
}

void NStatusBar::setModel( NCustomStatusModel & model ) {
  statusModel_ = &model;
  statusModel_->changed.connect( this, &NStatusBar::onModelDataChange );
}

NCustomStatusModel* NStatusBar::model() const {
  return statusModel_;
}

void NStatusBar::onModelDataChange( const NCustomStatusModel & sender, unsigned int index ) {
  std::vector<NCustomStatusItem*>::iterator it = statusItems_.begin();
  for ( ; it < statusItems_.end(); it++ ) {
	if ( index == (*it)->modelIndex() ) {
	  (*it)->setText( sender.text( index ) );
      (*it)->repaint();
	}
  } 
}
