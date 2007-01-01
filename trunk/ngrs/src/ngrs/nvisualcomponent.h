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
#ifndef NVISUALCOMPONENT_H
#define NVISUALCOMPONENT_H

#include "nvisual.h"
#include "ngraphics.h"
#include "nrect.h"
#include "nisvisualcomponent.h"
#include "ncolor.h"
#include <vector>
#include "nsize.h"
#include "nborder.h"
#include "nlayout.h"
#include "nmoveable.h"
#include "nmoveevent.h"
#include "nskin.h"
#include "nalignconstraint.h"

class NWindow;

const int nCrossCorner       = 9;
const int nUpperLeftCorner   = 1;
const int nUpperMiddleCorner = 2;
const int nUpperRightCorner  = 3;
const int nRightMiddleCorner = 4;
const int nLowerRightCorner  = 5;
const int nLowerMiddleCorner = 6;
const int nLowerLeftCorner   = 7;
const int nLeftMiddleCorner  = 8;

/**
@author Stefan
*/
class NVisualComponent : public NVisual
{
public:
    NVisualComponent();

    ~NVisualComponent();

    virtual void draw( NGraphics* g, const NRegion & repaintArea, NVisualComponent* sender );

    signal1<const NMoveEvent &> move;
    signal1<const NMoveEvent &> moveStart;
    signal1<const NMoveEvent &> moveEnd;

    virtual bool visit(NVisitor* v);

    bool transparent() const;
    virtual void setTransparent(bool on);
    void setTranslucent(NColor color, int percent);
    int translucent() const;
    const NColor & translucentColor();

    virtual void setBackground(const NColor & background);
    virtual void setParentBackground(bool on);
    const NColor & background();

    void setForeground(const NColor & foreground);
    void setParentForeground(bool on);
    const NColor & foreground();

    void setSkin(NSkin skin);
    void setGradientStyle(int style);
    void setGradientOrientation(int orientation);

    virtual void setScrollDx(int dx);
    virtual void setScrollDy(int dy);
    virtual int scrollDx() const;
    virtual int scrollDy() const;

    void setSpacing(int left, int top, int right, int bottom);
    void setSpacing(const NSize & spacing);

    int spacingWidth() const;
    int spacingHeight() const;

    const NSize & spacing() const;

    int clientWidth() const;
    int clientHeight() const;

    int viewWidth() const;
    int viewHeight() const;

    virtual void paint(NGraphics* g);

    virtual void setFont(const NFont & font);
    void setParentFont(bool on);
    const NFont & font() const;

    NVisualComponent* overObject(NGraphics* g, long absX, long absY);
    void setBorder(const NBorder & border);
    NBorder* border();
    void setLayout(const NLayout & layout);
    NLayout* layout();
    NLayout* layout() const;

    static NIsVisualComponent* isVisualComponent;

    virtual void updateAlign();

    int borderLeft() const;
    int borderRight() const;
    int borderTop() const;
    int borderBottom() const;

    virtual int absoluteLeft() const;
    virtual int absoluteTop() const;

    int absoluteSpacingLeft() const;
    int absoluteSpacingTop() const;
    NRect absoluteGeometry() const;
    NRect absoluteSpacingGeometry() const;

    void setMoveable(const NMoveable & moveable);
    const NMoveable & moveable() const;

    virtual void onMoveStart(const NMoveEvent & moveEvent);
    virtual void onMove(const NMoveEvent & moveEvent);
    virtual void onMoveEnd(const NMoveEvent & moveEvent);

    virtual void onMouseExit();
    virtual void onMouseEnter();


    void repaint(bool swap = true);

    void setWindow(class NWindow* win);
    NWindow* window();
    NWindow* window() const;

    int overRectPickPoint(int x, int y);
    int overPickPoint(int x, int y);

    void setMoveFocus(int pickpoint);

    virtual void resize();

    void setClipping(bool on);

    void setAlign(int align);
    int align() const;
    void setAlignConstraint(const NAlignConstraint & constraint);
    NAlignConstraint alignConstraint() const;

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    void setEvents(bool on);
    bool events() const;

    virtual void setEnable(bool on);
    bool enabled() const;

    int zOrder();
    int componentZOrderSize();
    NVisualComponent* componentByZOrder(int zOrder);

    void setClientSizePolicy(int clPolicy);

    virtual void add(NRuntime* component);
    virtual void add(NVisualComponent* component);
    virtual void add(NVisualComponent* component, int align);
    virtual void add(NVisualComponent* component, int align, bool update);
    virtual void add(NVisualComponent* component, const NAlignConstraint & align, bool update);

    virtual void onKeyPress(const NKeyEvent & event);

    void insert(NVisualComponent* component, unsigned int index);

    NRect blitMove(int dx, int dy, const NRect & area);

    virtual void removeChilds();
    virtual void removeChild( NVisualComponent * child );
    std::vector<NVisualComponent*>::iterator erase(std::vector<NVisualComponent*>::iterator first, std::vector<NVisualComponent*>::iterator last);
    virtual void erase(NVisualComponent* child);
    virtual void erase();

    const std::vector<NVisualComponent*> & visualComponents() const;

    std::vector<NVisualComponent*>::iterator vcEnd();
    std::vector<NVisualComponent*>::iterator vcBegin();

    virtual void setPreferredSize(int width, int height);

    NSkin skin_;

    bool ownerSize() const;
    int  ownerWidth() const;
    int  ownerHeight() const;

    virtual void setFocus();
    virtual bool focus() const;

    void enableFocus( bool on );
    bool focusEnabled() const;

    void setTabOrder( int index );
    int tabOrder() const;

    void setTabStop( bool on );
    bool tabStop() const;

    virtual void onExit();
    virtual void onEnter();
    
protected:
          
   virtual void drawChildren( NGraphics* g, const NRegion & repaintArea, NVisualComponent* sender );
   // this checks in reverse containerorder the mouse over events
   // override it to use a different order
   virtual NVisualComponent* checkChildrenEvent( NGraphics* g, int absX, int absY );

   NVisualComponent* checkChildEvent( NVisualComponent* child, NGraphics* g, int absX, int absY );
   

private:

   std::vector<NVisualComponent*> tabOrder_;

   std::vector<NVisualComponent*> visualComponents_;

   bool clipping_;
   bool events_;   

   int scrollDx_, scrollDy_;

   NLayout* layout_;

   NMoveable moveable_;
   class NWindow* win_;

   std::string alignStr_;

   int clSzPolicy;

   bool ownerSizeSet_;
   int ownerPreferredWidth_;
   int ownerPreferredHeight_;

   NAlignConstraint alignConstraint_;

   bool enabled_;

   NFont oldFont;
   bool disableParentFont_;

   bool tabStop_;
   bool focusEnabled_;

   void tabRight();
   void tabLeft();
};

#endif
