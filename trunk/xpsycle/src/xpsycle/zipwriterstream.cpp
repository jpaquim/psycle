/***************************************************************************
 *   Copyright (C) 2006 by Stefan Nattkemper   *
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
#include "zipwriterstream.h"
#include "zipwriter.h"

// the streambuffer descent

zipfilestreambuf::zipfilestreambuf( )
{
  setp( buffer, buffer + (bufferSize-1));
        setg( buffer + 4,     // beginning of putback area
              buffer + 4,     // read position
              buffer + 4);    // end position      
        // ASSERT: both input & output capabilities will not be used together
  open_ = 0;
}

zipfilestreambuf::~ zipfilestreambuf( )
{
}

zipfilestreambuf * zipfilestreambuf::open( zipwriter * writer, const char * name )
{
  if (!open_) {
    z = writer;
    f = zipwriter_addfile(z, name, 9);
    open_ = 1;
  }
  return this;
}

void zipfilestreambuf::close( )
{
  if (open_) {
    sync();
    open_ = 0;
  }
}

int zipfilestreambuf::overflow( int c ) // used for output buffer only
{
	if ( !f )
        return EOF;
    if (c != EOF) {
        *pptr() = c;
        pbump(1);
    }
    if ( flush_buffer() == EOF)
        return EOF;
    return c;
}

int zipfilestreambuf::underflow( ) // used for input buffer only
{
}

int zipfilestreambuf::sync( )
{
    if ( pptr() && pptr() > pbase()) {
        if ( flush_buffer() == EOF)
            return -1;
    }
    return 0;
}

int zipfilestreambuf::flush_buffer( )
{
  // Separate the writing of the buffer from overflow() and
  // sync() operation.
  int w = pptr() - pbase();
  
  int block_size = ZW_BUFSIZE;
  int blocks = (w / block_size);

  char *buf = pbase();

  for (int i = 0; i < blocks; i++) {		
    if (zipwriter_write(f, &buf[i*block_size], block_size) == 0)
      return EOF;
  }
  int r = w - (blocks*block_size);
  if (zipwriter_write(f, &buf[blocks*block_size], r) != 0)
      return EOF;

  std::cout << r << blocks << std::endl;

  pbump( -w);
  return w;
}

void zipfilestreambase::close( )
{
  buf.close();
}


zipfilestreambase::zipfilestreambase( )
{
   init( &buf);
}

zipfilestreambase::zipfilestreambase(  zipwriter *z, const char * name )
{
	init( &buf);
	if ( ! buf.open( z, name))
        clear( rdstate() | std::ios::badbit);
}

zipfilestreambase::~ zipfilestreambase( )
{
  buf.close();
}




zipwriterfilestream zipwriterfilestream( zipwriter * z )
{
}

zipwriterfilestream::~ zipwriterfilestream( )
{
}

zipwriterfilestream::zipwriterfilestream( zipwriter * z, const char * name )
	: zipfilestreambase( z, name), std::ostream( &buf)
{}


























