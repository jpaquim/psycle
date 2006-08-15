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
#ifndef ZIPWRITERSTREAM_H
#define ZIPWRITERSTREAM_H

#include <iostream>
#include <fstream>


class zipwriter_file;
class zipwriter;

/**
@author Stefan Nattkemper
*/

class zipfilestreambuf : public std::streambuf {

public :

		zipfilestreambuf();

		zipfilestreambuf* open(  zipwriter *writer, const char* name);

    void close();

		~zipfilestreambuf();

   
    virtual int     overflow( int c = EOF);
    virtual int     underflow();
    virtual int     sync();

private:

		static const int bufferSize = 10*65335 ;    // size of data buff
		char             buffer[bufferSize]; // data buffer

		zipwriter_file *f;
		zipwriter *z;

    int flush_buffer();

    bool open_;
		
};


class zipfilestreambase : virtual public std::ios {
protected:
    zipfilestreambuf buf;

public:

		zipfilestreambase();
		zipfilestreambase(  zipwriter *z, const char* name);
		~zipfilestreambase();

		void close();
   
};

class zipwriterfilestream : public zipfilestreambase, public std::ostream {
public:
		zipwriterfilestream( zipwriter *z, const char* name );

		~zipwriterfilestream();
 

};

#endif
