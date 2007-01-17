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
#include "isvisualcomponent.h"
#include "visualcomponent.h"
#include "window.h"

namespace ngrs {

  IsVisualComponent::IsVisualComponent()
    : NVisitor()
  {
  }


  IsVisualComponent::~IsVisualComponent()
  {
  }

  bool IsVisualComponent::visit_visualcomponent( VisualComponent * vComp )
  {
    return true;
  }


  bool IsVisualComponent::visit_window( Window * )
  {
    return false;
  }

  bool IsVisualComponent::visit_visualcomponent( Window * )
  {
    return false;
  }

}

