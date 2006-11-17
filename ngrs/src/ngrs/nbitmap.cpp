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
#include "nbitmap.h"
#include "napp.h"
#include "nfile.h"
#ifdef __unix__
#else
#include <map>
#endif


NBitmap::NBitmap()
 : NObject(), depth_(24),width_(0),height_(0), data_(0)
 #ifdef __unix__
 ,xi(0), clp(0)
 #endif
{

}

NBitmap::NBitmap( const std::string & filename ) : NObject(), depth_(24),width_(0),height_(0),data_(0)
#ifdef __unix__
,xi(0), clp(0)
#endif
{
  loadFromFile(filename);
}

NBitmap::NBitmap( const char ** data ) : NObject(), depth_(24),width_(0),height_(0),data_(0)
#ifdef __unix__
,xi(0), clp(0)
#endif
{
  #ifdef __unix__
  createFromXpmData(data);
  #endif
}


NBitmap::~NBitmap()
{
  #ifdef __unix__
  if (xi)
		XDestroyImage(xi);
  if (clp)
		XDestroyImage(clp);
  #endif
}

unsigned char const * NBitmap::dataPtr( ) const
{
  #ifdef __unix__
  return (unsigned char*) xi->data;
  #endif
}

void NBitmap::setDepth( int depth )
{
  depth_ = depth;
}

int NBitmap::depth( ) const
{
  #ifdef __unix__
  return (xi != 0) ? xi->depth : 0;
  #endif
}

int NBitmap::width( ) const
{
  #ifdef __unix__
  return (xi != 0) ? xi->width : 0;
  #endif
}

int NBitmap::height( ) const
{
  #ifdef __unix__
  return (xi != 0) ? xi->height : 0;
  #endif
}

int NBitmap::pixelsize( ) const
{
 int pixelsize_;
 switch(depth())
      {
      case 8:
        pixelsize_=1;
        break;
      case 16:
        pixelsize_=2;
        break;
      case 24:
        pixelsize_=4;
        break;
      default:
        pixelsize_=1;
      }
  return pixelsize_;
}

#ifdef __unix__
XImage * NBitmap::X11data( ) const
{
  return xi;
}
#endif

NBitmap::NBitmap( const NBitmap & rhs ) : depth_(24),width_(0),height_(0),data_(0)
#ifdef __unix__
,xi(0),clp(0)
#endif
{
    #ifdef __unix__
	if (rhs.X11data() == 0) {
		// empty Bitmap
		// do just nothing
	} else {
		xi = cloneXImage( rhs.X11data() );
	}

	if (rhs.X11ClpData() == 0) {
		// empty Bitmap
		// do just nothing
	} else {
		clp = cloneXImage( rhs.X11ClpData() );
	}
 #endif
}


#ifdef __unix__
void NBitmap::setX11Data( XImage * ximage, XImage* clp_ )
{
  xi = ximage;
  clp = clp_;
  //data_ = (unsigned char*) xi->data;
}
#endif

const NBitmap & NBitmap::operator =( const NBitmap & rhs )
{      
	deleteBitmapData();

    #ifdef __unix__
	if (rhs.X11data() == 0) {
		// empty Bitmap
		// do just nothing
	} else {
		xi = cloneXImage( rhs.X11data() );
	}

	if (rhs.X11ClpData() == 0) {
		// empty Bitmap
		// do just nothing
	} else {
		clp = cloneXImage( rhs.X11ClpData() );
	}

  #endif	
  return *this;
}

void NBitmap::loadFromFile( const std::string & filename )
{
	deleteBitmapData();

  try {
    NBitmap bmp1 = NApp::filter.at(0)->loadFromFile(NFile::replaceTilde(filename));
    #ifdef __unix__
    xi  = cloneXImage( bmp1.X11data() );
    clp = cloneXImage( bmp1.X11ClpData() );
    #endif
  } catch (const char* e) {
     throw "couldn`t open file";
  }
}


void NBitmap::createFromXpmData(const char** data)
{
	deleteBitmapData();
  #ifdef __unix__
  XpmColorSymbol cs[256];
  XpmAttributes attr;
  attr.valuemask = XpmCloseness;
  attr.colorsymbols = cs;
  attr.numsymbols = 256;
  attr.color_key = XPM_GRAY;

  XImage* xi1;
  int err = XpmCreateImageFromData(NApp::system().dpy(),(char**)(data),&xi1,&clp,0);
  if (err == XpmSuccess) {
     xi = xi1;
  }
  #else
  
  // code from ngrs0.8 .. needs rewrite
/*  const char* picInfo = data[0];  
  std::vector<int> breakList;
  int size = strlen(picInfo);
  
  for (int i = 0; i < size; i++) {    
    if (picInfo[i]==' ') {
      breakList.push_back(i);
    }  
  }
  if (picInfo[size-1]!=' ') breakList.push_back(size);

  std::vector<int>::iterator it = breakList.begin();
  
  char numBuf[20]; 
  int prevP = 0;
  int point = *it;

  int xwidth_ = 0;
  int xheight_ = 0;   

  memcpy(numBuf,picInfo+prevP,point-prevP); numBuf[point]='\0'; prevP = point; it++; point = *it;
  int width   = xwidth_ = atoi(numBuf); 
  memcpy(numBuf,picInfo+prevP,point-prevP); numBuf[point-prevP]='\0'; prevP = point; it++; point = *it;
  int height  = xheight_ = atoi(numBuf); 
  memcpy(numBuf,picInfo+prevP,point-prevP); numBuf[point-prevP]='\0'; prevP = point; it++; point = *it;
  int ncolors = atoi(numBuf); 
  memcpy(numBuf,picInfo+prevP,point-prevP); numBuf[point-prevP]='\0'; 
  int ncpp     = atoi(numBuf); 
  int flagPos  = ncpp+1;
  int colorPos = ncpp+4;
    
  std::map<std::string,long> colorTable;
  
  for (int i = 0; i< ncolors; i++) {
    const char* colorLine = data[i+1];
    char key[20];   strncpy(key,colorLine,ncpp); key[ncpp]='\0';
    char value[20]; strncpy(value,colorLine+colorPos,6); value[6]='\0';    
    char red[3];   sprintf(red,"%.2s\n",value);  
    char green[3]; sprintf(green,"%.2s\n",value+2);
    char blue[3];  sprintf(blue,"%.2s\n",value+4);    
    int r = strtol( red, (char **)NULL, 16 );
    int g = strtol( green, (char **)NULL, 16 );
    int b = strtol( blue, (char **)NULL, 16 );
    long int color = ((r<<16) | (g<<8) | b);
    colorTable[std::string(key)] = color;    
  }*/
  // end of ugly code
  
//  bmp = CreateBitmap( xwidth_, xheight_, 
  
  #endif
}

#ifdef __unix__
XImage * NBitmap::X11ClpData( ) const
{
  return clp;
}
#endif

#ifdef __unix__
XImage * NBitmap::cloneXImage( XImage * src_xi )
{
  if ( src_xi ) {
		int width  = src_xi->width;
		int height = src_xi->height;
		int depth  = src_xi->depth;
		int pad    = src_xi->bitmap_pad;
		int bytes_per_line = src_xi->bytes_per_line;

		int pixelsize = NApp::system().pixelSize( depth );

		unsigned char* src_data = reinterpret_cast<unsigned char*> (src_xi->data);
		unsigned char* dst_data = 0;

		dst_data = reinterpret_cast<unsigned char*> ( malloc( bytes_per_line * height ) );
		memcpy( dst_data, src_data, bytes_per_line * height );				

  	XImage* dst_xi = XCreateImage(NApp::system().dpy(), NApp::system().visual(), depth, ZPixmap,0,(char*) dst_data , width, height, pad , bytes_per_line );

		return dst_xi;
	}

  return 0;
}
#endif

void NBitmap::deleteBitmapData( )
{
  #ifdef __unix__
  if (xi) {
		// destroys the image
		XDestroyImage(xi);
		xi = 0;
	}

	if (clp) {
		// destroys the transparent image mask
		XDestroyImage(clp);
		clp = 0;
	}
	#endif
}

bool NBitmap::empty() const {
  #ifdef __unix__
  return !(xi || clp);
  #else
  return true;
  #endif
}

