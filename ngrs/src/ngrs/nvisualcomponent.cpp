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
#include "nproperty.h"

using namespace std;

NIsVisualComponent* NVisualComponent::isVisualComponent = new NIsVisualComponent();

NVisualComponent::NVisualComponent()
 : NVisual(), clipping_(1), events_(1), scrollDx_(0), scrollDy_(0), layout_(0), win_(0), clSzPolicy(0), ownerSizeSet_(0), ownerPreferredWidth_(0), ownerPreferredHeight_(0), enabled_(1), tabStop_(0), focusEnabled_(1)
{
  if (properties()) properties()->bind("align", *this, &NVisualComponent::align, &NVisualComponent::setAlign);

  ///\ todo ask bohan why this not works
 // if (properties()) properties()->bind<NVisualComponent,NColor,NColor >(std::string("background"), *this, &NVisualComponent::foreground, &NVisualComponent::setForeground);
}

NVisualComponent::~NVisualComponent()
{
  if (layout_) delete layout_;
}

void NVisualComponent::draw( NGraphics * g, const NRegion & repaintArea , NVisualComponent* sender)
{
  if (visible()) {
   if (sender == this) NWindow::paintFlag = true;

   NRegion oldRegion = g->region();                   // save old region

   NRegion region = geometry()->region();             // get component geometry
   region.move(g->xTranslation(),g->yTranslation());  // move offset left , top

   region &= oldRegion;   // do intersection

   if (!region.isEmpty()) {

    g->resetPen();

    g->setRepaintArea(repaintArea);  // set repaintArea to graphics

    bool clip_ = !(transparent() && (translucent()==100) && skin_.gradientStyle() == 0 );

    if (skin_.bitmapBgStyle()!=0) clip_ = true;

    if (clip_) g->setClipping(region);    //  setClipping
    g->setRegion(region);
    int gTx = g->xTranslation();          // store old graphics translation
    int gTy = g->yTranslation();

    if (!transparent() && (translucent() ==100) && NWindow::paintFlag) {
      g->setForeground(background());
      geometry()->fill(g,repaintArea);
      //std::cout << "j:" << name() << std::endl;
    }
    if (transparent() && (translucent()<100))
         g->fillTranslucent(left(),top(),width(),height(),skin_.transColor(), translucent());

    if (skin_.gradientStyle() == 1 && NWindow::paintFlag) {
         g->fillGradient(left(),top(),spacingWidth(),spacingHeight(),
            skin_.gradientStartColor(),skin_.gradientMidColor(),skin_.gradientEndColor(),
            skin_.gradientOrientation(),skin_.gradientPercent());
    } else
    if (skin_.gradientStyle() == 2 && NWindow::paintFlag) {
         g->fillRoundGradient(left(),top(),spacingWidth(),spacingHeight(),
            skin_.gradientStartColor(),skin_.gradientMidColor(),skin_.gradientEndColor(),
            skin_.gradientOrientation(),skin_.gradientStyle(),skin_.gradientArcWidth(),skin_.gradientArcHeight() );
    }

    if (skin_.bitmapBgStyle() == 1 && NWindow::paintFlag) {
      int w = skin_.bitmap().width();
      int h = skin_.bitmap().height();
      for (int yp = 0; yp < spacingHeight(); yp+=h) {
        for (int xp = 0; xp < spacingWidth(); xp+=w) {
          g->putBitmap( left()+xp,top()+yp,w,h,skin_.bitmap(),0,0);
       }
      }
    } else

    if (skin_.bitmapBgStyle() == 2 && NWindow::paintFlag) {

      int xp =(int)  d2i((spacingWidth()  - skin_.bitmap().width())  / 2.0f);
      int yp = (int) d2i((spacingHeight() - skin_.bitmap().height()) / 2.0f);
      g->putBitmap(left()+xp,top()+yp,skin_.bitmap());
    } else
    if (skin_.bitmapBgStyle() == 3 && NWindow::paintFlag) {
      // stretch      
      if ( width() && height() ) 
        g->putStretchBitmap(left(),top(),skin_.bitmap() , width(), height() );
    }

    if (moveable().style() & nMvRectPicker) geometry()->drawRectPicker(g);
    if (moveable().style() & nMvPolygonPicker) geometry()->drawPicker(g);

    bool spacingClip_ = (!(spacing().left() == 0 && spacing().top() ==0 && spacing().bottom()==0 && spacing().right() == 0 && borderTop()==0 && borderLeft()==0 && borderBottom()==0 && borderRight()==0));

    if (spacingClip_) {
       NRegion spacingRegion = geometry()->spacingRegion(NSize(spacing().left()+borderLeft(),spacing().top()+borderTop(),spacing().right()+borderRight(),spacing().bottom()+borderBottom()));


       spacingRegion.move(g->xTranslation(),g->yTranslation());
       spacingRegion = region & spacingRegion;

       g->setTranslation(g->xTranslation()+left()-scrollDx_+spacing().left()+borderLeft(),g->yTranslation()+top()-scrollDy_+spacing().top()+borderTop());

       g->setClipping(spacingRegion);
       g->setRegion(spacingRegion);

    } else {
        g->setTranslation(g->xTranslation()+left()-scrollDx_,g->yTranslation()-scrollDy_+top());
        if (!clip_) {
          g->setRegion(region);
          g->setClipping(region);
        }
    }

    if (NWindow::paintFlag) {
      g->setForeground(foreground());
      g->setFont(font());
      paint(g);
    }

    drawChildren(g,repaintArea,sender);   // the container children will be drawn

    g->setTranslation(gTx,gTy);           // set back to old translation
    if ((skin_.border()!=0)) {
        g->setClipping(region);
        skin_.border()->paint(g,*geometry());
    }
    if ( focus() ) {
//      g->setForeground( NColor (0,0,255) );
//      g->drawRect(geometry()->rectArea() );
    }
    g->setRegion(oldRegion);              // restore old region

   }
  }
}

void NVisualComponent::onExit() {
  repaint();
}

void NVisualComponent::onEnter() {
  repaint();
}

void NVisualComponent::drawChildren( NGraphics * g, const NRegion & repaintArea , NVisualComponent* sender )
{
  if (layout_ == 0) {
    std::vector<NRuntime*>::iterator itr = components.begin();
    for (;itr < components.end(); itr++) {
      NRuntime* child = *itr;
      if (child->visit(isVisualComponent)) {
       // we know that the Component is a visual Component and can type safe cast due to the visitor pattern
        NVisualComponent* visualChild = (NVisualComponent*) child;
        visualChild->draw(g,repaintArea,sender);
      }
    }
  } else layout_->drawComponents(this,g,repaintArea,sender);
}

bool NVisualComponent::visit( NVisitor * v )
{
  return v->visit_visualcomponent(this);
}

bool NVisualComponent::transparent( ) const
{
  return skin_.transparent();
}

void NVisualComponent::setTransparent( bool on )
{
  skin_.setTransparent(on);
}

void NVisualComponent::setBackground( const NColor & background )
{
   skin_.setBackground(background);
   skin_.useParentBackground(false);
}

const NColor & NVisualComponent::background( )
{
  if (skin_.parentBackground() && parent()->visit(NVisualComponent::isVisualComponent)) {
     return (static_cast<NVisualComponent*> (parent()))->background();
  }
  return skin_.background();
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
  skin_.setSpacing(NSize(left,top,right,bottom));
}

void NVisualComponent::paint( NGraphics * g )
{

}

const NFont & NVisualComponent::font( ) const
{
  if (skin_.parentFont() && parent()!=0 && parent()->visit(NVisualComponent::isVisualComponent)) {
     return ((NVisualComponent*) parent())->font();
  }
  return skin_.font();
}

NVisualComponent * NVisualComponent::overObject( NGraphics* g, long absX, long absY )
{
  NRegion oldRegion = g->region();        // save old region
  NRegion region = geometry()->region();  // get component geometry
  region.move(g->xTranslation(),g->yTranslation());
  region = oldRegion & region;            // intersection

  g->setTranslation(g->xTranslation()+left()-scrollDx_+spacing().left()+borderLeft(),g->yTranslation()+top()-scrollDy_+spacing().top()+borderTop());

  if (!region.isEmpty() && region.intersects(absX,absY) && events()) {
       g->setRegion(region);
       if (visualComponents_.size()>0)
         for( vector<NVisualComponent*>::iterator itr = visualComponents_.end()-1; itr >= visualComponents_.begin(); itr--) {
            NVisualComponent* visualChild = *itr;
            if (visualChild->visible()) {
              NVisualComponent* found = visualChild->overObject(g,absX, absY);
              if (found!=NULL) {
                     g->setTranslation(g->xTranslation()-left()-spacing().left()-borderLeft()+scrollDx_,g->yTranslation()-top()+scrollDy_-spacing().top()-borderTop());
                    g->setRegion(oldRegion);
                   return found;
             }
            }
          }
           g->setTranslation(g->xTranslation()-left()-spacing().left()-borderLeft()+scrollDx_,g->yTranslation()-top()+scrollDy_-spacing().top()-borderTop());
          g->setRegion(oldRegion);

         return this;
  }
    g->setTranslation(g->xTranslation()-left()-spacing().left()-borderLeft()+scrollDx_,g->yTranslation()-top()+scrollDy_-spacing().top()-borderTop());
  g->setRegion(oldRegion);

  return NULL;
}

int NVisualComponent::spacingWidth( ) const
{
  return (skin_.border()==0) ? geometry()->width() - ( spacing().left()+spacing().right()) : geometry()->width() - ( spacing().left()+spacing().right() + skin_.border()->spacing().left() + skin_.border()->spacing().right());
}

int NVisualComponent::spacingHeight( ) const
{
  return (skin_.border()==0) ?  geometry()->height() - ( spacing().top()+spacing().bottom()) : geometry()->height() - ( spacing().top()+spacing().bottom() + skin_.border()->spacing().top() + skin_.border()->spacing().bottom());
}

void NVisualComponent::setBorder( const NBorder & border)
{
  skin_.setBorder(border);
  //borderDeleteFlag_ = deleteFlag;
}

void NVisualComponent::setForeground( const NColor & foreground )
{
  skin_.setForeground(foreground);
  skin_.useParentForeground(false);
}

const NColor & NVisualComponent::foreground( )
{
  if (skin_.parentForeground() && parent()->visit(NVisualComponent::isVisualComponent)) {
     return ((NVisualComponent*) parent())->foreground();
  }
  return skin_.foreground();
}

void NVisualComponent::setLayout(const NLayout & layout) {
   if (layout_ != 0) delete layout_;
   layout_ = layout.clone();
   layout_->setParent(this);
}

void NVisualComponent::updateAlign( )
{
  if (layout_ != NULL) layout_->align(this);
}

int NVisualComponent::clientWidth( ) const
{
  if ((clSzPolicy & nHorizontal) != nHorizontal) return spacingWidth();
  if (layout_ == 0) return preferredWidth();
  return layout_->preferredWidth(this) ;
}

int NVisualComponent::clientHeight( ) const
{
  if ((clSzPolicy & nVertical) != nVertical) return spacingHeight();
  if (layout_ == 0) return preferredHeight();
  return layout_->preferredHeight(this);
}

int NVisualComponent::borderRight( ) const
{
  return (skin_.border()==0) ? 0 :  skin_.border()->spacing().right();
}

int NVisualComponent::borderTop( ) const
{
  return (skin_.border()==0) ? 0 :  skin_.border()->spacing().top();
}

int NVisualComponent::borderBottom( ) const
{
  return (skin_.border()==0) ? 0 :  skin_.border()->spacing().bottom();
}

int NVisualComponent::borderLeft( ) const
{
  return (skin_.border()==0) ? 0 :  skin_.border()->spacing().left();
}

const NSize & NVisualComponent::spacing( ) const
{
  return skin_.spacing();
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
  if (skin_.border()!=NULL) borderSizeLeft = skin_.border()->spacing().left();
  return absoluteLeft() + borderSizeLeft + spacing().left();
}

int NVisualComponent::absoluteSpacingTop( ) const
{
  int borderSizeTop = 0;
  if (skin_.border()!=NULL) borderSizeTop = skin_.border()->spacing().top();
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
  move.emit(moveEvent);
}

void NVisualComponent::repaint(bool swap )
{
  NWindow* win = window();
  if (win != 0) win->repaint(this,NRect(absoluteLeft(),absoluteTop(), width(), height()),swap);
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
   if (NRect(0,0,pickWidth,pickHeight).intersects(x,y))                      return nUpperLeftCorner;
   if (NRect(width()/2-pickWidth/2,0,pickWidth,pickHeight).intersects(x,y))  return nUpperMiddleCorner;
   if (NRect(width()-pickWidth,0,pickWidth,pickHeight).intersects(x,y))      return nUpperRightCorner;
   if (NRect(width()-pickWidth,height()/2 - pickHeight/2,pickWidth,pickHeight).intersects(x,y)) return nRightMiddleCorner;
   if (NRect(width()-pickHeight,height()-pickHeight,pickWidth,pickHeight).intersects(x,y)) return nLowerRightCorner;
   if (NRect(width()/2 -pickWidth/2,height()-pickHeight,pickWidth,pickHeight).intersects(x,y)) return nLowerMiddleCorner;
   if (NRect(0,height()-pickHeight,pickWidth,pickHeight).intersects(x,y)) return nLowerLeftCorner;
   if (NRect(0,height()/2 - pickHeight/2,pickWidth,pickHeight).intersects(x,y)) return nLeftMiddleCorner;
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

void NVisualComponent::onMouseExit() {
  NVisual::onMouseExit();
  NApp::system().setCursor( nCrDefault , window() );
}

void NVisualComponent::onMouseEnter() {
  NVisual::onMouseEnter();
  NApp::system().setCursor( cursor(), window() );
}

int NVisualComponent::align( ) const
{
  return alignConstraint_.align();
}

void NVisualComponent::setAlign( int align )
{
  alignConstraint_.setAlign(align);
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
  skin_.setTranslucent(color,percent);
}

void NVisualComponent::setParentBackground( bool on )
{
  skin_.useParentBackground(on);
}

NLayout * NVisualComponent::layout( )
{
  return layout_;
}


void NVisualComponent::setFont( const NFont & font )
{
  skin_.setFont(font);
  skin_.useParentFont(false);
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
  return skin_.border();
}

int NVisualComponent::translucent( ) const
{
  return skin_.translucent();
}

const NColor & NVisualComponent::translucentColor( )
{
  return skin_.transColor();
}

void NVisualComponent::add( NRuntime * component )
{
  NVisual::add(component);
}

void NVisualComponent::add( NVisualComponent * component )
{
  NVisual::add(component);
  if (component == this) return;

  visualComponents_.push_back(component);
  if (layout_!=0) {
      layout_->add(component);
      layout_->align(this);
  }

  // adding to tabOrderMap
  tabOrder_.push_back(component);
}

void NVisualComponent::add( NVisualComponent * component, int align, bool update )
{
  if (component == this) return;
  component->setAlign(align);

  NVisual::add(component);
  if (component == this) return;

  visualComponents_.push_back(component);
  if (layout_!=0) {
      layout_->add(component);
      if (update) layout_->align(this);
  }

  tabOrder_.push_back(component);
}

void NVisualComponent::add( NVisualComponent * component, const NAlignConstraint & align, bool update )
{
  if (component == this) return;
  component->setAlignConstraint(align);

  NVisual::add(component);
  if (component == this) return;

  visualComponents_.push_back(component);
  if (layout_!=0) {
      layout_->add(component);
      if (update) layout_->align(this);
  }

  tabOrder_.push_back(component);
}

NRect NVisualComponent::blitMove(int dx, int dy, const NRect & area)
{
   NVisualComponent* comp = this;

   int compLeft   = area.left();
   int compTop    = area.top();
   int compWidth  = area.width();
   int compHeight = area.height();

   NRect rect;

   NWindow* win = comp->window();
   NGraphics* g = win->graphics();

   if (dy !=0) {
    int diffY = dy;
    g->setRegion(NRect(compLeft,compTop, compWidth, compHeight));
    g->setClipping(g->region());
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
    g->setRegion(NRect(compLeft,compTop, compWidth, compHeight));
    g->setClipping(g->region());
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
    if (window()!=0) window()->checkForRemove(child);
    child->setParent(NULL);
    NApp::addRemovePipe(child);
  }
  components.clear();
  visualComponents_.clear();
  tabOrder_.clear();

  if (layout_!=0) {
      layout_->removeAll();
      layout_->align(this);
  }
}

void NVisualComponent::removeChild( NVisualComponent * child )
{
  std::cout << "remove child" << std::endl;

  std::vector<NRuntime*>::iterator itr = find(components.begin(),components.end(),child);
  components.erase(itr);
  child->setParent(0);
  std::vector<NVisualComponent*>::iterator vItr = find(visualComponents_.begin(),visualComponents_.end(),child);
  visualComponents_.erase(vItr);

  std::vector<NVisualComponent*>::iterator tItr = find(tabOrder_.begin(),tabOrder_.end(),child);
  if ( tItr != tabOrder_.end() ) tabOrder_.erase(tItr);

  if ( window() ) window()->checkForRemove(child);
    if ( layout_ ) {
      layout_->remove(child);
      layout_->align(this);
		}
  NApp::addRemovePipe(child);
  
}

void NVisualComponent::erase( NVisualComponent * child )
{
  std::vector<NRuntime*>::iterator itr = find(components.begin(),components.end(),child);
  components.erase(itr);
  child->setParent(0);
  std::vector<NVisualComponent*>::iterator vItr = find(visualComponents_.begin(),visualComponents_.end(),child);
  visualComponents_.erase(vItr);
  if (window()!=0) window()->checkForRemove(0);
  if (layout_!=0) {
      layout_->remove(child);
      layout_->align(this);
  }
}

void NVisualComponent::erase( )
{
	if (parent() && parent()->visit(NVisualComponent::isVisualComponent)) { 
  	static_cast<NVisualComponent*>(parent())->erase(this);
	} else
  if ( parent() )
    NVisual::erase();
}

const std::vector< NVisualComponent * > & NVisualComponent::visualComponents( ) const
{
  return visualComponents_;
}

void NVisualComponent::setSpacing( const NSize & spacing )
{
  setSpacing(spacing.left(),spacing.top(),spacing.right(),spacing.bottom());
}

void NVisualComponent::setParentForeground( bool on )
{
  skin_.useParentForeground(on);
}

void NVisualComponent::setParentFont( bool on )
{
  skin_.useParentFont(on);
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
  skin_.setGradientStyle(style);
}

void NVisualComponent::setGradientOrientation( int orientation )
{
  skin_.setGradientOrientation(orientation);
}

void NVisualComponent::onMoveStart( const NMoveEvent & moveEvent )
{
  moveStart.emit(moveEvent);
}

void NVisualComponent::onMoveEnd( const NMoveEvent & moveEvent )
{
  moveEnd.emit(moveEvent);
}

void NVisualComponent::insert( NVisualComponent * component, unsigned int index )
{
  if (component == this) return;
  if (index <= visualComponents_.size()) {
    NRuntime::insert(component,index);
    visualComponents_.insert(visualComponents_.begin()+index,component);
    if (layout_!=0) {
      layout_->insert(component, index);
      layout_->align(this);
    }
  } else add(component);
}

void NVisualComponent::add( NVisualComponent * component, int align )
{
  if (component == this) return;
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

int NVisualComponent::ownerWidth( ) const
{
  return ownerPreferredWidth_;
}

int NVisualComponent::ownerHeight( ) const
{
  return ownerPreferredHeight_;
}

void NVisualComponent::setAlignConstraint( const NAlignConstraint & constraint )
{
  alignConstraint_ = constraint;
}

NAlignConstraint NVisualComponent::alignConstraint( ) const
{
  return alignConstraint_;
}


void NVisualComponent::setFocus( )
{
  NWindow* win = window();
  if (win) {
    win->setFocus(this);
  }
}

void NVisualComponent::setEnable( bool on )
{
  setFont(oldFont);

  if (!on && enabled_) {
    oldFont = font();
    NFont fnt = font();
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

bool NVisualComponent::enabled( ) const
{
  return enabled_;
}


void NVisualComponent::setTabOrder( int index ) {

  if ( parent() != 0 ) {

     NVisualComponent* par = (NVisualComponent*) parent();

     if (index < par->tabOrder_.size() ) {
       std::vector<NVisualComponent*>::iterator it = par->tabOrder_.begin();
       int count = -1;
       for ( ; it < par->tabOrder_.end(); it++ ) {
         NVisualComponent* child = *it;
         count ++;
         if ( count == index ) break;
       }
       std::vector<NVisualComponent*>::iterator oldIt =
          find( par->tabOrder_.begin() , par->tabOrder_.end(), this);

       if (oldIt != par->tabOrder_.end() && count!=-1 && count < par->tabOrder_.size() ) {
         swap(it, oldIt);
       }
     }
  }
}

int NVisualComponent::tabOrder() const {

  if ( parent() ) {
    int count = 0;
    NVisualComponent* par = (NVisualComponent*) parent();
    std::vector<NVisualComponent*>::iterator it = par->tabOrder_.begin();
    for ( ; it < par->tabOrder_.end(); it++ ) {
      NVisualComponent* child = *it;
      if ( child == this ) return count;
      count++;
    }
  }

  return -1;
}

void NVisualComponent::setTabStop( bool on ) {
  tabStop_ = on;
}

bool NVisualComponent::tabStop() const {
  return tabStop_;
}

void NVisualComponent::onKeyPress(const NKeyEvent & event) {

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
      NVisualComponent* par = (NVisualComponent*) parent();
      std::vector<NVisualComponent*>::iterator it = find( par->tabOrder_.begin(),par->tabOrder_.end(), this );
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
}

void NVisualComponent::tabRight() {
  // first give child focus
  if ( visualComponents_.size() > 0) {
     std::vector<NVisualComponent*>::iterator it = visualComponents_.begin();
     window()->setFocus( *it );
  } else
  {
    NVisualComponent* par = (NVisualComponent*) parent();
    if (par->tabOrder_.size() > 0) {
      std::vector<NVisualComponent*>::iterator it=find( par->tabOrder_.begin(),par->tabOrder_.end(), this );
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

void NVisualComponent::tabLeft() {
 if (parent()->visit(isVisualComponent)) {
    NVisualComponent* par = (NVisualComponent*) parent();
 }
}

void NVisualComponent::enableFocus( bool on )
{
  focusEnabled_ = on;
}

bool NVisualComponent::focusEnabled( ) const
{
	if (focusEnabled_ && parent() && parent()->visit(NVisualComponent::isVisualComponent)) {
     return ((NVisualComponent*) parent())->focusEnabled();
  }
  return focusEnabled_;
}
