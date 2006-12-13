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
#include "nxpmfilter.h"
#include "napp.h"
#include "nfile.h"

NXPMFilter::NXPMFilter()
 : NImgFilter()
{
}


NXPMFilter::~NXPMFilter()
{
}

NBitmap NXPMFilter::loadFromFile( const std::string & filename )
{
  #ifdef __unix__
  XpmColorSymbol cs[256];
  XpmAttributes attr;
  attr.valuemask = XpmCloseness;
  attr.colorsymbols = cs;
  attr.numsymbols = 256;
  attr.color_key = XPM_GRAY;

  XImage* xi;
  XImage* clp;
  int err = XpmReadFileToImage(NApp::system().dpy(),(char*) NFile::replaceTilde(filename).c_str(),&xi,&clp,0);

 /* for (int j = 0; j< xi->height; j++)
  for (int i = 0; i< xi->width; i++) {
    printf("%d",xi->data[i*j]);
  }
  printf("\n");
  exit(0);*/

  NBitmap bitmap;
  if (err == XpmSuccess) {
     bitmap.setSysImgData( xi, clp );
  } else throw "couldn`t open file";
  
  return bitmap;
  #endif
}


