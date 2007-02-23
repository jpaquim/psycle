/***************************************************************************
*   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#include "greetdlg.h"
#include <ngrs/label.h>
#include <ngrs/memo.h>
#include <ngrs/item.h>

namespace psy { 
	namespace host {

		GreetDlg::GreetDlg()
			: ngrs::Window()
		{
			memo = new ngrs::Memo();
			memo->setReadOnly(true);

			ngrs::Label* greetings = new ngrs::Label();
			greetings->setText("Psyceledics Community, wants to thank the following people for their contributions in the developement of Psycle");
			greetings->setWordWrap(true);
			pane()->add(greetings,ngrs::nAlTop);

			pane()->add(memo, ngrs::nAlClient);

			setGreetz();

			setPosition(10,10,600,500);
		}


		GreetDlg::~GreetDlg()
		{
		}

		int GreetDlg::onClose( )
		{
			setVisible(false);
			return ngrs::nHideWindow;
		}

		void GreetDlg::setGreetz( )
		{
			memo->setText("all psycledelics members, ppl who helped writing this program");
		}

	} // end of host namespace
} // end of psycle namespace
