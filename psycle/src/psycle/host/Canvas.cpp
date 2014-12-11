// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#include <psycle/host/detail/project.private.hpp>
#include "Canvas.hpp"

namespace psycle { namespace host  { namespace canvas {

  Item::Item() : parent_(0), managed_(0), visible_(1)  { }

  Item::~Item() {
    Group* group = parent();
    while (group && group->parent())
      group = group->parent();
    if (group && group->widget() ) 
      if ( group->widget()->button_press_item_ == this )
        group->widget()->button_press_item_ = 0;
    if (parent_ && !managed_) {
      parent_->Erase(this);
    }
  }

  Item::Item(Group* parent) : parent_(parent), managed_(0), visible_(1) {
    assert(parent);
    parent->Add(this);
  }

  Canvas* Item::canvas() {     
      Group* p = this->parent();
      if (!p) return this->widget();
      while (p->parent()) p = p->parent();
      return p->widget();
    }

  void Item::Draw(CDC* cr,
    const CRgn& repaint_region,
  class Canvas* widget) {
  }

  void Item::GetBounds(double& x1, double& y1, double& x2, double& y2) const {
  }

  const CRgn& Item::region() const {
    return rgn_;
  }

  Item* Item::intersect(double x, double y) {
    return 0;
  }

  bool Item::OnEvent(Event* event) {
    //bool erg = m_signal_event_.emit(event);
    return 0;
  }

  void Item::GetFocus() {
    Group* group = this->parent();
    while ( group && group->parent() )
      group = group->parent();
    if (group && group->widget() ) 
      group->widget()->StealFocus(this);
  }

  void Item::QueueDraw() {	
    Group* group = this->parent();
    while (group && group->parent()) {
      group = group->parent();
    }	
    Canvas* canvas = 0;
    if (group && (canvas=group->widget()) ) {
      CRgn rgn;
      rgn.CreateRectRgn(0, 0, 0, 0);
      rgn.CombineRgn(&rgn, &region(), RGN_OR);
      rgn.OffsetRgn(parent()->absx(), parent()->absy());
      if (canvas->IsSaving() ) {
        canvas->save_rgn_.CombineRgn(&canvas->save_rgn_,&rgn,RGN_OR);
      } else {
        if (canvas->parent()) {
          canvas->parent_->InvalidateRgn(&rgn,0);
        }
      }
      rgn.DeleteObject();
    }
  }

  void Item::InvalidateRegion(CRgn* region) {
    Group* group = parent();
    while ( group && group->parent() )
      group = group->parent();
    Canvas* canvas = 0;
    if (group && (canvas=group->widget())) {
      if (canvas->IsSaving() ) {
        canvas->save_rgn_.CombineRgn(&canvas->save_rgn_,region,RGN_OR);
      } else {
        if (canvas->parent())
          canvas->parent_->InvalidateRgn(region, 0);
      }
    }
  } 

  void Item::show() {
     visible_ = true;
  }

  void Item::hide() {
     visible_ = false;
  }
    
  void Item::RectToRegion(CRgn* region,
    double x1,
    double y1,
    double x2,
    double y2) const {
      assert(region);
      swap_smallest(x1,x2);
      swap_smallest(y1,y2);
      //region->union_with_rect(CRect(x1, y1, x2, y2)); todo
  }

  Group::Group() : widget_(0), x_(0), y_(0) {
    rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  Group::Group(Canvas* widget) : widget_(widget), x_(0), y_(0) {
    rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  Group::Group(Group* parent, double x, double y)
    : Item(parent),
    widget_(0),
    x_(x),
    y_(y) {
      rgn_.CreateRectRgn(0, 0, 0, 0);
      SetXY(x, y);
  }

  Group::~Group() {
    std::vector<Item*>::iterator it = items_.begin();
    for ( ; it != items_.end(); ++it ) {
      Item* item = *it;
      if (item->managed())
        delete item;
    }
    rgn_.DeleteObject();
  }

  void Group::Clear() {
    std::vector<Item*>::iterator it = items_.begin();
    for ( ; it != items_.end(); ++it ) {
      Item* item = *it;
      if (item->managed())
        delete item;      
    }
    items_.clear();
  }

  void Group::QueueDraw() {	
    Group* group = this;
    while (group && group->parent()) {
      group = group->parent();
    }	
    if (group && group->widget() ) {
      CRgn rgn;
      rgn.CreateRectRgn(0, 0, 0, 0);
      rgn.CombineRgn(&rgn, &region(), RGN_OR);
      rgn.OffsetRgn(parent() ? parent()->absx() : x(), parent() ? parent()->absy() : y() );
      group->widget()->parent()->InvalidateRgn(&rgn,0);
      rgn.DeleteObject();
    }
  }


/*  void Rect::SetXY(double x, double y) {
    x2_ += x-x1_; 
    y2_ += y-y1_;
    x1_ = x;
    y1_ = y;
    CRgn new_rgn;
    new_rgn.CreateRectRgn(x1_, y1_, x2_+1, y2_+1);
    rgn_.CombineRgn(&rgn_, &new_rgn, RGN_OR);
    InvalidateRegion(&rgn_);    
    rgn_.CopyRgn(&new_rgn);
    update_ = false;
  }*/


  void Group::SetXY(double x, double y) {
    CRgn new_rgn;
    new_rgn.CreateRectRgn(0, 0, 0, 0);
    new_rgn.CopyRgn(&region());
    x_ = x;
    y_ = y;
    // update_ = true;
    new_rgn.CombineRgn(&region(), &new_rgn, RGN_OR);
    new_rgn.OffsetRgn(parent()->absx(), parent()->absy());
    InvalidateRegion(&new_rgn);        
    new_rgn.DeleteObject();
  }

  void Group::Move(double delta_x, double delta_y) {
    CRgn old_rgn;
    old_rgn.CreateRectRgn(0, 0, 0, 0);
    old_rgn.CopyRgn(&region());
    x_ += delta_x;
    y_ += delta_y;
    CRgn rgn;
    rgn.CreateRectRgn(0,0,0,0);
    rgn.CopyRgn(&old_rgn);
    rgn.OffsetRgn(delta_x, delta_y);
    rgn.CombineRgn(&rgn, &old_rgn, RGN_OR);
    rgn.OffsetRgn(parent()->absx(), parent()->absy());
    InvalidateRegion(&rgn);
    rgn.DeleteObject();
    old_rgn.DeleteObject();
  }

  void Group::Draw(CDC* cr,
    const CRgn& repaint_region,
  class Canvas* canvas) {    
    std::vector<Item*>::iterator it = items_.begin();
    for ( ; it != items_.end(); ++it ) {
      Item* item = *it;
      if ( !item->visible() )
        continue;
      if (canvas && !canvas->HasAutomaticDraw()) {
        XFORM rXform;
        cr->GetWorldTransform(&rXform);
        XFORM rXform_new = rXform;
        rXform_new.eDx = x();
        rXform_new.eDy = y();
        cr->SetGraphicsMode(GM_ADVANCED);
        cr->SetWorldTransform(&rXform_new);
        item->Draw(cr, repaint_region, canvas);
        cr->SetGraphicsMode(GM_ADVANCED);
        cr->SetWorldTransform(&rXform);
      } else {
        CRgn item_rgn;
        item_rgn.CreateRectRgn(0, 0, 0, 0);
        item_rgn.CopyRgn(&item->region());
        if (parent())
          item_rgn.OffsetRgn(absx(), absy());
        CRect rc;
        item_rgn.GetRgnBox(&rc);	  
        int erg = item_rgn.CombineRgn(&item_rgn, &repaint_region, RGN_AND);
        if (erg != NULLREGION) {		
          XFORM rXform;
          cr->GetWorldTransform(&rXform);
          XFORM rXform_new = rXform;
          rXform_new.eDx = absx();
          rXform_new.eDy = absy();
          cr->SetGraphicsMode(GM_ADVANCED);
          cr->SetWorldTransform(&rXform_new);
          item->Draw(cr, repaint_region, canvas);
          cr->SetGraphicsMode(GM_ADVANCED);
          cr->SetWorldTransform(&rXform);
        }
        item_rgn.DeleteObject();
      }
    }
  }

  void Group::Add(Item* item) {
    assert(item);
    item->set_parent(this);
    items_.push_back(item);
  }

  void Group::Insert(iterator it, Item* item) {
    assert(item);
    item->set_parent(this);
    items_.insert(it, item);
  }

  void Group::Erase(Item* item) {
    assert(item);
    std::vector<Item*>::iterator it = find(items_.begin(), items_.end(), item);
    assert(it != items_.end());
    CRgn new_rgn;
    new_rgn.CreateRectRgn(0, 0, 0, 0);
    new_rgn.CopyRgn(&region());
    items_.erase(it);
    Canvas* c = canvas();
    if (c) {       
       Item* bp = c->button_press_item_;
       do {
          if (bp == item) {
             c->button_press_item_ = 0;
             break;
          }
          bp = bp->parent();
       } while (bp->parent());
    }        
    // update_ = true;
    // new_rgn.CombineRgn(&region(), &new_rgn, RGN_OR);
    Group* par = parent();
    if (par) {
      new_rgn.OffsetRgn(par->absx(), par->absy());          
    }
    InvalidateRegion(&new_rgn);
    item->set_parent(0);
    new_rgn.DeleteObject();
  }

  void Group::RaiseToTop(Item* item) {
    assert(item);
    Erase(item);
    Add(item);
  }

  void Group::GetBounds(double& x1, double& y1, double& x2,
    double& y2) const {
      bool first = false;
      x1 = x2 = y1 = y2 = 0;      
      std::vector<Item*>::const_iterator it = items_.begin();
      for ( ; it != items_.end(); ++it ) {
        const Item* item = *it;
        if ( item->visible() ) {
          double item_x1, item_y1, item_x2, item_y2;
          item->GetBounds(item_x1, item_y1, item_x2, item_y2);
          if ( first ) {
            x1 = item_x1;
            y1 = item_y1;
            x2 = item_x2;
            y2 = item_y2;
          } else {
            x1 = std::min(item_x1, x1);
            y1 = std::min(item_y1, y1);
            x2 = std::max(item_x2, x2);
            y2 = std::max(item_y2, y2);
          }
        }
      }
      x1 += x();
      y1 += y();
      x2 += x();
      y2 += y();
  }

  const CRgn& Group::region() const {
    CRgn rgn;
    rgn.CreateRectRgn(0, 0, 0, 0);    
    std::vector<Item*>::const_iterator it = items_.begin();
    for ( ; it != items_.end(); ++it) {
      Item* item = *it;
      if (item->visible()) {
        int nCombineResult = rgn.CombineRgn(&rgn, &item->region(), RGN_OR);
        if (nCombineResult == NULLREGION) {
          rgn.DeleteObject();
          rgn.CreateRectRgn(0, 0, 0, 0);
        }
      }
    }
    rgn.OffsetRgn(x_, y_);
    rgn_.CopyRgn(&rgn);
    rgn.DeleteObject();
    return rgn_;
  }

  double Group::absx() const {
    int offset = x_;
    const Group* group = this;
    while (group->parent()) { 
      group = group->parent();
      offset += group->x();
    }
    return offset;
  }

  double Group::absy() const {
    int offset = y_;
    const Group* group = this;
    while (group->parent()) { 
      group = group->parent();
      offset += group->y();
    }
    return offset;
  }

  Item* Group::intersect(double x, double y) {
    Item* found = 0;
    std::vector<Item*>::const_reverse_iterator rev_it = items_.rbegin();
    for ( ; rev_it != items_.rend(); ++rev_it) {
      Item* item = *rev_it;      
      item = item->visible() ? item->intersect(x-this->x(),y-this->y()) : 0;
      if (item) {
        found = item;
        return item;
      }  
    }
    return found;
  }

  Rect::Rect() : 	
    x1_(0),
    y1_(0),
    x2_(0),
    y2_(0),
    r_(0),
    g_(0),
    b_(0),
    alpha_(1),
    r_outline_(0),
    g_outline_(0),
    b_outline_(0),
    alpha_outline_(0) {
      rgn_.CreateRectRgn(0,0,0,0);
  }

  Rect::Rect(Group* parent) :
  Item(parent),
    x1_(0),
    y1_(0),
    x2_(0),
    y2_(0),
    r_(0),
    g_(0),
    b_(0),
    alpha_(1),
    r_outline_(0),
    g_outline_(0),
    b_outline_(0),
    alpha_outline_(0) {
      set_name("rect");
      rgn_.CreateRectRgn(0,0,0,0);
  }

  Rect::Rect(Group* parent, double x1, double y1, double x2, double y2) 
    : Item(parent),
    x1_(x1),
    y1_(y1),
    x2_(x2),
    y2_(y2),
    r_(0),
    g_(0),
    b_(0),
    alpha_(1),
    r_outline_(0),
    g_outline_(0),
    b_outline_(0),
    alpha_outline_(0),
    update_(1) {
      set_name("rect");
      rgn_.CreateRectRgn(0, 0, 0, 0);
      SetXY(0, 0);
  }

  void Rect::SetPos(double x1, double y1, double x2, double y2) {
    CRgn new_rgn;
    new_rgn.CreateRectRgn(0, 0, 0, 0);
    new_rgn.CopyRgn(&region());
    x1_ = x1; 
    y1_ = y1;
    x2_ = x2;
    y2_ = y2;        
    update_ = true;
    new_rgn.CombineRgn(&region(), &new_rgn, RGN_OR);
    new_rgn.OffsetRgn(parent()->absx(), parent()->absy());
    InvalidateRegion(&new_rgn);        
    new_rgn.DeleteObject();
  }

  void Rect::SetXY(double x, double y) {
    CRgn new_rgn;
    new_rgn.CreateRectRgn(0, 0, 0, 0);
    new_rgn.CopyRgn(&region());
    x2_ += x-x1_; 
    y2_ += y-y1_;
    x1_ = x;
    y1_ = y;        
    update_ = true;
    new_rgn.CombineRgn(&region(), &new_rgn, RGN_OR);
    new_rgn.OffsetRgn(parent()->absx(), parent()->absy());
    InvalidateRegion(&new_rgn);        
    new_rgn.DeleteObject();
  }

  void Rect::SetColor(double r, double g, double b, double alpha) {
    r_ = r;
    g_ = g;
    b_ = b;
    alpha_ = alpha;
  }

  void Rect::SetOutlineColor(double r, double g, double b, double alpha) {
    r_outline_ = r;
    g_outline_ = g;
    b_outline_ = b;
    alpha_outline_ = alpha;
  }

  void Rect::Draw(CDC* devc,
    const CRgn& repaint_region,
  class Canvas* widget) {
    COLORREF cref = RGB(r_, g_, b_); 
    CRect rect(x1_, y1_, x2_, y2_);  
    COLORREF penclr = RGB(r_outline_, g_outline_, b_outline_);
    CPen pen;
    pen.CreatePen(PS_SOLID, 1, RGB(r_outline_, g_outline_, b_outline_));
    CBrush brush(RGB(r_, g_, b_));
    COLORREF brushclr = RGB(255,255,255); //r_, g_, b_);
    CBrush* pOldBrush = devc->SelectObject(&brush);
    CPen* pOldPen = devc->SelectObject(&pen);
    // this->paintRect(*devc, rect, penclr, brushclr, 127);
    devc->Rectangle(rect);
    devc->SelectObject(pOldPen);
    devc->SelectObject(pOldBrush);
  }

  Item* Rect::intersect(double x, double y) {
    if ( x1_ < x2_ ) {
      if ( y1_ < y2_ )
        return ( x >= x1_ && x < x2_ && y >= y1_ && y < y2_ ) ? this : 0;
      else 
        return ( x >= x1_ && x < x2_ && y >= y2_ && y < y1_ ) ? this : 0;
    } else {
      if ( y1_ < y2_ )
        return ( x >= x2_ && x < x1_ && y >= y1_ && y < y2_ ) ? this : 0;
      else 
        return ( x >= x2_ && x < x1_ && y >= y2_ && y < y1_ ) ? this : 0;
    }
  }

  void Rect::GetBounds(double& x1, double& y1, double& x2, double& y2) const {
    x1 = x1_;
    y1 = y1_;
    x2 = x2_;
    y2 = y2_;
    swap_smallest(x1,x2);
    swap_smallest(y1,y2);
  }


  bool Rect::paintRect(CDC &hdc, RECT dim, COLORREF penCol, COLORREF brushCol, unsigned int opacity)
{
        HDC tempHdc         = CreateCompatibleDC(hdc);
        BLENDFUNCTION blend = {AC_SRC_OVER, 0, 127, 0};

        HBITMAP hbitmap;       // bitmap handle 
        BITMAPINFO bmi;        // bitmap header 
        // zero the memory for the bitmap info 
        ZeroMemory(&bmi, sizeof(BITMAPINFO));

        // setup bitmap info  
        // set the bitmap width and height to 60% of the width and height of each of the three horizontal areas. Later on, the blending will occur in the center of each of the three areas. 
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = dim.right-dim.left;
        bmi.bmiHeader.biHeight = dim.bottom-dim.top;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;         // four 8-bit components 
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = (dim.right-dim.left) * (dim.bottom-dim.top) * 4;

        // create our DIB section and select the bitmap into the dc 
        hbitmap = CreateDIBSection(tempHdc, &bmi, DIB_RGB_COLORS, NULL, NULL, 0x0);
        SelectObject(tempHdc, hbitmap);

        SetDCPenColor(tempHdc, RGB(0,0,255));
        SetDCBrushColor(tempHdc, RGB(0,0,255));
        HBRUSH br = CreateSolidBrush(RGB(0,0,255));
        FillRect(tempHdc, &dim, br);

        AlphaBlend(hdc, dim.left, dim.top, dim.right, dim.bottom, tempHdc, dim.left, dim.top, dim.right, dim.bottom, blend);
        DeleteObject(hbitmap);
        DeleteObject(br);
        DeleteObject(tempHdc);
        return 0;
}

  const CRgn& Rect::region() const {
    if (update_) {
      rgn_.DeleteObject();
      rgn_.CreateRectRgn(x1_, y1_, x2_+1, y2_+1);
      update_ = false;
    }
    return rgn_;
  }

  Line::Line() :
    r_(0),
    g_(0),
    b_(0),
    alpha_(1),
    update_(1){
      rgn_.CreateRectRgn(0,0,0,0);
  }

  Line::Line(Group* parent)
    : Item(parent),
    r_(0),
    g_(0),
    b_(0),
    alpha_(1),
    update_(1) {
      rgn_.CreateRectRgn(0,0,0,0);
  }


  void Line::Draw(CDC* cr,
    const CRgn& repaint_region,
  class Canvas* widget) {
    COLORREF cref = RGB(r_, g_, b_); 
    cr->SelectObject(GetStockObject(DC_PEN));
    cr->SetDCPenColor(cref);
    Points::iterator it = pts_.begin();
    bool first = true;
    for ( ; it != pts_.end(); ++it ) {
      const std::pair<double, double>& pt = (*it);
      if (first) {        
        cr->MoveTo(pt.first, pt.second);
        first = false;
      } else {
        cr->LineTo(pt.first, pt.second);
      }
    }
  }

  Item* Line::intersect(double x, double y) {
    double distance_ = 5;
    std::pair<double, double>  p1 = PointAt(0);
    std::pair<double, double>  p2 = PointAt(1);

    double  ankathede    = p1.first - p2.first;
    double  gegenkathede = p1.second - p2.second;
    double  hypetenuse   = sqrt( ankathede*ankathede + gegenkathede*gegenkathede);

    if (hypetenuse == 0)
      return 0;

    double cos = ankathede    / hypetenuse;
    double sin = gegenkathede / hypetenuse;

    int dx = static_cast<int> ( -sin*distance_);
    int dy = static_cast<int> ( -cos*distance_);

    std::vector<CPoint> pts;
    CPoint p;
    p.x = p1.first + dx; p.y = p1.second - dy;
    pts.push_back(p);
    p.x = p2.first + dx; p.y = p2.second - dy;
    pts.push_back(p);
    p.x = p2.first - dx; p.y = p2.second + dy;
    pts.push_back(p);
    p.x = p1.first - dx; p.y = p1.second + dy;
    pts.push_back(p);

    CRgn rgn;
    rgn.CreatePolygonRgn(&pts[0],pts.size(), WINDING);
    Item* item = rgn.PtInRegion(x,y) ? this : 0;
    rgn.DeleteObject();
    return item;
  }

  void Line::SetPoints( const Points& pts ) {
    Group* group = parent();
    Canvas* canvas = 0;
    while ( group && group->parent() )
      group = group->parent();
    if (group && group->widget() ) 
      canvas = group->widget();    
    update_ = true;
    CRgn old_rect;
    if (canvas && canvas->HasAutomaticDraw()) {
      old_rect.CreateRectRgn(0,0,0,0);
      old_rect.CombineRgn(&old_rect, &region(), RGN_OR);
    }
    pts_ = pts;
    if (canvas && canvas->HasAutomaticDraw()) {
      CRgn new_rect;
      new_rect.CreateRectRgn(0, 0, 0, 0);
      new_rect.CombineRgn(&old_rect, &region(), RGN_OR);
      this->InvalidateRegion(&new_rect);
      new_rect.DeleteObject();
      old_rect.DeleteObject();
    }
  }

  void Line::SetXY(double x, double y) {
     if (pts_.size() > 0) {
       double delta_x = x - pts_[0].first;
       double delta_y = y - pts_[0].second;
       Points::iterator it = pts_.begin();    
       for ( ; it != pts_.end(); ++it ) {
         std::pair<double, double>& pt = (*it);
         pt.first += delta_x;
         pt.second += delta_y;
       } 
     }     
  }

  void Line::GetBounds(double& x1, double& y1, double& x2, double& y2) const {
    x1 = 0;
    y1 = 0;
    x2 = 0;
    y2 = 0;
    if ( pts_.size() >= 2 ) {
      std::pair<double, double>  p1 = PointAt(0);
      std::pair<double, double>  p2 = PointAt(1);
      x1 = p1.first;
      y1 = p1.second;
      x2 = p2.first;
      y2 = p2.second;
      swap_smallest(x1,x2);
      swap_smallest(y1,y2);
    }
  }

  /*void Line::SetVisible(bool on)
  {
    Item::SetVisible(on);
    update_ = 1;
  }*/

  const CRgn& Line::region() const {
    if ( update_ ) {       
      double x1, y1, x2, y2;
      double dist = 5;
      GetBounds(x1, y1, x2, y2);
      rgn_.DeleteObject();
      rgn_.CreateRectRgn(x1-dist, y1-dist, x2+2*dist+1, y2+2*dist+1);
      update_ = false;
    }
    return rgn_;
  }

  Text::Text()
    : x_(0), 
    y_(0), 
    r_(0),
    g_(0),
    b_(0),
    alpha_(1),
    update_(true) {
      LOGFONT lfLogFont;
      memset(&lfLogFont, 0, sizeof(lfLogFont));
      lfLogFont.lfHeight = 12;
      strcpy(lfLogFont.lfFaceName, "Arial");
      font_.CreateFontIndirect(&lfLogFont);
      rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  Text::Text(Group* parent)
    : Item(parent),
    x_(0),
    y_(0),
    r_(0),
    g_(0),
    b_(0),
    alpha_(1),
    update_(true) {
      set_name("text");
      LOGFONT lfLogFont;
      memset(&lfLogFont, 0, sizeof(lfLogFont));
      lfLogFont.lfHeight = 12;
      strcpy(lfLogFont.lfFaceName, "Arial");
      font_.CreateFontIndirect(&lfLogFont);
      rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  Text::Text(Group* parent, const std::string& text)
    : Item(parent),
    text_(text),
    x_(0),
    y_(0),
    r_(0),
    g_(0),
    b_(0),
    alpha_(1),
    update_(true) {
      LOGFONT lfLogFont;
      memset(&lfLogFont, 0, sizeof(lfLogFont));
      lfLogFont.lfHeight = 12;
      strcpy(lfLogFont.lfFaceName, "Arial");
      font_.CreateFontIndirect(&lfLogFont);
      rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  void Text::SetColor(double r, double g, double b, double alpha) {
    r_ = r;
    g_ = g;
    b_ = b;
    alpha_ = alpha;
  }

  void Text::SetXY(double x, double y) {
    x_ = x;
    y_ = y;
  }

  void Text::SetText(const std::string& text) {    
    CRgn new_rgn;
    new_rgn.CreateRectRgn(0, 0, 0, 0);
    new_rgn.CopyRgn(&region());
    text_ = text;
    update_ = true;
    new_rgn.CombineRgn(&region(), &new_rgn, RGN_OR);
    new_rgn.OffsetRgn(parent()->absx(), parent()->absy());
    InvalidateRegion(&new_rgn);        
    new_rgn.DeleteObject();
  }

  void Text::UpdateValues() const {    
    Canvas* c =  const_cast<Text*>(this)->canvas();
    if (c) {
      HDC dc = GetDC(0);
      SIZE extents = {0};
      HFONT old_font =
        reinterpret_cast<HFONT>(SelectObject(dc, font_));
      GetTextExtentPoint32(dc, text_.c_str(), text_.length(),
        &extents); 
      SelectObject(dc, old_font);
      ReleaseDC(0, dc);
      text_w = extents.cx;
      text_h = extents.cy;
      rgn_.DeleteObject();
      rgn_.CreateRectRgn(x_, y_, x_ + text_w +1, y_ + text_h+1);
      update_ = false;
    }
  }

  Item* Text::intersect(double x, double y) {
     return (x >= x_ && x <= x_+text_w && y >= y_ && y <= y_+text_h) ? this : 0;
  }

  void Text::GetBounds(double& x1, double& y1, double& x2, double& y2) const {
    x1 = x_;
    y1 = y_;
    if (update_ )
      UpdateValues();
    x2 = text_w + x1;
    y2 = text_h + y1;
  }

  const CRgn& Text::region() const {
    if (update_)
      UpdateValues();       
    return rgn_;
  }

  void Text::Draw(CDC* devc,                      
    const CRgn& repaint_region,
    Canvas* widget) {
      CFont* oldFont= devc->SelectObject(&font_);
      devc->SetBkMode(TRANSPARENT);
      COLORREF cref = RGB(r_ * 255, g_* 255, b_ * 255); 
      devc->SetTextColor(cref);
      devc->TextOut(x_, y_, text_.c_str());
      devc->SetBkMode(OPAQUE);
      devc->SelectObject(oldFont);
  }

  PixBuf::PixBuf()
    : image_(0),
    mask_(0),
    x_(0),
    y_(0),
    width_(0),
    height_(0),
    xsrc_(0),
    ysrc_(0),
    transparent_(false) {
      rgn_.CreateRectRgn(0, 0, 0, 0);
      SetXY(0, 0);
  }

  PixBuf::PixBuf(Group* parent)
    : Item(parent),
    image_(0),
    mask_(0),
    x_(0),
    y_(0),
    width_(0),
    height_(0),
    xsrc_(0),
    ysrc_(0),
    transparent_(false) {
      rgn_.CreateRectRgn(0, 0, 0, 0);
      // test case        
      image_ = new CBitmap();
      image_->LoadBitmap(IDB_KNOB);
      BITMAP bm;
      image_->GetBitmap(&bm);    
      width_ = bm.bmWidth;
      height_ = bm.bmHeight;    
      SetXY(0, 0);
  }

  PixBuf::PixBuf(Group* parent, double x, double y, CBitmap* image)
    : Item(parent),
    image_(image),
    mask_(0),
    x_(x),
    y_(y),
    width_(0),
    height_(0),
    xsrc_(0),
    ysrc_(0),
    transparent_(false) {
      BITMAP bm;
      image->GetBitmap(&bm);
      width_ = bm.bmWidth;
      height_ = bm.bmHeight;    
      rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  PixBuf::~PixBuf() {}

  void PixBuf::TransparentBlt(CDC* pDC,
    int xStart,
    int yStart,
    int wWidth,
    int wHeight,
    CDC* pTmpDC,
    CBitmap* bmpMask,
    int xSource, // = 0
    int ySource) // = 0)
  { 
    // We are going to paint the two DDB's in sequence to the destination.
    // 1st the monochrome bitmap will be blitted using an AND operation to
    // cut a hole in the destination. The color image will then be ORed
    // with the destination, filling it into the hole, but leaving the
    // surrounding area untouched.
    CDC hdcMem;
    hdcMem.CreateCompatibleDC(pDC);
    CBitmap* hbmT = hdcMem.SelectObject(bmpMask);
    pDC->SetTextColor(RGB(0,0,0));
    pDC->SetBkColor(RGB(255,255,255));
    if (!pDC->BitBlt( xStart, yStart, wWidth, wHeight, &hdcMem, xSource, ySource, 
      SRCAND)) {
        TRACE("Transparent Blit failure SRCAND");
    }
    // Also note the use of SRCPAINT rather than SRCCOPY.
    if (!pDC->BitBlt(xStart, yStart, wWidth, wHeight, pTmpDC, xSource, ySource,
      SRCPAINT)) {
        TRACE("Transparent Blit failure SRCPAINT");
    }
    // Now, clean up.
    hdcMem.SelectObject(hbmT);
    hdcMem.DeleteDC(); 
  }

  void PixBuf::Draw(CDC* devc,
    const CRgn& repaint_region,
  class Canvas* widget) {
    CDC memDC;
    CBitmap* oldbmp;
    memDC.CreateCompatibleDC(devc);
    oldbmp=memDC.SelectObject(image_);	
    if (!transparent_) {
      devc->BitBlt(x(), y(), width_, height_, &memDC, xsrc_, ysrc_, SRCCOPY);      
    } else {
      TransparentBlt(devc, x(),  y(), width_, height_, &memDC, mask_, xsrc_, ysrc_);
    }
    memDC.SelectObject(oldbmp);
    memDC.DeleteDC(); 
  }

  void PixBuf::GetBounds(double& x1, double& y1, double& x2, double& y2) const {
    x1 = x_;
    y1 = y_;
    x2 = x_ + width_;
    y2 = y_ + height_;    
  }

  void PixBuf::SetXY(double x, double y) {
    x_ = x;
    y_ = y;
  }

  void PixBuf::SetSize(int width, int height) {
    width_ = width;
    height_ = height;
  }

  void PixBuf::SetSource(int xsrc, int ysrc) {
    xsrc_ = xsrc;
    ysrc_ = ysrc;
  }

  void PixBuf::SetImage(CBitmap* image) {
    image_ = image;
    CSize size = image_->GetBitmapDimension();
    width_ = size.cx;
    height_ = size.cy;
  }

  void PixBuf::SetMask(CBitmap* mask) {
    mask_ = mask;
  }

  const CRgn& PixBuf::region() const {
    rgn_.DeleteObject();
    rgn_.CreateRectRgn(x_, y_, x_ + width_, y_ + height_);
    return rgn_;
  }

  Item* PixBuf::intersect(double x, double y) {
    double x1_, y1_, x2_, y2_;
    x1_ = x_;
    y1_ = y_;
    x2_ = x_ + width_;
    y2_ = y_ + height_;
    if ( x1_ < x2_ ) {
      if ( y1_ < y2_ )
        return ( x >= x1_ && x < x2_ && y >= y1_ && y < y2_ ) ? this : 0;
      else 
        return ( x >= x1_ && x < x2_ && y >= y2_ && y < y1_ ) ? this : 0;
    } else {
      if ( y1_ < y2_ )
        return ( x >= x2_ && x < x1_ && y >= y1_ && y < y2_ ) ? this : 0;
      else 
        return ( x >= x2_ && x < x1_ && y >= y2_ && y < y1_ ) ? this : 0;
    }
  }

  void PixBuf::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans) {
    BITMAP bm;
    // Get the dimensions of the source bitmap
    pBmpSource->GetObject(sizeof(BITMAP), &bm);
    // Create the mask bitmap
    pBmpMask->DeleteObject();
    pBmpMask->CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL);
    // We will need two DCs to work with. One to hold the Image
    // (the source), and one to hold the mask (destination).
    // When blitting onto a monochrome bitmap from a color, pixels
    // in the source color bitmap that are equal to the background
    // color are blitted as white. All the remaining pixels are
    // blitted as black.
    CDC hdcSrc, hdcDst;
    hdcSrc.CreateCompatibleDC(NULL);
    hdcDst.CreateCompatibleDC(NULL);
    // Load the bitmaps into memory DC
    CBitmap* hbmSrcT = (CBitmap*) hdcSrc.SelectObject(pBmpSource);
    CBitmap* hbmDstT = (CBitmap*) hdcDst.SelectObject(pBmpMask);
    // Change the background to trans color
    hdcSrc.SetBkColor(clrTrans);
    // This call sets up the mask bitmap.
    hdcDst.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcSrc,0,0,SRCCOPY);
    // Now, we need to paint onto the original image, making
    // sure that the "transparent" area is set to black. What
    // we do is AND the monochrome image onto the color Image
    // first. When blitting from mono to color, the monochrome
    // pixel is first transformed as follows:
    // if  1 (black) it is mapped to the color set by SetTextColor().
    // if  0 (white) is is mapped to the color set by SetBkColor().
    // Only then is the raster operation performed.
    hdcSrc.SetTextColor(RGB(255,255,255));
    hdcSrc.SetBkColor(RGB(0,0,0));
    hdcSrc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcDst,0,0,SRCAND);
    // Clean up by deselecting any objects, and delete the
    // DC's.
    hdcSrc.SelectObject(hbmSrcT);
    hdcDst.SelectObject(hbmDstT);
    hdcSrc.DeleteDC();
    hdcDst.DeleteDC();
  }


  Canvas::Canvas() :
  parent_(0),
    root_(this),
    save_(false),
    has_draw_(true),
    button_press_item_(0),
    steal_focus_(0),
    bg_image_(0),
    bg_width_(0),
    bg_height_(0),
    bg_color_(0),
    cw_(300),
    ch_(200) {
      save_rgn_.CreateRectRgn(0, 0, 0, 0);
  }


  Canvas::Canvas(CWnd* parent) :
  parent_(parent),
    root_(this),
    save_(false),
    has_draw_(true),
    button_press_item_(0),
    steal_focus_(0),
    bg_image_(0),
    bg_width_(0),
    bg_height_(0),
    cw_(300),
    ch_(200) {
      save_rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  Canvas::~Canvas() {
    save_rgn_.DeleteObject();
  }

  void Canvas::OnSize(int cx, int cy) {	
    cw_ = cx;
    ch_ = cy;
  }

  void Canvas::Draw(CDC *devc, const CRgn& rgn) {
    if (bg_image_)	{
      CDC memDC;
      CBitmap* oldbmp;
      memDC.CreateCompatibleDC(devc);
      oldbmp=memDC.SelectObject(bg_image_);
      if ((cw_ > bg_width_) || (ch_ > bg_height_)) {
        for (int cx=0; cx<cw_; cx+=bg_width_) {
          for (int cy=0; cy<ch_; cy+=bg_height_) {
            devc->BitBlt(cx,cy,bg_width_,bg_height_,&memDC,0,0,SRCCOPY);
          }
        }
      }
      else {
        devc->BitBlt(0,0,cw_,ch_,&memDC,0,0,SRCCOPY);
      }
      memDC.SelectObject(oldbmp);
      memDC.DeleteDC();
    } else {		
      CRect rect;
      rgn.GetRgnBox(&rect);
      devc->FillSolidRect(&rect, bg_color_);
    }
    root_.Draw(devc, rgn, this);
  }

  void Canvas::StealFocus(Item* item) {
    button_press_item_ = item;
    steal_focus_ = true;
  }

  Item* Canvas::OnEvent(Event* ev) {
    Item* item = button_press_item_;
    if (ev->type() == Event::BUTTON_PRESS || ev->type() == Event::BUTTON_2PRESS) {
      item = button_press_item_ = root_.intersect(ev->x(), ev->y());
    } else
    if (ev->type() == Event::MOTION_NOTIFY ) {
        item = button_press_item_ ? button_press_item_ : root_.intersect(ev->x(), ev->y());
    }     
    if (item) {
      Group* parent = item->parent();
      Event event(item, ev->type(),
          ev->x() - (parent ? parent->absx() : 0),
          ev->y() - (parent ? parent->absy() : 0),
          ev->button(),
          ev->shift());                        
      DelegateEvent(&event, item);
      item = event.item();
    }
    if (ev->type() == Event::BUTTON_RELEASE || ev->type() == Event::BUTTON_2PRESS) {
      button_press_item_ = 0;       
    }
    return item;
  }  

  bool Canvas::DelegateEvent(Event* ev, Item* item) {
    bool erg = item->OnEvent(ev);
    while (!erg && item->parent()) {
      // redirect event to parent      
      item = item->parent();
      button_press_item_ = item;
      ev->setxy(ev->x()+item->x(), ev->y()+item->y());
      ev->setitem(item);
      erg = item->OnEvent(ev);      
      if (steal_focus_) { 
        steal_focus_ = 0;
        return erg;
      }       
    }
    return erg;
  }

  void Canvas::Flush() {
    if (parent_)
      parent_->InvalidateRgn(&save_rgn_,0);
    save_rgn_.DeleteObject();
    save_rgn_.CreateRectRgn(0, 0, 0, 0);
  }

}}}

