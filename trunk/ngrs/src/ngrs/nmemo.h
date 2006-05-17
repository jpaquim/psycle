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
#ifndef NMEMO_H
#define NMEMO_H

#include "ntextbase.h"
#include "npoint.h"
/**
@author Stefan Nattkemper
*/


class NMemo : public NTextBase
{
   class TextArea : public NPanel {
     class Line {
       public :

          Line();
          Line(TextArea* area);
          ~Line();

          void setText( const std::string & text );
          const std::string & text() const;
          void insert(unsigned int pos, const std::string & text);
          void erase(unsigned int count);

          void setTop( int top );
          void move(int dy);
          int top() const;
          int height() const;

          void incPos();
          void decPos();
          void setPos( unsigned int pos );
          void setPosEnd();
          void setPosStart();
          unsigned int pos() const;

          NPoint screenPos() const;
          int width() const;

          std::string strFromPos() const;
          std::string deleteFromPos();
          std::string strToPos() const;
          std::string deleteToPos();

          void computeBreakPoints();

          void drawText(NGraphics *g);

       private:

          TextArea* pArea;
          std::string text_;
          int top_;
          unsigned int pos_;
          int height_;

          int findWidthMax(long width, const std::string & data, bool wbreak);

          std::vector<int> breakPoints;

     };

     public:
       TextArea();
       TextArea(NMemo* memo);

       ~TextArea();

       void setText(const std::string & text);
       std::string text() const;

       virtual void paint(NGraphics* g);
       virtual void onKeyPress(const NKeyEvent & keyEvent);

       void setWordWrap(bool on);
       bool wordWrap() const;

       void setReadOnly(bool on);
       bool readOnly() const;

       void appendLine(const std::string & text);
       void clear();

       void loadFromFile(const std::string & fileName);


       virtual int preferredWidth() const;
       virtual int preferredHeight() const;

       virtual void resize();

     private:

      NMemo* pMemo;
      bool wordWrap_;
      bool readOnly_;

      std::vector<Line>::iterator lineIndexItr;
      std::vector<Line> lines;

      void init();
      void drawCursor( NGraphics* g , int x, int y );
      int findVerticalStart() const;

      void insertLine(const std::string & text);
      void deleteLine();

      void moveLines(std::vector<Line>::iterator from, std::vector<Line>::iterator to, int dy);


   };

public:
    NMemo();

    ~NMemo();

    virtual std::string text() const;
    void setText(const std::string & text);

    void loadFromFile(const std::string & fileName);
    void saveToFile(const std::string & fileName);

    void setWordWrap(bool on);
    bool wordWrap() const;

    void setReadOnly(bool on);
    bool readOnly() const;

    void clear();

private:

    TextArea* textArea;
    class NScrollBar* vBar;
    class NScrollBar* hBar;

};

#endif
