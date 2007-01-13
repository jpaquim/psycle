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
#include "patternboxproperties.h"
#include "sequencerbar.h"
#include "patterndata.h"
#include <ngrs/nframeborder.h>
#include <ngrs/ntablelayout.h>
#include <ngrs/nalignlayout.h>
#include <ngrs/nlabel.h>
#include <ngrs/nedit.h>
#include <ngrs/ncolorcombobox.h>

namespace psycle { namespace host {

PatternBoxProperties::PatternBoxProperties()
 : ngrs::NFlipBox()
{
  ngrs::NFrameBorder fr;
  setBorder(fr);

  header()->add(new ngrs::NLabel("Pattern Properties"), ngrs::nAlClient);

  ngrs::NPanel* table = new ngrs::NPanel();
    ngrs::NTableLayout tableLayout(2,2);
    tableLayout.setVGap(5);
    tableLayout.setHGap(5);
    table->setLayout( tableLayout );
    table->add(new ngrs::NLabel("Name"), ngrs::NAlignConstraint(ngrs::nAlLeft,0,0),true);
    categoryEdt = new ngrs::NEdit();
       categoryEdt->setPreferredSize(150,15);
       categoryEdt->keyPress.connect(this, &PatternBoxProperties::onKeyPress);
    table->add(categoryEdt, ngrs::NAlignConstraint(ngrs::nAlLeft,1,0),true);
    table->add(new ngrs::NLabel("Color"), ngrs::NAlignConstraint(ngrs::nAlLeft,0,1),true);
    clBox = new ngrs::NColorComboBox();
      clBox->colorSelected.connect(this,&PatternBoxProperties::onColorChange);
      clBox->setPreferredSize(50,15);
    table->add(clBox, ngrs::NAlignConstraint(ngrs::nAlLeft,1,1),true);
  pane()->add(table, ngrs::nAlClient);

  cat_ = 0;
}


PatternBoxProperties::~PatternBoxProperties()
{
}

void PatternBoxProperties::setName( const std::string & name )
{
  categoryEdt->setText(name);
  categoryEdt->repaint();
}

const std::string & PatternBoxProperties::name( ) const
{
  return categoryEdt->name();
}

void PatternBoxProperties::onKeyPress( const ngrs::NKeyEvent & )
{
  nameChanged.emit(categoryEdt->text());
}

void PatternBoxProperties::onColorChange( const ngrs::NColor & color )
{
  if (cat_ != 0) {
     int r = color.red();
     int g = color.green();
     int b = color.blue();
     long href = (b << 16) | (g << 8) | r;
     cat_->category()->setColor( href );
     cat_->repaint();
  }
}

void PatternBoxProperties::setCategoryItem( CategoryItem * cat )
{
  cat_ = cat;
}


}}
