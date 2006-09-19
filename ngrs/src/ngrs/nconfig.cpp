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
#include "nfile.h"
#include <iostream>
#include <xercesc/sax2/Attributes.hpp>


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
       } catch (std::exception e) {
           return "";
        }
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

    pCfg->tagParse.emit(tagName);
    pCfg->attrs = &attrs;

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
         NBitmap bmp(src);
         skin->setBitmap(bmp,style);
      }
    } else
    if (tagName == "font") {
      NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         std::string name = getValue("name",attrs);
         std::string size = getValue("size",attrs);
         std::string color = getValue("color",attrs);
         std::stringstream str; str << size; int sz = 0; str >> sz;
         NFont font(name,sz,nMedium | nAntiAlias);
         font.setTextColor(NColor(color));
         skin->setFont(font);
      }
    } else
    if (tagName == "frmborder") {
     NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         NFrameBorder fr;
         fr.setSpacing(NSize(0,0,0,0));

         std::string styleStr = getValue("style",attrs);
         if (styleStr != "") {
           std::stringstream str; str << styleStr;
           int style  = 0;
           str >> style;
           fr.setOval(style);
         }


         std::string lcount  = getValue("lines",attrs);

         std::string arcw    = getValue("arcw",attrs);
         std::string arch    = getValue("arch",attrs);

         if (lcount!="") {
            int lines = str<int>(lcount);
            std::string dwidth    = getValue("linedistw",attrs);
            int dw = 0;
            if (dwidth!="")  dw = str<int>(dwidth);
             std::string dheight  = getValue("linedistw",attrs);
            int dh = 0;
            if (dheight!="") dh = str<int>(dheight);
            fr.setOval(lines,dw,dh);
         }

         int arcH = 0;
         if (arch!="") arcH = str<int>(arch);
         int arcW = 0; arcW = str<int>(arcw);
         if (arcw!="" || arch!="") {
           fr.setOval(fr.oval(),arcH,arcW);
         }

         skin->setBorder(fr);
      }
    } else
    if (tagName == "bvlborder") {
     NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         NBevelBorder br;
         br.setSpacing(NSize(0,0,0,0));

         int outerStyle = nNone;

         std::string styleStr = getValue("outer",attrs);
         if (styleStr != "") {
           if (styleStr == "raised")  outerStyle = nRaised; else
           if (styleStr == "lowered") outerStyle = nLowered;
         }

         int innerStyle = nNone;

         styleStr = getValue("inner",attrs);
         if (styleStr != "") {
           if (styleStr == "raised")  innerStyle = nRaised; else
           if (styleStr == "lowered") innerStyle = nLowered;
         }

         br.setStyle(outerStyle,innerStyle,2);
         skin->setBorder(br);
       }
    } else
    if (tagName == "bgcolor") {
      NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         skin->setBackground(attrsToColor(attrs));
         skin->useParentBackground(false);
         skin->setTransparent(false);
      }
    } else
    if (tagName == "fgcolor") {
      NSkin* skin = pCfg->findSkin(lastId);
      if (skin != 0) {
         skin->setForeground(attrsToColor(attrs));
         skin->useParentForeground(false);
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
         skin->setTranslucent(NColor(color),percent);
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

         int arcWidth = 0;
         if (arcw!="") {
           std::stringstream str3;
           str3 << arcw;
           str3 >> arcWidth;
         }

         int arcHeight = 0;
         if (arch!="") {
           std::stringstream str3;
           str3 << arch;
           str3 >> arcHeight;
         }

         skin->setGradient(NColor(start),NColor(mid),NColor(end),style,(direction=="hor") ? nHorizontal : nVertical,percent,arcWidth,arcHeight);
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

         skin->setSpacing(NSize(left,top,right,bottom));
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
    cout << "ngrs: configuration: xml error: " << message
         << " at line: " << exception.getLineNumber()
         << endl;
    cout << "ngrs: configuration: using defaults instead" << endl;
}


/* XPM */
const char * hbar_xpm[] = {
"20 10 4 1",
".	c None",
"+	c #CECECE",
"@	c #CECECD",
"#	c #DEDEDE",
".+@+.+@+.+@+.+@+.+@+",
"+#+@+#+@+#+@+#+@+#+@",
"@+.+@+.+@+.+@+.+@+.+",
"+@+#+@+#+@+#+@+#+@+#",
".+@+.+@+.+@+.+@+.+@+",
"+#+@+#+@+#+@+#+@+#+@",
"@+.+@+.+@+.+@+.+@+.+",
"+@+#+@+#+@+#+@+#+@+#",
".+@+.+@+.+@+.+@+.+@+",
"+#+@+#+@+#+@+#+@+#+@"};

/* XPM */
const char * vknob_xpm[] = {
"11 20 5 1",
"@	c #CCCCCC",
".	c #CECECE",
"+	c #CECECD",
"#	c #DEDEDE",
"$	c #CACACA",
".+@+.+@+.+@",
"#+@+#+@+#++",
"+$+@+$+@+$.",
"@+#+@+#+@++",
"+@+$+@+$+@@",
"#+@+#+@+#++",
"+$+@+$+@+$.",
"@+#+@+#+@++",
"+@+$+@+$+@@",
"#+@+#+@+#++",
"+$+@+$+@+$.",
"@+#+@+#+@++",
"+@+$+@+$+@@",
"#+@+#+@+#++",
"+$+@+$+@+$.",
"@+#+@+#+@++",
"+@+$+@+$+@@",
"#+@+#+@+#++",
"+$+@+$+@+$.",
"@+#+@+#+@++"};

const char * arrow_test_xpm[] = {
"12 6 2 1",
" 	c white",
".	c black",
"            ",
"     .      ",
"    ...     ",
"   .....    ",
"  .......   ",
"            "};


NConfig::NConfig()
{
  attrs = 0;
  std::string oldDir = NFile::workingDir();
  NFile::cdHome();
//  loadXmlConfig(".ngrs.xml");
  NFile::cd(oldDir);
}


NConfig::~NConfig()
{
}

NSkin NConfig::skin( const std::string & identifier )
{
  NSkin skin;
  NSkin* xmlSkin = 0;
  if (identifier == "") return NSkin(); else
  if ( (xmlSkin = findSkin(identifier)) && (xmlSkin!=0)) {
    skin = *xmlSkin;
  } else // default skins if xml not found
  if (identifier == "edit") {
    skin.setGradient(NColor(230,230,230),NColor(250,250,250),NColor(240,240,240),1,nVertical,10,4,4);
    skin.setTransparent(false);
  } else
  if (identifier == "toolbar") {
      NBevelBorder border(nLowered,nRaised);
      skin.setBorder(border);
  } else
  if (identifier == "filedlgpane") {
    skin.setBackground(NColor(200,200,200));
    skin.useParentBackground(false);
    skin.setTransparent(false);
  } else
  if (identifier == "pane") {    
    skin.setBackground(NColor(200,200,200));
    skin.useParentBackground(false);
    skin.setTransparent(false);
  }
  if (identifier == "lbitemsel") {
    skin.setBackground(NColor(68,123,205));
    skin.useParentBackground(false);
    skin.setTransparent(false);
  } else
  if (identifier == "splitbar") {
     NFrameBorder fr(true,3,3);
     fr.setSpacing(NSize(0,0,0,0));
     skin.setBorder(fr);
     skin.setGradient(NColor(200,200,200),NColor(220,220,220),NColor(220,220,220),2,nVertical,50,3,3);

  } else
  if (identifier == "mitemiconbg") {
    skin.setBackground(NColor(220,220,220));
    skin.useParentBackground(false);
    skin.setTransparent(false);
  } else
  if (identifier == "mitemover") {
     NFrameBorder fr(true,4,4);
     fr.setSpacing(NSize(0,0,0,0));
     skin.setBorder(fr);
     skin.setGradient(NColor(89,153,235),NColor(68,123,205),NColor(88,143,225),2,nVertical,10,4,4);
     skin.setTransparent(false);
  } else
  if (identifier == "mitemnone") {
  } else
  if (identifier == "popup_menu_bg") {
    skin.setBackground(NColor(240,240,240));
    skin.useParentBackground(false);
    skin.setTransparent(false);
  } else
  if (identifier == "sbar_pane") {
    skin.setBackground(NColor(230,230,230));
    skin.useParentBackground(false);
    skin.setTransparent(false);
  } else
  if (identifier == "mbtnover") {
     skin.setGradient(NColor(200,200,200),NColor(210,210,210),NColor(180,180,180),2,nVertical,10,5,5);
     NFrameBorder fr(true,5,5);
     fr.setSpacing(NSize(0,0,0,0));
     skin.setBorder(fr);
  } else
  if (identifier == "mbtnnone") {
  } else
  if (identifier == "mbar") {
     skin.setGradient(NColor(230,230,230),NColor(250,250,250),NColor(240,240,240),2,nVertical,10,4,4);
     skin.setTransparent(false);
  } else
  if (identifier == "clbox") {
  } else
  if (identifier == "btnup") {
     skin.setGradient(NColor(240,240,240),NColor(250,250,250),NColor(180,180,180),2,nVertical,10,5,5);
     NFrameBorder fr(true,5,5);
     fr.setSpacing(NSize(0,0,0,0));
     skin.setBorder(fr);
  } else 
  if (identifier == "btnover") {
     skin.setGradient(NColor(250,250,250),NColor(255,255,255),NColor(210,210,210),2,nVertical,10,5,5);
     NFrameBorder fr(true,5,5);
     fr.setSpacing(NSize(0,0,0,0));
     skin.setBorder(fr);
  } else 
  if (identifier == "btndown") {
     skin.setGradient(NColor(200,200,200),NColor(210,210,210),NColor(180,180,180),2,nVertical,10,5,5);
     NFrameBorder fr(true,5,5);
     fr.setSpacing(NSize(0,0,0,0));
     skin.setBorder(fr);
  } else
  if (identifier == "btnflat") {
  } else
  if (identifier == "tabup") {
     skin.setGradient(NColor(230,230,230),NColor(240,240,240),NColor(200,200,200),1,nVertical,10,5,5);
  } else
  if (identifier == "tabnone") {
     //fnt.setTextColor(NColor(100,100,100));
//     skin.setFont(fnt);
     skin.setGradient(NColor(200,200,200),NColor(210,210,210),NColor(180,180,180),1,nVertical,10,5,5);
  } else
  if (identifier == "sbar_vsl") {
  /*  NBitmap bmp;
    bmp.createFromXpmData(vknob_xpm);
    skin.setBitmap(bmp,2);*/
    skin.setBorder(NFrameBorder(true,4,4));
    skin.setGradient(NColor(240,240,240),NColor(250,250,250),NColor(220,220,220),2,nHorizontal,90,4,4);
    skin.setTransparent(false);
  } else
  if (identifier == "sbar_hsl") {
    /*NBitmap bmp;
    bmp.createFromXpmData(hbar_xpm);
    skin.setBitmap(bmp,2);*/
    skin.setBorder(NFrameBorder(true,4,4));
    skin.setGradient(NColor(240,240,240),NColor(250,250,250),NColor(220,220,220),2,nVertical,90,4,4);
    skin.setTransparent(false);
  }
  return skin;
}

void NConfig::loadXmlConfig(const std::string & configName, bool throw_allowed )
{
  attrs = 0;

  try {
    XMLPlatformUtils::Initialize();
  }
  catch (const XMLException&) {
    if(throw_allowed) throw;
    else return; // ahem...
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
            cout << "ngrs: configuration: error: Exception message is: \n"
                 << message << "\n";
            XMLString::release(&message);
            if(throw_allowed) throw;
            //else return -1;
        }
        catch (const SAXParseException& toCatch) {
            char* message = XMLString::transcode(toCatch.getMessage());
            cout << "ngrs: configuration: error: Exception message is: \n"
                 << message << "\n";
            XMLString::release(&message);
            if(throw_allowed) throw;
            //else return -1;
        }
   catch (...) {
       if(throw_allowed)
       {
           delete parser;
           delete defaultHandler;
           XMLPlatformUtils::Terminate();
           throw;
       }
       else std::cerr << "ngrs: configuration: error: Unexpected Exception" << std::endl;
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

std::string NConfig::getAttribValue( const std::string & name )
{
   std::string erg;
       try {
       XMLCh* str = XMLString::transcode(name.c_str());
       const XMLCh* strCh = attrs->getValue(str);
       char* id = XMLString::transcode(strCh);
       erg = std::string(id);
       XMLString::release(&id);
       XMLString::release(&str);
       } catch (std::exception e)
       {
           return "";
       }
      return erg;
}
