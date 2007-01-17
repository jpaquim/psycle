/***************************************************************************
 *   Copyright (C) 2006 by  Stefan Nattkemper   *
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
#include "zoombar.h"
#include <ngrs/button.h>
#include <ngrs/slider.h>
#include <ngrs/alignlayout.h>
#include <ngrs/image.h>

/* XPM */
const char * zoomin_xpm[] = {
"16 16 100 2",
"  	c None",
". 	c #856C6F",
"+ 	c #E8E6E7",
"@ 	c #FFFEFF",
"# 	c #E5E3E6",
"$ 	c #F0EEF3",
"% 	c #8F8F99",
"& 	c #EEEFF7",
"* 	c #EBECF1",
"= 	c #F9FAFE",
"- 	c #F3F4F8",
"; 	c #EAEBEF",
"> 	c #E2E3E7",
", 	c #B7BBC6",
"' 	c #8F96A6",
") 	c #A4AAB6",
"! 	c #BFC5D1",
"~ 	c #FAFBFD",
"{ 	c #F9FAFC",
"] 	c #F3F4F6",
"^ 	c #E6E7E9",
"/ 	c #8C95A6",
"( 	c #E8EBF0",
"_ 	c #E4E7EC",
": 	c #AFB7C2",
"< 	c #828C98",
"[ 	c #DADFE5",
"} 	c #B2B9C1",
"| 	c #929BA2",
"1 	c #D5DADE",
"2 	c #E3E7EA",
"3 	c #DDE1E4",
"4 	c #DADEE1",
"5 	c #006CAD",
"6 	c #016BAC",
"7 	c #0168AA",
"8 	c #0168A9",
"9 	c #0166A7",
"0 	c #0166A6",
"a 	c #0164A5",
"b 	c #006EB0",
"c 	c #016EAF",
"d 	c #016CAC",
"e 	c #C0C5C8",
"f 	c #DBE0E3",
"g 	c #BBBFC0",
"h 	c #E8ECED",
"i 	c #E3E7E8",
"j 	c #DCE0E1",
"k 	c #808A8B",
"l 	c #D3D9D9",
"m 	c #E8EEEE",
"n 	c #F9FBFA",
"o 	c #F6F8F7",
"p 	c #F3F5F4",
"q 	c #B7BEB7",
"r 	c #B5B7B4",
"s 	c #949790",
"t 	c #D3D6CB",
"u 	c #B1B499",
"v 	c #9FA098",
"w 	c #D2D2D0",
"x 	c #D1D1CF",
"y 	c #BDBCAA",
"z 	c #BCB895",
"A 	c #AEA773",
"B 	c #A39E78",
"C 	c #A19D82",
"D 	c #CAC6A9",
"E 	c #A6A495",
"F 	c #B5AD7F",
"G 	c #D2CEB5",
"H 	c #C6C4B8",
"I 	c #B1A776",
"J 	c #E2E1DC",
"K 	c #A99E70",
"L 	c #CCC7B3",
"M 	c #B8AB7F",
"N 	c #C8C1AE",
"O 	c #E0DCD1",
"P 	c #BCB9B0",
"Q 	c #9A9997",
"R 	c #FFFEFC",
"S 	c #F3F2F0",
"T 	c #9D9996",
"U 	c #9E8A7F",
"V 	c #79503E",
"W 	c #775041",
"X 	c #89726A",
"Y 	c #86726B",
"Z 	c #7B4F42",
"` 	c #8C716A",
" .	c #8C756F",
"..	c #9D8883",
"+.	c #F8F4F3",
"@.	c #F1EDEC",
"#.	c #8A716C",
"$.	c #877571",
"%.	c #FFFFFF",
"&.	c #FBFBFB",
"                                ",
"      G A F M I D &             ",
"    z N # S p ] O K &           ",
"  L H + @ a a R &.@.B           ",
"  C %.~ n a a = { o u J         ",
"  E %.a a a 9 8 d - t y         ",
"  v %.a a 0 8 6 b $ x P         ",
"  s %.; * 7 d ( h m r w         ",
"  q g 4 i 5 c 2 _ ^ k           ",
"    % e l [ f j 3 | T           ",
"      ' / } , ) < Q +.U         ",
"        > ! : 1     ..V `       ",
"                      #.W X     ",
"                        Y Z  .  ",
"                          $..   ",
"                                "};

/* XPM */
const char * zoomout_xpm[] = {
"16 16 101 2",
"  	c None",
". 	c #856C6F",
"+ 	c #E8E6E7",
"@ 	c #FFFEFF",
"# 	c #E5E3E6",
"$ 	c #F0EEF3",
"% 	c #8F8F99",
"& 	c #EBECF1",
"* 	c #F9FAFE",
"= 	c #F3F4F8",
"- 	c #EAEBEF",
"; 	c #E2E3E7",
"> 	c #B7BBC6",
", 	c #8F96A6",
"' 	c #E4E7EE",
") 	c #A4AAB6",
"! 	c #BFC5D1",
"~ 	c #FCFDFF",
"{ 	c #FAFBFD",
"] 	c #F9FAFC",
"^ 	c #F3F4F6",
"/ 	c #E6E7E9",
"( 	c #8C95A6",
"_ 	c #E8EBF0",
": 	c #E4E7EC",
"< 	c #E2E5EA",
"[ 	c #AFB7C2",
"} 	c #828C98",
"| 	c #DADFE5",
"1 	c #B2B9C1",
"2 	c #929BA2",
"3 	c #D5DADE",
"4 	c #E3E7EA",
"5 	c #DDE1E4",
"6 	c #DADEE1",
"7 	c #016EB0",
"8 	c #0168A9",
"9 	c #0164A5",
"0 	c #0075B5",
"a 	c #0175B6",
"b 	c #016FAF",
"c 	c #C0C5C8",
"d 	c #DBE0E3",
"e 	c #0079BA",
"f 	c #BBBFC0",
"g 	c #E9EDEE",
"h 	c #E8ECED",
"i 	c #E3E7E8",
"j 	c #DCE0E1",
"k 	c #808A8B",
"l 	c #D3D9D9",
"m 	c #E8EEEE",
"n 	c #F9FBFA",
"o 	c #F6F8F7",
"p 	c #F3F5F4",
"q 	c #B7BEB7",
"r 	c #B5B7B4",
"s 	c #949790",
"t 	c #D3D6CB",
"u 	c #B1B499",
"v 	c #9FA098",
"w 	c #FEFEFC",
"x 	c #D2D2D0",
"y 	c #D1D1CF",
"z 	c #BDBCAA",
"A 	c #BCB895",
"B 	c #AEA773",
"C 	c #A39E78",
"D 	c #A19D82",
"E 	c #CAC6A9",
"F 	c #A6A495",
"G 	c #B5AD7F",
"H 	c #D2CEB5",
"I 	c #C6C4B8",
"J 	c #B1A776",
"K 	c #E2E1DC",
"L 	c #A99E70",
"M 	c #CCC7B3",
"N 	c #B8AB7F",
"O 	c #C8C1AE",
"P 	c #E0DCD1",
"Q 	c #BCB9B0",
"R 	c #9A9997",
"S 	c #FFFEFC",
"T 	c #F3F2F0",
"U 	c #9D9996",
"V 	c #9E8A7F",
"W 	c #79503E",
"X 	c #775041",
"Y 	c #89726A",
"Z 	c #86726B",
"` 	c #7B4F42",
" .	c #8C716A",
"..	c #8C756F",
"+.	c #9D8883",
"@.	c #F8F4F3",
"#.	c #F1EDEC",
"$.	c #8A716C",
"%.	c #877571",
"&.	c #FFFFFF",
"*.	c #FBFBFB",
"                                ",
"      H B G N J E               ",
"    A O # T p ^ P L             ",
"  M I + @ ~ w S *.#.C           ",
"  D &.{ n ] ] * ] o u K         ",
"  F &.9 9 8 7 0 e = t z         ",
"  v &.9 8 b a e e $ y Q         ",
"  s &.- & - g _ h m r x         ",
"  q f 6 i < ' 4 : / k           ",
"    % c l | d j 5 2 U           ",
"      , ( 1 > ) } R @.V         ",
"        ; ! [ 3     +.W  .      ",
"                      $.X Y     ",
"                        Z ` ..  ",
"                          %..   ",
"                                "};

/* XPM */
const char * sl_xpm[] = {
"8 15 113 2",
"  	c #EDF7FD",
". 	c #EEF8FE",
"+ 	c #F1FBFE",
"@ 	c #E3EDFE",
"# 	c #F2FAFE",
"$ 	c #E8F1FE",
"% 	c #F1FAFD",
"& 	c #F2FBFE",
"* 	c #BBC6E8",
"= 	c #C2CCEF",
"- 	c #BFC9EC",
"; 	c #BEC8EB",
"> 	c #C0CBEE",
", 	c #C0CBEA",
"' 	c #BCC8E6",
") 	c #C3CFEC",
"! 	c #CBD8F8",
"~ 	c #CEDCFB",
"{ 	c #C5D2F2",
"] 	c #CFDDFC",
"^ 	c #CFDEFA",
"/ 	c #C7D5F2",
"( 	c #CAD8F6",
"_ 	c #D6E5FC",
": 	c #C6D6F2",
"< 	c #D1E1F8",
"[ 	c #C6D7F3",
"} 	c #D1E2F8",
"| 	c #C6D8F4",
"1 	c #D1E3FA",
"2 	c #C2D4F2",
"3 	c #E7F7FF",
"4 	c #BDCFE9",
"5 	c #E8FAFF",
"6 	c #B6CBE6",
"7 	c #E9FDFF",
"8 	c #B7CCEB",
"9 	c #E2F8FF",
"0 	c #BACFF2",
"a 	c #E1F3FF",
"b 	c #A3B8D4",
"c 	c #E6FAFF",
"d 	c #9AB0CF",
"e 	c #E7FCFF",
"f 	c #9BB2D7",
"g 	c #DEF4FF",
"h 	c #A1B9E2",
"i 	c #E3F6FF",
"j 	c #9FB3D5",
"k 	c #E6FBFF",
"l 	c #96ADD2",
"m 	c #E7FDFF",
"n 	c #97AEDA",
"o 	c #DEF7FF",
"p 	c #9AB2E4",
"q 	c #E4F8FF",
"r 	c #9EB0DA",
"s 	c #E3F8FF",
"t 	c #96AAD8",
"u 	c #99B0E4",
"v 	c #DEF8FF",
"w 	c #96B0E8",
"x 	c #9EB0E0",
"y 	c #DDF1FF",
"z 	c #98ACE0",
"A 	c #E2F7FF",
"B 	c #9DB3EB",
"C 	c #DCF4FF",
"D 	c #97B1ED",
"E 	c #CEE0FF",
"F 	c #9DAFE1",
"G 	c #CCE0FF",
"H 	c #9CB0E5",
"I 	c #CDE2FF",
"J 	c #9CB2EC",
"K 	c #CCE2FF",
"L 	c #99B2EE",
"M 	c #C2D4FF",
"N 	c #A7B9EA",
"O 	c #C1D6FF",
"P 	c #AABEF1",
"Q 	c #BED2FF",
"R 	c #A6BBF2",
"S 	c #C2D7FF",
"T 	c #A7BDF5",
"U 	c #BFD2FD",
"V 	c #B8CBF6",
"W 	c #C0D4FE",
"X 	c #BDD0FC",
"Y 	c #BCCFFB",
"Z 	c #B7CCFA",
"` 	c #C0D5FF",
" .	c #B8CDFD",
"..	c #B7CBED",
"+.	c #BACEF1",
"@.	c #B7CBEE",
"#.	c #BDD1F3",
"$.	c #B5C8ED",
"%.	c #BBCFF4",
"&.	c #B7CBF0",
"*.	c #E4F8FC",
"=.	c #E8F9FD",
"-.	c #E2F6FC",
";.	c #E5F8FC",
">.	c #E2F6FD",
",.	c #A5B9CF",
"'.	c #ADC1D8",
").	c #A2B6CC",
"!.	c #A6BAD0",
"~.	c #A8BCD0",
"{.	c #ABC0D4",
"].	c #A2B4C7",
"^.	c #AABCCF",
"  . + @ # $ % & ",
"* = - ; > , ' ) ",
"! ~ { ] { ^ / ( ",
"_ : < [ } | 1 2 ",
"3 4 5 6 7 8 9 0 ",
"a b c d e f g h ",
"i j k l m n o p ",
"q r s t m u v w ",
"a x y z A B C D ",
"E F G H I J K L ",
"M N O P Q R S T ",
"U V W X Y Z `  .",
"..+.@.#.$.%.&.0 ",
"*.=.-.=.;.=.>.=.",
",.'.).!.~.{.].^."};



ZoomBar::ZoomBar()
 : ngrs::Panel()
{
  init();
}

ZoomBar::~ZoomBar()
{
}

void ZoomBar::init( )
{
  orientation_ = ngrs::nHorizontal;
  increment_ = 1;

  setLayout( ngrs::AlignLayout() );

  zoomInBpm.createFromXpmData(zoomin_xpm);
  zoomOutBpm.createFromXpmData(zoomout_xpm);
  sliderBpm.createFromXpmData(sl_xpm);

  ngrs::Image* img = new ngrs::Image(zoomOutBpm);

  decBtn = new ngrs::Button(img);
    decBtn->setFlat(false);
    decBtn->setPreferredSize(20,15);
    decBtn->setFlat(true);
    decBtn->clicked.connect( this, &ZoomBar::onDecButton );
  add(decBtn, ngrs::nAlLeft);

  img = new ngrs::Image(zoomInBpm);

  incBtn = new ngrs::Button(img);
    incBtn->setFlat(false);
    incBtn->setPreferredSize(20,15);
    incBtn->setFlat(true);
    incBtn->clicked.connect( this, &ZoomBar::onIncButton );
  add(incBtn, ngrs::nAlRight);

  zoomSlider = new ngrs::Slider();
    zoomSlider->setOrientation(ngrs::nHorizontal);
    zoomSlider->setPreferredSize(120,15);
    zoomSlider->change.connect(this, &ZoomBar::onPosChanged);
    zoomSlider->customSliderPaint.connect(this,&ZoomBar::customSliderPaint);
  add(zoomSlider, ngrs::nAlClient);
}

void ZoomBar::setOrientation( int orientation )
{
  orientation_ = orientation;
}

int ZoomBar::orientation( ) const
{
  return orientation_;
}

void ZoomBar::onPosChanged( ngrs::Slider * slider )
{
  posChanged.emit(this, slider->pos() );
}

void ZoomBar::setRange( double min, double max )
{
  zoomSlider->setRange(min, max);
}

void ZoomBar::setPos( double pos )
{
  zoomSlider->setPos( pos );
}

double ZoomBar::pos( ) const
{
  return zoomSlider->pos();
}

void ZoomBar::customSliderPaint( ngrs::Slider * sl, ngrs::Graphics& g )
{
  g.putBitmap(0,0,sliderBpm.width(),sliderBpm.height(),sliderBpm,0,0);
}

void ZoomBar::onIncButton( ngrs::ButtonEvent * ev )
{
  zoomSlider->setPos( zoomSlider->pos() + increment_ );
}

void ZoomBar::onDecButton( ngrs::ButtonEvent * ev )
{
  zoomSlider->setPos(zoomSlider->pos() - increment_);
}


