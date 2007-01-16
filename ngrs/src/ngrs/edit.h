/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#ifndef EDIT_H
#define EDIT_H

#include "panel.h"
#include "fontmetrics.h"
#include "regexp.h"
#include <string>


/**
@author Stefan
*/

namespace ngrs {

  class Edit : public Panel
  {
  public:
    Edit();
    Edit(const std::string & text);

    ~Edit();

    void setAutoSize(bool on);
    void setReadOnly(bool on);

    void setText(const std::string & text);
    const std::string & text() const;
    std::string selText() const;

    void setInputPolicy( const std::string & regexp );

    virtual void paint(Graphics& g);
    virtual void onKeyPress(const KeyEvent & keyevent);

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    virtual void onMousePress( int x, int y, int button );
    virtual void onMouseOver( int x, int y );
    virtual void onMousePressed( int x, int y, int button );

    virtual void onEnter();
    virtual void onExit();

    void setVAlign(int align);
    void setHAlign(int align);

    int vAlign() const;
    int hAlign() const;

    void setPos(unsigned int pos);
    std::string::size_type pos() const;

    sigslot::signal1<const KeyEvent &> keyPress;

  private:

    RegExp regExp_;

    bool autoSize_, readOnly_;
    int valign_, halign_;
    int dx;

    std::string::size_type pos_;

    std::string::size_type selStartIdx_;
    std::string::size_type selEndIdx_;
    std::string::size_type selStartPos_;

    std::string text_;

    NPoint getScreenPos(Graphics& g, const std::string & text );
    int computeDx( Graphics& g, const std::string & text );
    void drawCursor(Graphics& g, const std::string & text );

    void init();

    void startSel();
    void computeSel();
    void endSel();

  };

}

#endif
