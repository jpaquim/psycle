// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#include <psycle/detail/project.private.hpp>
#include "plugin_resolver.hpp"
#include <universalis/os/fs.hpp>
namespace psycle { namespace host {

/***************************************************************/
// plugin_resolver

plugin_resolver::plugin_resolver() {
	if(loggers::trace()) {
		std::ostringstream s;
		s << "new plugin resolver";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
}

plugin_resolver::~plugin_resolver() throw() {
	if(loggers::trace()) {
		std::ostringstream s;
		s << "deleting plugin resolver";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	for(map::const_iterator i(map_.begin()) ; i != map_.end() ; ++i) delete i->second;
}

plugin_resolver::instanciator & plugin_resolver::operator[](const std::string & plugin_name) {
	if(loggers::trace()) {
		std::ostringstream s;
		s << "resolving plugin " << plugin_name << " ... ";
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	map::const_iterator i(map_.find(plugin_name));
	if(i != map_.end()) {
		if(loggers::trace()) {
			loggers::trace()("in cache.", UNIVERSALIS__COMPILER__LOCATION);
		}
		return *i->second;
	} else {
		if(loggers::trace()) {
			loggers::trace()("not in cache, resolving library ...", UNIVERSALIS__COMPILER__LOCATION);
		}
		instanciator & instanciator_(*new instanciator(*this, plugin_name));
		map_[plugin_name] = &instanciator_;
		return instanciator_;
	}
}

engine::node & plugin_resolver::operator()(std::string const & plugin_name, engine::node::name_type const & node_name) {
	return (*this)[plugin_name](node_name);
}

/***************************************************************/
// plugin_resolver::instanciator

plugin_resolver::instanciator::instanciator(plugin_resolver & plugin_resolver, const std::string & plugin_name)
try
:
		plugin_library_reference(plugin_name),
		plugin_resolver_(plugin_resolver),
		///\todo the version number is actually libtool's version info
		//library_resolver_(*new universalis::os::dynamic_link::resolver("-" + universalis::os::paths::package::name() + ".plugin." + name, universalis::os::paths::package::version::major_number())),
		library_resolver_(
			#if defined DIVERSALIS__OS__MICROSOFT
				// no prefix
			#elif defined DIVERSALIS__OS__CYGWIN
				"cyg"
			#else
				"lib"
			#endif
			"freepsycle-plugin-" + plugin_name, 0),
		node_instanciator_(library_resolver_.resolve_symbol<node_instanciator>(
			UNIVERSALIS__COMPILER__STRINGIZE(PSYCLE__ENGINE__NODE_INSTANTIATOR__SYMBOL(new))
		))
{
	if(loggers::information()) {
		std::ostringstream s;
		s << "new plugin instanciator for plugin " << plugin_name << ", loaded and resolved library " << full_name();
		loggers::information()(s.str());
	}
}
catch(std::exception const & e) {
	loggers::exception()(e.what(), UNIVERSALIS__COMPILER__LOCATION);
	throw;
}
catch(...) {
	loggers::exception()(universalis::compiler::exceptions::ellipsis_desc(), UNIVERSALIS__COMPILER__LOCATION);
	throw;
}

std::string plugin_resolver::instanciator::name() const throw() {
	return short_name() /* + " (" + full_name() + ")" */;
}

std::string plugin_resolver::instanciator::full_name() const throw() {
	return library_resolver_.path().string();
}

engine::node & plugin_resolver::instanciator::operator()(engine::node::name_type const & name) {
	if(loggers::trace()) {
		std::ostringstream s;
		s << name << ": new plugin instance from loaded library " << this->name();
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	engine::node & result = node_instanciator_(*this, name);
	if(loggers::trace()) {
		std::ostringstream s;
		s << "plugin library reference count: " << this->name() << ", incremented, new count: " << *this;
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	return result;
}

unsigned int plugin_resolver::instanciator::operator--() throw() {
	int const count = reference_counter::operator--();
	if(loggers::trace()) {
		std::ostringstream s;
		s << "plugin library reference count " << this->name() << ", decremented, new count: " << count;
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	if(!count) {
		plugin_resolver_.map_.erase(short_name());
		delete this;
	}
	return count;
}

plugin_resolver::instanciator::~instanciator() throw() {
	if(loggers::information()) {
		std::ostringstream s;
		s << "deleting plugin instanciator, unloading library " << short_name() << ": " << full_name();
		loggers::information()(s.str());
	}
}

}}
