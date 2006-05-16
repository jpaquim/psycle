/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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
#include "ngrs/niswindow.h"
#include "ngrs/nvisualcomponent.h"
#include "ngrs/nwindow.h"



NIsWindow::NIsWindow()
 : NVisitor()
{
}


NIsWindow::~NIsWindow()
{
}

bool NIsWindow::visit_window( NWindow * )
{
  return true;
}

bool NIsWindow::visit_visualcomponent( NVisualComponent * )
{
  return false;
}


bool NIsWindow::visit_window( NVisualComponent * )
{
  return false;
}


