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
				new engine::ports::inputs::single(*this, "in");
				new engine::ports::inputs::single(*this, "amplification", 1);
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
				::GstElementFactory * factory(0);
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
				::GstElement * element(0);
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
					::g_object_set(G_OBJECT(source_), "signal-handoffs", true, 0);
					::g_object_set(G_OBJECT(source_), "data", 2, 0); // subbuffers
					::g_object_set(G_OBJECT(source_), "sizetype", 2, 0); // fixed to sizemax
					::g_object_set(G_OBJECT(source_), "sizemax", 8192, 0);
					::g_object_set(G_OBJECT(source_), "parentsize", 16384, 0);
					::g_object_set(G_OBJECT(source_), "filltype", 3, 0); // random
//					if(!::g_signal_connect(G_OBJECT(source_), "handoff", G_CALLBACK(process_gstreamer), this)) throw engine::exceptions::runtime_error(UNIVERSALIS__COMPILER__LOCATION, "could not connect handoff signal");
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
