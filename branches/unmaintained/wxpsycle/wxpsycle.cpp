#include <wx/wx.h>
#include "mainwindow.h"
#include "bitmaps.h"

class MyApp : public wxApp {
public:
  virtual bool OnInit() {
    Bitmaps::init();
    MainWindow* frame = new MainWindow();
    frame->Show(true);
    SetTopWindow(frame);
    return true;
  }
};

IMPLEMENT_APP(MyApp)
