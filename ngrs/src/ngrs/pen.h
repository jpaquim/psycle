/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper  *
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
#ifndef NPEN_H
#define NPEN_H

#ifdef __unix__
#include <X11/Xlib.h>
#else
#include <windows.h>
#endif

namespace ngrs {

#ifdef __unix__
  enum LineStyle {nLineSolid          = LineSolid,
    nLineOnOffDash      = LineOnOffDash,
    nLineLineDoubleDash = LineDoubleDash };

  enum CapStyle  {nCapNotLast     =  CapNotLast,
    nCapButt        =  CapButt,
    nCapRound       =  CapRound,
    nCapProjecting  =  CapProjecting};

  enum JoinStyle { nJoinMiter = JoinMiter,
    nJoinRound = JoinRound,
    nJoinBevel = JoinBevel };

  enum FillStyle { nFillSolid          = FillSolid,
    nFillTiled          = FillTiled,
    nFillStippled       = FillStippled,
    nFillOpaqueStippled = FillOpaqueStippled };

  enum LogicStyle { nClear        = GXclear,
    nAnd          = GXand,
    nAndReverse   = GXandReverse,
    nCopy         = GXcopy,
    nAndInverted  = GXandInverted,
    nNoop         = GXnoop,
    nXor          = GXxor,
    nOr           = GXor,
    nNor          = GXnor,
    nEquiv        = GXequiv,
    nInvert       = GXinvert,
    nOrReverse    = GXorReverse,
    nCopyInverted = GXcopyInverted,
    nXorInverted  = GXorInverted,
    nNand         = GXnand,
    nSet          = GXset };

#else

  enum LineStyle {
    nLineSolid          = PS_SOLID,     // The pen is solid
    nLineOnOffDash      = PS_DOT,       // The pen is dashed
    nLineLineDoubleDash = PS_DASHDOTDOT // The pen has dashes and double dots
  };


  enum CapStyle  {nCapNotLast     =  0,
    nCapButt        =  1,
    nCapRound       =  2,
    nCapProjecting  =  3};

  enum JoinStyle { nJoinMiter = 0,
    nJoinRound = 1,
    nJoinBevel = 2 };

  enum FillStyle { nFillSolid          = 0,
    nFillTiled          = 1,
    nFillStippled       = 2,
    nFillOpaqueStippled = 3 };

  enum LogicStyle { nClear        = 0,
    nAnd          = 1,
    nAndReverse   = 2,
    nCopy         = 3,
    nAndInverted  = 4,
    nNoop         = 5,
    nXor          = 6,
    nOr           = 7,
    nNor          = 8,
    nEquiv        = 9,
    nInvert       = 10,
    nOrReverse    = 11,
    nCopyInverted = 12,
    nXorInverted  = 13,
    nNand         = 14,
    nSet          = 15 };

#endif

  /**
  @author  Stefan Nattkemper
  */

  class Pen{
  public:
    Pen();

    ~Pen();

    void setLineWidth(int width);
    int lineWidth() const;

    void setLineStyle(LineStyle style);
    LineStyle lineStyle() const;

    void setCapStyle(CapStyle style);
    CapStyle capStyle() const;

    void setJoinStyle(JoinStyle style);
    JoinStyle joinStyle() const;

    void setFillStyle(FillStyle style);
    FillStyle fillStyle() const;

    void setFunction(int function); // sets the logical flags
    int function() const;

  private:

    int lineWidth_;
    LineStyle lineStyle_;
    CapStyle  capStyle_;
    JoinStyle joinStyle_;
    FillStyle fillStyle_;
    int function_;

  };

}

#endif
