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
#include "pluginfinder.h"
#include "ladspamachine.h"
#include "plugin.h"
#include "configuration.h"
#include "global.h"
#include <ngrs/nfile.h>

namespace psycle
{
	namespace host
	{


		PluginInfo::PluginInfo() :
			subclass_( MACH_UNDEFINED ),
			mode_( MACHMODE_FX ),
			fileTime_( 0 ),
			allow_( 1 )
		{

		}

		PluginInfo::~PluginInfo() {

		}

		void PluginInfo::setType( Machine::type_type type ) {
			subclass_ = type;
		}

		Machine::type_type PluginInfo::type() const {
			return subclass_;
		}

		void PluginInfo::setMode( Machine::mode_type mode ) {
			mode_ = mode;
		}

		Machine::mode_type PluginInfo::mode() const {
			return mode_;
		}

		void PluginInfo::setName( const std::string & name ) {
			name_ = name;
		}

		const std::string & PluginInfo::name() const {
			return name_;
		}

		void PluginInfo::setAuthor( const std::string & author ) {
			author_ = author;
		}

		const std::string & PluginInfo::author() const {
			return author_;
		}


		void PluginInfo::setDesc( const std::string & desc ) {
			desc_ = desc;
		}

		const std::string & PluginInfo::desc() const {
			return desc_;
		}

		void PluginInfo::setVersion( const std::string & version ) {
			version_ = version;
		}

		const std::string & PluginInfo::version() const {
			return version_;
		}

		void PluginInfo::setLibName( const std::string & libName ) {
			libName_ = libName;
		}

		const std::string & PluginInfo::libName() const {
			return libName_;
		}

		void PluginInfo::setFileTime( time_t time ) {
			fileTime_ = time;
		}

		time_t PluginInfo::fileTime() const {
			return fileTime_;
		}

		void PluginInfo::setError( const std::string & error ) {
			error_ = error;
		}

		const std::string PluginInfo::error() const {
			return error_;
		}

		void PluginInfo::setAllow( bool allow ) {
			allow_ = allow;
		}

		bool PluginInfo::allow() const {
			return allow_;
		}

		void PluginInfo::setCategory( const std::string & category ) {
			category_ = category;
		}

		const std::string & PluginInfo::category() const {
			return category_;
		}


		PluginFinderKey::PluginFinderKey( ) : index_(0) {

		}

		PluginFinderKey::PluginFinderKey( const std::string & name, int index ) :
			name_( name ),
			index_( index) 
		{
		}

		PluginFinderKey::~PluginFinderKey() {
		}

		bool PluginFinderKey::operator<(const PluginFinderKey & key) const {
			if ( name() != key.name() ) return name() < key.name();
			return index() < key.index();	
		}		


		const std::string & PluginFinderKey::name() const {
			return name_;
		}

		int PluginFinderKey::index() const {
			return index_;
		}




		PluginFinder::PluginFinder()
		{
		}


		PluginFinder::~PluginFinder()
		{
		}
		
        std::map< PluginFinderKey, PluginInfo >::const_iterator PluginFinder::begin() {
          return map_.begin();
        }         
        
        std::map< PluginFinderKey, PluginInfo >::const_iterator PluginFinder::end() {
          return map_.end();
        }         
        
		PluginInfo PluginFinder::info( const PluginFinderKey & key ) const {
			std::map< PluginFinderKey, PluginInfo >::const_iterator it = map_.find( key );
			if ( it != map_.end() ) 
				return it->second;
			else
				return PluginInfo();
		}

		void PluginFinder::scanAll() {
			scanNatives();
			scanLadspa();		
		}

		void PluginFinder::scanLadspa() {
#ifdef __unix__             
#if !defined XPSYCLE__NO_LADSPA
			///\todo this just uses the first path in getenv
			const char* pcLADSPAPath = std::getenv("LADSPA_PATH");
			std::cout << "scanning:" << pcLADSPAPath << std::endl;
			if ( !pcLADSPAPath) pcLADSPAPath = "/usr/lib/ladspa/";
			std::string ladspa_path(pcLADSPAPath);
			int dotpos = ladspa_path.find(':',0);
			if (dotpos != ladspa_path.npos) ladspa_path = ladspa_path.substr(0,dotpos);
			

			const LADSPA_Descriptor * psDescriptor;
			LADSPA_Descriptor_Function pfDescriptorFunction;
			unsigned long lPluginIndex;

			std::vector<std::string> fileList;
			fileList = NFile::fileList( ladspa_path );

			std::vector<std::string>::iterator it = fileList.begin();
			for ( ; it < fileList.end(); it++ ) {
				std::string fileName = *it;
				LADSPAMachine plugin(0, 0 );
				pfDescriptorFunction = plugin.loadDescriptorFunction( ladspa_path+ "/" +fileName);
				if (pfDescriptorFunction) {
					for (lPluginIndex = 0;; lPluginIndex++) {
						psDescriptor = pfDescriptorFunction(lPluginIndex);
						if (psDescriptor == NULL) {
							break;
						}
						PluginInfo info;
						info.setType( MACH_LADSPA );
						info.setName( psDescriptor->Name );
						info.setLibName( fileName );
						PluginFinderKey key(fileName, lPluginIndex );
						map_[key] = info;
					}
				}
			}
#endif // XPSYCLE__NO_LADSPA
#endif
		}

		void PluginFinder::scanNatives() {

			std::string psycle_path = Global::pConfig()->pluginPath;

			std::vector<std::string> fileList;
			fileList = NFile::fileList( psycle_path );

			std::vector<std::string>::iterator it = fileList.begin();

			for ( ; it < fileList.end(); it++ ) {
				std::string fileName = *it;
				Plugin plugin(0, 0 );
				if ( plugin.LoadDll( fileName ) ) {
                   PluginInfo info;
                   info.setType( MACH_PLUGIN );
                   info.setName( plugin.GetName() );
                   info.setMode( plugin.mode() );
                   info.setLibName( plugin.GetDllName() );
                   std::ostringstream o;
                   std::string version;
			       if (!(o << plugin.GetInfo()->Version )) version = o.str();
                   info.setVersion( version );
                   info.setAuthor( plugin.GetInfo()->Author );				
                   PluginFinderKey key( plugin.GetDllName(), 0 );
                   map_[key] = info;               
				}
			}
		}
	}
}
