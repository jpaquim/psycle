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
#include "flowlayout.h"

using namespace std;

namespace ngrs {

  FlowLayout::FlowLayout( )
    : Layout( )
  {
    init( );
  }

  FlowLayout::FlowLayout( int align ) 
    : Layout( )
  {
    init( );
    setAlign( align );
  }

  FlowLayout::FlowLayout( int align, int hgap, int vgap, int baseLine ) 
    : Layout()
  {
    init();
    align_ = align;
    hgap_ = hgap;
    vgap_ = vgap;
    baseLine_ = nAlCenter;
  }

  void FlowLayout::init( )
  {
    maxX = maxY = 0;
    align_ = nAlClient;
    hgap_ = vgap_ = 5;
    lineBrk_ = true;
    baseLine_ = nAlBottom;
  }

  FlowLayout * FlowLayout::clone( ) const
  {
    return new FlowLayout(*this);
  }

  FlowLayout::~FlowLayout()
  {
  }

  void FlowLayout::align( VisualComponent * parent )
  {
    std::vector<VisualComponent*>::const_iterator itr   = parent->visualComponents().begin();
    std::vector<VisualComponent*>::const_iterator start = itr;

    int xp = hgap_;
    int yp = vgap_;
    int ymax = 2*vgap_;

    if (align_ == nAlLeft) {

      for (;itr < parent->visualComponents().end(); itr++) {
        VisualComponent* visualChild = *itr;
        if (visualChild->visible()) {
          if ((xp + visualChild->preferredWidth() + hgap_ <= parent->clientWidth()) || (!lineBrk_) ) {
            visualChild->setPosition(xp,yp,visualChild->preferredWidth(),visualChild->preferredHeight());
            xp = xp + visualChild->preferredWidth() + hgap_;
            if (ymax<visualChild->preferredHeight()) ymax = visualChild->preferredHeight();
          }
          else {
            std::vector<VisualComponent*>::const_iterator itrH = start;
            for (;itrH < itr; itrH++) {
              VisualComponent* visual = *itrH;
              if (baseLine_ == nAlBottom) {
                visual->setTop(yp+ymax-visual->preferredHeight());
              } else
                if (baseLine_ == nAlTop) {
                  visual->setTop(yp);
                }
            }
            yp = yp + ymax + vgap_;
            ymax = visualChild->preferredHeight();
            visualChild->setPosition(hgap_,yp,visualChild->preferredWidth(),visualChild->preferredHeight());
            xp = visualChild->preferredWidth() + 2*hgap_;
            start = itr;
          }
        }
      }
      if (itr != start) {
        std::vector<VisualComponent*>::const_iterator itrH = start;
        for (;itrH < parent->visualComponents().end() ; itrH++) {
          VisualComponent* visual = *itrH;
          if (visual->align()==nAlHeight) {
            visual->setTop(vgap_);
            visual->setHeight(parent->clientHeight()-2*vgap_);
          } else
            if (baseLine_ == nAlBottom) {
              visual->setTop(yp+ymax-visual->preferredHeight());
            } else
              if (baseLine_ == nAlTop) {
                if (baseLine_ == nAlTop) {
                  visual->setTop(yp);
                }
              } else 
                if (baseLine_ == nAlCenter) {
                  visual->setTop(yp+(ymax-visual->preferredHeight())/2);
                }
        }
      }

    } else
      if (align_ == nAlRight) {
        xp = parent->clientWidth();
        for (;itr < parent->visualComponents().end(); itr++) {
          VisualComponent* visualChild = *itr;
          xp = xp - visualChild->preferredWidth() - hgap_;
          if (xp<0) {
            xp = parent->clientWidth();
            std::vector<VisualComponent*>::const_iterator itrH = itr;		
            if ( itrH > parent->visualComponents().begin() ) itrH--; 
            while ( itrH >= start ) {
              VisualComponent* visual = *itrH;
              xp = xp - visual->preferredWidth() - hgap_;
              visual->setPosition(xp,yp+ymax-visual->preferredHeight(),visual->preferredWidth(),visual->preferredHeight());
              if ( itrH == start ) break;
              itrH--;
            }
            xp   = parent->clientWidth() - visualChild->preferredWidth() - hgap_;
            ymax = visualChild->preferredHeight();
            start = itr;
            yp = yp + ymax + vgap_;
          }
          if (ymax<visualChild->height()) ymax = visualChild->height();
        }
        if (start!=itr) {
          xp = parent->clientWidth();
          std::vector<VisualComponent*>::const_iterator itrH = itr; 
          if ( itrH > parent->visualComponents().begin() ) itrH--;
          while ( itrH >= start ) {
            VisualComponent* visual = *itrH;
            xp = xp - visual->preferredWidth() - hgap_;
            visual->setPosition(xp,yp+ymax-visual->preferredHeight(),visual->preferredWidth(),visual->preferredHeight());
            if ( itrH == parent->visualComponents().begin() ) break;
            itrH--;
          }
        }
      }

      /* else
      if (align_ == nAlClient) {
      xp = parent->clientWidth();
      for (;itr < parent->components.end(); itr++) {
      Runtime* child = *itr;
      if (child->visit(VisualComponent::isVisualComponent)) {
      VisualComponent* visualChild = (VisualComponent*) child;
      int offset = xp;
      xp = xp - visualChild->preferredWidth() - hgap_;
      if (xp<0) {
      xp = parent->clientWidth() - offset / 2;
      std::vector<Runtime*>::iterator itrH = itr; itrH--;
      for (;itrH >= start ; itrH--) {
      Runtime* child = *itrH;
      if (child->visit(VisualComponent::isVisualComponent)) {
      VisualComponent* visual = (VisualComponent*) child;
      xp = xp - visual->preferredWidth()-hgap_;
      visual->setPosition(xp,yp+ymax-visual->preferredHeight(),visual->preferredWidth(),visual->preferredHeight());
      }
      }
      xp   = parent->clientWidth() - visualChild->preferredWidth() - hgap_;
      ymax = visualChild->preferredHeight();
      start = itr;
      yp = yp + ymax + vgap_;
      }
      if (ymax<visualChild->preferredHeight()) ymax = visualChild->preferredHeight();
      }
      }
      if (start!=itr) {
      int offset = xp;
      xp = parent->clientWidth() - offset / 2;
      std::vector<Runtime*>::iterator itrH = itr; itrH--;
      for (;itrH >= start ; itrH--) {
      Runtime* child = *itrH;
      if (child->visit(VisualComponent::isVisualComponent)) {
      VisualComponent* visual = (VisualComponent*) child;
      xp = xp - visual->preferredWidth();
      xp = xp - hgap_;
      visual->setPosition(xp,yp+ymax-visual->preferredHeight(),visual->preferredWidth(),visual->preferredHeight());
      }
      }

      }*/

      maxY = yp + ymax + vgap_;
      maxX = xp;
  }

  int FlowLayout::preferredWidth( const VisualComponent * target ) const
  {
    int xp = hgap_;
    std::vector<VisualComponent*>::const_iterator itr   = parent()->visualComponents().begin();

    for (;itr < parent()->visualComponents().end(); itr++) {
      VisualComponent* visualChild = *itr;
      xp = xp + visualChild->preferredWidth() + hgap_;
    }

    if (xp == hgap_) xp+= hgap_;

    return xp + target->spacing().left() + target->spacing().right() + target->borderLeft() + target->borderRight() ;
  }

  int FlowLayout::preferredHeight( const VisualComponent * target ) const
  {
    int xp = hgap_;
    int yp = vgap_;
    int ymax = 2*vgap_;

    std::vector<VisualComponent*>::const_iterator itr   = parent()->visualComponents().begin();
    std::vector<VisualComponent*>::const_iterator start = itr;


    for (;itr < parent()->visualComponents().end(); itr++) {
      VisualComponent* visualChild = *itr;
      if (visualChild->visible()) {
        if ((xp + visualChild->preferredWidth() + hgap_ <= parent()->clientWidth()) || (!lineBrk_) ) {
          xp = xp + visualChild->preferredWidth() + hgap_;
          if (ymax<visualChild->preferredHeight()) ymax = visualChild->preferredHeight();
        }
        else {
          std::vector<VisualComponent*>::const_iterator itrH = start;
          for (;itrH < itr; itrH++) {
            VisualComponent* visual = *itrH;
          }
          yp = yp + ymax + vgap_;
          ymax = visualChild->preferredHeight();
          xp = visualChild->preferredWidth() + 2*hgap_;
          start = itr;
        }
      }
    }
    return yp + ymax + vgap_;
  }

  void FlowLayout::setAlign( int align )
  {
    align_ = align;
  }

  void FlowLayout::setHgap( int hgap )
  {
    hgap_ = hgap;
  }

  void FlowLayout::setVgap( int vgap )
  {
    vgap_ = vgap;
  }

  int FlowLayout::align( )
  {
    return align_;
  }

  int FlowLayout::hgap( )
  {
    return hgap_;
  }

  int FlowLayout::vgap( )
  {
    return vgap_;
  }

  void FlowLayout::setLineBreak( bool on )
  {
    lineBrk_ = on;
  }



  void FlowLayout::drawComponents( VisualComponent * target, Graphics& g, const ngrs::Region & repaintArea ,VisualComponent* sender)
  {
    if (!lineBrk_) {
      std::vector<Runtime*>::iterator itr = target->components.begin();
      int i = 0;
      for (;itr < target->components.end(); itr++) {
        Runtime* child = *itr;
        if (child->visit(VisualComponent::isVisualComponent)) {
          // we know that the Component is a visual Component and can type safe cast due to the visitor pattern
          VisualComponent* visualChild = (VisualComponent*) child; i++;
          visualChild->draw(g,repaintArea,sender);
          if(visualChild->width()+visualChild->left()>repaintArea.rectClipBox().left()+repaintArea.rectClipBox().width()) {
          }
        }
      }
    } else Layout::drawComponents(target,g,repaintArea,sender);
  }



  void FlowLayout::setBaseLine( int line )
  {
    baseLine_ = line;
  }

}

