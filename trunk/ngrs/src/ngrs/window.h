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
#ifndef NWINDOW_H
#define NWINDOW_H

#include "system.h"
#include "panel.h"
#include "iswindow.h"
#include "region.h"

/**
@author Stefan
*/

namespace ngrs {
  
  const int nDestroyWindow  = 1;
  const int nHideWindow     = 2;
  const int nDockWindow     = 3;

  class DockPanel;
  class CustomStatusModel;

  class Window : public Visual
  {
  public:
    Window();
    Window(const Window & topWindow);

    ~Window();

    virtual void setPosition(int x, int y, int width, int height);
    void setPositionToScreenMaximize();
    void setPositionToScreenCenter( );
    void setPositionToScreenTop(int height );

    virtual void setLeft(int left);
    virtual void setTop(int top);
    virtual void setWidth(int width);
    virtual void setHeight(int height);
    virtual void setSize(int width, int height);

    virtual void setVisible(bool on);
    virtual void onMouseExit();
    virtual void onMousePress  (int x, int y, int button);
    virtual void onMousePressed(int x, int y, int button);
    virtual void onMouseDoublePress(int x, int y, int button);
    virtual void onMouseOver(int x, int y);
    virtual void onKeyPress( const KeyEvent & keyEvent );
    virtual void onKeyRelease(const KeyEvent & event);
    virtual void add(Window* window);
    virtual void add( Runtime * component );

    virtual void setFocus();
    virtual void setFocus(class VisualComponent* comp);
    void setGrabEvents(bool on);

    WinHandle win() const;
    Panel* pane();
    Graphics& graphics();
    Graphics& graphics(VisualComponent* comp);

    void repaint(VisualComponent* sender, const ngrs::Region & repaintArea, bool swap = true);

    int width()  const;
    int height() const;

    virtual void setTitle(const std::string & title);
    std::string title();

    void setTitleBitmap( const Bitmap & bmp );

    virtual int onClose();

    void setDecoration(bool on);

    virtual bool visit(NVisitor* v);

    bool mapped();

    virtual int top()  const;
    virtual int left() const;

    void pack();

    static IsWindow* isWindow;

    bool changeState();
    void setChangeState(bool on);


    void setDoubleBuffer(bool on);
    bool doubleBuffered() const;

    void setModal(bool on);
    bool modal() const;

    int exitLoop();
    void setExitLoop(int flag);

    void checkForRemove(Runtime* obj);

    void setMoveFocus(VisualComponent* moveable, int pickPoint);

    VisualComponent* selectedBase() const;

    static bool paintFlag;

    friend class DockPanel;

    void setDock(DockPanel* dock);
    DockPanel* dock();

    const Rect & userGeometry() const;

    virtual void setMinimumWidth( int minWidth );
    virtual void setMinimumHeight( int minHeight );

    void requestSelection();
    virtual void onSelection();


    void setStatusModel( CustomStatusModel & model );
    CustomStatusModel* statusModel() const;


  private:

    bool modal_;
    bool changeState_;
    bool dblBuffer_;

    int dragRectPoint;
    int dragPoint;
    int exitLoop_;

    ngrs::Region oldDrag;

    WinHandle win_;
    Panel* pane_;
    Graphics graphics_;
    VisualComponent* dragBase_;
    VisualComponent* selectedBase_;
    VisualComponent* mousePressBase_;
    VisualComponent* oldFocus_;
    int dragX,dragY,dragOldX,dragOldY;
    Size dragOffset;
    std::string title_;
    VisualComponent* lastOver_;   

    void initDrag(VisualComponent*, int x, int y);
    void doDrag(VisualComponent*, int x, int y);
    void endDrag(VisualComponent*, int x, int y);

    void dragRectPicker(VisualComponent* dragBase, int x, int y, int varx, int vary);

    int oldWidth;
    int oldHeight;

    DockPanel* dock_;


    // user wanted size and position

    Rect userPos;

    void checkGeometry();

    CustomStatusModel* statusModel_;
  };

}

#endif
