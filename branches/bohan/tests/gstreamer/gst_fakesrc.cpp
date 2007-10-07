#include <gst/gst.h>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdint.h>
#include <stdexcept>
#include <sstream>

typedef int16_t sample;
unsigned int const sample_rate(44100);

class sine {
	public:
		float freq, phase;
		sine(float freq, float phase = 0): freq(freq), phase(phase) {}
};

static void cb_handoff (
	GstElement * fakesrc,
    GstBuffer * buffer,
    GstPad * pad,
    sine & s
) {
	//static float tt(0);
	//tt += float(GST_BUFFER_SIZE(buffer) / sizeof(sample)) / sample_rate;
	//std::cout <<  "x " << tt << "\n";
	sample * samples(reinterpret_cast<sample*>(GST_BUFFER_DATA(buffer)));
	for(unsigned int i(0); i < GST_BUFFER_SIZE(buffer) / sizeof *samples; ++i) {
		samples[i] = std::numeric_limits<sample>::max() * std::sin(s.phase);
		s.phase += s.freq * 2 * M_PI / sample_rate;
		s.freq *= 1.00001;
	}
	while(s.phase > 2 * M_PI) s.phase -= 2 * M_PI;
}

::GstClockTime const default_timeout_nanoseconds(static_cast<GstClockTime>(5e9));

void wait_for_state(::GstElement & element, ::GstState state_wanted, ::GstClockTime timeout_nanoseconds = default_timeout_nanoseconds) throw(std::exception) {
	::GstClockTime intermediate_timeout_nanoseconds(std::min(timeout_nanoseconds, static_cast<GstClockTime>(0.1e9)));
	::GstClockTime total_nanoseconds_waited(0);
	for(;;) {
		::GstState current_state, pending_state;
		::GstStateChangeReturn result(::gst_element_get_state(&element, &current_state, &pending_state, intermediate_timeout_nanoseconds));
		switch(result) {
			case ::GST_STATE_CHANGE_NO_PREROLL:
				std::clog << "no preroll\n";
			case ::GST_STATE_CHANGE_SUCCESS:
				if(current_state == state_wanted) {
					std::clog << "Ok\n";
					return;
				}
				else {
					std::ostringstream s;
						s
							<< "unexpected current state on element: " << ::gst_element_get_name(&element)
							<< "; current state: " << ::gst_element_state_get_name(current_state)
							<< ", expected state: " << ::gst_element_state_get_name(state_wanted)
							<< '\n';
					throw std::runtime_error(s.str());
				}
			case ::GST_STATE_CHANGE_ASYNC:
				if(pending_state != state_wanted) {
					std::ostringstream s;
						s
							<< "unexpected pending state on element: " << ::gst_element_get_name(&element)
							<< "; pending state: " << ::gst_element_state_get_name(pending_state)
							<< ", expected state: " << ::gst_element_state_get_name(state_wanted)
							<< '\n';
					throw std::runtime_error(s.str());
				}
				total_nanoseconds_waited += intermediate_timeout_nanoseconds;
				if(total_nanoseconds_waited > timeout_nanoseconds) {
					std::ostringstream s;
						s
							<< "timeout while waiting for state change on element: " << ::gst_element_get_name(&element)
							<< "; pending state: " << ::gst_element_state_get_name(pending_state)
							<< '\n';
					throw std::runtime_error(s.str());
				}
				{
					std::ostringstream s;
					s
						<< "waiting for element " << ::gst_element_get_name(&element)
						<< " to asynchronously change its state from " << ::gst_element_state_get_name(current_state)
						<< " to " << ::gst_element_state_get_name(state_wanted)
						<< " (waited a total of " << total_nanoseconds_waited * 1e-9
						<< " seconds ; will timeout after " << timeout_nanoseconds * 1e-9 << " seconds)"
						<< '\n';
					std::clog << s.str();
				}
				intermediate_timeout_nanoseconds *= 2; // makes timeouts progressively sparser
				continue;
			case ::GST_STATE_CHANGE_FAILURE: {
					std::ostringstream s;
						s
							<< "state change failure on element: " << ::gst_element_get_name(&element)
							<< "; state change: " << result
							<< ", current state: " << ::gst_element_state_get_name(current_state)
							<< ", pending state: " << ::gst_element_state_get_name(pending_state)
							<< '\n';
					throw std::runtime_error(s.str());
				}
			default: {
					std::ostringstream s;
						s
							<< "unknown state change on element: " << ::gst_element_get_name(&element)
							<< "; state change: " << result
							<< ", current state: " << current_state // don't use ::gst_element_state_get_name in unknown situation
							<< ", pending state: " << pending_state // don't use ::gst_element_state_get_name in unknown situation
							<< '\n';
					throw std::runtime_error(s.str());
			}
		}
	}
}

void set_state_synchronously(::GstElement & element, ::GstState state, ::GstClockTime timeout_nanoseconds = default_timeout_nanoseconds) throw(std::exception) {
	::gst_element_set_state(&element, state);
	wait_for_state(element, state, timeout_nanoseconds);
}

int main(int argc, char * argv[]) {
	// init gstreamer
	gst_init(&argc, &argv);
	
	GMainLoop * loop(g_main_loop_new(0, false));

	// setup pipeline
	GstElement * pipeline(gst_pipeline_new("pipeline"));
	GstElement * fakesrc(gst_element_factory_make("fakesrc", "source"));
	GstElement * queue(gst_element_factory_make("queue", "queue"));
	GstElement * capsfilter(gst_element_factory_make("capsfilter", "filter"));
	GstElement * audiosink(gst_element_factory_make("gconfaudiosink", "audiosink"));

	// setup caps
	GstCaps * caps(gst_caps_new_simple(
		"audio/x-raw-int",
		"width", G_TYPE_INT, gint(sizeof(sample) * 8),
		"depth", G_TYPE_INT, gint(sizeof(sample) * 8),
		"rate", G_TYPE_INT, gint(sample_rate),
		"channels", G_TYPE_INT, gint(1),
		"signed", G_TYPE_BOOLEAN, gboolean(std::numeric_limits<sample>::min() < 0),
		"endianness", G_TYPE_INT, gint(G_BYTE_ORDER),
		(void*)0
	));
	g_object_set(G_OBJECT(capsfilter), "caps", caps, (void*)0);

	gst_bin_add_many(GST_BIN(pipeline), fakesrc, queue, capsfilter, audiosink, (void*)0);
	gst_element_link_many(fakesrc, queue, capsfilter, audiosink, (void*)0);
	//gst_element_link_many(fakesrc, capsfilter, queue, audiosink, (void*)0);

	// setup fake source
	int const s(4096);
	g_object_set(G_OBJECT(fakesrc),
		"signal-handoffs", gboolean(true),
		"data", gint(/*FAKE_SRC_DATA_SUBBUFFER*/ 2), // data allocation method
		"parentsize", gint(4 * s),
		"sizemax", gint(s),
		"sizetype", gint(/*FAKE_SRC_SIZETYPE_FIXED*/ 2), // fixed to sizemax
		"filltype", gint(/*FAKE_SRC_FILLTYPE_NOTHING*/ 1),
		(void*)0
	);
	
	sine sine(100);
	
	g_signal_connect(fakesrc, "handoff", G_CALLBACK(cb_handoff), &sine);

	// play
	set_state_synchronously(*GST_ELEMENT(pipeline), GST_STATE_PLAYING);
	g_main_loop_run(loop);

	// clean up
	set_state_synchronously(*GST_ELEMENT(pipeline), GST_STATE_NULL);
	gst_object_unref(GST_OBJECT(pipeline));

	return 0;
}

