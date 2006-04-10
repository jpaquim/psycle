/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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

#include <nsystem.h>
#include <npanel.h>
#include <nalignlayout.h>
#include <niswindow.h>

#define nDestroyWindow 1
#define nHideWindow    2

/**
@author Stefan
*/
class NWindow : public NVisual
{
public:
    NWindow();
    NWindow(const NWindow & topWindow);

    ~NWindow();

    virtual void setPosition(int x, int y, int width, int height);
    void setPositionToScreenCenter( );
    virtual void setLeft(int left);
    virtual void setTop(int top);
    virtual void setWidth(int width);
    virtual void setHeight(int height);


    virtual void setVisible(bool on);
    virtual void onMouseExit();
    virtual void onMousePress  (int x, int y, int button);
    virtual void onMousePressed(int x, int y, int button);
    virtual void onMouseDoublePress(int x, int y, int button);
    virtual void onMouseOver(int x, int y);
    virtual void onKeyPress( const NKeyEvent & keyEvent );
    virtual void onKeyRelease(const NKeyEvent & event);
    virtual void add(NWindow* window);
    virtual void add( NRuntime * component );

    void setGrabEvents(bool on);

    Window win() const;
    NPanel* pane();
    NGraphics* graphics();
    NGraphics* graphics(NVisualComponent* comp);

    void repaint(NVisualComponent* sender, int x, int y, int w, int h  , bool swap = true);
    void repaint(NVisualComponent* sender, const NRect & repaintArea, bool swap = true);

    int width()  const;
    int height() const;

    virtual void setTitle(std::string title);
    std::string title();

    virtual int onClose();

    void setDecoration(bool on);

    virtual bool visit(NVisitor* v);

    bool mapped();

    virtual long top()  const;
    virtual long left() const;

    void pack();

    static NIsWindow* isWindow;

    bool changeState();
    void setChangeState(bool on);


    void setDoubleBuffer(bool on);
    bool doubleBuffered();

    void setModal(bool on);
    bool modal();

    int exitLoop();
    void setExitLoop(int flag);

    void checkForRemove(NVisualComponent* obj);

    void setMoveFocus(NVisualComponent* moveable, int pickPoint);

    NVisualComponent* selectedBase() const;

    static bool paintFlag;

private:

   bool modal_;
   bool changeState_;
   bool dblBuffer_;

   int dragRectPoint;
   int dragPoint;
   int exitLoop_;

   Window win_;
   NPanel* pane_;
   NGraphics* graphics_;
   NVisualComponent* dragBase_;
   NVisualComponent* selectedBase_;
   int dragX,dragY,dragOldX,dragOldY;
   NSize dragOffset;
   std::string title_;
   NVisualComponent* lastOver_;

   void initDrag(NVisualComponent*, int x, int y);
   void doDrag(NVisualComponent*, int x, int y);
   void endDrag(NVisualComponent*, int x, int y);

   void dragRectPicker(NVisualComponent* dragBase, int x, int y, int varx, int vary);

   int oldWidth;
   int oldHeight;

};

#endif
