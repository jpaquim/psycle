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
#include "natoms.h"

#ifdef __unix__

NAtoms::NAtoms(Display* dpy)
{
  wm_delete_window_ = XInternAtom(dpy, "WM_DELETE_WINDOW\0", False);;
  wm_motif_hint_ = XInternAtom(dpy,"_MOTIF_WM_HINTS\0",False);;
  net_wm_state_ = XInternAtom(dpy,"_NET_WM_STATE\0",False);
  net_wm_state_above_ = XInternAtom(dpy,"_NET_WM_STATE_ABOVE\0",False);
  net_wm_state_stays_on_top_ = XInternAtom(dpy,"_NET_WM_STATE_STAYS_ON_TOP\0",False);
  net_wm_state_modal_ = XInternAtom(dpy,"_NET_WM_STATE_MODAL\0",False);

	primary_sel_ = XInternAtom(dpy, "PRIMARY", False);
  targets_ = XInternAtom(dpy, "TARGETS", False);
}


NAtoms::~NAtoms()
{
}

Atom NAtoms::wm_delete_window( ) const
{
  return wm_delete_window_;
}

Atom NAtoms::wm_motif_hint( ) const
{
  return wm_motif_hint_;
}

Atom NAtoms::net_wm_state( ) const
{
  return net_wm_state_;
}

Atom NAtoms::net_wm_state_above( ) const
{
  return net_wm_state_above_;
}

Atom NAtoms::net_wm_state_stays_on_top( ) const
{
  return net_wm_state_stays_on_top_;
}

Atom NAtoms::net_wm_state_modal( ) const
{
  return net_wm_state_modal_;
}

Atom NAtoms::primary_sel( ) const
{
  return primary_sel_;
}

Atom NAtoms::targets( ) const
{
  return targets_;
}

#endif
