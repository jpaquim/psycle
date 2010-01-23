// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::plugins::outputs::gstreamer
#include <psycle/detail/project.private.hpp>
#include "gstreamer.hpp"
#include <psycle/stream/formats/riff_wave/format.hpp>
#include <gst/gst.h>
#include <universalis/compiler/numeric.hpp>
#include <universalis/exception.hpp>
#include <universalis/stdlib/thread.hpp>
#include <limits>
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace psycle { namespace plugins { namespace outputs {

using engine::exceptions::runtime_error;
using stream::formats::riff_wave::format;

PSYCLE__PLUGINS__NODE_INSTANTIATOR(gstreamer)

gstreamer::gstreamer(engine::plugin_library_reference & plugin_library_reference, engine::graph & graph, std::string const & name) throw(engine::exception)
:
	resource(plugin_library_reference, graph, name),
	pipeline_(),
	source_(),
	caps_filter_(),
	sink_(),
	caps_(),
	intermediate_buffer_()
{
	ports::inputs::single::create_on_heap(*this, "in");
	ports::inputs::single::create_on_heap(*this, "amplification", boost::cref(1));
}

void gstreamer::channel_change_notification_from_port(engine::port const & port) throw(engine::exception) {
	if(&port == &in_port()) {
		last_samples_.resize(port.channels());
		for(std::size_t i(0); i < last_samples_.size(); ++i) last_samples_[i] = 0;
	}
	resource::channel_change_notification_from_port(port);
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
			if(loggers::information()) {
				std::ostringstream s; s << "instantiating " << type << " " << name;
				loggers::information()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			::GstElementFactory * factory;
			if(!(factory = ::gst_element_factory_find(type.c_str()))) {
				std::ostringstream s;
				s << "could not find element type: " << type;
				throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(loggers::information()) {
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
		::GstClockTime intermediate_timeout_nanoseconds(std::min(timeout_nanoseconds, static_cast<GstClockTime>(0.5e9)));
		::GstClockTime total_nanoseconds_waited(0);
		for(;;) {
			::GstState current_state, pending_state;
			::GstStateChangeReturn const result(::gst_element_get_state(&element, &current_state, &pending_state, intermediate_timeout_nanoseconds));
			switch(result) {
				case ::GST_STATE_CHANGE_NO_PREROLL:
					if(loggers::information()()) loggers::information()("no preroll", UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
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
						universalis::os::loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION__NO_CLASS);
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
	unsigned int global_client_count;
	mutex global_client_count_mutex;
	once_flag global_client_count_init_once_flag = STD_ONCE_INIT;
	void global_client_count_init() {
		// note: we do not need to lock here, but this is a way to ensure it is initialised.
		scoped_lock<mutex> lock(global_client_count_mutex);
		global_client_count = 0;
	}
}

void gstreamer::do_open() throw(engine::exception) {
	resource::do_open();

	{ // initialize gstreamer
		call_once(global_client_count_init_once_flag, global_client_count_init);
		::scoped_lock<mutex> lock(global_client_count_mutex);
		if(!global_client_count++) {
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
		std::string const sink_name(name() + "-sink");
		try { sink_ = &instantiate("gconfaudiosink", sink_name); }
		catch(...) {
			psycle_log
			// maybe the user didn't configure his default audio-sink, falling back to several possibilities
			try { sink_ = &instantiate("autoaudiosink", sink_name); }
			catch(...) {
				psycle_log
				#if 0 // jacksink needs to be put in a jackbin, because it's pulling audio data.
					try { sink_ = &instantiate("jacksink", sink_name); }
					catch(...) {
				#endif
				try { sink_ = &instantiate("alsasink", sink_name); }
				catch(...) {
					psycle_log
					try { sink_ = &instantiate("esdsink", sink_name); }
					catch(...) {
						psycle_log
						try { sink_ = &instantiate("osssink", sink_name); }
						catch(...) {
							psycle_log
							try { sink_ = &instantiate("artssink", sink_name); }
							catch(...) {
								psycle_log
								throw;
		}}}}}}
	#undef psycle_log

	// audio format
	format format(in_port().channels(), in_port().events_per_second(), /*significant_bits_per_channel_sample*/ 16); ///\todo parametrable
	if(loggers::information()) {
		std::ostringstream s;
		s << "format: " << format.description();
		loggers::information()(s.str());
	}

	// create caps, audio format pad capabilities 
	caps_ = ::gst_caps_new_simple(
		"audio/x-raw-int", // note we could simply use "audio/x-raw-float" and we'd be freed from having to handle the conversion,
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

	// create a pipeline
	if(!(pipeline_ = ::gst_pipeline_new((name() + "-pipeline").c_str()))) throw runtime_error("could not create new empty pipeline", UNIVERSALIS__COMPILER__LOCATION);
	
	// add the elements to the pipeline
	//::gst_bin_add_many(GST_BIN(pipeline), source_, caps_filter_, sink_, (void*)0);
	if(!::gst_bin_add(GST_BIN(pipeline_), source_     )) throw runtime_error("could not add source element to pipeline",      UNIVERSALIS__COMPILER__LOCATION);
	if(!::gst_bin_add(GST_BIN(pipeline_), caps_filter_)) throw runtime_error("could not add caps filter element to pipeline", UNIVERSALIS__COMPILER__LOCATION);
	if(!::gst_bin_add(GST_BIN(pipeline_), sink_       )) throw runtime_error("could not add sink element to pipeline",        UNIVERSALIS__COMPILER__LOCATION);
	
	// link the element pads together
	if(!::gst_element_link_many(source_, caps_filter_, sink_, (void*)0)) throw runtime_error("could not link element pads", UNIVERSALIS__COMPILER__LOCATION);
	//if(!::gst_element_link_pads(source_,      "sink", caps_filter_, "src")) throw runtime_error("could not link source element sink pad to caps filter element src pad", UNIVERSALIS__COMPILER__LOCATION);
	//if(!::gst_element_link_pads(caps_filter_, "sink", sink_,        "src")) throw runtime_error("could not link caps filter element sink pad to sink element src pad",   UNIVERSALIS__COMPILER__LOCATION);

	// buffer settings

	unsigned int const periods(4); ///\todo parametrable
	unsigned int const period_frames(1024); ///\todo parametrable
	unsigned int const period_size(static_cast<unsigned int>(period_frames * format.bytes_per_sample()));
	if(loggers::information()) {
		real const latency(static_cast<real>(graph().events_per_buffer()) / format.samples_per_second());
		std::ostringstream s;
		s <<
			"period size: " << period_size << " bytes; "
			"periods: " << periods << "; "
			"total buffer size: " << periods * period_size << " bytes; "
			"latency: between " << latency << " and " << latency * periods << " seconds";
		loggers::information()(s.str());
	}

	// set properties of the fakesrc element
	::g_object_set(
		G_OBJECT(source_),
		"signal-handoffs", ::gboolean(true),
		"data"           , ::gint(/*FAKE_SRC_DATA_SUBBUFFER*/ 2), // data allocation method
		"parentsize"     , ::gint(period_size * periods),
		"sizemax"        , ::gint(period_size),
		"sizetype"       , ::gint(/*FAKE_SRC_SIZETYPE_FIXED*/ 2), // fixed to sizemax
		"filltype"       , ::gint(/*FAKE_SRC_FILLTYPE_NOTHING*/ 1),
		(void*)0
	);

	{ // allocate the intermediate buffer
		///\todo use gstreamer's lock-free ringbuffer
		// note: period_frames may be different from graph().events_per_buffer()
		std::size_t const bytes(static_cast<std::size_t>(graph().events_per_buffer() * format.bytes_per_sample()));
		if(!(intermediate_buffer_ = new char[bytes])) {
			std::ostringstream s; s << "not enough memory to allocate " << bytes << " bytes on heap";
			throw engine::exceptions::runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
		}
		intermediate_buffer_end_ = intermediate_buffer_ + bytes;
	}

	// set the pipeline state to ready
	set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_READY);

	// set to false so that started() returns false
	wait_for_state_to_become_playing_ = false;
}

bool gstreamer::opened() const {
	if(!pipeline_) return false;
	::GstState s(state(*pipeline_));
	return
		s == ::GST_STATE_READY ||
		s == ::GST_STATE_PAUSED ||
		s == ::GST_STATE_PLAYING;
}

void gstreamer::do_start() throw(engine::exception) {
	resource::do_start();
	intermediate_buffer_current_read_pointer_ = intermediate_buffer_;
	stop_requested_ = handoff_called_ = false;
	wait_for_state_to_become_playing_ = true;
	// register our callback to the handoff signal of the fakesrc element
	if(!::g_signal_connect(G_OBJECT(source_), "handoff", G_CALLBACK(handoff_static), this)) throw runtime_error("could not connect handoff signal", UNIVERSALIS__COMPILER__LOCATION);
	// set the pipeline state to playing
	#if 0 // Handoff is called before state is changed to playing.
		set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_PLAYING);
	#else
		::gst_element_set_state(GST_ELEMENT(pipeline_), ::GST_STATE_PLAYING);
	#endif
	{ scoped_lock lock(mutex_);
		while(!handoff_called_) condition_.wait(lock);
		wait_for_state_to_become_playing_ = false;
	}
	condition_.notify_one();
}

bool gstreamer::started() const {
	if(!opened()) return false;
	if(state(*pipeline_) == ::GST_STATE_PLAYING) return true;
}

/// this is called from within gstreamer's processing thread.
void gstreamer::handoff_static(::GstElement * source, ::GstBuffer * buffer, ::GstPad * pad, gstreamer * instance) {
	instance->handoff(*buffer, *pad);
}

/// this is called from within gstreamer's processing thread.
void gstreamer::handoff(::GstBuffer & buffer, ::GstPad & pad) {
	if(false && loggers::trace()) loggers::trace()("handoff", UNIVERSALIS__COMPILER__LOCATION);
	{ scoped_lock lock(mutex_);
		while(io_ready() && !stop_requested_ && !wait_for_state_to_become_playing_) condition_.wait(lock);
		if(stop_requested_) return;
		// Handoff is called before state is changed to playing.
		if(wait_for_state_to_become_playing_) {
			if(loggers::trace()) loggers::trace()("handoff called", UNIVERSALIS__COMPILER__LOCATION);
			handoff_called_ = true;
			condition_.notify_one();
			while(wait_for_state_to_become_playing_) condition_.wait(lock);
		}
	}
	::guint8 * out(GST_BUFFER_DATA(&buffer));
	std::size_t out_size(GST_BUFFER_SIZE(&buffer));
	while(true) { // copy the intermediate buffer to the gstreamer buffer
		std::size_t const in_size(intermediate_buffer_end_ - intermediate_buffer_current_read_pointer_);
		std::size_t const copy_size(std::min(in_size, out_size));
		std::memcpy(out, intermediate_buffer_current_read_pointer_, copy_size);
		intermediate_buffer_current_read_pointer_ += copy_size;
		out_size -= copy_size;
		if(!out_size) break;
		{ scoped_lock lock(mutex_);
			io_ready(true);
		}
		condition_.notify_one();
		{ scoped_lock lock(mutex_);
			while(io_ready() && !stop_requested_) condition_.wait(lock);
		}
		if(stop_requested_) return;
		out += copy_size;
	}
	if(intermediate_buffer_current_read_pointer_ == intermediate_buffer_end_) {
		{ scoped_lock lock(mutex_);
			io_ready(true);
		}
		condition_.notify_one();
	}
}

/// this is called from within psycle's host's processing thread(s).
void gstreamer::do_process() throw(engine::exception) {
	if(false && loggers::trace()) loggers::trace()("process", UNIVERSALIS__COMPILER__LOCATION);
	if(!in_port()) return;
	{ scoped_lock lock(mutex_);
		if(false && loggers::warning() && !io_ready()) loggers::warning()("blocking", UNIVERSALIS__COMPILER__LOCATION);
		while(!io_ready()) condition_.wait(lock);
	}
	{ // fill the intermediate buffer
		unsigned int const channels(in_port().channels());
		unsigned int const samples_per_buffer(graph().events_per_buffer());
		assert(last_samples_.size() == channels);
		for(unsigned int c(0); c < channels; ++c) {
			engine::buffer::channel & in(in_port().buffer()[c]);
			output_sample_type * out(reinterpret_cast<output_sample_type*>(intermediate_buffer_) + c);
			unsigned int spread(0);
			for(std::size_t e(0), s(in.size()); e < s; ++e) {
				real s(in[e].sample());
				{
					s *= std::numeric_limits<output_sample_type>::max();
					if     (s < std::numeric_limits<output_sample_type>::min()) s = std::numeric_limits<output_sample_type>::min();
					else if(s > std::numeric_limits<output_sample_type>::max()) s = std::numeric_limits<output_sample_type>::max();
				}
				last_samples_[c] = static_cast<output_sample_type>(s);
				for( ; spread <= in[e].index() ; ++spread, ++out) *out = last_samples_[c];
			}
			for( ; spread < samples_per_buffer ; ++spread, ++out) *out = last_samples_[c];
		}
	}
	{ scoped_lock lock(mutex_);
		intermediate_buffer_current_read_pointer_ = intermediate_buffer_;
		io_ready(false);
	}
	condition_.notify_one();
}

void gstreamer::do_stop() throw(engine::exception) {
	if(pipeline_) {
		{ scoped_lock lock(mutex_);
			stop_requested_ = true;
		}
		condition_.notify_one();
		set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_READY);
	}
	resource::do_stop();
}

void gstreamer::do_close() throw(engine::exception) {
	if(pipeline_) {
		{ scoped_lock lock(mutex_);
			stop_requested_ = true;
		}
		condition_.notify_one();
		set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_NULL);
		::gst_object_unref(GST_OBJECT(pipeline_)); pipeline_ = 0;
	}
	sink_ = caps_filter_ = source_ = 0; caps_ = 0;

	{ // deinitialize gstreamer
		call_once(global_client_count_init_once_flag, global_client_count_init);
		::scoped_lock<mutex> lock(global_client_count_mutex);
		if(!--global_client_count) {
			#if 0  // gst_deinit must not be called because gst_init won't work afterwards
				::gst_deinit();
			#else
				global_client_count = 1;
			#endif
		}
	}

	delete[] intermediate_buffer_; intermediate_buffer_ = 0;
	resource::do_close();
}

}}}
