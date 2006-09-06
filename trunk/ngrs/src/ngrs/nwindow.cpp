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
#include "nwindow.h"
#include "napp.h"
#include "nsystem.h"
#include "natoms.h"
#include "nconfig.h"
#include "ndockpanel.h"
#include "nalignlayout.h"
//#include <X11/extensions/Xinerama.h>

NIsWindow* NWindow::isWindow = new NIsWindow();
bool NWindow::paintFlag = true;

NWindow::NWindow()
 : NVisual()
{
  changeState_ = true;
  dblBuffer_ = true;
  win_ = NApp::system().registerWindow(NApp::system().rootWindow());
  userPos.setPosition(0,0,200,200);
  NApp::addWindow(win_, this);
  pane_ = new NPanel();
    pane_->setLayout(NAlignLayout());
    pane_->skin_ = NApp::config()->skin("pane");
    pane_->setName("window_pane");
  add(pane_);
  graphics_ = new NGraphics(win_);
  dragBase_ = 0;
  dragRectPoint = 0;
  setTitle("Window");
  selectedBase_ = lastOver_ = 0;
  modal_ = false;
  NVisual::setVisible(false);
  dock_ = 0;
  exitLoop_ = nDestroyWindow;
  oldFocus_ = 0;
  mousePressBase_ = 0;
}


NWindow::~NWindow()
{
  if (mapped()) NApp::system().unmapWindow(win_);
  delete graphics_;
  NApp::removeWindow(win_);
  NApp::system().destroyWindow(win_);
}

void NWindow::setVisible( bool on )
{
  NVisual::setVisible(on);
  if (on) {
      graphics_->setVisible(on);
      pane_->resize();
      repaint(pane(),NRect(0,0,width(),height()));
      if (modal_) {
         NApp::system().setModalMode( win() );
         //NApp::system().setStayAbove(win());
         NApp::system().setWindowPosition(win(), userPos.left(),userPos.top(),userPos.width(),userPos.height());
         NApp::system().setWindowMinimumSize(win(),minimumWidth(), minimumHeight());
         XMapWindow(NApp::system().dpy(),win_);
         checkGeometry();
         NApp::runModal(this);
      } else {
        XSync(NApp::system().dpy(),false);
        NApp::system().setWindowPosition(win(), userPos.left(),userPos.top(),userPos.width(),userPos.height());
        NApp::system().setWindowMinimumSize(win(),minimumWidth(), minimumHeight());
        XMapWindow(NApp::system().dpy(),win_);
        checkGeometry();
        XSync(NApp::system().dpy(),false);
      }
  } else if (mapped()) {
     if (lastOver_!=0) {
        lastOver_->onMouseExit();
        lastOver_ = 0;
        dragBase_ = 0;
     }
     XSync(NApp::system().dpy(),false);
     NApp::system().unmapWindow(win_);
     XSync(NApp::system().dpy(),false);
     graphics_->setVisible(on);
  }
}

Window NWindow::win( ) const
{
  return win_;
}

NPanel * NWindow::pane( )
{
  return pane_;
}


void NWindow::repaint( NVisualComponent * sender, const NRegion & repaintArea, bool swap )
{
  if (visible()) {
    paintFlag = false;

    if (sender->transparent()) {
     // find last non transparent

     while (sender->transparent() && sender->skin_.gradientStyle()==0 && sender !=pane() && sender->parent() != 0) {
       sender = static_cast<NVisualComponent*> (sender->parent());
      }
    }

    if (pane_->width() !=width() || pane_->height() !=height())
      pane_->setPosition(0,0,width(),height());
    graphics_->setRegion(repaintArea);
    pane_->draw(graphics_,repaintArea,sender);
    if (dblBuffer_ && swap) graphics()->swap(repaintArea.rectClipBox());
  }
}


NGraphics * NWindow::graphics( )
{
  return graphics_;
}

int NWindow::width( ) const
{
  XWindowAttributes attr;
  XGetWindowAttributes( NApp::system().dpy(), win_, &attr );
  return attr.width;
}

int NWindow::height( ) const
{
  XWindowAttributes attr;
  XGetWindowAttributes( NApp::system().dpy(), win_, &attr );
  return attr.height;
}

void NWindow::onMousePress( int x, int y, int button )
{
  oldFocus_ = 0;

  graphics_->setRegion(NRect(0,0,width(),height()));
  NVisualComponent* obj = pane()->overObject(graphics(),x,y);
  if (obj!=NULL) {
    // send mousepress
    if ( obj->enabled() ) {
      oldFocus_ = selectedBase_;
      selectedBase_ = obj;
      mousePressBase_ = obj;
      if (selectedBase_->focusEnabled() && oldFocus_ && oldFocus_ != selectedBase_) oldFocus_->onExit();
      if (selectedBase_) {
        selectedBase_->onMousePress( x - selectedBase_->absoluteSpacingLeft() +
        selectedBase_->scrollDx(), y - selectedBase_->absoluteSpacingTop() + selectedBase_->scrollDy(), button);
      }

      // end of mousepress
      if (selectedBase_ && selectedBase_!=pane_ && (mapped())) {
        if (button==1) {
          dragBase_ = selectedBase_;
          initDrag(obj,x,y);
        }
      }
    }

  }
}

void NWindow::onMousePressed( int x, int y, int button )
{
  if (mousePressBase_!=0 && mousePressBase_->enabled()) mousePressBase_->onMousePressed( x - mousePressBase_->absoluteSpacingLeft(), y - mousePressBase_->absoluteSpacingTop(), button);
  endDrag(dragBase_,x,y);
  dragBase_ = 0;
  if ( selectedBase_ && selectedBase_->enabled() && selectedBase_->focusEnabled() ) selectedBase_->onEnter(); 
   else
  if ( oldFocus_ ) selectedBase_ = oldFocus_;

}

void NWindow::onMouseOver( int x, int y )
{
  graphics_->setRegion(NRect(0,0,width(),height()));
  if (dragBase_!=NULL) {
    NApp::mouseOverWindow();
    if (dragBase_->moveable().style()!=0) doDrag(dragBase_,x,y);
    dragBase_->onMouseOver( x - dragBase_->absoluteSpacingLeft(), y - dragBase_->absoluteSpacingTop());
  } else {
    NVisualComponent* over = pane_->overObject(graphics_,x,y);
    if (lastOver_!=0 && over!=lastOver_ && lastOver_->enabled()) lastOver_->onMouseExit();
    if (over!=0 && over->enabled()) {
       if (over!=lastOver_ || lastOver_==0 && selectedBase_->enabled()) over->onMouseEnter();
       over->onMouseOver( x - over->absoluteSpacingLeft(), y - over->absoluteSpacingTop());
    }
    lastOver_ = over;
  }
}

void NWindow::initDrag( NVisualComponent * dragBase, int x, int y )
{
  oldDrag = dragBase->geometry()->region();

  dragBase_->onMoveStart(NMoveEvent(x - dragBase_->absoluteSpacingLeft() +
        dragBase_->scrollDx(), y - dragBase_->absoluteSpacingTop() + dragBase_->scrollDy()));
  dragX = x;
  dragY = y;
  dragRectPoint = 0;
  dragPoint = -1;
  dragOldX = x;
  dragOldY= y;

  dragOffset.setSize(dragBase_->absoluteLeft()-x,dragBase_->absoluteTop()-y,dragBase_->absoluteLeft() + dragBase_->width() - x,dragBase_->absoluteTop()  + dragBase_->height()- y);

  if (dragBase->moveable().style() & nMvRectPicker) dragRectPoint = dragBase->overRectPickPoint(x , y);
	
	if (dragBase->moveable().style() & nMvLeftBorder) {
		if ( abs(dragBase_->absoluteLeft()- x) <=5 ) dragRectPoint = nLeftMiddleCorner; else
		if ( abs(dragBase_->absoluteLeft() + dragBase_->width() - x) <=5 ) dragRectPoint = nRightMiddleCorner;
	} 

  dragPoint = dragBase->overPickPoint(x,y);
}

void NWindow::doDrag( NVisualComponent *, int x, int y )
{
  dragX = x;
  dragY = y;
  int pickOffset = 0 ;
  int varx=abs(dragOldX-dragX)+pickOffset;
  int vary=abs(dragOldY-dragY)+pickOffset;
  dragOldX = x;
  dragOldY = y;
  NVisualComponent* dragBase = dragBase_;
  if (dragBase_->parent()->visit(NVisualComponent::isVisualComponent)) {
     NVisualComponent* dragBaseParent = (NVisualComponent*) dragBase->parent();
     int newLeft = x+dragOffset.left() - dragBaseParent->absoluteSpacingLeft()+ dragBaseParent->scrollDx();
     int newTop  = y+dragOffset.top()  - dragBaseParent->absoluteSpacingTop() + dragBaseParent->scrollDy();

     if (dragRectPoint!=0) dragRectPicker(dragBase,x,y,varx,vary); else

     if (dragPoint != -1) {
         dragBase_->geometry()->setPicker(dragPoint, x - dragBaseParent->absoluteSpacingLeft() + dragBaseParent->scrollDx(), y - dragBaseParent->absoluteSpacingTop() + dragBaseParent->scrollDy());
         NRegion newDrag = dragBase->geometry()->region();
         NRegion repaintArea = newDrag | oldDrag;
         repaintArea.move(dragBaseParent->absoluteLeft() - dragBaseParent->scrollDx() ,dragBaseParent->absoluteTop() - dragBaseParent->scrollDy() );
         repaintArea.shrink(-5,-5); // this is done, cause pickpoints can be outside
                                    // the component region
         repaint(pane(),repaintArea);
         oldDrag = newDrag;
     } else {

     if (dragBase->moveable().style() & nMvHorizontal)  {
        if ((dragBase->moveable().style() & nMvParentLimit && newLeft<0) ||
            (dragBase->moveable().style() & nMvLeftLimit && newLeft<0)) {
           if (dragBase->left()!=0) dragBase->setLeft(0);
         } else
         if (dragBase->moveable().style() & nMvParentLimit && newLeft > dragBaseParent->spacingWidth()-dragBase->width()) {
           if (dragBase->left()+dragBase->width()!=dragBaseParent->spacingWidth()-dragBase->width())
             dragBase->setLeft(dragBaseParent->spacingWidth()-dragBase->width());
         } else dragBase->setLeft(newLeft);
       } else varx=0;

       if (dragBase->moveable().style() & nMvVertical )  {
       if ((dragBase->moveable().style() & nMvParentLimit && newTop<0) ||
           (dragBase->moveable().style() & nMvTopLimit && newTop<0)) {
           if (dragBase->top()!=0) dragBase->setTop(0);
         } else 
         if (dragBase->moveable().style() & nMvParentLimit && newTop > dragBaseParent->spacingHeight()-dragBase_->height()) {
           if (dragBase->top()+dragBase->height()!=dragBaseParent->spacingHeight()-dragBase->height()) 
             dragBase->setTop(dragBaseParent->spacingHeight()-dragBase_->height());
         } else dragBase_->setTop(newTop);
    } else vary=0;
    dragBase->onMove( NMoveEvent(x - dragBase_->absoluteSpacingLeft() +
        dragBase_->scrollDx(), y - dragBase_->absoluteSpacingTop() + dragBase_->scrollDy()) );
    if (!(dragBase_->moveable().style() & nMvNoneRepaint)) {
       NRegion newDrag = dragBase->geometry()->region();
       NRegion repaintArea = newDrag | oldDrag;
       repaintArea.move(dragBaseParent->absoluteLeft() - dragBaseParent->scrollDx(),dragBaseParent->absoluteTop() - dragBaseParent->scrollDy());
       repaint(pane(),repaintArea);
       oldDrag = newDrag;
    }
     }
   }
}


void NWindow::endDrag( NVisualComponent *, int x, int y )
{
   if (dragBase_!=0) dragBase_->onMoveEnd( NMoveEvent(x - dragBase_->absoluteSpacingLeft() +
        dragBase_->scrollDx(), y - dragBase_->absoluteSpacingTop() + dragBase_->scrollDy()));
   dragBase_ = NULL;
}

void NWindow::setTitle( std::string title )
{
  title_ = title;
  XSetStandardProperties(NApp::system().dpy(), win_, title.c_str(), title.c_str(), 0, NULL, 0, NULL);	
}

std::string NWindow::title( )
{
  return title_;
}

int NWindow::onClose( )
{
  if (dock_!=0) {
     return nDockWindow;
  }
  exitLoop_ = nDestroyWindow;
  setVisible(false);
  std::cout << nDestroyWindow << std::endl;
  return nDestroyWindow;
}

void NWindow::dragRectPicker( NVisualComponent * dragBase, int x, int y, int varx, int vary )
{
    if (dragBase_->parent()->visit(NVisualComponent::isVisualComponent)) {
     NVisualComponent* dragBaseParent = (NVisualComponent*) dragBase->parent();


     switch (dragRectPoint) {
        case nUpperLeftCorner   : dragBase->setWidth(dragBase->width() - (x+dragOffset.left() - dragBase->absoluteLeft()) );
                                  dragBase->setLeft(x+dragOffset.left() - dragBaseParent->absoluteSpacingLeft()+dragBaseParent->scrollDx());
                                  dragBase->setHeight(dragBase->height() - (y+dragOffset.top() - dragBase->absoluteTop()) );
                                  dragBase->setTop (y+dragOffset.top() - dragBaseParent->absoluteSpacingTop()+dragBaseParent->scrollDy());
                                 break;
       case nUpperMiddleCorner :
                                  dragBase->setHeight(dragBase->height() - (y+dragOffset.top() - dragBase->absoluteTop()) );
                                  dragBase->setTop (y+dragOffset.top() - dragBaseParent->absoluteSpacingTop()+dragBaseParent->scrollDy());
                                 break;
        case nUpperRightCorner  :
                                  dragBase->setWidth(x+dragOffset.right() - dragBase->absoluteLeft());
                                  dragBase->setHeight(dragBase->height() - (y+dragOffset.top() - dragBase->absoluteTop()) );
                                  dragBase->setTop (y+dragOffset.top() - dragBaseParent->absoluteSpacingTop()+dragBaseParent->scrollDy());
                                 break;
        case nRightMiddleCorner :
                                  dragBase->setWidth(x+dragOffset.right()-dragBase->absoluteLeft());
                                  break;
        case nLowerRightCorner  : dragBase->setHeight(y+dragOffset.bottom() - dragBase->absoluteTop() );
                                  dragBase->setWidth(x+dragOffset.right() - dragBase->absoluteLeft());
                                  break;
        case nLowerMiddleCorner : dragBase->setHeight(y+dragOffset.bottom() - dragBase->absoluteTop() );
                                  break;
        case nLowerLeftCorner   : dragBase->setHeight(y+dragOffset.bottom() - dragBase->absoluteTop() );
                                  dragBase->setWidth(dragBase->width() - (x+dragOffset.left() - dragBase->absoluteLeft()) );
                                  dragBase->setLeft(x+dragOffset.left() - dragBaseParent->absoluteSpacingLeft()+dragBaseParent->scrollDx());
                                  break;
        case nLeftMiddleCorner  : dragBase->setWidth(dragBase->width() - (x+dragOffset.left() - dragBase->absoluteLeft()) );
                                  dragBase->setLeft(x+dragOffset.left() - dragBaseParent->absoluteSpacingLeft()+dragBaseParent->scrollDx());
                                  break;
     }
     if (!(dragBase_->moveable().style() & nMvNoneRepaint)) {
        repaint(pane(),NRect(dragBase->absoluteLeft()-varx,dragBase->absoluteTop()-vary,dragBase->width()+2*varx,dragBase->height()+2*vary));
     }
   }
}

void NWindow::onKeyPress( const NKeyEvent & keyEvent )
{
  if ( selectedBase_ != NULL && selectedBase_->enabled() ) selectedBase_->onKeyPress(NKeyEvent(selectedBase_,keyEvent.buffer(),keyEvent.scancode()));
}

void NWindow::onKeyRelease( const NKeyEvent & keyEvent )
{
  if (selectedBase_ != NULL && selectedBase_->enabled() ) selectedBase_->onKeyRelease(NKeyEvent(selectedBase_,keyEvent.buffer(),keyEvent.scancode()));
}


void NWindow::setPosition( int x, int y, int width, int height )
{
  userPos.setPosition(x,y,width,height);
  NApp::system().setWindowPosition(win_,x,y,width,height);
}

void NWindow::setLeft( int left )
{
  setPosition(left, top(), width(), height());
}

void NWindow::setTop( int top )
{
  setPosition(left(), top, width(), height());
}

void NWindow::setWidth( int width )
{
  setPosition(left(), top(), width, height());
}

void NWindow::setHeight( int height )
{
  setPosition(left(), top(), width(), height);
}

void NWindow::add( NWindow * window )
{
  NRuntime::add(window);
  NApp::addWindow(window->win(),window);
}

void NWindow::setDecoration( bool on )
{
  NApp::system().setWindowDecoration(win(),on);
}

bool NWindow::visit( NVisitor * v )
{
  return v->visit_window(this);
}

bool NWindow::mapped( )
{
  return NApp::system().isWindowMapped(win_);
}

int NWindow::top( ) const
{
  return NApp::system().windowTop(win_);
}

int NWindow::left( ) const
{
  return NApp::system().windowLeft(win_);
}

void NWindow::pack( )
{
  if (pane()->layout()!=NULL) {
    pane()->layout()->align(pane());
    std::cout << "pack-width:" << pane()->layout()->preferredWidth(pane()) << std::endl;
    int pW = std::max( pane()->layout()->preferredWidth(pane()) +
                       pane()->spacing().left() + pane()->spacing().right() ,10);
    int pH = std::max( pane()->layout()->preferredHeight(pane()) +
                       pane()->spacing().top() + pane()->spacing().bottom()
                       ,10);
     setPosition(left(),top(),pW,pH);
     setMinimumWidth(pW);
     setMinimumHeight(pH);
  }
}

void NWindow::add( NRuntime * component )
{
  NRuntime::add(component);
}

void NWindow::onMouseExit( )
{
  if (lastOver_!=0) {
     lastOver_->onMouseExit();
     lastOver_=0;
  }
}

void NWindow::setGrabEvents(bool on )
{
  NApp::system().setWindowGrab(win_,on);
}

bool NWindow::changeState( )
{
  return changeState_;
}

void NWindow::setChangeState( bool on )
{
  changeState_ = on;
}

NWindow::NWindow( const NWindow & topWindow )
{
  changeState_ = true;
  dblBuffer_ = true;
  win_ = NApp::system().registerWindow(topWindow.win());
  NApp::addWindow(win_, this);
  pane_ = new NPanel();
  pane_->setTransparent(false);
  add(pane_);
  pane_->setBackground(NColor(255,255,255));
  pane_->setLayout(NAlignLayout());
  graphics_ = new NGraphics(win_);
  dragBase_ = 0;
  dragRectPoint = 0;
  setTitle("Window");
  selectedBase_ = lastOver_ = 0;
  NVisual::setVisible(false);
}

void NWindow::setDoubleBuffer( bool on )
{
  dblBuffer_ = on;
  graphics_->setDoubleBuffer(on);
}

bool NWindow::doubleBuffered( )
{
  return dblBuffer_;
}

void NWindow::setModal( bool on )
{
  modal_ = on;
}

bool NWindow::modal( )
{
  return modal_;
}

int NWindow::exitLoop( )
{
  return exitLoop_;
}

void NWindow::setExitLoop( int flag )
{
  exitLoop_ = flag;
}

void NWindow::setPositionToScreenCenter( )
{
  // todo xinerama display width / height for a screen
  setPosition( (NApp::system().screenWidth() - width()) / 2,  (NApp::system().screenHeight() - height()) / 2, width(), height());
}

void NWindow::checkForRemove( NRuntime * obj )
{
  if (obj == 0) {
     lastOver_ = 0;
     dragBase_ = 0;
     selectedBase_ = 0;
     oldFocus_ = 0;
     mousePressBase_ = 0;
  } else
  if ( oldFocus_ == obj || (oldFocus_ && oldFocus_->isChildOf(obj)))  {
		lastOver_ = 0;
    dragBase_ = 0;
    selectedBase_ = 0;
    mousePressBase_ = 0;
    oldFocus_ = 0;
	} else
  if ( mousePressBase_ == obj || (mousePressBase_ && mousePressBase_->isChildOf(obj)))  {
     lastOver_ = 0;
     dragBase_ = 0;
     selectedBase_ = 0;
     mousePressBase_ = 0;
     oldFocus_ = 0;
  } else
  if ( lastOver_ == obj || (lastOver_ && lastOver_->isChildOf(obj)))  {
     lastOver_ = 0;
     dragBase_ = 0;
     selectedBase_ = 0;
     mousePressBase_ = 0;
     oldFocus_ = 0;
  } else
  if ( dragBase_ == obj || (dragBase_ && dragBase_->isChildOf(obj)))    {
     lastOver_ = 0;
     dragBase_ = 0;
     selectedBase_ = 0;
     mousePressBase_ = 0;
     oldFocus_ = 0;
  } else
  if ( selectedBase_ == obj || (selectedBase_ && selectedBase_->isChildOf(obj))) {
    lastOver_ = 0;
    dragBase_ = 0;
    selectedBase_ = 0;
    mousePressBase_ = 0;
    oldFocus_ = 0;
  }
}

void NWindow::setMoveFocus( NVisualComponent * moveable, int pickPoint )
{
  dragBase_ = moveable;

  int x = moveable->absoluteLeft();
  int y = moveable->absoluteTop();

  dragBase_->onMoveStart( NMoveEvent(x - dragBase_->absoluteSpacingLeft() +
        dragBase_->scrollDx(), y - dragBase_->absoluteSpacingTop() + dragBase_->scrollDy()));
  dragX = x;
  dragY = y;
  dragRectPoint = 0;
  dragPoint = pickPoint;
  dragOldX = x;
  dragOldY= y;

  dragOffset.setSize(dragBase_->absoluteLeft()-x,dragBase_->absoluteTop()-y,dragBase_->absoluteLeft() + dragBase_->width() - x,dragBase_->absoluteTop()  + dragBase_->height()- y);

}

NGraphics * NWindow::graphics( NVisualComponent * comp )
{
  NRegion region = comp->geometry()->region();
  region.move(comp->absoluteLeft()-comp->left(),comp->absoluteTop()-comp->top());
  graphics_->setRegion(region);
  graphics_->setClipping(region);
  graphics_->setTranslation(comp->absoluteLeft()-comp->scrollDx(),comp->absoluteTop()-comp->scrollDy());
  return graphics_;
}

void NWindow::onMouseDoublePress( int x, int y, int button )
{
  graphics_->setRegion(NRect(0,0,width(),height()));
  NVisualComponent* obj = pane()->overObject(graphics(),x,y);
  if (obj) obj->onMouseDoublePress(x - obj->absoluteSpacingLeft(), y - obj->absoluteSpacingTop(), button);
}

NVisualComponent* NWindow::selectedBase( ) const
{
  return selectedBase_;
}

void NWindow::setDock( NDockPanel * dock )
{
  dock_ = dock;
}

NDockPanel * NWindow::dock( )
{
  return dock_;
}

void NWindow::setFocus( )
{

}

void NWindow::setFocus( NVisualComponent * comp )
{
  if ( comp ) {
    NVisualComponent* oldFocus = selectedBase_;
    checkForRemove(0);
    graphics_->setRegion(NRect(0,0,width(),height()));
    selectedBase_ = comp;
    if (oldFocus) oldFocus->onExit();
    selectedBase_->onEnter();
  }
}

void NWindow::setPositionToScreenMaximize( )
{
  setPosition(0,0,NApp::system().screenWidth(),NApp::system().screenHeight());
}

void NWindow::setPositionToScreenTop(int height)
{
  setPosition(0,0,NApp::system().screenWidth(),height);
}

void NWindow::setSize( int width, int height )
{
  setPosition(left(),top(),width,height);
}

const NRect & NWindow::userGeometry( ) const
{
  return userPos;
}

void NWindow::checkGeometry( )
{
  // works only with mapped windows
  if (userPos.left() != left() || userPos.top() != top() ) {
      std::cout << "userleft:" << userPos.left() << std::endl;
      std::cout << "usertop:" << userPos.top() << std::endl;
      XMoveWindow(NApp::system().dpy(), win(), userPos.left(), userPos.top() );
      std::cout << "wmleft:" << left() << std::endl;
      std::cout << "wmtop:" <<  top() << std::endl;
  }

  if (userPos.width() != width() || userPos.height() != height() ) {
     std::cout << userPos.width() << std::endl;
     std::cout << userPos.height() << std::endl;
     XResizeWindow(NApp::system().dpy(), win(), userPos.width(), userPos.height());
     std::cout << width() << std::endl;
     std::cout << height() << std::endl;
  }
}

void NWindow::setMinimumWidth( int minWidth )
{
  NVisual::setMinimumWidth(minWidth);
  NApp::system().setWindowMinimumSize(win(), minWidth , minimumHeight() );
}

void NWindow::setMinimumHeight( int minHeight )
{
  NVisual::setMinimumHeight(minHeight);
  NApp::system().setWindowMinimumSize(win(), minimumWidth(), minHeight );
}

void NWindow::requestSelection( )
{
  XConvertSelection( NApp::system().dpy(), NApp::system().atoms().primary_sel(), NApp::system().atoms().targets(), NApp::system().atoms().primary_sel(), win(), CurrentTime);
	XFlush(NApp::system().dpy());
}

void NWindow::onSelection( )
{
}







