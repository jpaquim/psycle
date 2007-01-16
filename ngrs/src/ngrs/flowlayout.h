/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper   *
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
#ifndef NFLOWLAYOUT_H
#define NFLOWLAYOUT_H

#include "layout.h"
#include "visualcomponent.h"
#include "visualcomponent.h"

namespace ngrs {

  const int nAlHeight = 10;

  /**
  @author Stefan
  */
  class FlowLayout : public Layout
  {
  public:
    FlowLayout();
    FlowLayout(int align);
    FlowLayout(int align, int hgap, int vgap, int baseLine = nAlCenter);
    virtual FlowLayout * clone( ) const;

    ~FlowLayout();

    virtual void align(VisualComponent* parent);

    virtual int preferredWidth(const VisualComponent* target) const;
    virtual int preferredHeight(const VisualComponent* target) const;

    void setAlign(int align);
    void setHgap(int hgap);
    void setVgap(int vgap);
    void setLineBreak(bool on);
    void setBaseLine(int line);

    int align();
    int hgap();
    int vgap();

    virtual void drawComponents(class VisualComponent* target, Graphics& g , const ngrs::Region & repaintArea,VisualComponent* sender);

  private:

    int maxX, maxY;
    int align_;
    int hgap_,vgap_;
    int baseLine_;

    bool lineBrk_;

    void init();

  };

}

#endif
