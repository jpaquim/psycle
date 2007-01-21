/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper  *
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
#include "aboutbox.h"
#include <ngrs/alignlayout.h>
#include <ngrs/label.h>
#include <ngrs/memo.h>

namespace psy {
  namespace host {

    AboutBox::AboutBox()
    {
      init();
    }

    AboutBox::~AboutBox()
    {
    }

    void AboutBox::init() {
      setTitle( "About Psycle X1" );
      ngrs::Panel* logoPnl = new ngrs::Panel();
       logoPnl->setSpacing( 10, 10, 10, 10 );
       logoPnl->setLayout( ngrs::AlignLayout() );
       ngrs::Label* lb = new ngrs::Label("Psycle X1" );
       logoPnl->add( lb, ngrs::nAlClient );
       pane()->add( logoPnl, ngrs::nAlTop );
       initText();

       pane()->add( new ngrs::Label("Feedback"), ngrs::nAlTop );
       lb = new ngrs::Label( bugs );
       lb->setWordWrap( true );
       lb->setSpacing( 5, 5, 5, 5 );
       pane()->add( lb , ngrs::nAlTop );
       
       pane()->add( new ngrs::Label("Version"), ngrs::nAlTop );
       lb = new ngrs::Label( "svn alpha" );
       lb->setSpacing( 5, 5, 5, 5 );
       lb->setWordWrap( true );
       pane()->add( lb , ngrs::nAlTop );

       pane()->add( new ngrs::Label("Authors"), ngrs::nAlTop );
       lb = new ngrs::Label( "Authors" + author );
       lb->setSpacing( 5, 5, 5, 5 );
       lb->setWordWrap( true );
       pane()->add( lb , ngrs::nAlTop );

       lb = new ngrs::Label( "Buildscripts written by "+building );
       lb->setSpacing( 5, 5, 5, 5 );
       lb->setWordWrap( true );
       pane()->add( lb , ngrs::nAlTop );

       pane()->add( new ngrs::Label("Tested by"), ngrs::nAlTop );
       lb = new ngrs::Label( testing );
       lb->setWordWrap( true );
       lb->setSpacing( 5, 5, 5, 5 );
       pane()->add( lb , ngrs::nAlTop );

       ngrs::Memo* memo = new ngrs::Memo();
       memo->setText( gpl );
       memo->setReadOnly(true);
       memo->setWordWrap( true );
       pane()->add( memo, ngrs::nAlClient );
       setPosition( 0, 0, 400, 300 );
       setPositionToScreenCenter();
    }

    void AboutBox::initText() {
      author = "Stefan Nattkemper,"
               "Josep Segura,"
               "D.W. Aley,"
               "Neil mather,";
      building = "Johan Boule";
      testing = "Neil Mather";
      bugs = "Todo list http://psycle.sourceforge.net/wiki/Xpsycle/Todo\n";
      home = "X Psycle homepage: http://psycle.sourceforge.net/wiki/Xpsycle\n";
      gpl = "This program is free software, released under the GNU "
	        "General Public License and you are welcome to redistribute it under "
	        "certain conditions.  It comes with ABSOLUTELY NO WARRANTY for "
	        "details read the GNU General Public License to be found in the file "
	        "COPYING distributed with this program, or online at:"
	        "http://www.gnu.org/copyleft/gpl.html\n";
    }

  }
}
