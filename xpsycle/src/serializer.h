/***************************************************************************
 *   Copyright (C) 2006 by Stefan   *
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
#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>


/// \todo add real detection of type size
typedef unsigned char byte;
/// \todo add real detection of type size
typedef unsigned short word;
/// \todo add real detection of type size
typedef unsigned long dword;


class Serializer
{
public:
    Serializer (std::string const & nameFile)
        : _stream (nameFile.c_str (), std::ios_base::out | std::ios_base::binary)
    {
        if (!_stream.is_open ())
            throw "couldn't open file";
    }

    std::size_t GetPos() {
      return  _stream.tellp();
    }

    void Seek(std::size_t pos) {
      _stream.seekp(pos);
    }

    void PutPChar (char* buf, int bytes)
    {
        _stream.write(buf, bytes);
        if (_stream.bad())
            throw "file write failed";
    }

    void PutFloat (float i)
    {
        _stream.write (reinterpret_cast<char *> (&i), sizeof (float));
        if (_stream.bad())
            throw "file write failed";
    }


    void PutInt (int i)
    {
        _stream.write (reinterpret_cast<char *> (&i), sizeof (int));
        if (_stream.bad())
            throw "file write failed";
    }

    void PutLong (long l)
    {
        _stream.write (reinterpret_cast<char *> (&l), sizeof (long));
        if (_stream.bad())
            throw "file write failed";
    }
    void PutDouble (double d)
    {
        _stream.write (reinterpret_cast<char *> (&d), sizeof (double));
        if (_stream.bad())
            throw "file write failed";
    }
    void PutString (std::string const & str)
    {
        int len = str.length ();
        _stream.write (str.data (), len);
        PutBool(0);
        if (_stream.bad())
            throw "file write failed";
    }
    void PutBool (bool b)
    {
        _stream.write (reinterpret_cast<char *> (&b), sizeof (bool));
        if (_stream.bad ())
            throw "file write failed";
    }

    int close() {
      _stream.close();
    }

private:
    std::ofstream _stream;
};


#endif

