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
         fr->setSpacing(NSize(0,0,0,0));
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
".	c None",
"+	c #CECECE",
"@	c #CECECD",
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
  if (identifier == "lbitemsel") {
    skin->font    = NFont("Suse sans",8,nMedium | nStraight | nAntiAlias);
    skin->font.setTextColor(NColor(0,0,0));
    skin->gradStartColor.setRGB(130,130,130);
    skin->gradMidColor.setRGB(150,150,150);
    skin->gradEndColor.setRGB(140,140,140);
    skin->gradientStyle   = 1;
    skin->gradientPercent = 10;
    skin->useParentFont    = false;

  } else
  if (identifier == "lbitemnone") {

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
      skin->gradStartColor.setRGB(240,240,240);
      skin->gradMidColor.setRGB(220,220,220);
      skin->gradEndColor.setRGB(230,230,230);
      skin->gradientPercent=0;
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
  } else
  if (identifier == "sbar_vsl") {
     NFrameBorder* fr = new NFrameBorder();
     fr->setSpacing(NSize(0,0,0,0));
     fr->setOval(true,4,4);
     skin->border = fr;
     skin->bitmap.createFromXpmData(vknob_xpm);
     skin->bitmapBgStyle = 2;

  } else
  if (identifier == "sbar_hsl") {
     NFrameBorder* fr = new NFrameBorder();
     fr->setSpacing(NSize(0,0,0,0));
     fr->setOval(true,4,4);
     skin->border = fr;
     skin->bitmap.createFromXpmData(hbar_xpm);
     skin->bitmapBgStyle = 2;
  }
}

void NConfig::loadXmlConfig(const std::string & configName )
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
