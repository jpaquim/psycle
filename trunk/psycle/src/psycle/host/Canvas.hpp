// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"
#include "PsycleConfig.hpp"

#undef GetGValue
#define GetGValue(rgb) (LOBYTE((rgb) >> 8))

namespace psycle { namespace host { namespace canvas {

  typedef int32_t ARGB;
  #define ToCOLORREF(argb) RGB((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, (argb >> 0) & 0xFF)
  #define GetAlpha(argb) (argb>>24) & 0xFF

  class Item;  class Canvas;
  struct Event {
    enum Type {
      BUTTON_PRESS,
      BUTTON_2PRESS,
      BUTTON_RELEASE,
      MOTION_NOTIFY,
      MOTION_OUT,
      KEY_DOWN,
      KEY_UP
    };		
    Event(Item* item, Type type, double x, double y, int button, unsigned int shift) 
      : item_(item), type_(type), x_(x), y_(y), button_(button), shift_(shift) {}
        
    Type type() const { return type_; }
    double x() const { return x_; }
    double y() const { return y_; }
    unsigned int button() const { return button_; }
    unsigned int shift() const { return shift_; }
    void setxy(double x, double y) { x_ = x; y_ = y; }
    void setitem(Item* item) { item_ = item; }
    Item* item() const { return item_; }           

  private:
    Type type_;
    double x_, y_;
    unsigned int button_, shift_;
    Item* item_;            
  };

  class Skin {
  public:
    enum COLOR {
      TOPCOLOR = 1,
      BOTTOMCOLOR,
      HTOPCOLOR,
      HBOTTOMCOLOR,
      FTOPCOLOR,
      FBOTTOMCOLOR,
      HFTOPCOLOR,
      HFBOTTOMCOLOR,
      TITLECOLOR,
    };

    enum SIZE {
      DIALSIZE = 1
    };

    enum BMP {
      DIALBMP = 1
    };

    static CBitmap* get_bmp(int key) {
      CBitmap* bmp = 0;
      PsycleConfig* cfg = &PsycleGlobal::conf();            
      switch (key) {
         case BMP::DIALBMP:
           bmp = &cfg-> macParam().dial;
        break;
      default:;
      }
      return bmp;
    }

    static void get_size(int key, double &x, double &y) {
      PsycleConfig* cfg = &PsycleGlobal::conf();            
      switch (key) {
        case SIZE::DIALSIZE: 
          x = cfg->macParam().dialwidth;
          y = cfg->macParam().dialheight;
        break;
      default:;
      }
    }

    static ARGB get_color(int key) {
      PsycleConfig* cfg = &PsycleGlobal::conf();      
      int cr = 0;
      switch (key) {
        case COLOR::TOPCOLOR:      cr = cfg->macParam().topColor; break;
        case COLOR::BOTTOMCOLOR:   cr = cfg->macParam().bottomColor; break;
        case COLOR::HTOPCOLOR:     cr = cfg->macParam().hTopColor; break;
        case COLOR::HBOTTOMCOLOR:  cr = cfg->macParam().hBottomColor; break;
        case COLOR::FTOPCOLOR:     cr = cfg->macParam().fontTopColor; break;
        case COLOR::FBOTTOMCOLOR:  cr = cfg->macParam().fontBottomColor; break;
        case COLOR::HFTOPCOLOR:    cr = cfg->macParam().fonthTopColor; break;
        case COLOR::HFBOTTOMCOLOR: cr = cfg->macParam().fonthBottomColor; break;
        case COLOR::TITLECOLOR:    cr = cfg->macParam().titleColor; break;                
        default:;
      }           
      int a = 0;
      return (((ARGB) (GetBValue(cr)) << 0)  |
              ((ARGB) (GetGValue(cr)) << 8)  |
              ((ARGB) (GetRValue(cr)) << 16) |
              ((ARGB) (a) << 24));
    }    
  };

  class Item {
  public:
    Item();
    Item(class Group* parent);
    void Init();
    virtual ~Item();

    typedef std::vector<Item*>::iterator iterator;
    virtual iterator begin() { return dummy.begin(); }
    virtual iterator end() { return dummy.end(); }
    virtual bool empty() const { return true; }
    Canvas* canvas();
    virtual Canvas* widget() const { return 0; }
    void set_name(const std::string& name) { name_ = name; }
    const std::string& name() const { return name_; }
    void set_parent(class Group* parent) { parent_ = parent; }
    Group* parent() { return parent_; }
    const Group* parent() const { return parent_; }
    virtual const CRgn& region() const { return rgn_; }
    virtual void Draw(CDC* cr, const CRgn& repaint_region, class Canvas* widget) {}
    virtual void GetBounds(double& x1, double& y1, double& x2, double& y2) const {}
    virtual Item* intersect(double x, double y) { 
       if (update_) { region(); }    
       CPoint pt(x, y);
       return rgn_.PtInRegion(pt) ? this : 0;
    }
    virtual void intersect(std::vector<Item*>& items, double x1, double y1, double x2, double y2) {}
    virtual bool OnEvent(Event* ev) { return 0; }
    virtual double x() const { return 0; }
    virtual double y() const { return 0; }    
    virtual double zoomabsx() const;
    virtual double zoomabsy() const;
    void boundingclientrect(double &x1, double &y1, double &x2, double &y2) {
      GetBounds(x1, y1, x2, y2);      
      x1 = zoomabsx();
      y1 = zoomabsy();
      x2 += zoomabsx();
      y2 += zoomabsy();
    }
    void GetFocus();        
    void set_manage(bool on) { managed_ = on; }
    bool managed() { return managed_; }
    // helper
    void swap_smallest(double& x1, double& x2) const {
      if (x1 > x2) {        
        std::swap(x1, x2);        
      }
    }    
    virtual void show() {STR(); visible_ = true; FLS(); }
    virtual void hide() {STR(); visible_ = false; FLS(); }
    bool visible() const { return visible_; }
    virtual void enablepointerevents() { pointer_events_ = true; }
    virtual void disablepointerevents() { pointer_events_ = false; }
    bool pointerevents() const { return pointer_events_; }
    void STR();  // store old region    
    void FLS();  // invalidate combine new & old region                   
    virtual void needsupdate(); 
    virtual double acczoom() const;
  protected:
    mutable bool update_;
    mutable CRgn rgn_;
    void checkbuttonpress();
  private:
    Group* parent_;
    CRgn fls_rgn_;
    std::string name_;
    bool managed_, visible_, pointer_events_, has_store_;
    std::vector<Item*> dummy;
  };

  class PaintItem : public Item {  
    PaintItem(Group* parent) : Item(parent) {}
    virtual void Draw(CDC* cr, const CRgn& repaint_region,
        class Canvas* widget); 
  };

  class Group : public Item {  
  public:
    friend Canvas;
    friend Item;
    Group();
    Group(Canvas* widget);
    Group(Group* parent, double x, double y);
    ~Group();
    
    void GetBounds(double& x1, double& y1, double& x2, double& y2) const;
    virtual void Draw(CDC* cr,
      const CRgn& repaint_region,
    class Canvas* widget);
    virtual Item* intersect(double x, double y);
    virtual void intersect(std::vector<Item*>& items, double x1, double y1, double x2, double y2);  
    typedef std::vector<Item*>::iterator iterator;
    virtual iterator begin() { return items_.begin(); }
    virtual iterator end() { return items_.end(); }
    virtual bool empty() const { return items_.empty(); }
    virtual bool size() const { return items_.size(); }    
    bool is_root() const { return is_root_; }
    void set_zorder(Item* item, int z);
    int zorder(Item* item) const;
    void Add(Item* item);
    void Erase(Item* item);
    void Clear();
    void Clear2() { items_.clear(); }
    void Insert(iterator it, Item* item);
    void RaiseToTop(Item* item);        
    void SetXY(double x, double y) { 
      STR();
      x_ = x;
      y_ = y;
      FLS();
    }
    double x() const { return x_; }
    double y() const { return y_; }    
    void setzoom(double zoom) { zoom_ = zoom; }
    virtual double zoom() const { return zoom_; }
    Canvas* widget() { return widget_; }
    virtual Canvas* widget() const { return widget_; }
    virtual const CRgn& region() const;    
  protected:
    std::vector<Item*> items_;
  private:    
    Canvas* widget_;    
    double x_, y_;    
    bool is_root_;
    double zoom_;
  };

  class Rect : public Item {
  public:
    Rect();
    Rect(Group* parent);
    Rect(Group* parent, double x1, double y1, double x2, double y2 );
    virtual ~Rect() {}

    void SetXY(double x, double y);
    void SetPos(double x1, double y1, double x2, double y2) {   
      STR();
      x1_ = x1;
      y1_ = y1;
      x2_ = x2;
      y2_ = y2;    
      FLS();
    }    
    double x() const { return x1_; }
    double y() const { return y1_; }
    double x1() const { return x1_; }
    double y1() const { return y1_; }
    double x2() const { return x2_; }
    double y2() const { return y2_; }    
    // fill colors
    void SetFillColor(ARGB color) { fillcolor_ = color; skin_ = 0; FLS(); }
    ARGB fillcolor() const { return fillcolor_; }
    void SetSkinFillColor(int skin) {      
      fillcolor_ = Skin::get_color(skin);
      skin_ = skin;
      FLS();
    }
    int skinfillcolor() const { return skin_; }
    // stroke colors
    void SetStrokeColor(ARGB color) { strokecolor_ = color; skin_stroke_ = 0; FLS(); }
    ARGB strokecolor() const { return strokecolor_; }
    void SetSkinStrokeColor(int skin) {      
      fillcolor_ = Skin::get_color(skin);
      skin_stroke_ = skin;
      FLS();
    }
    int skinstrokecolor() const { return skin_; }    
    void SetBorder(double bx, double by) { STR(); bx_ = bx; by_ = by; FLS(); }
    void border(double &bx, double &by) const { bx = bx_; by = by_; }    
    virtual void Draw(CDC* cr, const CRgn& repaint_region,
        class Canvas* widget);
    virtual const CRgn& region() const;    
    virtual void intersect(std::vector<Item*>& items, double x1, double y1, double x2, double y2);
    virtual void GetBounds(double& x1, double& y1, double& x2,
      double& y2) const;
  private:
    double x1_, y1_, x2_, y2_, bx_, by_;
    ARGB fillcolor_, strokecolor_;    
    int skin_, skin_stroke_;    
    bool paintRect(CDC &hdc, RECT dim, COLORREF penCol, COLORREF brushCol, unsigned int opacity);
  };

  class PixBuf : public Item {
  public:
    PixBuf();
    PixBuf(Group* parent);
    PixBuf(Group* parent, double x, double y, CBitmap* image);    
    ~PixBuf();
    // todo copy ctor, =operator
    virtual void Draw(CDC* cr,
      const CRgn& repaint_region,
    class Canvas* widget);
    virtual const CRgn& region() const;
    void SetXY(double x, double y) {
      STR();
      x_ = x;
      y_ = y;   
      FLS();
    }
    double x() const { return x_; }
    double y() const { return y_; }
    void SetSize(int width, int height) { 
      STR();
      width_ = width; 
      height_ = height; 
      FLS();
    }
    void SetSource(int xsrc, int ysrc) {
      xsrc_ = xsrc;
      ysrc_ = ysrc;
      FLS();
    }
    void SetImage(int skin);
    void SetImage(CBitmap* image);
    void SetMask(CBitmap* mask) { mask_ = mask; }

    void Load(const std::string& filename);
    
    virtual void intersect(std::vector<Item*>& items, double x1, double y1, double x2, double y2);

    void GetBounds(double& x1, double& y1, double& x2, double& y2) const;
    int width() const { return width_; }
    int height() const { return height_; }

    void SetTransparent(bool on, double r=0.0f, double g=0.0f, double b=0.0f) {
      transparent_ = on;      
      mask_ = new CBitmap();
      PrepareMask(image_, mask_, RGB(r, g, b));
    }

  private:    
    void PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans);
    void TransparentBlt(CDC* pDC,
      int xStart,
      int yStart,
      int wWidth,
      int wHeight,
      CDC* pTmpDC,
      CBitmap* bmpMask,
      int xSource = 0,
      int ySource = 0);	
    void GetDim();
    CBitmap* image_;
    CBitmap* mask_;
    double x_;
    double y_;
    int width_;
    int height_;
    int xsrc_;
    int ysrc_;
    int skin_;    
    bool transparent_;  
    bool shared_;
    bool pmdone;
    CPngImage image;
  };

  class Line : public Item {
  public:
    Line() : Item(), color_(0), skin_(0) {}
    Line(Group* parent) : Item(parent), color_(0), skin_(0) {}
    ~Line() {}

    virtual void Draw(CDC* cr,
      const CRgn& repaint_region,
    class Canvas* widget);
    virtual Item* intersect(double x, double y);
    virtual void GetBounds(double& x1, double& y1, double& x2,
      double& y2) const;
    typedef std::vector<std::pair<double,double> > Points;
    typedef std::pair<double, double> Point;
    void SetPoints(const Points& pts) {  STR(); pts_ = pts; FLS(); }
    void SetPoint(int idx, const Point& pt) { STR(); pts_[idx] = pt; FLS(); }
    const Points& points() const { return pts_; }
    const Point& PointAt(int index) const { return pts_.at(index); }
   void SetColor(ARGB color) { color_ = color; skin_ = 0; FLS(); }
    ARGB color() const { return color_; }
    void SetSkinColor(int skin) {      
      color_ = Skin::get_color(skin);
      skin_ = skin;
      FLS();
    }
    int skincolor() const { return skin_; }
    void SetXY(double x, double y);
    double x() const { return pts_.size() > 0 ? pts_[0].first : 0; }
    double y() const { return pts_.size() > 0 ? pts_[0].second : 0; }
    virtual const CRgn& region() const;
    //virtual void show();
    //virtual void hide();

  private:
    Points pts_;
    ARGB color_;
    int skin_;    
  };

  class Text : public Item {
  public:
    Text() : Item() { Init(1.0); }
    Text(Group* parent);
    Text(Group* parent, const std::string& text);
    void Init(double zoom);
    virtual ~Text() { }

    void SetText(const std::string& text) { STR(); text_ = text; FLS(); }
    const std::string& text() const { return text_; }
    void SetXY(double x, double y) {
      STR();
      x_ = x;
      y_ = y;    
      FLS();
    }
    double x() const { return x_; }
    double y() const { return y_; }
    void SetColor(ARGB color) { color_ = color; skin_ = 0; FLS(); }
    ARGB color() const { return color_; }
    void SetSkinColor(int skin) {      
      color_ = Skin::get_color(skin);
      skin_ = skin;
      FLS();
    }
    int skincolor() const { return skin_; }
    void SetFont(const CFont& font) {
      LOGFONT lf;
      const_cast<CFont&>(font).GetLogFont(&lf);
      font_.DeleteObject();
      font_.CreateFontIndirect(&lf);
    }
    virtual void Draw(CDC* cr,
      const CRgn& repaint_region,
    class Canvas* widget);
    virtual void GetBounds(double& x1, double& y1, double& x2,
      double& y2) const;
    virtual const CRgn& region() const;    
    virtual void intersect(std::vector<Item*>& items, double x1, double y1, double x2, double y2);

  private:    
    std::string text_;
    double x_, y_;
    ARGB color_;
    int skin_;
    CFont font_;
    mutable int text_w, text_h;    
  };


  class Canvas
  {
    friend class Item;
    friend class Group;
  public:
    Canvas() : parent_(0) { Init(); }
    Canvas(CWnd* parent) : parent_(parent) { Init(); }
    void Init();
    ~Canvas();

    void SetParent(CWnd* parent) {parent_ = parent;}
    Group* root() { return root_; } 
    void Draw(CDC *devc, const CRgn& rgn);
    void DrawFlush(CDC *devc, const CRgn& rgn);
    void SetColor(ARGB color) { color_ = color; skin_ = 0; }
    ARGB color() const { return color_; }
    void SetSkinColor(int skin) {
      color_ = Skin::get_color(skin);
      skin_ = skin;
    }
    int skincolor() const { return skin_; }
        
    void set_bg_image(CBitmap* bg_image, int width, int height) {
      bg_image_ = bg_image;
      bg_width_ = width;
      bg_height_ = width;
    }

    void StealFocus(Item* item);
    virtual canvas::Item* OnEvent(Event* ev);
    virtual void OnSize(int cx, int cy) {
       cw_ = cx;
       ch_ = cy;
    }
    void SetSave(bool on) { save_ = on; }
    bool IsSaving() const { return save_; }
    void Flush();        
    CWnd* parent() { return parent_; }
    int cw() const { return cw_; }
    int ch() const { return ch_; }
    void setpreferredsize(double width, double height) {
      pw_ = width;
      ph_ = height;
    }
    void preferredsize(double& width, double& height) const {
      width = pw_;
      height = ph_;
    }
    void InvalidateSave() {
      if (parent_ &&::IsWindow(parent_->m_hWnd)) {
          parent_->InvalidateRgn(&save_rgn_, 0);        
      }
    }
  void SetCapture() {
    if (parent_ && ::IsWindow(parent_->m_hWnd)) {          
      ::SetCapture(parent_->m_hWnd);        
    }
  }
  void ReleaseCapture() {
    if (parent_ && ::IsWindow(parent_->m_hWnd)) {
      ::ReleaseCapture();        
    }
  }
  void ShowCursor() { while (::ShowCursor(TRUE) < 0); }
  void HideCursor() { while (::ShowCursor(FALSE) >= 0); }
  void SetCursorPos(int x, int y) {
    if (parent_) {
      CPoint point(x, y);
      parent_->ClientToScreen(&point);				
		  ::SetCursorPos(point.x, point.y);
    }
  }

  protected:
    void Invalidate(CRgn& rgn);
    
  private:
    Canvas(const Canvas& other) {}
    Canvas& operator=(Canvas rhs) {}
    bool DelegateEvent(Event* event, Item* item);
    CWnd* parent_;    
    Group* root_;
    bool save_, steal_focus_, managed_;    
    Item *button_press_item_,  *out_item_;    
    CBitmap* bg_image_;
    int bg_width_, bg_height_;
    int cw_, ch_, pw_, ph_;
    int skin_;
    ARGB color_;    
    CRgn save_rgn_;    
  };

}}}

