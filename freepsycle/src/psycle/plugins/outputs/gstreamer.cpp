///\file
///\implementation psycle::plugins::outputs::gstreamer
#include <psycle/detail/project.private.hpp>
#include "gstreamer.hpp"
#include <gst/gst.h>
extern "C"
{
	#include <gst/gconf/gconf.h>
}
namespace psycle
{
	namespace plugins
	{
		namespace outputs
		{
			PSYCLE__PLUGINS__NODE_INSTANCIATOR(gstreamer)

			gstreamer::gstreamer(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name) throw(engine::exception)
			:
				resource(plugin_library_reference, graph, name),
				pipeline_(0),
				source_(0),
				sink_(0)
			{
				engine::ports::inputs::single::create(*this, "in");
				engine::ports::inputs::single::create(*this, "amplification", boost::cref(1));
			}

			void gstreamer::do_name(std::string const & name)
			{
				resource::do_name(name);
				if(pipeline_) ::g_object_set(G_OBJECT(pipeline_), "name", (name + "-pipeline").c_str(), 0);
				if(source_) ::g_object_set(G_OBJECT(source_), "name", (name + "-src").c_str(), 0);
				if(sink_) ::g_object_set(G_OBJECT(sink_), "name", (name + "-sink").c_str(), 0);
			}

			::GstElement & gstreamer::instanciate(std::string const & type, std::string const & name)
			{
				::GstElementFactory * factory;
				if(!(factory = ::gst_element_factory_find(type.c_str())))
				{
					std::ostringstream s;
					s << "could not find element type: " << type;
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(loggers::information()())
				{
					std::ostringstream s;
					s
						<< "The element type " << ::gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory))
						<< " is a member of the group " << ::gst_element_factory_get_klass(factory) << "." << std::endl
						<< "Description:" << std::endl
						<< ::gst_element_factory_get_description(factory);
					loggers::information()(s.str());
				}
				::GstElement * element;
				if(!(element = ::gst_element_factory_create(factory, name.c_str())))
				{
					std::ostringstream s;
					s << "found element type: " << ::gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory)) << ", but could not create element instance: " << name;
					throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
				}
				return *element;
			}

			namespace
			{
				void process_gstreamer(::GstElement & object, ::GstBuffer & buffer, ::GstPad & pad, gstreamer & gstreamer)
				{
					::gint16 * data(reinterpret_cast< ::gint16 * >(GST_BUFFER_DATA(&buffer)));
					std::size_t size(GST_BUFFER_SIZE(&buffer));
					for(std::size_t i(0) ; i < size ; ++i)
					{
						static double t(0);
						*data++ = static_cast< ::gint16 >(std::sin(t));
						t = t + 0.0001;
					}
					std::cout << size << " @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
				}
			}
						
			void gstreamer::do_open() throw(engine::exception)
			{
				resource::do_open();
				{
					static bool once = false;
					static boost::mutex mutex;
					boost::mutex::scoped_lock lock(mutex);
					if(!once)
					{
						once = true;
						::gst_init(0, 0);
					}
				}
				if(!(pipeline_ = ::gst_pipeline_new((name() + "-pipeline").c_str()))) throw engine::exceptions::runtime_error("could not create new empty pipeline", UNIVERSALIS__COMPILER__LOCATION);
				try
				{
					source_ = &instanciate("fakesrc", name() + "-src");
					try
					{
						::gst_bin_add(GST_BIN(pipeline_), source_);
					}
					catch(...)
					{
						::gst_object_unref(GST_OBJECT(source_));
						source_ = 0;
						throw;
					}
					try
					{
						if(!(sink_ = ::gst_gconf_get_default_audio_sink())) throw engine::exceptions::runtime_error("could not create gconf default audio sink", UNIVERSALIS__COMPILER__LOCATION);
					}
					catch(...)
					{
						try
						{
							sink_ = &instanciate("alsasink", name() + "-sink");
						}
						catch(...)
						{
							#if 0 // jacksink needs to be put in a jackbin, because it's pulling audio data.
							loggers::exception()("exception: caught ; trying next type ...", UNIVERSALIS__COMPILER__LOCATION);
							try
							{
								sink_ = &instanciate("jacksink", name() + "-sink");
							}
							catch(...)
							#endif
							{
								loggers::exception()("exception: caught ; trying next type ...", UNIVERSALIS__COMPILER__LOCATION);
								try
								{
									sink_ = &instanciate("esdsink", name() + "-sink");
								}
								catch(...)
								{
									loggers::exception()("exception: caught ; trying next type ...", UNIVERSALIS__COMPILER__LOCATION);
									try
									{
										sink_ = &instanciate("artssink", name() + "-sink");
									}
									catch(...)
									{
										loggers::exception()("exception: caught ; trying next type ...", UNIVERSALIS__COMPILER__LOCATION);
										sink_ = &instanciate("osssink", name() + "-sink");
										throw;
									}
								}
							}
						}
					}
					try
					{
						::gst_bin_add(GST_BIN(pipeline_), sink_);
					}
					catch(...)
					{
						::gst_object_unref(GST_OBJECT(sink_));
						sink_ = 0;
						throw;
					}
					//::GstPad * source_pad(::gst_element_get_pad(source_, "src" );
					//::GstPad *   sink_pad(::gst_element_get_pad(  sink_, "sink");
					if(!::gst_element_link_pads(source_, "src", sink_, "sink")) throw engine::exceptions::runtime_error("could not link source and sink", UNIVERSALIS__COMPILER__LOCATION);

					::g_object_set(G_OBJECT(source), "data", /*FAKE_SRC_DATA_SUBBUFFER*/ 2, 0); // data allocation method
					::g_object_set(G_OBJECT(source), "parentsize", 16384, 0);
					::g_object_set(G_OBJECT(source), "sizemax", 8192, 0);
					::g_object_set(G_OBJECT(source), "sizetype", /*FAKE_SRC_SIZETYPE_FIXED*/ 2, 0); // fixed to sizemax

					::g_object_set(G_OBJECT(source), "filltype", /*FAKE_SRC_FILLTYPE_RANDOM*/ 3, 0);
					//::g_object_set(G_OBJECT(source), "filltype", /*FAKE_SRC_FILLTYPE_NOTHING*/ 3, 0);

					::g_object_set(G_OBJECT(source_), "signal-handoffs", true, 0);
					//if(!::g_signal_connect(G_OBJECT(source_), "handoff", G_CALLBACK(process_gstreamer), this)) throw engine::exceptions::runtime_error("could not connect handoff signal", UNIVERSALIS__COMPILER__LOCATION);
				}
				catch(...)
				{
					::gst_object_unref(GST_OBJECT(pipeline_));
					pipeline_ = 0;
					throw;
				}
			}
		
			bool gstreamer::opened() const
			{
				return pipeline_;
			}

			void gstreamer::do_start() throw(engine::exception)
			{
				resource::do_start();
				if(!::gst_element_set_state(pipeline_, ::GST_STATE_PLAYING)) throw engine::exceptions::runtime_error("could not set the whole pipeline state to playing", UNIVERSALIS__COMPILER__LOCATION);
			}
		
			bool gstreamer::started() const
			{
				if(!opened()) return false;
				return ::gst_element_get_state(pipeline_) == ::GST_STATE_PLAYING;
			}
		
			void gstreamer::do_process() throw(engine::exception)
			{
				::gst_bin_iterate(GST_BIN(pipeline_));
			}
			
			void gstreamer::do_stop() throw(engine::exception)
			{
				if(!::gst_element_set_state(pipeline_, ::GST_STATE_READY)) throw engine::exceptions::runtime_error("could not set the whole pipeline state to ready", UNIVERSALIS__COMPILER__LOCATION);
				resource::do_stop();
			}
		
			void gstreamer::do_close() throw(engine::exception)
			{
				if(!::gst_element_set_state(pipeline_, ::GST_STATE_NULL)) throw engine::exceptions::runtime_error("could not set the whole pipeline state to null", UNIVERSALIS__COMPILER__LOCATION);
				::gst_object_unref(GST_OBJECT(pipeline_));
				pipeline_ = 0;
				resource::do_close();
			}
			
			gstreamer::~gstreamer() throw()
			{
				close();
			}
		}
	}
}

#if 0

// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2006 johan boule <bohan@jabber.org>
// copyright 2006 psycle development team http://psycle.sourceforge.net

//////////////////////////////////////////////////////////////////////////////////////////////
//
// simple audio output test with gtreamer
//
// compile with: c++ $(pkg-config --cflags --libs gstreamer-0.10) test-gstreamer-output.cpp
//
//////////////////////////////////////////////////////////////////////////////////////////////

#include <gst/gst.h>
//#include <cstdint>
#include <inttypes.h>
#include <limits>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <ctime>

namespace test
{
	class test
	{
		public:
			test()
			:
				caps_set(false)
			{
				// generate a sine wave in a buffer
				// member data: int16_t audio_buffer[];
				{
					float const amplitude(std::numeric_limits<int16_t>::max());
					float const step(2 * M_PI / sizeof audio_buffer * sizeof *audio_buffer);
					float phase(0); // M_PI / 2
					for(int i(0); i < sizeof audio_buffer / sizeof *audio_buffer; ++i, phase += step)
					{
						audio_buffer[i] = static_cast<int16_t>(amplitude * std::sin(phase));
						//std::cout << std::setw(6) << i << " " << audio_buffer[i] << "\n";
					}
				}

				::gst_init(0, 0);

				::GstElement * pipeline;
				if(!(pipeline = ::gst_pipeline_new("pipeline"))) throw std::runtime_error("could not create new empty pipeline");
				try
				{
					// create a fakesrc
					// member data: ::GstElement * source;
					if(!(source = ::gst_element_factory_make("fakesrc", "source"))) throw std::runtime_error("could not create element instance of type fakesrc");
					try
					{
						::gst_bin_add(GST_BIN(pipeline), source);
					}
					catch(...)
					{
						::gst_object_unref(GST_OBJECT(source));
						throw;
					}

					::g_object_set(G_OBJECT(source), "data", /*FAKE_SRC_DATA_SUBBUFFER*/ 2, 0); // data allocation method
					::g_object_set(G_OBJECT(source), "parentsize", 2 * sizeof audio_buffer, 0);
					::g_object_set(G_OBJECT(source), "sizemax", sizeof audio_buffer, 0);
					::g_object_set(G_OBJECT(source), "sizetype", /*FAKE_SRC_SIZETYPE_FIXED*/ 2, 0); // fixed to sizemax

					//::g_object_set(G_OBJECT(source), "filltype", /*FAKE_SRC_FILLTYPE_RANDOM*/ 3, 0);
					::g_object_set(G_OBJECT(source), "filltype", /*FAKE_SRC_FILLTYPE_NOTHING*/ 3, 0);

					//::g_object_set(G_OBJECT(source), "num-buffers", 100, 0);
					//::g_object_set(G_OBJECT(source), "datarate", 1000, 0);

					::g_object_set(G_OBJECT(source), "signal-handoffs", true, 0);
					if(!::g_signal_connect(G_OBJECT(source), "handoff", G_CALLBACK(handoff_static), this)) throw std::runtime_error("could not connect handoff signal");

					// create a caps filter
					::GstElement * caps_filter;
					if(!(caps_filter = ::gst_element_factory_make("capsfilter", "caps filter"))) throw std::runtime_error("could not create element instance of type capsfilter");
					try
					{
						::gst_bin_add(GST_BIN(pipeline), caps_filter);
					}
					catch(...)
					{
						::gst_object_unref(GST_OBJECT(caps_filter));
						throw;
					}
					// member data: ::GstCaps * caps
					caps = ::gst_caps_new_simple
					(
						"audio/x-raw-int",
						"rate", G_TYPE_INT, 44100,
						"channels", G_TYPE_INT, 2,
						"width", G_TYPE_INT, 16,
						"depth", G_TYPE_INT, 16,
						"signed", G_TYPE_BOOLEAN, true,
						"endianness", G_TYPE_INT, G_BYTE_ORDER,
						0
					);
					try
					{
						::g_object_set(G_OBJECT(caps_filter), "caps", caps, 0);
					}
					catch(...)
					{
						::gst_caps_unref(caps);
						throw;
					}
					//::gst_caps_unref(caps);

					//if(!::gst_element_link_pads(source, "src", caps_filter, "sink")) throw std::runtime_error("could not link source and caps filter");
					if(!::gst_element_link(source, caps_filter)) throw std::runtime_error("could not link source and caps filter");

					// create a default audio sink
					::GstElement * sink;
					if(!(sink = ::gst_element_factory_make("gconfaudiosink", "sink"))) throw std::runtime_error("could not create gconf default audio sink");
					try
					{
						::gst_bin_add(GST_BIN(pipeline), sink);
					}
					catch(...)
					{
						::gst_object_unref(GST_OBJECT(sink));
						throw;
					}

					//if(!::gst_element_link_pads(caps_filter, "src", sink, "sink")) throw std::runtime_error("could not link caps filter and sink");
					if(!::gst_element_link(caps_filter, sink)) throw std::runtime_error("could not link caps filter and sink");

					//::gst_element_link_many (source, caps_filter, sink, 0);

					if(!::gst_element_set_state(pipeline, ::GST_STATE_PLAYING)) throw std::runtime_error("could not set the whole pipeline state to playing");

					//::GMainLoop * const loop(g_main_loop_new(0, false));
					//::g_main_loop_run(loop);
					
					std::cout << "sleeping ... " << std::endl;
					{
						::timespec time = { 5, 0 };
						::nanosleep(&time, 0);
					}

					if(!::gst_element_set_state(pipeline, ::GST_STATE_NULL)) throw std::runtime_error("could not set the whole pipeline state to null");
				}
				catch(...)
				{
					::gst_object_unref(GST_OBJECT(pipeline));
					throw;
				}
				::gst_object_unref(GST_OBJECT(pipeline));
			}

		private:
			int16_t audio_buffer[300];
			::GstElement * source;
			::GstCaps * caps;
			bool caps_set;

			void static handoff_static(::GstElement * source, ::GstBuffer * buffer, GstPad * pad, void * instance)
			{
				reinterpret_cast<test*>(instance)->handoff(*buffer);
			}

			void handoff(::GstBuffer & buffer)
			{
				if(!caps_set)
				{
					::gst_buffer_set_caps(&buffer, caps);
					caps_set = true;
				}
				std::size_t size(GST_BUFFER_SIZE(&buffer));
				//std::cout << "handoff: buffer size: " << size << "\n";
				void * address(GST_BUFFER_DATA(&buffer));
				std::memcpy(address, audio_buffer, sizeof audio_buffer);
			}
	};
}

int main()
{
	std::cout << "entering main" << std::endl;
	test::test test;
	std::cout << "leaving main" << std::endl;
}

#endif
