// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2006 johan boule <bohan@jabber.org>
// copyright 2004-2006 psycle development team http://psycle.sourceforge.net

///\implementation psycle::plugins::outputs::gstreamer
#include <psycle/detail/project.private.hpp>
#include "gstreamer.hpp"
#include <gst/gst.h>
#include <universalis/exception.hpp>
#include <cstddef>
#include <limits>
#include <cmath>
#include <cstdint>
namespace psycle
{
	namespace plugins
	{
		namespace outputs
		{
			///\todo hardcoded for now
			namespace
			{
				unsigned int const samples_per_second(44100);
				unsigned char const channels(1);
				unsigned char const bits_per_sample(16);
				bool const samples_signed(true);
				typedef std::int16_t output_sample_type;
				std::size_t const bytes_per_buffer(16384);
				unsigned int const samples_per_buffer(bytes_per_buffer / sizeof(output_sample_type));
				unsigned int const buffers(3);
			}

			PSYCLE__PLUGINS__NODE_INSTANCIATOR(gstreamer)

			gstreamer::gstreamer(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name) throw(engine::exception)
			:
				resource(plugin_library_reference, graph, name),
				pipeline_(),
				source_(),
				sink_(),
				caps_(),
				caps_set_(),
				buffer_(),
				current_read_buffer_(), current_write_buffer_()
			{
				engine::ports::inputs::single::create(*this, "in");
				engine::ports::inputs::single::create(*this, "amplification", boost::cref(1));
			}

			void gstreamer::do_name(std::string const & name)
			{
				resource::do_name(name);
				if(pipeline_) ::gst_element_set_name(pipeline_, (name + "-pipeline").c_str());
				if(source_  ) ::gst_element_set_name(source_  , (name + "-src"     ).c_str());
				if(sink_    ) ::gst_element_set_name(sink_    , (name + "-sink"    ).c_str());
			}

			namespace
			{
				::GstElement & instanciate(std::string const & type, std::string const & name) throw(universalis::exception)
				{
					try
					{
						if(loggers::information()())
						{
							std::ostringstream s; s << "instanciating " << type << " " << name;
							loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
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
							loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
						::GstElement * element;
						if(!(element = ::gst_element_factory_create(factory, name.c_str())))
						{
							std::ostringstream s;
							s << "found element type: " << ::gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory)) << ", but could not create element instance: " << name;
							throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
						return *element;
					}
					UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW__NO_CLASS
					return *static_cast< ::GstElement* >(0); // dummy return to avoid warning
				}

				bool wait_for_state(::GstElement & element, ::GstState state_wanted, ::GstClockTime timeout_nanoseconds = static_cast<GstClockTime>(60e9)) throw(universalis::exception)
				{
					::GstState current_state, pending_state;
					::GstClockTime intermediate_timeout_nanoseconds(static_cast<GstClockTime>(0.1e9));
					GstStateChangeReturn result(::gst_element_get_state(&element, &current_state, &pending_state, intermediate_timeout_nanoseconds));
					switch(result)
					{
						case ::GST_STATE_CHANGE_NO_PREROLL:
							universalis::operating_system::loggers::information()("no preroll", UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						case ::GST_STATE_CHANGE_SUCCESS:
							return current_state == state_wanted;
						case ::GST_STATE_CHANGE_ASYNC:
							if(pending_state != state_wanted) return false;
							else
							{
								::GstClockTime timeout_total_nanoseconds(intermediate_timeout_nanoseconds);
								intermediate_timeout_nanoseconds += static_cast<GstClockTime>(0.5e9);
								for(;;)
								{
									{
										std::ostringstream s; s << "waiting for element " << ::gst_element_get_name(&element) << " state to asynchronously change from " << ::gst_element_state_get_name(current_state) << " to " << ::gst_element_state_get_name(state_wanted) << " (waited a total of " << timeout_total_nanoseconds * 1e-9 << " seconds ; will timeout after " << timeout_nanoseconds * 1e-9 << " seconds)";
										universalis::operating_system::loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
									}
									{
										result = ::gst_element_get_state(&element, &current_state, &pending_state, intermediate_timeout_nanoseconds);
										switch(result)
										{
											case ::GST_STATE_CHANGE_ASYNC:
												if(pending_state != state_wanted) return false;
												else break;
											case ::GST_STATE_CHANGE_NO_PREROLL:
												universalis::operating_system::loggers::information()("no preroll", UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
											case ::GST_STATE_CHANGE_SUCCESS:
												return current_state == state_wanted;
											case ::GST_STATE_CHANGE_FAILURE:
												{
													std::ostringstream s;
														s
															<< "could not get element " << ::gst_element_get_name(&element) << " state: "
															<< "state change: " << result << ", "
															<< "current state: " << ::gst_element_state_get_name(current_state) << ", "
															<< "pending state: " << ::gst_element_state_get_name(pending_state);
													throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
												}
											default: goto unkown;
										}
									}
									timeout_total_nanoseconds += intermediate_timeout_nanoseconds;
									if(timeout_total_nanoseconds > timeout_nanoseconds) break;
									intermediate_timeout_nanoseconds *= 2;
								}
							}
						case ::GST_STATE_CHANGE_FAILURE:
							return false;
						default:
						unkown:
							{
								std::ostringstream s;
									s
										<< "element " << ::gst_element_get_name(&element) << " "
										<< "did an unknown state change: " << result << ", "
										<< "current state: " << current_state << ", "
										<< "pending state: " << pending_state;
								throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
							}
					}
				}
			}

			void gstreamer::do_open() throw(engine::exception)
			{
				resource::do_open();

				// initialize gstreamer
				{
					static bool once = false;
					static boost::mutex mutex;
					boost::mutex::scoped_lock lock(mutex);
					if(!once)
					{
						once = true;
						int * argument_count(0);
						char *** arguments = 0;
						::GError * error(0);
						if(!::gst_init_check(argument_count, arguments, &error))
						{
							std::ostringstream s; s << "could not initialize gstreamer: " << error->code << ": " << error->message;
							::g_clear_error(&error);
							throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
					}
				}

				// create a pipeline
				if(!(pipeline_ = ::gst_pipeline_new((name() + "-pipeline").c_str()))) throw engine::exceptions::runtime_error("could not create new empty pipeline", UNIVERSALIS__COMPILER__LOCATION);
				try
				{
					// pad capabilities, audio format
					caps_ = ::gst_caps_new_simple
					(
						"audio/x-raw-int",
						"rate"      , G_TYPE_INT    , samples_per_second,
						"channels"  , G_TYPE_INT    , channels,
						"width"     , G_TYPE_INT    , bits_per_sample,
						"depth"     , G_TYPE_INT    , bits_per_sample,
						"signed"    , G_TYPE_BOOLEAN, samples_signed,
						"endianness", G_TYPE_INT    , G_BYTE_ORDER,
						0
					);
					try
					{
						// create a fakesrc
						source_ = &instanciate("fakesrc", name() + "-src");
						try
						{
							if(!::gst_bin_add(GST_BIN(pipeline_), source_)) throw engine::exceptions::runtime_error("could not add source element to pipeline", UNIVERSALIS__COMPILER__LOCATION);
						}
						catch(...)
						{
							::gst_object_unref(GST_OBJECT(source_)); source_ = 0;
							throw;
						}

						// set properties of the fakesrc element
						::g_object_set
						(
							G_OBJECT(source_),
							"data", /*FAKE_SRC_DATA_SUBBUFFER*/ 2, // data allocation method
							"parentsize", bytes_per_buffer * buffers,
							"sizemax", bytes_per_buffer,
							"sizetype", /*FAKE_SRC_SIZETYPE_FIXED*/ 2, // fixed to sizemax
							//"filltype", /*FAKE_SRC_FILLTYPE_RANDOM*/ 3,
							"filltype", /*FAKE_SRC_FILLTYPE_NOTHING*/ 1,
							//"num-buffers", 0,
							//"datarate", 0,
							"signal-handoffs", true,
							0
						);

						buffer_ = new std::int8_t[bytes_per_buffer * buffers];

						// register our callback to the handoff signal of the fakesrc element
						if(!::g_signal_connect(G_OBJECT(source_), "handoff", G_CALLBACK(handoff_static), this)) throw engine::exceptions::runtime_error("could not connect handoff signal", UNIVERSALIS__COMPILER__LOCATION);

						// get the source element's src pad
						::GstPad * source_pad(::gst_element_get_pad(source_, "src" ));
						if(!source_pad) throw engine::exceptions::runtime_error("no src pad found on source element", UNIVERSALIS__COMPILER__LOCATION);
						try
						{
							// set the caps of the source element's src pad
							if(!::gst_pad_set_caps(source_pad, caps_)) throw engine::exceptions::runtime_error("could not set capabilities on source element's src pad", UNIVERSALIS__COMPILER__LOCATION);
							::gst_pad_use_fixed_caps(source_pad);

							// create an audio sink
							try
							{
								sink_ = &instanciate("gconfaudiosink", name() + "-sink");
							}
							catch(std::exception const & e)
							{
								// maybe the user didn't configure his default audio-sink, falling back to several possibilities
								#if 0 // jacksink needs to be put in a jackbin, because it's pulling audio data.
								if(loggers:information())
								{
									std::ostringstream s;
									s
										<< "exception: caught while trying to instanciate audio sink: " <<
									loggers::information()("exception: caught while trying to instanciate audio sink ; trying next type ...", UNIVERSALIS__COMPILER__LOCATION);
								try
								{
									sink_ = &instanciate("jacksink", name() + "-sink");
								}
								catch(...)
								#endif
								{
									loggers::information()("exception: caught while trying to instanciate audio sink ; trying next type ...", UNIVERSALIS__COMPILER__LOCATION);
									try
									{
										sink_ = &instanciate("alsasink", name() + "-sink");
									}
									catch(...)
									{
										loggers::information()("exception: caught while trying to instanciate audio sink ; trying next type ...", UNIVERSALIS__COMPILER__LOCATION);
										try
										{
											sink_ = &instanciate("esdsink", name() + "-sink");
										}
										catch(...)
										{
											loggers::information()("exception: caught while trying to instanciate audio sink ; trying next type ...", UNIVERSALIS__COMPILER__LOCATION);
											try
											{
												sink_ = &instanciate("osssink", name() + "-sink");
											}
											catch(...)
											{
												loggers::information()("exception: caught while trying to instanciate audio sink ; trying next type ...", UNIVERSALIS__COMPILER__LOCATION);
												try
												{
													sink_ = &instanciate("artssink", name() + "-sink");
												}
												catch(...)
												{
													loggers::information()("exception: caught while trying to instanciate audio sink ; no more type to try ; bailing out.", UNIVERSALIS__COMPILER__LOCATION);
													throw;
												}
											}
										}
									}
								}
							}
							try
							{
								if(!::gst_bin_add(GST_BIN(pipeline_), sink_)) throw engine::exceptions::runtime_error("could not add sink element to pipeline", UNIVERSALIS__COMPILER__LOCATION);
							}
							catch(...)
							{
								::gst_object_unref(GST_OBJECT(sink_)); sink_ = 0;
								throw;
							}

							// get the sink element's sink pad
							::GstPad * sink_pad(::gst_element_get_pad(sink_, "sink"));
							if(!sink_pad) throw engine::exceptions::runtime_error("no sink pad found on sink element", UNIVERSALIS__COMPILER__LOCATION);
							try
							{
								// set the caps of the sink element's sink pad
								if(!::gst_pad_set_caps(sink_pad, caps_)) throw engine::exceptions::runtime_error("could not set capabilities on sink element's sink pad", UNIVERSALIS__COMPILER__LOCATION);

								// connect the source element's src pad to the sink element's sink pad
								{
									//::GstPadLinkReturn const result(::gst_pad_link_filtered(source_pad, sink_pad, caps_));
									::GstPadLinkReturn const result(::gst_pad_link(source_pad, sink_pad));
									if(GST_PAD_LINK_FAILED(result))
									{
										std::ostringstream s; s << "could not link source and sink: code: " << result;
										throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
									}
								}
							}
							catch(...)
							{
								::gst_object_unref(GST_OBJECT(sink_pad));
								throw;
							}
							::gst_object_unref(GST_OBJECT(sink_pad));
						}
						catch(...)
						{
							::gst_object_unref(GST_OBJECT(source_pad));
							throw;
						}
						::gst_object_unref(GST_OBJECT(source_pad));
					}
					catch(...)
					{
						::gst_caps_unref(caps_); caps_ = 0;
						delete buffer_; buffer_ = 0;
						throw;
					}
					::gst_element_set_state(pipeline_, ::GST_STATE_READY);
					wait_for_state(*pipeline_, ::GST_STATE_READY);
				}
				catch(...)
				{
					::gst_object_unref(GST_OBJECT(pipeline_)); pipeline_ = 0;
					throw;
				}
			}

			bool gstreamer::opened() const
			{
				return pipeline_ && (wait_for_state(*pipeline_, ::GST_STATE_READY) || wait_for_state(*pipeline_, ::GST_STATE_PLAYING));
			}

			void gstreamer::do_start() throw(engine::exception)
			{
				resource::do_start();
				current_read_buffer_ = current_write_buffer_;
				::gst_element_set_state(pipeline_, ::GST_STATE_PLAYING);
				wait_for_state(*pipeline_, ::GST_STATE_PLAYING);
			}
		
			bool gstreamer::started() const
			{
				return opened() && wait_for_state(*pipeline_, ::GST_STATE_PLAYING);
			}
		
			void gstreamer::handoff_static(::GstElement * source, ::GstBuffer * buffer, ::GstPad * pad, gstreamer * instance)
			{
				instance->handoff(*buffer, *pad);
			}

			void gstreamer::handoff(::GstBuffer & buffer, ::GstPad & pad)
			{
				if(!caps_set_)
				{
					loggers::trace()("caps not set on buffer ; setting them", UNIVERSALIS__COMPILER__LOCATION);
					::gst_buffer_set_caps(&buffer, caps_);
					caps_set_ = true;
				}
				if(loggers::trace())
				{
					std::ostringstream s; s << "handoff " << current_read_buffer_;
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				{
					unsigned int current_write_buffer;
					{
						boost::mutex::scoped_lock lock(mutex_);
						current_write_buffer = current_write_buffer_;
					}
					if(current_read_buffer_ == current_write_buffer)
					{
						loggers::warning()("underrun");
						//loggers::trace()("notifying condition", UNIVERSALIS__COMPILER__LOCATION);
						condition_.notify_one();
						return;
					}
				}
				{
					output_sample_type * out(reinterpret_cast<output_sample_type*>(GST_BUFFER_DATA(&buffer)));
					if(loggers::trace())
					{
						std::size_t const size(GST_BUFFER_SIZE(&buffer));
						std::ostringstream s; s << "buffer size: " << size << ", data address: " << out;
						loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
					}
					{
						output_sample_type * in(reinterpret_cast<output_sample_type*>(buffer_) + current_read_buffer_ * samples_per_buffer);
						for(unsigned int event(0) ; event < samples_per_buffer ; ++event)
						{
							*out++ = *in++; // just use std::memcpy
						}
					}
				}
				{
					boost::mutex::scoped_lock lock(mutex_);
					++current_read_buffer_ %= buffers;
				}
				//loggers::trace()("notifying condition", UNIVERSALIS__COMPILER__LOCATION);
				condition_.notify_one();
			}

			void gstreamer::do_process() throw(engine::exception)
			{
				if(loggers::trace())
				{
					std::ostringstream s; s << "process " << current_write_buffer_;
					loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				{
					engine::buffer::channel & in(single_input_ports()[0]->buffer()[0]);
					assert(samples_per_buffer == in.size());
					output_sample_type * out(reinterpret_cast<output_sample_type*>(buffer_) + current_write_buffer_ * samples_per_buffer);
					for(unsigned int event(0) ; event < in.size(); ++event)
					{
						out[event] = static_cast<output_sample_type>(std::numeric_limits<output_sample_type>::min() * in[event].sample());
					}
				}
				{
					unsigned int const next_write_buffer((current_write_buffer_ + 1) % buffers);
					boost::mutex::scoped_lock lock(mutex_);
					if(current_read_buffer_ == next_write_buffer)
					{
						//loggers::trace()("waiting for condition notification", UNIVERSALIS__COMPILER__LOCATION);
						condition_.wait(lock);
					}
					current_write_buffer_ = next_write_buffer;
				}
			}
			
			void gstreamer::do_stop() throw(engine::exception)
			{
				::gst_element_set_state(pipeline_, ::GST_STATE_READY);
				wait_for_state(*pipeline_, ::GST_STATE_READY);
				::gst_caps_unref(caps_); caps_ = 0;
				caps_set_ = false;
				resource::do_stop();
			}
		
			void gstreamer::do_close() throw(engine::exception)
			{
				::gst_element_set_state(pipeline_, ::GST_STATE_NULL);
				wait_for_state(*pipeline_, ::GST_STATE_NULL);
				::gst_object_unref(GST_OBJECT(pipeline_)); pipeline_ = 0;
				// deinitialize gstreamer
				{
					static bool once = false;
					static boost::mutex mutex;
					boost::mutex::scoped_lock lock(mutex);
					if(!once)
					{
						once = true;
						::gst_deinit();
					}
				}
				delete buffer_; buffer_ = 0;
				resource::do_close();
			}
			
			gstreamer::~gstreamer() throw()
			{
				close();
			}
		}
	}
}
