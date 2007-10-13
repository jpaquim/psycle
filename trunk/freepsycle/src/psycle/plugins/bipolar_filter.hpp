// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\interface psycle::plugins::bipolar_filter - an abstract plugin with one input port named "in" and one output port named "out".
#pragma once
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__BIPOLAR_FILTER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace plugins {

	/// an abstract plugin with one input port named "in" and one output port named "out".
	/// This plugin does nothing. It exists so that other plugins can factorize code by
	/// having a class derived from this class and linking to the library of this plugin.
	/// This is mainly intended as an example showing that it is possible to have
	/// an inheritance hierarchy accross libraries to reduce the overall size of the generated code.
	/// the library of a plugin class deriving from this class will be
	/// linked at runtime against the library generated by this plugin,
	/// hence, the code here is not duplicated amongst all library, but rather shared accross them.
	class UNIVERSALIS__COMPILER__DYNAMIC_LINK bipolar_filter : public engine::node {
		protected: friend class virtual_factory_access;
			bipolar_filter(engine::plugin_library_reference &, engine::graph &, std::string const & name, real const & logical_zero);
		protected:
			void channel_change_notification_from_port(engine::port const &) throw(engine::exception) /*override*/;
			void seconds_per_event_change_notification_from_port(engine::port const &) /*override*/;
			void do_process_first() throw(engine::exception) /*override*/;
			void do_process() throw(engine::exception) /*override*/;
		private:
			real logical_zero_;
	};
}}
#include <universalis/compiler/dynamic_link/end.hpp>

