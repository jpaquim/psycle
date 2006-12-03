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
 #else
 , hBmp(0), memDC_(0)
 #endif
{
}

NBitmap::NBitmap( const std::string & filename ) : NObject(), depth_(24),width_(0),height_(0),data_(0)
#ifdef __unix__
,xi(0), clp(0)
#else
,hBmp(0),  memDC_(0)
#endif
{
  loadFromFile(filename);
}

NBitmap::NBitmap( const char ** data ) : NObject(), depth_(24),width_(0),height_(0),data_(0)
#ifdef __unix__
,xi(0), clp(0)
#else
,hBmp(0), memDC_(0)
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
  #else
  if ( hBmp ) {

  }
  if ( memDC_ ) {
    DeleteDC( memDC_ );
  }
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
  #else
  if ( hBmp ) {
    BITMAP bitmap;       
    GetObject( hBmp, sizeof(BITMAP), (LPSTR)&bitmap);
    return bitmap.bmWidth;
  }  
  return 0;
  #endif
}

int NBitmap::height( ) const
{
  #ifdef __unix__
  return (xi != 0) ? xi->height : 0;
  #else
  if ( hBmp ) {
    BITMAP bitmap;       
    GetObject( hBmp, sizeof(BITMAP), (LPSTR)&bitmap);
    return bitmap.bmHeight;
  }  
  return 0;
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
  #else
  if ( rhs.hdata() ) {
    memDC_ = CreateCompatibleDC( rhs.memDC() );
    hBmp = CreateCompatibleBitmap( rhs.memDC(), rhs.width(), rhs.height() );
    SelectObject( memDC_, hBmp );
    BitBlt( memDC_, 0,0, rhs.width(), rhs.height(), rhs.memDC(), 0, 0, SRCCOPY);
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
  #else
  if ( rhs.hdata() ) {
    memDC_ = CreateCompatibleDC( rhs.memDC() );
    hBmp = CreateCompatibleBitmap( rhs.memDC(), rhs.width(), rhs.height() );
    SelectObject( memDC_, hBmp );
    BitBlt( memDC_, 0,0, rhs.width(), rhs.height(), rhs.memDC(), 0, 0, SRCCOPY);
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
  const char* picInfo = data[0];  
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
    std::string colorLine( data[i+1] );  
    std::string key   = colorLine.substr(0,ncpp);
    std::string value;
    if ( ncpp + 4 < colorLine.length() ) 
      value = std::string(colorLine).substr(4+ncpp);
    else
      value = "None";
        
    long int color  = 255;
    if ( value.find("None")  != std::string::npos ) color =  ((255<<16) | (255<<8) | 255); else
    if ( value.find("black") != std::string::npos ) color =  0; else
    {
      std::string vstr = std::string(value).substr(1);  
      char red[3];   sprintf(red,"%.2s\n",value.c_str());  
      char green[3]; sprintf(green,"%.2s\n",value.c_str()+2);
      char blue[3];  sprintf(blue,"%.2s\n",value.c_str()+4);    
      int r = strtol( red, (char **)NULL, 16 );
      int g = strtol( green, (char **)NULL, 16 );
      int b = strtol( blue, (char **)NULL, 16 );
      
      color = (b << 16) | (g << 8) | r;
    }
    colorTable[std::string(key)] = color;    
  }
  // end of ngrs0.8 code
  
  HDC dc = GetDC( NULL );

  memDC_ = CreateCompatibleDC( dc );
  hBmp = CreateCompatibleBitmap( dc, xwidth_, xheight_ );
  SelectObject( memDC_, hBmp );
    
  for (int y=0; y<height; y++) {  
     const char* scanLine = data[1+ncolors+y];
     for (int x=0; x<width; x++) {
        long colorValue = 0;
        char pixel[20]; memcpy(pixel,scanLine+x*ncpp,ncpp); pixel[ncpp]='\0';
        std::map<std::string,long>::iterator itr;
        if ( (itr = colorTable.find(std::string(pixel))) != colorTable.end()) 
          colorValue = itr->second; 
        else 
          colorValue = 200;
        SetPixel( memDC_, x, y,  colorValue );
     }
  }
    
  #endif
}

#ifdef __unix__
XImage * NBitmap::X11ClpData( ) const
{
  return clp;
}
#else
HBITMAP NBitmap::hdata() const {
  return hBmp;       
}        

HDC NBitmap::memDC() const {
  return memDC_;    
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

