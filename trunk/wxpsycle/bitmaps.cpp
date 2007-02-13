#include "bitmaps.h"
#include <wx/wx.h>
#include <cassert>

namespace Bitmaps {

  wxBitmap* _new;
  wxBitmap* save;
  wxBitmap* open;

  void init() {

#define load(var,path) \
    var = new wxBitmap(wxT(path));		\
    assert(var->Ok());

    load(_new,"images/new.png");
    load(save,"images/save.png");
    load(open,"images/open.png");
#undef load
  }

}
