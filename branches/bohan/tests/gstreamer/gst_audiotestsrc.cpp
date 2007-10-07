#include <gst/gst.h>
#include <iostream>
#include <stdexcept>
#include <sstream>

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
	GstElement * audiosrc(gst_element_factory_make("audiotestsrc", "source"));
	GstElement * audiosink(gst_element_factory_make("gconfaudiosink", "audiosink"));

	gst_bin_add_many(GST_BIN(pipeline), audiosrc, audiosink, (void*)0);
	gst_element_link_many(audiosrc, audiosink, (void*)0);

	// play
	set_state_synchronously(*GST_ELEMENT(pipeline), GST_STATE_PLAYING);
	g_main_loop_run(loop);

	// clean up
	set_state_synchronously(*GST_ELEMENT(pipeline), GST_STATE_NULL);

	gst_object_unref(GST_OBJECT(pipeline));

	return 0;
}

