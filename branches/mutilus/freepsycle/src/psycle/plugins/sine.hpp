///\file
///\brief \interface psycle::plugins::sine - oscillator with a sinusoidal wave form
#pragma once
#include "plugin.hpp"
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__PLUGINS__SINE
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace plugins
	{
		/// oscillator with a sinusoidal wave form
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK sine : public engine::node
		{
			protected: friend class factory;
				sine(engine::plugin_library_reference &, engine::graph &, std::string const & name);
			public:
				void inline frequency(real const & frequency) { this->step_ = frequency * frequency_to_step_; }
			protected:
				void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES seconds_per_event_change_notification_from_port(engine::port const &);
				void UNIVERSALIS__COMPILER__VIRTUAL__OVERRIDES do_process() throw(engine::exception);
			private:
				real phase_;
				real step_;
				real frequency_to_step_;
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>