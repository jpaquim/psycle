#include <psycle/core/psycleCorePch.hpp>

#include "convert_internal_machines.private.hpp"
#include "helpers/scale.hpp"
#include "helpers/math/pi.hpp"
#include "machine.h"
#include "player.h"
#include "song.h"

namespace psy {
	namespace core {
		namespace convert_internal_machines {
			namespace math = common::math;
			typedef common::Scale::Real Real;

			Converter::Converter(std::string const & plugin_path)
			:
				plugin_path_(plugin_path)
			{}
			
			Converter::~Converter() throw()
			{
				for(std::map<Machine * const, const int *>::const_iterator i = machine_converted_from.begin() ; i != machine_converted_from.end() ; ++i) delete const_cast<int *>(i->second);
			}
			
			Machine & Converter::redirect(const int & index, const int & type, RiffFile & riff, CoreSong & song)
			{
				Plugin & plugin = * new Plugin(Player::Instance(), index, &song);
				Machine * pointer_to_machine = &plugin;
				try {
					if(!plugin.LoadDll(plugin_path_, const_cast<char *>((plugin_names()(type) + ".dll").c_str()))) {
						pointer_to_machine = 0; // for delete pointer_to_machine in the catch clause
						delete & plugin;
						pointer_to_machine = new Dummy(Player::Instance(), index, &song);
					}
					Machine & machine = *pointer_to_machine;
					machine_converted_from[&machine] = new int(type);
					machine.Init();
					//BOOST_STATIC_ASSERT(sizeof(int) == 4);
					{
						char c[16];
						riff.ReadChunk(c, 16); c[15] = 0;
						machine.SetEditName(c);
					}
					riff.Read(machine._inputMachines);
					riff.Read(machine._outputMachines);
					riff.Read(machine._inputConVol);
					riff.Read(machine._connection);
					riff.Read(machine._inputCon);
					riff.Skip(96);  // ConnectionPoints, 12*8bytes
					riff.Read(machine._connectedInputs);
					riff.Read(machine._connectedOutputs);
					riff.Read(machine._panning);
					machine.SetPan(machine._panning);
					riff.Skip(40); // skips shiatz
					switch(type) {
						case delay:
							{
								int parameters [2]; riff.ReadChunk(parameters, sizeof parameters);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters, 5);
							}
							break;
						case flanger:
							{
							int parameters [2]; riff.ReadChunk(parameters, sizeof parameters);
							retweak(machine, type, parameters, sizeof parameters / sizeof *parameters, 7);
							}
							break;
						case gainer:
							riff.Skip(4);
							{
								int parameters [1]; riff.ReadChunk(parameters, sizeof parameters);
								if(type == gainer) retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							}
							break;
						default:
							riff.Skip(8);
					}
					switch(type) {
						case distortion:
							int parameters [4]; riff.ReadChunk(parameters, sizeof parameters);
							retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							break;
						default:
							riff.Skip(16);
					}
					switch(type) {
						case ring_modulator:
							{
								unsigned char parameters [4];
								riff.Read(parameters[0]);
								riff.Read(parameters[1]);
								riff.Skip(1);
								riff.Read(parameters[2]);
								riff.Read(parameters[3]);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							}
							riff.Skip(40);
							break;
						case delay:
							riff.Skip(5);
							{
								int parameters [4];
								riff.Read(parameters[0]);
								riff.Read(parameters[2]);
								riff.Read(parameters[1]);
								riff.Read(parameters[3]);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							}
							riff.Skip(24);
							break;
						case flanger:
							riff.Skip(4);
							{
								unsigned char parameters [1]; riff.ReadChunk(parameters, sizeof parameters);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters, 9);
							}
							{
								int parameters [6];
								riff.Read(parameters[0]);
								riff.Skip(4);
								riff.Read(parameters[3]);
								riff.Read(parameters[5]);
								riff.Skip(8);
								riff.Read(parameters[2]);
								riff.Read(parameters[1]);
								riff.Read(parameters[4]);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							}
							riff.Skip(4);
							break;
						case filter_2_poles:
							riff.Skip(21);
							{
								int parameters [6];
								riff.ReadChunk(&parameters[1], sizeof parameters - sizeof *parameters);
								riff.Read(parameters[0]);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							}
							break;
						default:
							riff.Skip(45);
					}
					return machine;
				}
				catch(...) {
					delete pointer_to_machine;
					throw;
				}
			}

			void Converter::retweak(CoreSong & song) const {
				///\todo this code is really needed to properly load the pattern data
				#if 0
				/// \todo must each twk repeat the machine number ?
				// int previous_machines [MAX_TRACKS]; for(int i = 0 ; i < MAX_TRACKS ; ++i) previous_machines[i] = 255;
				for(int pattern(0) ; pattern < MAX_PATTERNS ; ++pattern)
				{
					if(!song.IsPatternUsed(pattern)) continue;
					PatternEntry * const lines(reinterpret_cast<PatternEntry*>(song.ppPatternData[pattern]));
					for(int line = 0 ; line < song.patternLines[pattern] ; ++line)
					{
						PatternEntry * const events(lines + line * MAX_TRACKS);
						for(int track(0); track < song.tracks() ; ++track)
						{
							PatternEntry & event(events[track]);
							if(event._note == psy::core::commands::tweak_effect)
							{
								event._mach += 0x40;
								event._note = psy::core::commands::tweak;
							}
							if(event._note == psy::core::commands::tweak)
							{
								std::map<Machine * const, const int *>::const_iterator i(machine_converted_from.find(song._pMachine[event._mach]));
								if(i != machine_converted_from.end())
								{
									//Machine & machine(*i->first);
									const int & type(*i->second);
									int parameter(event._inst);
									int value((event._cmd << 8) + event._parameter);
									retweak(type, parameter, value);
									event._inst = parameter;
									event._cmd = value >> 8; event._parameter = 0xff & value;
								}
							}
						}
					}
				}
				#endif
			}

			Converter::Plugin_Names::Plugin_Names()
			{
				(*this)[ring_modulator] = new std::string("ring_modulator");
				(*this)[distortion] = new std::string("distortion");
				(*this)[delay] = new std::string("delay");
				(*this)[filter_2_poles] = new std::string("filter_2_poles");
				(*this)[gainer] = new std::string("gainer");
				(*this)[flanger] = new std::string("flanger");
			}

			Converter::Plugin_Names::~Plugin_Names()
			{
				delete (*this)[ring_modulator];
				delete (*this)[distortion];
				delete (*this)[delay];
				delete (*this)[filter_2_poles];
				delete (*this)[gainer];
				delete (*this)[flanger];
			}

			const bool Converter::Plugin_Names::exists(const int & type) const throw()
			{
				return find(type) != end();
			}

			const std::string & Converter::Plugin_Names::operator()(const int & type) const
			{
				const_iterator i = find(type);
				//if(i == end()) throw std::exception("internal machine replacement plugin not declared");
				if(i == end()) throw;
				return *i->second;
			}

			const Converter::Plugin_Names & Converter::plugin_names()
			{
				static const Plugin_Names plugin_names;
				return plugin_names;
			}

			template<typename Parameter>
			void Converter::retweak(Machine & machine, const int & type, Parameter parameters [], const int & parameter_count, const int & parameter_offset)
			{
				for(int parameter(0) ; parameter < parameter_count ; ++parameter)
				{
					int new_parameter(parameter_offset + parameter);
					int new_value(parameters[parameter]);
					retweak(type, new_parameter, new_value);
					machine.SetParameter(new_parameter, new_value);
				}
			}

			void Converter::retweak(const int & type, int & parameter, int & integral_value) const
			{
				Real value(integral_value);
				const Real maximum(0xffff);
				switch(type)
				{
					case gainer:
						{
							enum Parameters { gain };
							static const int parameters [] = { gain };
							parameter = parameters[--parameter];
							switch(parameter)
							{
								case gain:
								if ( value < 1.0f) value = 0;
								else value = common::scale::Exponential(maximum, exp(-4.), exp(+4.)).apply_inverse(value / 0x100);
									break;
							}
						}
						break;
					case distortion:
						{
							enum Parameters { input_gain, output_gain, positive_threshold, positive_clamp, negative_threshold, negative_clamp, symmetric };
							static const int parameters [] = { positive_threshold, positive_clamp, negative_threshold, negative_clamp };
							parameter = parameters[--parameter];
							switch(parameter)
							{
								case negative_threshold:
								case negative_clamp:
								case positive_threshold:
								case positive_clamp:
									value *= maximum / 0x100;
									break;
							}
						}
						break;
					case delay:
						{
							enum Parameters { dry, wet, left_delay, left_feedback, right_delay, right_feedback };
							static const int parameters [] = { left_delay, left_feedback, right_delay, right_feedback, dry, wet };
							parameter = parameters[--parameter];
							switch(parameter)
							{
								case left_delay:
								case right_delay:
									value *= Real(2 * 3 * 4 * 5 * 7) / Player::Instance()->timeInfo().samplesPerRow();
									break;
								case left_feedback:
								case right_feedback:
									value = (100 + value) * maximum / 200;
									break;
								case dry:
								case wet:
									value = (0x100 + value) * maximum / 0x200;
									break;
							}
						}
						break;
					case flanger:
						{
							enum Parameters { delay, modulation_amplitude, modulation_radians_per_second, modulation_stereo_dephase, interpolation, dry, wet, left_feedback, right_feedback };
							static const int parameters [] = { delay, modulation_amplitude, modulation_radians_per_second, left_feedback, modulation_stereo_dephase, right_feedback, dry, wet, interpolation };
							parameter = parameters[--parameter];
							switch(parameter)
							{
								case delay:
									value *= maximum / 0.1 / Player::Instance()->timeInfo().sampleRate();
									break;
								case modulation_amplitude:
								case modulation_stereo_dephase:
									value *= maximum / 0x100;
									break;
								case modulation_radians_per_second:
								if ( value < 1.0f) value = 0;
								else value = common::scale::Exponential(maximum, 0.0001 * common::math::pi * 2, 100 * common::math::pi * 2).apply_inverse(value * 3e-9 * Player::Instance()->timeInfo().sampleRate());
									break;
								case left_feedback:
								case right_feedback:
									value = (100 + value) * maximum / 200;
									break;
								case dry:
								case wet:
									value = (0x100 + value) * maximum / 0x200;
									break;
								case interpolation:
									value = value != 0;
									break;
								}
						}
						break;
					case filter_2_poles:
						{
							enum Parameters { response, cutoff_frequency, resonance, modulation_sequencer_ticks, modulation_amplitude, modulation_stereo_dephase };
							static const int parameters [] = { response, cutoff_frequency, resonance, modulation_sequencer_ticks, modulation_amplitude, modulation_stereo_dephase };
							parameter = parameters[--parameter];
							switch(parameter)
							{
								case cutoff_frequency:
								if ( value < 1.0f) value = 0;
								else value = common::scale::Exponential(maximum, 15 * math::pi, 22050 * math::pi).apply_inverse(std::asin(value / 0x100) * Player::Instance()->timeInfo().sampleRate());
									break;
								case modulation_sequencer_ticks:
								if ( value < 1.0f) value = 0;
								else value = common::scale::Exponential(maximum, math::pi * 2 / 10000, math::pi * 2 * 2 * 3 * 4 * 5 * 7).apply_inverse(value * 3e-8 * Player::Instance()->timeInfo().samplesPerRow());
									break;
								case resonance:
								case modulation_amplitude:
								case modulation_stereo_dephase:
									value *= maximum / 0x100;
									break;
							}
						}
						break;
					case ring_modulator:
						{
							enum Parameters { am_radians_per_second, am_glide, fm_radians_per_second, fm_bandwidth };
							static const int parameters [] = { am_radians_per_second, am_glide, fm_radians_per_second, fm_bandwidth };
							parameter = parameters[--parameter];
							switch(parameter)
							{
								case am_radians_per_second:
								if ( value < 1.0f) value = 0;
								else value = common::scale::Exponential(maximum, 0.0001 * math::pi * 2, 22050 * math::pi * 2).apply_inverse(value * 2.5e-3 * Player::Instance()->timeInfo().sampleRate());
									break;
								case am_glide:
								if ( value < 1.0f) value = 0;
								else value = common::scale::Exponential(maximum, 0.0001 * math::pi * 2, 15 * 22050 * math::pi * 2).apply_inverse(value * 5e-6 * Player::Instance()->timeInfo().sampleRate()) * Player::Instance()->timeInfo().sampleRate();
									break;
								case fm_radians_per_second:
								if ( value < 1.0f) value = 0;
								else value = common::scale::Exponential(maximum, 0.0001 * math::pi * 2, 100 * math::pi * 2).apply_inverse(value * 2.5e-5 * Player::Instance()->timeInfo().sampleRate());
									break;
								case fm_bandwidth:
								if ( value < 1.0f) value = 0;
								else value = common::scale::Exponential(maximum, 0.0001 * math::pi * 2, 22050 * math::pi * 2).apply_inverse(value * 5e-4 * Player::Instance()->timeInfo().sampleRate());
									break;
							}
						}
						break;
				}
				integral_value = std::floor(value + Real(0.5));
			}
		}
	}
}
