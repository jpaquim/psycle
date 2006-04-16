// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright � 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\interface psycle::host::plugin_resolver
#pragma once
#include <psycle/engine/engine.hpp>
#include <universalis/operating_system/dynamic_link/resolver.hpp>
#include <cassert>
#include <string>
#include <iostream>
#include <map>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__HOST__PLUGIN_RESOLVER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace host
	{
		/// manages plugins.
		/// resolves libraries, handles their loading and unloading,
		/// instanciates nodes simply from their plugin names.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK plugin_resolver
		{
			private:
				class instanciator : protected engine::plugin_library_reference
				{
					friend class plugin_resolver;
					private:
						class instance
						{
							public:
								instance(instanciator &, engine::graph &, std::string const & name) throw(engine::exception);
								virtual ~instance() throw();
								inline operator engine::node const & () const throw() { return *node_; }
								inline operator engine::node       & ()       throw() { return *node_; }
								inline engine::node const & node() const throw() { return *node_; }
								inline engine::node       & node()       throw() { return *node_; }
							private:
								engine::node * const node_;
						};
					public:
						instanciator(plugin_resolver &, std::string const & name) throw(engine::exception);
						virtual ~instanciator() throw();
						instance & operator()(engine::graph &, std::string const & name);
						std::string                      name() const throw();
						std::string                 full_name() const throw();
						std::string const inline & short_name() const throw() { return plugin_library_reference::name(); }
					protected:
						virtual instanciator & operator--() throw();
					private:
						plugin_resolver & plugin_resolver_;
						universalis::operating_system::dynamic_link::resolver const library_resolver_;
						typedef engine::node & (* const node_instanciator) (engine::reference_counter &, engine::graph &, std::string const & name);
						node_instanciator const node_instanciator_;
				};
				instanciator & operator[](std::string const & plugin_name) throw(engine::exception);
				typedef std::map<std::string, instanciator*> map; map map_;
			public:
				/// creates a new plugin resolver.
				plugin_resolver();
				/// deletes the plugin resolver.
				/// to be able to delete the resolver correctly,
				/// there must be no more node instances of any of the plugins,
				/// they must all have been deleted previous to this very object deletion.
				virtual ~plugin_resolver() throw();
				/// instanciates a node whose contained in the library plugin_name.
				/// the loading (and unloading) of the library is transparent.
				instanciator::instance & operator()(std::string const & plugin_name, engine::graph &, std::string const & node_name) throw(engine::exception);
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
