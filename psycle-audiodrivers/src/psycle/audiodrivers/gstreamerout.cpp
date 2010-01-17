// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#if defined PSYCLE__GSTREAMER_AVAILABLE

#include "gstreamerout.h"
#include <universalis/os/loggers.hpp>
#include <universalis/exception.hpp>
#include <universalis/stdlib/mutex.hpp>
#include <universalis/stdlib/thread.hpp>
#include <gst/gst.h>

namespace psycle { namespace audiodrivers {

namespace loggers = universalis::os::loggers;
using namespace universalis::stdlib;
using universalis::exceptions::runtime_error;

namespace {
	::GstElement & instantiate(std::string const & type, std::string const & name) throw(universalis::exception) {
		try {
			if(loggers::information()) {
				std::ostringstream s; s << "psycle: audiodrivers: gstreamer: instantiating " << type << " " << name;
				loggers::information()(s.str());
			}
			::GstElementFactory * factory;
			if(!(factory = ::gst_element_factory_find(type.c_str()))) {
				std::ostringstream s;
				s << "could not find element type: " << type;
				throw runtime_error(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			if(loggers::information()) {
				std::ostringstream s;
				s <<
					"psycle: audiodrivers: gstreamer:"
					" The element type " << ::gst_plugin_feature_get_name(GST_PLUGIN_FEATURE(factory)) <<
					" is a member of the group " << ::gst_element_factory_get_klass(factory) <<
					". Description: " << ::gst_element_factory_get_description(factory);
				loggers::information()(s.str());
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

	::GstState inline state(::GstElement /*const*/ & element) {
		// note: GST_STATE(&element) directly accesses the structure data, so this would be wrong for multi-threading.
		::GstState current_state, pending_state;
		::GstClockTime const timeout(0); // don't wait
		//::GstStateChangeReturn const result =
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
					if(current_state == state_wanted) {
						if(loggers::trace()) {
							std::ostringstream s;
							s << "psycle: audiodrivers: gstreamer: waited for state: " << ::gst_element_state_get_name(current_state);
							loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
						}
						return;
					} else {
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

AudioDriverInfo GStreamerOut::info() const {
	return AudioDriverInfo("gstreamer", "GStreamer Driver", "Output through the GStreamer infrastructure", true);
}

GStreamerOut::GStreamerOut()
:
	pipeline_(),
	source_(),
	caps_filter_(),
	sink_(),
	caps_()
{}

void GStreamerOut::do_open() throw(std::exception) {
	{ // initialize gstreamer
		universalis::stdlib::call_once(global_client_count_init_once_flag, global_client_count_init);
		scoped_lock<mutex> lock(global_client_count_mutex);
		if(!global_client_count++) {
			if(loggers::trace()) loggers::trace()("psycle: audiodrivers: gstreamer: init", UNIVERSALIS__COMPILER__LOCATION);
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
		std::string const sink_name("sink");
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

	// create caps, audio format pad capabilities
	///\todo: fixed to 16bit sample size right now.
	int const bits_per_channel_sample = sizeof(output_sample_type) * 8;
	int const significant_bits_per_channel_sample = bits_per_channel_sample;
	int const bytes_per_sample = playbackSettings().numChannels() * sizeof(output_sample_type);
	caps_ = ::gst_caps_new_simple(
		"audio/x-raw-int", // note we could simply use "audio/x-raw-float" and we'd be freed from having to handle the conversion,
		"rate"      , G_TYPE_INT    , ::gint(playbackSettings().samplesPerSec()),
		"channels"  , G_TYPE_INT    , ::gint(playbackSettings().numChannels()),
		"width"     , G_TYPE_INT    , ::gint(bits_per_channel_sample),
		"depth"     , G_TYPE_INT    , ::gint(significant_bits_per_channel_sample), // depth may be smaller than width; e.g. width could be 24, and depth 20, leaving 4 unused bits.
		"signed"    , G_TYPE_BOOLEAN, ::gboolean(std::numeric_limits<output_sample_type>::min() < 0),
		"endianness", G_TYPE_INT    , ::gint(G_BYTE_ORDER),
		(void*)0
	);

	// create a capsfilter
	caps_filter_ = &instantiate("capsfilter", "caps-filter");

	// set caps on the capsfilter
	::g_object_set(G_OBJECT(caps_filter_), "caps", caps_, (void*)0);

	// create a fakesrc
	source_ = &instantiate("fakesrc", "src");

	// create a pipeline
	if(!(pipeline_ = ::gst_pipeline_new("pipeline"))) throw runtime_error("could not create new empty pipeline", UNIVERSALIS__COMPILER__LOCATION);
	
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

	unsigned int const period_size(static_cast<unsigned int>(playbackSettings().blockSamples() * bytes_per_sample));
	if(loggers::information()) {
		float const latency(float(playbackSettings().blockSamples()) / playbackSettings().samplesPerSec());
		std::ostringstream s;
		s <<
			"psycle: audiodrivers: gstreamer: "
			"period size: " << period_size << " bytes; "
			"periods: " << playbackSettings().blockCount() << "; "
			"total buffer size: " << playbackSettings().blockCount() * period_size << " bytes; "
			"latency: between " << latency << " and " << latency * playbackSettings().blockCount() << " seconds";
		loggers::information()(s.str());
	}

	// set properties of the fakesrc element
	::g_object_set(
		G_OBJECT(source_),
		"signal-handoffs", ::gboolean(true),
		"data"           , ::gint(/*FAKE_SRC_DATA_SUBBUFFER*/ 2), // data allocation method
		"parentsize"     , ::gint(period_size * playbackSettings().blockCount()),
		"sizemax"        , ::gint(period_size),
		//"num-buffers"    , ::gint(periods_),
		"sizetype"       , ::gint(/*FAKE_SRC_SIZETYPE_FIXED*/ 2), // fixed to sizemax
		"filltype"       , ::gint(/*FAKE_SRC_FILLTYPE_NOTHING*/ 1),
		(void*)0
	);

	// set the pipeline state to ready
	set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_READY);
}

void GStreamerOut::do_start() throw(std::exception) {
	// register our callback to the handoff signal of the fakesrc element
	if(!::g_signal_connect(G_OBJECT(source_), "handoff", G_CALLBACK(handoff_static), this)) throw runtime_error("could not connect handoff signal", UNIVERSALIS__COMPILER__LOCATION);
	// set the pipeline state to playing
	// Handoff is called before state is changed to playing.
	set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_PLAYING);
}

/// this is called from within gstreamer's processing thread.
void GStreamerOut::handoff_static(::GstElement * source, ::GstBuffer * buffer, ::GstPad * pad, GStreamerOut * instance) {
	instance->handoff(*buffer, *pad);
}

/// this is called from within gstreamer's processing thread.
void GStreamerOut::handoff(::GstBuffer & buffer, ::GstPad & pad) {
	if(false && loggers::trace()) loggers::trace()("handoff", UNIVERSALIS__COMPILER__LOCATION);
	output_sample_type * const out = reinterpret_cast<output_sample_type * const>(GST_BUFFER_DATA(&buffer));
	std::size_t const frames = GST_BUFFER_SIZE(&buffer) / sizeof(output_sample_type) / playbackSettings().numChannels();
	// The callback is unable to process more than MAX_SAMPLES_WORKFN samples at a time,
	// so we may have to call it several times to fill the output buffer.
	int chunk = MAX_SAMPLES_WORKFN, done = 0, remaining = frames;
	while(remaining) {
		if(remaining < chunk) chunk = remaining;
		float const * const in = callback(chunk);
		Quantize16WithDither(in, out + (done * 2), chunk); // * 2 since Quantize assumes a stereo signal.
		done += chunk;
		remaining = frames - done;
	}
}

void GStreamerOut::do_stop() throw(std::exception) {
	if(pipeline_) set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_READY);
}

void GStreamerOut::do_close() throw(std::exception) {
	if(pipeline_) {
		set_state_synchronously(*GST_ELEMENT(pipeline_), ::GST_STATE_NULL);
		::gst_object_unref(GST_OBJECT(pipeline_)); pipeline_ = 0;
	}
	sink_ = caps_filter_ = source_ = 0; caps_ = 0;

	{ // deinitialize gstreamer
		universalis::stdlib::call_once(global_client_count_init_once_flag, global_client_count_init);
		scoped_lock<mutex> lock(global_client_count_mutex);
		if(!--global_client_count) {
			#if 0 // gst_deinit must not be called because gst_init won't work afterwards
				if(loggers::trace()) loggers::trace()("psycle: audiodrivers: gstreamer: deinit", UNIVERSALIS__COMPILER__LOCATION);
				::gst_deinit();
			#else
				global_client_count = 1;
			#endif
		}
	}
}

GStreamerOut::~GStreamerOut() throw() {
	before_destruction();
}

}}

#endif // defined PSYCLE__GSTREAMER_AVAILABLE
