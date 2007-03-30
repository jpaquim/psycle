/***************************************************************************
*   Copyright (C) 2007 by  Stefan Nattkemper  *
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
#include "samplefilefilter.h"
#include "sample.h"
#include <iostream>

namespace psy {
  namespace core {

    SampleFileFilter::SampleFileFilter( Sample& sample ) 
      : sample_(sample)
    {
    }

    SampleFileFilter::~SampleFileFilter()
    {
    }

    Sample& SampleFileFilter::sample() {
      return sample_;
    }

    bool SampleFileFilter::read( BinRead& in ) {
      std::cout << "no default load filter" << std::endl;
      return true;
    }

  }
}
