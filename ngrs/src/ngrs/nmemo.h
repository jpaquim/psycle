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
#ifndef NMEMO_H
#define NMEMO_H

#include "ntextbase.h"
#include "npoint3d.h"
#include <algorithm>

/**
@author Stefan Nattkemper
*/

namespace ngrs {

  class NMemo : public NTextBase
  {
    class TextArea : public NPanel {
      class Line {
      public :		

        Line();
        Line( TextArea* area );
        ~Line();

        void setText( const std::string & text );
        const std::string & text() const;
        void insert( std::string::size_type pos, const std::string & text );
        void erase( unsigned int count );

        void setTop( int top );
        void move(int dy);
        int top() const;
        int height() const;

        void incPos();
        void decPos();
        void setPos( unsigned int pos );
        void setPosEnd();
        void setPosStart();

        std::string::size_type pos() const;

        NPoint3D screenPos() const;
        void setPosToScreenPos(int x, int y);
        bool setPosLineUp(int x);
        bool setPosLineDown(int x);

        int width() const;

        std::string strFromPos() const;
        std::string deleteFromPos();
        std::string strToPos() const;
        std::string deleteToPos();

        void computeBreakPoints();

        void drawText( NGraphics *g );
        void repaint();
        NRect repaintLineArea() const;

        bool operator<( const Line  & rhs ) const;
        // needed for visual c++ express 2005 to avoid error c2784
        friend bool operator<( int top, const Line& r)
        { return top < r.top(); }
        // end of bugfix
        bool operator<( int rhsTop ) const;

      private:

        TextArea* pArea;
        std::string text_;
        int top_;
        int height_;
        std::string::size_type pos_;

        int findWidthMax( long width, const std::string & data, bool wbreak );

        std::vector<std::string::size_type> breakPoints;

      };

    public:
      TextArea();
      TextArea( NMemo* memo );

      ~TextArea();

      void setText(const std::string & text);
      std::string text() const;

      virtual void paint(NGraphics* g);
      virtual void onKeyPress(const NKeyEvent & keyEvent);
      virtual void onMousePress(int x, int y, int button);

      void setWordWrap(bool on);
      bool wordWrap() const;

      void setReadOnly(bool on);
      bool readOnly() const;

      void appendLine(const std::string & text);
      void clear();

      void loadFromFile(const std::string & fileName);

      NRect selection() const;

      virtual int preferredWidth() const;
      virtual int preferredHeight() const;

      virtual void resize();

    private:

      NMemo* pMemo;
      bool wordWrap_;
      bool readOnly_;
      int xupdownpos;
      NRect selection_;

      std::vector<Line>::iterator lineIndexItr;
      std::vector<Line> lines;

      void init();
      void drawCursor( NGraphics* g, int x, int y );

      void insertLine( const std::string & text );
      void deleteLine();

      void moveLines( std::vector<Line>::iterator from, std::vector<Line>::iterator to, int dy );


    };

  public:
    NMemo();

    ~NMemo();

    virtual std::string text( ) const;
    void setText( const std::string & text );

    void loadFromFile( const std::string & fileName );
    void saveToFile( const std::string & fileName );

    void setWordWrap( bool on );
    bool wordWrap() const;

    void setReadOnly( bool on );
    bool readOnly( ) const;

    void clear( );

    virtual void resize( );

  private:

    TextArea* textArea;
    class NScrollBar* vBar;
    class NScrollBar* hBar;

  };

}

#endif
