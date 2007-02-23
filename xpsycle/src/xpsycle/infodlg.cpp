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

#include "infodlg.h"
#include <ngrs/memo.h>

namespace psycle { namespace host {

InfoDlg::InfoDlg()
	: ngrs::Window()
{
	memo = new ngrs::Memo();
		memo->setWordWrap(true);
		memo->setReadOnly(true);
	pane()->add(memo,ngrs::nAlClient);

	setSize(500,500);
	setPositionToScreenCenter();
}


InfoDlg::~InfoDlg()
{
}

int InfoDlg::onClose( )
{
	setVisible(false);
	return ngrs::nHideWindow;
}

void InfoDlg::loadFromFile( const std::string & fileName )
{
	try {
		memo->loadFromFile(fileName);
		setTitle(fileName);
	}
	catch (const char* e) {
		setTitle("Help path wrong");
	}
}

}}
