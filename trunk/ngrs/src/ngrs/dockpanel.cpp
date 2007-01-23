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
#include "dockpanel.h"
#include "alignlayout.h"
#include "label.h"
#include "image.h"
#include "button.h"
#include "window.h"
#include "app.h"
#include "config.h"
#include "point.h"

namespace ngrs {

  /* XPM */
  const char * winundock_xpm[] = {
    "16 10 2 1",
    " 	c None",
    ".	c #003063",
    "................",
    ".              .",
    "................",
    ".              .",
    ".          ... .",
    ".           .. .",
    ".          . . .",
    ".         .    .",
    ".              .",
    "................"
  };


  const char * windock_xpm[] = {
    "16 10 2 1",
    " 	c None",
    ".	c #003063",
    "................",
    ".              .",
    "................",
    ".              .",
    ".            . .",
    ".         . .  .",
    ".         ..   .",
    ".         ...  .",
    ".              .",
    "................"
  };


  DockPanel::DockPanel()
    : Panel()
  {
    init();
  }

  DockPanel::DockPanel( VisualComponent * clientComponent )
  {
    init();
    pane()->add(clientComponent, nAlClient);
  }

  void DockPanel::init( )
  {
    // an alignLayout for top : dochBar and client the pane
    setLayout( AlignLayout() );

    // create header with the button to dock and undock the pane

    dockBar_ = new Panel();
    dockBar_->setLayout( AlignLayout() );
    // create the dockBar icon bitmaps
    dockBmp.createFromXpmData( windock_xpm );
    undockBmp.createFromXpmData( winundock_xpm );
    dockImg = new Image( );
    dockImg->setSharedBitmap( &undockBmp );
    dockImg->setPreferredSize( 20, 10 );
    Button* unCoupleBtn = new Button( dockImg );
    unCoupleBtn->clicked.connect( this, &DockPanel::onUndockWindow );
    dockBar_->add( unCoupleBtn, nAlRight );
    dockBar_->setSkin( App::config()->skin("dockbar_bg") );
    dockBarLabel_ = new Label("Song Explorer");
    dockBar_->add( dockBarLabel_, nAlClient );
    add( dockBar_, nAlTop );

    // create the pane

    area_ = new Panel();
    // set as area btw pane default AlignLayout
    area_->setLayout( AlignLayout( ) );
    add( area_, nAlClient );

    // start state = docked
    undockedWindow = 0;
    autoHideDock_ = true;
  }

  DockPanel::~DockPanel()
  {
  }


  Panel * DockPanel::pane( )
  {
    return area_;
  }

  void DockPanel::setAutoHideOnDockOut( bool on ) {
    autoHideDock_ = on;
  }

  void DockPanel::onUndockWindow( ButtonEvent * ev )
  {
    if (undockedWindow!=0) {
      dockWindow();
      App::addRemovePipe(undockedWindow);
      undockedWindow=0;
    } else {

      add ( undockedWindow = new Window() );
      undockedWindow->setDock(this);
      Point newWinSize(area_->width(),area_->height() + dockBar_->height());

      erase(area_);
      erase(dockBar_);

      undockedWindow->pane()->add(dockBar_,nAlTop);
      undockedWindow->pane()->add(area_,nAlClient);

      dockImg->setSharedBitmap(&dockBmp);

      undockedWindow->setPosition(0,0,newWinSize.x(),newWinSize.y());
      undockedWindow->setVisible(true);

      if ( autoHideDock_ ) {
        setVisible(false);
      }

      window()->pane()->resize();
      window()->repaint(window()->pane(),Rect(0,0,window()->width(),window()->height()));
      window()->checkForRemove(0);
    }
  }

  void DockPanel::onDockWindow( )
  {
    dockWindow();
    delete undockedWindow;
    undockedWindow = 0;
  }


  void DockPanel::dockWindow( )
  {
    undockedWindow->setVisible(false);
    App::lastOverWin_ = 0;

    undockedWindow->pane()->erase(dockBar_);
    undockedWindow->pane()->erase(area_);

    Panel::add(dockBar_,nAlTop);
    Panel::add(area_,nAlClient);

    dockImg->setSharedBitmap(&undockBmp);

    if ( autoHideDock_ ) {
        setVisible(true);
    }

    window()->pane()->resize();
    window()->repaint(window()->pane(),Rect(0,0,window()->width(),window()->height()));
    window()->checkForRemove(0);
  }

}
