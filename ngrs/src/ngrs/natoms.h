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
#ifndef NATOMS_H
#define NATOMS_H

#ifdef __unix__

#include <X11/Xutil.h>
#include <X11/Xatom.h>


/**
@author Stefan Nattkemper
*/
class NAtoms {
public:
    NAtoms(Display* dpy);

    ~NAtoms();

	 Atom wm_delete_window() const;
   Atom wm_motif_hint() const;
   Atom net_wm_state() const;
   Atom net_wm_state_above() const;
   Atom net_wm_state_stays_on_top() const;
   Atom net_wm_state_modal() const;

   Atom primary_sel() const;
   Atom targets() const;

private:

   Atom wm_delete_window_;
   Atom wm_motif_hint_;
   Atom net_wm_state_;
   Atom net_wm_state_above_;
   Atom net_wm_state_stays_on_top_;
   Atom net_wm_state_modal_;

   Atom primary_sel_;
   Atom targets_;

};

#endif // unix endif
#endif
