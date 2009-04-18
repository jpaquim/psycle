// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

#include <psycle/core/config.private.hpp>
#include "machinefactory.h"

#include "pluginfinder.h"
#include "internalhost.hpp"
#include "nativehost.hpp"
#include "vsthost.h"
#include "ladspahost.hpp"

namespace psy { namespace core {

MachineFactory& MachineFactory::getInstance() {
	static MachineFactory factory;
	return factory;
}

MachineFactory::MachineFactory()
:callbacks_(0)
,finder_(0)
{}

void MachineFactory::Initialize(MachineCallbacks* callbacks)
{
	callbacks_ = callbacks;
	//\todo: probably we need a destructor for this
	finder_= new PluginFinder();
	FillHosts();
}
void MachineFactory::Initialize(MachineCallbacks* callbacks,PluginFinder* finder)
{
	callbacks_ = callbacks;
	finder_ = finder;
	FillHosts();
}
void MachineFactory::Finalize(bool deleteFinder) {
	if (deleteFinder) {
		delete finder_;
	}
}
void MachineFactory::FillHosts()
{
	finder_->Initialize();
	//Please, keep the same order than with the Hosts::type enum. (machinekey.hpp)
	hosts_.push_back( &InternalHost::getInstance(callbacks_) );
	finder_->addHost(Hosts::INTERNAL);
	// InternalHost doesn't have a path, so we call FillFinderData now.
	InternalHost::getInstance(callbacks_).FillFinderData(*finder_);

	hosts_.push_back( &NativeHost::getInstance(callbacks_) );
	finder_->addHost(Hosts::NATIVE);
	
	hosts_.push_back( &LadspaHost::getInstance(callbacks_) );
	finder_->addHost(Hosts::LADSPA);

	#ifdef _WIN32
		hosts_.push_back( &vst::host::getInstance(callbacks_) );
		finder_->addHost(Hosts::VST);
	#endif
}

Machine* MachineFactory::CreateMachine(MachineKey key,Machine::id_type id)
{
	assert(key.host() >= 0 && key.host() < Hosts::NUM_HOSTS);

	// a check for master is done, because we don't add it into the finder,
	// because a user don't create master machines.
	if ( key != MachineKey::master() && !finder_->hasKey(key)) {
		return 0;
	}
	if ( !finder_->info(key).allow() ) {
		return hosts_[Hosts::INTERNAL]->CreateMachine(*finder_,MachineKey::dummy(),id);
	}

	return hosts_[key.host()]->CreateMachine(*finder_,key,id);
#if 0
	for (int i=0; i< hosts_.size(); ++i)
	{
		if ( hosts_[i]->hostCode() == key.host() ) {
			return hosts_[i]->CreateMachine(key,id);
			break;
		}
	}
	return 0;
#endif
}

Machine* MachineFactory::CloneMachine(Machine& mac)
{
	Machine* newmac = CreateMachine(mac.getMachineKey());
	newmac->CloneFrom(mac);
	return newmac;
}
///\FIXME: This only returns the first path, should regenerate the string
std::string const & MachineFactory::getPsyclePath() const { return NativeHost::getInstance(0).getPluginPath(0); }
void MachineFactory::setPsyclePath(std::string path,bool cleardata)
{
	NativeHost::getInstance(0).setPluginPath(path);
	NativeHost::getInstance(0).FillFinderData(*finder_,cleardata);
}

///\FIXME: This only returns the first path, should regenerate the string
std::string const & MachineFactory::getLadspaPath() const { return LadspaHost::getInstance(0).getPluginPath(0); }
void MachineFactory::setLadspaPath(std::string path,bool cleardata)
{
	LadspaHost::getInstance(0).setPluginPath(path);
	LadspaHost::getInstance(0).FillFinderData(*finder_,cleardata);
}

#ifdef _WIN32
	///\FIXME: This only returns the first path, should regenerate the string
	std::string const & MachineFactory::getVstPath() const { return vst::host::getInstance(0).getPluginPath(0); }
	void MachineFactory::setVstPath(std::string path,bool cleardata)
	{
		vst::host::getInstance(0).setPluginPath(path);
		vst::host::getInstance(0).FillFinderData(*finder_,cleardata);
	}
#endif
	
void MachineFactory::RegenerateFinderData()  {
	finder_->Initialize(true);
	for(std::size_t i = 0; i < hosts_.size(); ++i) hosts_[i]->FillFinderData(*finder_, true);
}

}}
