/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper                               *
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
#ifndef ALIGNLAYOUT_H
#define ALIGNLAYOUT_H

#include "layout.h"
#include <vector>

/**
@author  Stefan
*/

namespace ngrs {

  class AlignLayout : public Layout
  {
  public:
    AlignLayout();
    AlignLayout(int hgap, int vgap);
    virtual AlignLayout* clone()  const;   // Uses the copy constructor

    ~AlignLayout();

    virtual void align(VisualComponent* parent);
    virtual int preferredWidth(const VisualComponent* target) const;
    virtual int preferredHeight(const VisualComponent* target) const;

    void setHgap(int hgap);
    void setVgap(int vgap);

    virtual void add(VisualComponent* comp);
    virtual void remove(VisualComponent* comp);
    virtual void removeAll();


  private:

    int hgap_,vgap_;
    mutable int maxX_;
    mutable int maxY_;
    mutable bool preferredWidthChanged_;
    mutable bool preferredHeightChanged_;

    std::vector<VisualComponent*> components;

    void setChanged( bool on );

  };

}

#endif
