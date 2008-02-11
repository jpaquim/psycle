/* -*- mode:c++, indent-tabs-mode:t -*- */
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycle development team http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::outputs::gstreamer
#include <psycle/detail/project.private.hpp>
#include "gstreamer.hpp"
#include <psycle/stream/formats/riff_wave/format.hpp>
#include <gst/gst.h>
#include <universalis/compiler/numeric.hpp>
#include <universalis/exception.hpp>
#include <thread>
#include <limits>
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace psycle { namespace plugins { namespace outputs {
	using engine::exceptions::runtime_error;
	using stream::formats::riff_wave::format;

	namespace {
		///\todo parametrable
		typedef std::int16_t output_sample_type;
	}

	PSYCLE__PLUGINS__NODE_INSTANTIATOR(gstreamer)

	gstreamer::gstreamer(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name) throw(engine::exception)
	:
		resource(plugin_library_reference, graph, name),
		pipeline_(),
		source_(),
		caps_filter_(),
		sink_(),
		caps_(),
		buffer_(),
		current_read_position_(), current_write_position_()
	{
		ports::inputs::single::create_on_heap(*this, "in");
		ports::inputs::single::create_on_heap(*this, "amplification", boost::cref(1));
	}

	void gstreamer::do_name(std::string const & name) {
		resource::do_name(name);
		if(pipeline_) ::gst_element_set_name(pipeline_, (name + "-pipeline").c_str());
		if(source_  ) ::gst_element_set_name(source_  , (name + "-src"     ).c_str());
		if(sink_    ) ::gst_element_set_name(sink_    , (name + "-sink"    ).c_str());
	}

	namespace {
		::GstElement & instantiate(std::string const & type, std::string const & name) throw(universalis::exception) {
			try {
				if(loggers::information()()) {
					std::ostringstream s; s << "instantiating " << type << " " << name;
					loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				::GstElementFactory * factory;
				if(!(factory = ::gst_element_factory_find(type.c_str()))) {
					std::ostringstream s;
					s << "could not find element type: " << type;
					throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				if(loggers::information()()) {
					std::ostringstream s;
					s
						<< "The element type " << ::gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory))
						<< " is a member of the group " << ::gst_element_factory_get_klass(factory) << "." << std::endl
						<< "Description:" << std::endl
						<< ::gst_element_factory_get_description(factory);
					loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				::GstElement * element;
				if(!(element = ::gst_element_factory_create(factory, name.c_str()))) {
					std::ostringstream s;
					s << "found element type: " << ::gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory)) << ", but could not create element instance: " << name;
					throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
				}
				return *element;
			}
			UNIVERSALIS__EXCEPTIONS__CATCH_ALL_AND_CONVERT_TO_STANDARD_AND_RETHROW__NO_CLASS
			return *static_cast< ::GstElement* >(0); // dummy return to avoid warning
		}

		::GstState inline state(::GstElement & element) {
			// note: GST_STATE(&element) directly accesses the structure data, so this would be wrong for multi-threading.
			::GstState current_state, pending_state;
			::GstClockTime const timeout(0); // don't wait
			//::GstStateChangeReturn const result(
				::gst_element_get_state(&element, &current_state, &pending_state, timeout);
			return current_state;
		}
		
		::GstClockTime const default_timeout_nanoseconds(static_cast<GstClockTime>(5e9));

		void wait_for_state(::GstElement & element, ::GstState state_wanted, ::GstClockTime timeout_nanoseconds = default_timeout_nanoseconds) throw(universalis::exception) {
			::GstClockTime intermediate_timeout_nanoseconds(std::min(timeout_nanoseconds, static_cast<GstClockTime>(0.1e9)));
			::GstClockTime total_nanoseconds_waited(0);
			for(;;) {
				::GstState current_state, pending_state;
				::GstStateChangeReturn const result(::gst_element_get_state(&element, &current_state, &pending_state, intermediate_timeout_nanoseconds));
				switch(result) {
					case ::GST_STATE_CHANGE_NO_PREROLL:
						universalis::operating_system::loggers::information()("no preroll", UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
					case ::GST_STATE_CHANGE_SUCCESS:
						if(current_state == state_wanted) return;
						else {
							std::ostringstream s;
								s
									<< "unexpected current state on element: " << ::gst_element_get_name(&element)
									<< "; current state: " << ::gst_element_state_get_name(current_state)
									<< ", expected state: " << ::gst_element_state_get_name(state_wanted);
							throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						}
					case ::GST_STATE_CHANGE_ASYNC:
						if(pending_state != state_wanted) {
							std::ostringstream s;
								s
									<< "unexpected pending state on element: " << ::gst_element_get_name(&element)
									<< "; pending state: " << ::gst_element_state_get_name(pending_state)
									<< ", expected state: " << ::gst_element_state_get_name(state_wanted);
							throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						}
						total_nanoseconds_waited += intermediate_timeout_nanoseconds;
						if(total_nanoseconds_waited > timeout_nanoseconds) {
							std::ostringstream s;
								s
									<< "timeout while waiting for state change on element: " << ::gst_element_get_name(&element)
									<< "; pending state: " << ::gst_element_state_get_name(pending_state);
							throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						}
						{
							std::ostringstream s;
							s
								<< "waiting for element " << ::gst_element_get_name(&element)
								<< " to asynchronously change its state from " << ::gst_element_state_get_name(current_state)
								<< " to " << ::gst_element_state_get_name(state_wanted)
								<< " (waited a total of " << total_nanoseconds_waited * 1e-9
								<< " seconds ; will timeout after " << timeout_nanoseconds * 1e-9 << " seconds)";
							universalis::operating_system::loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						}
						intermediate_timeout_nanoseconds *= 2; // makes timeouts progressively sparser
						continue;
					case ::GST_STATE_CHANGE_FAILURE: {
							std::ostringstream s;
								s
									<< "state change failure on element: " << ::gst_element_get_name(&element)
									<< "; state change: " << result
									<< ", current state: " << ::gst_element_state_get_name(current_state)
									<< ", pending state: " << ::gst_element_state_get_name(pending_state);
							throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
						}
					default: {
							std::ostringstream s;
								s
									<< "unknown state change on element: " << ::gst_element_get_name(&element)
									<< "; state change: " << result
									<< ", current state: " << current_state // don't use ::gst_element_state_get_name in unknown situation
									<< ", pending state: " << pending_state; // don't use ::gst_element_state_get_name in unknown situation
							throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
					}
				}
			}
		}

		void set_state_synchronously(::GstElement & element, ::GstState state, ::GstClockTime timeout_nanoseconds = default_timeout_nanoseconds) throw(universalis::exception) {
			::gst_element_set_state(&element, state);
			wait_for_state(element, state, timeout_nanoseconds);
		}
	}
	
	namespace {
		unsigned int   global_client_count;
		std::mutex     global_client_count_mutex;
		std::once_flag global_client_count_init_once_flag = STD_ONCE_INIT;
		void           global_client_count_init() {
			// note: we do not need to lock here, but this is a way to ensure it is initialised.
			std::scoped_lock<std::mutex> lock(global_client_count_mutex);
			global_client_count = 0;
		}
	}

	void gstreamer::do_open() throw(engine::exception) {
		resource::do_open();

		{ // initialize gstreamer
			std::call_once(global_client_count_init_once_flag, global_client_count_init);
			std::scoped_lock<std::mutex> lock(global_client_count_mutex);
			if(!global_client_count) {
				++global_client_count;
				int * argument_count(0);
				char *** arguments(0);
				::GError * error(0);
				if(!::gst_init_check(argument_count, arguments, &error)) {
					std::ostringstream s; s << "could not initialize gstreamer: " << error->code << ": " << error->message;
					::g_clear_error(&error);
					throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}}}

		// create an audio sink
		#define psycle_log loggers::information()("exception: caught while trying to instantiate audio sink ; trying next type ...", UNIVERSALIS__COMPILER__LOCATION);
			try { sink_ = &instantiate("gconfaudiosink", name() + "-sink"); }
			catch(...) {
				psycle_log
				// maybe the user didn't configure his default audio-sink, falling back to several possibilities
				try { sink_ = &instantiate("autoaudiosink", name() + "-sink"); }
				catch(...) {
					psycle_log
					#if 0 // jacksink needs to be put in a jackbin, because it's pulling audio data.
						try { sink_ = &instantiate("jacksink", name() + "-sink"); }
						catch(...) {
					#endif
					try { sink_ = &instantiate("alsasink", name() + "-sink"); }
					catch(...) {
						psycle_log
						try { sink_ = &instantiate("esdsink", name() + "-sink"); }
						catch(...) {
							psycle_log
							try { sink_ = &instantiate("osssink", name() + "-sink"); }
							catch(...) {
								psycle_log
								try { sink_ = &instantiate("artssink", name() + "-sink"); }
								catch(...) {
									psycle_log
									throw;
			}}}}}}
		#undef psycle_log

		// audio format
		format format(in_port().channels(), in_port().events_per_second(), /*significant_bits_per_channel_sample*/ 16); ///\todo parametrable
		if(loggers::information()()) {
			std::ostringstream s;
			s << "format: " << format.description();
			loggers::information()(s.str());
		}

		// create caps, audio format pad capabilities 
		caps_ = ::gst_caps_new_simple(
			"audio/x-raw-int", // note we could simply use "audio/x-raw-float" and we'd be freed from having to handle the convertion,
			"rate"      , G_TYPE_INT    , ::gint(format.samples_per_second()),
			"channels"  , G_TYPE_INT    , ::gint(format.channels()),
			"width"     , G_TYPE_INT    , ::gint(format.bits_per_channel_sample()),
			"depth"     , G_TYPE_INT    , ::gint(format.significant_bits_per_channel_sample()),
			"signed"    , G_TYPE_BOOLEAN, ::gboolean(/*format.sample_signed()*/ std::numeric_limits<output_sample_type>::min() < 0),
			"endianness", G_TYPE_INT    , ::gint(/*format.sample_endianness()*/ G_BYTE_ORDER),
			(void*)0
		);

		// create a capsfilter
		caps_filter_ = &instantiate("capsfilter", name() + "-caps-filter");

		// set caps on the capsfilter
		::g_object_set(G_OBJECT(caps_filter_), "caps", caps_, (void*)0);

		// create a fakesrc
		source_ = &instantiate("fakesrc", name() + "-src");

		// create a queue (note that this used to work without a queue in the past)
		queue_ = &instantiate("queue", name() + "-queue");
		
		// create a pipeline
		if(!(pipeline_ = ::gst_pipeline_new((name() + "-pipeline").c_str()))) throw runtime_error("could not create new empty pipeline", UNIVERSALIS__COMPILER__LOCATION);
		
		// add the elements to the pipeline
		//::gst_bin_add_many(GST_BIN(pipeline), source_, caps_filter_, sink_, (void*)0);
		if(!::gst_bin_add(GST_BIN(pipeline_), source_     )) throw runtime_error("could not add source element to pipeline",      UNIVERSALIS__COMPILER__LOCATION);
		if(!::gst_bin_add(GST_BIN(pipeline_), queue_      )) throw runtime_error("could not add queue element to pipeline",       UNIVERSALIS__COMPILER__LOCATION);
		if(!::gst_bin_add(GST_BIN(pipeline_), caps_filter_)) throw runtime_error("could not add caps filter element to pipeline", UNIVERSALIS__COMPILER__LOCATION);
		if(!::gst_bin_add(GST_BIN(pipeline_), sink_       )) throw runtime_error("could not add sink element to pipeline",        UNIVERSALIS__COMPILER__LOCATION);
		
		// link the element pads together
		if(!::gst_element_link_many(source_, queue_, caps_filter_, sink_, (void*)0)) throw runtime_error("could not link element pads", UNIVERSALIS__COMPILER__LOCATION);
		//if(!::gst_element_link_pads(source_,      "sink", caps_filter_, "src")) throw runtime_error("could not link source element sink pad to caps filter element src pad", UNIVERSALIS__COMPILER__LOCATION);
		//if(!::gst_element_link_pads(caps_filter_, "sink", sink_,        "src")) throw runtime_error("could not link caps filter element sink pad to sink element src pad",   UNIVERSALIS__COMPILER__LOCATION);

		// buffer settings
		samples_per_buffer_ = 1024; ///\todo parametrable
		buffer_size_ = static_cast<unsigned int>(samples_per_buffer_ * format.bytes_per_sample());
		if(loggers::information()()) {
			std::ostringstream s;
			s << "buffer size: " << buffer_size_ << " bytes";
			loggers::information()(s.str());
		}
		buffers_ = 4; ///\todo parametrable
		if(loggers::information()()) {
			std::ostringstream s;
			s << buffers_ << " buffers; total buffer size: " << buffers_ * buffer_size_ << " bytes";
			loggers::information()(s.str());
		}
		{
			real const latency(static_cast<real>(samples_per_buffer_) / format.samples_per_second());
			if(loggers::information()()) {
				std::ostringstream s;
				s << "latency: between " << latency << " and " << latency * buffers_ << " seconds ";
				loggers::information()(s.str());
			}
		}

		// set properties of the fakesrc element
		::g_object_set(
			G_OBJECT(source_),
			"signal-handoffs", ::gboolean(true),
			"data"           , ::gint(/*FAKE_SRC_DATA_SUBBUFFER*/ 2), // data allocation method
			"parentsize"     , ::gint(buffer_size_ * buffers_),
			"sizemax"        , ::gint(buffer_size_),
			"sizetype"       , ::gint(/*FAKE_SRC_SIZETYPE_FIXED*/ 2), // fixed to sizemax
			"filltype"       , ::gint(/*FAKE_SRC_FILLTYPE_NOTHING*/ 1),
			(void*)0
		);

		{ // allocate a buffer
			std::size_t const bytes(buffer_size_ * buffers_);
			if(!(buffer_ = new char[bytes])) {
				std::ostringstream s; s << "not enough memory to allocate " << bytes << " bytes on heap";
				throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}

		current_read_position_ = current_write_position_;
		wait_for_state_to_become_playing_ = true;
		stop_requested_ = false;

		// set the pipeline state to ready
		set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_READY);
	}

	bool gstreamer::opened() const {
		return pipeline_ && (
			state(*pipeline_) == ::GST_STATE_READY ||
			state(*pipeline_) == ::GST_STATE_PAUSED ||
			state(*pipeline_) == ::GST_STATE_PLAYING
		);
	}

	void gstreamer::do_start() throw(engine::exception) {
		resource::do_start();
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			current_read_position_ = current_write_position_;
			wait_for_state_to_become_playing_ = true;
			stop_requested_ = false;
		}
		// register our callback to the handoff signal of the fakesrc element
		if(!::g_signal_connect(G_OBJECT(source_), "handoff", G_CALLBACK(handoff_static), this)) throw runtime_error("could not connect handoff signal", UNIVERSALIS__COMPILER__LOCATION);
		// set the pipeline state to playing
		set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_PLAYING);
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			wait_for_state_to_become_playing_ = false;
		}
		condition_.notify_one();
	}

	bool gstreamer::started() const {
		return opened() && state(*pipeline_) == ::GST_STATE_PLAYING;
	}

	void gstreamer::handoff_static(::GstElement * source, ::GstBuffer * buffer, ::GstPad * pad, gstreamer * instance) {
		instance->handoff(*buffer, *pad);
	}

	void gstreamer::handoff(::GstBuffer & buffer, ::GstPad & pad) {
		if(false && loggers::trace()) {
			std::ostringstream s; s << "handoff " << current_read_position_;
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			if(current_read_position_ == current_write_position_) {
				if(stop_requested_) return; 
				// Handoff is called before state is changed to playing.
				if(wait_for_state_to_become_playing_) {
					loggers::trace()("waiting for state to become playing", UNIVERSALIS__COMPILER__LOCATION);
					return;
				} else {
					loggers::warning()("underrun", UNIVERSALIS__COMPILER__LOCATION);
				}
				condition_.wait(lock);
				if(stop_requested_) return; 
			}
		}
		if(!loggers::trace()()) {
			static const char c [] = { '-', '\\', '|', '/' };
			std::cout << ' ' << c[current_read_position_ % sizeof c] << '\r' << std::flush;
		}
		{
			output_sample_type * out(reinterpret_cast<output_sample_type*>(GST_BUFFER_DATA(&buffer)));
			if(false && loggers::trace()) {
				std::size_t const size(GST_BUFFER_SIZE(&buffer));
				std::ostringstream s; s << "buffer size: " << size << ", data address: " << out;
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			{
				output_sample_type * in(reinterpret_cast<output_sample_type*>(buffer_) + current_read_position_ * samples_per_buffer_);
				std::memcpy(out, in, buffer_size_);
			}
		}
		{
			std::scoped_lock<std::mutex> lock(mutex_);
			++current_read_position_ %= buffers_;
		}
		condition_.notify_one();
	}

	void gstreamer::do_process() throw(engine::exception) {
		if(false && loggers::trace()) {
			std::ostringstream s; s << "process " << current_write_position_;
			loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		if(!in_port()) return;
		for(unsigned int c(0); c < in_port().channels(); ++c) {
			engine::buffer::channel & in(in_port().buffer()[c]);
			assert(samples_per_buffer_ == in.size());
			output_sample_type * out(reinterpret_cast<output_sample_type*>(buffer_) + current_write_position_ * samples_per_buffer_);
			for(std::size_t e(0), s(in.size()); e < s; ++e) {
				real s(in[e].sample()); ///\todo support for sparse stream
				{
					s *= std::numeric_limits<output_sample_type>::max();
					if     (s < std::numeric_limits<output_sample_type>::min()) s = std::numeric_limits<output_sample_type>::min();
					else if(s > std::numeric_limits<output_sample_type>::max()) s = std::numeric_limits<output_sample_type>::max();
				}
				out[c] = static_cast<output_sample_type>(s);
				++out; ///\todo interleaved channels?
			}
		}
		condition_.notify_one();
		{
			unsigned int const next_write_buffer((current_write_position_ + 1) % buffers_);
			std::scoped_lock<std::mutex> lock(mutex_);
			if(current_read_position_ == next_write_buffer) condition_.wait(lock);
			current_write_position_ = next_write_buffer;
		}
	}
	
	void gstreamer::do_stop() throw(engine::exception) {
		if(pipeline_) {
			{
				std::scoped_lock<std::mutex> lock(mutex_);
				stop_requested_ = true;
			}
			condition_.notify_one();
			set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_READY);
		}
		resource::do_stop();
	}

	void gstreamer::do_close() throw(engine::exception) {
		if(pipeline_) {
			{
				std::scoped_lock<std::mutex> lock(mutex_);
				stop_requested_ = true;
			}
			condition_.notify_one();
			set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_NULL);
			::gst_object_unref(GST_OBJECT(pipeline_)); pipeline_ = 0;
		}
		if(false) { // seems the pipeline owns its element. need to check the doc.
			if(sink_) ::gst_object_unref(GST_OBJECT(sink_)); sink_ = 0;
			if(caps_filter_) ::gst_object_unref(GST_OBJECT(caps_filter_)); caps_filter_ = 0;
			if(queue_) ::gst_object_unref(GST_OBJECT(queue_)); queue_ = 0;
			if(source_) ::gst_object_unref(GST_OBJECT(source_)); source_ = 0;
			if(caps_) ::gst_caps_unref(caps_); caps_ = 0;
		} else sink_ = caps_filter_ = queue_ = source_ = 0; caps_ = 0;

		{ // deinitialize gstreamer
			std::call_once(global_client_count_init_once_flag, global_client_count_init);
			std::scoped_lock<std::mutex> lock(global_client_count_mutex);
			if(!--global_client_count) ::gst_deinit();
		}

		delete[] buffer_; buffer_ = 0;
		resource::do_close();
	}
}}}

