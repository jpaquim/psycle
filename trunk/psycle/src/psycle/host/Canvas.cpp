// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#include <psycle/host/detail/project.private.hpp>
#include "Canvas.hpp"
#include <algorithm> // for std::transform
#include <cctype> // for std::tolower

namespace psycle { namespace host  { namespace canvas {

  Item::Item() : parent_(0), managed_(0), visible_(1), pointer_events_(1), has_store_(false), update_(false) {
    rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  Item::Item(Group* parent) : parent_(parent), managed_(0), visible_(1), pointer_events_(1), has_store_(false), update_(false) {
    rgn_.CreateRectRgn(0, 0, 0, 0);
    if (parent) {
      parent->Add(this);
    }
  }

  Item::~Item() {
    rgn_.DeleteObject();
    Canvas* c = canvas();
    if (c) {
      Item* bp = c->button_press_item_;
      while (bp) {       
        if (bp == this) {
          c->button_press_item_ = 0;
          break;
        }
        bp = bp->parent();
      }
    }
    if (parent_ && !managed_) {
      parent_->Erase(this);
    }
  }

  Canvas* Item::canvas() {     
    Group* p = this->parent();
    if (!p) return this->widget();
    while (p->parent()) p = p->parent();
    return p->widget();
  }

  void Item::needsupdate() {
    update_ = true;
    if (parent()) {
      parent()->needsupdate();
    }
  }

  void Item::GetFocus() {
    Canvas* c = canvas();
    if (c) {
      c->StealFocus(this);
    }    
  }

  void Item::store() {    
    rgn_.DeleteObject();
    rgn_.CreateRectRgn(0, 0, 0, 0);
    rgn_.CopyRgn(&region());
    has_store_ = true;
  }

  void Item::flush() {
    if (has_store_) {  
      needsupdate();
      rgn_.CombineRgn(&region(), &rgn_, RGN_OR);
      has_store_ = false;
    } else {
      rgn_.DeleteObject();
      rgn_.CreateRectRgn(0, 0, 0, 0);
      rgn_.CopyRgn(&region());
    }  
    if (parent()) {
      rgn_.OffsetRgn(parent()->absx(), parent()->absy());
      Canvas* c = this->canvas();
      if (c) {
        c->Invalidate(rgn_);
      }
    }    
  }

  void Item::show() {
    store(); 
    visible_ = true; 
    flush();
  }

  void Item::hide() {
    store();
    visible_ = false;    
    flush();
  }

  double Item::absx() const { return (parent() ? parent()->absx() : 0) + x(); }
  double Item::absy() const { return (parent() ? parent()->absy() : 0) + y(); }

  void PaintItem::Draw(CDC* devc, const CRgn& repaint_rgn, class Canvas* widget) {
  }

  Group::Group() : widget_(0), x_(0), y_(0), is_root_(false) {
    rgn_.CreateRectRgn(0, 0, 0, 0);
    zoom_ =1.0;
  }

  Group::Group(Canvas* widget) : widget_(widget), x_(0), y_(0), is_root_(false) {
    rgn_.CreateRectRgn(0, 0, 0, 0);
    zoom_ = 1.0;
  }

  Group::Group(Group* parent, double x, double y) :
  Item(parent),
    widget_(0),
    x_(x),
    y_(y),
    is_root_(false) {
      rgn_.CreateRectRgn(0, 0, 0, 0);
      zoom_ = 1.0;
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
    store();
    std::vector<Item*>::iterator it = items_.begin();
    for ( ; it != items_.end(); ++it ) {
      Item* item = *it;
      if (item->managed())
        delete item;      
    }
    items_.clear();    
    flush();
  }

  void Group::SetXY(double x, double y) {
    store();
    x_ = x;
    y_ = y;    
    flush();
  }

  void Group::Draw(CDC* cr, const CRgn& repaint_rgn, Canvas* canvas) {    
    std::vector<Item*>::iterator it = items_.begin();
    for ( ; it != items_.end(); ++it ) {
      Item* item = *it;
      if (!item->visible())
        continue;
      if (canvas && !canvas->HasAutomaticDraw()) {
        XFORM rXform;
        cr->GetWorldTransform(&rXform);
        XFORM rXform_new = rXform;
        rXform_new.eDx = x()*zoom();
        rXform_new.eDy = y()*zoom();
        cr->SetGraphicsMode(GM_ADVANCED);
        cr->SetWorldTransform(&rXform_new);
        item->Draw(cr, repaint_rgn, canvas);
        cr->SetGraphicsMode(GM_ADVANCED);
        cr->SetWorldTransform(&rXform);
      } else {
        CRgn item_rgn;
        item_rgn.CreateRectRgn(0, 0, 0, 0);
        item_rgn.CopyRgn(&item->region());
        if (parent()) {
          item_rgn.OffsetRgn(absx(), absy());
        }        
        CRect rc;
        item_rgn.GetRgnBox(&rc);	  
        int erg = item_rgn.CombineRgn(&item_rgn, &repaint_rgn, RGN_AND);
        if (erg != NULLREGION) {		
          XFORM rXform;
          cr->GetWorldTransform(&rXform);
          XFORM rXform_new = rXform;
          rXform_new.eDx = absx()*zoom();
          rXform_new.eDy = absy()*zoom();
          cr->SetGraphicsMode(GM_ADVANCED);
          cr->SetWorldTransform(&rXform_new);
          item->Draw(cr, repaint_rgn, canvas);
          cr->SetGraphicsMode(GM_ADVANCED);
          cr->SetWorldTransform(&rXform);
        }
        item_rgn.DeleteObject();
      }
    }
  }

  void Group::Add(Item* item) {
    assert(item);
    store();
    item->set_parent(this);
    items_.push_back(item);    
    flush();
  }

  void Group::Insert(iterator it, Item* item) {
    assert(item);
    store();
    item->set_parent(this);
    items_.insert(it, item);    
    flush();
  }

  void Group::Erase(Item* item) {
    assert(item);
    std::vector<Item*>::iterator it = find(items_.begin(), items_.end(), item);
    assert(it != items_.end());
    store();
    items_.erase(it);
    Canvas* c = canvas();
    if (c) {       
      Item* bp = c->button_press_item_;
      while (bp) {       
        if (bp == item) {
          c->button_press_item_ = 0;
          break;
        }
        bp = bp->parent();
      }
      Item* out = c->out_item_;
      while (out) {       
        if (out == item) {
          c->out_item_ = 0;
          break;
        }
        out = out->parent();
      }
    }        
    item->set_parent(0);    
    flush();
  }

  void Group::RaiseToTop(Item* item) {    
    Erase(item);
    Add(item);
  }

  void Group::GetBounds(double& x1, double& y1, double& x2, double& y2) const {
    CRect rect;
    if (update_) region();
    rgn_.GetRgnBox(rect);
    x1 = rect.left;
    y1 = rect.top;
    x2 = rect.right;
    y2 = rect.bottom;
  }

  const CRgn& Group::region() const {
    if (update_) {
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
      double zoom = parent() ? parent()->zoom() : 1.0;
      rgn.OffsetRgn(x_*zoom, y_*zoom);
      rgn_.CopyRgn(&rgn);
      rgn.DeleteObject();
      update_ = false;
    }
    return rgn_;
  }

  double Group::absx() const {
    double offset = x_;
    const Group* g = this;
    while (g->parent()) { 
      g = g->parent();
      offset += g->x();
    }
    return offset;
  }

  double Group::absy() const {
    double offset = y_;
    const Group* g = this;
    while (g->parent()) { 
      g = g->parent();
      offset += g->y();
    }
    return offset;
  }

  void Group::set_zorder(Item* item, int z) {
    assert(item);
    if (z<0 || z>=items_.size()) return;        
    std::vector<Item*>::iterator it = find(items_.begin(), items_.end(), item);
    assert(it != items_.end());
    store();
    items_.erase(it);
    items_.insert(begin()+z, item);    
    flush();
  }

  int Group::zorder(Item* item) const {
    int z = -1;
    for (int k = 0; k < items_.size(); k++) {
      if (items_[k] == item) {
        z = k;
        break;
      }
    }
    return z;
  }

  Item* Group::intersect(double x, double y) {    
    Item* found = 0;
    std::vector<Item*>::const_reverse_iterator rev_it = items_.rbegin();
    for ( ; rev_it != items_.rend(); ++rev_it) {
      Item* item = *rev_it;
      double zoom = parent() ? parent()->zoom() : 1.0;
      item = item->visible() ? item->intersect((x-this->x())*zoom, (y-this->y())*zoom) : 0;
      if (item) {
        found = item;
        return item;
      }  
    }
    return found;
  }

  void Group::intersect(std::vector<Item*>& items, double x1, double y1, double x2, double y2) {
    Item* found = 0;
    int size = items.size();
    std::vector<Item*>::const_reverse_iterator rev_it = items_.rbegin();
    for ( ; rev_it != items_.rend(); ++rev_it) {
      Item* item = *rev_it;
      double zoom = parent() ? parent()->zoom() : 1.0;
      if (item->visible()) {        
        item->intersect(items, (x1-x_)*zoom, (y1-y_)*zoom, (x2-x_), (y2-y_));        
      }
    }
    if (size!=items.size()) {
      items.push_back(this);
    }
  }

  Rect::Rect() {    
    rgn_.CreateRectRgn(0, 0, 0, 0);
    x1_ = y1_ = x2_ = y2_ = bx_ = by_ = skin_ = skin_outline_ = 0;
    r_ = g_ = b_ = r_outline_ = g_outline_= b_outline_  = 0;    
    alpha_ = alpha_outline_ = 1;
  }

  Rect::Rect(Group* parent) : Item(parent) {    
    rgn_.CreateRectRgn(0, 0, 0, 0);
    x1_ = y1_ = x2_ = y2_ = bx_ = by_ = skin_ = skin_outline_ = 0;
    r_ = g_ = b_ = r_outline_ = g_outline_= b_outline_  = 0;    
    alpha_ = alpha_outline_ = 1;
  }

  Rect::Rect(Group* parent, double x1, double y1, double x2, double y2) 
    : Item(parent),
    x1_(x1),
    y1_(y1),
    x2_(x2),
    y2_(y2) {
      alpha_ = alpha_outline_ = 1;
      skin_ = skin_outline_ = 0;
      r_ = g_ = b_ = r_outline_ = g_outline_= b_outline_  = 0;
      bx_ = by_ = 0;      
      rgn_.CreateRectRgn(0, 0, 0, 0);
      SetXY(0, 0);
  }

  void Rect::SetPos(double x1, double y1, double x2, double y2) {
    store();
    x1_ = x1;
    y1_ = y1;
    x2_ = x2;
    y2_ = y2;    
    flush();
  }

  void Rect::SetXY(double x, double y) {
    store();
    x2_ += x-x1_; 
    y2_ += y-y1_;
    x1_ = x;
    y1_ = y;            
    flush();
  }

  void Rect::SetColor(double r, double g, double b, double alpha) {    
    r_ = r;
    g_ = g;
    b_ = b;
    alpha_ = alpha;
    skin_ = 0;
    flush();
  }

  void Rect::SetColor(int skin) {    
    Skin::get_color(skin, r_, g_, b_);    
    alpha_ = 1; // todo
    skin_ = skin;
    flush();
  }

  void Rect::SetOutlineColor(double r, double g, double b, double alpha) {
    r_outline_ = r;
    g_outline_ = g;
    b_outline_ = b;
    alpha_outline_ = alpha;    
    flush();
  }

  void Rect::SetOutlineColor(int skin) {    
    Skin::get_color(skin, r_outline_, g_outline_, b_outline_);    
    alpha_outline_ = 1; // todo
    skin_outline_ = skin;
    flush();
  }

  void Rect::Draw(CDC* devc, const CRgn& repaint_rgn, class Canvas* widget) {
    double zoom = parent()->zoom();
    CRect rect(x1_*zoom, y1_*zoom, x2_*zoom, y2_*zoom);      
    CPen pen;
    pen.CreatePen(PS_SOLID, 1, RGB(r_outline_, g_outline_, b_outline_));
    CBrush brush(RGB(r_, g_, b_));    
    CBrush* pOldBrush = devc->SelectObject(&brush);
    CPen* pOldPen = devc->SelectObject(&pen);    
    if (alpha_ == 1) {
      if (bx_!=0 || by_!=0) {
        CPoint pt(bx_*zoom, by_*zoom);
        devc->RoundRect(rect, pt);
      } else {
        devc->Rectangle(rect);
      }
    } else {
      this->paintRect(*devc, rect, RGB(r_outline_, g_outline_, b_outline_)
        , RGB(r_, g_, b_), (alpha_*127));
    }
    devc->SelectObject(pOldPen);
    devc->SelectObject(pOldBrush);
  }

  Item* Rect::intersect(double x, double y) {
    if (update_) { region(); }
    double zoom = parent() ? parent()->zoom() : 1.0;
    CPoint pt(x*zoom, y*zoom);
    return rgn_.PtInRegion(pt) ? this : 0;    
  }

  void Rect::intersect(std::vector<Item*>& items, double x1, double y1, double x2, double y2) {
    if (update_) { region(); }
    double zoom = parent() ? parent()->zoom() : 1.0;
    CRect rect(x1*zoom, y1*zoom, x2*zoom, y2*zoom);
    if (rgn_.RectInRegion(rect)) {
      items.push_back(this); 
    }
  }

  void Rect::GetBounds(double& x1, double& y1, double& x2, double& y2) const {
    CRect rect;
    rgn_.GetRgnBox(rect);
    x1 = rect.left;
    y1 = rect.top;
    x2 = rect.right;
    y2 = rect.bottom;
    swap_smallest(x1, x2);
    swap_smallest(y1, y2);
  }

  bool Rect::paintRect(CDC &hdc, RECT dim, COLORREF penCol, COLORREF brushCol, unsigned int opacity) {
    XFORM rXform;
    hdc.GetWorldTransform(&rXform);
    XFORM rXform_new = rXform;
    rXform_new.eDx = absx();
    rXform_new.eDy = absy();
    hdc.SetGraphicsMode(GM_ADVANCED);
    hdc.SetWorldTransform(&rXform_new);                
    dim.right -= dim.left;
    dim.bottom -= dim.top;
    dim.left = dim.top = 0;

    HDC tempHdc         = CreateCompatibleDC(hdc);
    BLENDFUNCTION blend = {AC_SRC_OVER, 0, opacity, 0};

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

    SetDCPenColor(tempHdc, penCol);
    SetDCBrushColor(tempHdc, brushCol);
    HBRUSH br = CreateSolidBrush(brushCol);
    FillRect(tempHdc, &dim, br);
    AlphaBlend(hdc, dim.left, dim.top, dim.right, dim.bottom, tempHdc, dim.left, dim.top, dim.right, dim.bottom, blend);
    DeleteObject(hbitmap);
    DeleteObject(br);
    DeleteObject(tempHdc);
    hdc.SetGraphicsMode(GM_ADVANCED);
    hdc.SetWorldTransform(&rXform);
    return 0;
  }

  const CRgn& Rect::region() const {
    if (update_) {
      rgn_.DeleteObject();
      double zoom = parent() ? parent()->zoom() : 1.0;
      rgn_.CreateRectRgn(x1_*zoom, y1_*zoom, (x2_+1)*zoom, (y2_+1)*zoom);
      update_ = false;
    }
    return rgn_;
  }

  Line::Line() : alpha_(1) {
    r_ = g_ = b_ = 0;    
    rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  Line::Line(Group* parent) : Item(parent), alpha_(1) {
    r_ = g_ = b_ = 0;    
    rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  void Line::Draw(CDC* cr,
    const CRgn& repaint_region,
  class Canvas* widget) {    
    cr->SelectObject(GetStockObject(DC_PEN));
    cr->SetDCPenColor(RGB(r_, g_, b_));
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
    double  hypetenuse   = sqrt(ankathede*ankathede + gegenkathede*gegenkathede);

    if (hypetenuse == 0)
      return 0;

    double cos = ankathede    / hypetenuse;
    double sin = gegenkathede / hypetenuse;

    int dx = static_cast<int> (-sin*distance_);
    int dy = static_cast<int> (-cos*distance_);

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

  void Line::SetPoints(const Points& pts) {
    store();    
    pts_ = pts;    
    flush();
  }

  void Line::SetXY(double x, double y) {     
    store();
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
    flush();
  }

  void Line::GetBounds(double& x1, double& y1, double& x2, double& y2) const {
    x1 = y1 = x2 = y2 = 0;    
    if (pts_.size() > 1) {
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

  const CRgn& Line::region() const {
    if (update_) {       
      double x1, y1, x2, y2;
      double dist = 5;
      GetBounds(x1, y1, x2, y2);
      rgn_.DeleteObject();
      double zoom = parent() ? parent()->zoom() : 1.0;
      rgn_.CreateRectRgn((x1-dist)*zoom, (y1-dist)*zoom, (x2+2*dist+1)*zoom, (y2+2*dist+1)*zoom);
      update_ = false;
    }
    return rgn_;
  }

  Text::Text() : alpha_(1) {
    x_ = y_ = r_ = g_ = b_ = skin_ = 0;
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    lfLogFont.lfHeight = 12;
    strcpy(lfLogFont.lfFaceName, "Arial");
    font_.CreateFontIndirect(&lfLogFont);
    rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  Text::Text(Group* parent) : Item(parent), alpha_(1) {
    x_ = y_ = r_ = g_ = b_ = skin_ = 0;
    set_name("text");
    LOGFONT lfLogFont;
    memset(&lfLogFont, 0, sizeof(lfLogFont));
    int h = parent ? 12*parent->zoom() : 12;
    lfLogFont.lfHeight = h;
    strcpy(lfLogFont.lfFaceName, "Arial");
    font_.CreateFontIndirect(&lfLogFont);
    rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  Text::Text(Group* parent, const std::string& text)
    : Item(parent),
    text_(text),
    alpha_(1) {
      x_ = y_ = r_ = g_ = b_ = skin_ = 0;
      LOGFONT lfLogFont;
      memset(&lfLogFont, 0, sizeof(lfLogFont));
      lfLogFont.lfHeight = 12*parent->zoom();
      strcpy(lfLogFont.lfFaceName, "Arial");
      font_.CreateFontIndirect(&lfLogFont);
      rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  void Text::SetColor(double r, double g, double b, double alpha) {
    r_ = r;
    g_ = g;
    b_ = b;
    alpha_ = alpha;
    flush();
  }

  void Text::SetColor(int skin) {
    Skin::get_color(skin, r_, g_, b_);    
    alpha_ = 1; // todo
    skin_ = skin;
    flush();
  }

  void Text::SetXY(double x, double y) {    
    store();
    x_ = x;
    y_ = y;    
    flush();
  }

  void Text::SetText(const std::string& text) {    
    store();
    text_ = text;    
    flush();
  }

  Item* Text::intersect(double x, double y) {
    if (update_) { region(); }
    CPoint pt(x, y);
    return rgn_.PtInRegion(pt) ? this : 0;    
  }

  void Text::intersect(std::vector<Item*>& items, double x1, double y1, double x2, double y2) {
    if (update_) { region(); }
    double zoom = parent() ? parent()->zoom() : 1.0;
    CRect rect(x1*zoom, y1*zoom, x2*zoom, y2*zoom);
    if (rgn_.RectInRegion(rect)) {
      items.push_back(this); 
    }
  }  

  void Text::GetBounds(double& x1, double& y1, double& x2, double& y2) const {
    x1 = x_;
    y1 = y_;
    x2 = text_w + x1;
    y2 = text_h + y1;
  }

  const CRgn& Text::region() const {
    if (update_) {
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
        double zoom = parent() ? parent()->zoom() : 1.0;
        rgn_.CreateRectRgn(x_*zoom, y_*zoom, x_*zoom + text_w +1, y_*zoom + text_h+1);
        update_ = false;
      }
    }
    return rgn_;
  }

  void Text::Draw(CDC* devc,                      
    const CRgn& repaint_region,
    Canvas* widget) {
      double zoom = parent()->zoom();
      CFont* oldFont= devc->SelectObject(&font_);
      devc->SetBkMode(TRANSPARENT);
      COLORREF cref = RGB(r_, g_, b_); 
      devc->SetTextColor(cref);
      devc->TextOut(x_*zoom, y_*zoom, text_.c_str());
      devc->SetBkMode(OPAQUE);
      devc->SelectObject(oldFont);
  }

  // PixBuf
  PixBuf::PixBuf() : transparent_(false), shared_(false), pmdone(false) {
    image_ = mask_ = 0;
    x_ = y_ = width_ = height_ = xsrc_ = ysrc_ = skin_ = 0;
    rgn_.CreateRectRgn(0, 0, 0, 0);
    SetXY(0, 0);
  }

  PixBuf::PixBuf(Group* parent) : Item(parent), transparent_(false), shared_(false), pmdone(false) {
    image_ = mask_ = 0;
    x_ = y_ = width_ = height_ = xsrc_ = ysrc_ = skin_ = 0;
    rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  PixBuf::PixBuf(Group* parent, double x, double y, CBitmap* image)
    : Item(parent),
    image_(image),
    mask_(0),
    x_(x),
    y_(y),   
    transparent_(false), pmdone(false),
    shared_(true) {
      width_ = height_ = xsrc_ = ysrc_ = skin_ = 0;
      BITMAP bm;
      image->GetBitmap(&bm);
      width_ = bm.bmWidth;
      height_ = bm.bmHeight;  
      rgn_.CreateRectRgn(0, 0, 0, 0);      
      region();
  }

  PixBuf::~PixBuf() {
    if (!shared_ && image_) {
      delete image_;
    }
  }

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

  inline void PremultiplyBitmapAlpha(HDC hDC, HBITMAP hBmp)
  {
    BITMAP bm = { 0 };
    GetObject(hBmp, sizeof(bm), &bm);
    BITMAPINFO* bmi = (BITMAPINFO*) _alloca(sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
    ::ZeroMemory(bmi, sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD)));
    bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    BOOL bRes = ::GetDIBits(hDC, hBmp, 0, bm.bmHeight, NULL, bmi, DIB_RGB_COLORS);
    if( !bRes || bmi->bmiHeader.biBitCount != 32 ) return;
    LPBYTE pBitData = (LPBYTE) ::LocalAlloc(LPTR, bm.bmWidth * bm.bmHeight * sizeof(DWORD));
    if( pBitData == NULL ) return;
    LPBYTE pData = pBitData;
    ::GetDIBits(hDC, hBmp, 0, bm.bmHeight, pData, bmi, DIB_RGB_COLORS);
    for( int y = 0; y < bm.bmHeight; y++ ) {
      for( int x = 0; x < bm.bmWidth; x++ ) {
        pData[0] = (BYTE)((DWORD)pData[0] * pData[3] / 255);
        pData[1] = (BYTE)((DWORD)pData[1] * pData[3] / 255);
        pData[2] = (BYTE)((DWORD)pData[2] * pData[3] / 255);
        pData += 4;
      }
    }
    ::SetDIBits(hDC, hBmp, 0, bm.bmHeight, pBitData, bmi, DIB_RGB_COLORS);
    ::LocalFree(pBitData);
  }

  void PixBuf::Draw(CDC* devc,
    const CRgn& repaint_region,
  class Canvas* widget) {
    CDC memDC;
    CBitmap* oldbmp;
    memDC.CreateCompatibleDC(devc);
    oldbmp=memDC.SelectObject(image_);
    double zoom = parent()->zoom();
    if (!transparent_) {
      if (zoom != 1.0) {
        devc->StretchBlt(x()*zoom, y()*zoom, width_*zoom, height_*zoom, &memDC, xsrc_, ysrc_, width_, height_, SRCCOPY);      
      } else {
        devc->BitBlt(x(), y(), width_, height_, &memDC, xsrc_, ysrc_, SRCCOPY);      
      }
    } else {     
      //PremultiplyBitmapAlpha(dc, image_);
      //BLENDFUNCTION blend = {AC_SRC_OVER, 0, 60, 0};      
      TransparentBlt(devc, x()*zoom,  y()*zoom, width_, height_, &memDC, mask_, xsrc_, ysrc_);
      //AlphaBlend(*devc, x(),  y(), width_, height_, memDC, xsrc_, ysrc_, x()+width_, y()+height_,blend);      
     /* if (!pmdone) {
        //PremultiplyBitmapAlpha(*devc, *image_);
        pmdone = true;
      }
      BLENDFUNCTION bf;
      bf.BlendOp = AC_SRC_OVER;
      bf.BlendFlags = 0;
      bf.SourceConstantAlpha = 255;
      bf.AlphaFormat = AC_SRC_ALPHA;
      AlphaBlend(*devc, x(),  y(), width_, height_, memDC, xsrc_, ysrc_, width_, height_,bf);*/
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
    store();
    x_ = x;
    y_ = y;   
    flush();
  }

  void PixBuf::SetSize(int width, int height) {
    store();
    width_ = width;
    height_ = height;    
    flush();
  }

  void PixBuf::SetSource(int xsrc, int ysrc) {    
    xsrc_ = xsrc;
    ysrc_ = ysrc;
    flush();
  }

  void PixBuf::SetImage(CBitmap* image) {
    store();
    image_ = image;
    BITMAP bm;
    image_->GetBitmap(&bm);    
    width_ = bm.bmWidth;
    height_ = bm.bmHeight;
    flush();
  }

  void PixBuf::SetImage(int skin) {
    store();
    shared_ = true;
    image_ = Skin::get_bmp(skin);    
    BITMAP bm;
    image_->GetBitmap(&bm);    
    width_ = bm.bmWidth;
    height_ = bm.bmHeight;
    flush();
  }

  void PixBuf::Load(const std::string& filename) {
    std::string tmp = filename;
    std::size_t dotpos = filename.find_last_of(".");
    if (dotpos == std::string::npos) dotpos = 0;
    std::string extension = filename.substr(dotpos, 4);
    std::string extlower = extension;
    std::transform(extlower.begin(), extlower.end(), extlower.begin(), std::tolower);
    if (extlower == "png") {
      CPngImage image;
      image.Load(_T(filename.c_str()));
      image_ = new CBitmap();
      image_->Attach(image.Detach());      
    } else {
      CImage image;
      image.Load(_T(filename.c_str()));
      image_ = new CBitmap();
      image_->Attach(image.Detach());
    }        
    shared_ = false;            
    BITMAP bm;
    image_->GetBitmap(&bm);    
    width_ = bm.bmWidth;
    height_ = bm.bmHeight;
  }

  void PixBuf::SetMask(CBitmap* mask) {
    mask_ = mask;
  }

  const CRgn& PixBuf::region() const {
    if (update_) {
      rgn_.DeleteObject();
      double zoom = parent() ? parent()->zoom() : 1.0;
      rgn_.CreateRectRgn(x_*zoom, y_*zoom, (x_ + width_)*zoom, (y_ + height_)*zoom);
      update_ = false;
    }
    return rgn_;
  }

  Item* PixBuf::intersect(double x, double y) {
    if (update_) { region(); }
    double zoom = parent() ? parent()->zoom() : 1.0;
    CPoint pt(x*zoom, y*zoom);
    return rgn_.PtInRegion(pt) ? this : 0;
  }

  void PixBuf::intersect(std::vector<Item*>& items, double x1, double y1, double x2, double y2) {
    if (update_) { region(); }
    double zoom = parent() ? parent()->zoom() : 1.0;
    CRect rect(x1*zoom, y1*zoom, x2*zoom, y2*zoom);
    if (rgn_.RectInRegion(rect)) {
      items.push_back(this); 
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

  // Canvas
  Canvas::Canvas() :
  parent_(0),
    steal_focus_(0),
    bg_image_(0) {
      save_ = has_draw_ = true;
      bg_width_ = bg_height_ = 0;    
      button_press_item_ = out_item_ = 0;
      r_ = g_ = b_ = alpha_ = skin_= 0;
      cw_ = pw_ = ch_ = ph_ = 300;      
      root_ = new Group(this);
      root_->set_manage(true);
      save_rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  Canvas::Canvas(CWnd* parent) :
  parent_(parent),    
    steal_focus_(0),
    bg_image_(0) {
      save_ = has_draw_ = false;
      bg_width_ = bg_height_ = 0;
      button_press_item_ = out_item_ = 0;
      r_ = g_ = b_ = alpha_ = skin_= 0;
      cw_ = pw_ = ch_ = ph_ = 200;      
      root_ = new Group(this);
      root_->is_root_ = true;
      root_->set_manage(true);
      save_rgn_.CreateRectRgn(0, 0, 300, 200);
  }

  Canvas::~Canvas() {
    if (root_->managed()) {
      delete root_;
    } 
    save_rgn_.DeleteObject();
  }

  void Canvas::DrawFlush(CDC *devc, const CRgn& rgn) {
    save_rgn_.CombineRgn(&save_rgn_,&rgn,RGN_OR);
    Draw(devc, save_rgn_);
    save_rgn_.DeleteObject();
    save_rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  void Canvas::Draw(CDC *devc, const CRgn& rgn) {
    if (bg_image_)	{
      CDC memDC;      
      memDC.CreateCompatibleDC(devc);
      CBitmap* oldbmp = memDC.SelectObject(bg_image_);
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
      devc->FillSolidRect(&rect, RGB(r_, g_, b_));
    }
    root_->Draw(devc, rgn, this);
  }

  void Canvas::StealFocus(Item* item) {
    button_press_item_ = item;
    steal_focus_ = true;
  }

  Item* Canvas::OnEvent(Event* ev) {
    Item* item = button_press_item_;
    if (ev->type() == Event::BUTTON_PRESS || ev->type() == Event::BUTTON_2PRESS) {
      item = button_press_item_ = root_->intersect(ev->x(), ev->y());
    } else
      if (ev->type() == Event::MOTION_NOTIFY ) {        
        item = button_press_item_ ? button_press_item_ : root_->intersect(ev->x(), ev->y());
        // check mouseout
        if (!button_press_item_ && out_item_ && item!=out_item_) {
          Group* parent = out_item_->parent();
          Event event(out_item_, Event::MOTION_OUT,
            ev->x() - (parent ? parent->absx() : 0),
            ev->y() - (parent ? parent->absy() : 0),
            ev->button(),
            ev->shift());                        
          DelegateEvent(&event, out_item_);
        }
        out_item_ = item;
      }     
      if (item) {
        Group* parent = item->parent();
        const double x = ev->x(); //  - (parent ? parent->absx() : 0);
        const double y = ev->y(); // - (parent ? parent->absy() : 0);
        Event e(item, ev->type(), x, y, ev->button(), ev->shift());
        OutputDebugString("mousemove-2");
        DelegateEvent(&e, item);
        item = e.item();
      }
      if (ev->type() == Event::BUTTON_RELEASE || ev->type() == Event::BUTTON_2PRESS) {
        button_press_item_ = 0;       
      }
      return item;
  }  

  bool Canvas::DelegateEvent(Event* ev, Item* item) {
    bool erg = item->pointerevents() && item->OnEvent(ev);
    while (!erg && item->parent()) {
      // redirect event to parent      
      item = item->parent();
      if (button_press_item_) {
        button_press_item_ = item;
      }      
      ev->setitem(item);      
      erg = item->pointerevents() && item->OnEvent(ev);      
      if (steal_focus_) { 
        steal_focus_ = 0;
        return erg;
      }       
    }
    return erg;
  }

  void Canvas::Invalidate(CRgn& rgn) {
    if (IsSaving()) {
      save_rgn_.CombineRgn(&save_rgn_,&rgn,RGN_OR);
    } else
      if (parent_) {
        parent_->InvalidateRgn(&rgn, 0);
      }
  }

  void Canvas::Flush() {
    if (parent_) {
      parent_->InvalidateRgn(&save_rgn_,0);
    }
    save_rgn_.DeleteObject();
    save_rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  void Canvas::SetColor(double r, double g, double b, double alpha) {    
    r_ = r;
    g_ = g;
    b_ = b;
    alpha_ = alpha;
    skin_ = 0;    
  }

  void Canvas::SetColor(int skin) {    
    Skin::get_color(skin, r_, g_, b_);    
    alpha_ = 1; // todo
    skin_ = skin;    
  }

}}}
