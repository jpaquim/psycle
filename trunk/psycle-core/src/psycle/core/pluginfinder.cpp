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
#include "psycleCorePch.hpp"

#include "pluginfinder.h"

#include "file.h"
#include "ladspamachine.h"
#include "plugin.h"

namespace psy
{
	namespace core
	{
		PluginInfo::PluginInfo()
		:
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




		PluginFinder::PluginFinder(std::string const & psycle_path, std::string const & ladspa_path)
		:
			psycle_path_(psycle_path),
			ladspa_path_(ladspa_path)
		{
			scanAll();
		}

		PluginFinder::~PluginFinder()
		{
		}
		
		std::map< PluginFinderKey, PluginInfo >::const_iterator PluginFinder::begin() const {
			return map_.begin();
		}         
				
		std::map< PluginFinderKey, PluginInfo >::const_iterator PluginFinder::end() const {
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
			scanInternal();
			scanNatives();
			scanLadspa();
		}

		void PluginFinder::scanInternal() {
			PluginFinderKey key = PluginFinderKey::internalSampler();
			PluginInfo info;
			info.setType( MACH_SAMPLER );
			info.setName( key.name() );
			map_[key]=info;
		}

		void PluginFinder::scanLadspa() {
			std::string ladspa_path = ladspa_path_;
			///\todo this just uses the first path in getenv
			#if defined __unix__ || defined __APPLE__
				std::string::size_type dotpos = ladspa_path.find(':',0);
				if ( dotpos != ladspa_path.npos ) ladspa_path = ladspa_path.substr( 0, dotpos );
			#else
			#endif
			const LADSPA_Descriptor * psDescriptor;
			LADSPA_Descriptor_Function pfDescriptorFunction;
			unsigned long lPluginIndex;

			std::vector<std::string> fileList;
			fileList = File::fileList(ladspa_path, File::list_modes::files);

			std::vector<std::string>::iterator it = fileList.begin();
			for ( ; it < fileList.end(); ++it ) {
				std::string fileName = *it;
				#if defined __unix__ || defined __APPLE__
					// problem of so.0.0.x .. .so all three times todo
				#else
					if ( fileName.find( ".dll" ) == std::string::npos ) continue;
				#endif

				class DummyCallbacks : public MachineCallbacks {
					private:
						PlayerTimeInfo ti;
					public:
						const PlayerTimeInfo& timeInfo() const { return ti; }
						bool autoStopMachines() const { return false; }
				} dummycallbacks;

				LADSPAMachine plugin(&dummycallbacks, 0, 0 );
				pfDescriptorFunction = plugin.loadDescriptorFunction( ladspa_path + File::slash() + fileName );

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
						PluginFinderKey key(fileName, ladspa_path + File::slash() + fileName, lPluginIndex );
						map_[key] = info;
					}
				}
			}
		}

		void PluginFinder::scanNatives() {
			std::vector<std::string> fileList;
			fileList = File::fileList(psycle_path_, File::list_modes::files);

			std::vector<std::string>::iterator it = fileList.begin();

			for ( ; it < fileList.end(); ++it ) {
				std::string fileName = *it;
				#if defined __unix__ || defined __APPLE__
					///\todo problem of so.x.y.z .. .so all three times todo
				#else
					if ( fileName.find( ".dll" ) == std::string::npos ) continue;
				#endif

				class DummyCallbacks : public MachineCallbacks {
					private:
						PlayerTimeInfo ti;
					public:
						const PlayerTimeInfo& timeInfo() const { return ti; }
						bool autoStopMachines() const { return false; }
				} dummycallbacks;

				Plugin plugin(&dummycallbacks, 0, 0 );
				if ( plugin.LoadDll( psycle_path_, fileName ) ) {
					PluginInfo info;
					info.setType( MACH_PLUGIN );
					info.setName( plugin.GetName() );
					info.setMode( plugin.mode() );
					info.setLibName( plugin.GetDllName() );
					std::ostringstream o;
					std::string version;
					if (!(o << plugin.GetInfo().Version )) version = o.str();
					info.setVersion( version );
					info.setAuthor( plugin.GetInfo().Author );
					///\todo .. path should here stored and not evaluated in plugin
					PluginFinderKey key( plugin.GetDllName(), fileName );
					map_[key] = info;               
				}
			}
		}
	}
}
