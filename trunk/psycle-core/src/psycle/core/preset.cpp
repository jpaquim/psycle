// -*- mode:c++; indent-tabs-mode:t -*-
/***************************************************************************
*   Copyright (C) 2007 Psycledelics     *
*   psycle.sf.net   *
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
#include <psycle/core/psycleCorePch.hpp>

#include "preset.h"

#include "binread.h"
#include "machine.h"
#include "plugin.h"

namespace psy {
	namespace core {

		using namespace std;


		Preset::Preset( )     
		{
		}

		Preset::Preset( int numpars, int dataSize ) : 
			params_( numpars ),
			data_( dataSize )
		{
		}

	bool Preset::read( BinRead & prsIn ) 
		{
				// read the preset name     
			char cbuf[32];
			prsIn.read(cbuf,32);
			cbuf[31] = '\0';
			name_ = cbuf;
			if ( prsIn.eof() || prsIn.bad() ) return false;
			// load parameter values
			for(std::size_t i(0); i < params_.size(); ++i) params_[i] = prsIn.readInt4LE();
			if ( prsIn.eof() || prsIn.bad() ) return false;
			// load special machine data
			if ( data_.size() ) {
				prsIn.read( &data_[0], static_cast<std::streamsize>(data_.size()) ); 
				if ( prsIn.eof() || prsIn.bad() ) return false;
			}
			return true;
		}

		const std::string & Preset::name( ) const
		{
			return name_;
		}

		const std::vector<int>& Preset::parameter() const {
			return params_;
		}

		void Preset::tweakMachine( Machine & mac )
		{
			if( mac.type() == MACH_PLUGIN ) {
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
