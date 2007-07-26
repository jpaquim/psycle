// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007 psycledelics http://psycle.sourceforge.net
///\file
///\interface psycle::file_format_upgrade::detail::version3::instrument
namespace psycle {
	namespace file_format_upgrade {
		namespace detail {
			namespace version3 {
				class instrument {
					public:
						/// identifier of the instrument
						std::int32_t id;
					
						/// new note action
						///\verbatim
						/// values overview:
						/// 0 = Note Cut      [Fast Release 'Default']
						/// 1 = Note Release  [Release Stage]
						/// 2 = Note Continue [No NNA]
						///\endverbatim
						std::int8_t new_note_action;

						class env_type {
							public:
								/// in samples at 44.1kHz
								std::int32_t attack_time;
								/// in samples at 44.1kHz
								std::int32_t decay_time;
								/// in % [0, 100]
								std::int32_t sustain_level;
								/// in samples at 44.1kHz
								std::int32_t release_time;
						} amplitude_env;
						
						class filter_env_type : public env_type {
							public:
								/// [0, 127]
								std::int32_t cutoff_frequency;
								/// [0, 127]
								std::int32_t resonance;
								/// [-128, +128]
								std::int32_t amount;
								/// filter type [0, 4]
								std::int32_t type;
						} filter_env;

						/// ...
						std::int32_t panning;
						/// boolean (0 for false)
						std::int8_t r_panning;
						/// boolean (0 for false)
						std::int8_t r_cutoff;
						/// boolean (0 for false)
						std::int8_t r_resonance;
				};
			}
		}
	}
}
