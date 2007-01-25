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
#ifndef CUSTOMTREEVIEW_H
#define CUSTOMTREEVIEW_H

#include "panel.h"
#include "flipbox.h"
#include "treenode.h"
#include "image.h"

/**
@author  Stefan Nattkemper
*/

namespace ngrs {
  
  class ScrollBox;
  class Label;
  
  class TreeNodeGui : public Panel {
  public:

    class FlipperImage : public Image {
    public :
       FlipperImage( const TreeNodeGui& nodeGui );
      ~FlipperImage();

      virtual void paint( Graphics& g );
      virtual int preferredWidth() const;

    private:
       const TreeNodeGui& nodeGui_;
    };

    class ChildPanel : public Panel {
    public :
      ChildPanel( const TreeNodeGui& nodeGui );
      ~ChildPanel();

      virtual void paint( Graphics& g );

    private:
      const TreeNodeGui& nodeGui_; 

    };

    TreeNodeGui( class CustomTreeView& treeView, TreeNode* node );
    ~TreeNodeGui();
    TreeNode* node() const;
    const class CustomTreeView& treeView() const;
    int flipperOffset() const;

    virtual void onMouseOver( int x, int y );
    virtual void add( TreeNodeGui* nodeGui );
    virtual void resize();
    virtual int preferredWidth() const;
    virtual int preferredHeight() const;
    virtual void setSkin( const Skin& skin );
  
  private:

      FlipperImage* img_;
      Label* label_;
      ChildPanel* children_;
      bool expanded_;

      Bitmap expandBmp;
      Bitmap expandedBmp;

      class CustomTreeView& treeView_;
      TreeNode* node_;

      void onImgClick( ButtonEvent* ev );

  };

  class CustomTreeView : public Panel {
  public:
    
    CustomTreeView();
    CustomTreeView( TreeNode* rootNode );

    ~CustomTreeView();

    signal1<TreeNode&> nodeClicked;
    signal1<TreeNode&> nodeDblClick;

    void updateTree();
    TreeNode* selectedTreeNode();

  protected:

    void buildTree( TreeNode* node, TreeNodeGui* nodeGui );

  private:

    ScrollBox* scrollBox_;
    Panel* scrollArea_;
    TreeNode* rootNode_;
    TreeNodeGui* selectedTreeNodeGui_;
    Skin nodeSkinSelected_;
    Skin nodeSkinNone_;

    void init();
    void onNodeMousePress( ButtonEvent* ev );
    void onNodeDblClick( ButtonEvent* ev );

  };

}

#endif
