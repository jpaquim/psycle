// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#include <psycle/host/detail/project.private.hpp>
#include "Canvas.hpp"

namespace psycle { namespace host { namespace canvas {

Item::Item() : parent_(0), managed_(0), visible_(1)  { }

  Item::~Item() {
    Group* group = parent();
    while ( group && group->parent() )
      group = group->parent();
    if (group && group->widget() ) 
	  if ( group->widget()->button_press_item_ == this )
	    group->widget()->button_press_item_ = 0;
	if ( parent_ && !managed_ ) {
      parent_->Erase(this);
    }
  }

  Item::Item(Group* parent) : parent_(parent), managed_(0), visible_(1) {
	assert(parent);
    parent->Add(this);
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

  void Item::SetVisible(bool on) {
	   visible_ = on;
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

  // todo
  /*bool Item::on_drag_data_received(
       const Glib::RefPtr<Gdk::DragContext>& context,
       int,
       int,
       const Gtk::SelectionData& selection_data,
       guint,
       guint time) {
    return 0;
  }*/

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
  }
  
  Group::~Group() {
    std::vector<Item*>::iterator it = items_.begin();
    for ( ; it != items_.end(); ++it ) {
      Item* item = *it;
	  if ( item->managed() )
		delete item;
    }
  }

  void Group::Clear()
  {
	std::vector<Item*>::iterator it = items_.begin();
    for ( ; it != items_.end(); ++it ) {
      Item* item = *it;
	  if ( item->managed() )
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
	}
  }

  void Group::SetXY(double x, double y) {
    double delta_x = x - x_;
    double delta_y = y - y_;
    CRgn old_rgn;
	old_rgn.CreateRectRgn(0,0,0,0);
	old_rgn.CopyRgn(&region());
    x_ = x;
    y_ = y;
    CRgn rgn;
	rgn.CreateRectRgn(0,0,0,0);
	rgn.CopyRgn(&old_rgn);
	rgn.OffsetRgn(delta_x, delta_y);
	rgn.CombineRgn(&rgn,&old_rgn, RGN_OR);
	rgn.OffsetRgn(parent()->absx(), parent()->absy());
    InvalidateRegion(&rgn);
  }

  void Group::Move(double delta_x, double delta_y) {
	CRgn old_rgn;
	old_rgn.CreateRectRgn(0,0,0,0);
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
			rXform_new.eDx = x();
			rXform_new.eDy = y();
			cr->SetGraphicsMode(GM_ADVANCED);
			cr->SetWorldTransform(&rXform_new);
			item->Draw(cr, repaint_region, canvas);
			cr->SetGraphicsMode(GM_ADVANCED);
			cr->SetWorldTransform(&rXform);
		}
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
    items_.erase(it);
  }

  void Group::RaiseToTop(Item* item) {
    assert(item);
    Erase(item);
    Add(item);
  }

  void Group::GetBounds(double& x1, double& y1, double& x2,
                        double& y2) const {
    bool first = false;
    x1 = 0;
    x2 = 0;
    y1 = 0;
    y2 = 0;
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
    for ( ; it != items_.end(); ++it ) {
      Item* item = *it;
	  if (item->visible()) {
	    int nCombineResult = rgn.CombineRgn(&rgn, &item->region(), RGN_OR);
	    if ( nCombineResult == NULLREGION ) {
	      rgn.DeleteObject();
		  rgn.CreateRectRgn(0, 0, 0, 0);
	    }
	  }
	}
    rgn.OffsetRgn(x_, y_);
	rgn_.CopyRgn(&rgn);
    return rgn_;
  }

  double Group::absx() const {
    int offset = x_;
    const Group* group = this;
    while ( group->parent() ) { 
      group = group->parent();
      offset += group->x();
    }
    return offset;
  }

  double Group::absy() const {
    int offset = y_;
    const Group* group = this;
    while ( group->parent() ) { 
      group = group->parent();
      offset += group->y();
    }
    return offset;
  }

  Item* Group::intersect(double x, double y) {
    Item* found = 0;
    std::vector<Item*>::const_reverse_iterator rev_it = items_.rbegin();
    for ( ; rev_it != items_.rend(); ++rev_it ) {
      Item* item = *rev_it;
      item = item->intersect(x-this->x(),y-this->y());
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
	rgn_.CreateRectRgn(0,0,0,0);
  }

  void Rect::SetPos(double x1, double y1, double x2, double y2) {
    if (update_) {
	  rgn_.DeleteObject();
      CreateRectRgn(x1_, y1_, x2_, y2_);
    }
    x1_ = x1;
    y1_ = y1;
    x2_ = x2; 
    y2_ = y2;
    CRgn new_rgn;
	new_rgn.CreateRectRgn(x1_, y1_, x2_, y2_);
	rgn_.CombineRgn(&rgn_, &new_rgn, RGN_OR);
    InvalidateRegion(&rgn_);
	rgn_.DeleteObject();
	rgn_.CopyRgn(&new_rgn);
    update_ = false;
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
   COLORREF cref = RGB(r_ * 255, g_* 255, b_ * 255); 
   CRect rect(x1_, y1_, x2_, y2_);
   devc->FillSolidRect(&rect, cref);
   /* cr->set_source_rgba(r_, g_, b_, alpha_);
    cr->rectangle(x1_, y1_, x2_ - x1_, y2_ -y1_);
    cr->fill();
    cr->set_source_rgba(r_outline_, g_outline_, b_outline_, alpha_outline_);
    cr->rectangle(x1_, y1_, x2_ - x1_, y2_ -y1_);
    cr->stroke();*/
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

  const CRgn& Rect::region() const {
    if (update_) {
	  rgn_.DeleteObject();
      rgn_.CreateRectRgn(x1_, y1_, x2_, y2_);
      update_ = false;
    }
    return rgn_;
  }

  Line::Line()
      : r_(0),
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
    Points::iterator it = pts_.begin();
    bool first = true;
    for ( ; it != pts_.end(); ++it ) {
      const std::pair<double, double>& pt = (*it);
      if ( first) {
        COLORREF cref = RGB(r_ * 255, g_* 255, b_ * 255); 
		cr->SetDCPenColor(cref);
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
	int err = rgn.CreatePolygonRgn(&pts[0],pts.size(), WINDING);
    return rgn.PtInRegion(x,y) ? this : 0;
  }

  void Line::SetPoints( const Points& pts ) {
    Group* group = parent();
	Canvas* canvas = 0;
    while ( group && group->parent() )
      group = group->parent();
    if (group && group->widget() ) 
		canvas = group->widget();
    double d = 2;
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
	}
  }

  void Line::SetColor(double r, double g, double b, double alpha) {
    r_ = r;
    g_ = g;
    b_ = b;
    alpha_ = alpha;
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

  void Line::SetVisible(bool on)
  {
	  Item::SetVisible(on);
	  update_ = 1;
  }

  const CRgn& Line::region() const {
    if ( update_ ) {       
      double x1, y1, x2, y2;
      double dist = 5;
      GetBounds(x1, y1, x2, y2);
	  rgn_.DeleteObject();
	  rgn_.CreateRectRgn(x1-dist, y1-dist, x2+2*dist, y2+2*dist);
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

  void Text::UpdateValues() const {
   const Group* group = this->parent();
    while ( group && group->parent() )
      group = group->parent();
    if (group->widget() ) {
	  HDC dc = GetDC(0);
	  SIZE extents = {0};
	  HFONT old_font =
      reinterpret_cast<HFONT>(SelectObject(dc, font_));
      GetTextExtentPoint32(dc, text_.c_str(), text_.length(),
						   &extents); 
	  SelectObject(dc, old_font);
	  ReleaseDC(0,dc);
	  text_w = extents.cx;
	  text_h = extents.cy;
	  rgn_.DeleteObject();
	  rgn_.CreateRectRgn(x_, y_, x_ + text_w, y_ + text_h);
      update_ = false;
    }
  }

  void Text::GetBounds(double& x1, double& y1, double& x2, double& y2) const {
    x1 = x_;
    y1 = y_;
    if ( update_ )
     UpdateValues();
    x2 = text_w + x1;
    y2 = text_h + y1;
  }

  const CRgn& Text::region() const {
    if ( update_ )
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
		transparent_(false)
  {
	  rgn_.CreateRectRgn(0, 0, 0, 0);
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
		transparent_(false)
  {
	  rgn_.CreateRectRgn(0, 0, 0, 0);
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
		transparent_(false)
  {
	CSize size = image_->GetBitmapDimension();
	width_ = size.cx;
	height_ = size.cy;
	rgn_.CreateRectRgn(0, 0, 0, 0);
  }

  PixBuf::~PixBuf()
  {
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

  void PixBuf::Draw(CDC* devc,
                  const CRgn& repaint_region,
                  class Canvas* widget) {
	CDC memDC;
	CBitmap* oldbmp;
	memDC.CreateCompatibleDC(devc);
	oldbmp=memDC.SelectObject(image_);	
	if (!transparent_) 
		devc->BitBlt(x(), y(), width_, height_, &memDC, xsrc_, ysrc_, SRCCOPY);
	else
		TransparentBlt(devc, x(),  y(), width_, height_, &memDC, mask_, xsrc_, ysrc_);
	memDC.SelectObject(oldbmp);
	memDC.DeleteDC(); 
  }
  
  void PixBuf::GetBounds(double& x1, double& y1, double& x2, double& y2) const {
    x1 = x_;
    y1 = y_;
	x2 = x_ + width_;
	y2 = y_ + height_;    
  }

  void PixBuf::SetXY(double x, double y)
  {
	  x_ = x;
	  y_ = y;
  }

  void PixBuf::SetSize(int width, int height)
  {
	  width_ = width;
	  height_ = height;
  }

  void PixBuf::SetSource(int xsrc, int ysrc)
  {
	 xsrc_ = xsrc;
	 ysrc_ = ysrc;
  }

  void PixBuf::SetImage(CBitmap* image)
  {
	image_ = image;
	CSize size = image_->GetBitmapDimension();
	width_ = size.cx;
	height_ = size.cy;
  }

  void PixBuf::SetMask(CBitmap* mask)
  {
	  mask_ = mask;
  }

  const CRgn& PixBuf::region() const {
    rgn_.DeleteObject();
    rgn_.CreateRectRgn(x_, y_, x_ + width_, y_ + height_);
    return rgn_;
  }

  void PixBuf::SetTransparent(bool on)
  {
	  transparent_ = on;
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
	cw_(300),
	ch_(200)
{
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
	ch_(200)
{
	save_rgn_.CreateRectRgn(0, 0, 0, 0);
}

void Canvas::OnSize(int cx, int cy) 
{	
	cw_ = cx;
	ch_ = cy;
}

void Canvas::Draw(CDC *devc, const CRgn& rgn)
{
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

void Canvas::OnEvent(Event* ev) {
	if ( ev->type == Event::BUTTON_PRESS ) {
		button_press_item_ = root_.intersect(ev->x, ev->y);
		if (button_press_item_) {
			Event event;
			event.type = ev->type;
			event.button = ev->button;
			event.shift = ev->shift;
		    Group* parent = button_press_item_->parent();
			event.x = ev->x - (parent ? parent->absx() : 0);
			event.y = ev->y - (parent ? parent->absy() : 0);
			DelegateEvent(&event, button_press_item_);
		}
	} else
		if ( ev->type == Event::BUTTON_RELEASE ) {
		if (button_press_item_) {
			Event event;
			event.type = ev->type;
			event.button = ev->button;
		    Group* parent = button_press_item_->parent();
			event.x = ev->x - (parent ? parent->absx() : 0);
			event.y = ev->y - (parent ? parent->absy() : 0);
			event.shift = ev->shift;
			DelegateEvent(&event, button_press_item_);
		}
		button_press_item_ = 0;
	} else
		if ( ev->type == Event::MOTION_NOTIFY ) {
		Item* item = button_press_item_ ? button_press_item_ : root_.intersect(ev->x, ev->y);
		if (item) {
			Event event;
			event.type = ev->type;
			event.shift = ev->shift;
			event.button = ev->button;
		    Group* parent = item->parent();
			event.x = ev->x - (parent ? parent->absx() : 0);
			event.y = ev->y - (parent ? parent->absy() : 0);
			DelegateEvent(&event, item);
		}
	}	else
	if ( ev->type == Event::BUTTON_2PRESS ) {
		Item* item = button_press_item_ ? button_press_item_ : root_.intersect(ev->x, ev->y);
		if (item) {
			Event event;
			event.type = ev->type;
			event.shift = ev->shift;
			event.button = ev->button;
		    Group* parent = item->parent();
			event.x = ev->x - (parent ? parent->absx() : 0);
			event.y = ev->y - (parent ? parent->absy() : 0);
			DelegateEvent(&event, item);
		}
	}
}

bool Canvas::DelegateEvent(Event* ev, Item* item) {
    bool erg = item->OnEvent(ev);
    while ( !erg && item->parent() ) {
      // redirect event to parent
      item = item->parent();
      erg = item->OnEvent(ev);
      if ( steal_focus_ ) { 
        steal_focus_ = 0;
        return erg;
      }       
    }
    return erg;
}

void Canvas::Flush()
{
	if (parent_)
		parent_->InvalidateRgn(&save_rgn_,0);
	save_rgn_.DeleteObject();
	save_rgn_.CreateRectRgn(0, 0, 0, 0);
}

}}}
