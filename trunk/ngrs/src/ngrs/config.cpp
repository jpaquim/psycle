/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by  Stefan Nattkemper   *
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

#include "config.h"
#include "bevelborder.h"
#include "frameborder.h"
#include "xmlparser.h"
#include "file.h"
#include <iostream>

using namespace std;

namespace ngrs {

  Color Config::attrsToColor( const XmlParser & parser ) {
    std::string rStr = parser.getAttribValue( "r" );
    std::string gStr = parser.getAttribValue( "g" );
    std::string bStr = parser.getAttribValue( "b" );

    std::stringstream str1; str1 << rStr; int r = 0; str1 >> r;
    std::stringstream str2; str2 << gStr; int g = 0; str2 >> g;
    std::stringstream str3; str3 << bStr; int b = 0; str3 >> b;

    return Color( r, g, b );
  }



  void Config::onTagParse( const XmlParser & parser, const std::string & tagName ) {    
    if ( tagName == "vcskin" ) {
      Skin skin;
      std::string id = parser.getAttribValue( "id" );
      lastId = id;
      skinMap[id] = skin;
    } else
      if (tagName == "bitmap") {
        Skin* skin = findSkin(lastId);
        if ( skin ) {
          std::string src = parser.getAttribValue("src");
          std::string styleStr = parser.getAttribValue("style");
          std::stringstream str; str << styleStr;
          int style  = 0;
          str >> style;
          Bitmap bmp(src);
          skin->setBitmap(bmp,style);
        }
      } else
        if (tagName == "font") {
          Skin* skin = findSkin(lastId);
          if (skin != 0) {
            std::string name = parser.getAttribValue("name");
            std::string size = parser.getAttribValue("size");
            std::string color = parser.getAttribValue("color");
            std::stringstream str; str << size; int sz = 0; str >> sz;
            Font font(name,sz,nMedium | nAntiAlias);
            font.setTextColor(Color(color));
            skin->setFont(font);
          }
        } else
          if (tagName == "frmborder") {
            Skin* skin = findSkin(lastId);
            if ( skin ) {
              FrameBorder fr;
              fr.setSpacing(Size(0,0,0,0));

              std::string styleStr = parser.getAttribValue("style");
              if (styleStr != "") {
                std::stringstream str; str << styleStr;
                int style  = 0;
                str >> style;
                fr.setOval(style);
              }

              std::string lcount  = parser.getAttribValue("lines");

              std::string arcw    = parser.getAttribValue("arcw");
              std::string arch    = parser.getAttribValue("arch");

              if ( lcount!="" ) {
                int lines = str<int>(lcount);
                std::string dwidth    = parser.getAttribValue("linedistw");
                int dw = 0;
                if (dwidth!="")  dw = str<int>(dwidth);
                std::string dheight  = parser.getAttribValue("linedistw");
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

              skin->setBorder( fr );
            }
          } else
            if (tagName == "bvlborder") {
              Skin* skin = findSkin(lastId);
              if ( skin ) {
                BevelBorder br;
                br.setSpacing(Size(0,0,0,0));

                int outerStyle = nNone;

                std::string styleStr = parser.getAttribValue("outer");
                if (styleStr != "") {
                  if (styleStr == "raised")  outerStyle = nRaised; 
                  else
                    if (styleStr == "lowered") outerStyle = nLowered;
                }

                int innerStyle = nNone;

                styleStr = parser.getAttribValue("inner");
                if (styleStr != "") {
                  if (styleStr == "raised")  innerStyle = nRaised; 
                  else
                    if (styleStr == "lowered") innerStyle = nLowered;
                }

                br.setStyle(outerStyle,innerStyle,2);
                skin->setBorder(br);
              }
            } else
              if ( tagName == "bgcolor" ) {
                Skin* skin = findSkin(lastId);
                if ( skin ) {
                  skin->setBackground( attrsToColor( parser ) );
                  skin->useParentBackground( false );
                  skin->setTransparent( false );
                }
              } else
                if (tagName == "fgcolor") {
                  Skin* skin = findSkin( lastId );
                  if ( skin ) {
                    skin->setForeground( attrsToColor( parser ));
                    skin->useParentForeground(false);
                  }
                } else
                  if (tagName == "translucent") {
                    Skin* skin = findSkin( lastId );
                    if ( skin ) {
                      std::string color = parser.getAttribValue( "color" );
                      std::string percentStr = parser.getAttribValue("percent");
                      std::stringstream str; str << percentStr;
                      int percent  = 0;
                      str >> percent;
                      skin->setTranslucent( Color( color ), percent );
                    }
                  } else
                    if (tagName == "gradient") {
                      Skin* skin = findSkin(lastId);
                      if ( skin ) {
                        std::string start = parser.getAttribValue( "start" );
                        std::string mid = parser.getAttribValue( "mid" );
                        std::string end = parser.getAttribValue( "end" );
                        std::string styleStr   = parser.getAttribValue( "style" );
                        std::string percentStr = parser.getAttribValue( "percent" );
                        std::string direction  = parser.getAttribValue( "dir" );
                        std::string arcw  = parser.getAttribValue( "arcw" );
                        std::string arch  = parser.getAttribValue( "arch" );
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
                        skin->setGradient( Color(start), Color(mid), Color(end),style,(direction=="hor") ? nHorizontal : nVertical,percent,arcWidth,arcHeight);
                      }
                    } else
                      if (tagName == "spacing") {
                        Skin* skin = findSkin( lastId );
                        if ( skin ) {
                          std::string l = parser.getAttribValue( "left" );
                          std::string t = parser.getAttribValue( "top" );
                          std::string r = parser.getAttribValue( "right" );
                          std::string b = parser.getAttribValue( "bottom" );
                          std::stringstream str; str << l; int left = 0; str >> left;
                          std::stringstream str1; str1 << t; int top  = 0; str1 >> top;
                          std::stringstream str2; str2 << r; int right  = 0; str1 >> right;
                          std::stringstream str3; str3 << b; int bottom  = 0; str1 >> bottom;

                          skin->setSpacing(Size(left,top,right,bottom));
                        }
                      }
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
    "+#+@+#+@+#+@+#+@+#+@"
  };

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
    "@+#+@+#+@++"
  };

  const char * arrow_test_xpm[] = {
    "12 6 2 1",
    " 	c white",
    ".	c black",
    "            ",
    "     .      ",
    "    ...     ",
    "   .....    ",
    "  .......   ",
    "            "
  };


  Config::Config()
  {
    std::string oldDir = ngrs::File::workingDir();
    ngrs::File::cdHome();
    loadXmlConfig(".ngrs.xml");
    ngrs::File::cd(oldDir);
  }


  Config::~Config()
  {
  }

  Skin Config::skin( const std::string & identifier )
  {
    ///\ todo write a mem parse xml file
    Skin newSkin;
    Skin* xmlSkin = 0;
    if (identifier == "") return Skin(); else
      if ( (xmlSkin = findSkin(identifier)) && (xmlSkin!=0)) {
        newSkin = *xmlSkin;
      } else // default skins if xml not found
        if (identifier == "edit") {
          newSkin.setBackground( Color( 255, 255, 255 ) );
          newSkin.useParentBackground(false);
          newSkin.setTransparent(false);
        } else
          if (identifier == "toolbar") {
            BevelBorder border( nNone, nRaised );
            border.setSpacing( Size( 0, 2, 0, 2 ) );
            newSkin.setBorder(border);
          } else
            if (identifier == "filedlgpane") {
              newSkin.setBackground(Color(236,233,216));
              newSkin.useParentBackground(false);
              newSkin.setTransparent(false);
            } else
              if (identifier == "pane") {    
                newSkin.setBackground(Color(236,233,216));
                newSkin.useParentBackground(false);
                newSkin.setTransparent(false);
              } else
                if (identifier == "lbitemsel") {
                  newSkin.setBackground(Color( 49, 106, 197));
                  newSkin.useParentBackground( false );
                  newSkin.setTransparent( false );
                  newSkin.setTextColor( Color(255, 255, 255) );
                } else
                  if (identifier == "splitbar") {
                    newSkin.setTransparent( true );
                  } else
                    if (identifier == "mitemiconbg") {
                      newSkin.setGradient( Color(252,251,248), Color(228,224,214), Color(199,199,177),1,nHorizontal,50,3,3);
                      newSkin.useParentBackground(false);
                      newSkin.setTransparent(false);
                    } else
                      if (identifier == "mitemover") {
                        newSkin.setBackground( Color( 193, 210, 238 ) );     
                        newSkin.setTransparent( false );
                        newSkin.useParentBackground(false);
                        newSkin.setSpacing( Size(0,0,0,0) );
                        FrameBorder fr;
                        fr.setSpacing(Size(0,0,0,0));
                        fr.setColor( Color( 49, 106, 197 ) );
                        newSkin.setBorder(fr);
                      } else
                        if (identifier == "mitemnone") {
                        } else
                          if (identifier == "popup_menu_bg") {
                            newSkin.setBackground(Color( 252, 252, 249 ));
                            newSkin.useParentBackground(false);
                            newSkin.setTransparent(false);
                            FrameBorder fr;
                            fr.setSpacing(Size(2,2,2,2));
                            fr.setColor( Color(210,210,210) );
                            newSkin.setBorder(fr);
                          } else
                            if (identifier == "sbar_pane") {
                              newSkin.setBackground( Color( 244, 243, 238 ) );
                              newSkin.useParentBackground(false);
                              newSkin.setTransparent(false);
                            } else
                              if (identifier == "mbtnover") {
                                newSkin.setBackground( Color( 193, 210, 238 ) );     
                                newSkin.setTransparent( false );
                                newSkin.useParentBackground(false);
                                FrameBorder fr;
                                fr.setSpacing(Size(0,0,0,0));
                                fr.setColor( Color( 49, 106, 197 ) );
                                newSkin.setBorder(fr);
                              } else
                                if (identifier == "mbtnnone") {
                                } else
                                  if (identifier == "mbar") {
                                    newSkin.setGradient( Color(240,239,229), Color(238,235,217), Color(205,202,187),1,nVertical,87,3,3);
                                    newSkin.useParentBackground(false);
                                    newSkin.setTransparent(false);
                                  } else
                                    if (identifier == "clbox") {
                                    } else
                                      if ( identifier == "scb_btn_over" ) {
                                        newSkin.setGradient(Color(242,254,255),Color(220,237,253),Color(185,221,251),2,nVertical,10,5,5);
                                        newSkin.setTransparent( false );
                                        FrameBorder fr(true,5,5);
                                        fr.setSpacing(Size(0,0,0,0));
                                        fr.setColor (Color( 49, 106, 197 ) );
                                        newSkin.setBorder(fr);
                                      } else
                                        if ( identifier == "scb_btn_up" ) {
                                          newSkin.setGradient(Color(173,201,249),Color(220,237,253),Color(173,201,249),2,nVertical,10,5,5);
                                          newSkin.setTransparent( false );
                                          FrameBorder fr(true,5,5);
                                          fr.setSpacing(Size(0,0,0,0));
                                          fr.setColor (Color( 255, 255, 255 ) );
                                          newSkin.setBorder(fr);
                                        } else
                                          if ( identifier == "scb_btn_down" ) {
                                            return skin("ccbx_btn_down");
                                          } else
                                            if ( identifier == "ccbx_btn_down" ) {
                                              newSkin.setGradient(Color(117,158,252),Color(154,175,241),Color(196,212,234),2,nVertical,20,5,5);
                                              newSkin.setTransparent( false );
                                              FrameBorder fr(true,5,5);
                                              fr.setSpacing(Size(0,0,0,0));
                                              fr.setColor( Color( 189, 206, 247 ) );
                                              newSkin.setBorder(fr);
                                            } else
                                              if ( identifier == "ccbx_btn_up" ) {
                                                newSkin.setGradient(Color(216,227,252),Color(202,216,253),Color(173,201,249),2,nVertical,10,5,5);
                                                newSkin.setTransparent( false );
                                                FrameBorder fr(true,5,5);
                                                fr.setSpacing(Size(0,0,0,0));
                                                fr.setColor( Color( 189, 206, 247 ) );
                                                newSkin.setBorder(fr);
                                              } else
                                                if ( identifier == "ccbx_btn_over" ) {
                                                  newSkin.setGradient(Color(242,254,255),Color(220,237,253),Color(185,221,251),2,nVertical,10,5,5);
                                                  newSkin.setTransparent( false );
                                                  FrameBorder fr(true,5,5);
                                                  fr.setSpacing(Size(0,0,0,0));
                                                  fr.setColor( Color( 141, 169, 225 ) );
                                                  newSkin.setBorder(fr);
                                                } else
                                                  if (identifier == "btnup") {
                                                    newSkin.setGradient(Color(255,255,255),Color(243,243,239),Color(214,208,192),2,nVertical,50,5,5);
                                                    newSkin.setTransparent( false );
                                                    FrameBorder fr(true,5,5);
                                                    fr.setSpacing(Size(0,0,0,0));
                                                    fr.setColor( Color( 0, 60, 116 ) );
                                                    newSkin.setBorder(fr);
                                                  } else 
                                                    if (identifier == "btnover") {
                                                      newSkin.setBackground( Color( 193, 210, 238 ) );     
                                                      newSkin.setTransparent( false );
                                                      newSkin.useParentBackground(false);
                                                      FrameBorder fr;
                                                      fr.setSpacing(Size(0,0,0,0));
                                                      fr.setColor( Color( 49, 106, 197 ) );
                                                      newSkin.setBorder(fr);
                                                    } else 
                                                      if (identifier == "btndown") {
                                                        newSkin.setGradient(Color(200,200,200),Color(210,210,210),Color(180,180,180),2,nVertical,10,5,5);
                                                        newSkin.setTransparent( false );	 
                                                        FrameBorder fr(true,5,5);
                                                        fr.setSpacing(Size(0,0,0,0));
                                                        newSkin.setBorder(fr);
                                                      } else
                                                        if (identifier == "btnflat") {
                                                        } else
                                                          if (identifier == "tabup") {
                                                            newSkin.setGradient(  Color(255,255,255),Color(243,243,239),Color(214,208,192),2,nVertical,10,5,5);
                                                            newSkin.setTransparent( false );
                                                          } else
                                                            if (identifier == "tabnone") {
                                                              //fnt.setTextColor(Color(100,100,100));
                                                              //     skin.setFont(fnt);Color(236,233,216)
                                                              newSkin.setGradient( Color(255,255,255),Color(243,243,239),Color(236,233,216),1,nVertical,10,5,5);
                                                              newSkin.setTransparent( false );
                                                            } else
                                                              if (identifier == "ccbx") {
                                                                FrameBorder fr;
                                                                fr.setColor( Color(127, 157, 185) );
                                                                fr.setSpacing( Size( 2,2,2,2 ) );
                                                                newSkin.setBorder( fr ); 
                                                              } else
                                                                if (identifier == "scb_btn_over_vsl") {
                                                                  newSkin.setGradient(Color(242,254,255),Color(220,237,253),Color(185,221,251),2,nVertical,10,5,5);
                                                                  newSkin.setTransparent( false );
                                                                  FrameBorder fr(true,5,5);
                                                                  fr.setSpacing(Size(0,0,0,0));
                                                                  fr.setColor (Color( 49, 106, 197 ) );
                                                                  newSkin.setBorder(fr);
                                                                } else
                                                                  if (identifier == "scb_btn_down_vsl") {
                                                                    newSkin.setGradient(Color(242,254,255),Color(220,237,253),Color(185,221,251),2,nVertical,10,5,5);
                                                                    newSkin.setTransparent( false );
                                                                    FrameBorder fr(true,5,5);
                                                                    fr.setSpacing(Size(0,0,0,0));
                                                                    fr.setColor (Color( 49, 106, 197 ) );
                                                                    newSkin.setBorder(fr);
                                                                  } else
                                                                    if (identifier == "scb_btn_up_vsl") {
                                                                      newSkin.setGradient(Color(173,201,249),Color(220,237,253),Color(173,201,249),2,nHorizontal,10,5,5);
                                                                      newSkin.setTransparent( false );
                                                                      FrameBorder fr(true,5,5);
                                                                      fr.setSpacing(Size(0,0,0,0));
                                                                      fr.setColor (Color( 255, 255, 255 ) );
                                                                      newSkin.setBorder(fr);
                                                                    } else
                                                                      if (identifier == "scb_btn_up_hsl") {
                                                                        newSkin.setGradient(Color(173,201,249),Color(220,237,253),Color(173,201,249),2,nVertical,10,5,5);
                                                                        newSkin.setTransparent( false );
                                                                        FrameBorder fr(true,5,5);
                                                                        fr.setSpacing(Size(0,0,0,0));
                                                                        fr.setColor (Color( 255, 255, 255 ) );
                                                                        newSkin.setBorder(fr);
                                                                      } else
                                                                        if (identifier == "scb_btn_down_hsl") {
                                                                          return skin( "scb_btn_down_vsl" );
                                                                        } else
                                                                          if (identifier == "scb_btn_over_hsl") {
                                                                            return skin( "scb_btn_over_vsl" );
                                                                          } else
                                                                            if (identifier == "stat_bar_bg") {
                                                                              newSkin.setGradient( Color(169,167,153), Color(238,235,217), Color(223,220,203),1,nVertical,18,3,3);
                                                                              newSkin.useParentBackground(false);
                                                                              newSkin.setTransparent(false);	  
                                                                            } else
                                                                              if ( identifier == "tool_sep" ) {
                                                                                newSkin.setForeground( Color( 197, 194, 184 ) );
                                                                                newSkin.useParentForeground( false );
                                                                                newSkin.setTransparent( false );
                                                                              } else
                                                                                if ( identifier == "dockbar_bg" ) {
                                                                                  newSkin.setBackground( Color( 204, 199, 186 ) );
                                                                                  newSkin.useParentBackground( false );
                                                                                  newSkin.setTransparent( false );
                                                                                }

                                                                                return newSkin;
  }

  int Config::loadXmlConfig( const std::string & configName )
  {
    XmlParser parser;
    parser.tagParse.connect( this, &Config::onTagParse );
    return parser.parseFile( configName.c_str() );
  }

  Skin* Config::findSkin( const std::string & id )
  {
    std::map<std::string, Skin>::iterator itr;
    if ( (itr = skinMap.find(id)) == skinMap.end() )
    {
      return 0;
    } else {
      return &itr->second;
    }
  }

}
