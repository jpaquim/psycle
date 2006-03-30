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
#ifndef DATACOMPRESSION_H
#define DATACOMPRESSION_H

typedef unsigned char byte;

class DataCompression {
  public:

   DataCompression();
   ~DataCompression();
  // compresses.
  static int BEERZ77Comp2(byte * pSource, byte ** pDestination, int size);
  /// decompresses.
  static bool BEERZ77Decomp2(byte * pSourcePos, byte ** pDestination);

  /// squashes.
  static int SoundSquash(signed short * pSource, byte ** pDestination, int size);
  /// desquashes.
  static bool SoundDesquash(byte * pSourcePos, signed short ** pDestination);
};

#endif
