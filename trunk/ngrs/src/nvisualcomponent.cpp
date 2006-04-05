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
#include "nvisualcomponent.h"
#include "nwindow.h"
#include "napp.h"
#include "nconfig.h"

using namespace std;

NIsVisualComponent* NVisualComponent::isVisualComponent = new NIsVisualComponent();

NVisualComponent::NVisualComponent()
 : NVisual()
{
  skin_.transparent = true;
  skin_.useParentBgColor = true;
  skin_.useParentFgColor = true;
  skin_.useParentFont    = true;
  skin_.bitmapBgStyle = 0;
  skin_.translucent = 100;
  skin_.gradientStyle = 0;
  skin_.gradientOrientation = nHorizontal;
  skin_.gradientPercent = 50;
  skin_.border = 0;

  scrollDx_ = scrollDy_ = 0;
  skin_.border  = 0;
  layout_  = 0;
  win_ = 0;
  align_ = 0;
  clipping_ = events_ = true;

  clSzPolicy = 0;

  ownerSizeSet_ = false;
  ownerPreferredWidth_  = 0;
  ownerPreferredHeight_ = 0;
}


NVisualComponent::~NVisualComponent()
{
  if (layout_!=0) layout_->setParent(0);
}


void NVisualComponent::draw( NGraphics * g, const NRect & repaintArea )
{
  if (visible()) {
   Region oldRegion = g->region();
   Region region = geometry()->region();
   XOffsetRegion(region,g->xTranslation(),g->yTranslation());
   XIntersectRegion(oldRegion,region,region);
   if (!XEmptyRegion(region)) {
    g->setRepaintArea(repaintArea);
    bool clip_ = !(transparent() && (translucent()==100) && skin_.gradientStyle == 0 );
    if (skin_.bitmapBgStyle!=0) clip_ = true;

    g->setRegion(region, clip_);

    int gTx = g->xTranslation();
    int gTy = g->yTranslation();

    if (!transparent() && (translucent() ==100)) {
      g->setForeground(background());
      geometry()->fill(g,repaintArea);
    }
    if (transparent() && (translucent()<100))
         g->fillTranslucent(left(),top(),width(),height(),skin_.transColor, translucent());

    if (skin_.gradientStyle == 1) {
         g->fillGradient(left(),top(),spacingWidth(),spacingHeight(),
            skin_.gradStartColor,skin_.gradMidColor,skin_.gradEndColor,
            skin_.gradientOrientation,skin_.gradientOrientation);
    } else
    if (skin_.gradientStyle == 2) {
         g->fillRoundGradient(left(),top(),spacingWidth(),spacingHeight(),
            skin_.gradStartColor,skin_.gradMidColor,skin_.gradEndColor,
            skin_.gradientOrientation,skin_.gradientOrientation,skin_.arcWidth,skin_.arcHeight);
    }

    if (skin_.bitmapBgStyle == 1) {
      int w = skin_.bitmap.width();
      int h = skin_.bitmap.height();
      for (int yp = 0; yp < spacingHeight(); yp+=h) {
        for (int xp = 0; xp < spacingWidth(); xp+=w) {
          g->putBitmap(left()+xp,top()+yp,w,h,skin_.bitmap,0,0);
       }
      }
    } else

    if (skin_.bitmapBgStyle == 2) {

      int xp =(int)  d2i((spacingWidth()  - skin_.bitmap.width())  / 2.0f);
      int yp = (int) d2i((spacingHeight() - skin_.bitmap.height()) / 2.0f);
      g->putBitmap(left()+xp,top()+yp,skin_.bitmap);
    }

    if (moveable().style() & nMvRectPicker) geometry()->drawRectPicker(g);
    geometry()->drawPicker(g);

    bool spacingClip_ = (!(spacing().left() == 0 && spacing().top() ==0 && spacing().bottom()==0 && spacing().right() == 0 && borderTop()==0 && borderLeft()==0 && borderBottom()==0 && borderRight()==0));

    if (spacingClip_) {
       Region spacingRegion = geometry()->spacingRegion(NSize(spacing().left()+borderLeft(),spacing().top()+borderTop(),spacing().right()+borderRight(),spacing().bottom()+borderBottom()));

       XOffsetRegion(spacingRegion,g->xTranslation(),g->yTranslation());
       XIntersectRegion(region,spacingRegion,spacingRegion);

       g->setTranslation(g->xTranslation()+left()-scrollDx_+spacing().left()+borderLeft(),g->yTranslation()+top()-scrollDy_+spacing().top()+borderTop());

       g->setRegion(spacingRegion, true);
    } else {
        g->setTranslation(g->xTranslation()+left()-scrollDx_,g->yTranslation()-scrollDy_+top());
        if (!clip_) g->setRegion(region,true);
    }
    g->setForeground(foreground());
    g->setFont(font());
    paint(g);
    drawChildren(g,repaintArea);
    g->setTranslation(gTx,gTy);
    if ((skin_.border!=0)) {
        g->setRegion(region, true);
        skin_.border->paint(g,*geometry());
    }
    g->setRegion(oldRegion, false);
    geometry()->destroyRegion();
    if (spacingClip_) geometry()->destroySpacingRegion();
   }
  }
}

void NVisualComponent::drawChildren( NGraphics * g, const NRect & repaintArea )
{
  if (layout_ == 0) {
    std::vector<NRuntime*>::iterator itr = components.begin();
    for (;itr < components.end(); itr++) {
      NRuntime* child = *itr;
      if (child->visit(isVisualComponent)) {
       // we know that the Component is a visual Component and can type safe cast due to the visitor pattern
        NVisualComponent* visualChild = (NVisualComponent*) child;
        visualChild->draw(g,repaintArea);
      }
    }
  } else layout_->drawComponents(this,g,repaintArea);
}

bool NVisualComponent::visit( NVisitor * v )
{
  return v->visit_visualcomponent(this);
}

bool NVisualComponent::transparent( ) const
{
  return skin_.transparent;
}

void NVisualComponent::setTransparent( bool on )
{
  skin_.transparent = on;
}

void NVisualComponent::setBackground( const NColor & background )
{
   skin_.bgColor = background;
   skin_.useParentBgColor = false;
}

const NColor & NVisualComponent::background( )
{
  if (skin_.useParentBgColor && parent()->visit(NVisualComponent::isVisualComponent)) {
     return (static_cast<NVisualComponent*> (parent()))->background();
  }
  return skin_.bgColor;
}

void NVisualComponent::setScrollDx( int dx )
{
  scrollDx_=dx;
}

void NVisualComponent::setScrollDy( int dy )
{
  scrollDy_=dy;
}

void NVisualComponent::setSpacing( int left, int top, int right, int bottom )
{
  skin_.spacing.setSize(left,top,right,bottom);
}

void NVisualComponent::paint( NGraphics * g )
{

}

const NFont & NVisualComponent::font( ) const
{
  if (skin_.useParentFont && parent()!=0 && parent()->visit(NVisualComponent::isVisualComponent)) {
     return ((NVisualComponent*) parent())->font();
  }
  return skin_.font;
}

NVisualComponent * NVisualComponent::overObject( NGraphics* g, long absX, long absY )
{
  Region oldRegion = g->region();
  Region region = geometry()->region();
  XOffsetRegion(region,g->xTranslation(),g->yTranslation());
  XIntersectRegion(oldRegion,region,region);

  g->setTranslation(g->xTranslation()+left()-scrollDx_+spacing().left()+borderLeft(),g->yTranslation()+top()-scrollDy_+spacing().top()+borderTop());

  if (!XEmptyRegion(region) && XPointInRegion(region,absX,absY) && events()) {
       g->setRegion(region, false);
       if (visualComponents_.size()>0)
         for( vector<NVisualComponent*>::iterator itr = visualComponents_.end()-1; itr >= visualComponents_.begin(); itr--) {
            NVisualComponent* visualChild = *itr;
            if (visualChild->visible()) {
              NVisualComponent* found = visualChild->overObject(g,absX, absY);
              if (found!=NULL) {
                     g->setTranslation(g->xTranslation()-left()-spacing().left()-borderLeft()+scrollDx_,g->yTranslation()-top()+scrollDy_-spacing().top()-borderTop());
                    g->setRegion(oldRegion, false);
                    geometry()->destroyRegion();
                   return found;
             }
            }
          }
           g->setTranslation(g->xTranslation()-left()-spacing().left()-borderLeft()+scrollDx_,g->yTranslation()-top()+scrollDy_-spacing().top()-borderTop());
          g->setRegion(oldRegion, false);
          geometry()->destroyRegion();
         return this;
  }
    g->setTranslation(g->xTranslation()-left()-spacing().left()-borderLeft()+scrollDx_,g->yTranslation()-top()+scrollDy_-spacing().top()-borderTop());
  g->setRegion(oldRegion, false);
  geometry()->destroyRegion();
  return NULL;
}

int NVisualComponent::spacingWidth( ) const
{
  return (skin_.border==0) ? geometry()->width() - ( spacing().left()+spacing().right()) : geometry()->width() - ( spacing().left()+spacing().right() + skin_.border->spacing().left() + skin_.border->spacing().right());
}

int NVisualComponent::spacingHeight( ) const
{
  return (skin_.border==0) ?  geometry()->height() - ( spacing().top()+spacing().bottom()) : geometry()->height() - ( spacing().top()+spacing().bottom() + skin_.border->spacing().top() + skin_.border->spacing().bottom());
}

void NVisualComponent::setBorder( NBorder * border )
{
  skin_.border = border;
}

void NVisualComponent::setForeground( const NColor & foreground )
{
  skin_.fgColor = foreground;
  skin_.useParentFgColor = false;
}

const NColor & NVisualComponent::foreground( )
{
  if (skin_.useParentFgColor && parent()->visit(NVisualComponent::isVisualComponent)) {
     return ((NVisualComponent*) parent())->foreground();
  }
  return skin_.fgColor;
}

void NVisualComponent::setLayout(NLayout* layout) {
   layout_ = layout;
   if (layout_!=0) layout_->setParent(this);

}

void NVisualComponent::updateAlign( )
{
  if (layout_ != NULL) layout_->align(this);
}

int NVisualComponent::clientWidth( ) const
{
  if (layout_ == 0||(clSzPolicy &nHorizontal)!=nHorizontal) return spacingWidth();
  return layout_->preferredWidth(this);
}

int NVisualComponent::clientHeight( ) const
{
  if (layout_ == 0||(clSzPolicy & nVertical)!= nVertical) return spacingHeight(); 
  return layout_->preferredHeight(this);
}

int NVisualComponent::borderRight( ) const
{
  return (skin_.border==0) ? 0 :  skin_.border->spacing().right();
}

int NVisualComponent::borderTop( ) const
{
  return (skin_.border==0) ? 0 :  skin_.border->spacing().top();
}

int NVisualComponent::borderBottom( ) const
{
  return (skin_.border==0) ? 0 :  skin_.border->spacing().bottom();
}

int NVisualComponent::borderLeft( ) const
{
  return (skin_.border==0) ? 0 :  skin_.border->spacing().left();
}

const NSize & NVisualComponent::spacing( ) const
{
  return skin_.spacing;
}

int NVisualComponent::absoluteLeft( ) const
{
  if (parent()!=NULL && parent()->visit(isVisualComponent)) return ((NVisualComponent*)parent()) -> absoluteSpacingLeft()  + left () - ((NVisualComponent*)parent())->scrollDx(); 
                 else return 0;
}

int NVisualComponent::absoluteTop( ) const
{
   if (parent()!=NULL && parent()->visit(isVisualComponent)) return ((NVisualComponent*)parent()) -> absoluteSpacingTop()  + top ()  - ((NVisualComponent*)parent())->scrollDy(); 
                 else return 0;
}

int NVisualComponent::absoluteSpacingLeft( ) const
{
  int borderSizeLeft = 0;
  if (skin_.border!=NULL) borderSizeLeft = skin_.border->spacing().left();
  return absoluteLeft() + borderSizeLeft + spacing().left();
}

int NVisualComponent::absoluteSpacingTop( ) const
{
  int borderSizeTop = 0;
  if (skin_.border!=NULL) borderSizeTop = skin_.border->spacing().top();
  return absoluteTop() + borderSizeTop + spacing().top();
}

int NVisualComponent::scrollDx( ) const
{
  return scrollDx_;
}

int NVisualComponent::scrollDy( ) const
{
  return scrollDy_;
}

void NVisualComponent::setMoveable( const NMoveable & moveable )
{
  moveable_ = moveable;
}

const NMoveable & NVisualComponent::moveable( ) const
{
  return moveable_;
}

void NVisualComponent::onMove( const NMoveEvent & moveEvent )
{

}

void NVisualComponent::repaint(bool swap )
{
  NWindow* win = window();
  if (win != 0) win->repaint(absoluteLeft(),absoluteTop(), width(), height(),swap);
}

void NVisualComponent::setWindow( class NWindow * win )
{
  win_ = win;
}

NWindow * NVisualComponent::window( )
{
  if (parent()==0) return 0;

  if (parent()->visit(NWindow::isWindow)) return (NWindow*) parent();

  if (parent()->visit(NVisualComponent::isVisualComponent)) return ((NVisualComponent*)parent())->window();

  return 0;
}

int NVisualComponent::viewHeight( ) const
{
 return spacingHeight();
}

int NVisualComponent::viewWidth( ) const
{
  return spacingWidth();
}

void NVisualComponent::resize( )
{
  updateAlign();
}

int NVisualComponent::overRectPickPoint( int x, int y )
{
  if (moveable_.style() & nMvRectPicker) {
   int pickWidth  = 4;
   int pickHeight = 4;
   x = x - absoluteLeft();
   y = y - absoluteTop();
   if (mausin(x,y,NRect(0,0,pickWidth,pickHeight)))                      return nUpperLeftCorner;
   if (mausin(x,y,NRect(width()/2-pickWidth/2,0,pickWidth,pickHeight)))  return nUpperMiddleCorner;
   if (mausin(x,y,NRect(width()-pickWidth,0,pickWidth,pickHeight)))      return nUpperRightCorner;
   if (mausin(x,y,NRect(width()-pickWidth,height()/2 - pickHeight/2,pickWidth,pickHeight))) return nRightMiddleCorner;
   if (mausin(x,y,NRect(width()-pickHeight,height()-pickHeight,pickWidth,pickHeight)))      return nLowerRightCorner;
   if (mausin(x,y,NRect(width()/2 -pickWidth/2,height()-pickHeight,pickWidth,pickHeight))) return nLowerMiddleCorner;
   if (mausin(x,y,NRect(0,height()-pickHeight,pickWidth,pickHeight))) return nLowerLeftCorner;
   if (mausin(x,y,NRect(0,height()/2 - pickHeight/2,pickWidth,pickHeight))) return nLeftMiddleCorner;
 }
 return 0;
}


int NVisualComponent::overPickPoint( int x, int y )
{
  if (parent()==NULL) return -1;
  int picker = -1;
  if (parent()->visit(NVisualComponent::isVisualComponent)) {
     NVisualComponent* dragBaseParent = (NVisualComponent*) parent();

     int x_ = x - dragBaseParent->absoluteSpacingLeft()- dragBaseParent->scrollDx();
     int y_ = y - dragBaseParent->absoluteSpacingTop()- dragBaseParent->scrollDy();
     picker = geometry()->overPicker(x_,y_);
   }
  return picker;
}

void NVisualComponent::setClipping( bool on )
{
  clipping_ = on;
}

int NVisualComponent::align( )
{
  return align_;
}

void NVisualComponent::setAlign( int align )
{
  align_ = align;
}

int NVisualComponent::preferredWidth( ) const
{
  if (ownerSizeSet_) return ownerPreferredWidth_;
  if (layout_ == 0) return width(); else
  return layout_->preferredWidth(this)+ spacing().left() + spacing().right() + borderLeft() + borderRight();
}

int NVisualComponent::preferredHeight( ) const
{
  if (ownerSizeSet_) return ownerPreferredHeight_;
  if (layout_ == 0) return height(); else
  return layout_->preferredHeight(this)+ spacing().top() + spacing().bottom() + borderTop() + borderBottom();
}

void NVisualComponent::setEvents( bool on )
{
  events_ = on;
}

bool NVisualComponent::events( ) const
{
  return events_;
}

void NVisualComponent::setTranslucent( NColor color, int percent )
{
  skin_.translucent = percent;
  skin_.transColor  = color;
}

void NVisualComponent::setParentBackground( bool on )
{
  skin_.useParentBgColor = on;
}

NLayout * NVisualComponent::layout( )
{
  return layout_;
}


void NVisualComponent::setFont( const NFont & font )
{
  skin_.font = font;
  skin_.useParentFont = false;
}

int NVisualComponent::zOrder( )
{
  if (parent() == 0) return -1;
  int i = -1;
  std::vector<NRuntime*>::iterator itr = parent()->components.begin();
  for (;itr < parent()->components.end(); itr++) {
      NRuntime* child = *itr;
      if (child->visit(NVisualComponent::isVisualComponent)) i++;
      if (child == this) return i;
  }
  return i;
}

NVisualComponent * NVisualComponent::componentByZOrder( int zOrder )
{
  if (zOrder < 0) return 0;
  int i = -1;
  std::vector<NVisualComponent*>::iterator itr = visualComponents_.begin();
  for (;itr < visualComponents_.end(); itr++) {
      NVisualComponent* child = *itr;
      i++;
      if (i == zOrder) return (NVisualComponent*) child;
  }
  return 0;
}

int NVisualComponent::componentZOrderSize( )
{
  int i = 0;
  std::vector<NRuntime*>::iterator itr = components.begin();
  for (;itr < components.end(); itr++) {
      NRuntime* child = *itr;
      if (child->visit(NVisualComponent::isVisualComponent)) i++;
  }
  return i;
}

void NVisualComponent::setClientSizePolicy( int clPolicy )
{
  clSzPolicy = clPolicy;
}

NBorder * NVisualComponent::border( )
{
  return skin_.border;
}

int NVisualComponent::translucent( ) const
{
  return skin_.translucent;
}

const NColor & NVisualComponent::translucentColor( )
{
  return skin_.transColor;
}

void NVisualComponent::add( NRuntime * component )
{
  NVisual::add(component);
}

void NVisualComponent::add( NVisualComponent * component )
{
  NVisual::add(component);
  visualComponents_.push_back(component);
  if (layout_!=0) layout_->align(this);
}

NRect NVisualComponent::blitMove(int dx, int dy, const NRect & area)
{
   NVisualComponent* comp = this;

   int compLeft   = area.left();
   int compTop    = area.top();
   int compWidth  = area.width();
   int compHeight = area.height();

   NRect repaintArea();
   NRect rect;

   NWindow* win = comp->window();
   NGraphics* g = win->graphics();

   if (dy !=0) {
    int diffY = dy;
    g->setRectRegion(NRect(compLeft,compTop, compWidth, compHeight));
    g->setRegion(g->region(),true);
    if (diffY > 0) {
       g->copyArea(compLeft  , compTop    + diffY, // src_x, sry_y
                 compWidth , compHeight - diffY, // width, height
                 compLeft  , compTop             // destX, destY
       );
       g->swap(NRect(compLeft,compTop, compWidth, compHeight - diffY));
       rect = NRect(compLeft,compTop + compHeight - diffY,compWidth,diffY);
     } else {
       g->copyArea(compLeft  , compTop, // src_x, sry_y
                   compWidth , compHeight + diffY, // width, height
                   compLeft  , compTop    - diffY             // destX, destY
       );
       g->swap(NRect(compLeft,compTop - diffY, compWidth, compHeight + diffY));
       rect = NRect(compLeft,compTop,compWidth,-diffY);
       }
    }

   if (dx !=0) {
    int diffX = dx;
    g->setRectRegion(NRect(compLeft,compTop, compWidth, compHeight));
    g->setRegion(g->region(),true);
    if (diffX > 0) {
       g->copyArea(compLeft  + diffX, compTop, // src_x, sry_y
                 compWidth - diffX, compHeight, // width, height
                 compLeft  , compTop             // destX, destY
       );
       g->swap(NRect(compLeft,compTop, compWidth - diffX, compHeight));
       rect = NRect(compLeft + compWidth - diffX,compTop,diffX,compHeight);
     } else {
       g->copyArea(compLeft  , compTop, // src_x, sry_y
                   compWidth + diffX , compHeight, // width, height
                   compLeft - diffX  , compTop            // destX, destY
       );
       g->swap(NRect(compLeft - diffX,compTop, compWidth + diffX, compHeight));
       rect = NRect(compLeft,compTop,-diffX,compHeight);
     }
   }

  return rect;
}

NRect NVisualComponent::absoluteGeometry( ) const
{
  return NRect(absoluteLeft(),absoluteTop(),width(),height());
}

NRect NVisualComponent::absoluteSpacingGeometry( ) const
{
  return NRect(absoluteSpacingLeft(),absoluteSpacingTop(),spacingWidth(),spacingHeight());
}

void NVisualComponent::removeChilds( )
{
   for (vector<NRuntime*>::iterator it = components.begin(); it < components.end(); it++) {
   NRuntime* child = *it;
    child->setParent(NULL);
    NApp::addRemovePipe(child);
  }
  if (window()!=0) window()->checkForRemove(0);
  components.clear();
  visualComponents_.clear();
}

void NVisualComponent::removeChild( NVisualComponent * child )
{
  std::vector<NRuntime*>::iterator itr = find(components.begin(),components.end(),child);
  components.erase(itr);
  child->setParent(0);
  std::vector<NVisualComponent*>::iterator vItr = find(visualComponents_.begin(),visualComponents_.end(),child);
  visualComponents_.erase(vItr);
  NApp::addRemovePipe(child);
  if (window()!=0) window()->checkForRemove(0);
}

const std::vector< NVisualComponent * > & NVisualComponent::visualComponents( )
{
  return visualComponents_;
}

void NVisualComponent::setSpacing( const NSize & spacing )
{
  setSpacing(spacing.left(),spacing.top(),spacing.right(),spacing.bottom());
}

void NVisualComponent::setParentForeground( bool on )
{
  skin_.useParentFgColor = on;
}

void NVisualComponent::setParentFont( bool on )
{
  skin_.useParentFont = on;
}

NLayout * NVisualComponent::layout( ) const
{
  return layout_;
}

void NVisualComponent::setSkin( NSkin skin )
{
  skin_ = skin;
}

void NVisualComponent::setGradientStyle( int style )
{
  skin_.gradientStyle = style;
}

void NVisualComponent::setGradientOrientation( int orientation )
{
  skin_.gradientOrientation = orientation;
}

void NVisualComponent::onMoveStart( const NMoveEvent & moveEvent )
{
}

void NVisualComponent::onMoveEnd( const NMoveEvent & moveEvent )
{
}

void NVisualComponent::insert( NVisualComponent * component, unsigned int index )
{
  if (index <= visualComponents_.size()) {
    NRuntime::insert(component,index);
    visualComponents_.insert(visualComponents_.begin()+index,component);
  } else add(component);
}

void NVisualComponent::add( NVisualComponent * component, int align )
{
  component->setAlign(align);
  add(component);
}

void NVisualComponent::setPreferredSize( int width, int height )
{
  ownerSizeSet_ = true;
  ownerPreferredWidth_  = width;
  ownerPreferredHeight_ = height;
}

bool NVisualComponent::ownerSize() const {
  return ownerSizeSet_;
}

void NVisualComponent::setMoveFocus( int pickpoint )
{
  if (window() !=0 ) {
     window()->setMoveFocus(this,pickpoint);
  }
}

bool NVisualComponent::focus( ) const
{
  bool b = window()->selectedBase() == this;
  return b;
}

NWindow * NVisualComponent::window( ) const
{
  if (parent()==0) return 0;

  if (parent()->visit(NWindow::isWindow)) return (NWindow*) parent();

  if (parent()->visit(NVisualComponent::isVisualComponent)) return ((NVisualComponent*)parent())->window();

  return 0;
}


