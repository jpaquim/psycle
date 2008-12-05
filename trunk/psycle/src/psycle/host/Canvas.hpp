#pragma once

namespace TestCanvas {

  struct Event {
	  enum Type {
		  BUTTON_PRESS,
		  BUTTON_2PRESS,
		  BUTTON_RELEASE,
		  MOTION_NOTIFY
	  };		
	  Type type;
	  double x, y;
	  int button;
  };

  class Item {
   public:
    Item();
    Item(class Group* parent);
    virtual ~Item();

    void set_name(const std::string& name) {
      name_ = name; 
    }

    const std::string& name() const { return name_; }

    void set_parent(class Group* parent) {
      parent_ = parent;
    }

    Group* parent() { return parent_; }
    const Group* parent() const { return parent_; }

    virtual const CRgn& region() const;
    virtual void Draw(CDC* cr,
                      const CRgn& repaint_region,
                      class Canvas* widget);
    virtual void GetBounds(double& x1, double& y1, double& x2, double& y2) const;
    virtual Item* intersect(double x, double y);

    virtual bool OnEvent(Event* ev);
	
    // sigc::signal<bool, GdkEvent*>& signal_event() { return m_signal_event_; }

    void GetFocus();
    void QueueDraw();
    void InvalidateRegion(CRgn* region);

    void set_manage(bool on) {
      managed_ = on;
    }

    bool managed() { return managed_; }

    // helper
    void swap_smallest(double& x1, double& x2) const {
      if ( x1 > x2 ) {
        double tmp;
        tmp = x1;
        x1 = x2;
        x2 = tmp;
      }
    }

   void RectToRegion(CRgn* region,
                     double x1,
                     double y1,
                     double x2,
                     double y2) const;

  // virtual bool on_drag_data_received(
  //    const Glib::RefPtr<Gdk::DragContext>& context, int, int,
  //    const Gtk::SelectionData& selection_data, guint, guint time);

   void SetVisible(bool on) {
	   visible_ = on;
   }

   bool visible() const { return visible_; }

   private:
    Group* parent_;
    CRgn rgn_;
    std::string name_;
    bool managed_;
	bool visible_;
    // sigc::signal<bool, GdkEvent*> m_signal_event_;
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
   
    double x() const { return x_; }
    double y() const { return y_; }
    double absx() const;
    double absy() const;
    Canvas* widget() { return widget_; }
    const Canvas* widget() const { return widget_; }

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
    virtual ~Rect() {}

    void SetPos(double x1, double y1, double x2, double y2);

    double x1() const { return x1_; }
    double y1() const { return y1_; }
    double x2() const { return x2_; }
    double y2() const { return y2_; }

    void SetColor(double r, double g, double b, double alpha);
    void SetOutlineColor(double r, double g, double b, double alpha);

    virtual void Draw(CDC* cr,
                      const CRgn& repaint_region,
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

		virtual Item* intersect(double x, double y);

		void GetBounds(double& x1, double& y1, double& x2, double& y2) const;

	private:
		CBitmap* image_;
		double x_;
		double y_;
		int width_;
		int height_;
		int xsrc_;
		int ysrc_;

		mutable CRgn rgn_;

  };

  class Line : public Item {
   public:
     Line();
     Line(Group* parent);

     virtual void Draw(CDC* cr,
                      const CRgn& repaint_region,
                      class Canvas* widget);

     virtual Item* intersect(double x, double y);
     virtual void GetBounds(double& x1, double& y1, double& x2,
                            double& y2) const;

     typedef std::vector<std::pair<double,double> > Points;
     void SetPoints(const Points& pts);
    
     const Points& points() const {
       pts_;
     }

     const std::pair<double,double>& PointAt(int index) const {
       return pts_.at(index);
     }

     void SetColor(double r, double g, double b, double alpha);
     virtual const CRgn& region() const;

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
    virtual ~Text() { }

    void SetText(const std::string& text) {
      text_ = text;
      update_ = true;
    }
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
	public:
		Canvas(CWnd* parent);
		~Canvas();

		Group* root() { return &root_; } 

		void Draw(CDC *devc, const CRect& repaint_region);

		void set_bg_color(const COLORREF& color) {
		  bg_color_ = color;
		}

		void StealFocus(Item* item);

		virtual void OnEvent(Event* ev);

	private:
		bool DelegateEvent(Event* event, Item* item);

		CWnd* parent_;
		COLORREF bg_color_;
		Group root_;
		Item* button_press_item_;
		bool steal_focus_;
};

}