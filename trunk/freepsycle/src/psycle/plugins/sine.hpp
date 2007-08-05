///\file
///\brief \interface psycle::plugins::sine - oscillator with a sinusoidal wave form
#pragma once
#include "plugin.hpp"
namespace psycle
{
	namespace plugins
	{
		/// oscillator with a sinusoidal wave form
		class sine : public engine::node
		{
		public:
			sine(engine::plugin_library_reference &, engine::graph &, std::string const & name);
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
