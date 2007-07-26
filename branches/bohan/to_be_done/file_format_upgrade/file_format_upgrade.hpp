// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007 psycledelics http://psycle.sourceforge.net
///\file
///\interface psycle::file_format_upgrade
namespace psycle {
	namespace file_format_upgrade {
		typedef std::istream data_input;
		typedef std::ostream data_output;

		namespace version2_to_version3 {
			data_output upgrade(data_input const & original_data);
		}
			
		namespace version3_to_version4 {
			data_output upgrade(data_input const & original_data);
		}

		namespace detail {
			namespace version2 {
				// header PSY2SONG
				
				class root {
					public:
						void load(in)
						{
							detail::version3::root root;
							{
								char x[32];
								in.read(x, sizeof x);
								x[sizeof x - 1] = 0;
								root.name = x;
							}
							{
								char x[32];
								in.read(x, sizeof x);
								x[sizeof x - 1] = 0;
								root.author = x;
							}
							{
								char x[128];
								in.read(x, sizeof x);
								x[sizeof x - 1] = 0;
								root.comment = x;
							}
							
						/// note: *may* be null-terminated
						char name[32];
						/// note: *may* be null-terminated
						char author[32];
						/// note: *may* be null-terminated
						char comment[128];
						/// beats per minute
						std::int32_t bpm;
						/// lines per beat
						/// note: if < 0, set it to 4
						/// note: assumes we output at 44100 samples/sec, so lpm = 44100 * 60 / (sample_rate * bpm);
						std::int32_t lpm;
						/// octave. 440Hz A note is in octave number \todo which one?
						/// note: this is a gui setting, so it is not essential to preserve it.
						std::int8_t octave;
						/// bus machine
						std::uint8_t bus_machine[64];
						/// pattern play order sequence
						std::uint8_t sequence[128];
						/// sequence length
						std::int32_t sequence_length;
						/// number of tracks
						std::int32_t tracks;
						/// number of patterns
						std::int32_t patterns;
						
						// followed by pattern[patterns]
						
						/// note: this is a gui setting, so it is not essential to preserve it.
						std::int32_t instrument_selected;
						/// number of instruments
						int static const instruments = 255;
						/// names of instruments
						char instrument_name[32][instruments];
				};
				
				class pattern {
					public:
						/// number of lines in pattern
						std::int32 lines;
						/// note: *may* be null-terminated
						char name[32];
						
						// followed by pattern_line[lines]
				
				class pattern_line {
					public:
						/// pattern entries
						pattern_entry entries[32];
						
				};

				class pattern_entry {
				};
			}

			namespace version3 {
				// header PSY3SONG
				class root
				{
					public:
				};
				
			}

			namespace version4 {
				class root
				{
					public:
				};
			}
		}
	}
}
