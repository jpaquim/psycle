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
#include "visualcomponent.h"
#include "window.h"
#include "app.h"
#include "config.h"
#include "property.h"
#include <algorithm>

using namespace std;

namespace ngrs {

  IsVisualComponent* VisualComponent::isVisualComponent = new IsVisualComponent();

  VisualComponent::VisualComponent()
    : Visual(), clipping_(1), events_(1), scrollDx_(0), scrollDy_(0), layout_(0), win_(0), clSzPolicy(0), ownerSizeSet_(0), ownerPreferredWidth_(0), ownerPreferredHeight_(0), enabled_(1), tabStop_(0), focusEnabled_(1), allowDrop_( false )
  {
    // if (properties()) properties()->bind("align", *this, &VisualComponent::align, &VisualComponent::setAlign);

    ///\ todo ask bohan why this not works
    // if (properties()) properties()->bind<VisualComponent,Color,Color >(std::string("background"), *this, &VisualComponent::foreground, &VisualComponent::setForeground);
  }

  VisualComponent::~VisualComponent()
  {
    if (layout_) delete layout_;
  }

  void VisualComponent::draw( Graphics& g, const ngrs::Region & repaintArea , VisualComponent* sender)
  {
    if (visible()) {
      if (sender == this) Window::paintFlag = true;

      ngrs::Region oldRegion = g.region();                   // save old region

      ngrs::Region region = geometry()->region();             // get component geometry
      region.move(g.xTranslation(),g.yTranslation());  // move offset left , top

      region &= oldRegion;   // do intersection

      if (!region.empty()) {

        g.resetPen();

        g.setRepaintArea(repaintArea);  // set repaintArea to graphics

        bool clip_ = !(transparent() && (translucent()==100) && skin_.gradientStyle() == 0 );

        if (skin_.bitmapBgStyle()!=0) clip_ = true;

        if (clip_) g.setClipping(region);    //  setClipping
        g.setRegion(region);
        int gTx = g.xTranslation();          // store old graphics translation
        int gTy = g.yTranslation();

        if (!transparent() && (translucent() ==100) && Window::paintFlag && !skin_.gradientStyle() ) {
          g.setForeground(background());
          geometry()->fill(g,repaintArea);
          //std::cout << "j:" << name() << std::endl;
        }
        if (transparent() && (translucent()<100))
          g.fillTranslucent(left(),top(),width(),height(),skin_.transColor(), translucent());

        if (!transparent() && skin_.gradientStyle() == 1 && Window::paintFlag) {
          g.fillGradient(left(),top(),spacingWidth(),spacingHeight(),
            skin_.gradientStartColor(),skin_.gradientMidColor(),skin_.gradientEndColor(),
            skin_.gradientOrientation(),skin_.gradientPercent());
        } else
          if (!transparent() && skin_.gradientStyle() == 2 && Window::paintFlag) {
            g.fillRoundGradient(left(),top(),spacingWidth(),spacingHeight(),
              skin_.gradientStartColor(),skin_.gradientMidColor(),skin_.gradientEndColor(),
              skin_.gradientOrientation(),skin_.gradientStyle(),skin_.gradientArcWidth(),skin_.gradientArcHeight() );
          }

          if (skin_.bitmapBgStyle() == 1 && Window::paintFlag) {
            int w = skin_.bitmap().width();
            int h = skin_.bitmap().height();
            for (int yp = 0; yp < spacingHeight(); yp+=h) {
              for (int xp = 0; xp < spacingWidth(); xp+=w) {
                g.putBitmap( left()+xp,top()+yp,w,h,skin_.bitmap(),0,0);
              }
            }
          } else

            if (skin_.bitmapBgStyle() == 2 && Window::paintFlag) {

              int xp =(int)  d2i((spacingWidth()  - skin_.bitmap().width())  / 2.0f);
              int yp = (int) d2i((spacingHeight() - skin_.bitmap().height()) / 2.0f);
              g.putBitmap(left()+xp,top()+yp,skin_.bitmap());
            } else
              if (skin_.bitmapBgStyle() == 3 && Window::paintFlag) {
                // stretch      
                if ( width() && height() ) 
                  g.putStretchBitmap(left(),top(),skin_.bitmap() , width(), height() );
              }

              if (moveable().style() & nMvRectPicker) geometry()->drawRectPicker(g);
              if (moveable().style() & nMvPolygonPicker) geometry()->drawPicker(g);

              bool spacingClip_ = (!(spacing().left() == 0 && spacing().top() ==0 && spacing().bottom()==0 && spacing().right() == 0 && borderTop()==0 && borderLeft()==0 && borderBottom()==0 && borderRight()==0));

              if (spacingClip_) {
                ngrs::Region spacingRegion = geometry()->spacingRegion(Size(spacing().left()+borderLeft(),spacing().top()+borderTop(),spacing().right()+borderRight(),spacing().bottom()+borderBottom()));


                spacingRegion.move(g.xTranslation(),g.yTranslation());
                spacingRegion = region & spacingRegion;

                g.setTranslation(g.xTranslation()+left()-scrollDx_+spacing().left()+borderLeft(),g.yTranslation()+top()-scrollDy_+spacing().top()+borderTop());

                g.setClipping(spacingRegion);
                g.setRegion(spacingRegion);

              } else {
                g.setTranslation(g.xTranslation()+left()-scrollDx_,g.yTranslation()-scrollDy_+top());
                if (!clip_) {
                  g.setRegion(region);
                  g.setClipping(region);
                }
              }

              Font* oldFont = 0;
              bool oldUseParentFont;

              if ( skin_.overrideFontColor() ) {
                Font fnt = font();
                oldUseParentFont = skin_.parentFont();
                oldFont = new Font( fnt );
                fnt.setTextColor( skin_.textColor() );
                setFont( fnt );
              }

              if (Window::paintFlag) {
                g.setForeground(foreground());	
                g.setFont( font() );
                paint(g);
              }

              drawChildren( g, repaintArea, sender);   // the container children will be drawn	

              if ( oldFont ) {
                setFont( *oldFont );
                skin_.useParentFont( oldUseParentFont );
                delete oldFont;
              }

              g.setTranslation(gTx,gTy);           // set back to old translation
              if ((skin_.border()!=0)) {
                g.setClipping(region);
                skin_.border()->paint(g,*geometry());
              }
              if ( focus() ) {
                //      g.setForeground( Color (0,0,255) );
                //      g.drawRect(geometry()->rectArea() );
              }
              g.setRegion(oldRegion);              // restore old region

      }
    }
  }

  void VisualComponent::onExit() {
    repaint();
  }

  void VisualComponent::onEnter() {
    repaint();
  }

  void VisualComponent::onDrop() {
  }

  void VisualComponent::onDropEnter() {
  }
  
  void VisualComponent::doDrag() {
    App::doDrag( true, window() );
  }
  
  void VisualComponent::setAllowDrop( bool on ) {
    allowDrop_ = on;
  }

  bool VisualComponent::allowDrop() const {
    return allowDrop_;
  }

  void VisualComponent::drawChildren( Graphics& g, const ngrs::Region & repaintArea , VisualComponent* sender )
  {
    if ( layout_ == 0 ) {
      std::vector< VisualComponent* >::const_iterator itr = visualComponents().begin();
      for ( ;itr < visualComponents().end(); itr++ )
        (*itr)->draw( g, repaintArea, sender );
    } 
    else 
      layout_->drawComponents( this, g, repaintArea, sender );
  }

  bool VisualComponent::visit( NVisitor * v )
  {
    return v->visit_visualcomponent(this);
  }

  bool VisualComponent::transparent( ) const
  {
    return skin_.transparent();
  }

  void VisualComponent::setTransparent( bool on )
  {
    skin_.setTransparent(on);
  }

  void VisualComponent::setBackground( const Color & background )
  {
    skin_.setBackground(background);
    skin_.useParentBackground(false);
  }

  const Color & VisualComponent::background( )
  {
    if (skin_.parentBackground() && parent()->visit(VisualComponent::isVisualComponent)) {
      return (static_cast<VisualComponent*> (parent()))->background();
    }
    return skin_.background();
  }

  void VisualComponent::setScrollDx( int dx )
  {
    scrollDx_=dx;
  }

  void VisualComponent::setScrollDy( int dy )
  {
    scrollDy_=dy;
  }

  void VisualComponent::setSpacing( int left, int top, int right, int bottom )
  {
    skin_.setSpacing(Size(left,top,right,bottom));
  }

  void VisualComponent::paint( Graphics& g )
  {

  }

  const Font & VisualComponent::font( ) const
  {
    if (skin_.parentFont() && parent()!=0 && parent()->visit(VisualComponent::isVisualComponent)) {
      return ((VisualComponent*) parent())->font();
    }
    return skin_.font();
  }

  VisualComponent * VisualComponent::overObject( Graphics& g, long absX, long absY )
  {
    ngrs::Region oldRegion = g.region();        // save old region
    ngrs::Region region = geometry()->region();  // get component geometry
    region.move(g.xTranslation(),g.yTranslation());
    region = oldRegion & region;            // intersection

    g.setTranslation(g.xTranslation()+left()-scrollDx_+spacing().left()+borderLeft(),g.yTranslation()+top()-scrollDy_+spacing().top()+borderTop());
    VisualComponent* found = 0;

    if ( !region.empty() && region.intersects( absX, absY ) && events() ) {
      g.setRegion(region);       
      found = checkChildrenEvent( g, absX, absY );
      if (!found) found = this;   
    }    

    g.setTranslation(g.xTranslation()-left()-spacing().left()-borderLeft()+scrollDx_,g.yTranslation()-top()+scrollDy_-spacing().top()-borderTop());
    g.setRegion(oldRegion);

    return found;
  }

  // default order and event check
  // overridden e.g in ntoolbar
  VisualComponent* VisualComponent::checkChildrenEvent( Graphics& g, int absX, int absY ) {
    VisualComponent* found = 0;
    std::vector<VisualComponent*>::const_reverse_iterator rev_it = visualComponents().rbegin();
    for ( ; rev_it != visualComponents().rend(); ++rev_it ) {
      found = checkChildEvent( *rev_it, g, absX, absY );
      if ( found ) break;
    } 
    return found;
  }     

  VisualComponent* VisualComponent::checkChildEvent( VisualComponent* child, Graphics& g, int absX, int absY  ) {                
    if ( child->visible() )
      return child->overObject( g, absX, absY );
    else       
      return 0;          
  }

  int VisualComponent::spacingWidth( ) const
  {
    return (skin_.border()==0) ? geometry()->width() - ( spacing().left()+spacing().right()) : geometry()->width() - ( spacing().left()+spacing().right() + skin_.border()->spacing().left() + skin_.border()->spacing().right());
  }

  int VisualComponent::spacingHeight( ) const
  {
    return (skin_.border()==0) ?  geometry()->height() - ( spacing().top()+spacing().bottom()) : geometry()->height() - ( spacing().top()+spacing().bottom() + skin_.border()->spacing().top() + skin_.border()->spacing().bottom());
  }

  void VisualComponent::setBorder( const Border & border)
  {
    skin_.setBorder(border);
    //borderDeleteFlag_ = deleteFlag;
  }

  void VisualComponent::setForeground( const Color & foreground )
  {
    skin_.setForeground(foreground);
    skin_.useParentForeground(false);
  }

  const Color & VisualComponent::foreground( )
  {
    if (skin_.parentForeground() && parent()->visit(VisualComponent::isVisualComponent)) {
      return ((VisualComponent*) parent())->foreground();
    }
    return skin_.foreground();
  }

  void VisualComponent::setLayout(const Layout & layout) {
    if (layout_ != 0) delete layout_;
    layout_ = layout.clone();
    layout_->setParent(this);
  }

  void VisualComponent::updateAlign( )
  {
    if (layout_ != NULL) layout_->align(this);
  }

  int VisualComponent::clientWidth( ) const
  {
    if ((clSzPolicy & nHorizontal) != nHorizontal) return spacingWidth();
    if (layout_ == 0) return preferredWidth();
    return layout_->preferredWidth(this) ;
  }

  int VisualComponent::clientHeight( ) const
  {
    if ((clSzPolicy & nVertical) != nVertical) return spacingHeight();
    if (layout_ == 0) return preferredHeight();
    return layout_->preferredHeight(this);
  }

  int VisualComponent::borderRight( ) const
  {
    return (skin_.border()==0) ? 0 :  skin_.border()->spacing().right();
  }

  int VisualComponent::borderTop( ) const
  {
    return (skin_.border()==0) ? 0 :  skin_.border()->spacing().top();
  }

  int VisualComponent::borderBottom( ) const
  {
    return (skin_.border()==0) ? 0 :  skin_.border()->spacing().bottom();
  }

  int VisualComponent::borderLeft( ) const
  {
    return (skin_.border()==0) ? 0 :  skin_.border()->spacing().left();
  }

  const Size & VisualComponent::spacing( ) const
  {
    return skin_.spacing();
  }

  int VisualComponent::absoluteLeft( ) const
  {
    if (parent()!=NULL && parent()->visit(isVisualComponent)) return ((VisualComponent*)parent()) -> absoluteSpacingLeft()  + left () - ((VisualComponent*)parent())->scrollDx(); 
    else return 0;
  }

  int VisualComponent::absoluteTop( ) const
  {
    if (parent()!=NULL && parent()->visit(isVisualComponent)) return ((VisualComponent*)parent()) -> absoluteSpacingTop()  + top ()  - ((VisualComponent*)parent())->scrollDy(); 
    else return 0;
  }

  int VisualComponent::absoluteSpacingLeft( ) const
  {
    int borderSizeLeft = 0;
    if (skin_.border()!=NULL) borderSizeLeft = skin_.border()->spacing().left();
    return absoluteLeft() + borderSizeLeft + spacing().left();
  }

  int VisualComponent::absoluteSpacingTop( ) const
  {
    int borderSizeTop = 0;
    if (skin_.border()!=NULL) borderSizeTop = skin_.border()->spacing().top();
    return absoluteTop() + borderSizeTop + spacing().top();
  }

  int VisualComponent::scrollDx( ) const
  {
    return scrollDx_;
  }

  int VisualComponent::scrollDy( ) const
  {
    return scrollDy_;
  }

  void VisualComponent::setMoveable( const Moveable & moveable )
  {
    moveable_ = moveable;
  }

  const Moveable & VisualComponent::moveable( ) const
  {
    return moveable_;
  }

  void VisualComponent::onMove( const MoveEvent & moveEvent )
  {
    move.emit(moveEvent);
  }

  void VisualComponent::repaint(bool swap )
  {
    Window* win = window();
    if (win != 0) win->repaint(this,Rect(absoluteLeft(),absoluteTop(), width(), height()),swap);
  }

  void VisualComponent::setWindow( class Window * win )
  {
    win_ = win;
  }

  Window * VisualComponent::window( )
  {
    if (parent()==0) return 0;

    if (parent()->visit(Window::isWindow)) return (Window*) parent();

    if (parent()->visit(VisualComponent::isVisualComponent)) return ((VisualComponent*)parent())->window();

    return 0;
  }

  int VisualComponent::viewHeight( ) const
  {
    return spacingHeight();
  }

  int VisualComponent::viewWidth( ) const
  {
    return spacingWidth();
  }

  void VisualComponent::resize( )
  {
    updateAlign();
  }

  int VisualComponent::overRectPickPoint( int x, int y )
  {
    if (moveable_.style() & nMvRectPicker) {
      int pickWidth  = 4;
      int pickHeight = 4;
      x = x - absoluteLeft();
      y = y - absoluteTop();
      if (Rect(0,0,pickWidth,pickHeight).intersects(x,y))                      return nUpperLeftCorner;
      if (Rect(width()/2-pickWidth/2,0,pickWidth,pickHeight).intersects(x,y))  return nUpperMiddleCorner;
      if (Rect(width()-pickWidth,0,pickWidth,pickHeight).intersects(x,y))      return nUpperRightCorner;
      if (Rect(width()-pickWidth,height()/2 - pickHeight/2,pickWidth,pickHeight).intersects(x,y)) return nRightMiddleCorner;
      if (Rect(width()-pickHeight,height()-pickHeight,pickWidth,pickHeight).intersects(x,y)) return nLowerRightCorner;
      if (Rect(width()/2 -pickWidth/2,height()-pickHeight,pickWidth,pickHeight).intersects(x,y)) return nLowerMiddleCorner;
      if (Rect(0,height()-pickHeight,pickWidth,pickHeight).intersects(x,y)) return nLowerLeftCorner;
      if (Rect(0,height()/2 - pickHeight/2,pickWidth,pickHeight).intersects(x,y)) return nLeftMiddleCorner;
    }
    return 0;
  }


  int VisualComponent::overPickPoint( int x, int y )
  {
    if (parent()==NULL) return -1;
    int picker = -1;
    if (parent()->visit(VisualComponent::isVisualComponent)) {
      VisualComponent* dragBaseParent = (VisualComponent*) parent();

      int x_ = x - dragBaseParent->absoluteSpacingLeft()- dragBaseParent->scrollDx();
      int y_ = y - dragBaseParent->absoluteSpacingTop()- dragBaseParent->scrollDy();
      picker = geometry()->overPicker(x_,y_);
    }
    return picker;
  }

  void VisualComponent::setClipping( bool on )
  {
    clipping_ = on;
  }

  void VisualComponent::onMouseExit() {
    Visual::onMouseExit();
    App::system().setCursor( nCrDefault , window() );
  }

  void VisualComponent::onMouseEnter() {
    Visual::onMouseEnter();
    if ( !App::drag() )
      App::system().setCursor( cursor(), window() );
    else
      App::system().setCursor( nCrDrag, window() );
  }

  int VisualComponent::align( ) const
  {
    return alignConstraint_.align();
  }

  void VisualComponent::setAlign( int align )
  {
    alignConstraint_.setAlign(align);
  }

  int VisualComponent::preferredWidth( ) const
  {
    if (ownerSizeSet_) return ownerPreferredWidth_;
    if (layout_ == 0) return width(); else
      return layout_->preferredWidth(this)+ spacing().left() + spacing().right() + borderLeft() + borderRight();
  }

  int VisualComponent::preferredHeight( ) const
  {
    if (ownerSizeSet_) return ownerPreferredHeight_;
    if (layout_ == 0) return height(); else
      return layout_->preferredHeight(this)+ spacing().top() + spacing().bottom() + borderTop() + borderBottom();
  }

  void VisualComponent::setEvents( bool on )
  {
    events_ = on;
  }

  bool VisualComponent::events( ) const
  {
    return events_;
  }

  void VisualComponent::setTranslucent( Color color, int percent )
  {
    skin_.setTranslucent(color,percent);
  }

  void VisualComponent::setParentBackground( bool on )
  {
    skin_.useParentBackground(on);
  }

  Layout * VisualComponent::layout( )
  {
    return layout_;
  }


  void VisualComponent::setFont( const Font & font )
  {
    skin_.setFont(font);
    skin_.useParentFont(false);
  }

  int VisualComponent::zOrder( )
  {
    if (parent() == 0) return -1;
    int i = -1;
    std::vector<Runtime*>::iterator itr = parent()->components.begin();
    for (;itr < parent()->components.end(); itr++) {
      Runtime* child = *itr;
      if (child->visit(VisualComponent::isVisualComponent)) i++;
      if (child == this) return i;
    }
    return i;
  }

  VisualComponent * VisualComponent::componentByZOrder( int zOrder )
  {
    if (zOrder < 0) return 0;
    int i = -1;
    std::vector<VisualComponent*>::iterator itr = visualComponents_.begin();
    for (;itr < visualComponents_.end(); itr++) {
      VisualComponent* child = *itr;
      i++;
      if (i == zOrder) return (VisualComponent*) child;
    }
    return 0;
  }

  int VisualComponent::componentZOrderSize( )
  {
    int i = 0;
    std::vector<Runtime*>::iterator itr = components.begin();
    for (;itr < components.end(); itr++) {
      Runtime* child = *itr;
      if (child->visit(VisualComponent::isVisualComponent)) i++;
    }
    return i;
  }

  void VisualComponent::setClientSizePolicy( int clPolicy )
  {
    clSzPolicy = clPolicy;
  }

  Border * VisualComponent::border( )
  {
    return skin_.border();
  }

  int VisualComponent::translucent( ) const
  {
    return skin_.translucent();
  }

  const Color & VisualComponent::translucentColor( )
  {
    return skin_.transColor();
  }

  void VisualComponent::add( Runtime * component )
  {
    Visual::add(component);
  }

  void VisualComponent::add( VisualComponent * component )
  {
    Visual::add(component);
    if (component == this) return;

    visualComponents_.push_back(component);
    if (layout_!=0) {
      layout_->add(component);
      layout_->align(this);
    }

    // adding to tabOrderMap
    tabOrder_.push_back(component);
  }

  void VisualComponent::add( VisualComponent * component, int align, bool update )
  {
    if (component == this) return;
    component->setAlign(align);

    Visual::add(component);
    if (component == this) return;

    visualComponents_.push_back(component);
    if (layout_!=0) {
      layout_->add(component);
      if (update) layout_->align(this);
    }

    tabOrder_.push_back(component);
  }

  void VisualComponent::add( VisualComponent * component, const AlignConstraint & align, bool update )
  {
    if (component == this) return;
    component->setAlignConstraint(align);

    Visual::add(component);
    if (component == this) return;

    visualComponents_.push_back(component);
    if (layout_!=0) {
      layout_->add(component);
      if (update) layout_->align(this);
    }

    tabOrder_.push_back(component);
  }

  Rect VisualComponent::blitMove(int dx, int dy, const Rect & area)
  {
    VisualComponent* comp = this;

    int compLeft   = area.left();
    int compTop    = area.top();
    int compWidth  = area.width();
    int compHeight = area.height();

    Rect rect;

    Window* win = comp->window();
    Graphics& g = win->graphics();

    if (dy !=0) {
      int diffY = dy;
      g.setRegion(Rect(compLeft,compTop, compWidth, compHeight));
      g.setClipping(g.region());
      if (diffY > 0) {
        g.copyArea(compLeft  , compTop    + diffY, // src_x, sry_y
          compWidth , compHeight - diffY, // width, height
          compLeft  , compTop             // destX, destY
          );
        g.swap(Rect(compLeft,compTop, compWidth, compHeight - diffY));
        rect = Rect(compLeft,compTop + compHeight - diffY,compWidth,diffY);
      } else {
        g.copyArea(compLeft  , compTop, // src_x, sry_y
          compWidth , compHeight + diffY, // width, height
          compLeft  , compTop    - diffY             // destX, destY
          );
        g.swap(Rect(compLeft,compTop - diffY, compWidth, compHeight + diffY));
        rect = Rect(compLeft,compTop,compWidth,-diffY);
      }
    }

    if (dx !=0) {
      int diffX = dx;
      g.setRegion(Rect(compLeft,compTop, compWidth, compHeight));
      g.setClipping(g.region());
      if (diffX > 0) {
        g.copyArea(compLeft  + diffX, compTop, // src_x, sry_y
          compWidth - diffX, compHeight, // width, height
          compLeft  , compTop             // destX, destY
          );
        g.swap(Rect(compLeft,compTop, compWidth - diffX, compHeight));
        rect = Rect(compLeft + compWidth - diffX,compTop,diffX,compHeight);
      } else {
        g.copyArea(compLeft  , compTop, // src_x, sry_y
          compWidth + diffX , compHeight, // width, height
          compLeft - diffX  , compTop            // destX, destY
          );
        g.swap(Rect(compLeft - diffX,compTop, compWidth + diffX, compHeight));
        rect = Rect(compLeft,compTop,-diffX,compHeight);
      }
    }

    return rect;
  }

  Rect VisualComponent::absoluteGeometry( ) const
  {
    return Rect(absoluteLeft(),absoluteTop(),width(),height());
  }

  Rect VisualComponent::absoluteSpacingGeometry( ) const
  {
    return Rect(absoluteSpacingLeft(),absoluteSpacingTop(),spacingWidth(),spacingHeight());
  }

  void VisualComponent::removeChilds( )
  {
    for (vector<Runtime*>::iterator it = components.begin(); it < components.end(); it++) {
      Runtime* child = *it;
      if (window()!=0) window()->checkForRemove(child);
      child->setParent(NULL);
      App::addRemovePipe(child);
    }
    components.clear();
    visualComponents_.clear();
    tabOrder_.clear();

    if (layout_!=0) {
      layout_->removeAll();
      layout_->align(this);
    }
  }

  void VisualComponent::removeChild( VisualComponent * child )
  {
    std::vector<Runtime*>::iterator itr = find(components.begin(),components.end(),child);
    components.erase(itr);
    child->setParent(0);
    std::vector<VisualComponent*>::iterator vItr = find(visualComponents_.begin(),visualComponents_.end(),child);
    visualComponents_.erase(vItr);

    std::vector<VisualComponent*>::iterator tItr = find(tabOrder_.begin(),tabOrder_.end(),child);
    if ( tItr != tabOrder_.end() ) tabOrder_.erase(tItr);

    if ( window() ) window()->checkForRemove(child);
    if ( layout_ ) {
      layout_->remove(child);
      layout_->align(this);
    }
    App::addRemovePipe(child);

  }

  void VisualComponent::erase( VisualComponent * child )
  {
    std::vector<Runtime*>::iterator itr = find(components.begin(),components.end(),child);
    components.erase(itr);
    child->setParent(0);
    std::vector<VisualComponent*>::iterator vItr = find(visualComponents_.begin(),visualComponents_.end(),child);
    visualComponents_.erase(vItr);
    if (window()!=0) window()->checkForRemove(0);
    if (layout_!=0) {
      layout_->remove(child);
      layout_->align(this);
    }
  }

  std::vector<VisualComponent*>::iterator VisualComponent::erase( std::vector<VisualComponent*>::iterator first, std::vector<VisualComponent*>::iterator last) { 
    if ( first == vcEnd() ) return vcEnd();

    std::vector<Runtime*>::iterator start_itr = find( components.begin(), components.end(), *first );
    std::vector<Runtime*>::iterator end_itr;
    if ( last != vcEnd() )
      end_itr = find( components.begin(), components.end(), *last  );      
    else
      end_itr = components.end();

    if ( start_itr == components.end() ) return visualComponents_.end();
    components.erase( start_itr, end_itr );

    std::vector<VisualComponent*>::iterator it = first;
    for ( ; it != visualComponents_.end() && it <= last; it++ ) {
      (*it)->setParent( 0 );     
      if ( layout_ ) {
        layout_->remove( *it );
      }
    }
    it = visualComponents_.erase( first, last );  
    if ( layout_ )layout_->align( this );
    if ( window() ) window()->checkForRemove(0);

    return it;
  }

  void VisualComponent::eraseAll() {
    std::vector<Runtime*>::iterator it = components.begin();
    for ( ; it < components.end(); it ++ ) {
      (*it)->setParent( 0 );
    }
    components.clear();

    std::vector<VisualComponent*>::iterator vit = vcBegin();
    if ( layout_ )
      for ( ; vit < vcEnd(); vit ++ ) {
        layout_->remove( *vit );
      }  

      visualComponents_.clear();  

      if ( layout_ ) layout_->align( this );
      if ( window() ) window()->checkForRemove(0);
  }

  std::vector<VisualComponent*>::iterator VisualComponent::vcBegin() {
    return visualComponents_.begin();                                        
  }  

  std::vector<VisualComponent*>::iterator VisualComponent::vcEnd() {
    return visualComponents_.end();                                        
  }                                                                  

  void VisualComponent::erase( )
  {
    if (parent() && parent()->visit(VisualComponent::isVisualComponent)) { 
      static_cast<VisualComponent*>(parent())->erase(this);
    } else
      if ( parent() )
        Visual::erase();
  }

  const std::vector< VisualComponent * > & VisualComponent::visualComponents( ) const
  {
    return visualComponents_;
  }

  void VisualComponent::setSpacing( const Size & spacing )
  {
    setSpacing(spacing.left(),spacing.top(),spacing.right(),spacing.bottom());
  }

  void VisualComponent::setParentForeground( bool on )
  {
    skin_.useParentForeground(on);
  }

  void VisualComponent::setParentFont( bool on )
  {
    skin_.useParentFont(on);
  }

  Layout * VisualComponent::layout( ) const
  {
    return layout_;
  }

  void VisualComponent::setSkin( Skin skin )
  {
    skin_ = skin;
  }

  void VisualComponent::setGradientStyle( int style )
  {
    skin_.setGradientStyle(style);
  }

  void VisualComponent::setGradientOrientation( int orientation )
  {
    skin_.setGradientOrientation(orientation);
  }

  void VisualComponent::onMoveStart( const MoveEvent & moveEvent )
  {
    moveStart.emit(moveEvent);
  }

  void VisualComponent::onMoveEnd( const MoveEvent & moveEvent )
  {
    moveEnd.emit(moveEvent);
  }

  void VisualComponent::insert( VisualComponent * component, unsigned int index )
  {
    if (component == this) return;
    if (index <= visualComponents_.size()) {
      Runtime::insert(component,index);
      visualComponents_.insert(visualComponents_.begin()+index,component);
      if (layout_!=0) {
        layout_->insert(component, index);
        layout_->align(this);
      }
    } else add(component);
  }

  void VisualComponent::add( VisualComponent * component, int align )
  {
    if (component == this) return;
    component->setAlign(align);
    add(component);
  }

  void VisualComponent::setPreferredSize( int width, int height )
  {
    ownerSizeSet_ = true;
    ownerPreferredWidth_  = width;
    ownerPreferredHeight_ = height;
  }

  bool VisualComponent::ownerSize() const {
    return ownerSizeSet_;
  }

  void VisualComponent::setMoveFocus( int pickpoint )
  {
    if (window() !=0 ) {
      window()->setMoveFocus(this,pickpoint);
    }
  }

  bool VisualComponent::focus( ) const
  {
    bool b = window()->selectedBase() == this;
    return b;
  }

  Window * VisualComponent::window( ) const
  {
    if (parent()==0) return 0;

    if (parent()->visit(Window::isWindow)) return (Window*) parent();

    if (parent()->visit(VisualComponent::isVisualComponent)) return ((VisualComponent*)parent())->window();

    return 0;
  }

  int VisualComponent::ownerWidth( ) const
  {
    return ownerPreferredWidth_;
  }

  int VisualComponent::ownerHeight( ) const
  {
    return ownerPreferredHeight_;
  }

  void VisualComponent::setAlignConstraint( const AlignConstraint & constraint )
  {
    alignConstraint_ = constraint;
  }

  AlignConstraint VisualComponent::alignConstraint( ) const
  {
    return alignConstraint_;
  }


  void VisualComponent::setFocus( )
  {
    Window* win = window();
    if (win) {
      win->setFocus(this);
    }
  }

  void VisualComponent::setEnable( bool on )
  {
    setFont(oldFont);

    if (!on && enabled_) {
      oldFont = font();
      Font fnt = font();
      fnt.setTextColor(skin_.disabledTextColor());
      disableParentFont_ = skin_.parentFont();
      skin_.useParentFont(false);
      setFont(fnt);
    } else
      if (on && !enabled_) {
        setFont(oldFont);
        skin_.useParentFont(disableParentFont_);
      }

      enabled_ = on;
  }

  bool VisualComponent::enabled( ) const
  {
    return enabled_;
  }


  void VisualComponent::setTabOrder( int index ) {

    if ( parent() != 0 ) {

      VisualComponent* par = (VisualComponent*) parent();

      if (index < par->tabOrder_.size() ) {
        std::vector<VisualComponent*>::iterator it = par->tabOrder_.begin();
        int count = -1;
        for ( ; it < par->tabOrder_.end(); it++ ) {
          VisualComponent* child = *it;
          count ++;
          if ( count == index ) break;
        }
        std::vector<VisualComponent*>::iterator oldIt =
          find( par->tabOrder_.begin() , par->tabOrder_.end(), this);

        if (oldIt != par->tabOrder_.end() && count!=-1 && count < par->tabOrder_.size() ) {
          swap(it, oldIt);
        }
      }
    }
  }

  int VisualComponent::tabOrder() const {

    if ( parent() ) {
      int count = 0;
      VisualComponent* par = (VisualComponent*) parent();
      std::vector<VisualComponent*>::iterator it = par->tabOrder_.begin();
      for ( ; it < par->tabOrder_.end(); it++ ) {
        VisualComponent* child = *it;
        if ( child == this ) return count;
        count++;
      }
    }

    return -1;
  }

  void VisualComponent::setTabStop( bool on ) {
    tabStop_ = on;
  }

  bool VisualComponent::tabStop() const {
    return tabStop_;
  }

  void VisualComponent::onKeyPress(const KeyEvent & event) {
#ifdef __unix__
    if ( parent() ) {
      switch ( event.scancode() ) {
      case XK_Tab :
        tabRight();
        break;
      case XK_ISO_Left_Tab :
        tabLeft();
        break;
      default : ;
      }
    }

    /*  if ((event.scancode() == XK_Tab ) tabRight();
    || event.scancode() == XK_ISO_Left_Tab  )&& parent() )



    {
    if (parent()->visit(isVisualComponent)) {
    VisualComponent* par = (VisualComponent*) parent();
    std::vector<VisualComponent*>::iterator it = find( par->tabOrder_.begin(),par->tabOrder_.end(), this );
    if (  event.scancode() == XK_ISO_Left_Tab && it != par->tabOrder_.end() ) {
    if ( it != par->tabOrder_.begin() && (par->tabOrder_.size() > 0) ) {
    it--;
    window()->setFocus( *it);
    } else {
    if ( it == par->tabOrder_.begin() && (par->tabOrder_.size() > 0) ) {
    window()->setFocus( par->tabOrder_.back() );
    }
    }
    }	else {
    if ( it != par->tabOrder_.end() ) {
    it++;
    if ( it != par->tabOrder_.end() ) {
    window()->setFocus( *it );
    } else {
    if (par->tabOrder_.size() > 0) window()->setFocus( *(par->tabOrder_.begin()) );
    }
    } else {
    if (par->tabOrder_.size() > 0) window()->setFocus( *(par->tabOrder_.begin()) );
    }
    }
    }
    }*/
#endif
  }

  void VisualComponent::tabRight() {
    // first give child focus
    if ( visualComponents_.size() > 0) {
      std::vector<VisualComponent*>::iterator it = visualComponents_.begin();
      window()->setFocus( *it );
    } else
    {
      VisualComponent* par = (VisualComponent*) parent();
      if (par->tabOrder_.size() > 0) {
        std::vector<VisualComponent*>::iterator it=find( par->tabOrder_.begin(),par->tabOrder_.end(), this );
        if ( it != par->tabOrder_.end() ) {
          it++;
          if ( it != par->tabOrder_.end() )
            window()->setFocus( *it );
          else
            window()->setFocus( *(par->tabOrder_.begin()) );
        } else
          window()->setFocus( *(par->tabOrder_.begin()) );
      }
    }

  }

  void VisualComponent::tabLeft() {
    if (parent()->visit(isVisualComponent)) {
      VisualComponent* par = (VisualComponent*) parent();
    }
  }

  void VisualComponent::enableFocus( bool on )
  {
    focusEnabled_ = on;
  }

  bool VisualComponent::focusEnabled( ) const
  {
    if (focusEnabled_ && parent() && parent()->visit(VisualComponent::isVisualComponent)) {
      return ((VisualComponent*) parent())->focusEnabled();
    }
    return focusEnabled_;
  }

}
