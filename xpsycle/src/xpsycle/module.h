/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper   *
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
#ifndef MODULE_H
#define MODULE_H

#include <psycore/song.h>
#include <ngrs/panel.h>

class Module : public ngrs::Panel
{
  public:

    Module();

    virtual ~Module() = 0;

    void setSong( psy::core::Song* song );
    psy::core::Song* song(); // maybe 0

  private:

    psy::core::Song* song_;

};

#endif
