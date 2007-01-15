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
#include "preset.h"
#include "machine.h"
#include "plugin.h"
#include "fileio.h"

namespace psycle {
  namespace host {

    using namespace std;


    Preset::Preset( )     
    {
    }

    Preset::Preset( int numpars, int dataSize ) : 
      params_( numpars ),
      data_( dataSize )
    {
    }

    bool Preset::loadFromFile( RiffFile & f )
    {
       // read the preset name
      char cbuf[32];
      if (!f.ReadChunk(cbuf,sizeof(cbuf))) 
        return 0;
      name_ = string(cbuf);

      // load parameter values
      for ( std::vector<int>::iterator it = params_.begin(); it < params_.end(); it++ ) {
        int & paramValue = *it;
        if ( !f.Read( paramValue ) ) 
          return 0;
      }

      // load special machine data
      if ( data_.size() ) {
        if (!f.ReadChunk( &data_[0], data_.size() ) ) 
          return 0;
      }
      
      return true;
    }

    const std::string & Preset::name( ) const
    {
      return name_;
    }

    void Preset::tweakMachine( Machine & mac )
    {
      if( mac._type == MACH_PLUGIN ) {
        int i = 0;
        for (std::vector<int>::iterator it = params_.begin(); it < params_.end(); it++) {
          try {
            reinterpret_cast<Plugin&>(mac).proxy().ParameterTweak(i, *it);
          }
          catch(const std::exception &) {
          }
          catch(...) {
          }
          i++;
        }

        try {
          if ( data_.size() > 0 ) 
            reinterpret_cast<Plugin&>(mac).proxy().PutData( &data_[0]); // Internal save
        }
        catch(const std::exception &) {
        }
        catch(...) {
        }
      }
    }

  }
}
