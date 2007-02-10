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
#include "skinreader.h"
#include <ngrs/alignlayout.h>
#include <ngrs/label.h>
#include <ngrs/memo.h>
#include <ngrs/tabbook.h>
#include <ngrs/image.h>
#include <ngrs/bevelborder.h>
#include <ngrs/button.h>

namespace psycle {
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

        SkinReader::Instance()->setDefaults();
        DefaultBitmaps & icons =  SkinReader::Instance()->bitmaps();
        ngrs::Image* logoImg = new ngrs::Image( icons.logoRight()  );
        logoImg->setVAlign( ngrs::nAlCenter );

        ngrs::Panel* logoPnl = new ngrs::Panel();
        logoPnl->setSpacing( 10, 10, 10, 10 );
        logoPnl->setLayout( ngrs::AlignLayout() );
        ngrs::Label* lb = new ngrs::Label("Psycle X1" );
        logoPnl->add( logoImg , ngrs::nAlRight );
        pane()->add( logoPnl, ngrs::nAlTop );
        initText();

       ngrs::TabBook *tabBk = new ngrs::TabBook;
       tabBk->setSpacing( 5, 0, 5, 5 );
       const ngrs::BevelBorder border;
       tabBk->setBorder( border );
       ngrs::Panel *aboutPage = new ngrs::Panel;
            aboutPage->setLayout( ngrs::AlignLayout() );
       ngrs::Panel *creditsPage = new ngrs::Panel;
            creditsPage->setLayout( ngrs::AlignLayout() );
       ngrs::Panel *licensePage = new ngrs::Panel;
            licensePage->setLayout( ngrs::AlignLayout() );

        ngrs::Panel *infoPnl = new ngrs::Panel();
            infoPnl->setLayout( ngrs::AlignLayout() );
            infoPnl->setSpacing( 0, 5, 0, 5 );
        infoPnl->add( new ngrs::Label("Psycle X1"), ngrs::nAlTop );
        lb = new ngrs::Label( "version: svn alpha" );
        lb->setSpacing( 5, 5, 5, 5 );
        lb->setWordWrap( true );
        infoPnl->add( lb, ngrs::nAlTop );


        ngrs::Panel *feedPnl = new ngrs::Panel();
            feedPnl->setLayout( ngrs::AlignLayout() );

        feedPnl->add( new ngrs::Label("Feedback"), ngrs::nAlTop );
        lb = new ngrs::Label( bugs );
        lb->setWordWrap( true );
        lb->setSpacing( 5, 5, 5, 5 );
        feedPnl->add( lb , ngrs::nAlTop );
        lb = new ngrs::Label( "Mailing list: " + maillist );
        lb->setWordWrap( true );
        lb->setSpacing( 5, 5, 5, 5 );
        feedPnl->add( lb , ngrs::nAlTop );

        aboutPage->add( infoPnl, ngrs::nAlTop );
        aboutPage->add( feedPnl, ngrs::nAlTop );

       creditsPage->add( new ngrs::Label("Authors"), ngrs::nAlTop );
       lb = new ngrs::Label( author );
       lb->setSpacing( 5, 5, 5, 5 );
       lb->setWordWrap( true );
       creditsPage->add( lb , ngrs::nAlTop );

       lb = new ngrs::Label( "Buildscripts written by "+building );
       lb->setSpacing( 5, 5, 5, 5 );
       lb->setWordWrap( true );
       creditsPage->add( lb , ngrs::nAlTop );

       creditsPage->add( new ngrs::Label("Tested by"), ngrs::nAlTop );
       lb = new ngrs::Label( testing );
       lb->setWordWrap( true );
       lb->setSpacing( 5, 5, 5, 5 );
       creditsPage->add( lb , ngrs::nAlTop );

       ngrs::Memo* memo = new ngrs::Memo();
       memo->setText( gpl );
       memo->setReadOnly(true);
       memo->setWordWrap( true );
       licensePage->add( memo, ngrs::nAlClient );

       tabBk->addPage( aboutPage, "About" );
       tabBk->addPage( creditsPage, "Credits" );
       tabBk->addPage( licensePage, "License" );

       tabBk->setActivePage( aboutPage );

       ngrs::Panel *btnPnl = new ngrs::Panel();
            btnPnl->setLayout( ngrs::AlignLayout() );
            btnPnl->setSpacing( 5, 5, 5, 5 );
            ngrs::Button *clsBtn = new ngrs::Button( "Close" );
            clsBtn->clicked.connect( this, &AboutBox::onClsBtn );
            clsBtn->setFlat( false );
       btnPnl->add( clsBtn, ngrs::nAlRight );

       pane()->add( btnPnl, ngrs::nAlBottom  );
       pane()->add( tabBk, ngrs::nAlTop  );

        setPosition( 0, 0, 380, 400 );
        setPositionToScreenCenter();
    }

    void AboutBox::initText() {
      author = "Stefan Nattkemper,"
               "Josep Segura,"
               "D.W. Aley,"
               "Neil Mather";
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
      maillist = "psycle-devel@list.sourceforge.net";

    }

    void AboutBox::onClsBtn( ngrs::ButtonEvent* ev )
    {
        onClose();
    }

  }
}
