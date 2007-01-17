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
#ifndef NSTATUSBAR_H
#define NSTATUSBAR_H

#include "panel.h"

namespace ngrs {

  class CustomStatusItem;
  class TextStatusItem;
  class CustomStatusModel;

  /**
  @author  Stefan Nattkemper
  */

  // view for a ncustomstatus based data model

  class NStatusBar : public Panel
  {
  public:

    NStatusBar();

    ~NStatusBar();

    virtual void add(CustomStatusItem* component);
    virtual void add(CustomStatusItem* component, int align);
    virtual void add(VisualComponent* component, int align);

    void setModel( CustomStatusModel & model );
    CustomStatusModel* model() const;

  private:

    CustomStatusModel* statusModel_;
    std::vector<CustomStatusItem*> statusItems_;

    void onModelDataChange( const CustomStatusModel & sender, unsigned int index );

  };

}

#endif
