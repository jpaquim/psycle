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
#ifndef NPROGRESSBAR_H
#define NPROGRESSBAR_H

#include "panel.h"
#include "gradient.h"

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace ngrs {

  class Label;

  /**
  @author Stefan
  */
  class ProgressBar : public Panel
  {
  public:
    ProgressBar();

    ~ProgressBar();

    virtual void resize();

    void setText(const std::string & text);
    std::string text();

    void setMin(int min);
    int min() const;
    void setMax(int max);
    int max() const;
    void setStep(int step);
    int step() const;
    void setValue(int value);
    int value() const;
    void setRange(int min, int max);

    void setOrientation(int orientation);

  private:

    Border* border_;
    Label* label_;

    int progress_;
    int min_,max_,step_;
    int orientation_;

    Gradient* progressGradient_;

  };

}

#endif
