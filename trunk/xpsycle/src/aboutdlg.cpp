/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "aboutdlg.h"
#include "nlabel.h"

AboutDlg::AboutDlg()
 : NWindow()
{
  setTitle("About");

  pane()->add(new NLabel("Psycle version (X alpha 0.1)"),nAlTop);
  pane()->add(new NLabel("(c) 2006 by Psycledelics Community"),nAlTop);
  pane()->add(new NLabel("GNU Public Licence 2.0"),nAlTop);

  setPosition(10,10,400,200);
}


AboutDlg::~AboutDlg()
{
}

int AboutDlg::onClose( )
{
  setVisible(false);
  return nHideWindow;
}


