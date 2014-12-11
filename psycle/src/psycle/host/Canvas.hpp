// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle { namespace host { namespace canvas {

  struct Item;
  struct Canvas;

  struct Event {
    enum Type {
      BUTTON_PRESS,
      BUTTON_2PRESS,
      BUTTON_RELEASE,
      MOTION_NOTIFY
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

  class Item {
  public:
    Item();
    Item(class Group* parent);
    virtual ~Item();

    Canvas* canvas();    

    virtual Canvas* widget() const { return 0; }
    void set_name(const std::string& name) { name_ = name; }
    const std::string& name() const { return name_; }

    void set_parent(class Group* parent) { parent_ = parent; }
    Group* parent() { return parent_; }
    const Group* parent() const { return parent_; }

    virtual const CRgn& region() const;
    virtual void Draw(CDC* cr, const CRgn& repaint_region,
                      class Canvas* widget);
    virtual void GetBounds(double& x1, double& y1, double& x2, double& y2) const;
    virtual Item* intersect(double x, double y);

    virtual bool OnEvent(Event* ev);

    virtual double x() const { return 0; }
    virtual double y() const { return 0; }    

    void GetFocus();
    virtual void QueueDraw();
    void InvalidateRegion(CRgn* region);

    void set_manage(bool on) { managed_ = on; }
    bool managed() { return managed_; }

    // helper
    void swap_smallest(double& x1, double& x2) const {
      if (x1 > x2) {        
        std::swap(x1, x2);        
      }
    }

    void RectToRegion(CRgn* region,
      double x1,
      double y1,
      double x2,
      double y2) const;    

    virtual void show();
    virtual void hide();
    bool visible() const { return visible_; }

  private:
    Group* parent_;
    CRgn rgn_;
    std::string name_;
    bool managed_;
    bool visible_;    
  };

  class Group : public Item {
  public:
    Group();
    Group(Canvas* widget);
    Group(Group* parent, double x, double y);
    ~Group();

    void SetXY(double x, double y);
    void Move(double delta_x, double delta_y);
    void GetBounds(double& x1, double& y1, double& x2, double& y2) const;

    virtual void Draw(CDC* cr,
      const CRgn& repaint_region,
    class Canvas* widget);

    virtual Item* intersect(double x, double y);

    typedef std::vector<Item*>::iterator iterator;
    iterator begin() { return items_.begin(); }
    iterator end() { return items_.end(); }

    void Add(Item* item);
    void Erase(Item* item);
    void Clear();
    void Insert(iterator it, Item* item);
    void RaiseToTop(Item* item);    
    virtual void QueueDraw();

    double x() const { return x_; }
    double y() const { return y_; }
    double absx() const;
    double absy() const;
    Canvas* widget() { return widget_; }
    virtual Canvas* widget() const { return widget_; }

    virtual const CRgn& region() const;

  private:
    Canvas* widget_;
    std::vector<Item*> items_;
    double x_, y_;
    mutable CRgn rgn_;
  };

  class Rect : public Item {
  public:
    Rect();
    Rect(Group* parent);
    Rect(Group* parent, double x1, double y1, double x2, double y2 );
    virtual ~Rect() {rgn_.DeleteObject(); }

    void SetXY(double x, double y);
    void SetPos(double x1, double y1, double x2, double y2);

    double x1() const { return x1_; }
    double y1() const { return y1_; }
    double x2() const { return x2_; }
    double y2() const { return y2_; }

    void SetColor(double r, double g, double b, double alpha);
    void SetOutlineColor(double r, double g, double b, double alpha);

    virtual void Draw(CDC* cr, const CRgn& repaint_region,
      class Canvas* widget);

    virtual const CRgn& region() const;
    virtual Item* intersect(double x, double y);
    virtual void GetBounds(double& x1, double& y1, double& x2,
      double& y2) const;

  private:
    double x1_, y1_, x2_, y2_;
    double r_, g_, b_, alpha_;
    double r_outline_, g_outline_, b_outline_, alpha_outline_;
    mutable CRgn rgn_;
    mutable bool update_;
    bool paintRect(CDC &hdc, RECT dim, COLORREF penCol, COLORREF brushCol, unsigned int opacity);
  };

  class PixBuf : public Item {
  public:
    PixBuf();
    PixBuf(Group* parent);
    PixBuf(Group* parent, double x, double y, CBitmap* image);
    ~PixBuf();

    virtual void Draw(CDC* cr,
      const CRgn& repaint_region,
    class Canvas* widget);

    virtual const CRgn& region() const;

    void SetXY(double x, double y);

    double x() const { return x_; }
    double y() const { return y_; }

    void SetSize(int width, int height);
    void SetSource(int xsrc, int ysrc);
    void SetImage(CBitmap* image);
    void SetMask(CBitmap* mask);

    virtual Item* intersect(double x, double y);

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
    CBitmap* image_;
    CBitmap* mask_;
    double x_;
    double y_;
    int width_;
    int height_;
    int xsrc_;
    int ysrc_;
    mutable CRgn rgn_;
    bool transparent_;
  };

  class Line : public Item {
  public:
    Line();
    Line(Group* parent);
    ~Line() {rgn_.DeleteObject();}

    virtual void Draw(CDC* cr,
      const CRgn& repaint_region,
    class Canvas* widget);
    virtual Item* intersect(double x, double y);
    virtual void GetBounds(double& x1, double& y1, double& x2,
      double& y2) const;
    typedef std::vector<std::pair<double,double> > Points;
    void SetPoints(const Points& pts);
    const Points& points() const { return pts_; }
    const std::pair<double,double>& PointAt(int index) const {
      return pts_.at(index);
    }
    void SetColor(double r, double g, double b, double alpha) {      
      r_ = r; 
      g_ = g;
      b_ = b;
      alpha_ = alpha;
    }
    void SetXY(double x, double y);
    double x() const { 
      return pts_.size() > 0 ? pts_[0].first : 0;
    }
    double y() const {
      return pts_.size() > 0 ? pts_[0].second : 0;
    }
    virtual const CRgn& region() const;
    //virtual void show();
    //virtual void hide();

  private:
    Points pts_;
    double r_, g_, b_, alpha_;
    mutable CRgn rgn_;
    mutable bool update_;
  };

  class Text : public Item {
  public:
    Text();
    Text(Group* parent);
    Text(Group* parent, const std::string& text);
    virtual ~Text() { rgn_.DeleteObject(); }

    void SetText(const std::string& text);
    const std::string& text() const { return text_; }

    void SetXY(double x, double y);

    double x() const { return x_; }
    double y() const { return y_; }

    void SetColor(double r, double g, double b, double alpha);
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

    virtual Item* intersect(double x, double y);

  private:
    void UpdateValues() const;
    std::string text_;
    double x_, y_;
    double r_, g_, b_, alpha_;
    CFont font_;
    mutable int text_w, text_h;
    mutable bool update_;
    mutable CRgn rgn_;
  };


  class Canvas
  {
    friend class Item;
    friend class Group;
  public:
    Canvas();
    Canvas(CWnd* parent);
    ~Canvas();

    void SetParent(CWnd* parent) {parent_ = parent;}
    Group* root() { return &root_; } 

    void Draw(CDC *devc, const CRgn& rgn);

    void set_bg_color(const COLORREF& color) {
      bg_color_ = color;
    }

    void set_bg_image(CBitmap* bg_image, int width, int height) {
      bg_image_ = bg_image;
      bg_width_ = width;
      bg_height_ = width;
    }

    void StealFocus(Item* item);

    virtual canvas::Item* OnEvent(Event* ev);
    virtual void OnSize(int cx, int cy);

    void SetSave(bool on) { save_ = on; }
    bool IsSaving() const { return save_; }
    void Flush();
    void SetAutomaticDraw(bool on) {
      has_draw_ = on;
    }
    bool HasAutomaticDraw() const {
      return has_draw_;
    }

    CWnd* parent() { return parent_; }

    int cw() const { return cw_; }
    int ch() const { return ch_; }
    
  private:
    bool DelegateEvent(Event* event, Item* item);

    CWnd* parent_;
    COLORREF bg_color_;
    Group root_;
    bool save_;
    bool has_draw_;
    Item* button_press_item_;
    bool steal_focus_;
    CBitmap* bg_image_;
    int bg_width_;
    int bg_height_;
    int cw_;
    int ch_;
    CRgn save_rgn_;
  };

}}}

