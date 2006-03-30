///\file
///\interface psycle::plugins::outputs::gstreamer
#pragma once
#include <psycle/detail/project.hpp>
#include "../resource.hpp"
#include <gst/gstelement.h>
namespace psycle
{
	namespace plugins
	{
		namespace outputs
		{
			/// outputs to a soundcard device via alsa output implementation.
			class gstreamer : public resource
			{
				public:
					gstreamer(engine::plugin_library_reference &, engine::graph &, const std::string & name) throw(engine::exception);
					virtual ~gstreamer() throw();
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_name(std::string const &);
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES opened() const;
					bool UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES started() const;
				protected:
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_open() throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_start() throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_process() throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_stop() throw(engine::exception);
					void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_close() throw(engine::exception);
				private:
					::GstElement static & instanciate(std::string const & type, std::string const & name);
					::GstElement * pipeline_, * source_, * sink_;
			};
		}
	}
}
