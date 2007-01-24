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
#ifndef VISUALCOMPONENT_H
#define VISUALCOMPONENT_H

#include "visual.h"
#include "graphics.h"
#include "rect.h"
#include "isvisualcomponent.h"
#include "color.h"
#include <vector>
#include "size.h"
#include "border.h"
#include "layout.h"
#include "moveable.h"
#include "moveevent.h"
#include "skin.h"
#include "alignconstraint.h"

namespace ngrs {

  class Window;

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
  @author  Stefan
  */
  class VisualComponent : public Visual
  {
  public:
    VisualComponent();

    ~VisualComponent();

    virtual void draw( Graphics& g, const ngrs::Region & repaintArea, VisualComponent* sender );

    signal1<const MoveEvent &> move;
    signal1<const MoveEvent &> moveStart;
    signal1<const MoveEvent &> moveEnd;

    virtual bool visit(NVisitor* v);

    bool transparent() const;
    virtual void setTransparent(bool on);
    void setTranslucent(Color color, int percent);
    int translucent() const;
    const Color & translucentColor();

    virtual void setBackground(const Color & background);
    virtual void setParentBackground(bool on);
    const Color & background();

    void setForeground(const Color & foreground);
    void setParentForeground(bool on);
    const Color & foreground();

    void setSkin(Skin skin);
    void setGradientStyle(int style);
    void setGradientOrientation(int orientation);

    virtual void setScrollDx(int dx);
    virtual void setScrollDy(int dy);
    virtual int scrollDx() const;
    virtual int scrollDy() const;

    void setSpacing(int left, int top, int right, int bottom);
    void setSpacing(const Size & spacing);

    int spacingWidth() const;
    int spacingHeight() const;

    const Size & spacing() const;

    int clientWidth() const;
    int clientHeight() const;

    int viewWidth() const;
    int viewHeight() const;

    virtual void paint(Graphics& g);

    virtual void setFont(const Font & font);
    void setParentFont(bool on);
    const Font & font() const;

    VisualComponent* overObject(Graphics& g, long absX, long absY);
    void setBorder(const Border & border);
    Border* border();
    void setLayout(const Layout & layout);
    Layout* layout();
    Layout* layout() const;

    static IsVisualComponent* isVisualComponent;

    virtual void updateAlign();

    int borderLeft() const;
    int borderRight() const;
    int borderTop() const;
    int borderBottom() const;

    virtual int absoluteLeft() const;
    virtual int absoluteTop() const;

    int absoluteSpacingLeft() const;
    int absoluteSpacingTop() const;
    Rect absoluteGeometry() const;
    Rect absoluteSpacingGeometry() const;

    void setMoveable(const Moveable & moveable);
    const Moveable & moveable() const;

    virtual void onMoveStart(const MoveEvent & moveEvent);
    virtual void onMove(const MoveEvent & moveEvent);
    virtual void onMoveEnd(const MoveEvent & moveEvent);

    virtual void onMouseExit();
    virtual void onMouseEnter();

    virtual void doDrag();
    virtual void onDropEnter();
    virtual void onDrop();
    void setAllowDrop( bool on );
    bool allowDrop() const;

    void repaint(bool swap = true);

    void setWindow(class Window* win);
    Window* window();
    Window* window() const;

    int overRectPickPoint(int x, int y);
    int overPickPoint(int x, int y);

    void setMoveFocus(int pickpoint);

    virtual void resize();

    void setClipping(bool on);

    void setAlign(int align);
    int align() const;
    void setAlignConstraint(const AlignConstraint & constraint);
    AlignConstraint alignConstraint() const;

    virtual int preferredWidth() const;
    virtual int preferredHeight() const;

    void setEvents(bool on);
    bool events() const;

    virtual void setEnable(bool on);
    bool enabled() const;

    int zOrder();
    int componentZOrderSize();
    VisualComponent* componentByZOrder(int zOrder);

    void setClientSizePolicy(int clPolicy);

    virtual void add(Runtime* component);
    virtual void add(VisualComponent* component);
    virtual void add(VisualComponent* component, int align);
    virtual void add(VisualComponent* component, int align, bool update);
    virtual void add(VisualComponent* component, const AlignConstraint & align, bool update);

    virtual void onKeyPress(const KeyEvent & event);

    void insert(VisualComponent* component, unsigned int index);

    Rect blitMove(int dx, int dy, const Rect & area);

    virtual void removeChilds();
    virtual void removeChild( VisualComponent * child );
    std::vector<VisualComponent*>::iterator erase(std::vector<VisualComponent*>::iterator first, std::vector<VisualComponent*>::iterator last);
    virtual void erase(VisualComponent* child);
    virtual void erase();
    virtual void eraseAll();

    const std::vector<VisualComponent*> & visualComponents() const;

    std::vector<VisualComponent*>::iterator vcEnd();
    std::vector<VisualComponent*>::iterator vcBegin();

    virtual void setPreferredSize(int width, int height);

    Skin skin_;

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

    virtual void drawChildren( Graphics& g, const ngrs::Region & repaintArea, VisualComponent* sender );
    // this checks in reverse containerorder the mouse over events
    // override it to use a different order
    virtual VisualComponent* checkChildrenEvent( Graphics& g, int absX, int absY );

    VisualComponent* checkChildEvent( VisualComponent* child, Graphics& g, int absX, int absY );


  private:

    std::vector<VisualComponent*> tabOrder_;

    std::vector<VisualComponent*> visualComponents_;

    bool clipping_;
    bool events_;   

    int scrollDx_, scrollDy_;

    Layout* layout_;

    Moveable moveable_;
    class Window* win_;

    std::string alignStr_;

    int clSzPolicy;

    bool ownerSizeSet_;
    int ownerPreferredWidth_;
    int ownerPreferredHeight_;

    AlignConstraint alignConstraint_;

    bool enabled_;

    Font oldFont;
    bool disableParentFont_;

    bool tabStop_;
    bool focusEnabled_;

    bool allowDrop_;

    void tabRight();
    void tabLeft();
  };

}

#endif
