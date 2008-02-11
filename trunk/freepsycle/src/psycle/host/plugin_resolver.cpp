// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\implementation psycle::host::plugin_resolver
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include <psycle/detail/project.private.hpp>
#include "plugin_resolver.hpp"
#include <universalis/operating_system/paths.hpp>
#include <universalis/compiler/typenameof.hpp>
#include <universalis/compiler/exceptions/ellipsis.hpp>
namespace psycle { namespace host {

	/***************************************************************/
	// plugin_resolver

	plugin_resolver::plugin_resolver()
	{
		if(loggers::trace()())
		{
			std::ostringstream s;
			s << "new plugin resolver";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
	}

	plugin_resolver::~plugin_resolver() throw()
	{
		if(loggers::trace()())
		{
			std::ostringstream s;
			s << "delete plugin resolver";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		for(map::const_iterator i(map_.begin()) ; i != map_.end() ; ++i) delete i->second;
	}

	plugin_resolver::instanciator & plugin_resolver::operator[](const std::string & name) throw(engine::exception)
	{
		if(loggers::trace()())
		{
			std::ostringstream s;
			s << "resolving plugin " << name << " ... ";
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		map::const_iterator i(map_.find(name));
		if(i != map_.end())
		{
			if(loggers::trace()())
			{
				loggers::trace()("in cache.", UNIVERSALIS__COMPILER__LOCATION);
			}
			return *i->second;
		}
		else
		{
			if(loggers::trace()())
			{
				loggers::trace()("not in cache, resolving library ...", UNIVERSALIS__COMPILER__LOCATION);
			}
			instanciator & instanciator_(*new instanciator(*this, name));
			map_[name] = &instanciator_;
			return instanciator_;
		}
	}

	plugin_resolver::instanciator::instance & plugin_resolver::operator()(const std::string & plugin_name, engine::graph & graph, const std::string & node_name) throw(engine::exception)
	{
		return (*this)[plugin_name](graph, node_name);
	}

	/***************************************************************/
	// plugin_resolver::instanciator

	plugin_resolver::instanciator::instanciator(plugin_resolver & plugin_resolver, const std::string & name) throw(engine::exception)
	try
	:
			plugin_library_reference(name),
			plugin_resolver_(plugin_resolver),
			///\todo the version number is actually libtool's version info
			//library_resolver_(*new universalis::operating_system::dynamic_link::resolver("-" + universalis::operating_system::paths::package::name() + ".plugin." + name, universalis::operating_system::paths::package::version::major_number())),
			library_resolver_(*new universalis::operating_system::dynamic_link::resolver(
				#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
					// currently no prefix, when built with scons at least
				#else
					"lib"
				#endif
				"freepsycle-plugin-" + name, 0)),
			node_instanciator_(library_resolver_.resolve_symbol<node_instanciator>(UNIVERSALIS__COMPILER__STRINGIZED(PSYCLE__ENGINE__NODE_INSTANTIATOR__SYMBOL(new))))
	{
		if(loggers::information()())
		{
			std::ostringstream s;
			s << "new plugin instanciator for plugin " << name << ", loaded and resolved library " << this->full_name();
			loggers::information()(s.str());
		}
	}
	catch(std::exception const & e)
	{
		loggers::exception()(e.what(), UNIVERSALIS__COMPILER__LOCATION);
		throw;
	}
	catch(...)
	{
		loggers::exception()(universalis::compiler::exceptions::ellipsis(), UNIVERSALIS__COMPILER__LOCATION);
		throw;
	}

	std::string plugin_resolver::instanciator::name() const throw()
	{
		return short_name() /* + " (" + full_name() + ")" */;
	}

	std::string plugin_resolver::instanciator::full_name() const throw()
	{
		return library_resolver_.path().string();
	}

	plugin_resolver::instanciator::instance & plugin_resolver::instanciator::operator()(engine::graph & graph, const std::string & name)
	{
		if(loggers::trace()())
		{
			std::ostringstream s;
			s << graph.qualified_name() << '.' << name << " new plugin instance from loaded library " << this->name();
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		instance & result(*new instance(*this, graph, name));
		if(loggers::trace()())
		{
			std::ostringstream s;
			s << "plugin library reference count: " << this->name() << ", incremented, new count: " << *this;
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		return result;
	}

	unsigned int plugin_resolver::instanciator::operator--() throw()
	{
		int const count(reference_counter::operator--());
		if(loggers::trace()())
		{
			std::ostringstream s;
			s << "plugin library reference count " << this->name() << ", decremented, new count: " << count;
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		if(!count)
		{
			plugin_resolver_.map_.erase(short_name());
			delete this;
		}
		return count;
	}

	plugin_resolver::instanciator::~instanciator() throw()
	{
		if(loggers::information()())
		{
			std::ostringstream s;
			s << "delete plugin instanciator, unloading library " << short_name() << ": " << full_name();
			loggers::information()(s.str());
		}
		delete &library_resolver_;
	}

	/***************************************************************/
	// plugin_resolver::instanciator::instance

	plugin_resolver::instanciator::instance::instance(plugin_resolver::instanciator & instanciator, engine::graph & graph, const std::string & name) throw(engine::exception)
	:
		node_(&instanciator.node_instanciator_(instanciator, graph, name))
	{
		if(loggers::information()())
		{
			std::ostringstream s;
			s << node().qualified_name() << " new node instance of " << universalis::compiler::typenameof(node()) << " from loaded library " << instanciator.name();
			loggers::information()(s.str());
		}
		++node().plugin_library_reference();
	}

	plugin_resolver::instanciator::instance::~instance() throw()
	{
		if(loggers::information()())
		{
			std::ostringstream s;
			s << node().qualified_name() << " delete node instance of " << universalis::compiler::typenameof(node()) << " from loaded library " << node().plugin_library_reference().name();
			loggers::information()(s.str());
		}
		engine::reference_counter & reference_counter(node().plugin_library_reference());
		node().free_heap();
		--reference_counter;
	}
}}
