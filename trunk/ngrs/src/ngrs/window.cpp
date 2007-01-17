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
#include "window.h"
#include "app.h"
#include "system.h"
#include "atoms.h"
#include "config.h"
#include "dockpanel.h"
#include "alignlayout.h"
#include "statusmodel.h"
//#include <X11/extensions/Xinerama.h>

#ifdef _MSC_VER
#undef min 
#undef max
#endif

namespace ngrs {

  IsWindow* Window::isWindow = new IsWindow();
  bool Window::paintFlag = true;

  Window::Window()
    : Visual(), 
      statusModel_(0), 
      win_( App::system().registerWindow(App::system().rootWindow()) ),
      graphics_( win_ )
  {  
    changeState_ = true;
    dblBuffer_ = true;    
    userPos.setPosition(0,0,200,200);
    App::addWindow(win_, this);

    pane_ = new Panel();  
    pane_->setSkin( App::config()->skin("pane") );
    pane_->setLayout( AlignLayout() );    
    add(pane_);
    
    dragBase_ = 0;
    dragRectPoint = 0;
    setTitle("Window");
    selectedBase_ = lastOver_ = 0;
    modal_ = false;
    Visual::setVisible(false);
    dock_ = 0;
    exitLoop_ = nDestroyWindow;
    oldFocus_ = 0;
    mousePressBase_ = 0;
  }

  Window::Window( const Window & topWindow ) :
     Visual(),
     win_( App::system().registerWindow(App::system().rootWindow()) ),
     graphics_( win_ )
  {
    changeState_ = true;
    dblBuffer_ = true;
    App::addWindow(win_, this);
    pane_ = new Panel();
    pane_->setTransparent(false);
    add(pane_);
    pane_->setBackground(Color(255,255,255));
    pane_->setLayout(AlignLayout());
    dragBase_ = 0;
    dragRectPoint = 0;
    setTitle("Window");
    selectedBase_ = lastOver_ = 0;
    Visual::setVisible(false);
  }

  Window::~Window()
  {
    if (mapped()) App::system().unmapWindow(win_);
    App::removeWindow(win_);
    App::system().destroyWindow(win_);
  }

  void Window::setVisible( bool on )
  {
    Visual::setVisible(on);
    if (on) {
      graphics_.setVisible(on);
      pane_->resize();
      repaint( pane(), Rect( 0, 0, width(), height() ) );
      if (modal_) {
#ifdef __unix__
        App::system().setModalMode( win() );
        //App::system().setStayAbove(win());
        App::system().setWindowPosition(win(), userPos.left(),userPos.top(),userPos.width(),userPos.height());
        App::system().setWindowMinimumSize(win(),minimumWidth(), minimumHeight());
        XMapWindow(App::system().dpy(),win_);
        checkGeometry();
        App::runModal(this);
#else
        /*         App::system().setModalMode( win() );
        App::system().setWindowPosition(win(), userPos.left(),userPos.top(),userPos.width(),userPos.height());
        App::system().setWindowMinimumSize(win(),minimumWidth(), minimumHeight());*/
        ShowWindow( win(), 1 );
        UpdateWindow( win() );
        //         checkGeometry();
        App::runModal(this);
#endif
      } else {
#ifdef __unix__
        XSync(App::system().dpy(),false);
        App::system().setWindowPosition(win(), userPos.left(),userPos.top(),userPos.width(),userPos.height());
        App::system().setWindowMinimumSize(win(),minimumWidth(), minimumHeight());
        XMapWindow(App::system().dpy(),win_);
        checkGeometry();
        XSync(App::system().dpy(),false);
#else

        long style =  GetWindowLongPtr(      
          win(),
          GWL_STYLE
          );    

        if ( style & WS_POPUP ) {
          ShowWindow( win(), SW_SHOWNA );
          UpdateWindow( win() );
        } else {
          ShowWindow( win(), 1);
          UpdateWindow( win() );
        }  
#endif
      }
    } else if (mapped()) {
      if (lastOver_!=0) {
        lastOver_->onMouseExit();
        lastOver_ = 0;
        dragBase_ = 0;
      }
#ifdef __unix__
      XSync(App::system().dpy(),false);
      App::system().unmapWindow(win_);
      XSync(App::system().dpy(),false);
      graphics_.setVisible(on);
#else
      ShowWindow( win_, SW_HIDE );
      UpdateWindow( win_ );
      graphics_.setVisible( on );
#endif
    }
  }

  WinHandle Window::win( ) const
  {
    return win_;
  }

  Panel * Window::pane( )
  {
    return pane_;
  }


  void Window::repaint( VisualComponent * sender, const ngrs::Region & repaintArea, bool swap )
  {
    if ( visible() ) {
      paintFlag = false;

      if (sender->transparent()) {
        // find last non transparent

        while (sender->transparent() && sender->skin_.gradientStyle()==0 && sender !=pane() && sender->parent() != 0) {
          sender = static_cast<VisualComponent*> (sender->parent());
        }
      }

      if (pane_->width() !=width() || pane_->height() !=height())
        pane_->setPosition(0,0,width(),height());
      graphics_.setRegion(repaintArea);
      pane_->draw(graphics_,repaintArea,sender);
      if (dblBuffer_ && swap) graphics().swap(repaintArea.rectClipBox());
    }
  }


  Graphics& Window::graphics( )
  {
    return graphics_;
  }

  int Window::width( ) const
  {
#ifdef __unix__
    XWindowAttributes attr;
    XGetWindowAttributes( App::system().dpy(), win_, &attr );
    return attr.width;
#else
    RECT r;
    GetClientRect( win_, &r );
    return ( r.right - r.left );
#endif
  }

  int Window::height( ) const
  {
#ifdef __unix__
    XWindowAttributes attr;
    XGetWindowAttributes( App::system().dpy(), win_, &attr );
    return attr.height;
#else
    RECT r;
    GetClientRect( win_, &r );
    return ( r.bottom - r.top );
#endif
  }

  void Window::onMousePress( int x, int y, int button )
  {
    oldFocus_ = 0;
    dragBase_ = 0;
    mousePressBase_ = 0;

    graphics_.setRegion(Rect(0,0,width(),height()));
    VisualComponent* obj = pane()->overObject(graphics(),x,y);
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
        if (selectedBase_ && !dragBase_ && selectedBase_!=pane_ && (mapped())) {
          if ( selectedBase_->moveable().style() ) {
            dragBase_ = selectedBase_;
            initDrag(obj,x,y);
          }
        }
      }

    }
  }

  void Window::onMousePressed( int x, int y, int button )
  {
    if (mousePressBase_!=0 && mousePressBase_->enabled()) mousePressBase_->onMousePressed( x - mousePressBase_->absoluteSpacingLeft(), y - mousePressBase_->absoluteSpacingTop(), button);
    endDrag(dragBase_,x,y);
    dragBase_ = 0;
    if ( selectedBase_ && selectedBase_->enabled() && selectedBase_->focusEnabled() ) selectedBase_->onEnter(); 
    else
      if ( oldFocus_ ) selectedBase_ = oldFocus_;
    mousePressBase_ = 0;
  }

  void Window::onMouseOver( int x, int y )
  {
    graphics_.setRegion(Rect(0,0,width(),height()));
    if (dragBase_!=NULL) {    
      if (dragBase_->moveable().style()!=0) doDrag(dragBase_,x,y);
      dragBase_->onMouseOver( x - dragBase_->absoluteSpacingLeft(), y - dragBase_->absoluteSpacingTop());
    } else 
      if (mousePressBase_!=NULL) {   
        mousePressBase_->onMouseOver( x - mousePressBase_->absoluteSpacingLeft(), y - mousePressBase_->absoluteSpacingTop());
      } else
      {
        VisualComponent* over = pane_->overObject(graphics_,x,y);
        if (lastOver_!=0 && over!=lastOver_ && lastOver_->enabled()) lastOver_->onMouseExit();
        if (over!=0 && over->enabled()) {
          if (over!=lastOver_ || lastOver_==0 && selectedBase_->enabled()) over->onMouseEnter();
          over->onMouseOver( x - over->absoluteSpacingLeft(), y - over->absoluteSpacingTop());
        }
        lastOver_ = over;
      }
  }

  void Window::initDrag( VisualComponent * dragBase, int x, int y )
  {
    oldDrag = dragBase->geometry()->region();

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

    dragBase_->onMoveStart( MoveEvent(
      dragBase_,
      x - dragBase_->absoluteSpacingLeft() + dragBase_->scrollDx(),
      y - dragBase_->absoluteSpacingTop() + dragBase_->scrollDy(),
      dragPoint
      ));
  }

  void Window::doDrag( VisualComponent *, int x, int y )
  {
    dragX = x;
    dragY = y;
    int pickOffset = 0 ;
    int varx=abs(dragOldX-dragX)+pickOffset;
    int vary=abs(dragOldY-dragY)+pickOffset;
    dragOldX = x;
    dragOldY = y;
    VisualComponent* dragBase = dragBase_;
    if (dragBase_->parent()->visit(VisualComponent::isVisualComponent)) {
      VisualComponent* dragBaseParent = (VisualComponent*) dragBase->parent();
      int newLeft = x+dragOffset.left() - dragBaseParent->absoluteSpacingLeft()+ dragBaseParent->scrollDx();
      int newTop  = y+dragOffset.top()  - dragBaseParent->absoluteSpacingTop() + dragBaseParent->scrollDy();

      if (dragRectPoint!=0) dragRectPicker(dragBase,x,y,varx,vary); else

        if (dragPoint != -1) {
          dragBase_->geometry()->setPicker(dragPoint, x - dragBaseParent->absoluteSpacingLeft() + dragBaseParent->scrollDx(), y - dragBaseParent->absoluteSpacingTop() + dragBaseParent->scrollDy());
          ngrs::Region newDrag = dragBase->geometry()->region();
          ngrs::Region repaintArea = newDrag | oldDrag;
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
          dragBase->onMove( MoveEvent(dragBase_, x - dragBase_->absoluteSpacingLeft() +
            dragBase_->scrollDx(), y - dragBase_->absoluteSpacingTop() + dragBase_->scrollDy(), dragPoint ));
          if (!(dragBase_->moveable().style() & nMvNoneRepaint)) {
            ngrs::Region newDrag = dragBase->geometry()->region();
            ngrs::Region repaintArea = newDrag | oldDrag;
            repaintArea.move(dragBaseParent->absoluteLeft() - dragBaseParent->scrollDx(),dragBaseParent->absoluteTop() - dragBaseParent->scrollDy());
            repaint(pane(),repaintArea);
            oldDrag = newDrag;
          }
        }
    }
  }


  void Window::endDrag( VisualComponent *, int x, int y )
  {
    if (dragBase_!=0) dragBase_->onMoveEnd( MoveEvent(dragBase_, x - dragBase_->absoluteSpacingLeft() +
      dragBase_->scrollDx(), y - dragBase_->absoluteSpacingTop() + dragBase_->scrollDy(), dragPoint ));
    dragBase_ = NULL;
  }

  void Window::setTitle( const std::string & title )
  {
    title_ = title;
#ifdef __unix__
    XSetStandardProperties(App::system().dpy(), win_, title.c_str(), title.c_str(), 0, NULL, 0, NULL);	
#else
    SetWindowText( win_, title.c_str() );
#endif    
  }

  void Window::setTitleBitmap( const Bitmap & bmp ) {
    // small icon size
#ifdef WIN32 
#else
#endif        
  }     

  std::string Window::title( )
  {
    return title_;
  }

  int Window::onClose( )
  {
    if (dock_!=0) {
      return nDockWindow;
    }
    exitLoop_ = nDestroyWindow;
    setVisible(false);
    return nDestroyWindow;
  }

  void Window::dragRectPicker( VisualComponent * dragBase, int x, int y, int varx, int vary )
  {
    if (dragBase_->parent()->visit(VisualComponent::isVisualComponent)) {
      VisualComponent* dragBaseParent = (VisualComponent*) dragBase->parent();


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
        repaint(pane(),Rect(dragBase->absoluteLeft()-varx,dragBase->absoluteTop()-vary,dragBase->width()+2*varx,dragBase->height()+2*vary));
      }
    }
  }

  void Window::onKeyPress( const KeyEvent & keyEvent )
  {
    if ( selectedBase_ != NULL && selectedBase_->enabled() ) selectedBase_->onKeyPress(KeyEvent(selectedBase_,keyEvent.buffer(),keyEvent.scancode(), keyEvent.shift() ));
  }

  void Window::onKeyRelease( const KeyEvent & keyEvent )
  {
    if (selectedBase_ != NULL && selectedBase_->enabled() ) selectedBase_->onKeyRelease(KeyEvent(selectedBase_,keyEvent.buffer(),keyEvent.scancode(), keyEvent.shift() ));
  }


  void Window::setPosition( int x, int y, int width, int height )
  {
    userPos.setPosition(x,y,width,height);
    App::system().setWindowPosition(win_,x,y,width,height);
  }

  void Window::setLeft( int left )
  {
    setPosition(left, top(), width(), height());
  }

  void Window::setTop( int top )
  {
    setPosition(left(), top, width(), height());
  }

  void Window::setWidth( int width )
  {
    setPosition(left(), top(), width, height());
  }

  void Window::setHeight( int height )
  {
    setPosition(left(), top(), width(), height);
  }

  void Window::add( Window * window )
  {
    Runtime::add(window);
    App::addWindow(window->win(),window);
  }

  void Window::setDecoration( bool on )
  {
    App::system().setWindowDecoration(win(),on);
  }

  bool Window::visit( NVisitor * v )
  {
    return v->visit_window(this);
  }

  bool Window::mapped( )
  {
    return App::system().isWindowMapped(win_);
  }

  int Window::top( ) const
  {
    return App::system().windowTop(win_);
  }

  int Window::left( ) const
  {
    return App::system().windowLeft(win_);
  }

  void Window::pack( )
  {
    if (pane()->layout()!=NULL) {
      pane()->layout()->align(pane());
      // std::cout << "pack-width:" << pane()->layout()->preferredWidth(pane()) << std::endl;

      // should really not be hardcoded but read out from xlib
      int windowBorderWidth = 2;
      int windowBorderHeight = 2;

      int pW = std::max( pane()->preferredWidth() + windowBorderWidth ,10);
      int pH = std::max( pane()->preferredHeight() + windowBorderHeight
        ,10);
      setPosition(left(),top(),pW,pH);
      setMinimumWidth(pW);
      setMinimumHeight(pH);
    }
  }

  void Window::add( Runtime * component )
  {
    Runtime::add(component);
  }

  void Window::onMouseExit( )
  {
    if (lastOver_!=0) {
      lastOver_->onMouseExit();
      lastOver_=0;
    }
  }

  void Window::setGrabEvents(bool on )
  {
    App::system().setWindowGrab(win_,on);
  }

  bool Window::changeState( )
  {
    return changeState_;
  }

  void Window::setChangeState( bool on )
  {
    changeState_ = on;
  }

  void Window::setDoubleBuffer( bool on )
  {
    dblBuffer_ = on;
    graphics_.setDoubleBuffer(on);
  }

  bool Window::doubleBuffered( ) const
  {
    return dblBuffer_;
  }

  void Window::setModal( bool on )
  {
    modal_ = on;
  }

  bool Window::modal( ) const
  {
    return modal_;
  }

  int Window::exitLoop( )
  {
    return exitLoop_;
  }

  void Window::setExitLoop( int flag )
  {
    exitLoop_ = flag;
  }

  void Window::setPositionToScreenCenter( )
  {
    // todo xinerama display width / height for a screen
    setPosition( (App::system().screenWidth() - width()) / 2,  (App::system().screenHeight() - height()) / 2, width(), height());
  }

  void Window::checkForRemove( Runtime * obj )
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

  void Window::setMoveFocus( VisualComponent * moveable, int pickPoint )
  {
    dragBase_ = moveable;

    int x = moveable->absoluteLeft();
    int y = moveable->absoluteTop();

    dragBase_->onMoveStart( MoveEvent(dragBase_, x - dragBase_->absoluteSpacingLeft() +
      dragBase_->scrollDx(), y - dragBase_->absoluteSpacingTop() + dragBase_->scrollDy(), pickPoint));

    dragX = x;
    dragY = y;
    dragRectPoint = 0;
    dragPoint = pickPoint;
    dragOldX = x;
    dragOldY= y;

    dragOffset.setSize(dragBase_->absoluteLeft()-x,dragBase_->absoluteTop()-y,dragBase_->absoluteLeft() + dragBase_->width() - x,dragBase_->absoluteTop()  + dragBase_->height()- y);

  }

  Graphics& Window::graphics( VisualComponent * comp )
  {
    ngrs::Region region = comp->geometry()->region();
    region.move(comp->absoluteLeft()-comp->left(),comp->absoluteTop()-comp->top());
    graphics_.setRegion(region);
    graphics_.setClipping(region);
    graphics_.setTranslation(comp->absoluteLeft()-comp->scrollDx(),comp->absoluteTop()-comp->scrollDy());
    return graphics_;
  }

  void Window::onMouseDoublePress( int x, int y, int button )
  {
    graphics_.setRegion(Rect(0,0,width(),height()));
    VisualComponent* obj = pane()->overObject(graphics(),x,y);
    if (obj) {
      dragBase_ = 0;             
      obj->onMouseDoublePress(x - obj->absoluteSpacingLeft(), y - obj->absoluteSpacingTop(), button);
    }    
  }

  VisualComponent* Window::selectedBase( ) const
  {
    return selectedBase_;
  }

  void Window::setDock( DockPanel * dock )
  {
    dock_ = dock;
  }

  DockPanel * Window::dock( )
  {
    return dock_;
  }

  void Window::setFocus( )
  {

  }

  void Window::setFocus( VisualComponent * comp )
  {
    if ( comp ) {
      VisualComponent* oldFocus = selectedBase_;
      checkForRemove(0);
      graphics_.setRegion(Rect(0,0,width(),height()));
      selectedBase_ = comp;
      if (oldFocus) oldFocus->onExit();
      selectedBase_->onEnter();
    }
  }

  void Window::setPositionToScreenMaximize( )
  {
    setPosition(0,0,App::system().screenWidth(),App::system().screenHeight());
  }

  void Window::setPositionToScreenTop(int height)
  {
    setPosition(0,0,App::system().screenWidth(),height);
  }

  void Window::setSize( int width, int height )
  {
    App::system().setWindowSize(win(), width,height);
  }

  const Rect & Window::userGeometry( ) const
  {
    return userPos;
  }

  void Window::checkGeometry( )
  {
#ifdef __unix__
    // works only with mapped windows
    if (userPos.left() != left() || userPos.top() != top() ) {
      //std::cout << "userleft:" << userPos.left() << std::endl;
      //std::cout << "usertop:" << userPos.top() << std::endl;
      XMoveWindow(App::system().dpy(), win(), userPos.left(), userPos.top() );
      //std::cout << "wmleft:" << left() << std::endl;
      //std::cout << "wmtop:" <<  top() << std::endl;
    }

    if (userPos.width() != width() || userPos.height() != height() ) {
      //std::cout << userPos.width() << std::endl;
      //std::cout << userPos.height() << std::endl;
      XResizeWindow(App::system().dpy(), win(), userPos.width(), userPos.height());
      //std::cout << width() << std::endl;
      //std::cout << height() << std::endl;
    }
#endif
  }

  void Window::setMinimumWidth( int minWidth )
  {
    Visual::setMinimumWidth(minWidth);
    App::system().setWindowMinimumSize(win(), minWidth , minimumHeight() );
  }

  void Window::setMinimumHeight( int minHeight )
  {
    Visual::setMinimumHeight(minHeight);
    App::system().setWindowMinimumSize(win(), minimumWidth(), minHeight );
  }

  void Window::requestSelection( )
  {
#ifdef __unix__
    XConvertSelection( App::system().dpy(), App::system().atoms().primary_sel(), App::system().atoms().targets(), App::system().atoms().primary_sel(), win(), CurrentTime);
    XFlush(App::system().dpy());
#endif
  }

  void Window::onSelection( )
  {
  }

  void Window::setStatusModel( CustomStatusModel & model ) {
    statusModel_ = &model;
  }

  CustomStatusModel* Window::statusModel() const {
    return statusModel_;
  }

}
