/***************************************************************************
 *   Copyright (C) 2005 by Stefan   *
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

#include "nconfig.h"
#include "nbevelborder.h"
#include "nframeborder.h"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/sax2/Attributes.hpp>
#include <nfile.h>
#include <iostream>


XERCES_CPP_NAMESPACE_USE;

using namespace std;

class MySAX2Handler : public DefaultHandler {

public:
    void startElement(
        const   XMLCh* const    uri,
        const   XMLCh* const    localname,
        const   XMLCh* const    qname,
        const   Attributes&     attrs
    );
    void fatalError(const SAXParseException&);

    void setCfg(NConfig* cfg) { pCfg = cfg;};

    std::string getValue(const std::string & name, const Attributes& attrs) {
       std::string erg;
       try {
       XMLCh* str = XMLString::transcode(name.c_str());
       const XMLCh* strCh = attrs.getValue(str);
       char* id = XMLString::transcode(strCh);
       erg = std::string(id);
       XMLString::release(&id);
       XMLString::release(&str);
       
       } catch (std::exception e) { return ""; }
      return erg;
    }

    NColor attrsToColor(const Attributes& attrs) {
       std::string rStr = getValue("r",attrs);
       std::string gStr = getValue("g",attrs);
       std::string bStr = getValue("b",attrs);

       std::stringstream str1; str1 << rStr; int r = 0; str1 >> r;
       std::stringstream str2; str2 << gStr; int g = 0; str2 >> g;
       std::stringstream str3; str3 << bStr; int b = 0; str3 >> b;

       return NColor(r,g,b);
    }

private:

    std::string lastId;
    NConfig* pCfg;

};


void MySAX2Handler::startElement(const   XMLCh* const    uri,
                            const   XMLCh* const    localname,
                            const   XMLCh* const    qname,
                            const   Attributes&     attrs)
{
    char* message = XMLString::transcode(localname);
    //cout << "I saw element: "<< message << endl;
    std::string tagName = std::string(message);


  ///todo   NBorder* border;
 
    if (tagName == "path") {
      std::string id   = getValue("id",attrs);
      std::string path = getValue("src",attrs);
      pCfg->pathMap[id]=path;
    }

    if (tagName == "vcskin") {
        NSkin skin;
        std::string id      = getValue("id",attrs);
        lastId = id;
        pCfg->skinMap[id] = skin;
    } else
    if (tagName == "bitmap") {
      NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         std::string src = getValue("src",attrs);
         std::string styleStr = getValue("style",attrs);
         std::stringstream str; str << styleStr;
         int style  = 0;
         str >> style;
         skin->bitmap.loadFromFile(src);
         skin->bitmapBgStyle = style;
      }
    } else
    if (tagName == "font") {
      NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         std::string name = getValue("name",attrs);
         std::string size = getValue("size",attrs);
         std::stringstream str; str << size; int sz = 0; str >> sz;
         skin->font.setName(name);
         skin->font.setSize(sz);
         std::string color = getValue("color",attrs);
         skin->font.setTextColor(NColor(color));
      }
    } else
    if (tagName == "frmborder") {
     NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         NFrameBorder* fr = new NFrameBorder();
         skin->border = fr;

         std::string styleStr = getValue("style",attrs);
         if (styleStr != "") {
           std::stringstream str; str << styleStr;
           int style  = 0;
           str >> style;
           fr->setOval(style);
         }

         std::string arcw  = getValue("arcw",attrs);
         std::string arch  = getValue("arch",attrs);

         if (arcw!="") {
           int arc  = 0;
           std::stringstream str3;
           str3 << arcw;
           str3 >> arc;
           fr->setOval(fr->oval(),arc,fr->arcHeight());
         }

         if (arch!="") {
           int arc  = 0;
           std::stringstream str3;
           str3 << arch;
           str3 >> arc;
           fr->setOval(fr->oval(),fr->arcWidth(),arc);
         }
      }
    } else

    if (tagName == "bgcolor") {
      NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         skin->bgColor = attrsToColor(attrs);
         skin->useParentBgColor = false;
         skin->transparent = false;
      }
    } else
    if (tagName == "fgcolor") {
      NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         skin->fgColor = attrsToColor(attrs);
         skin->useParentFgColor = false;
      }
    } else
    if (tagName == "translucent") {
      NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         std::string color = getValue("color",attrs);
         std::string percentStr = getValue("percent",attrs);
         std::stringstream str; str << percentStr;
         int percent  = 0;
         str >> percent;
         skin->translucent = percent;
         skin->transColor = NColor(color);
      }
    } else
    if (tagName == "gradient") {
      NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         std::string start = getValue("start",attrs);
         std::string mid = getValue("mid",attrs);
         std::string end = getValue("end",attrs);
         std::string styleStr   = getValue("style",attrs);
         std::string percentStr = getValue("percent",attrs);
         std::string direction  = getValue("dir",attrs);
         std::string arcw  = getValue("arcw",attrs);
         std::string arch  = getValue("arch",attrs);
         std::stringstream str; str << styleStr;
         int style  = 0;
         str >> style;
         std::stringstream str1; 
         str1 << percentStr;
         int percent = 50;
         str1 >> percent;
         skin->gradStartColor = NColor(start);
         skin->gradMidColor   = NColor(mid);
         skin->gradEndColor   = NColor(end);
         skin->gradientStyle   = style;
         skin->gradientPercent = percent;

         if (direction == "hor") skin->gradientOrientation = nHorizontal;
                             else skin->gradientOrientation = nVertical;

         if (arcw!="") {
           int arc  = 0;
           std::stringstream str3;
           str3 << arcw;
           str3 >> arc;
           skin->arcWidth  = arc;
         }

         if (arch!="") {
           int arc  = 0;
           std::stringstream str3;
           str3 << arch;
           str3 >> arc;
           skin->arcHeight  = arc;
         }

      }
    } else
    if (tagName == "spacing") {
      NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         std::string l = getValue("left",attrs);
         std::string t = getValue("top",attrs);
         std::string r = getValue("right",attrs);
         std::string b = getValue("bottom",attrs);
         std::stringstream str; str << l; int left = 0; str >> left;
         std::stringstream str1; str1 << t; int top  = 0; str1 >> top;
         std::stringstream str2; str2 << r; int right  = 0; str1 >> right;
         std::stringstream str3; str3 << b; int bottom  = 0; str1 >> bottom;

         skin->spacing.setSize(left,top,right,bottom);
      }
    }
    /*if (tagName!="ngrs") {
        NSkin skin;
        pCfg->skinMap[tagName] = skin;
    }*/
    XMLString::release(&message);
}

void MySAX2Handler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    cout << "xml error: " << message
         << " at line: " << exception.getLineNumber()
         << endl;
    cout << "using instead defaults" << endl;
}



/* XPM */
/* XPM */
const char *chipset2[] = {
/* width height num_colors chars_per_pixel */
"   195   195       93            2",
/* colors */
".. c #000000",
".# c #030303",
".a c #050505",
".b c #070707",
".c c #090909",
".d c #0a0a0a",
".e c #0c0c0c",
".f c #0e0e0e",
".g c #0f0f0f",
".h c #111111",
".i c #121212",
".j c #141414",
".k c #151515",
".l c #171717",
".m c #181818",
".n c #191919",
".o c #1b1b1b",
".p c #1c1c1c",
".q c #1e1e1e",
".r c #1f1f1f",
".s c #202020",
".t c #212121",
".u c #232323",
".v c #242424",
".w c #252525",
".x c #272727",
".y c #282828",
".z c #292929",
".A c #2a2a2a",
".B c #2c2c2c",
".C c #2d2d2d",
".D c #2e2e2e",
".E c #2f2f2f",
".F c #303030",
".G c #323232",
".H c #333333",
".I c #343434",
".J c #353535",
".K c #363636",
".L c #373737",
".M c #393939",
".N c #3a3a3a",
".O c #3b3b3b",
".P c #3c3c3c",
".Q c #3d3d3d",
".R c #3e3e3e",
".S c #3f3f3f",
".T c #404040",
".U c #424242",
".V c #434343",
".W c #444444",
".X c #454545",
".Y c #464646",
".Z c #474747",
".0 c #484848",
".1 c #494949",
".2 c #4a4a4a",
".3 c #4b4b4b",
".4 c #4d4d4d",
".5 c #4e4e4e",
".6 c #4f4f4f",
".7 c #505050",
".8 c #515151",
".9 c #525252",
"#. c #535353",
"## c #545454",
"#a c #555555",
"#b c #565656",
"#c c #575757",
"#d c #585858",
"#e c #595959",
"#f c #5a5a5a",
"#g c #5b5b5b",
"#h c #5c5c5c",
"#i c #5d5d5d",
"#j c #5e5e5e",
"#k c #5f5f5f",
"#l c #606060",
"#m c #616161",
"#n c #626262",
"#o c #636363",
"#p c #646464",
"#q c #666666",
"#r c #676767",
"#s c #686868",
"#t c #696969",
"#u c #6b6b6b",
"#v c #6c6c6c",
"#w c #6e6e6e",
"#x c #6f6f6f",
"#y c #717171",
"#z c #737373",
"#A c #757575",
/* pixels */
".p.o.o.p.u.D.M.T.H.F.H.J.D.t.p.s.s.v.x.w.t.r.t.v.A.z.x.u.s.u.z.B.z.B.J.N.z.i.p.K.z.q.h.h.p.w.x.u.x.v.u.v.v.s.m.h.m.n.p.r.r.s.s.s.h.i.j.h.g.k.q.w.h.h.g.e.b.c.h.k.o.w.G.H.A.t.o.o.r.q.p.r.w.z.y.x.q.t.v.x.x.w.v.u.f.p.q.i.o.E.I.y.m.v.x.t.u.B.C.u.j.m.q.s.t.p.k.h.l.B.x.u.G.A.t.H.w.C.D.x.r.p.o.o.s.r.A.M.M.C.x.A.B.C.D.C.B.z.w.v.R.F.B.I.M.J.L.T.C.C.C.C.z.t.s.w.y.G.J.G.K.O.H.t.u.t.t.x.z.w.p.j.w.z.y",
".n.r.u.u.v.A.M.W.E.D.F.H.D.v.t.w.w.x.x.w.v.u.w.y.D.v.q.r.z.E.z.s.v.J.Q.E.n.k.y.N.x.r.l.m.s.x.x.v.p.w.B.D.x.p.j.g.k.n.s.w.w.s.n.k.m.l.k.i.h.k.o.r.h.i.g.e.d.e.i.n.v.A.E.E.z.u.p.p.q.o.p.r.u.w.x.w.m.s.z.B.z.v.u.u.h.k.i.c.f.s.H.L.E.v.o.t.z.D.z.v.f.l.r.t.r.o.l.m.o.A.v.t.E.A.u.F.J.D.y.u.r.o.n.n.t.E.J.D.z.C.D.A.B.C.D.D.C.y.u.s.L.M.K.E.B.F.N.R.K.H.E.B.v.o.o.s.D.E.I.O.P.I.C.y.x.s.o.r.x.y.s.l.u.w.w",
".t.r.s.x.E.L.P.R.E.C.C.D.z.v.v.y.z.x.v.v.x.y.y.y.y.t.q.u.B.E.A.u.B.O.K.q.k.y.H.D.q.p.o.p.r.t.s.r.o.w.F.D.r.k.j.m.o.q.s.u.t.p.l.j.q.n.l.j.k.l.l.l.j.j.g.e.c.f.k.n.B.C.A.y.v.s.q.o.n.p.r.s.s.t.u.v.p.w.D.F.A.v.u.v.v.v.s.m.l.u.M.0.H.q.m.B.N.I.v.q.p.w.A.w.q.n.p.t.r.y.t.s.C.A.v.C.G.v.r.v.y.t.r.t.w.Q.R.t.l.B.J.A.B.C.D.D.C.y.v.t.z.P.U.I.A.F.L.J.F.C.A.A.x.u.w.B.G.C.H.R.P.B.x.D.x.p.j.o.y.F.D.y.u.x.w",
".C.s.p.B.T.1.S.F.F.C.y.w.t.t.w.z.w.u.s.u.x.z.w.t.n.u.A.B.y.x.D.J.M.I.u.l.y.Q.J.m.p.r.u.v.v.t.s.s.t.y.A.v.m.j.p.x.t.q.n.j.h.i.k.m.o.l.i.j.l.l.j.g.n.k.f.c.c.f.l.o.C.x.s.q.p.p.p.m.l.p.r.r.q.o.q.t.C.D.E.D.z.x.x.x.t.r.t.w.t.p.s.C.F.u.q.y.G.F.B.B.J.I.F.z.u.q.s.w.u.w.t.u.z.z.x.z.A.s.t.C.E.w.s.v.B.P.L.p.k.A.I.A.D.D.C.B.A.A.A.A.o.G.T.R.J.F.C.y.z.w.x.A.B.B.E.I.A.A.F.L.G.w.v.z.r.o.m.t.D.M.O.O.v.x.x",
".F.C.C.L.V.W.M.A.B.y.t.p.r.w.A.D.r.q.r.u.x.x.s.n.p.v.B.D.A.C.I.P.L.z.q.w.K.N.z.j.s.v.z.A.y.v.u.u.u.v.u.s.q.r.v.y.o.m.i.e.e.g.k.m.i.g.g.i.l.k.h.d.i.e.a.a.d.i.q.u.x.q.j.h.j.m.m.k.m.o.q.q.o.m.p.s.P.J.D.y.x.y.y.y.u.p.s.C.B.q.l.o.I.J.B.n.k.v.H.N.O.H.y.t.t.u.u.t.w.u.w.y.x.x.z.w.D.y.z.C.y.s.t.A.E.E.z.s.t.A.D.B.E.C.z.y.z.D.I.L.x.z.J.T.Q.F.y.z.B.y.z.E.F.C.B.D.s.B.E.y.u.w.v.r.m.q.u.A.E.I.N.Q.B.C.z",
".B.L.U.T.J.B.A.D.u.t.r.p.t.C.H.H.s.s.t.v.w.u.q.o.B.w.t.z.L.Q.L.E.A.w.B.L.H.s.n.v.q.r.t.t.r.p.o.o.o.p.r.w.z.z.v.s.f.g.j.j.j.i.g.f.c.d.g.i.j.i.f.c.b...a.d.i.p.v.y.p.l.f.f.i.m.o.o.q.r.o.n.l.m.p.s.M.J.E.A.y.x.x.x.z.t.u.B.D.x.t.u.s.J.P.B.q.v.z.y.B.v.p.p.t.w.t.p.w.u.B.E.w.w.A.v.z.y.w.s.q.v.G.P.C.t.q.x.B.z.y.B.D.B.x.w.z.E.L.P.M.A.y.H.J.A.y.F.A.x.y.D.H.G.G.I.u.C.B.q.m.u.v.n.o.u.B.E.A.y.B.E.O.L.F",
".z.J.Q.L.z.s.w.F.r.v.x.x.C.I.H.C.y.y.y.w.u.s.t.v.D.x.v.D.Q.U.G.s.v.z.I.M.y.g.k.C.q.p.n.m.l.l.l.k.l.m.p.v.z.y.t.p.g.h.l.l.m.i.f.d.c.d.j.l.j.g.d.c.#.b.g.m.s.u.t.p.i.g.f.g.k.p.t.v.w.r.n.j.k.n.q.s.w.E.M.M.E.x.v.w.p.p.o.p.p.o.q.s.i.w.K.P.L.A.r.k.q.r.u.z.C.B.v.n.v.t.H.L.v.u.B.v.t.y.w.p.r.F.N.M.u.o.p.w.z.v.v.z.z.y.x.x.A.E.I.L.S.K.B.z.z.A.B.D.C.x.v.z.F.H.M.Q.I.E.x.q.o.q.r.p.x.A.D.D.y.t.t.u.N.J.B",
".C.A.x.v.x.A.C.F.t.A.F.G.J.L.D.s.D.D.B.x.s.s.w.B.w.A.F.M.P.K.z.q.B.C.E.C.s.i.n.z.y.u.q.o.p.q.q.p.p.p.p.q.r.t.t.t.o.m.l.i.h.h.i.i.c.f.m.n.k.g.d.d.d.i.o.w.z.v.l.h.e.g.h.k.n.t.x.z.A.t.l.h.k.o.r.t.i.z.T.W.K.x.t.w.t.y.B.w.r.s.w.z.t.n.q.E.K.B.w.x.o.w.I.Q.P.G.v.o.u.t.L.P.v.t.C.v.y.I.G.v.x.I.D.n.m.p.s.t.s.r.t.w.v.w.x.z.B.D.E.F.O.R.L.y.v.C.C.v.P.G.y.y.A.B.F.K.V.F.v.w.u.n.n.u.F.E.C.C.z.w.u.t.x.u.n",
".N.F.y.z.E.D.u.j.q.y.H.L.K.E.z.x.v.D.D.s.p.u.x.u.u.B.O.S.D.k.l.y.O.G.x.r.p.t.y.B.p.p.p.p.p.p.p.p.r.q.o.m.m.o.p.q.s.n.i.g.h.g.e.c.d.i.h.c.d.g.f.a.#.e.m.r.q.m.g.e.b.a.a.f.n.r.p.n.n.D.B.B.Q.M.B.M.s.o.u.J.M.B.t.u.t.y.z.u.p.q.y.I.M.F.x.t.s.s.r.q.m.l.p.t.o.i.r.F.z.P.i.m.E.t.x.A.z.s.z.R.T.B.n.o.r.q.q.q.r.t.v.w.t.s.v.C.L.M.G.z.C.B.B.A.C.D.z.u.O.U.O.D.G.R.I.o.l.E.O.C.o.n.w.A.O.G.x.s.r.s.r.p.y.B.u",
".N.L.I.G.E.B.x.u.A.E.K.L.I.D.z.y.C.t.o.s.y.z.u.p.z.M.T.G.o.i.v.J.N.B.r.r.z.E.x.p.p.p.p.p.p.p.p.p.q.p.o.n.n.o.o.o.q.l.h.h.h.g.e.c.#.h.m.j.g.f.d.#.f.i.n.o.m.h.e.d.a.c.h.m.p.r.n.l.g.u.s.w.J.G.y.H.F.u.k.n.D.M.E.s.x.v.t.q.p.t.B.H.G.B.x.s.s.r.q.p.q.o.q.u.x.z.E.K.O.G.s.s.n.n.A.u.v.k.v.T.T.q.h.s.r.r.p.p.p.r.s.u.t.s.v.A.H.K.I.F.z.A.A.B.D.C.z.v.r.y.E.I.J.J.D.x.v.z.C.y.v.w.w.v.B.y.v.t.u.s.r.q.p.G.F",
".N.R.S.M.D.x.A.E.I.J.K.J.F.C.z.z.z.n.l.w.B.w.t.x.M.R.J.p.i.u.G.J.E.z.v.y.D.D.u.k.q.q.q.q.q.q.q.q.n.o.o.o.o.o.n.n.n.k.h.h.h.g.f.d.c.i.k.h.f.f.f.e.p.p.o.k.h.d.c.c.b.h.n.s.s.p.l.k.e.p.p.p.A.y.r.w.1.U.D.r.z.P.Q.E.f.e.i.p.w.v.o.j.z.x.u.r.s.q.p.o.t.t.p.r.B.J.K.F.C.j.C.D.l.w.G.y.p.G.L.A.t.y.w.l.u.s.r.p.p.p.q.r.v.w.x.z.B.D.E.F.x.y.y.z.z.x.z.B.s.v.E.M.G.t.q.w.B.s.o.v.E.D.x.r.p.r.t.u.s.r.q.q.v.S.R",
".L.P.R.L.A.w.B.H.F.G.G.F.D.B.z.y.t.u.y.B.w.t.D.S.R.E.n.j.x.K.H.t.p.y.G.G.z.s.q.s.r.r.r.r.r.r.r.r.m.n.o.o.o.m.k.j.m.l.j.i.h.h.g.g.n.j.d.a.a.f.j.l.x.t.m.g.c.b.d.f.g.n.s.u.r.n.k.k.m.q.q.q.t.r.n.o.B.M.O.A.m.n.B.K.t.o.o.x.K.K.y.m.s.s.r.r.s.q.o.n.s.t.q.l.s.E.B.r.w.m.R.M.w.E.t.u.J.0.S.m.h.D.H.r.w.u.s.q.p.p.q.q.x.z.B.A.w.u.v.x.z.y.x.y.u.t.z.G.G.F.H.G.x.o.p.w.v.l.l.A.L.H.A.y.p.r.s.r.p.p.s.w.O.Y.Q",
".G.G.G.D.A.z.B.E.x.y.z.B.B.B.z.x.x.F.I.z.w.G.U.0.F.p.i.x.M.K.w.l.n.w.E.E.v.p.r.v.r.r.r.r.r.r.r.r.m.n.n.o.n.l.j.i.l.m.m.l.k.j.i.i.q.h.a.a.d.j.m.m.v.q.k.e.d.e.h.j.m.p.s.s.o.k.m.n.u.s.t.u.s.s.s.q.e.r.L.O.u.g.o.I.M.y.k.j.s.D.G.E.o.p.r.s.q.p.o.o.q.v.u.m.n.v.v.n.P.P.W.B.A.A.g.v.2.J.y.y.w.q.v.G.x.v.t.r.q.q.r.r.t.x.B.A.v.q.p.r.x.w.A.E.B.w.y.E.z.C.A.t.r.w.x.w.k.l.v.K.M.F.D.J.t.s.q.o.o.s.C.K.S.M.D",
".B.x.v.x.D.H.F.C.u.u.v.x.z.A.y.w.H.K.D.z.J.Z.Z.M.o.o.y.M.K.t.m.q.x.u.r.t.x.y.u.p.s.s.s.s.s.s.s.s.o.o.o.m.l.k.i.i.l.n.p.n.l.l.l.m.i.c.b.i.n.n.j.g.q.m.i.g.h.j.m.m.q.p.p.m.k.k.n.o.u.p.u.y.s.w.B.x.l.o.C.R.L.r.k.r.z.s.k.h.j.o.s.u.o.p.r.r.q.p.q.r.p.v.x.u.r.u.w.v.N.R.A.j.B.G.z.0.F.u.r.z.C.y.w.y.y.x.v.u.t.u.v.w.o.s.x.y.v.s.r.s.r.s.A.M.N.E.A.D.u.x.u.o.p.w.s.j.i.v.I.O.G.A.C.L.v.t.p.o.s.A.L.T.E.v.t",
".x.s.r.y.J.P.N.I.C.z.w.v.x.y.y.x.F.E.F.M.Y.Z.L.u.q.C.N.L.x.m.o.y.z.t.n.p.v.y.v.q.t.t.t.t.t.t.t.t.o.n.n.k.j.i.j.j.j.m.o.n.m.l.l.m.c.b.g.o.o.k.h.j.j.h.g.j.m.n.o.n.p.m.j.j.j.l.m.l.t.l.s.w.o.v.E.x.v.m.l.t.D.B.m.g.p.z.I.J.C.t.q.q.p.r.t.s.q.q.r.t.u.t.w.y.x.u.v.z.v.w.k.s.J.S.0.0.i.w.B.u.w.K.G.q.y.x.w.v.w.x.z.A.r.t.u.v.u.u.u.v.w.r.v.H.N.K.K.Q.I.B.t.q.o.m.j.l.v.I.O.I.y.u.y.E.w.w.u.w.C.G.K.L.w.w.z",
".v.s.s.B.O.V.T.O.M.G.y.u.v.x.y.y.v.x.L.3.3.L.t.n.F.Q.Q.A.o.r.w.y.s.u.v.t.q.q.t.x.t.t.t.t.t.t.t.t.p.n.m.j.i.h.j.j.g.k.n.o.k.j.k.m.c.f.k.o.i.e.i.s.f.f.h.k.p.q.p.l.n.k.h.h.k.m.l.j.u.k.s.v.k.q.B.s.y.t.j.k.D.W.Q.x.l.A.N.M.w.l.l.s.r.t.u.s.q.q.t.v.y.r.r.y.y.q.p.v.x.t.u.Y.U.N.W.k.r.u.B.F.F.A.C.H.w.w.v.v.w.y.A.C.z.x.v.t.t.u.u.v.H.t.n.w.F.K.V.8.W.D.r.t.q.h.o.F.J.S.P.A.q.s.v.t.A.A.B.E.J.H.D.y.C.K.K",
".u.y.K.U.Q.J.M.Y.R.H.B.D.A.v.y.G.s.L.3.0.I.q.o.t.M.J.w.i.l.x.w.m.q.i.w.C.j.l.y.l.l.u.y.q.n.s.u.q.p.q.n.k.j.n.n.l.u.n.g.c.c.f.g.h.i.s.p.f.g.x.B.s.e.d.f.k.q.t.s.q.n.l.i.f.g.i.k.o.m.g.q.r.g.t.C.i.s.y.y.q.k.q.D.M.k.p.r.r.n.n.r.w.q.s.u.v.w.w.v.u.v.v.u.u.t.s.s.r.f.h.E.O.s.j.n.i.t.r.p.q.r.v.w.w.y.x.w.w.t.t.r.q.x.w.w.v.v.u.s.s.J.H.u.m.F.3.Y.B.Y.6.V.x.j.p.A.B.0.K.w.x.x.s.i.e.q.x.E.G.D.y.w.v.J.A.q",
".D.J.S.X.R.I.D.F.N.D.v.x.B.B.z.z.K.P.R.H.v.r.y.H.I.v.p.w.x.p.m.s.t.s.l.g.n.p.r.w.w.w.u.q.n.s.t.s.v.l.p.C.y.j.h.w.q.m.g.f.f.i.i.i.o.l.i.l.v.A.t.j.r.o.l.l.o.t.x.z.k.k.k.i.i.j.k.l.h.q.p.m.q.t.v.z.w.w.v.o.j.o.E.T.K.E.x.t.v.x.y.w.r.s.t.u.v.v.v.v.u.u.t.t.s.r.r.r.p.h.r.A.q.r.y.t.q.r.s.s.r.s.v.x.t.t.u.v.v.w.w.w.v.v.u.u.u.u.t.s.O.K.L.W.0.S.I.C.M.R.N.z.t.x.D.D.K.D.y.y.u.p.n.p.t.H.R.Q.E.u.w.A.v.t.s",
".O.T.V.Q.K.F.A.v.A.B.y.u.v.A.B.A.V.O.C.t.q.w.F.M.u.n.q.A.A.p.m.r.o.x.g.e.B.A.m.x.E.z.v.w.u.q.n.o.l.w.A.s.k.k.k.h.l.k.i.h.i.k.j.g.n.j.o.A.D.t.l.j.x.w.t.p.m.n.q.t.k.j.l.l.l.k.k.j.j.u.h.i.H.C.r.J.J.F.B.A.v.r.A.L.Z.M.x.u.z.E.A.v.t.t.s.s.t.t.u.v.t.t.s.r.r.q.q.p.w.i.i.q.r.y.D.y.n.q.s.s.q.q.t.v.r.r.s.u.v.v.w.x.t.s.s.t.t.u.u.u.u.C.S.3.W.E.w.z.w.y.A.A.B.B.A.y.v.w.y.y.r.m.s.A.H.N.S.R.J.A.w.w.q.q.v",
".T.U.N.A.x.D.E.y.o.C.H.w.o.u.F.J.L.C.t.s.x.B.D.A.h.u.z.r.q.w.v.o.j.s.r.x.O.J.s.s.C.y.z.G.C.q.m.p.G.P.M.x.q.x.x.q.h.h.j.j.k.j.g.e.h.r.F.I.v.i.k.u.s.v.x.u.n.g.e.e.m.l.m.k.k.m.m.l.t.p.i.t.Q.L.w.A.I.B.E.Q.T.H.z.A.O.G.x.t.w.z.y.w.u.t.s.r.r.s.t.u.s.s.r.q.q.p.p.o.r.m.m.r.s.u.x.y.l.p.s.r.n.m.o.r.t.t.s.r.q.r.q.q.q.q.r.r.s.t.v.v.n.K.X.M.A.z.A.x.n.l.r.C.H.D.u.r.q.u.w.u.o.o.z.K.Y.M.C.E.L.M.z.n.x.u.z",
".L.M.E.s.s.B.D.y.s.C.D.v.t.C.J.K.w.s.q.u.B.C.u.m.o.x.y.n.k.t.y.v.u.p.B.L.D.G.K.x.x.u.x.E.D.t.u.E.T.t.q.L.L.n.i.A.h.i.j.j.i.g.f.e.n.B.I.z.m.k.r.y.p.t.x.v.q.j.i.i.p.n.l.i.i.l.o.p.u.k.z.Q.I.D.C.t.r.l.r.L.Y.T.G.y.y.C.C.z.s.q.v.A.v.u.s.q.q.q.r.s.r.r.r.q.p.p.o.o.k.q.r.r.n.l.r.F.m.o.p.o.l.k.m.o.s.r.q.o.n.l.k.i.n.n.p.r.r.s.t.u.G.U.Y.J.y.x.y.w.n.o.v.I.O.I.B.y.q.q.q.p.s.A.K.S.V.H.s.s.C.G.z.q.x.t.y",
".A.C.A.w.w.z.w.r.F.y.s.v.I.R.L.A.r.q.q.t.v.u.q.n.y.q.o.r.p.k.s.G.L.u.A.C.h.u.T.M.v.t.r.u.w.A.I.Q.o.e.f.v.C.o.g.j.g.i.i.h.g.h.i.j.B.B.u.k.m.w.x.r.v.v.u.u.r.t.w.y.r.p.k.i.i.l.o.r.r.q.T.1.u.m.B.v.p.l.j.t.E.I.B.r.u.E.L.H.v.o.s.y.u.t.s.r.q.p.p.p.s.r.r.q.q.p.o.o.i.t.r.l.j.h.q.O.r.q.o.m.l.m.n.n.n.m.m.l.l.k.j.i.l.m.n.p.r.r.t.t.F.w.y.H.D.o.m.w.s.z.H.N.Q.P.O.O.p.n.k.m.y.M.U.T.F.B.v.q.o.p.v.A.s.m.v",
".u.t.v.z.y.u.r.t.E.v.u.G.T.T.H.w.q.u.w.v.o.n.r.v.v.k.k.r.s.m.r.D.F.y.v.o.h.o.D.M.z.v.p.q.z.J.J.E.c.v.y.j.n.G.C.g.h.h.i.g.h.i.n.s.F.s.i.j.r.u.t.q.z.w.u.u.u.w.w.v.q.o.m.l.l.m.p.p.z.H.P.H.t.q.w.z.y.A.v.m.n.t.s.l.w.B.F.C.x.t.t.u.t.t.s.s.q.p.n.m.s.s.s.r.q.q.p.p.n.x.q.m.s.n.l.E.z.u.n.l.o.r.q.p.l.l.l.l.k.j.i.i.l.l.m.o.q.q.r.s.n.c.e.t.y.l.i.r.x.K.Q.I.C.H.Q.S.o.o.o.r.E.Q.Q.I.t.w.w.p.k.j.t.A.G.u.x",
".u.p.q.v.u.p.t.E.s.v.H.V.U.H.A.B.p.y.E.y.o.k.r.A.f.o.r.k.o.x.y.p.j.z.q.j.v.p.i.w.B.z.u.v.H.P.D.g.o.w.r.e.e.r.w.q.i.j.h.g.h.l.u.z.y.k.h.s.t.i.k.w.x.v.u.w.w.s.k.d.n.n.o.p.o.o.n.n.P.V.u.g.A.G.u.v.v.I.I.t.l.u.w.p.x.u.s.t.w.y.t.p.s.s.t.s.r.o.m.k.t.t.s.r.r.q.q.p.s.A.r.s.F.v.g.l.E.w.o.l.q.t.u.r.n.n.n.l.k.i.h.h.l.l.m.n.q.r.r.r.p.q.o.m.p.u.s.j.z.P.S.z.o.v.G.L.q.t.t.x.H.P.H.v.v.r.o.o.q.s.r.r.2.J.D",
".s.p.m.o.v.x.u.p.n.I.0.U.z.m.p.A.H.x.n.j.n.r.r.o.c.f.m.s.w.u.m.h.u.h.o.K.I.l.f.n.A.C.P.X.I.i.h.y.y.m.i.k.h.e.q.L.p.o.i.e.k.x.E.D.i.k.o.p.r.r.s.r.p.s.v.t.p.m.m.m.u.g.h.t.r.g.q.N.O.n.d.o.y.s.o.r.A.i.g.v.B.o.l.t.p.q.r.s.w.y.u.o.r.r.r.r.r.r.r.r.f.n.r.n.k.p.t.t.r.o.f.p.M.D.l.m.k.W.M.k.v.C.r.s.x.D.q.i.t.p.g.s.n.n.n.n.n.n.p.p.o.p.q.l.h.l.w.H.W.J.y.w.x.q.k.k.r.A.A.q.j.m.o.l.p.o.m.i.j.r.E.N.x.H.N",
".x.v.s.u.v.v.u.s.I.M.O.G.u.q.t.B.E.w.n.j.l.p.r.q.n.l.j.m.o.o.m.k.i.m.t.C.A.q.j.l.n.t.C.E.v.m.n.u.p.l.l.n.j.d.f.o.s.m.h.g.p.x.q.f.l.l.n.n.o.q.s.t.r.s.t.q.n.l.n.p.e.q.r.i.k.z.G.C.u.s.q.p.n.m.v.D.p.x.K.Q.G.p.l.s.u.q.p.r.w.x.y.A.s.s.s.s.s.s.s.s.p.u.u.p.o.t.x.x.s.t.k.j.B.N.G.x.w.i.0##.k.y.9.C.x.O.G.r.s.r.l.p.m.m.n.n.n.n.o.o.q.t.t.r.p.t.D.M.H.B.x.y.v.o.k.i.n.A.I.D.t.p.t.y.n.k.k.o.y.G.I.G.q.x.E",
".x.y.y.x.t.u.x.B.Z.N.z.q.p.u.w.y.v.u.q.q.q.q.o.m.r.o.k.k.l.m.n.n.d.q.t.m.l.s.u.o.k.r.t.m.l.s.t.q.m.m.n.m.j.g.c.a.s.u.w.z.B.z.o.c.o.n.m.l.m.p.s.u.u.t.r.n.l.l.n.q.h.r.q.h.u.R.M.m.h.u.A.s.l.p.w.z.j.G.U.J.p.g.k.q.z.q.o.t.v.v.z.G.t.t.t.t.t.t.t.t.u.x.x.v.u.u.t.r.z.E.A.j.l.L.T.x.u.i.p.F.z.s.s.r.i.s.w.v.v.k.d.j.m.m.n.n.n.n.o.o.s.u.x.v.r.t.B.G.y.w.v.v.p.k.l.p.E.F.K.O.J.y.r.r.o.k.m.z.O.S.H.v.q.r.y",
".t.w.x.u.q.t.D.K.T.G.q.l.p.v.u.s.o.s.y.A.z.t.m.h.o.o.p.q.p.p.o.m.j.p.k.d.f.v.D.A.x.w.r.k.m.t.v.q.r.r.p.k.k.l.j.e.h.v.I.H.w.n.n.q.n.m.m.m.n.p.r.s.r.q.p.n.m.m.o.p.y.k.i.w.N.M.w.h.i.p.v.v.u.v.q.j.A.N.G.j.f.o.B.z.x.q.p.w.x.r.t.B.v.v.v.v.v.v.v.v.q.u.x.y.x.u.o.j.t.D.N.x.g.y.L.u.R##.u.n#b.P.g.S.y.s.y.R.U.z.o.w.n.n.o.n.n.o.p.p.p.s.u.s.o.l.o.r.y.w.u.o.i.j.t.D.U.D.y.M.Y.N.t.f.p.r.w.H.Q.O.C.o.w.r.w",
".s.u.u.r.q.u.C.K.y.t.o.p.s.t.q.m.u.w.y.A.z.u.p.m.u.u.u.t.q.o.l.k.n.g.d.f.p.x.B.C.F.u.l.m.p.n.q.s.p.q.o.k.k.m.k.e.e.l.t.q.g.b.e.o.k.l.n.p.q.q.p.p.n.n.o.p.p.p.n.m.u.l.s.M.O.v.k.q.l.l.o.v.v.q.o.s.R.L.t.j.t.S.T.C.o.l.p.z.z.s.q.u.w.w.w.w.w.w.w.w.q.s.v.y.y.x.x.x.g.r.N.N.o.n.A.x.r.y.s.m.p.k.j.z.H.t.o.u.w.u.l.g.n.n.o.n.n.o.p.p.n.q.t.t.n.l.j.k.r.t.s.q.o.v.G.O.J.t.p.H.V.P.z.o.o.v.D.I.I.C.w.s.w.q.t",
".t.t.t.u.w.y.z.y.i.n.s.u.u.s.q.q.D.y.t.q.s.w.z.A.E.C.y.r.l.i.i.j.h.c.e.u.D.w.r.s.x.o.j.n.m.i.k.r.h.i.k.k.j.g.e.c.u.l.f.g.k.k.k.k.k.l.o.q.r.r.p.o.l.l.n.p.r.r.o.m.h.w.N.K.t.h.n.A.m.l.p.r.m.j.w.O.C.u.l.r.N.Z.J.l.g.l.s.x.z.w.t.s.v.v.v.v.v.v.v.v.w.v.v.w.w.z.J.U.p.s.I.R.C.o.n.q.o.g.t.v.f.l.w.g.Q.P.H.o.m.M.S.r.m.n.o.o.o.o.p.o.n.p.t.v.u.r.r.q.e.m.r.y.E.N.P.N.x.x.z.C.B.w.w.y.r.x.C.C.y.v.w.y.o.m.p",
".p.q.u.E.L.K.z.o.g.n.u.v.s.r.x.C.D.v.n.m.t.A.E.F.y.y.y.u.n.i.h.h.e.e.m.B.B.n.i.j.m.s.s.n.i.j.l.m.j.f.g.l.j.d.h.r.G.q.e.j.s.u.r.p.n.o.n.o.p.q.p.p.o.m.l.m.p.r.p.n.q.F.J.u.g.i.q.t.p.s.q.n.n.v.F.L.i.i.p.G.N.B.l.f.q.y.A.u.r.y.y.t.u.u.u.u.u.u.u.u.y.x.y.x.s.r.D.R.T.I.z.F.S.K.r.m.t.q.n.m.p.p.m.i.o.s.A.s.i.v.J.x.m.m.o.o.o.o.o.p.n.o.s.v.v.t.t.s.j.m.r.y.K.S.N.C.A.H.G.v.k.j.r.z.D.y.v.t.t.u.s.t.m.o.s",
".k.l.v.M.0.W.D.l.o.s.v.t.p.r.D.N.t.n.k.o.y.F.F.B.j.p.w.z.u.o.g.e.g.m.u.w.o.f.f.i.k.D.H.r.j.n.q.h.q.f.d.h.g.f.t.O.p.j.f.k.o.n.o.s.q.p.m.m.m.p.q.r.q.n.j.j.m.q.q.p.X.G.m.h.m.p.l.h.v.w.r.k.y.R.L.p.m.p.G.T.H.h.g.v.E.N.I.q.k.v.A.t.v.v.v.v.v.v.v.v.r.v.A.A.p.h.j.u.9.P.m.p.2#..K.v.u.s.D.C.j.g.z.P.m.h.I.3.A.h.r.G.m.m.o.o.o.o.o.p.m.n.q.r.q.p.n.m.w.t.q.t.H.Q.J.u.A.E.z.k.g.p.z.C.P.C.q.m.s.w.p.i.s.w.y",
".x.O.0.R.x.o.z.O.z.C.t.i.t.O.K.m.o.s.n.w.M.v.f.s.A.l.m.E.E.o.h.q.#.l.v.k.c.j.n.j.o.k.j.o.n.h.e.g.x.a.m.w.f.x.U.q.a.u.n.c.p.r.h.h.i.l.n.n.o.q.p.m.j.h.m.p.l.i.u.P.O.n.c.i.r.m.l.q.n.q.f.t.1.B.c.x.m.M.S.u.f.r.z.r.q.f.e.g.g.m.s.t.p.s.w.v.t.t.v.y.z.r.u.L.V.K.s.i.g.K.0.z.k.J.Z.w.l.r.u.n.o.A.Q.Y.J.G.m.o.J.s.g.q.9.F.h.j.p.o.p.u.p.p.q.r.s.t.v.u.v.u.u.v.w.z.C.D.B.o.k.q.s.w.M.7.Y.N.y.q.p.q.l.h.t.t.l",
".P.Q.M.B.q.q.C.O.L.w.n.r.t.r.q.s.k.y.o.d.n.u.n.o.r.r.t.v.t.o.h.f.k.b.a.i.p.m.j.m.l.g.f.j.l.h.h.k.c.v.u.g.s.K.A.h.l.o.k.n.u.h.e.x.t.i.j.v.w.k.h.q.k.m.l.k.n.w.G.L.p.m.n.o.p.m.o.r.n.i.F.U.z.i.h.e.L.F.t.i.l.s.t.n.q.D.g.h.S.G.h.z.y.z.z.y.w.w.x.y.u.k.g.n.w.z.y.w.v.z.U.0.x.i.C#b.y.w.k.f.F.6.V.p.N.P.C.n.v.L.E.l.l.C.D.m.i.s.q.a.q.q.q.s.r.r.s.r.v.u.u.u.u.v.w.w.n.q.n.l.B.X.0.R.K.S.Y.P.z.q.s.A.s.v.p",
".V.K.x.q.r.y.D.H.z.p.o.y.u.h.j.y.n.x.m.d.i.s.t.u.m.y.x.j.i.u.w.m.r.E.C.j.b.l.p.j.e.c.f.j.l.j.k.m.r.x.t.w.H.w.g.k.q.v.y.H.K.y.x.Q.B.t.q.r.o.h.i.o.l.q.p.n.w.L.J.v.c.n.u.r.p.r.q.j.h.J.Q.D.u.i.i.C.R.t.g.k.r.q.n.q.u.C.m.s.V.A.c.j.w.v.u.v.x.y.y.x.F.z.p.j.m.u.A.A.q.f.n.P.T.q.k.G.W.y.m.F.X.P.t.g.D.D.A.k.i.I.T.E.e.D.1.R.w.n.r.v.r.r.r.r.q.q.p.p.u.u.u.t.s.q.p.o.m.p.r.y.T.9.0.F.v.K.W.P.y.n.t.E.r.A.u",
".H.z.s.u.B.F.B.v.m.x.F.C.t.p.q.s.z.s.q.y.C.r.q.A.r.y.s.h.l.H.R.L.F.Q.L.l.c.g.m.e.a.e.m.n.l.j.n.r.2.u.E.V.v.e.m.n.t.J.N.G.H.M.K.D.J.X.S.u.k.v.z.o.m.p.t.x.F.I.w.h.j.n.p.q.s.v.p.i.y.1.I.g.o.u.y.Y.t.i.k.u.u.l.l.v.v.k.E.R.u.k.o.k.r.p.p.r.t.t.o.k.D.J.G.x.u.z.z.t.r.y.l.n.X##.C.e.F.D.P.2.N.k.e.r.q.l.i.l.m.j.y.Z.y.m.t.Q.M.j.i.D.p.p.q.q.r.r.r.s.t.u.u.u.s.q.n.m.B.v.E.Y.6.U.I.E.B.z.x.v.w.x.v.u.r.D.A",
".v.u.u.y.D.C.v.o.D.L.P.H.v.q.q.t.H.x.B.K.E.q.n.p.o.n.k.p.C.N.S.S.2.s.g.r.x.f.a.h.c.k.p.n.j.m.x.I.Q.K.R.I.i.k.x.l.K.K.E.s.m.s.q.g.q.M.R.x.k.s.r.h.t.o.q.z.C.t.k.h.p.m.o.r.r.p.r.y.1.A.w.n.f.D.W.u.e.j.p.u.s.m.m.s.p.r.W.Z.l.h.I.T.o.n.o.r.t.r.l.g.h.u.D.B.A.D.z.r.t.N.v.f.w.U.H.o.c.L.2.I.n.p.u.s.i.i.b.q.L.o.i.O.7.t.f.r.T.S.z.m.n.o.p.q.s.u.v.w.s.t.u.u.u.s.q.p.F.L.Y.9.Y.D.x.G.H.x.q.s.C.H.C.u.u.F.G",
".w.y.z.y.v.u.u.v.U.G.B.F.C.s.w.N.E.H.M.D.p.s.r.a.g.e.m.E.O.K.E.E.V.D.t.y.v.h.b.h.i.m.n.i.j.w.L.V.n.L.A.t.J.v.h.G.0.z.o.r.l.h.l.n.h.m.A.L.G.p.h.l.u.m.k.r.q.i.l.w.m.o.t.t.n.j.z.S.D.j.i.s.A.L.D.e.p.o.m.k.n.s.s.m.n.O.Q.y.l.g.r.V.d.f.l.q.w.A.A.z.h.o.w.z.y.z.y.u.v.s.r.y.z.l.k.A.w.D.z.p.p.w.s.g.b.m.k.A.7.Z.w.w.J.X.N.o.A.5.Y.p.q.r.s.s.t.w.x.x.t.t.u.v.v.v.v.u.u.T.7.X.I.E.E.B.r.s.u.y.A.C.B.A.D.G.G",
".w.z.z.v.r.s.B.J.L.x.q.y.G.H.K.P.x.B.M.G.r.w.w.c.e.l.A.O.P.G.y.y.y.4##.H.k.l.o.h.m.n.n.n.x.J.M.J.l.w.j.x.6.L.s.2.L.n.p.z.s.r.A.A.x.q.x.S.X.J.x.y.r.p.o.n.l.l.r.A.p.r.r.p.r.A.I.O.g.k.j.A.T.D.f.j.u.o.j.j.m.s.s.r.E.L.w.i.k.w.v.r.b.d.h.k.p.w.D.I.x.w.x.z.y.u.u.x.D.p.F.4.S.q.l.v.0.t.g.p.w.i.a.d.f.l.G.R.Q.1.X.s.g.M.2.B.j.r.J.N.x.x.x.v.u.u.s.s.u.u.u.u.v.w.x.x.w.J.P.I.G.K.H.x.j.s.B.A.w.x.H.T.N.E.z",
".q.v.z.x.u.y.J.U.v.B.x.q.C.W.P.q.t.j.E.Y.L.z.z.s.j.y.O.S.L.E.F.G.x.S.Z.M.w.t.s.o.m.n.q.y.N.T.E.m.s.s.w.A.Q#d#e.U.g.i.y.B.p.z.C.h.u.n.i.l.y.D.u.h.j.u.y.r.n.u.u.p.A.w.m.h.B.W.P.q.D.f.G.5.v.f.p.l.l.k.n.s.p.k.r.E.1.o.k.m.o.4#..e.q.o.k.e.c.f.m.t.B.u.u.A.A.t.t.A.m.y.W.R.p.j.t.n.L.z.p.k.h.e.e.i.A.o.3.8.m.r.W.t.q.p.I.0.D.f.v#h.E.D.A.x.t.r.o.n.w.v.u.t.u.v.x.y.J.s.o.D.Q.K.z.u.C.F.I.E.D.L.4#f.U.D.u",
".B.E.x.p.B.V.Q.w.p.E.z.y.H.z.n.v.q.x.v.E.V.K.s.B.q.w.q.w.P.I.w.H.x.K.t.k.C.B.n.u.p.k.y.X.W.z.r.A.C.z.t.q.B.P.N.B.s.r.r.s.s.t.t.t.h.q.u.i.m.S.2.I.s.x.z.v.q.r.v.y#e.V.A.E.U.M.p.i.n.N.T.w.k.t.u.g.i.i.h.k.n.t.A.F.p.y.a.f.3.V.h.h.n.n.m.j.h.j.m.s.A.j.s.C.r.y.F.p.o.1.Q.h.j.u.E.W.a.i.x.u.j.c.i.r.z.5.K.j.s.s.u.Y.m.l.k.x.N.E.n.k.F.F.n.j.y.m.f.A.t.u.v.w.w.y.y.z.z.C.C.v.m.j.n.t.D.L.G.y.P#c#..M.P.3.W",
".G.w.t.B.I.G.B.A.y.C.q.j.t.t.r.C.F.z.o.x.P.B.m.x.p.C.z.x.G.C.w.I.N.e.i.J.z.r.A.z.l.P.8.X.G.B.A.w.t.x.y.t.r.z.J.P.s.t.t.t.t.t.u.u.n.m.s.u.n.q.G.S.p.B.f.i.0.O.w#..M.X.5.7.R.l.g.w.O.K.y.l.j.p.q.j.p.p.p.q.p.q.q.r.j.C.A.o.t.z.t.o.m.l.l.j.j.j.m.o.i.H.A.n.z.z.s.F.1.S.t.k.B.I.K.V.a.g.l.j.c.e.p.A.X.X.y.m.w.p.g.r.w.u.f.g.N.6.I.h.F.Z.0.z.n.E.D.e.t.u.v.x.y.z.A.B.D.E.C.w.q.p.v.B.D.p.u.U#a.2.L.C.I.Y.V",
".y.t.v.D.E.y.x.C.w.v.j.h.s.B.E.P.E.Q.J.u.p.q.A.S.t.9.8.u.g.w.I.K.w.i.w.G.u.D.M.u.K#b#h.R.x.C.H.z.y.z.B.y.q.l.r.A.v.u.u.u.u.u.t.t.x.s.t.x.r.k.t.K.7.L.t.C.X.5.4.8.p.D.Q.T.C.j.k.S.0.y.f.i.s.r.m.m.s.t.u.u.t.o.k.h.n.A.U.I.g.g.E.L.p.m.j.i.k.k.m.l.n.B.u.s.B.s.v.2.T.z.i.k.J.Y.J.q.n.j.e.c.f.q.E.N.U.C.n.r.v.k.d.g.r.C.u.k.y.P.R.P.l.D.3.P.g.p.D.f.t.u.w.x.B.C.E.F.G.F.D.z.x.z.D.H.B.u.K#a#c.M.x.C.C.O.O",
".y.J.M.y.r.y.y.s.w.x.u.y.H.K.J.K.A.X.T.o.j.K.4.7.5#..S.o.e.o.y.z.e.K.J.u.H.K.D.N#g.5.P.J.E.B.E.I.E.w.w.D.A.o.d.d.v.v.v.u.u.t.t.t.A.C.w.p.w.B.v.p.7.T.O.A.n.N.Z.s.D.B.u.A.V.Y.U.Y.D.j.e.r.A.u.o.r.p.r.s.u.t.r.n.l.s.n.E.R.u.h.w.U.y.r.l.i.i.l.m.k.v.j.s.v.m.G.4.T.o.h.o.N.V.M.u.f.A.r.h.g.q.E.N.R.x.l.s.v.g.d.l.m.p.y.B.x.p.i.F#i.K.k.x.Y.O.s.s.A.t.v.w.y.C.E.G.H.E.D.C.B.B.C.D.E.z.Y#f.9.L.B.G.N.E.C.B",
".S.Z.X.F.t.u.t.q.J.H.L.O.L.G.w.n.J.D.u.u.O#c#a.P#u.F.g.r.C.n.e.g.q.H.F.H.P.z.G#l#d.N.w.E.I.B.A.J.y.s.s.A.A.o.g.h.v.v.v.u.u.t.t.s.v.D.u.o.B.E.p.k.q.1.W.y.D.E.t.x.A.B.r.x.2#g.Y.y.g.j.s.x.x.t.s.v.q.q.q.r.s.s.r.r.o.h.h.z.O.v.l.x.C.x.p.j.i.j.k.l.m.o.v.n.q.6#b.q.m.k.M#d.V.k.i.D.C.t.l.n.x.G.H.F.q.n.y.s.#.d.u.l.y.v.r.w.z.o.q.N#b.C.g.z.5.P.l.p.u.v.y.A.C.E.G.H.B.C.D.E.D.C.A.z.C.4#c.P.t.y.M.S.K.w.s",
".Z.N.M.S.M.y.y.L.H.E.M.Q.F.z.w.j.B.h.p.Z#..1.O.G.Z.O.M.N.G.E.y.i.G.x.M.P.v.L#f#j.G.G.H.H.G.E.D.C.A.A.z.v.l.f.m.w.w.w.v.v.v.v.v.v.t.C.x.A.J.s.k.H.l.z.Z.9.T.y.w.D.h.x.v.m.y.K.I.C.k.u.z.u.n.o.u.w.t.s.q.p.p.p.r.s.l.q.g.j.M.Q.r.g.z.x.t.n.j.i.k.m.m.r.l.C.8.W.r.q.p.S.4.H.j.g.q.B.v.q.l.o.v.x.r.l.q.k.o.j.c.u.C.j.r.C.A.x.D.A.m.i.G.2.E.i.D.Q.A.s.w.x.y.A.C.D.F.F.z.C.F.F.D.A.z.z.I.G.D.C.C.E.H.I.L.E.z",
".K.y.A.Q.V.K.I.S.y.w.L.R.E.D.H.x.m.l.Q#f.0.F.F.I.r.3#s.7.A.K.V.y.H.F.O.L.H#d#o.R.v.J.Q.J.E.G.F.B.G.G.z.n.g.j.s.x.x.x.y.y.y.y.y.y.G.V.L.F.S.K.D.Z.B.f.H.6.u.k.B.j.s.C.z.r.r.v.I.4.I.C.t.p.q.s.r.r.w.u.s.r.q.p.p.r.p.q.r.o.q.I.M.p.m.r.v.t.l.g.h.k.o.l.q.X#b.u.e.A.H.7.N.g.g.w.t.m.u.q.o.n.p.o.i.d.m.e.f.k.p.A.C.r.l.F.K.E.B.w.u.A.k.X.7.B.n.F.V.W.y.z.z.A.B.B.C.C.C.G.I.G.C.A.E.I.L.t.k.w.K.L.C.y.C.W.T",
".x.A.D.G.N.R.J.x.z.z.T.1.K.J.O.C.q.F.6.6.K.P.V.D.I.F.2#..K.x.E.C.L.L.z.Q#t#o.P.F.M.K.J.K.K.J.G.F.D.y.n.i.r.G.F.u.z.z.A.A.B.B.B.C.Y#j.Z.A.1#i.8.3.M.G.y.r.u.y.y.u.z.s.o.y.D.q.p.K.Z.B.j.n.y.z.p.h.t.t.s.s.r.q.q.p.u.b.u.A.b.k.V.O.d.l.v.w.o.h.f.h.e.p.U.X.q.p.B.p#l.O.i.t.3.J.h.x.y.v.r.p.m.j.h.f.m.g.j.s.v.o.q.w.z.w.v.F.H.v.E#..q.k.C.M.o.k.G.N.A.A.A.A.A.A.A.A.E.I.J.F.z.A.I.Q.K.A.r.v.G.M.F.u.t###.",
".R.T.E.z.P.K.u.w.m.y.o.q.S.S.A.A.w.z.w.H.T.G.A.0.y.L.p.d.y.L.D.H.G.x.R#n#m.R.z.K.y.J.P.J.G.L.O.I.x.q.k.n.x.F.F.C.C.A.N.x.s.B.A.7.8.t.k.z.o.i.E.W#..R.A.w.x.v.t.v.s.s.t.s.s.s.s.u.S.1.S.v.l.s.v.n.r.l.m.p.p.k.i.n.o.p.q.p.n.o.u.A.p.y.p.i.o.f.a.o.f.7.U.f.k.u.B.6.X.u.t.2#p.9.E.o.n.s.s.o.o.s.n.g.b.h.r.w.v.s.q.p.p.t.x.x.y.I.3#h#c.X.q.f.s.k.a.n.3.L.E.G.E.B.C.B.A.B.C.C.C.D.G.I.v.z.A.w.s.s.z.G.4.1.X",
".K.N.y.l.t.v.y.N.F.n.m.z.A.v.A.G.n.Z.2.G.D.J.K.R.X.i.k.H.C.B.N.K.B.1#i#b.W.O.J.E.K.J.H.F.H.M.I.D.i.n.t.A.E.H.F.D.F.A.v.Y.y.y#r.1.p.j.s.u.g.k.y.u.V.J.y.x.y.x.v.w.t.t.s.t.s.s.t.s.z.N.W.N.t.k.p.w.n.n.r.u.u.p.k.j.o.v.s.l.t.H.x.d.l.H.x.c.c.e.d.i.J.D.p.l.u.u.w.K.w.Q.1.V.P.U.R.H.k.m.m.i.c.c.h.p.h.m.r.v.u.t.s.t.t.s.q.v.O.5.0.K.S.Y.C.g.q.z.q.m.s#j.5.o.I.Y.G.F.H.E.B.D.H.I.G.D.y.M.W.Q.B.y.N.5.3.J.F",
".H.M.D.s.v.y.J.3.V.s.u.x.h.q.R.U.I#d.9.A.p.C.G.E.F.l.x.K.B.I.S.D.W#l#n.T.A.L.P.F.Q.J.E.I.M.I.y.t.g.r.B.H.G.D.E.H.M.z.S.O.E#b#f.A.p.A.J.D.A.S.U.w.C.y.w.z.A.x.w.x.s.t.t.t.t.s.s.s.o.A.Q.W.J.p.k.t.p.r.t.s.r.r.o.l.u.i.n.G.L.v.g.e.f.w.t.f.g.j.q.D.X.l.h.x.x.s.r.j.n.W.8.I.r.G.2.6.H.z.s.p.m.l.p.w.p.q.t.t.t.u.w.x.x.q.t.K.4.5.M.r.y.T.Q.B.y.u.o.o.r.U.F.q.J.G.s.K.M.H.E.F.K.L.F.A.z.K.R.M.B.y.K.Y.V.B.y",
".v.A.D.I.O.P.M.M.P.N.H.n.h.L#b.3#k.6.I.u.s.o.m.r.d.D.E.z.M.J.C.Q#o#c.U.E.C.I.O.M.N.H.J.Q.O.y.p.n.v.A.E.C.A.z.F.L.H.F.V.L###g.n.l.H.D.x.z.P.7.Y.y.u.v.y.z.y.y.w.y.t.t.s.t.t.s.s.s.u.t.z.N.S.H.r.i.r.u.q.j.i.o.r.n.m.j.y.P.F.f.c.t.j.d.h.w.y.n.u.U.r.l.v.B.s.p.u.q.P.K.F.z.u.w.L.1.U.I.t.l.n.s.o.h.o.o.p.r.s.t.v.x.u.u.K.6.3.E.u.A.w.B.H.J.w.j.s.U.4.i.h.Q.M.z.G.P.G.H.H.G.E.C.C.C.I.C.w.x.B.E.B.x.D.H.J",
".m.k.t.L.X.V.C.i.x.S.I.r.I.2.0.X#o.T.y.D.z.n.i.n.m.y.C.L.S.w.A#l#h.V.B.D.J.J.J.N.K.K.N.O.B.n.m.x.E.D.B.z.y.B.I.K.u.O.z#g#s.q.i.C.M.z.v.O.6.Z.I.C.w.z.B.y.w.w.w.w.u.t.u.s.t.s.r.r.u.s.s.y.K.Q.F.n.p.p.n.i.i.l.n.l.g.G.Q.t.g.i.n.g.o.c.g.B.A.n.n.z.e.u.z.u.r.p.z.X#..F.l.s.y.s.o.w.X.W.I.n.k.y.C.s.m.n.n.p.s.t.u.u.v.L.3.3.K.r.s.F.w.g.g.t.v.z.S#b.O.k.r.G.y.H.S.A.B.H.L.G.w.t.A.J.Q.F.v.y.J.O.G.w.s.G.M",
".z.r.A.P.V.W.I.k.n.C.y.J#..U.v.G.R.K.R.R.C.v.v.s.H.x.O.U.A.K#b#h.J.L.M.J.K.M.L.H.N.M.J.z.n.j.u.J.B.y.y.A.E.I.H.F.A.V#e#d.E.n.H.K.B.H.U#c#d.K.s.F.z.C.C.x.u.v.v.s.u.t.t.s.s.s.s.q.k.s.t.q.u.F.L.J.n.l.m.q.q.k.i.k.z.F.z.h.e.j.i.#.b.c.k.m.j.s.u.k.G.H.w.q.t.n.w.4.I.y.o.o.s.v.r.n.N.1.0.E.o.u.L.V.q.p.p.q.t.t.t.t.D.0.3.G.s.B.C.n.m.e.j.A.R.5.Z.C.g.z.B.s.z.C.C.K.I.L.K.C.u.u.D.N.D.C.C.D.F.G.I.I.x.u.D",
".P.K.U.W.H.J.R.L.t.o.v.R.2.L.v.x.r.B.V.V.y.v.D.x.G.I.S.K.G#b#n.O.x.I.P.O.K.M.J.G.N.I.x.n.l.t.D.G.z.y.y.A.E.G.E.C.V.8#x.B.f.Q.I.E.v.W.5.Y.Q.v.k.x.w.A.A.w.w.y.v.o.v.u.t.s.s.s.r.r.m.p.s.q.m.p.E.U.z.n.l.s.r.j.k.w.M.o.e.j.l.c.#.e...a.j.g.e.x.J.t.1.N.F.w.o.z.K.E.d.n.v.r.s.y.z.u.q.A.M.M.x.l.s.J.D.z.u.r.t.t.u.u.W.V.J.t.u.C.v.e.l.l.J.2.3.1.O.l.x.D.G.K.L.H.R#f.3.S.E.x.z.F.J.L.s.y.D.C.y.x.D.J.F.x.z",
".J.N.3.U.m.n.O.0.B.p.C.M.C.L.S.x.w.o.F.P.t.o.z.z.A.I.y.H#k#k.O.B.P.F.E.M.Q.J.G.N.I.D.r.k.u.H.G.u.E.D.A.y.x.z.A.B.8#q.z.l.Q.G.A.I.R#l#..z.w.D.A.B.r.w.y.w.y.B.v.m.v.u.t.s.s.s.q.r.y.m.k.t.p.i.s.N.O.s.h.n.n.f.p.L.t.n.f.d.f.h.g.f.o.b.i.o.e.q.O.R.F.F.U.C.h.X#f.n.b.k.x.D.y.r.s.w.r.m.D.6.2.w.j.o.O.H.x.t.s.v.x.w#a.G.p.A.E.q.g.l.C.t.S#a.O.w.M.3.S.n.z.G.l.N#l.8#k.Y.y.t.G.S.P.G.B.D.F.F.D.D.F.H.I.K.E",
".G.D.C.F.K.I.A.s.K.x.u.J.V.Q.H.E.z.l.a.e.o.w.A.E.F.v.E.2.8.P.B.E.Q.H.D.H.K.H.I.N.y.m.l.y.G.B.x.x.t.x.x.w.t.q.G.8#f.h.o.2.N.y.K.L#j.X.C.z.z.u.v.B.y.x.x.w.x.v.w.u.x.w.w.v.v.u.u.t.v.n.l.r.t.o.m.q.r.h.k.k.g.x.J.r.g.g.f.f.e.f.e.e.l.h.j.m.h.b.i.y.w.w.W.U.2.9.v.h.q.i.i.q.w.s.p.r.n.s.u.x.O.Y.G.h.H.5.M.l.s.w.r.A.s.u.r.H.C.j.n.q.o.5#..I.v.s.E.1.u.C.B.x.P#a.1.x.y.5.2.G.Q.2.R.K.F.I.J.G.A.y.A.E.H.G.B",
".F.C.z.A.B.A.v.r.z.N.X.Q.F.A.J.U.B.q.h.m.v.z.C.D.x.K.X.W.J.y.B.I.C.M.L.B.D.O.M.y.n.o.v.E.I.D.y.x.G.f.z.M.l.z#..4.q.C.C.F.L.H.R#n.3.L.z.B.B.x.x.C.y.y.x.w.w.v.v.u.x.w.w.v.v.u.u.t.q.o.p.t.s.m.m.q.p.h.p.u.p.w.y.h.h.i.h.g.h.g.f.f.d.h.k.k.n.q.s.r.Y.5.4.5.1.Q.F.p.s.p.n.u.C.F.z.s.v.x.u.t.E.T.W.N.f.u.1.W.o.s.K.x.r.q.g.r.q.f.r.w.1.7.N.s.x.y.s.w.C.p.t.U#..0.H.y.s.S.6.8.5.I.s.D.D.J.N.M.G.E.G.K.C.E.A",
".F.C.y.u.s.s.t.u.v.V.6.M.o.p.F.S.u.r.q.t.w.x.z.B.A.X.2.H.o.s.C.G.x.K.N.F.E.M.E.o.m.x.H.H.D.B.A.y.D.s.G.E.r.V.8.u.l.y.O.G.w.X#i.9.J.B.y.D.D.x.w.A.y.x.y.w.x.v.w.u.x.w.w.v.v.u.u.t.r.s.u.v.q.j.j.n.J.z.D.J.C.w.r.h.k.k.k.j.i.i.j.j.j.q.q.l.q.z.v.i.s.F.o.E.F.v.X.U.r.W.1.t.e.v.E.t.x.v.u.r.u.H.X#..w.h.H#g.R.o.r.A.x.t.e.k.n.o.K.T#c.W.t.l.z.z.l.e.z.s.I#.#..I.u.B.x.L.V.5.2.D.p.z.S.P.N.N.O.M.G.B.E.H.E",
".F.E.A.u.q.r.x.C.F.L.J.x.p.q.u.t.k.o.t.t.q.q.v.C.O.S.I.q.q.D.E.v.E.B.F.N.L.z.q.p.C.M.N.C.v.x.B.y.w.D.r.y.0.V.q.h.I.r.B.L.Q#k#k.w.D.z.A.D.C.u.t.w.y.x.x.x.w.v.v.u.x.w.w.v.v.u.u.t.x.x.w.u.q.j.h.g.D.q.n.r.o.h.h.j.m.m.m.m.m.m.l.l.n.u.w.r.q.x.A.x.G.E.o.w.v.r.T.6.P.3.W.r.b.f.t.z.s.q.s.u.r.p.E.V.2.I.m.E#..Q.n.z.E.C.o.u.A.H.8#c.L.B.s.u.x.q.k.n.s.R#..3.I.w.z.E.K.I.z.v.H.Q.N.F.W.L.A.F.Q.V.M.z.E.J.H",
".F.F.D.y.u.w.E.L.O.z.p.u.A.v.l.g.j.s.A.y.q.q.z.J.R.E.p.o.z.H.C.t.E.B.E.J.H.w.r.t.I.N.L.A.w.B.A.v.z.q.n.S#a.t.c.G.w.F.u.M#s#n.L.C.C.B.C.D.z.u.u.x.z.x.y.w.w.w.w.v.x.w.w.v.v.u.u.t.y.v.q.p.s.q.l.e.l.e.d.f.l.j.m.x.p.p.p.o.n.n.n.n.j.l.s.u.q.r.H.1.0.G.H.w.u.C.E.0#f.A.h.s.A.k.g.w.r.o.r.w.s.k.o.A.Q#c.H.h.E#g.2.B.B.G.v.B.G.M.5.4.y.t.x.A.n.f.n.C.K.5.7.H.o.u.F.G.L.A.p.m.z.R.Y.Q.x.t.s.y.I.P.T.T.B.F.B",
".D.E.E.C.A.B.G.K.L.y.u.E.F.s.m.p.t.C.I.G.B.D.J.M.E.u.p.v.z.x.y.D.w.H.G.s.r.F.I.A.E.F.G.D.E.E.z.s.B.F.Y.V.q.f.u.E.q.H.W###f.Q.x.N.C.B.B.A.y.y.y.z.y.x.x.w.x.v.v.u.x.w.w.v.v.u.u.t.u.q.l.k.s.w.r.i.n.r.n.p.y.t.p.y.q.p.p.p.o.p.o.o.m.n.t.w.o.j.B.2.F.m.z.n.o.E.s.L.T.I.y.v.y.v.l.b.t.q.r.x.u.p.r.x.E.Z#a.G.k.P#d.O.w.F.w.y.z.B.K.x.J.x.w.t.f.f.w.H.8.V.D.s.s.y.E.F.C.p.x.K.G.C.L.O.f.p.A.y.s.w.L.0.D.E.y",
".y.z.B.C.C.B.B.B.z.B.E.E.x.s.t.x.F.G.H.I.N.S.L.A.s.t.w.A.w.r.x.G.x.D.x.j.k.G.V.V.G.B.C.E.E.A.w.v.y.2.9.u.a.o.G.o.z.M#c#c.H.z.J.G.E.C.B.z.z.A.y.w.z.x.y.w.x.v.v.v.x.w.w.v.v.u.u.t.s.r.m.k.o.u.s.m.i.w.q.j.r.n.d.i.r.q.p.q.p.p.o.p.r.r.t.t.p.m.q.x.C.C.F.y.u.w.q.p.p.2#c.J.n.t.t.b.m.p.s.v.w.u.u.v.F.y.2#f.D.k.F.X.y.K.z.x.w.w.z.j.E.p.p.p.f.u.P.T.0.E.m.r.z.A.z.A.x.r.G.Q.E.B.D.v.j.s.z.z.s.r.y.H.G.D.u",
".u.v.y.B.B.z.v.s.q.B.F.u.o.u.x.t.J.E.A.F.S.X.H.l.s.v.y.y.u.v.x.A.D.s.j.k.o.A.X#h.O.E.A.A.y.r.t.B.B.V.r.b.B.w.f.z.w#c.0.w.N.Q.D.O.F.E.C.A.z.z.v.q.y.x.x.w.x.v.w.u.x.w.w.v.v.u.u.t.w.w.s.l.k.o.p.m.f.x.q.e.q.t.n.t.r.q.r.p.q.p.p.p.k.o.n.l.r.A.r.c.j.E.v.C.B.u.G.p.A.M.1.2.J.o.r.G.b.j.s.x.w.w.s.o.r.y.D.3##.v.i.S.H.T.G.B.A.B.G.o.g.c.m.t.m.H.9#..x.w.u.u.y.B.z.v.C.D.H.x.r.O.N.g.r.m.k.s.E.G.y.o.F.y.n",
".t.n.r.C.E.y.y.D.u.t.s.s.u.y.D.H.D.w.y.M.T.K.B.y.t.w.y.y.v.u.x.B.s.i.k.w.y.n.n.y.0.U.x.p.G.C.p.v.q.j.k.p.q.l.n.w.2.Q.C.y.C.J.J.G.E.E.D.C.A.z.y.y.x.x.w.w.w.v.v.v.t.o.r.A.y.m.m.w.g.n.M.s.f.z.t.q.s.s.s.r.s.r.r.s.o.o.n.o.n.n.m.n.o.n.n.p.q.r.t.s.d.h.r.A.E.C.y.u.t.r.B.U.0.L.t.l.h.p.h.i.x.m.h.F.u.w.s.z.2#k.W.j.h.W.R.m.J.B.c.3.k.m.n.t.N.2.P.r.q.u.y.y.x.s.r.r.D.v.r.v.z.w.t.v.B.r.o.C.M.E.q.j.d.s.m",
".u.v.t.q.n.q.s.r.v.u.t.t.x.B.E.G.r.E.S.U.G.s.r.y.v.t.r.t.w.x.v.s.k.j.p.C.E.u.p.s.Q.8.V.x.y.B.u.u.p.p.s.u.t.w.K.X.K.E.x.y.E.K.K.J.F.E.D.C.A.z.y.y.x.x.x.w.w.w.v.v.p.x.x.r.s.z.t.g.j.c.x.r.j.v.o.r.t.s.r.s.s.r.r.r.o.o.o.o.n.n.m.n.o.n.n.p.q.r.s.s.k.l.o.t.y.A.z.x.F.A.x.H.X.2.M.u.q.f.d.k.t.F.D.s.y.B.z.w.J.2.3.O.n.Q#l.W.r.x.B.U.R.A.u.M.Y.R.C.v.t.w.z.z.w.v.t.t.t.u.v.t.n.i.t.F.A.y.y.y.r.j.h.l.w.C.t",
".H.O.J.r.l.t.w.r.w.u.t.v.z.D.C.B.E.I.H.z.t.s.t.t.v.q.n.p.v.x.r.j.i.o.v.A.C.z.v.t.m.J.P.D.w.p.m.s.l.o.o.o.v.G.Q.U.t.t.v.y.D.I.J.K.F.E.D.C.B.z.y.y.x.x.x.x.w.w.w.w.r.y.y.s.u.y.r.d.u.e.r.x.s.u.p.u.t.t.t.s.s.q.q.q.p.p.o.n.n.n.m.m.o.n.n.o.p.r.s.s.w.r.n.m.q.w.z.A.w.u.m.k.y.R.R.D.v.l.u.r.b.t.L.q.w.C.E.x.v.H.1#b.C.h.I.5.N.s.e.d.T.O.R.Z.T.A.q.s.w.y.z.z.x.w.w.w.p.r.s.r.j.h.p.A.E.N.N.y.j.h.o.x.B.E.y",
".L.Q.O.D.v.v.w.t.v.t.r.t.x.z.x.u.D.D.v.n.r.z.y.o.u.t.s.s.q.p.k.h.m.t.v.r.s.z.B.w.m.o.E.0.W.u.l.z.s.r.p.r.G.S.L.u.p.s.v.y.A.D.F.G.F.F.E.C.B.A.z.y.y.y.x.x.x.w.w.w.z.t.u.A.v.j.h.r.D.o.t.y.u.v.u.q.s.s.t.r.r.q.q.q.o.o.n.n.n.n.m.m.n.m.m.o.p.q.s.s.z.w.r.m.m.p.w.B.u.A.x.p.r.G.W.3.H.y.w.m.c.l.B.t.u.x.C.C.y.z.O.5#m.E.i.I#d.Z.t.x.q.U.9.U.x.o.p.q.w.x.y.y.y.z.z.A.x.q.j.l.p.r.q.m.M.U.T.G.v.z.F.G.o.C.E",
".z.y.C.I.C.r.m.q.u.s.p.q.s.s.q.n.i.w.E.y.t.u.t.o.r.x.B.w.m.f.f.k.m.u.t.n.p.y.B.v.C.l.r.R.U.A.o.o.w.D.L.O.U.V.F.k.p.r.u.x.x.B.F.I.G.F.E.D.C.A.z.z.y.y.y.x.x.x.w.w.y.v.y.z.n.e.j.C.w.s.r.s.p.u.z.m.t.t.s.s.r.q.p.q.p.p.o.n.n.n.m.m.n.m.m.n.o.q.r.r.v.x.x.t.m.m.s.y.C.z.x.w.r.s.G.Y.Z.L.m.m.H.E.p.s.A.v.x.D.F.D.E.K.J.M.z.y.T#..1.C.r.O.V.F.o.l.s.w.t.u.u.w.y.z.A.A.C.s.i.f.n.u.x.v.I.J.E.A.F.N.H.u.h.F.O",
".w.s.z.M.M.z.s.w.s.r.q.o.n.n.n.o.A.w.o.j.m.t.v.s.r.x.C.v.j.c.g.o.i.q.u.s.u.z.w.n.z.n.i.j.q.G.G.n.h.A.S.V.L.z.r.m.p.r.s.u.x.C.G.K.G.G.F.D.C.B.A.z.y.y.y.y.x.x.x.x.p.C.D.n.e.l.w.z.o.w.u.w.q.v.M.z.u.u.t.s.q.q.q.p.o.o.n.o.o.n.m.m.n.m.m.n.o.q.r.r.q.v.A.y.s.o.p.s.I.s.k.q.u.p.o.u.Z.3.M.N.1.C.d.p.E.x.v.A.H.H.C.y.A.p.z.F.j.B.5.p.X.E.r.t.v.r.t.C.r.q.r.t.w.x.w.v.w.v.q.h.g.s.G.P.w.r.k.p.A.E.t.i.s.M.S",
".A.z.B.I.N.L.F.A.p.r.s.q.n.n.s.w.P.u.d.c.m.v.y.y.u.s.q.n.k.k.l.m.l.p.t.u.w.v.t.p.y.u.u.l.i.J.0.L.j.s.C.F.x.k.i.n.p.p.r.v.z.D.F.G.G.G.F.E.C.B.A.z.z.z.y.y.y.x.x.x.t.w.r.g.k.y.C.u.q.y.w.H.x.r.Q.Q.u.t.u.t.q.p.q.o.o.o.n.n.o.n.m.m.m.l.l.n.o.p.r.r.p.s.w.z.z.v.p.k.E.y.t.w.C.A.w.q.G.Z.9.3.K.n.h.p.t.w.z.B.C.C.B.x#v.0.D.F.i.q#g#c.X.z.l.s.A.w.u.u.q.p.q.r.t.s.p.l.q.w.w.r.q.x.F.H.m.i.i.o.r.r.p.r.N.G.E",
".v.y.v.r.y.J.F.s.n.r.t.r.o.p.x.F.m.l.s.A.w.n.r.D.x.m.e.f.n.u.p.h.u.r.q.r.s.r.s.w.v.m.w.w.f.k.H.N.J.v.o.v.y.t.q.s.r.q.s.x.A.C.B.y.G.G.F.E.C.B.A.A.z.z.z.y.y.y.x.x.G.i.b.l.B.D.w.u.s.u.s.M.y.i.K.T.u.u.t.t.q.p.p.o.o.o.n.n.o.n.m.m.m.l.l.n.o.p.r.q.r.q.r.y.E.B.q.f.f.p.w.s.p.v.v.q.r.x.S.P.q.u.G.o.#.q.G.E.y.w.x.x.A#e.V.A.B.z.W.7.s.A.A.s.t.z.t.g.q.p.p.r.r.p.j.f.p.u.w.C.K.M.y.j.l.m.t.w.q.m.z.S.Z.u.k",
".A.H.H.z.s.w.E.M.B.t.y.q.e.w.O.t.e.z.L.y.l.n.q.n.r.i.f.i.m.k.j.n.r.h.n.t.n.k.n.k.t.t.h.j.r.g.k.S.T.H.t.q.t.u.s.o.o.q.s.w.x.z.B.B.E.F.G.H.I.H.F.E.K.x.t.B.D.y.x.F.f.l.u.x.w.w.v.u.D.v.q.R#a.F.h.A.V.S.k.h.E.g.a.N.n.n.n.n.n.o.n.n.o.n.n.n.n.o.o.n.r.s.u.v.x.y.z.z.b.j.x.B.x.t.u.x.q.p.s.M.2.Q.x.t.u.k.o.y.p.e.y.7.s.S#w.2.T#g.E.p.t.t.u.u.u.u.v.v.o.t.s.l.j.p.m.d.w.h.m.K.R.y.j.i.r.d.e.v.G.H.W#e.H.B.B",
".H.D.y.v.r.s.x.C.l.y.i.e.H.M.n.f.r.B.C.t.m.m.m.h.k.g.j.n.l.i.r.H.k.x.F.u.s.O.T.x.B.I.D.D.G.p.f.p.G.z.t.q.u.v.q.m.o.p.r.u.w.y.A.B.E.F.G.H.I.H.G.F.A.G.F.x.x.E.y.m.m.q.w.y.x.v.w.w.r.l.i.l.x.M.F.i.L.Z.J.k.p.D.w.h.o.n.n.n.n.o.n.m.o.n.n.n.n.o.o.n.q.s.t.v.x.x.y.z.m.n.o.s.v.x.w.u.w.t.m.w.W.3.O.D.A.r.m.m.q.s.z.I#y.T.O#p#A#a.E.K.z.y.x.w.u.t.t.s.x.A.z.s.p.r.u.v.n.n.y.N.L.v.p.w.i.l.m.o.B.T.1.0.N.Y.Q",
".u.m.k.s.x.w.v.y.i.r.k.t.Q.A.e.l.F.x.p.o.p.o.k.f.j.l.l.i.k.w.D.H.x.t.z.D.A.E.J.E.z.K.M.Q.X.O.x.u.s.r.r.s.t.t.r.n.q.q.q.s.v.y.B.C.E.G.H.I.J.I.H.G.A.J.K.A.y.A.s.e.t.v.y.z.x.w.w.w.A.u.z.o.j.U##.B.h.x.R.C.e.n.x.d.o.o.o.n.n.n.n.n.n.m.m.m.m.n.n.m.p.r.s.u.w.w.x.y.A.r.i.j.r.x.w.s.t.y.o.k.H.1.1.W.I.D.q.k.x.L.E.m.J.D.w.V.W.x.w.w.F.D.A.w.u.r.q.q.v.u.t.r.l.k.r.C.j.n.r.v.q.k.l.r.k.q.w.B.K.U.Q.E.J#..Z",
".l.g.j.x.C.u.p.r.x.i.G.Y.q.d.v.N.D.o.g.n.u.q.m.l.o.q.k.g.t.L.G.l.u.g.j.w.q.j.n.q.i.o.o.p.B.K.F.x.n.o.r.t.t.q.q.r.q.p.o.q.s.v.z.B.E.F.H.I.J.I.H.G.I.E.E.I.A.l.j.t.A.A.A.A.y.x.x.x.v.u.E.A.l.v.P.R.A.q.J.7.Q.r.s.D.o.o.o.n.n.m.m.m.m.l.l.l.l.m.m.l.o.q.r.t.u.v.w.x.D.v.m.j.n.q.s.s.n.B.y.m.r.E.T##.J.I.B.w.F.P.I.o.g.6#o.S.p.C.T.K.F.D.A.x.u.t.s.t.s.n.o.u.t.n.r.B.B.t.i.f.g.l.k.e.k.i.u.Q.T.F.u.v.B.U.R",
".E.A.A.B.t.g.e.j.C.w.L.H.f.j.J.L.v.k.h.s.w.p.m.p.r.h.h.x.J.C.r.n.t.D.I.x.t.K.P.x.D.B.u.l.l.w.y.n.o.r.r.s.q.o.q.s.r.p.p.o.q.t.x.z.B.C.E.F.G.G.F.F.I.D.D.D.s.h.q.H.E.D.B.A.z.y.x.x.n.z.z.y.w.g.h.G.0.v.e.r.S.B.e.e.p.p.o.n.n.m.l.l.m.l.l.l.l.m.m.l.n.p.q.s.t.u.v.w.v.w.w.q.k.i.o.t.r.x.t.q.q.n.y.Y.z.C.I.K.I.E.D.H.U.Q.8.F.s.D.m.z.B.A.y.w.w.v.x.x.u.p.s.G.L.E.A.C.J.F.x.p.p.w.t.k.h.k.x.J.G.v.r.z.v.x.C",
".W.T.M.x.j.f.l.v.t.Q.t.c.w.U.B.h.r.n.p.w.v.n.k.o.p.e.q.T.N.j.j.M.F.F.J.E.y.G.M.A.2.X.U.J.u.u.v.i.r.s.q.q.p.o.q.s.r.r.p.p.p.q.s.u.x.y.A.C.D.D.C.C.y.I.F.o.h.s.D.F.D.C.A.A.A.z.y.w.r.J.x.p.F.x.f.n.X.3.A.g.B.Y.L.t.r.q.p.o.m.m.l.k.l.k.k.k.k.l.l.k.m.o.p.q.s.t.u.v.q.w.A.w.l.g.k.p.y.q.i.n.w.p.k.w.v.w.C.J.C.t.E.Z#z.I.L.F.M.P.j.K.z.y.y.y.x.x.x.w.m.j.o.z.I.I.B.x.q.z.E.z.r.o.r.t.A.Q.R.y.n.s.y.u.w.n.u",
".S.X.R.w.n.u.H.N.t.E.l.g.L.Q.p.e.r.r.t.v.t.o.n.n.m.w.I.L.A.p.y.Q.u.d.f.r.m.e.i.o.C.B.N.K.p.l.q.f.r.p.o.p.q.q.q.r.r.q.r.q.p.q.p.q.s.t.v.x.z.z.y.y.u.z.u.j.m.A.D.y.B.A.z.z.B.B.z.v.t.v.p.l.t.y.r.l.h.D.R.w.g.u.O.H.q.p.q.o.m.l.l.k.k.j.j.j.j.k.k.j.l.n.o.p.r.s.t.u.u.u.w.v.q.m.j.g.r.p.i.m.z.u.j.h.J.B.x.z.C.E.U#c.0.E.F.w.y.H.v.E.z.B.A.z.x.w.s.q.e.g.j.n.u.C.B.w.n.q.t.r.k.h.q.B.O.1.X.y.l.o.s.j.s.r.r",
".N.Y.V.D.w.I.S.P.F.d.v.5.I.f.n.B.q.s.t.s.r.s.s.q.m.Y.0.n.j.P.X.y.j.p.w.q.o.G.P.B.i.h.D.L.q.k.v.s.n.m.m.p.r.r.p.o.r.q.q.r.p.o.o.m.o.p.r.t.v.v.u.u.z.j.f.q.B.z.t.v.z.y.x.y.B.B.x.u.I.n.w.H.r.u.N.O.n.f.U#i.y.h.E.N.q.q.q.o.m.l.k.k.k.j.j.j.j.k.k.j.l.m.n.p.r.r.t.t.z.t.p.r.u.t.j.c.c.r.t.o.t.u.n.h.3.L.v.s.F.X#b#m.r.K.H.E.A.A.H.t.C.D.D.C.x.s.o.k.j.q.r.l.r.D.L.K.M.w.i.h.l.r.B.M.B.A.B.B.t.h.d.g.l.x.q",
".q.s.o.o.y.L.L.E.f.D.P.A.l.n.s.r.m.A.s.p.D.v.j.x.L.R.i.g.1.2.n.h.s.v.w.x.C.P.4#b.Y.I.v.t.v.u.q.p.q.q.r.r.q.q.p.p.o.o.p.q.q.p.q.q.k.r.x.u.p.o.r.v.c.g.n.u.y.z.x.v.x.w.x.w.w.x.w.x.m.v.x.r.v.P.7#a.X.z.h.B.Y.t.g.I.5.x.e.o.s.f.d.p.j.j.i.j.j.k.k.k.k.k.m.n.o.o.p.q.u.u.t.s.q.p.o.o.d.#.k.o.c.o.O.T.R#c.U.u.U#j.2.z.D.C.B.A.z.y.x.x.B.H.v.w.G.m.e.q.l.o.u.w.n.i.r.G.e.w.s.m.y.v.n.v.l.p.B.L.H.s.g.f.h.C.G",
".x.H.J.D.H.P.A.f.E.K.G.r.i.o.v.x.u.s.u.v.o.q.E.N.K.l.l.U.2.q.g.O.t.F.O.L.z.s.A.L.O.E.v.w.z.v.s.s.r.r.r.r.q.q.p.p.o.o.p.q.q.p.q.q.o.q.s.s.t.s.p.m.g.j.o.t.w.x.v.u.x.w.w.w.w.w.w.x.A.r.o.z.P.W.Q.I.6.U.y.s.J.V.E.i.y.I.A.h.j.x.r.a.i.i.j.j.j.j.k.k.j.j.l.m.n.n.p.p.u.t.t.r.q.p.o.o.d.#.i.j.a.f.A.C.m.u.B.T#c.3.G.C.F.E.D.C.A.z.y.x.v.q.A.x.h.w.J.n.C.x.s.s.q.n.r.y.4.z.N.1.w.m.z.t.C.r.m.u.D.B.o.h.r.y.y",
".s.m.x.R.Q.v.l.v.S.H.u.o.p.u.y.A.s.q.H.C.f.s.W.P.j.j.R.9.z.f.G#g.L.R.2.7.Y.F.y.B.C.z.x.A.B.x.v.u.t.s.s.s.r.q.p.p.o.o.p.q.q.p.q.q.t.p.o.r.u.t.l.f.l.n.q.s.u.u.u.t.u.u.u.u.u.u.u.u.C.p.s.O.1.R.y.o.P.S.I.k.l.S.V.n.d.H.X.C.g.l.o.i.j.j.j.j.j.j.j.j.j.k.k.l.m.m.n.n.t.t.s.r.q.p.o.o.k.e.i.i.#.c.o.n.U.Q.U###..E.n.D.E.E.E.E.D.B.A.z.E.u.A.v.h.x.O.u.s.s.u.v.r.l.l.o.S.E.t.C.K.x.m.s.J.t.h.m.y.F.E.B.K.v.w",
".k.h.r.E.y.l.z.1.F.t.o.v.C.A.x.z.v.w.B.y.w.Q.R.m.k.V.7.x.g.Q#a.P.P.E.H.1#c.5.N.D.x.z.A.C.B.x.w.x.u.u.t.s.s.r.p.p.o.o.p.q.q.p.q.q.t.q.q.r.q.m.j.j.r.r.r.r.r.r.r.r.t.u.t.t.t.t.u.t.r.y.M.V.O.y.q.t.m.x.z.n.f.i.A.Z.q.q.F.U.D.d.d.A.k.k.k.j.j.j.j.j.i.i.j.k.l.m.n.n.s.s.r.r.q.p.o.o.q.n.m.l.h.j.n.m.N.O.L.G.B.v.y.H.C.C.D.E.E.E.D.B.z.B.m.u.W.V.Q#e.C.P.2.2.P.A.w.w.s.Q.H.z.Y.0.I.L.x.w.t.r.t.C.R.3.S.x.D",
".y.V.N.h.g.L.Y.H.s.l.o.A.F.y.t.w.F.w.j.w.1.Y.w.l.6.8.B.g.J#f.T.e.C.l.i.z.U.W.K.x.z.B.C.A.z.y.z.A.x.x.v.t.s.r.q.p.o.o.p.q.q.p.q.q.r.s.v.u.l.e.k.u.u.t.s.r.q.q.p.p.s.r.r.r.r.r.r.s.t.M.X.L.t.m.q.v.d.n.j.r.E.j.i.Y.X.p.g.A.S.F.o.l.m.m.k.j.j.j.h.h.h.h.i.j.k.l.m.m.r.r.r.q.p.p.p.o.n.p.m.k.n.n.m.o.u.z.C.v.s.y.C.x.z.z.C.D.E.F.E.C.w.u.d.j.N.J.A.V.X.4##.8.0.N.A.s.y.E.6.7.H.Q.0.E.m.s.v.t.r.z.O.0.J.u.H",
".V.Z.J.n.x.W.O.j.t.q.s.y.x.r.r.x.s.y.y.O.W.p.k.8#b.B.i.P#d.I.h.C.E.y.t.t.y.E.F.F.D.F.E.z.x.B.C.C.z.z.x.u.s.q.p.p.o.o.p.q.q.p.q.q.q.q.u.w.p.h.o.B.u.t.t.s.r.q.p.o.r.r.r.r.r.r.r.r.J.Q.L.t.k.p.p.k.g.m.g.x.5.X.v.y.I.I.u.h.v.T.J.e.m.n.m.k.j.h.g.g.f.f.g.i.j.k.k.l.q.q.q.q.p.p.p.o.f.n.h.f.m.m.j.r.S.D.A.E.w.t.w.t.w.y.A.D.E.C.A.z.s.g.l.k.f.x.J.l.W.N.E.G.T.Y.N.w.H.s.u.J.N.G.u.k.o.m.k.k.s.A.A.x.w.q.D",
".L.r.s.T.1.H.q.r.y.y.y.w.s.q.u.y.m.L.0.R.w.n.J#g.A.n.L#e.R.g.q.Z.H.O.L.w.o.w.H.M.F.H.F.A.B.F.F.B.A.z.y.w.t.q.p.o.o.o.p.q.q.p.q.q.t.m.n.x.B.v.t.y.r.r.t.s.s.q.p.o.p.o.p.o.o.p.o.p.P.G.r.l.o.q.j.b.i.l.E.P.P.0.S.m.j.L.U.v.h.r.G.G.n.n.m.k.j.h.f.f.f.f.f.h.i.j.j.j.q.q.p.p.p.p.p.p.d.p.j.e.n.j.h.s.E.f.h.r.o.r.C.C.y.z.A.B.A.x.u.s.i.g.v.w.l.K.9.W.N.F.x.B.T.9.7.Y.K.H.g.k.1.Y.v.B.w.q.i.h.r.A.w.l.v.z.x",
".h.i.G.9.3.y.n.A.y.B.B.x.u.w.y.z.D.U.Y.t.l#.#l.z.j.J#l.Z.e.y.2.u.r.C.x.g.e.s.D.C.E.H.F.C.E.J.G.z.A.z.z.w.t.q.p.o.o.o.p.q.q.p.q.q.w.h.f.x.M.I.x.s.q.q.s.t.s.r.p.o.p.o.p.o.o.p.o.p.H.q.h.p.u.n.e.d.o.q#b##.i.k.S.n.i.r.L.S.s.c.n.6.o.n.n.k.j.g.f.e.e.f.e.g.h.i.i.i.p.p.p.p.p.p.p.p.i.v.o.h.r.l.h.x.r.h.r.x.r.B.H.p.y.z.A.A.y.u.p.m.p.B.u.o.v.j.n.0.v.C.E.D.H.R.X.Y.E.E.K.A.q.P.9.V.z.y.r.n.r.B.A.u.H.K.v",
".x.O.Q.B.y.I.G.s.l.P.x.i.H.B.k.H.Z.R.o.k.T.4.C.i.8.0.I.r.w.M.P.F.I.E.G.G.x.k.u.O.G.E.E.E.G.F.B.x.B.A.z.w.u.t.q.p.n.g.g.m.o.k.l.r.t.o.e.g.y.C.r.n.v.r.r.s.u.s.p.n.s.e.o.v.d.e.D.O.a.h.o.n.j.e.g.l.E.S.I.s.s.n.w.W.t.#.b.A.O.B.k.j.H.t.g.f.j.h.e.d.d.d.c.c.f.f.h.i.i.j.k.l.m.m.m.m.g.h.i.i.j.j.g.f.e...k.A.p.m.u.q.x.p.s.F.F.r.j.n.D.y.B.C.r.l.r.u.w.B.D.B.H.N.D.n.D.D.G.D.q.l.G#..N.B.o.o.y.H.E.v.F.T.N",
".Z.0.P.y.w.I.K.C.y.x.s.w.D.x.A.U.2.m.i.2#g.D.n.R.8.O.C.E.H.B.A.F.A.A.F.J.z.m.l.t.I.G.D.A.z.A.C.D.B.A.y.w.u.s.q.p.o.k.j.n.r.r.p.p.D.T.H.m.w.U.X.N.p.u.r.k.m.u.p.f.o.l.p.l.g.y.I.s.#.b.d.c.d.h.o.u.N.N.u.k.u.r.k.s.o.l.h.n.E.N.y.h.A.Q.Q.w.f.h.k.g.d.d.c.d.e.h.h.i.k.l.m.n.o.o.o.o.m.k.j.k.k.k.j.k.m.a.d.p.o.v.B.r.z.A.C.z.m.e.j.y.i.N.p.d.M.T.n.f.u.A.J.M.H.D.J.Q.B.y.B.I.E.v.w.D#..M.A.J.P.D.t.u.E.s.h",
".T.K.x.p.u.G.L.K.F.t.B.H.t.w.Q.W.k.x.0##.M.v.P#h.M.B.z.I.L.E.B.H.y.B.F.H.E.v.o.j.C.F.I.F.B.z.B.E.A.z.y.v.t.s.p.o.m.n.l.l.l.m.h.e.l.R.X.B.y.J.U.0.u.q.n.n.s.u.t.p.r.k.l.n.t.J.D.d.f.b.#.a.e.o.B.J.H.C.m.g.q.o.c.c.m.t.p.h.o.F.G.u.h.w.E.t.f.d.e.f.d.e.d.d.f.h.i.i.m.m.o.p.p.q.p.p.r.p.n.l.k.k.l.o.n.h.m.n.l.u.D.u.x.z.y.p.g.h.t.H.v.E.m.u.3.M.d.f.o.u.I.N.A.r.I#..S.F.y.F.K.E.t.o.N.S.Y.V.F.s.z.Q.q.k.e",
".v.p.m.s.z.C.D.F.O.D.z.z.A.P.Q.w.i.7#q.G.e.L#h.Y.r.C.G.C.E.N.L.B.F.H.G.E.G.H.B.t.f.o.D.J.I.D.C.D.z.z.x.v.s.r.o.n.f.m.p.n.m.r.s.p.a.o.L.Z.O.m.o.U.S.x.p.z.A.p.o.B.u.h.l.C.G.w.n.i.n.j.e.g.o.z.J.P.m.q.p.k.f.d.e.j.n.p.r.m.e.h.z.S.w.g.g.x.D.q.h.l.e.f.f.e.f.h.i.j.n.o.p.q.r.r.r.r.r.q.p.m.j.i.k.l.m.y.F.t.e.j.u.x.x.s.k.h.r.B.D.x.H.q.K.5.I.j.l.n.i.o.x.A.s.o.D.W#g.W.C.y.C.D.A.z.i.L.Z.E.n.x.L.O.g.F.H",
".w.v.z.G.F.y.u.w.Q.G.r.x.W.V.z.q#i.6.P.t.q.V.Z.o.A.K.L.C.B.H.F.w.E.F.F.D.G.J.F.y.d.j.x.F.H.E.C.C.z.y.w.u.r.q.n.n.g.n.q.l.l.x.P.2.q.j.s.V.1.u.l.H.Z.Q.G.z.s.l.l.q.m.r.z.I.C.k.f.q.o.m.k.o.v.A.B.A.g.i.n.j.b.c.l.s.l.p.s.m.f.e.q.E.L.n.f.u.F.v.i.d.h.g.g.f.g.i.j.j.o.p.q.r.s.s.s.r.q.p.q.m.j.h.k.m.B.L.H.o.d.f.o.v.y.q.h.i.u.I.H.y.p.z.0.Q.g.i.E.v.o.q.p.n.s.A.B.v#c.1.L.C.y.y.B.F.n.y.x.p.z.P.G.i.w.W.0",
".C.D.F.H.E.y.x.A.s.D.L.W.S.p.q.3#o.J.o.E.T.Q.D.q.M.C.A.H.H.A.A.I.x.z.D.E.D.B.y.x.R.M.F.A.y.y.y.x.y.x.v.t.r.p.n.m.o.l.j.e.d.i.F.0.U.B.h.l.J.N.x.o.y.Q.P.r.i.v.u.h.j.I.M.w.p.l.l.q.k.i.l.o.r.q.l.h.q.e.e.g.f.m.r.l.j.t.s.j.m.v.n.c.u.C.z.l.k.v.t.g.h.i.h.g.h.h.j.j.n.o.p.q.r.r.r.r.n.p.o.n.j.k.p.t.R.M.q.f.n.s.o.p.i.k.n.p.s.A.N.X.v.N.G.o.m.m.n.z.D.A.s.n.z.N.E.m.L.O.Q.O.F.y.v.v.q.l.q.I.R.J.w.q.T.L.I",
".z.A.z.y.z.D.E.C.i.B.4.2.v.n.Q#f.A.B.B.K.T.M.F.J.L.z.v.D.G.C.C.K.w.x.A.D.A.u.C.P#i.1.D.q.q.w.x.w.x.w.v.s.q.p.m.l.p.j.k.s.o.g.i.w.V.W.A.h.r.L.C.i.i.B.O.E.v.w.x.p.y.P.D.l.o.q.l.p.j.j.l.l.m.i.f.a.q.b.b.h.g.p.u.g.m.p.o.l.p.u.p.d.i.I.Q.t.k.A.K.C.i.i.i.h.h.i.k.k.n.n.p.q.q.r.q.q.o.p.o.l.l.q.B.L.E.y.i.l.I.G.o.m.c.k.w.D.z.u.I.2.X.L.u.r.C.A.t.x.I.D.x.v.C.H.E.y.z.G.P.Q.O.G.z.w.m.o.B.O.H.s.z.U.K.s.p",
".A.B.y.v.A.H.E.u.F.B.P.C.n.7#s.H.k.C.F.H.T.N.D.K.I.L.E.u.w.H.E.t.D.y.y.B.w.u.N#b.1.F.h.d.k.y.E.C.x.w.u.s.q.o.m.l.k.h.v.X.0.D.m.o.L#a.9.H.x.A.A.x.r.u.P.6.S.m.g.y.W.J.j.h.z.r.e.t.n.o.o.n.k.h.h.g.h.#.g.m.d.i.s.i.r.h.k.u.o.c.o.O.e.r.z.p.h.i.s.z.j.i.i.i.h.i.k.k.m.n.o.p.q.q.q.q.q.p.n.j.k.v.M.Z.e.k.m.E.Z.J.k.m.n.s.I.U.J.q.q.C.Z.F.K.B.p.6#h.o.G.z.y.D.z.u.z.M.H.I.J.K.O.Q.M.I.D.H.D.t.m.r.H.T.j.k.p",
".w.J.x.q.G.C.r.A.V.7.h.d#f#d.l.o.D.B.B.z.t.t.L.4.I.H.D.z.y.z.B.A.y.B.E.A.x.F.0#j.l.f.j.t.p.d.h.y.p.p.o.n.m.k.j.h.h.d.d.o.Q.6.P.n.b.r.P.I.t.z.B.o.p.z.e.h.R.H.q.W.N.f.l.W.6.U.C.h.l.n.l.i.l.s.n.d...d.k.n.m.j.i.h.o.g.f.n.r.q.B.S.N.G.k.f.p.d.#.p.C.y.s.m.j.j.k.k.c.i.o.r.p.n.o.r.u.g.m.u.x.O.L.e.g.p.H.R.F.j.i.u.v.r.z.L.K.u.k.k.i.s.F.T.1.V.I.x.v.y.x.v.x.A.z.t.E.N.B.y.U.W.G.E.z.M.C.f.n.N.K.g.z.t.w",
".w.p.u.z.u.u.H.V.G.n.y.V.P.A.v.o.x.w.x.y.v.w.H.V.Y.I.y.E.I.B.r.o.u.x.t.s.J.1.U.z.h.i.q.z.w.j.f.k.x.x.w.r.p.m.o.s.p.n.f.e.o.K.V.Q.p.s.A.C.t.t.z.r.l.t.j.g.H.3.V.E.n.x.O.M.A.L.R.y.i.n.r.q.k.h.k.q.e.h.l.n.k.j.k.k.i.l.j.h.r.G.F.t.G.L.p.b.h.o.h.g.x.v.p.l.i.h.i.j.i.l.p.p.o.o.r.t.j.x.h.n.V.K.h.k.h.G.T.D.h.f.o.v.s.t.o.o.M##.Z.u.t.u.w.A.A.z.w.t.M.C.B.G.z.o.v.Q.v.E.A.y.F.y.w.O.U.x.s.I.P.z.l.m.w.r.u",
".v.e.s.z.k.y.U.J.q.h.V#c.r.e.y.w.r.q.s.y.z.A.D.I.0.P.B.w.A.J.O.O.w.t.t.E.Z.4.E.c.m.r.x.A.y.r.j.d.n.q.q.o.k.i.h.j.F.K.F.r.n.D.2#e.L.s.r.x.u.s.r.p.u.A.G.s.d.u.I.p.n.L.Z.C.j.y.S.O.s.r.s.r.j.e.i.p.m.m.n.l.j.j.j.l.g.k.j.l.y.J.x.f.l.A.w.i.m.o.h.e.q.n.j.g.f.e.g.i.o.o.o.n.n.p.q.s.m.j.x.N.E.o.i.k.M.P.H.r.m.t.w.q.5.H.o.o.E.U.0.1.y.u.o.k.m.v.H.Q#g.Y.H.G.D.C.T#d.k.C.G.C.G.I.L.W.K.t.v.N.I.h.d.p.t.p.u",
".p.m.q.q.u.O.K.e.o.T.X.D.q.k.o.D.m.m.p.w.C.C.y.v.A.S.S.z.v.P.7.7.K.A.H.2.0.z.j.m.v.y.v.p.q.u.q.g.e.g.j.m.n.k.g.c.o.C.L.C.r.r.B.K.N.x.t.z.w.u.s.i.u.m.w.z.f.c.A.X.Q.G.B.v.n.n.z.K.L.D.q.i.k.p.l.g.m.m.k.j.i.h.i.j.l.g.m.B.D.q.i.j.e.i.n.s.n.d.g.z.j.i.g.e.c.d.g.g.o.n.m.m.l.l.l.l.q.o.O.S.d.a.u.z.Z.A.k.q.z.v.o.n.S.L.I.D.o.i.o.T.t.q.n.i.l.z.R.5.Q.K.x.t.I.1.0.N.l.F.J.z.J.1.W.F.M.7##.O.s.q.y.A.s.o.u",
".n.v.o.q.L.L.r.i.E.Z.y.f.B.D.l.v.l.n.r.w.B.B.v.o.j.B.T.V.P.L.K.K.R.M.S.Y.D.e.d.y.x.z.t.j.l.v.u.l.o.l.i.m.s.v.r.n.g.t.E.G.z.s.o.l.C.y.x.u.q.z.E.t.H.r.h.w.H.q.s.Z.Y.z.h.o.v.m.k.w.L.M.z.h.h.v.t.i.i.h.h.h.j.i.g.f.j.o.A.H.u.e.d.r.m.#.a.h.h.i.v.I.g.f.d.d.d.f.g.i.m.m.m.l.j.j.h.h.r.S.F.g.j.h.o.U.n.g.h.s.s.j.l.s.f.I.6.Y.I.B.v.l.m.m.m.k.l.r.B.J.m.x.H.R.5#..O.m.x.A.C.z.A.J.J.y.1#e#b.N.s.r.v.u.s.o.u",
".o.t.o.C.T.u.m.Y.P.o.h.r.B.B.t.g.o.t.z.y.y.w.u.o.r.g.p.Q.U.y.n.u.I.P.L.t.f.e.j.n.q.t.r.n.p.v.t.l.t.k.c.b.h.q.v.v.A.C.E.H.I.H.D.A.w.u.w.o.i.y.N.L.u.C.t.r.A.p.g.u.y.y.n.i.r.q.k.q.s.I.L.u.j.p.w.y.i.i.g.i.j.j.g.e.l.E.M.x.i.g.l.n.f.c.e.l.u.M.J.i.f.e.d.e.f.g.h.j.i.k.n.l.j.i.l.p.I.F.l.c.j.B.E.B.b.i.p.o.w.I.I.y.k.q.C.R.2.Z.C.d.e.h.k.l.n.o.r.s.L.Q.3#a.2.H.t.s.I.s.z.N.D.w.N.6.0.y.t.L.N.s.i.r.s.p.u",
".s.u.A.G.E.u.C.2.C.h.x.O.r.r.B.g.s.C.G.A.s.r.s.t.x.k.h.w.C.s.p.u.t.C.y.h.e.n.r.h.l.n.n.n.o.o.n.k.n.i.c.b.b.g.o.u.y.x.x.z.B.A.z.z.w.r.s.t.l.p.F.L.f.m.x.r.l.z.I.x.h.z.w.l.r.s.l.r.j.u.I.L.w.k.q.E.r.n.k.i.j.j.j.i.E.I.C.q.m.u.q.f.c.i.C.M.K.Q.H.f.g.f.f.f.h.j.k.k.j.m.o.k.h.l.w.H.I.e.e.o.m.G.J.c.m.q.n.l.w.O.Q.F.O.m.g.m.y.p.e.f.f.e.h.l.q.t.v.t.N.P.Q.L.w.k.q.D.I.v.F.S.K.N.5#a.I.p.n.K.Y.M.B.C.s.o.s",
".w.A.N.C.l.H.V.v.i.F.5.R.j.m.C.u.w.J.N.C.p.n.s.v.j.G.L.t.m.v.u.f.m.m.o.r.r.q.p.r.p.m.l.l.k.j.l.p.k.q.t.q.k.j.p.w.t.x.D.H.E.x.t.t.B.n.u.E.s.h.o.y##.x.z.E.s.R.4.q.h.x.u.s.A.q.h.t.p.j.w.T.N.l.g.x.A.u.m.i.i.l.m.l.2.A.i.r.D.z.m.e.v.s.R.4.D.p.B.H.g.h.g.i.j.k.l.m.k.o.o.l.g.o.G.X.f.g.b.p.O.B.e.c.n.l.n.n.e.d.r.Q.V.X.L.o.c.g.n.m.n.j.h.i.n.r.t.t.i.y.D.u.x.O.W.P.B.E.L.E.E.5.6.t.j.F.J.q.t.R.O.n.s.m.q",
".R.Q.v.r.M.L.p.k.C.O.L.s.l.s.s.j.I.D.v.r.u.y.u.m.r.w.A.A.t.o.m.m.k.n.t.z.C.z.o.h.j.j.l.m.m.m.n.o.b.t.e.n.v.l.v.o.R.H.A.D.F.B.x.y.w.m.v.U.Z.B.g.e.E.M.Q.R.1.6.I.f.j.j.j.j.o.u.p.g.i.j.j.m.z.I.v.e.r.N.B.k.n.j.i.F.q.p.w.F.y.k.i.s.D.0.9.P.p.m.z.M.f.k.q.r.n.l.n.p.o.q.k.s.x.O.Z.d.b.n.B.G.v.f.d.j.a.l.A.G.v.k.i.t.v.D.G.x.m.j.i.g.e.f.h.k.o.q.t.t.u.E.p.u.3.4.M.T.A.F.y.K#c.1.s.o.A.z.x.v.v.B.L.T.u.p.h",
".F.D.u.F.S.B.o.C.1.u.q.V.4.C.q.E.v.t.s.u.w.x.v.v.v.s.p.r.u.w.u.s.t.m.i.n.y.C.w.m.h.h.h.j.m.o.o.o.r.b.y.m.B.0.x.L.W.J.z.x.x.u.u.x.s.n.n.y.K.K.x.k.h.B.V.Z.W.S.G.t.v.h.g.u.D.x.q.q.l.k.k.n.w.D.B.v.d.u.S.H.j.A.M.j.s.m.n.s.q.j.r.F.7.Y.J.v.t.v.u.p.l.o.p.q.p.p.r.t.u.j.s.o.1.6.c.k.c.A.Q.x.f.d.j.n.k.m.x.Q.1.S.x.k.s.x.y.r.k.k.k.i.f.g.i.k.o.r.t.t.s.D.p.p.P.P.z.I.y.C.P.U.F.t.u.y.C.C.B.y.w.x.B.F.C.y.v",
".q.x.z.C.E.v.w.O.w.i.h.x.I.D.u.s.u.r.u.A.w.p.o.v.E.u.k.k.q.w.y.x.u.n.h.i.o.v.z.A.k.i.g.g.k.j.h.e.e.I.d.p#i.Q.r#h.H.A.u.v.x.x.x.z.y.x.n.f.p.L.O.A.t.G.U.T.K.D.H.O.u.P.R.t.e.o.v.r.q.m.m.p.r.v.G.Q.t.h.H.9.L.s.A.B.y.q.k.l.n.s.G.X#c.P.p.m.u.w.j.#.r.r.r.q.r.r.u.w.r.w.m.6.0.j.s.d.L.M.C.m.e.h.k.h.s.i.e.k.J.Z.V.I.o.r.o.l.j.k.k.i.g.g.i.l.n.r.t.t.q.B.s.o.D.A.t.F.W.Q.9.3.m.h.D.F.E.E.E.B.x.u.s.s.W.U.V",
".p.I.D.i.f.v.F.E.f.C.E.i.g.F.P.F.z.s.s.y.v.m.n.v.H.B.r.l.l.o.t.w.o.r.s.n.h.j.u.F.l.h.e.e.h.j.h.d.h.n.v.2.D.v#b.J.t.t.w.B.D.B.w.t.R.X.K.o.l.H.0.4.6.Q.C.z.z.E.T.8.L.5.Y.k.b.h.t.p.u.p.n.q.p.p.C.T.3.D.k.I#b.K.o.P.E.x.q.o.s.E.T.5.K.y.r.s.s.i.d.g.t.s.q.q.q.t.t.t.x.p.7.6.l.g.q.M.9.D.i.n.s.m.l.t.E.z.m.g.i.u.R.3.q.m.j.h.k.n.l.i.h.j.k.l.o.r.r.t.n.A.x.t.z.x.y.N.7.U.S.M.s.t.G.D.C.D.C.B.y.v.s.r.R.U.U",
".E.L.E.i.e.v.z.l.u.z.D.z.r.s.K.2.z.q.n.s.w.v.x.E.A.D.C.t.j.g.n.w.o.s.u.q.m.j.n.s.h.e.c.c.i.l.o.m.n.m#a.G.f.3.X.h.A.C.E.D.B.w.q.k.z.M.Q.B.k.j.E.Y.5.I.r.q.z.G.W#a#d.E.i.m.u.o.i.m.x.p.n.s.r.p.x.I.6#a.x.g.N#i.X.u.C.z.v.s.y.H.Q.T.o.n.r.u.j...e.s.p.r.r.r.q.r.t.t.A.0.X.k.l.w.w.9.x.m.m.w.w.n.q.E.H.W.4.Q.u.l.o.v.s.m.i.j.m.o.l.j.k.k.l.m.p.r.r.s.m.w.x.u.t.u.A.N.F.F.m.i.A.F.x.A.z.y.x.x.x.y.z.z.l.t.r",
".S.C.B.G.x.p.l.e.s.k.l.v.u.n.q.B.D.y.s.r.u.w.z.A.s.z.D.y.m.h.l.t.v.q.n.o.t.s.k.f.t.p.k.g.f.g.k.m.w.Y.F.o.1.U.g.u.D.D.B.w.u.w.v.r.e.o.H.M.w.h.j.A.C.y.y.C.A.x.D.O.1.G.u.A.D.s.f.c.s.q.s.v.w.u.u.x.E.6#g.E.h.V#f.F.u.w.u.q.t.B.A.t.p.m.k.i.d.d.l.u.n.q.s.r.p.r.w.B.S.4.g.c.w.O#c.B.h.p.v.s.x.F.w.h.k.A.W.6.U.y.g.b.s.m.j.m.o.n.m.m.m.m.m.o.q.r.s.s.n.q.u.s.n.n.w.B.u.K.v.k.C.z.n.z.w.u.t.t.x.A.D.E.h.n.i",
".M.v.F.S.C.m.l.j.k.r.t.n.r.y.s.f.E.I.E.s.o.t.w.s.t.u.v.v.s.o.l.j.q.p.o.o.p.p.m.j.C.y.s.j.d.d.j.p.V.x.i.3.O.f.I.t.v.y.y.t.u.y.y.w.s.t.E.S.V.L.z.t.q.w.C.F.B.w.u.w.y.2#b.Q.v.s.p.g.l.t.y.A.z.A.z.x.v.J#i#j.w.i.L.6.t.x.v.m.q.x.q.f.r.m.f.b.g.o.r.o.n.r.u.r.q.u.J.T.0.d.t.s.z#c.N.l.q.C.x.n.y.V.L.k.l.i.l.E.K.z.p.o.o.k.l.p.r.n.m.p.n.n.n.p.q.r.s.t.r.o.r.s.n.q.v.t.x.L.P.F.x.r.q.u.w.u.s.t.w.y.y.y.y.B.v",
".x.y.M.L.p.n.w.o.j.q.t.p.l.n.m.g.q.F.H.t.m.v.z.u.B.t.n.r.w.v.k.#.e.q.y.r.e.d.o.C.v.t.r.j.e.h.u.G.A.l.T.G.g.s.A.C.t.A.D.z.x.w.r.k.A.n.f.s.P.S.x.c.t.t.r.v.D.J.I.C.D.V.9.2.J.y.y.D.d.t.F.D.B.G.G.C.L.G.I###e.r.h.Z.z.C.w.m.o.w.r.c.f.j.j.g.m.w.t.l.o.s.v.r.o.y.S.9.f.q.k.S#..D.m.p.B.G.E.w.w.L.4#a.V.u.h.r.w.m.m.x.k.i.k.r.s.o.o.t.n.n.o.p.r.r.s.s.v.o.s.v.s.x.C.v.n.q.L.K.k.o.z.n.x.v.t.t.v.v.s.p.E.G.E",
".u.u.u.v.D.J.A.n.i.g.d.h.k.m.l.j.e.f.H.U.u.n.y.s.t.r.q.t.x.z.v.s.#.e.k.k.e.c.e.j.r.n.v.F.v.f.j.D.e.J.D.g.r.J.A.o.u.y.v.p.r.v.t.l.l.s.j.d.r.X.Y.B.e.e.w.H.x.B.I.s.u.u.J.4.7.S.z.s.p.a.n.H.x.s.z.y.H.q.n.M#a#..I.h.m.2.J.e.t.z.p.H.w.c.f.E.m.s.F.k.q.B.u.u.y.Y.8.h.o.u.R.7.Q.k.i.D.C.A.t.u.x.f.p.9#a#b.M.o.u.u.n.t.o.p.p.p.o.n.m.l.o.p.p.q.s.t.t.u.s.q.q.s.v.w.t.p.q.m.q.w.t.l.o.y.r.l.m.w.v.n.n.u.D.K.P",
".y.v.p.n.t.A.x.p.i.j.j.k.n.o.o.o.t.s.h.v.V.E.i.v.v.u.t.r.q.r.u.w.m.l.i.e.f.h.j.n.K.y.u.B.D.t.l.l.8.l.x.3.E.i.z.I.v.q.n.p.u.w.u.r.k.u.z.t.n.x.M.Y.x.c.e.B.E.D.G.C.G.E.J.Y#b#a.T.y.y.t.f.h.E.J.B.C.y.H.H.D.P.4.X.C.h.J.X.Q.x.f.k.L.D.u.u.u.o.u.x.n.C.h.p.s.9.8.e.s.m.V#b.R.n.j.w.G.v.C.x.B.G.p.e.m.J.S.E.p.u.t.p.w.r.s.s.s.r.q.p.o.q.q.q.r.r.s.s.s.q.q.r.s.u.u.s.q.i.j.t.H.I.x.t.v.p.v.x.s.p.r.p.l.I.G.F",
".w.t.o.m.q.v.t.o.k.m.n.o.o.p.q.s.y.x.g.k.R.O.r.t.r.u.w.u.q.p.t.x.x.p.h.g.k.o.m.k.f.r.p.f.q.I.x.c.H.v.m.A.Q.I.v.u.u.s.s.t.q.n.s.A.m.t.D.F.p.f.u.V.Z.t.g.n.t.A.E.z.z.C.B.A.O.6.2.O.F.F.t.h.m.x.E.I.w.L.N.y.u.J.X.0.z.f.t.S.F.j.i.k.D.U.I.q.u.x.r.v.v.r.i.9.6.i.s.n.T.2.S.p.m.D.I.t.B.G.x.u.F.C.m.g.o.B.A.v.y.v.s.z.v.v.v.v.u.t.s.r.s.s.s.r.r.q.q.q.p.r.s.s.r.q.q.r.i.i.s.G.K.C.u.t.u.y.y.s.o.o.p.o.K.v.s",
".q.r.t.v.w.w.s.o.n.p.q.n.k.j.m.q.n.o.E.A.l.F.S.u.n.r.v.y.w.t.t.t.u.q.l.m.r.r.l.f.d.l.t.v.B.K.H.x.c.N.J.o.S#c.N.o.k.u.z.s.l.q.H.T.z.r.r.y.w.l.k.r.U.V.G.f.c.w.H.x.F.K.I.z.z.P#.#i.Q.z.B.w.e.q.L.D.E.A.z.y.q.n.E.2.0.m.f.D.S.Q.B.h.t.L.L.z.y.w.r.x.u.u#a.3.h.g.y.M#c.J.i.l.D.N.D.s.E.F.y.t.x.B.A.y.w.C.C.B.A.y.x.y.x.y.y.y.x.w.v.u.u.t.t.s.q.p.p.o.q.t.u.s.o.l.m.p.u.n.l.s.w.u.s.u.y.q.p.u.r.m.s.E.A.m.k",
".p.s.w.z.z.x.x.y.n.o.o.k.f.f.h.l.h.n.M.L.j.o.K.F.A.y.u.u.v.u.s.q.o.o.m.n.o.n.n.m.M.m.j.G.H.m.p.Q.s.k.J.W.A.B.O.E.h.r.r.k.s.P.0.X.R.E.q.n.u.w.o.d.q.S.W.u.g.p.B.G.L.I.E.B.w.A.S#b.6.N.u.C.Q.L.w.v.J.w.r.y.y.p.s.E.A.C.o.l.F.O.E.u.k.m.F.O.y.o.s.v.y###..m.g.z.D#a.J.s.q.C.F.u.v.J.v.x.G.G.u.p.y.H.I.B.y.y.x.y.z.w.z.z.z.z.z.x.w.w.u.t.t.s.q.p.p.o.s.u.u.r.l.i.j.l.J.x.n.k.l.n.p.s.r.s.q.l.n.x.E.G.p.f.l",
".t.t.u.v.s.s.z.J.p.o.m.i.e.c.d.f.n.v.s.B.L.m.h.P.O.G.t.l.m.o.p.p.o.n.m.i.j.n.v.D.R.C.l.f.j.r.v.w.Z.j.d.z.F.r.y.S.G.y.m.l.H.2.V.y.V.U.H.o.m.w.y.q.f.B.R.T.H.j.g.C.J.x.s.B.E.w.v.C.7#n.T.M#e.E.b.y.C.D.B.x.B.E.t.g.c.v.s.g.t.z.p.l.n.h.B.O.u.i.u.z.2.8.j.k.D.H#..M.h.t.E.G.y.t.z.G.z.u.D.E.s.u.P.2.D.p.p.u.s.v.A.v.z.z.A.z.z.y.x.w.s.s.s.r.r.q.q.q.t.t.r.o.l.j.j.j.I.D.w.q.o.o.q.r.j.z.w.g.k.K.O.v.j.i.q",
".q.q.r.s.n.k.t.G.q.n.i.g.g.h.h.g.n.t.g.m.J.A.n.F.I.C.v.m.k.k.m.m.m.l.k.l.n.v.E.N.t.N.H.g.h.J.O.t.O.S.l.e.O.X.H.N.S.B.u.H.U.P.x.k.E.T.T.C.q.q.y.y.q.t.x.N.Y.z.f.l.G.E.D.J.N.J.C.x.S#f#g.5.S.l.b.v.p.y.C.C.A.w.n.b.t.n.x.I.F.z.u.k.t.D.A.s.q.n.w.S.9.c.r.x.F##.K.r.q.y.B.w.y.G.C.t.F.w.x.x.u.Q.7.W.s.g.p.A.t.w.D.z.y.y.y.y.x.w.v.u.q.q.q.r.r.s.s.s.r.p.m.l.l.m.l.l.s.s.s.q.o.p.q.q.v.x.w.s.y.H.z.j.o.q.t",
".h.k.r.u.n.f.k.v.s.n.h.g.j.k.l.k.g.g.p.i.g.O.Z.n.q.t.x.t.p.k.k.k.g.i.n.s.x.E.J.M.c.k.B.G.u.h.m.A.g.I.1.z.e.D.P.g.E.u.E.3.0.x.l.v.m.E.S.O.z.p.p.r.t.q.f.l.X.2.w.g.k.A.G.A.x.E.H.E.K.p.7#c.m.q.S.f.f.h.v.J.y.e.a.n.I.d.C.7.y.e.r.p.x.0.z.#.p.w.B#b.f.s.i.J.9.M.s.i.G.C.z.A.A.z.y.x.A.v.z.A.I.6.X.h.m.f.w.L.z.y.I.D.x.x.x.x.w.v.u.t.o.p.p.q.s.t.t.u.p.l.i.j.m.p.o.m.c.g.j.h.h.j.o.q.Q.o.l.N.U.t.f.j.x.v.v",
".n.l.k.l.n.n.l.i.i.p.v.r.j.e.i.o.f.o.p.g.h.u.D.z.f.k.w.B.t.j.i.o.#.u.d.f.V.O.f.m.w.P.C.F.Z.p.c.X.R.A.T.4.w.g.D.S.F.Q.Y.Q.w.l.q.y.p.p.F.Z.S.t.m.y.t.y.t.m.A.Y.0.J.b.A.B.y.O.I.q.z.w.H.D.O.9.Q.q.D.p.o.n.m.k.i.p.x.r.v.2.F.#.d.k.G.J.p.j.o.u.Q.O.g.#.l.N.3.J.e.d.B.w.x.x.v.u.w.v.s.A.y.w.H#c.T.g.F.m.V#b.Q.m.h.q.w.q.v.y.w.s.q.r.r.k.i.h.j.o.r.p.n.q.s.s.i.g.B.M.A.h.o.t.o.k.n.m.j.q.A.H.E.t.m.o.t.u.v.y",
".m.o.n.m.m.l.k.l.i.o.t.t.p.l.k.m.o.i.l.t.s.l.o.A.w.n.i.m.q.m.g.d.m.i.i.y.J.t.i.t.7.x.r.v.n.B.C.d.Q.H.p.z.V.B.i.B.I.K.J.D.w.r.s.t.w.q.q.B.M.O.D.r.l.t.u.n.g.n.I.1.C.e.n.F.q.n.C.C.x.L.E.E.4.9.N.y.x.j.d.l.q.n.l.p.y#a.m.f.z.l.E.K.i.s.g.k.X.Q.j.l.l.H.Q.D.q.t.w.s.A.x.v.x.z.z.A.B.t.I.G##.N.i.H.p.5.Y.C.f.d.r.z.u.G.z.s.r.w.z.v.p.q.p.o.o.q.r.q.p.d.h.o.j.c.k.r.i.g.j.l.m.q.v.r.k.p.s.v.y.A.B.C.D.z.w.w",
".m.n.n.l.k.k.m.n.i.l.q.v.x.v.q.m.r.m.o.w.u.j.i.q.F.t.l.o.u.w.w.w.q.q.L.Q.r.l.H.T.A.d.y.N.t.T#a.s.d.D.D.z.S.Q.v.u.I.A.s.r.v.y.v.q.u.u.k.i.z.V.R.x.v.v.B.F.q.h.A.6#g.v.j.x.y.G.Q.E.E.I.D.B.R.8.6.U.H.q.h.o.r.o.q.z.5.q.m.l.j.R.Y.m.h.e.v.P.J.q.f.b.F.Q.J.k.g.A.F.u.x.t.u.A.A.v.v.B.v.J#c.E.r.A.l.8.0.I.j.e.u.Q.J.p.C.x.t.t.w.x.t.p.s.t.u.t.s.s.t.u.f.i.v.z.o.l.n.j.m.j.f.h.q.z.y.s.s.n.l.r.D.L.L.I.y.t.s",
".n.k.h.i.m.o.o.m.l.k.m.s.A.C.x.s.m.w.w.o.m.r.o.f.t.w.z.y.r.p.z.N.o.B.M.x.l.G.X.M.c.m.w.D.G.x.x.O.n.t.T.Y.D.M.1.O.A.t.o.q.u.x.v.r.i.u.v.i.k.D.Q.O.B.n.n.B.D.q.o.v.Y.3.x.e.u.D.u.z.K.y.A.G.D.H.5#l.J.E.w.q.q.u.H.T.C.h.n.j.I.1.g.i.o.n.P.V.k.f.q.r.O.A.o.m.o.r.w.C.q.p.u.B.x.n.p.z.H#d.G.i.v.z.T#d.p.r.s.y.K.P.B.i.k.r.z.B.t.m.o.v.p.s.v.u.s.r.u.x.r.m.x.I.A.s.u.u.s.q.l.j.m.t.x.y.v.r.n.o.v.A.C.C.s.p.r",
".p.i.e.g.o.t.p.k.p.l.j.o.w.C.B.z.s.y.v.l.k.r.q.i.h.p.y.w.j.d.i.v.m.u.i.g.F.Z.F.f.t.E.f.c.F.z.n.S.4.i.g.v.q.t.H.G.o.r.u.w.u.s.t.v.f.r.z.r.i.n.F.V.R.A.n.q.C.D.s.h.n.7.9.C.m.g.k.O.K.A.B.G.v.u.O.7.z.D.A.s.v.K.Q.K.a.v.i.K.V.m.k.c.p.Q.E.h.m.n.u.0.z.i.f.s.w.n.o.D.q.r.u.x.r.n.x.M#..J.p.s.q.P#a.t.o.u.z.y.t.n.i.f.s.u.z.z.s.l.o.y.r.t.v.t.p.n.p.s.y.i.k.u.r.n.p.p.o.v.z.x.q.l.q.w.u.v.t.p.j.i.n.t.s.t.y",
".p.k.g.k.r.u.o.i.p.m.k.l.q.y.C.F.F.s.m.r.q.l.m.u.j.k.o.t.s.o.m.o.m.t.t.F.X.H.i.k.q.t.m.h.v.O.V.O.U.0.z.n.R.0.H.D.j.r.z.z.s.o.r.w.q.n.q.v.r.m.u.G.1.V.G.r.s.C.G.B.t.A.7#a.v.h.C.L.B.I.F.x.z.E.y.m.q.s.r.t.I.T.G.j.o.g.H.3.r.g.z.p.M.I.n.c.k.E.K.I.f.g.k.o.r.r.r.q.w.w.u.q.q.z.P.2.Q.f.v.v.K#c.y.h.A.y.x.v.p.k.j.n.S.B.o.r.w.v.u.x.w.w.v.s.o.l.k.k.I.p.j.o.l.m.p.m.i.r.F.J.z.p.o.s.o.s.u.p.g.e.h.n.v.y.E",
".r.r.t.t.t.r.n.k.l.m.o.m.l.r.A.I.I.u.n.s.s.m.m.u.q.k.j.r.y.A.y.y.s.t.P.2.H.l.q.D.j.j.B.A.f.j.x.s.f.O.M.p.A.G.o.e.q.s.u.t.r.q.r.u.w.p.m.r.u.s.p.r.B.N.Q.D.p.o.v.A.y.e.q.K.E.E.D.f.s.I.D.t.F.C.j.b.s.o.u.G.M.E.q.g.v.I.U.h.q.C.f.X.O.g.i.s.o.F.J.c.d.m.n.j.l.s.t.l.v.w.s.p.x.N.U.O.f.D.j.S.7.w.A.p.y.u.w.B.C.x.x.C.Q.v.j.q.z.z.v.v.y.w.u.s.r.q.o.m.K.y.v.v.q.t.x.s.n.m.t.B.E.A.x.A.l.k.l.m.l.k.k.h.q.r.v",
".s.y.D.B.v.n.l.m.h.n.q.p.j.m.x.J.A.F.y.m.m.v.u.l.r.n.k.l.l.k.p.v.F.h.A.F.f.o.J.m.C.q.q.B.v.e.n.T.5.C.R#b.K.l.w.G.y.t.o.o.r.t.s.o.t.s.q.p.r.r.o.l.l.I.5.8.Q.x.s.w.u.G.o.i.T.5.A.f.m.C.w.z.I.m.#.z.z.v.H.V.H.f.f.z.F.R.x.k.#.o.Z.B.a.d.e.u.T.D.b.a.q.n.j.l.m.n.p.q.n.s.r.r.E.T.J.l.B.i.6#f.t.i.v.O.x.v.x.A.x.v.C.Q.o.i.k.v.z.s.q.v.u.s.q.r.v.w.u.s.x.t.w.v.o.s.y.s.x.i.e.l.B.I.L.M.n.g.e.j.u.w.n.d.g.e.i",
".x.t.o.m.o.q.q.p.o.l.k.m.r.u.t.r.q.A.D.t.l.o.r.o.u.q.k.j.l.m.l.j.l.m.n.o.p.r.s.s.p.w.y.s.r.s.n.f.F.M.P.K.z.r.q.s.t.t.t.t.t.t.t.t.t.s.q.p.p.q.s.t.j.m.B.S.U.G.x.y.C.u.J.z.C.K.r.t.b.m.s.q.j.i.u.E.q.L.V.G.q.n.r.r.Y.r.e.i.o.G.I.j.j.e.B.P.k.#.k.t...l.t.k.d.f.k.l.k.m.s.O.4.D.i.w.y.0.1.w.g.v.C.s.n.A.A.m.o.K.5#a.i.k.K.T.w.m.w.t.z.n.c.e.x.M.E.l.i.o.u.t.m.j.t.G.N.M.A.i.e.j.u.v.g.o.v.v.p.k.j.k.i.g.n",
".q.q.q.t.v.w.r.o.n.n.o.q.s.s.r.p.l.w.E.F.y.t.p.k.r.p.n.n.p.p.n.n.q.q.r.s.t.u.u.v.t.q.q.w.B.y.p.h.y.z.x.v.t.r.r.r.t.t.t.t.t.t.t.t.s.r.p.o.o.p.r.s.p.p.u.G.W.0.L.u.H.y.B.v.t.x.t.y.l.j.h.h.k.m.r.v.B.s.s.A.A.o.k.p.u.l.r.r.e.l.D.C.q.w.w.x.x.p.g.d.n.h.a.a.m.x.w.p.t.i.N.5.y.l.v.k.V.R.B.m.m.y.z.r.A.A.w.u.L.3.R.r.u.p.A.K.t.c.f.t.t.w.u.s.v.x.v.n.m.n.q.s.u.t.o.h.I.N.P.I.C.w.t.o.o.r.t.r.m.j.j.k.k.i.o",
".k.n.t.z.B.z.s.n.n.q.u.v.t.q.o.n.l.r.B.I.F.v.n.m.p.r.s.s.r.r.s.s.v.v.v.v.w.w.x.x.w.r.s.w.z.y.z.D.O.F.w.t.v.x.u.r.t.t.t.t.t.t.t.t.s.r.p.o.o.p.r.s.u.v.s.u.K.3.Z.J.u.x.x.J.J.x.C.C.y.n.e.g.p.w.v.o.E.k.e.q.D.A.q.m.k.w.T.U.q.f.t.L.L.H.j.j.K.z.d.h.n.f.c.f.i.i.j.m.p.K.S.E.p.k.y.R.Y.B.k.n.v.w.v.w.v.o.s.M.1.U.x.j.p.U.V.p.b.d.l.m.j.B.M.G.s.k.n.s.s.p.n.o.A.H.v.f.l.r.A.I.G.w.n.l.w.u.p.m.k.k.k.k.n.k.q",
".m.o.t.w.x.v.r.n.r.u.x.v.r.n.n.o.u.q.t.A.z.p.n.t.p.t.w.w.t.r.r.t.v.v.v.v.v.v.v.v.s.y.B.r.m.v.O.3.Q.H.z.v.w.y.w.r.t.t.t.t.t.t.t.t.s.r.p.o.o.p.r.s.v.z.y.q.q.G.X.7.x.A.o.J.L.t.F.A.H.x.q.q.y.C.x.r.x.v.l.g.s.J.I.v.c.l.H.R.A.i.e.m.H.O.u.g.s.E.v.p.j.f.i.q.q.i.h.k.B.Y.C.e.o.C.M##.w.p.q.z.y.s.u.C.A.r.I##.2.u.r.Q.I#..P.f.f.A.t.f.g.u.I.F.s.l.n.u.u.u.o.l.t.I.J.z.n.j.j.r.q.k.k.q.y.t.m.k.l.m.m.j.q.m.r",
".p.p.o.o.p.r.u.w.v.w.w.s.n.l.n.q.y.p.n.r.s.n.o.u.s.v.x.w.s.p.p.r.u.u.u.t.t.t.t.s.q.B.A.n.p.I.X.Y.s.s.q.p.q.s.r.s.t.t.t.t.t.t.t.t.s.r.p.o.o.p.r.s.u.u.w.t.n.n.I.6.Y.Q.m.v.x.p.K.I.D.C.B.A.z.y.v.t.o.w.v.i.i.w.H.H.q.g.g.v.M.H.q.h.h.J.U.m.e.G.V.p.A.k.i.v.M.L.C.x.S.u.p.n.k.P.4.y.g.o.x.A.x.v.v.x.w.v.G.R.F.k.m.D#b.M.g.h.K.I.m.j.o.o.r.s.s.q.p.o.n.w.y.o.k.s.D.I.P.B.q.o.m.j.o.z.t.p.k.l.o.q.o.j.r.n.r",
".o.m.k.j.l.r.z.E.x.v.s.p.m.l.o.p.s.n.m.r.u.t.s.s.u.v.w.u.r.q.p.p.t.t.t.s.s.s.s.s.s.v.t.t.J.Y.S.z.q.u.w.u.r.p.q.t.t.t.t.t.t.t.t.t.s.r.p.o.o.p.r.s.u.n.p.x.w.r.u.H.4.Z.K.M.O.Q.0.X.z.A.C.y.t.o.o.q.n.l.n.q.h.d.o.K.T.A.e.h.L.V.G.w.f.h.K.T.p.n.F.y.I.B.w.y.D.F.I.N.t.j.i.u.N.X.F.f.q.v.t.q.t.B.A.q.q.A.u.j.w.T.J.j.K.B.s.u.A.t.l.r.y.n.g.l.s.t.o.k.j.v.E.y.l.i.l.v.M.C.t.o.n.p.t.z.o.m.k.m.p.r.p.l.r.m.q",
".q.p.o.n.m.q.w.B.u.t.o.o.o.o.n.n.o.o.n.n.w.D.E.z.y.w.t.s.s.u.t.s.u.u.u.u.u.u.u.u.x.p.r.J.V.P.z.o.z.B.B.y.u.r.r.t.t.t.t.t.t.t.t.t.s.r.p.o.o.p.r.s.t.q.r.u.w.u.s.r.H.R.5.Y.X.4.U.P.F.B.y.u.s.q.q.p.n.h.j.q.n.d.f.r.G.J.n.b.j.x.A.H.w.c.k.T.R.k.i.z.v.H.I.u.h.h.s.E.j.u.l.x.1.I.e.p.w.w.s.q.t.z.y.t.y.K.E.o.E###..I.g.F.2.L.k.k.v.s.x.p.k.o.q.o.p.r.q.q.v.E.C.q.m.q.o.q.n.n.q.u.u.q.n.n.l.m.o.q.q.o.q.l.p",
".v.v.w.t.q.p.p.r.s.q.m.o.r.r.n.k.p.q.m.i.t.J.P.J.y.u.r.r.u.w.v.t.w.w.x.x.x.x.y.y.B.q.y.V.S.t.l.x.o.o.n.n.o.p.r.t.t.t.t.t.t.t.t.t.s.r.p.o.o.p.r.s.q.A.B.p.k.q.v.t.u.F.3.D.q.C.l.n.T.G.t.r.v.z.w.r.m.r.p.k.q.x.m.a.k.P.K.h.d.o.H#..w.B.i.g.K.J.j.g.k.s.s.i.e.j.m.j.S.x.J.W.w.g.m.y.o.s.x.B.w.s.v.D.q.C.E.r.j.t.N.1.r.u.S.4.G.r.u.q.p.p.r.v.p.k.s.D.A.k.k.F.T.L.B.C.l.t.t.r.t.y.t.i.p.o.m.l.l.n.p.q.o.k.n",
".p.q.s.t.t.t.s.s.p.x.z.t.q.r.s.q.k.r.v.o.e.g.w.O.C.s.q.A.E.x.u.y.t.s.q.q.o.p.p.q.p.y.G.H.C.v.t.t.s.r.q.q.p.p.r.s.p.q.q.p.p.p.p.q.p.q.q.r.r.s.t.t.s.s.s.s.s.s.s.s.r.q.y.P.P.H.N.2.J.6.V.v.q.l.f.n.p.o.q.r.u.t.p.k.#.w.S.P.u.h.k.r.P.u.j.h.l.N.U.r.j.h.h.f.f.f.e.f.s.Q.Q.q.g.p.u.j.o.p.r.s.v.w.y.y.v.s.v.z.o.g.t.T.4.p.h.B.R.z.m.t.m.p.o.k.f.g.o.w.t.i.j.j.i.C.N.u.j.l.o.p.p.n.m.k.l.l.l.m.m.m.n.n.o.m.r",
".q.q.s.u.v.u.t.r.x.z.v.p.n.m.i.c.g.y.L.I.t.m.v.J.U.O.F.B.E.H.D.w.u.t.r.q.o.p.p.p.q.r.s.r.q.p.p.q.q.p.o.p.p.q.s.t.p.q.q.p.p.p.p.q.p.p.q.q.r.s.s.s.s.s.s.s.s.s.s.s.t.p.s.G.N.K.G.J.D.h.C.S.s.m.y.m.l.l.m.o.o.p.q.o.r.j.q.H.H.o.d.c.K.O.U.F.m.B.U.O.j.i.i.h.g.f.f.f.s.z.A.s.m.p.t.u.t.t.t.s.r.s.v.x.x.u.x.D.A.q.o.w.1.N.t.o.K.3.L.f.n.p.o.l.i.h.m.q.G.x.A.z.m.q.v.i.l.m.o.o.o.m.l.k.l.m.m.m.m.n.n.n.o.m.r",
".r.r.r.u.x.x.t.p.t.t.p.n.q.s.p.j.b.n.D.I.E.v.p.m.G.O.M.x.r.y.C.y.w.v.t.r.q.q.p.p.v.p.j.g.i.m.p.q.o.o.o.p.q.q.s.s.p.q.q.p.p.p.p.q.o.p.p.q.r.r.s.s.r.r.r.r.r.r.r.r.v.p.p.w.G.I.y.n.A.z.A.K.L.t.g.k.n.o.p.o.n.p.r.t.t.d.d.x.X.0.K.u.g.x.S.J.i.h.v.y.k.j.j.h.h.h.g.g.o.h.l.z.C.s.o.t.w.x.w.s.o.o.s.v.B.w.v.z.F.C.r.g.l.P.Q.o.l.G.I.l.p.p.o.m.l.l.k.k.A.x.G.I.s.l.n.l.p.p.o.m.l.k.j.j.m.m.m.n.n.n.o.o.p.n.t",
".u.s.r.u.y.y.u.p.m.q.t.u.u.v.z.C.k.f.g.w.N.O.x.g.v.Q.3.S.A.w.E.L.z.y.v.t.r.p.o.n.u.q.l.h.h.k.n.p.o.o.o.o.p.q.q.q.o.p.p.o.o.o.o.p.o.o.o.p.q.q.r.r.q.q.q.q.q.q.q.q.s.u.s.p.u.y.s.g.A.Z.N.y.O.K.q.u.s.u.v.u.s.r.s.u.s.u.m.g.v.S.M.m.i.l.C.S.L.u.n.o.j.i.i.g.g.f.h.h.h.c.i.E.N.B.o.m.r.t.v.t.p.o.s.w.z.y.t.p.w.F.A.o.e.z.U.O.r.k.y.S.v.r.o.m.n.m.m.j.m.l.v.D.w.m.n.u.q.p.n.k.j.j.j.j.o.o.o.o.p.p.p.p.s.q.w",
".w.u.t.u.x.x.u.r.o.w.C.z.o.j.r.D.A.o.i.t.N.X.P.C.u.F.U.0.P.y.w.D.z.z.y.w.t.q.o.o.m.o.q.o.j.g.i.l.p.p.o.o.o.o.o.o.o.p.p.o.o.o.o.p.n.n.o.o.p.q.q.q.q.q.q.q.q.q.q.q.p.x.w.l.h.m.q.m.d.d.q.w.j.B.Q.s.r.s.u.v.t.r.r.r.k.C.B.g.e.v.L.J.G.o.p.R.3.G.n.s.j.i.h.f.e.e.e.e.e.d.h.v.H.G.x.q.m.p.t.u.s.s.t.v.s.y.x.o.o.x.A.v.r.g.l.J.M.q.m.B.C.v.n.k.l.n.n.m.k.i.j.q.t.k.i.s.p.n.l.j.i.j.k.l.p.p.p.p.q.q.q.q.w.t.y",
".x.w.v.u.u.u.v.w.n.s.w.t.j.d.j.w.y.w.t.s.u.D.P.Y.v.k.n.H.U.M.z.u.A.z.y.y.v.s.q.o.d.j.p.o.i.e.f.i.n.n.o.o.o.n.m.n.n.o.o.n.n.n.n.o.m.m.n.n.o.p.p.q.p.p.p.p.p.p.p.p.o.v.v.k.d.j.r.t.p.j.x.R.S.N.R.W.p.o.o.p.r.s.r.o.j.q.y.x.l.l.M#e.O.y.l.x.G.l.d.p.h.h.f.e.e.d.e.e.g.g.e.g.r.F.K.H.o.p.q.r.t.u.v.u.m.w.A.r.m.n.p.n.q.e.f.r.A.q.f.d.D.w.o.k.k.n.o.o.j.k.f.g.q.n.j.q.l.l.j.j.j.l.n.o.o.o.p.p.p.q.q.q.y.v.y",
".y.z.y.u.q.r.w.A.s.l.i.l.p.n.m.q.z.E.E.v.k.m.E.V.R.A.p.y.R.0.T.J.z.z.z.y.x.t.r.p.i.k.m.m.l.m.p.r.i.j.m.o.o.p.o.o.n.o.o.n.n.n.n.o.m.m.m.n.o.o.p.p.o.o.o.o.o.o.o.o.o.o.l.h.i.o.r.t.l.u.e.h.I.s.g.B.G.z.r.p.t.v.u.q.z.r.r.v.o.d.h.y.R.V.D.p.s.l.f.o.i.i.g.f.f.e.g.g.h.g.e.e.l.w.H.P.A.v.o.n.p.s.r.q.o.r.r.p.n.m.j.h.m.A.G.x.n.m.p.q.x.u.o.m.j.l.m.m.m.v.m.e.q.r.k.q.i.i.i.j.k.n.n.o.p.p.p.q.q.q.q.r.y.t.v",
".y.A.A.u.o.o.w.E.J.q.d.k.y.y.p.h.S.U.Q.F.v.u.D.O.5.W.C.m.q.G.L.E.A.z.z.z.y.u.s.q.s.o.l.l.q.w.B.E.e.g.k.n.p.q.q.q.n.o.o.n.n.n.n.o.l.l.m.n.n.o.o.p.o.o.o.o.o.o.o.o.q.g.a.f.q.u.u.q.C.o.k.h.g.E.Q.v.3.Q.B.v.y.B.y.u.C.t.n.r.D.G.s.c.K.5.N.k.p.w.k.d.l.k.i.h.g.g.h.h.g.e.g.n.n.o.x.K.L.A.n.j.l.q.o.m.v.n.f.h.m.m.j.i.e.y.G.s.e.d.n.x.s.r.q.m.j.j.i.j.y.M.z.k.q.q.f.i.f.g.h.j.l.o.p.p.p.p.q.q.q.r.r.r.x.s.t",
".r.r.q.r.t.u.r.q.u.n.n.s.s.n.o.v.e.u.J.M.L.K.y.j.l.U.U.r.g.i.H#f.P.y.F.I.q.s.E.v.r.v.q.t.s.F.Q.b.e.i.p.q.n.k.p.t.p.p.o.o.o.o.n.n.m.m.m.m.m.m.m.m.w.m.i.p.u.s.p.q.d.a.o.z.p.r.B.v.q.t.u.w.v.y.E.K.t.N.R.w.l.u.x.n.t.t.s.x.J.R.G.r.t.s.r.p.n.m.k.k.i.i.i.i.i.i.i.i.j.j.j.j.j.k.k.j.I.L.F.p.i.m.p.m.e.n.o.g.f.m.k.e.A.N.w.c.h.s.A.R...d.k.l.j.i.l.n.U.C.m.e.f.g.i.l.l.l.l.m.m.n.n.n.n.o.q.t.u.v.v.v.s.v.B",
".s.r.r.u.x.w.q.l.o.l.m.r.r.o.o.q.F.C.M.4.7.R.E.C.o.H.Y.3.M.n.q.P.W.M.w.p.A.E.A.x.D.w.D.m.P.W.b.A.n.t.t.q.o.r.r.m.p.p.o.o.o.o.n.n.m.m.m.m.m.m.m.m.f.g.j.p.u.u.o.h.j.j.w.y.m.t.C.u.r.t.s.r.p.q.t.x.d.s.O.S.A.m.u.M.v.v.u.y.J.Y.5.3.s.r.q.o.n.l.k.j.i.i.i.i.i.i.i.i.k.j.j.j.j.k.k.j.y.G.J.A.q.m.m.o.m.g.i.o.l.e.k.z.M.f.j.v.m.B.L.n.c.e.n.r.j.g.p.K.v.p.n.o.r.n.m.n.m.m.m.m.m.n.n.n.o.p.s.u.v.v.v.v.v.v.A",
".u.s.s.v.y.w.p.i.l.p.r.q.r.t.r.o.B.u.x.N.U.I.y.x.t.n.A.S.N.v.k.h.I.5.P.v.B.u.m.C.v.B.q.4.U.f.t.j.O.J.w.h.i.p.p.i.p.p.o.o.o.o.n.n.m.m.m.m.m.m.m.m.g.m.q.o.q.r.n.h.m.s.C.A.s.z.D.q.t.t.t.r.p.p.r.t.A.u.H.1.U.s.l.z.s.w.x.s.q.y.F.L.q.p.o.n.m.l.k.j.i.i.i.i.i.i.i.i.k.k.j.j.j.k.j.k.n.w.I.J.z.l.i.o.l.g.i.o.k.h.r.L.t.g.d.u.N.E.l.g.j.k.l.l.n.t.E.L.e.h.r.z.z.q.l.m.m.m.n.n.n.n.n.n.p.r.t.u.w.w.v.w.z.v.z",
".u.w.x.w.u.s.p.n.n.u.w.q.s.y.z.t.y.F.G.C.H.T.U.L.J.s.q.y.J.1.Q.f.l.S.Z.L.E.x.u.D.K.t.2.2.n.i.s.H.0.H.l.e.f.m.q.s.p.p.o.o.o.o.n.n.m.m.m.m.m.m.m.m.p.q.p.p.k.j.t.E.t.u.A.E.E.D.A.t.w.w.w.w.w.y.z.B.O.i.e.r.V.W.G.s.E.Q.Y.R.C.p.l.l.o.o.n.m.l.l.k.k.j.j.j.j.j.j.j.j.l.l.k.k.l.l.k.k.k.m.w.D.x.j.f.m.b.k.n.g.i.v.B.v.b.s.r.w.L.r.b.m.k.n.j.f.v.R.L.p.i.p.z.D.v.k.i.l.o.o.o.o.p.p.p.p.q.s.v.x.w.w.w.v.C.v.y",
".t.y.z.u.n.l.q.v.n.t.t.n.o.x.B.v.B.K.L.D.H.V.6.5.G.s.r.v.C.X.X.s.f.i.A.L.J.T.V.C.N.7.X.j.l.C.E#e.C.l.g.s.x.p.p.y.p.p.o.o.o.o.n.n.k.k.k.k.k.k.k.k.k.e.i.o.k.e.w.W.K.A.u.D.J.B.w.B.x.x.x.y.B.B.D.E.9.P.p.h.E.7.1.E.u.J.Z.3.Q.y.p.o.n.n.n.m.m.l.l.l.j.j.j.j.j.j.j.j.l.l.k.k.l.l.k.k.k.f.i.q.o.g.e.k.b.h.j.h.s.C.u.h.h.j.J.I.h.e.s.d.i.j.k.t.I.L.x.k.z.A.A.w.m.f.i.p.o.o.o.p.q.q.r.r.u.v.x.x.x.x.x.w.B.v.z",
".r.v.v.q.j.j.r.z.l.o.o.m.q.u.y.x.z.t.v.C.B.x.I.1.L.v.r.s.j.j.D.W.q.e.t.F.u.G.W.H.u.V.i.i.w.F.5.B.h.i.y.R.N.p.e.i.p.p.o.o.o.o.n.n.j.j.j.j.j.j.j.j.h.c.g.n.m.g.o.E.Z.R.B.x.C.x.t.H.y.y.w.x.y.y.x.v.4#h.9.z.j.z.Q.Q.j.m.v.C.y.q.n.p.n.n.n.n.n.m.m.m.k.k.k.k.k.k.k.k.m.m.l.l.l.m.l.l.k.e.c.f.h.g.f.i.j.b.f.v.B.q.h.g.r.w.y.m.e.k.o.e.j.e.t.O.I.k.h.u.H.A.t.n.i.f.l.t.m.m.n.o.q.r.s.t.v.w.x.y.y.y.w.v.x.x.A",
".q.r.q.o.n.o.s.w.p.o.s.z.B.A.y.A.H.z.A.H.D.t.v.G.Z.T.F.w.n.h.z.5.J.y.w.w.n.i.r.C.w.c.y.B.x.1.x.c.i.A.S.R.z.l.g.i.p.p.o.o.o.o.n.n.j.j.j.j.j.j.j.j.k.k.i.i.q.z.t.i.L.6.X.D.B.z.x.E.A.z.x.x.x.x.t.r.g.F.Y.K.j.h.A.X.L.B.t.s.t.r.p.p.n.n.n.n.o.o.o.o.k.k.k.k.k.k.k.k.m.l.l.m.l.m.l.m.g.j.i.f.f.g.h.g.k.j.u.D.v.i.f.n.v.H.i.b.q.n.#.n.m.q.D.I.t.e.j.D.E.u.m.j.l.k.n.r.k.l.n.p.r.t.u.v.x.y.z.z.z.y.x.w.s.y.B",
".p.n.k.l.q.u.t.r.w.u.A.N.R.I.E.I.E.M.M.D.z.z.q.g.D.T.H.s.z.z.n.m.0.J.h.g.C.p.a.m.y.B.o.O.7.C.n.p.m.O.W.x.f.j.C.J.p.p.o.o.o.o.n.n.j.j.j.j.j.j.j.j.i.n.g.c.w.1.S.m.o#.#g.N.G.I.B.y.C.B.y.A.B.C.y.v.n.y.X#b.Y.u.u.Q#f.U.z.u.u.s.n.n.n.n.n.o.o.p.p.p.k.k.k.k.k.k.k.k.l.l.m.m.l.m.l.m.b.n.q.j.f.i.j.f.b.A.O.y.j.h.n.t.I.k.h.o.b.b.m.i.p.J.J.l.f.p.A.x.y.o.i.k.p.o.k.j.j.k.m.p.r.u.w.x.z.z.A.B.A.y.x.w.p.z.C",
".e.f.t.C.p.d.i.G.w.t.T.Z.E.S.X.k.B.C.D.D.A.x.v.t.m.N.3.Q.w.o.o.n.u.Q#..T.x.O.V.s.N.W.F.2.C.c.J.p.R.Z.x.d.r.O.P.Q.d.c.d.h.n.p.m.i.j.i.h.i.i.i.i.h.a.a.t.A.f.k.O.Y.k.y.3.3.y.v.E.m.v.x.x.x.w.v.v.u.L.b.j.U.T.B.r.g.x.v.t.q.p.p.o.o.r.r.r.r.r.r.r.r.q.q.p.p.p.p.o.o.l.m.o.q.s.p.g.b.#.f.l.e.c.j.j.c.s.z.w.i.c.n.z.D.a.c.d.f.f.g.i.k.J.A.j.a.h.B.G.x.d.g.l.r.r.p.m.i.e.x.J.z.i.e.o.y.D.y.u.y.E.E.v.m.q.q.p",
".n.p.v.w.v.w.G.P.L.B.I.E.s.N.9.Y.C.B.A.y.v.u.u.t.p.m.v.K.K.x.p.u.u.D.V.V.I.S.2.Q.I##.O.B.z.e.l.Y.Z.j.k.C.A.N.R.j.k.i.h.j.l.n.o.n.i.h.h.h.h.h.h.g.f.c.m.G.B.h.p.3.7.s.i.S#c.O.u.E.w.w.v.w.x.w.u.t.u.z.e.l.Z.U.u.C.v.u.s.q.q.q.p.q.r.r.r.r.r.r.r.r.p.p.o.o.o.o.n.n.j.l.n.n.m.l.l.m.m.c.a.i.k.g.k.w.D.r.e.d.o.y.x.p.a.h.k.i.c.e.o.B.D.n.e.j.y.D.u.k.g.j.m.n.o.n.k.k.g.q.s.i.g.q.w.t.C.E.E.A.w.x.D.J.z.u.o",
".w.y.u.q.y.M.R.N.K.E.x.q.o.D.2#g.O.K.E.y.t.s.u.u.y.j.i.E.X.U.I.C.n.f.p.z.r.p.w.z.o.p.D.m.i.O.5#j.z.i.h.L.3.I.i.j.t.r.o.l.j.k.o.r.j.h.g.g.g.f.g.f.A.s.c.s.V.z.g.z.3.Y.o.j.W.3.A.r.u.r.r.u.y.y.w.v.y.v.q.n.E.5.5.G.u.s.r.p.p.r.s.t.r.r.r.r.r.r.r.r.p.p.o.o.o.o.n.n.l.o.o.l.i.h.n.r.w.g.b.h.n.o.v.H.v.j.d.l.z.A.p.e.g.j.i.h.f.l.w.E.n.f.j.A.G.u.g.d.m.l.l.l.l.l.n.n.E.B.q.j.s.H.D.o.y.F.I.z.n.n.C.T.z.t.l",
".u.u.t.v.C.K.F.v.B.G.A.v.w.n.r.O.3.X.N.D.v.t.v.w.x.w.s.t.J.Z.R.z.z.p.p.v.s.m.p.w.C.k.w.q.u#a#c.z.g.z.E.N.V.r.c.w.v.u.s.o.i.h.k.n.l.i.g.f.e.d.d.d.v.D.m.g.F.R.A.k.l#f#..i.i.S.W.z.t.s.p.s.y.A.y.w.y.w.G.w.i.I.U.j.s.r.q.p.r.s.t.u.r.r.r.r.r.r.r.r.p.p.o.o.o.o.n.n.o.p.p.m.j.h.j.j.m.p.k.f.o.C.E.t.d.h.t.G.B.l.d.f.o.h.c.h.t.A.w.o.b.m.B.H.u.f.c.j.o.n.l.j.k.n.s.u.L.A.r.u.E.G.x.o.s.x.A.v.o.n.u.C.p.p.l",
".p.o.v.G.J.B.r.m.w.G.C.B.F.l.g.u.0.U.L.C.v.t.v.v.o.v.t.k.l.y.C.y.F.G.C.A.D.G.H.K.G.O.q.F#h.4.t.f.v.F.4.S.f.h.C.o.r.s.s.p.j.f.f.g.m.k.h.f.d.c.d.c.c.u.I.r.f.F.X.G.i.C.5.Z.r.s.Y#c.A.w.t.t.v.w.w.v.y.6.1.G.I.x.i.s.s.r.q.p.q.s.u.v.r.r.r.r.r.r.r.r.p.p.o.o.o.o.n.n.m.n.n.p.q.n.i.d.e.l.o.o.x.E.v.g.c.m.D.J.u.e.b.p.j.g.i.r.A.A.m.d.g.x.F.v.i.e.g.k.p.m.k.k.m.r.w.A.m.e.j.B.E.p.k.r.q.o.o.v.D.D.t.j.j.r.u",
".r.s.C.N.M.A.s.u.o.v.q.w.M.I.E.V.E.B.z.v.s.s.u.u.t.q.t.w.p.h.t.O.x.J.D.s.z.E.C.E.E.x.x#e#k.A.n.t.R.1.V.t.f.t.y.j.o.o.q.p.l.h.e.c.l.i.g.e.e.e.g.g.s.j.A.F.j.r.O.I.A.e.w.3.E.j.F.Y.E.B.w.u.s.t.w.A.w.M.W.V.K.t.q.C.t.s.r.p.p.r.s.t.r.r.r.r.r.r.r.r.q.q.p.p.p.p.o.o.k.k.j.o.r.r.l.f.i.e.i.B.G.t.h.f.w.v.u.t.m.i.o.x.g.o.A.C.u.j.g.i.x.z.r.g.f.p.o.d.n.l.l.l.n.u.z.C.i.h.v.L.D.h.f.w.r.m.m.y.M.O.z.i.p.A.E",
".B.F.H.D.A.B.D.D.o.p.m.w.T.2.1.6.m.o.r.t.u.u.u.s.y.u.x.E.B.t.z.P.F.T.M.C.H.F.y.G.V.B###e.w.j.w.H.V.0.s.e.C.C.i.p.p.o.n.p.o.m.h.d.h.f.d.e.f.i.m.n.O.l.g.F.M.y.o.q.z.A.A.w.w.E.B.m.w.w.v.r.s.w.I.P.x.g.t.H.j.g.x.o.w.v.s.q.p.q.q.r.r.r.r.r.r.r.r.r.q.q.p.p.p.p.o.o.o.n.j.i.j.l.n.n.o.f.i.w.A.q.g.h.H.B.p.i.l.v.C.D.p.A.G.y.k.e.l.y.C.r.f.e.m.t.o.e.k.j.l.m.p.s.v.z.q.z.J.J.x.m.o.y.v.s.r.u.z.A.w.q.u.C.B",
".J.R.I.o.j.z.I.E.B.z.y.H.X.2.R.E.j.n.t.y.z.x.v.s.o.v.v.q.w.G.D.s.A.N.H.E.L.C.v.L.C.4#r.n.e.G.z#c.0.j.g.x.z.w.x.o.s.o.m.o.q.q.m.h.d.b.b.d.h.m.r.t.v.o.g.D.1.D.e.v.t#g.4.k.z.6.U.u.k.n.q.p.s.D.U.6.X.R.v.p.D.t.i.v.x.v.t.q.p.p.o.o.r.r.r.r.r.r.r.r.p.p.o.o.o.o.n.n.w.s.m.e.a.c.m.t.m.t.q.i.l.v.t.h.A.A.v.l.p.B.I.G.K.H.z.n.g.h.u.G.u.j.e.n.q.k.h.l.i.j.k.m.n.q.t.t.h.x.D.q.j.r.y.y.x.z.x.o.e.e.m.v.u.y.s",
".t.w.A.C.t.i.h.n.B.D.N.W.R.C.x.E.I.D.x.w.y.A.A.z.r.w.A.B.z.x.z.C.F.x.I.M.z.C.L.z.9#c.H.g.v.U.X.Z.b.q.z.u.s.x.u.j.n.i.m.w.t.j.n.z.l.b.i.r.f.n.J.H.e.o.T.S.o.l.x.s.9.W.g.j.8.W.h.p.D.r.p.y.x.p.u.J.R.G.t.p.t.w.u.q.u.t.s.r.q.q.q.q.q.q.q.q.q.q.q.q.t.o.x.w.k.v.B.i.m.y.E.u.n.p.o.l.h.s.q.b.f.z.H.u.b.m.C.M.N.C.p.h.u.x.o.f.j.B.E.r.d.i.n.p.m.l.l.m.n.o.o.m.k.k.o.r.G.J.i.i.U.E.e.x.n.s.u.q.k.i.o.w.v.v.A",
".x.x.D.K.I.A.v.x.y.L.U.P.F.y.A.E.J.F.B.y.x.x.w.v.v.v.w.y.A.A.A.A.t.O.F.B.Q.D.B#d#d.r.l.H.N.4.1.n.p.z.B.w.s.t.s.k.p.s.n.h.l.t.o.e.q.p.h.h.C.P.B.h.m.T.M.s.u.l.u#b.J.h.u.8.Z.j.e.B.s.o.u.J.O.D.s.r.G.z.s.r.u.w.t.p.u.t.s.r.q.q.q.q.p.q.q.r.r.q.q.p.n.j.r.t.i.m.s.f.i.q.t.t.t.t.s.n.s.m.l.p.o.p.z.O.c.e.l.s.w.u.q.l.m.r.v.v.w.v.p.i.f.j.o.o.n.l.k.k.m.o.q.q.o.n.o.q.N.O.t.k.K.1.J.l.r.v.y.w.s.o.p.s.o.q.x",
".s.o.p.w.A.z.y.z.F.P.P.D.x.B.C.x.x.y.z.z.z.z.A.C.z.v.t.v.A.D.B.z.B.A.F.I.F.P.6#b.H.m.k.T#f.W.s.l.B.C.C.z.v.r.p.o.r.s.n.m.y.M.D.j.A.N.v.m.L.I.k.i.S.K.B.r.o.F.W.Z.h.t.Y.1.x.i.t.y.w.x.D.Q.2.0.O.B.t.s.s.u.v.v.s.q.u.t.s.r.q.q.q.q.p.q.r.s.s.r.q.p.p.i.q.w.q.t.E.F.m.j.n.u.y.w.r.r.x.n.n.t.p.h.r.K.A.t.n.l.l.n.m.l.j.l.w.H.D.o.h.h.g.j.n.p.o.m.k.k.l.o.r.t.s.q.p.p.n.z.N.B.i.x.L.v.s.t.u.w.w.t.q.o.o.s.A",
".t.n.h.h.l.r.y.D.Q.K.B.v.A.G.z.n.r.v.z.B.z.z.B.D.z.w.u.v.y.B.B.C.J.u.B.D.D.8#e.C.h.I.V.1.1.t.f.F.D.A.z.B.y.r.p.s.s.m.r.H.Q.J.F.I.u.J.I.A.u.h.n.U.V.r.o.z.H.4.Z.h.s#a.Y.h.m.D.x.w.C.B.x.u.D.O.M.C.p.r.u.v.u.t.s.s.u.t.s.r.q.q.q.q.p.q.s.t.t.s.q.p.x.p.r.x.u.u.F.R.v.p.o.w.y.s.q.t.v.w.u.p.j.j.m.m.T.I.w.o.m.l.l.k.l.m.r.x.w.o.n.p.i.k.n.p.p.o.m.l.m.o.q.r.r.q.q.q.i.s.U.0.t.f.z.0.v.r.o.o.r.t.r.p.u.z.H",
".E.C.x.r.r.x.G.M.O.D.w.z.B.w.s.r.y.B.E.C.x.t.s.t.v.x.y.x.w.x.B.F.A.L.t.u.3.1.x.s.w.O#h.4.h.h.F.A.A.x.y.A.z.s.p.r.o.t.H.P.E.n.p.C.j.n.C.y.g.p.U.4.s.v.m.E#..R.l.t.6.Y.s.h.y.w.m.z.S.U.M.x.r.y.x.r.t.u.v.u.s.r.s.u.u.t.s.r.q.q.q.q.q.r.s.t.t.s.r.q.y.v.u.x.u.k.j.t.B.v.u.y.w.p.r.A.t.u.t.t.w.w.t.m.C.w.q.n.m.m.l.l.k.m.m.k.o.x.A.w.m.m.n.o.p.p.o.o.o.n.m.m.o.q.r.s.v.n.n.K.S.v.l.K.E.y.r.n.o.r.t.u.v.z.G",
".A.E.G.D.B.C.E.F.y.x.B.E.v.i.n.E.y.A.B.A.x.u.t.s.t.x.A.z.w.w.A.F.G.z.E.W.V.t.j.A.X.3.V.v.k.B.J.t.z.y.z.z.z.x.t.p.i.E.R.E.n.l.o.p.r.l.t.q.o.W.5.r.m.o.G.2.O.h.n.7.0.g.n.K.t.p.A.p.O.Z.2.U.H.C.y.s.y.x.w.t.r.r.s.u.u.t.s.r.q.q.q.q.s.s.s.r.r.s.s.s.s.z.z.C.H.v.j.m.A.x.y.y.v.u.x.F.w.j.m.G.O.H.C.J.o.n.n.n.m.l.i.i.f.k.k.j.q.D.G.A.u.r.o.m.n.o.p.p.p.n.l.l.n.q.s.s.r.o.d.k.K.I.n.k.N.K.E.y.s.r.u.y.p.r.w",
".o.t.w.x.z.B.y.r.t.w.z.v.o.m.u.E.v.v.v.w.y.y.x.w.u.w.x.z.z.z.A.B.L.y.Y.8.t.j.G.M.Z.0.p.f.I.M.r.x.y.B.A.v.x.E.D.w.y.I.H.v.q.x.y.s.D.v.u.C.O.U.F.i.A.y.0.Z.k.n.U.W.t.e.A.I.k.B.T.p.r.C.K.J.C.w.u.u.y.w.v.t.t.t.s.r.u.t.s.r.q.q.q.q.u.t.r.q.q.r.t.u.q.B.x.v.L.K.z.B.x.z.z.w.y.C.B.x.u.o.s.E.K.I.H.L.q.s.u.r.n.j.h.j.h.h.l.q.u.x.C.I.D.y.q.l.k.l.n.m.p.n.m.o.r.t.t.s.w.z.C.x.t.H.M.u.B.I.M.H.x.p.r.w.n.l.o",
".q.r.o.p.x.E.B.s.w.u.o.h.o.C.D.t.C.z.v.v.x.w.t.q.w.v.u.y.C.D.A.x.w#..7.A.t.B.M.4.Y.m.j.E.B.x.A.w.v.B.A.r.x.M.Q.I.4.G.r.v.B.y.v.v.y.v.w.Y#..v.h.O.y#g.2.i.v#..Y.m.k.y.z.r.y.L.P.R.z.z.A.z.t.r.y.F.w.u.t.u.v.u.r.o.u.t.s.r.q.q.q.q.v.t.r.p.p.r.t.v.t.C.l.d.t.D.x.C.w.z.y.v.A.G.y.k.o.F.E.p.p.E.G.s.o.r.u.r.l.i.l.r.p.h.l.v.u.m.x.S.K.C.s.l.j.j.k.k.o.n.o.r.v.w.t.q.E.k.C.D.f.t.S.t.j.w.L.L.z.n.n.s.t.m.m",
".j.l.p.u.A.B.u.m.l.t.u.q.s.C.E.z.G.z.u.v.x.w.w.y.u.A.n.s.U.P.u.x.Z.S.v.t.N.M.F.R.f.q.C.G.B.v.v.w.A.D.u.C.2.4.V.3.R.l.n.x.l.p.F.H.r.x.X.X.p.h.x.E.6.2.j.j##.0.j.r.v.u.u.w.v.z.N.4.U.F.u.t.w.v.w.y.v.u.u.t.t.s.r.r.o.v.y.t.m.m.s.x.D.q.m.x.B.v.w.E.q.G.J.v.p.y.C.w.x.w.z.E.D.w.w.y.B.M.V.S.F.s.p.q.y.v.s.p.n.m.o.o.o.p.s.v.v.u.u.t.I.D.y.u.r.o.o.p.l.q.v.t.p.p.w.D.t.x.w.v.D.M.F.p.t.p.u.G.F.t.o.u.t.o.j",
".s.w.z.B.z.w.r.n.p.r.r.p.r.v.x.w.o.y.y.r.x.M.O.D.s.p.F.U.F.l.v.Q.L.L.y.A.O.G.s.y.u.x.z.z.x.w.y.A.u.x.B.T.3.H.z.X.O.E.u.z.L.K.A.x.w.9.Z.u.t.r.G#n.Z.f.v#b.P.h.p.D.u.t.z.D.C.w.v.A.L.B.w.w.y.v.v.x.v.v.u.u.t.s.s.s.r.p.o.q.t.u.v.v.h.g.n.w.v.l.e.g.g.t.y.r.n.r.t.r.t.v.B.E.w.m.n.u.n.u.E.J.I.F.E.E.y.w.s.p.n.o.p.q.s.s.r.s.s.t.v.v.t.I.N.B.o.m.n.m.n.u.z.A.y.w.A.E.w.v.x.E.H.E.x.t.v.C.A.r.s.B.B.t.o.r.s",
".I.I.H.B.s.n.p.s.t.r.s.w.y.x.y.B.m.u.y.A.H.Q.R.L.E.q.K.U.p.j.I.P.r.w.u.A.K.E.s.v.E.C.z.y.y.z.z.A.t.I.T.Z.S.y.u.Q.E.S.A.t.O.K.t.F#d.1.H.p.u.W.8.V.y.j.Z#a.k.g.J.v.y.A.I.T.V.N.B.t.B.y.y.z.y.u.t.v.w.v.v.u.t.t.s.s.x.l.h.r.A.y.t.r.j.p.w.y.t.o.m.n.e.k.r.t.q.o.p.s.t.A.F.E.w.t.C.P.p.p.s.z.H.L.I.E.y.w.s.p.o.p.q.s.v.t.s.q.r.s.v.w.m.w.G.I.C.v.s.s.t.y.D.G.F.E.E.E.w.z.I.O.D.o.r.G.F.G.x.k.m.A.E.y.l.v.y",
".K.F.z.s.m.k.q.y.v.s.w.H.K.F.E.K.E.s.x.S.U.D.B.N.U.H.y.p.q.N.R.p.s.u.u.x.B.B.z.C.B.D.E.F.D.B.A.z.G.2.4.K.C.K.N.I.r.C.y.u.x.v.E.7#c.A.q.C.O##.1.f.n.6.5.q.l.H.J.s.z.y.D.P.Y.V.L.D.v.x.z.z.v.s.s.u.w.w.w.v.u.u.t.t.z.o.m.w.z.s.n.p.z.A.w.o.n.v.E.I.p.o.t.A.y.s.u.D.x.A.B.z.w.x.J.V.I.D.y.y.C.A.v.p.z.w.u.s.r.s.t.v.u.t.t.r.r.r.t.u.C.m.m.I.S.E.u.w.B.C.F.G.G.G.G.F.A.J.S.N.u.i.u.Q.M.v.m.q.t.q.u.D.s.C.B",
".w.r.p.s.s.q.r.w.v.t.z.L.N.F.C.G.N.A.E.V.S.x.r.D.N.R.v.j.H.T.A.i.F.C.C.A.w.x.A.y.x.D.H.G.B.z.C.H.S.Z.S.A.A.T.T.z.w.o.E.T.F.E.Z#c.l.B.z.J.7.P.o.y.A.4.E.g.F.I.r.D.G.z.t.v.B.F.D.y.v.z.z.v.r.q.s.v.x.x.w.w.v.u.u.u.s.r.v.z.u.l.m.u.u.s.l.f.i.s.x.w.A.u.v.B.z.t.w.I.t.u.u.u.t.u.u.s.O.K.F.B.w.t.q.n.x.w.v.u.v.v.u.u.s.t.t.v.t.t.q.p.H.w.r.A.J.J.C.w.G.F.C.A.B.D.G.H.O.Q.K.x.t.z.H.I.C.p.l.t.v.p.o.w.A.H.E",
".j.j.q.D.F.w.r.s.w.v.z.D.C.w.r.r.B.J.M.G.C.C.y.q.r.I.G.K.T.t.e.z.B.x.D.D.x.C.F.v.A.E.F.A.u.v.F.P.V.B.z.G.H.I.C.p.0.G.Z#i.6.0.Z.D.m.x.M.W.N.p.p.L.w.y.w.w.y.x.v.w.3.X.K.z.w.z.A.x.x.A.z.s.p.s.v.w.y.y.x.x.w.v.v.u.l.s.y.w.p.m.r.y.k.l.n.t.w.t.k.d.z.t.s.u.t.p.t.C.n.r.v.B.J.K.y.k.F.E.D.y.v.u.w.w.u.x.y.z.y.x.v.u.s.t.u.v.v.t.p.n.l.F.I.r.r.M.T.G.E.C.z.v.v.z.H.N.Y.L.s.m.E.W.N.r.l.v.w.p.p.x.v.m.C.G.C",
".r.r.A.I.G.t.q.w.z.B.z.u.s.s.q.n.r.F.H.x.u.C.B.r.o.s.F.S.J.j.h.E.u.o.v.z.u.F.M.y.B.C.B.x.u.w.E.K.L.u.D.O.A.r.r.m.5.6.7.4.4.5.M.i.L.s.M.W.s.p.F.v.r.q.D.C.o.y.G.m.V.Z.X.M.D.D.B.z.v.z.x.s.r.w.x.u.y.y.y.x.w.w.v.v.o.v.w.p.n.u.w.r.m.m.t.F.F.u.k.j.r.q.p.p.o.o.r.u.r.t.y.E.R.X.O.A.B.z.x.w.w.v.w.x.s.v.z.C.C.A.x.u.v.v.u.t.s.q.q.p.g.w.D.v.u.G.M.I.y.z.z.x.w.C.N.X.K.z.r.x.O.U.G.o.l.x.B.s.p.x.z.t.D.y.r",
".K.G.H.I.y.n.r.F.D.G.B.q.o.v.y.t.t.t.v.y.w.t.w.A.D.k.r.B.s.y.H.q.C.p.r.r.k.z.K.u.y.z.A.B.B.B.A.z.C.H.V.L.j.k.v.n.F#..W.q.F.X.J.z.n.I.H.z.G.z.s.E.E.A.w.A.D.z.v.w.h.t.F.C.w.t.r.m.s.w.w.s.u.A.z.u.z.z.y.x.x.w.w.v.w.A.t.i.n.B.x.g.p.j.n.w.u.k.n.z.m.p.r.p.q.s.s.r.B.y.t.s.C.P.N.E.F.B.x.u.t.s.q.n.r.u.A.E.F.B.x.u.z.x.u.s.q.q.r.q.G.o.p.J.M.u.l.t.t.x.A.y.y.E.S.4.n.q.D.S.Q.C.u.x.x.t.v.B.x.p.v.L.G.r.g",
".P.P.D.r.A.S.R.A.C.t.p.t.t.o.q.y.t.r.q.s.w.y.x.u.A.z.s.r.K.3.S.q.p.q.t.v.x.y.z.z.C.s.n.x.F.B.s.k.P.J.A.p.l.s.M.5##.I.C.I.z.C.P.K.y.q.p.v.x.r.v.E.v.p.q.u.u.r.y.K.T.D.s.u.z.y.v.u.z.w.t.t.v.x.x.w.u.v.v.t.q.s.y.E.A.s.r.z.A.r.k.k.k.K.z.d.i.q.q.y.e.C.v.h.s.u.o.B.A.L.O.C.t.y.F.H.w.t.p.p.s.t.q.n.x.K.N.E.F.Q.N.B.u.r.p.p.p.o.o.s.n.k.n.w.A.u.r.t.z.r.v.z.D.W.O.d.F.L.S.O.z.m.q.D.t.t.p.o.z.L.H.u.m.p.s",
".J.I.F.B.C.F.H.G.t.q.p.s.u.s.u.x.u.t.s.s.u.v.v.v.x.E.F.z.A.K.R.O.t.v.y.A.A.z.x.w.B.A.A.B.y.x.H.T.F.z.q.n.v.K.Z.7.U.E.G.K.y.v.z.p.u.w.w.x.z.C.F.F.E.A.x.w.v.y.G.P.I.A.v.x.x.v.v.y.y.w.u.v.x.y.x.v.v.t.s.w.z.B.z.x.w.u.t.u.q.k.i.k.E.H.p.f.r.r.j.l.z.v.x.A.D.K.I.x.T.7#f.9.T.F.z.x.w.v.r.o.n.p.s.v.H.M.L.D.C.I.H.z.A.A.w.t.r.s.s.t.q.F.F.s.z.U.U.C.o.I.w.x.1.Q.n.p.D.U.W.C.r.y.D.x.H.y.o.q.E.P.K.y.s.u.x",
".B.B.G.K.D.u.x.H.q.v.v.s.t.v.v.s.w.w.v.t.s.s.t.v.u.A.E.B.x.x.H.S.u.x.A.C.D.B.y.x.z.B.D.C.y.B.O.4.r.p.m.s.L.3.6.Y.H.E.I.F.s.t.y.p.N.S.N.y.u.C.D.v.x.A.y.s.q.u.x.x.u.v.x.z.w.s.v.B.x.w.w.x.y.y.x.v.y.v.v.A.H.H.z.q.r.w.x.s.n.n.q.r.z.t.h.k.x.u.n.x.T.u.B.H.q.s.C.l.x.J.W.Z.N.v.n.o.x.w.t.o.k.n.w.E.N.K.G.D.A.A.A.A.z.D.B.s.r.w.x.s.z.v.r.s.x.E.J.M.z.A.M.S.F.t.w.D.J.S.O.x.s.D.G.z.D.z.y.C.E.D.B.C.A.B.z",
".v.x.D.I.E.v.u.z.z.F.D.u.s.w.u.m.w.y.x.u.q.p.s.w.t.o.q.y.B.y.w.A.t.v.x.z.B.C.C.C.A.v.t.z.I.L.I.E.n.r.v.C.V.8.Y.F.H.J.F.s.k.x.O.Q.2.0.Q.B.u.x.y.v.r.z.B.u.s.w.r.i.l.q.v.y.w.u.x.B.w.x.y.z.A.z.x.w.x.y.B.E.F.C.w.s.m.v.y.t.t.B.C.y.g.l.o.t.u.r.y.N.B.q.A.H.t.y.Y#a.x.w.A.I.I.B.C.M.w.v.r.o.n.s.B.H.K.C.A.D.C.x.x.E.u.A.z.r.q.y.z.s.x.o.m.r.o.j.r.H.Q.H.Y.T.h.h.L.V.Q.D.w.B.C.w.x.F.l.w.M.P.y.i.o.G.P.M.C",
".u.x.y.y.C.F.z.p.I.M.J.y.u.x.v.n.w.x.y.v.r.p.s.v.x.n.n.w.B.w.t.x.y.x.x.x.y.z.B.C.C.w.t.B.M.Q.F.r.w.A.E.H.Q.X.O.z.F.I.x.i.k.D.V.6.O.I.F.I.F.z.z.D.B.I.I.C.z.D.y.q.m.o.q.t.w.z.z.x.w.y.A.A.A.z.y.x.w.D.J.F.x.r.u.z.l.s.v.u.x.F.D.t.i.r.x.w.q.u.B.B.u.B.x.w.D.B.M#g.Y.K.B.F.H.H.M.U.w.s.o.p.t.z.D.E.H.z.z.G.F.x.x.E.B.F.D.v.v.A.B.w.l.H.L.q.i.u.E.A.S.9.M.o.w.y.B.2.H.u.q.A.D.v.t.B.o.y.L.M.v.i.q.K.T.Q.F",
".w.x.v.s.z.I.D.r.H.J.J.E.B.B.z.u.u.v.w.v.t.s.t.u.A.x.x.x.r.n.w.L.H.F.D.A.z.y.z.z.C.E.F.H.L.O.K.E.I.F.E.F.G.H.H.E.u.A.r.m.y.G.K.T.z.u.z.L.L.z.v.A.G.F.C.w.v.z.z.w.p.o.n.o.u.B.A.u.x.z.B.B.z.y.z.A.z.G.J.D.r.m.t.C.n.p.o.q.u.w.q.i.u.r.t.u.v.H.G.q.I.Q.w.r.L.B.k.u.Y.N.D.B.D.C.B.C.w.s.p.r.w.B.B.A.G.A.B.H.G.A.z.D.O.N.J.F.C.C.B.B.k.z.I.B.r.v.G.M.W.Q.x.r.J.V.K.w.r.x.z.v.w.C.A.s.L.D.x.x.z.B.F.K.x.B.z",
".y.v.u.w.x.y.C.H.C.D.G.M.K.E.x.x.s.s.t.v.x.x.u.s.v.y.y.t.l.j.u.F.L.L.L.K.H.E.B.z.G.L.O.M.M.P.L.E.I.D.D.K.I.z.x.B.q.v.s.B.T.O.A.C.y.w.x.z.z.v.u.v.A.v.v.x.A.x.u.v.u.s.o.m.q.z.A.v.z.A.B.A.y.y.A.C.H.E.A.u.s.q.s.t.q.n.n.q.q.m.k.k.y.j.r.D.B.G.J.v.E.F.C.F.T.Y.M.B.I.L.K.I.H.I.G.C.z.x.u.s.t.u.w.x.D.B.B.D.E.F.I.K.P.G.E.I.G.z.A.G.o.m.t.F.D.t.A.S.N.l.z.U.M.R.O.h.o.y.D.z.x.A.C.B.N.B.p.p.C.P.N.D.p.t.s",
".z.t.w.D.w.m.y.X.B.z.G.R.Q.C.s.t.q.q.r.v.z.A.v.r.n.p.o.m.l.o.n.l.K.N.Q.S.Q.M.G.C.I.K.L.N.T.T.F.n.C.x.F.V.T.x.l.o.r.v.u.L#..X.x.y.B.D.u.g.h.u.E.D.A.w.C.P.T.H.y.x.v.w.r.m.n.w.z.x.A.B.C.A.x.x.A.E.N.B.q.p.v.y.q.i.s.p.q.v.s.l.m.w.A.k.A.R.B.q.B.E.B.w.J.L.y.K.T.y.q.D.K.F.B.E.F.B.C.B.z.t.o.n.s.x.y.z.z.z.C.K.S.T.F.t.t.E.E.v.y.J.o.C.H.y.A.O.S.L.k.v.D.T.2.B.j.B.y.s.y.K.F.r.x.R.u.v.r.r.C.O.J.v.H.G.u",
".z.B.A.w.B.H.A.m.O.J.F.C.y.w.x.A.x.r.l.n.v.z.w.r.r.i.f.m.s.q.m.l.y.B.w.s.G.W.N.r.I.F.L.T.P.C.B.K.D.V.2.L.v.u.s.l.r.C.X.7.W.A.t.A.A.q.t.F.H.B.I.Y.G.m.x.N.z.q.x.y.p.r.t.s.q.r.w.A.z.C.D.x.q.t.G.U.u.n.n.u.t.n.n.u.e.k.k.i.n.x.u.l.y.v.H.H.r.x.F.s.W.C.p.u.B.A.y.B.q.m.q.A.H.I.J.M.y.v.w.x.v.q.s.A.v.v.u.z.N.Z.V.J.A.B.D.z.n.f.m.A.v.v.y.D.H.I.E.A.l.C.T.T.D.q.t.E.q.K.J.p.n.I.P.D.q.q.z.J.G.u.q.u.E.H.D",
".F.E.D.B.G.L.L.G.D.L.G.r.q.C.G.y.r.v.y.w.t.t.x.B.v.o.m.t.z.A.C.F.x.A.x.t.A.M.L.C.q.K.R.B.m.v.N.Y.x.I.N.E.v.x.B.y.s.G.W.W.H.s.v.F.z.q.v.N.2.4.4.6.O.x.z.B.r.v.H.D.v.v.v.t.r.q.r.t.D.x.r.v.F.K.F.y.v.r.q.u.s.p.l.m.t.q.l.h.k.w.D.G.h.K.B.q.J.D.s.M.G.N.Q.K.B.y.B.E.B.x.z.H.M.K.J.L.K.z.t.w.z.w.n.k.n.u.D.L.R.S.J.z.y.x.z.C.x.n.k.m.w.w.x.B.D.E.E.C.y.v.v.A.J.K.B.q.O.U.U.O.J.K.I.C.o.y.J.L.B.s.y.J.z.D.C",
".G.C.B.E.H.J.O.T.z.D.A.t.t.B.E.A.y.D.F.C.x.u.v.y.x.u.t.x.A.D.I.O.w.w.w.u.u.y.F.L.F.H.A.s.z.P.R.I.u.w.z.x.w.x.D.H.C.B.A.A.F.H.z.p.g.f.h.w.N.R.I.w.T.L.H.z.v.J.T.H.z.y.w.w.w.u.s.q.y.s.r.B.O.P.B.m.v.v.t.r.r.s.n.f.u.p.j.h.l.r.C.N.C.y.z.A.A.F.F.w.m.G.R.J.C.I.P.O.D.x.u.v.x.y.B.F.C.F.I.D.r.i.i.q.p.x.M.W.T.J.C.C.J.C.z.E.J.G.y.s.x.x.y.z.z.z.C.F.S.G.s.t.N.3.W.E.T.L.M.U.Q.z.p.r.A.I.N.G.z.z.E.I.x.B.z",
".y.v.y.F.F.z.D.N.E.p.q.G.H.s.s.H.Q.K.D.C.F.C.t.j.u.v.w.w.v.v.z.E.x.t.t.v.s.o.w.J.U.y.q.I.Z.R.y.k.A.u.t.x.x.t.x.E.z.y.t.s.C.J.w.e.i.r.x.y.A.C.u.i.H.E.D.A.C.K.I.w.z.x.w.y.A.A.w.s.k.u.H.L.H.A.u.t.u.w.t.l.n.u.s.h.i.f.h.o.o.j.n.x.R.q.t.C.w.I.N.l.q.t.u.v.D.P.S.M.T.L.B.v.v.B.K.R.w.E.L.C.n.g.m.y.B.E.M.T.P.I.G.N.E.s.j.k.t.z.y.v.y.z.z.w.v.w.B.G.P.Q.I.x.A.R.1.1.J.y.y.J.K.v.n.q.Q.N.F.x.C.J.D.q.B.B.u",
".v.v.B.K.G.u.s.y.F.p.o.E.H.w.B.T.R.L.E.C.E.C.s.i.r.w.A.z.v.t.u.v.z.s.s.x.v.o.q.B.E.E.M.R.G.l.g.p.C.u.u.B.A.r.q.w.r.H.N.z.j.i.m.o.n.H.S.I.y.w.A.z.p.m.m.u.B.x.n.l.A.y.x.z.C.C.y.u.k.z.N.M.z.q.t.B.t.v.r.i.l.u.v.o.d.d.k.t.q.g.e.k.o.D.r.o.L.G.u.L.J.x.s.A.I.G.A.y.V.R.J.B.y.B.G.J.I.B.t.p.t.x.t.q.B.B.D.H.H.G.H.M.D.w.o.l.p.w.A.A.z.A.z.x.t.v.B.G.s.H.M.B.r.w.K.T.L.E.B.E.I.H.G.H.N.J.A.u.A.H.y.h.x.y.t",
".z.C.I.L.H.x.s.t.x.A.x.r.x.L.V.W.C.G.H.D.w.s.t.v.s.z.E.C.z.z.y.w.y.t.t.x.x.r.q.v.y.R.U.z.g.h.u.C.u.q.t.z.z.s.r.v.G.M.I.s.d.c.k.t.v.K.R.E.n.k.r.x.r.r.p.x.I.z.r.y.C.B.A.A.B.A.y.w.A.D.E.B.x.w.y.B.s.t.q.l.l.q.t.s.r.p.q.s.p.k.j.l.f.u.E.E.w.m.t.N.G.C.H.Q.M.x.r.x.H.K.L.F.A.x.t.o.F.D.y.q.p.t.t.s.s.w.y.y.A.D.B.w.G.K.K.G.D.F.F.D.z.A.z.y.x.w.y.B.p.A.L.N.I.C.B.C.K.K.F.A.C.J.N.M.x.B.C.x.u.u.t.r.p.t.v",
".F.J.J.E.C.B.z.v.q.D.G.A.D.S.V.K.v.z.B.x.s.q.s.w.w.C.D.A.z.B.A.v.u.v.w.w.u.t.v.y.J.J.y.o.r.E.F.w.i.l.r.t.u.t.v.y.O.A.o.m.o.q.s.v.R.M.D.v.r.q.n.k.r.F.E.E.R.N.D.I.A.B.C.B.A.A.B.D.P.F.w.s.w.B.B.A.t.t.u.u.p.j.k.p.z.y.s.n.m.q.q.o.n.f.E.P.j.h.B.u.s.y.G.K.F.y.x.A.E.M.Q.M.H.F.A.v.p.E.M.A.l.j.q.z.t.E.F.w.v.E.C.r.q.A.G.E.A.A.x.t.z.y.z.B.C.A.y.w.C.y.F.S.W.L.B.z.E.E.C.z.y.A.E.G.x.w.x.x.t.p.q.w.o.q.q",
".H.J.E.t.t.A.E.z.o.w.K.V.Y.O.E.x.D.w.p.o.u.w.r.l.B.E.C.v.u.y.x.r.r.x.z.v.r.t.z.E.H.n.g.x.M.H.u.m.d.j.p.o.p.s.w.A.u.o.p.v.v.s.y.K.S.x.e.g.t.w.l.#.c.C.G.y.L.O.A.w.t.w.z.z.y.A.F.K.S.I.x.r.t.z.C.D.v.u.z.C.s.e.e.l.y.z.s.i.k.t.s.k.f.l.g.p.J.B.n.y.s.x.v.p.r.A.A.r.s.z.B.x.v.z.B.A.o.y.E.y.u.v.u.s.F.T.Q.w.r.E.H.w.n.y.E.D.C.F.F.B.z.y.z.C.F.E.x.s.G.t.s.H.M.C.x.C.D.C.F.J.G.z.A.I.M.w.n.s.y.t.p.q.u.s.k",
".F.C.y.u.q.o.u.D.w.A.G.L.X.E.L.v.t.w.x.u.w.C.F.D.F.A.u.t.w.y.w.u.n.s.v.v.y.F.H.G.i.q.v.C.H.v.j.m.v.w.y.u.q.o.p.s.l.p.s.s.q.q.t.w.p.q.s.r.n.g.f.j.j.o.u.s.o.q.B.M.n.s.v.v.x.G.M.O.z.C.F.D.z.w.y.B.n.p.r.s.q.p.n.n.i.A.M.E.r.o.q.r.q.l.j.l.s.w.v.r.u.u.s.s.t.v.z.A.A.u.x.F.G.x.w.A.s.v.w.G.O.D.s.x.Q.L.u.u.t.s.Q.0.H.T.3.J.F.V.K.B.u.w.B.D.x.r.C.T.H.H.D.B.y.y.w.v.x.B.z.E.R.O.C.A.U.O.C.q.n.q.r.p.o.w.w",
".B.z.w.t.o.m.r.A.M.I.D.y.H.s.G.v.s.v.w.u.w.C.G.E.B.z.w.v.w.x.w.v.y.x.v.v.B.G.B.t.q.x.z.C.E.s.j.q.r.r.q.m.i.k.p.v.l.p.s.s.q.q.t.w.t.s.q.o.j.f.h.n.u.t.s.q.q.s.w.B.r.q.p.r.x.H.H.C.u.x.B.A.w.t.t.v.r.r.p.o.n.n.p.q.i.r.z.A.u.p.p.p.y.t.p.m.o.p.o.m.F.B.v.p.o.q.u.x.w.r.v.G.H.A.y.C.B.z.s.u.D.A.y.J.I.G.t.w.x.v.Q.Y.C.K.U.G.E.Q.G.B.A.x.y.C.A.w.A.K.Q.P.M.J.G.F.D.C.G.G.x.v.F.I.G.M.L.N.J.y.q.q.t.v.s.y.x",
".z.y.x.v.q.m.q.y.U.O.D.r.z.m.E.u.r.u.w.t.w.D.I.G.w.x.z.y.x.w.w.x.F.z.s.u.F.K.x.h.w.E.E.D.A.o.k.w.q.n.l.f.d.i.r.A.l.p.s.s.q.q.t.w.x.t.p.k.g.d.i.r.C.v.p.o.t.w.v.u.v.p.k.p.A.K.D.s.q.u.z.z.w.s.q.q.w.s.n.k.k.n.r.u.m.i.n.y.A.s.o.p.C.y.u.n.k.j.l.m.M.F.v.m.j.n.t.x.r.o.u.H.K.C.A.D.F.B.p.n.x.z.D.P.A.B.r.x.A.z.R.V.z.B.N.G.F.M.D.F.G.x.v.B.F.A.y.A.R.R.N.J.G.E.B.B.K.J.y.r.y.D.I.V.B.M.R.H.u.q.v.B.v.B.y"
};


NConfig::NConfig()
{
  std::string oldDir = NFile::workingDir();
  NFile::cdHome();
  loadXmlConfig(".ngrs.xml");
  NFile::cd(oldDir);
}


NConfig::~NConfig()
{
}

void NConfig::setSkin( NSkin * skin, const std::string & identifier )
{
  NSkin* xmlSkin;
  if (identifier == "") return; else
  if ( (xmlSkin = findSkin(identifier)) && (xmlSkin!=0)) {
    skin->font = xmlSkin->font;
    skin->useParentFont = false;
    skin->bgColor = xmlSkin->bgColor;
    skin->useParentBgColor = xmlSkin->useParentBgColor;
    skin->gradStartColor  = xmlSkin->gradStartColor;
    skin->gradMidColor    = xmlSkin->gradMidColor;
    skin->gradEndColor    = xmlSkin->gradEndColor;
    skin->gradientStyle   = xmlSkin->gradientStyle;
    skin->gradientOrientation = xmlSkin->gradientOrientation;
    skin->arcWidth = xmlSkin->arcWidth;
    skin->arcHeight = xmlSkin->arcHeight;
    skin->gradientPercent = xmlSkin->gradientPercent;
    skin->translucent = xmlSkin->translucent;
    skin->transColor  = xmlSkin->transColor;
    skin->transparent = xmlSkin->transparent;
    skin->spacing = xmlSkin->spacing;
    skin->bitmap = xmlSkin->bitmap;
    skin->border = xmlSkin->border;
    skin->bitmapBgStyle = xmlSkin->bitmapBgStyle;
  } else
  if (identifier == "edit") {
    skin->gradStartColor.setRGB(230,230,230);
    skin->gradMidColor.setRGB(50,50,50);
    skin->gradEndColor.setRGB(100,100,100);
    skin->gradientStyle   = 1;
    skin->gradientPercent = 10;
    skin->gradientOrientation = nVertical;
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
    skin->font.setTextColor(NColor(230,230,230));

    skin->useParentBgColor = false;
    skin->useParentFgColor = false;
    skin->useParentFont    = false;
    skin->transparent      = false;
  } else
  if (identifier == "filedlgpane") {
    skin->bgColor = NColor(200,200,200);
    skin->useParentBgColor = false;
    skin->useParentFgColor = false;
    skin->useParentFont    = false;
    skin->transparent      = false;
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
  } else

  if (identifier == "pane") {
    skin->bgColor = NColor(200,200,200);
    skin->useParentBgColor = false;
    skin->useParentFgColor = false;
    skin->useParentFont    = false;
    skin->transparent      = false;
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
  } else

  if (identifier == "mitemover") {
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
    skin->font.setTextColor(NColor(0,0,0));

    skin->gradStartColor.setRGB(130,130,130);
    skin->gradMidColor.setRGB(150,150,150);
    skin->gradEndColor.setRGB(140,140,140);
    skin->gradientStyle   = 1;
    skin->gradientPercent = 10;
    skin->useParentFont    = false;

  } else
  if (identifier == "mitemnone") {
    skin->fgColor = NColor(50,50,150);
    skin->useParentBgColor = false;
    skin->useParentFgColor = false;
    skin->useParentFont    = false;
    skin->transparent      = true;
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
    skin->font.setTextColor(NColor(150,150,180));
    skin->translucent = 100;
    skin->border = 0;
  } else
  if (identifier == "popmnubg") {
    skin->fgColor = NColor(230,230,230);
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
    skin->font.setTextColor(NColor(230,230,230));
    skin->useParentBgColor = false;
    skin->useParentFgColor = false;
    skin->useParentFont    = false;
    skin->transparent      = false;
    skin->bgColor.setRGB(230,230,230);
  } else
  if (identifier == "sbar_pane") {
      skin->gradientStyle = 1;
      skin->gradStartColor.setRGB(200,200,200);
      skin->gradMidColor.setRGB(230,230,230);
      skin->gradEndColor.setRGB(200,200,200);
  } else
  if (identifier == "mbtnover") {
    skin->gradStartColor.setRGB(130,130,130);
    skin->gradMidColor.setRGB(150,150,150);
    skin->gradEndColor.setRGB(140,140,140);
    skin->gradientStyle   = 1;
    skin->gradientPercent = 10;
    skin->useParentFont    = false;
  } else
  if (identifier == "mbtnnone") {
  } else
  if (identifier == "mbar") {
    skin->gradStartColor.setRGB(230,230,230);
    skin->gradMidColor.setRGB(250,250,250);
    skin->gradEndColor.setRGB(240,240,240);
    skin->gradientStyle   = 1;
    skin->gradientPercent = 10;
    skin->gradientOrientation = nVertical;
  } else
  if (identifier == "clbox") {
    skin->bitmapBgStyle = 0;
    skin->bgColor = NColor(230,230,230);
    skin->fgColor = NColor(0,0,0);
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
    skin->useParentBgColor = false;
    skin->useParentFgColor = false;
    skin->useParentFont    = false;
    skin->transparent      = false;
  } else
  if (identifier == "btnup") {
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);

    skin->gradStartColor.setRGB(230,230,230);
    skin->gradMidColor.setRGB(240,240,240);
    skin->gradEndColor.setRGB(180,180,180);
    skin->gradientStyle   = 1;
    skin->gradientPercent = 10;
    skin->gradientOrientation = nVertical;
    skin->useParentFont    = false;

  } else 
  if (identifier == "btnover") {
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);

    skin->gradStartColor.setRGB(240,240,240);
    skin->gradMidColor.setRGB(250,250,250);
    skin->gradEndColor.setRGB(180,180,180);
    skin->gradientStyle   = 1;
    skin->gradientPercent = 10;
    skin->gradientOrientation = nVertical;
    skin->useParentFont    = false;
  } else 
  if (identifier == "btndown") {
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
    skin->gradStartColor.setRGB(200,200,200);
    skin->gradMidColor.setRGB(210,210,210);
    skin->gradEndColor.setRGB(180,180,180);
    skin->gradientStyle   = 1;
    skin->gradientPercent = 10;
    skin->gradientOrientation = nVertical;
    skin->useParentFont    = false;
  } else
  if (identifier == "btnflat") {
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
    skin->transparent = true;
  } else
  if (identifier == "tabup") {
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
    skin->font.setTextColor(NColor(0,0,0));

    skin->gradStartColor.setRGB(230,230,230);
    skin->gradMidColor.setRGB(240,240,240);
    skin->gradEndColor.setRGB(200,200,200);
    skin->gradientStyle   = 1;
    skin->gradientPercent = 10;
    skin->gradientOrientation = nVertical;
    skin->useParentFont    = false;
  } else
  if (identifier == "tabnone") {
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
    skin->font.setTextColor(NColor(0,0,0));

    skin->gradStartColor.setRGB(200,200,200);
    skin->gradMidColor.setRGB(210,210,210);
    skin->gradEndColor.setRGB(180,180,180);
    skin->gradientStyle   = 1;
    skin->gradientPercent = 10;
    skin->gradientOrientation = nVertical;
    skin->useParentFont    = false;
  }
}

int NConfig::loadXmlConfig(const std::string & configName )
{
  try {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException& toCatch) {
    // Do your failure processing here
//    return 1;
  }

  SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();

  MySAX2Handler* defaultHandler = new MySAX2Handler();
  defaultHandler->setCfg(this);
  parser->setContentHandler(defaultHandler);
  parser->setErrorHandler(defaultHandler);
  try {
    parser->parse(configName.c_str());
  }
        catch (const XMLException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            cout << "Exception message is: \n"
                 << message << "\n";
            XMLString::release(&message);
            //return -1;
        }
        catch (const SAXParseException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            cout << "Exception message is: \n"
                 << message << "\n";
            XMLString::release(&message);
            //return -1;
        }
   catch (...) {
       cout << "Unexpected Exception \n" ;
            //return -1;
   }

  delete parser;
  delete defaultHandler;
  XMLPlatformUtils::Terminate();
}

NSkin* NConfig::findSkin( const std::string & id )
{
  std::map<std::string, NSkin>::iterator itr;
  if ( (itr = skinMap.find(id)) == skinMap.end() )
  {
      return 0;
  } else {
    return &itr->second;
  }
}

std::string NConfig::findPath( const std::string & id )
{
  std::map<std::string, std::string>::iterator itr;
  if ( (itr = pathMap.find(id)) == pathMap.end() )
  {
      return "";
  } else {
    return itr->second;
  }
}
