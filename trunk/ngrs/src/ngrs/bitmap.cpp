/***************************************************************************
 *   Copyright (C) 2005, 2006, 2007 by Stefan Nattkemper  *
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
#include "bitmap.h"
#include "app.h"
#include "file.h"
#ifdef __unix__
#else
#include <time.h>
#include <map>
#endif

namespace ngrs {

  template<class T> inline T str2hex( const std::string &  value, int pos ) {	
    T result;	
    std::stringstream str;
    str << value.substr( pos, 2 );
    str >> std::hex >> result;
    return result;
  }

  Bitmap::Bitmap()
    : Object(), sysData_(0), clpData_(0), clpColor_(0)
  {
  }

  Bitmap::Bitmap( const std::string & filename ) : Object(), sysData_(0), clpData_(0), clpColor_(0)
  {
    loadFromFile(filename);
  }

  Bitmap::Bitmap( const char ** data ) : Object(), sysData_(0), clpData_(0), clpColor_(0)
  {
    createFromXpmData(data);
  }

  Bitmap::Bitmap( const Bitmap & rhs ) : Object(), sysData_(0), clpData_(0), clpColor_(0)
  {
    if ( rhs.sysData() ) {
      sysData_ = cloneSysImage( rhs.sysData() );
    }

    if ( rhs.clpData() ) {
      clpData_ = cloneSysImage( rhs.clpData() );
      clpData_ = 0;
    }
  }

  const Bitmap & Bitmap::operator =( const Bitmap & rhs ) {
    deleteBitmapData();    

    if ( rhs.sysData() ) {
      sysData_ = cloneSysImage( rhs.sysData() );
    }		

    if ( rhs.clpData() ) {       
      clpData_ = cloneSysImage( rhs.clpData() );      
    }		

    return *this;		
  }      


  Bitmap::~Bitmap()
  {
    deleteBitmapData();                   
  }

  int Bitmap::depth( ) const
  {
#ifdef __unix__
    return ( sysData_) ? sysData_->depth : 0;  
#else
    return 24;
#endif
  }

  int Bitmap::width( ) const
  {
#ifdef __unix__
    return ( sysData_ ) ? sysData_->width : 0;
#else
    if ( sysData_ ) {
      BITMAP bitmap;       
      GetObject( sysData_, sizeof(BITMAP), (LPSTR)&bitmap );
      return bitmap.bmWidth;
    }  
    return 0;
#endif
  }

  int Bitmap::height( ) const
  {
#ifdef __unix__
    return ( sysData_ ) ? sysData_->height : 0;
#else
    if ( sysData_ ) {
      BITMAP bitmap;       
      GetObject( sysData_, sizeof(BITMAP), (LPSTR)&bitmap );
      return bitmap.bmHeight;
    }  
    return 0;
#endif
  }

  int Bitmap::pixelsize( ) const
  {
    int pixelsize_;
    switch( depth() )
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


  NSysImage Bitmap::sysData( ) const
  {
    return sysData_;
  }

  NSysImage Bitmap::clpData( ) const
  {
    return clpData_;
  }


  void Bitmap::setSysImgData( NSysImage data, NSysImage clp ) {
    sysData_ = data;
    clpData_ = clp;
  }  

  void Bitmap::loadFromFile( const std::string & filename )
  {
    deleteBitmapData();

    try {
      Bitmap bmp1 = App::filter.at(0)->loadFromFile(ngrs::File::replaceTilde(filename));

      sysData_ = cloneSysImage( bmp1.sysData() );
      clpData_ = cloneSysImage( bmp1.clpData() );
    } catch (const char* e) {
      throw "couldn`t open file";
    }
  }


  void Bitmap::createFromXpmData(const char** data)
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
    int err = XpmCreateImageFromData(App::system().dpy(),(char**)(data),&xi1,&clpData_,0);
    if (err == XpmSuccess) {
      sysData_ = xi1;
    }
#else

    clpColor_ = 02;
    // code from ngrs0.8 .. needs rewrite
    const char* picInfo = data[0];  
    std::vector< size_t > breakList;
    size_t size = strlen(picInfo);

    for ( size_t i = 0; i < size; i++) {    
      if ( picInfo[i]==' ' ) {
        breakList.push_back( i );
      }  
    }
    if ( picInfo[ size-1 ] != ' ' ) breakList.push_back( size );

    std::vector<size_t>::iterator it = breakList.begin();

    char numBuf[20]; 
    size_t prevP = 0;
    size_t point = *it;

    int xwidth_ = 0;
    int xheight_ = 0;   

    memcpy(numBuf,picInfo+prevP,point-prevP); numBuf[point]='\0'; prevP = point; it++; point = *it;
    int width   = xwidth_ = atoi(numBuf); 
    memcpy(numBuf,picInfo+prevP,point-prevP); numBuf[point-prevP]='\0'; prevP = point; it++; point = *it;
    int height  = xheight_ = atoi(numBuf); 
    memcpy(numBuf,picInfo+prevP,point-prevP); numBuf[point-prevP]='\0'; prevP = point; it++; point = *it;
    int ncolors = atoi(numBuf); 
    memcpy(numBuf,picInfo+prevP,point-prevP); numBuf[point-prevP]='\0';
    unsigned int ncpp     = atoi( numBuf ); 
    int flagPos  = ncpp+1;
    int colorPos = ncpp+4;

    std::map<std::string,long> colorTable;
    bool trans = false;
    std::string transKey;
    srand( static_cast< unsigned int >(time( NULL )) );

    for (int i = 0; i< ncolors; i++) {
      std::string colorLine( data[i+1] );  
      std::string key   = colorLine.substr(0,ncpp);
      std::string value;
      if ( ncpp + 3 < colorLine.length() ) 
        value = colorLine.substr(3+ncpp);
      else
        value = "None";

      long int color  = 255;

      if ( value.find("None")  != std::string::npos ) {
        color =  clpColor_; 
        trans = true;
        colorTable[ key ] = color;
        transKey = key;
      } else {
        if ( value.find("black") != std::string::npos ) {
          color =  0; 
        } else {
          // convert #rrggbb xpm color string to r,g,b int values 
          int r = str2hex<int>( value, 1 );
          int g = str2hex<int>( value, 3 );
          int b = str2hex<int>( value, 5 );

          color = (b << 16) | (g << 8) | r;
        }        
        colorTable[key] = color;

        if ( trans ) {
          // check if new clp Color is needed
          if ( clpColor_ == color ) {
            // create new random clp color

            bool generateClpAgain;
            do {
              generateClpAgain = false;  
              clpColor_ = (int) ( rand() * 2147483647 );
              std::map<std::string,long>::iterator it = colorTable.begin();
              for ( ; it != colorTable.end(); it++ ) {
                long c = it->second;
                if ( c == clpColor_ ) {
                  generateClpAgain = true;
                  break;
                }                  
              }
              if ( !generateClpAgain ) {                
                colorTable[ transKey ] = clpColor_;
              }                
            } while ( generateClpAgain );
          }     
        }     
      }
    } 
    // end of ngrs0.8 code

    HDC dc = GetDC( NULL );
    HDC memDC_ = CreateCompatibleDC( dc );

    sysData_ = CreateCompatibleBitmap( dc, xwidth_, xheight_ );
    HBITMAP oldBitmap = (HBITMAP)SelectObject( memDC_, sysData_ );

    for ( int y=0; y < height; y++ ) {  
      const char* scanLine = data[1+ncolors+y];
      for ( int x=0; x < width; x++ ) {
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
	SelectObject( memDC_, oldBitmap );
    ReleaseDC( NULL, dc );
    DeleteDC( memDC_ );

    if ( trans ) {  
      clpData_ = createClipMask( sysData_ , clpColor_ );
    } 

#endif
  }


  NSysImage Bitmap::cloneSysImage( NSysImage src_img )
  {
    if ( src_img ) {
#ifdef __unix__
      int width  = src_img->width;
      int height = src_img->height;
      int depth  = src_img->depth;
      int pad    = src_img->bitmap_pad;
      int bytes_per_line = src_img->bytes_per_line;

      int pixelsize = App::system().pixelSize( depth );

      unsigned char* src_data = reinterpret_cast<unsigned char*> (src_img->data);
      unsigned char* dst_data = 0;

      dst_data = reinterpret_cast<unsigned char*> ( malloc( bytes_per_line * height ) );
      memcpy( dst_data, src_data, bytes_per_line * height );				

      XImage* dst_xi = XCreateImage(App::system().dpy(), App::system().visual(), depth, ZPixmap,0,(char*) dst_data , width, height, pad , bytes_per_line );

      return dst_xi;
#else
      HBITMAP dest_img = (HBITMAP) CopyImage( src_img, IMAGE_BITMAP, 0, 0 , LR_COPYRETURNORG );
      return dest_img;
#endif
    }

    return 0;
  }


  void Bitmap::deleteBitmapData( )
  {
    if ( sysData_ ) {
#ifdef __unix__
      XDestroyImage( sysData_ );
#else 
      DeleteObject( sysData_ );
#endif
      sysData_ = 0;			
    }		
    if ( clpData_ ) {
#ifdef __unix__
      XDestroyImage( clpData_ );
#else 
      DeleteObject( clpData_ );
#endif	     
      clpData_  = 0;
    }     
  }

  bool Bitmap::empty() const {
    return !( sysData_ || clpData_ );
  }

#ifdef __unix__
#else

  HBITMAP Bitmap::createClipMask(HBITMAP hbmColour, COLORREF crTransparent)
  {
    HDC hdcMem, hdcMem2;
    HBITMAP hbmMask;
    BITMAP bm;

    // Create monochrome (1 bit) mask bitmap.  

    GetObject(hbmColour, sizeof(BITMAP), &bm);
    hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

    // Get some HDCs that are compatible with the display driver

    hdcMem = CreateCompatibleDC(0);
    hdcMem2 = CreateCompatibleDC(0);

    HBITMAP oldColour = (HBITMAP)SelectObject(hdcMem, hbmColour);
    HBITMAP oldMask = (HBITMAP)SelectObject(hdcMem2, hbmMask);

    // Set the background colour of the colour image to the colour
    // you want to be transparent.
    SetBkColor(hdcMem, crTransparent);

    // Copy the bits from the colour image to the B+W mask... everything
    // with the background colour ends up white while everythig else ends up
    // black...Just what we wanted.

    BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

    // Take our new mask and use it to turn the transparent colour in our
    // original colour image to black so the transparency effect will
    // work right.
    BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

    // Clean up.

	SelectObject(hdcMem, oldColour);
	SelectObject(hdcMem2, oldMask);
    DeleteDC(hdcMem);
    DeleteDC(hdcMem2);

    return hbmMask;
  } 

#endif

}
