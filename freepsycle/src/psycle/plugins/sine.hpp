// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#pragma once
#include "plugin.hpp"
#define PSYCLE__DECL  PSYCLE__PLUGINS__SINE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

/// oscillator with a sinusoidal wave form
class PSYCLE__DECL sine : public node {

	public:
		sine(class plugin_library_reference &, name_type const &);

		void freq(real const & freq) { step_ = std::abs(freq) * freq_to_step_; if(freq < 0) phase_ = math::pi - phase_; }
		real freq() const { return freq_to_step_ ? step_ / freq_to_step_ : 0; }

		void amp(real const & amp) { amp_ = amp; }
		real amp() const { return amp_; }

	protected:
		void seconds_per_event_change_notification_from_port(port const &) override;
		void do_process() override;

	private:
		#define PSYCLE__TODO__IDEAL 1
		#if PSYCLE__TODO__IDEAL
			// Note: not ideal
			template<
				buffer::flags phase_flag,
				buffer::flags  freq_flag,
				buffer::flags   amp_flag
			>
			void do_process_template(); friend class node;
		#else
			// Note: ideal
			template<bool have_phase, bool have_freq, bool have_amp>
			void do_process_template(
				std::size_t out_begin, std::size_t out_end,
				std::size_t phase_begin, std::size_t freq_begin, std::size_t amp_begin
			);

			template<int Ports>
			void do_process_split(ports::inputs::single * ports[Ports]);

			// a variant of what's declared in engine::node
			template<bool... Evaluated_Bools, typename Node>
			static void do_process_template_switch_x(std::size_t begin, std::size_t end, Node & node) {
				node.template do_process_template<Evaluated_Bools...>(begin, end);
			}

			// a variant of what's declared in engine::node
			template<bool... Evaluated_Bools, typename Node, typename... Bools_To_Evaluate>
			static void do_process_template_switch_x(std::size_t begin, std::size_t end, Node & node, bool bool_to_evaluate, Bools_To_Evaluate... bools_to_evaluate) {
				if(bool_to_evaluate)
					do_process_template_switch_x<Evaluated_Bools..., true>(begin, end, node, bools_to_evaluate...);
				else
					do_process_template_switch_x<Evaluated_Bools..., false>(begin, end, node, bools_to_evaluate...);
			}

			// a variant of what's declared in engine::node
			template<bool... Evaluated_Bools, typename Node, typename... Begins>
			static void do_process_template_switch_xx(std::size_t begin, std::size_t end, Node & node, Begins... begins) {
				node.template do_process_template<Evaluated_Bools...>(begin, end, begins...);
			}

			// a variant of what's declared in engine::node
			template<bool... Evaluated_Bools, typename Node, typename... Bools_To_Evaluate>
			static void do_process_template_switch_xx(std::size_t begin, std::size_t end, Node & node, bool bool_to_evaluate, std::size_t input_begin, Bools_To_Evaluate... bools_to_evaluate) {
				if(bool_to_evaluate)
					do_process_template_switch_xx<Evaluated_Bools..., true>(begin, end, node, input_begin, bools_to_evaluate...);
				else
					do_process_template_switch_xx<Evaluated_Bools..., false>(begin, end, node, input_begin, bools_to_evaluate...);
			}
		#endif

		real phase_, step_, freq_to_step_, amp_;
		ports::output out_port_;
		ports::inputs::single phase_port_, freq_port_, amp_port_;
};

}}
#include <psycle/detail/decl.hpp>
