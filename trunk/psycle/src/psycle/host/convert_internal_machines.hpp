#pragma once
#include <string>
#include <exception>
#include <map>
#include <psycle/helpers/scale.hpp>
#include "internal_machines.hpp"
namespace psycle
{
	namespace host
	{
		namespace convert_internal_machines
		{
			class Converter
			{
			public:
				enum Type
				{
					master,
					ring_modulator, distortion, sampler, delay, filter_2_poles, gainer, flanger,
					plugin,
					vsti, vstfx,
					scope,
					dummy = 255
				};

				virtual ~Converter() throw()
				{
					for(std::map<Machine * const, const int *>::const_iterator i = machine_converted_from.begin() ; i != machine_converted_from.end() ; ++i) delete const_cast<int *>(i->second);
				}

				Machine & redirect(const int & index, const int & type, RiffFile & riff) throw(std::exception)
				{
					Plugin & plugin = * new Plugin(index);
					Machine * pointer_to_machine = &plugin;
					try
					{
						if(!plugin.LoadDll(const_cast<char *>((plugin_names()(type) + ".dll").c_str())))
						{
							pointer_to_machine = 0; // for delete pointer_to_machine in the catch clause
							delete & plugin;
							pointer_to_machine = new Dummy(index);
						}
						Machine & machine = *pointer_to_machine;
						machine_converted_from[&machine] = new int(type);
						machine.Init();
						assert(sizeof(int) == 4);
						riff.Read(machine._editName, 16); /* sizeof machine._editName); */ machine._editName[16] = 0;
						riff.Read(machine._inputMachines, sizeof machine._inputMachines);
						riff.Read(machine._outputMachines, sizeof machine._outputMachines);
						riff.Read(machine._inputConVol, sizeof machine._inputConVol);
						riff.Read(machine._connection, sizeof machine._connection);
						riff.Read(machine._inputCon, sizeof machine._inputCon);
						#if defined (_WINAMP_PLUGIN_)
							riff.Skip(96) ; // sizeof(CPoint) == 8
						#else
							riff.Read(machine._connectionPoint, sizeof machine._connectionPoint);
						#endif
						riff.Read(&machine._numInputs, sizeof machine._numInputs);
						riff.Read(&machine._numOutputs, sizeof machine._numOutputs);
						riff.Read(&machine._panning, sizeof machine._panning);
						machine.SetPan(machine._panning);
						riff.Skip(40); // skips shiatz
						switch(type)
						{
						case delay:
							{
								int parameters [2]; riff.Read(parameters, sizeof parameters);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters, 5);
							}
							break;
						case flanger:
							{
								int parameters [2]; riff.Read(parameters, sizeof parameters);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters, 7);
							}
							break;
						case gainer:
							riff.Skip(sizeof(int));
							{
								int parameters [1]; riff.Read(parameters, sizeof parameters);
								if(type == gainer) retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							}
							break;
						default:
							riff.Skip(2 * sizeof(int));
						}
						switch(type)
						{
						case distortion:
							int parameters [4]; riff.Read(parameters, sizeof parameters);
							retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							break;
						default:
							riff.Skip(4 * sizeof(int));
						}
						switch(type)
						{
						case ring_modulator:
							{
								unsigned char parameters [4];
								riff.Read(&parameters[0], 2 * sizeof *parameters);
								riff.Skip(sizeof(char));
								riff.Read(&parameters[2], 2 * sizeof *parameters);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							}
							riff.Skip(40);
							break;
						case delay:
							riff.Skip(5);
							{
								int parameters [4];
								riff.Read(&parameters[0], sizeof *parameters);
								riff.Read(&parameters[2], sizeof *parameters);
								riff.Read(&parameters[1], sizeof *parameters);
								riff.Read(&parameters[3], sizeof *parameters);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							}
							riff.Skip(24);
							break;
						case flanger:
							riff.Skip(4);
							{
								unsigned char parameters [1]; riff.Read(parameters, sizeof parameters);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters, 9);
							}
							{
								int parameters [6];
								riff.Read(&parameters[0], sizeof *parameters);
								riff.Skip(4);
								riff.Read(&parameters[3], sizeof *parameters);
								riff.Read(&parameters[5], sizeof *parameters);
								riff.Skip(8);
								riff.Read(&parameters[2], sizeof *parameters);
								riff.Read(&parameters[1], sizeof *parameters);
								riff.Read(&parameters[4], sizeof *parameters);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							}
							riff.Skip(4);
							break;
						case filter_2_poles:
							riff.Skip(21);
							{
								int parameters [6];
								riff.Read(&parameters[1], sizeof parameters - sizeof *parameters);
								riff.Read(&parameters[0], sizeof *parameters);
								retweak(machine, type, parameters, sizeof parameters / sizeof *parameters);
							}
							break;
						default:
							riff.Skip(45);
						}
						return machine;
					}
					catch(...)
					{
						delete pointer_to_machine;
						throw;
					}
				}

				void retweak(Song & song) const
				{
					/// \todo must each twk repeat the machine number ?
					// int previous_machines [MAX_TRACKS]; for(int i = 0 ; i < MAX_TRACKS ; ++i) previous_machines[i] = 255;
					for(int pattern(0) ; pattern < MAX_PATTERNS ; ++pattern)
					{
						if(!song.ppPatternData[pattern]) continue;
						PatternEntry * const lines(reinterpret_cast<PatternEntry*>(song.ppPatternData[pattern]));
						for(int line = 0 ; line < song.patternLines[pattern] ; ++line)
						{
							PatternEntry * const events(lines + line * MAX_TRACKS);
							for(int track(0); track < song.SONGTRACKS ; ++track)
							{
								PatternEntry & event(events[track]);
								if(event._note == notecommands::tweakeffect)
								{
									event._mach += 0x40;
									event._note = notecommands::tweak;
								}
								if(event._note == notecommands::tweak)
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
				}

			private:
				class Plugin_Names : private std::map<const int, const std::string *>
				{
				public:
					Plugin_Names()
					{
						(*this)[ring_modulator] = new std::string("ring_modulator");
						(*this)[distortion] = new std::string("distortion");
						(*this)[delay] = new std::string("delay");
						(*this)[filter_2_poles] = new std::string("filter_2_poles");
						(*this)[gainer] = new std::string("gainer");
						(*this)[flanger] = new std::string("flanger");
					}
					~Plugin_Names()
					{
						delete (*this)[ring_modulator];
						delete (*this)[distortion];
						delete (*this)[delay];
						delete (*this)[filter_2_poles];
						delete (*this)[gainer];
						delete (*this)[flanger];
					}
					const bool exists(const int & type) const throw()
					{
						return find(type) != end();
					}
					const std::string & operator()(const int & type) const throw(std::exception)
					{
						const_iterator i = find(type);
						if(i == end()) throw std::exception("internal machine replacement plugin not declared");
						return *i->second;
					}
				};

			public:
				static const Plugin_Names & plugin_names()
				{
					static const Plugin_Names plugin_names;
					return plugin_names;
				}

			private:
				std::map<Machine * const, const int *> machine_converted_from;

				template<typename Parameter> void retweak(Machine & machine, const int & type, Parameter parameters [], const int & parameter_count, const int & parameter_offset = 1)
				{
					for(int parameter(0) ; parameter < parameter_count ; ++parameter)
					{
						int new_parameter(parameter_offset + parameter);
						int new_value(parameters[parameter]);
						retweak(type, new_parameter, new_value);
						machine.SetParameter(new_parameter, new_value);
					}
				}

				void retweak(const int & type, int & parameter, int & integral_value) const
				{
					typedef double Real;
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
								else value = scale::Exponential(maximum, exp(-4.), exp(+4.)).apply_inverse(value / 0x100);
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
								value *= Real(2 * 3 * 4 * 5 * 7) / Global::pPlayer->SamplesPerRow();
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
								value *= maximum / 0.1 / Global::pConfig->GetSamplesPerSec();
								break;
							case modulation_amplitude:
							case modulation_stereo_dephase:
								value *= maximum / 0x100;
								break;
							case modulation_radians_per_second:
								if ( value < 1.0f) value = 0;
								else value = scale::Exponential(maximum, 0.0001 * math::pi * 2, 100 * math::pi * 2).apply_inverse(value * 3e-9 * Global::pConfig->GetSamplesPerSec());
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
								else value = scale::Exponential(maximum, 15 * math::pi, 22050 * math::pi).apply_inverse(std::asin(value / 0x100) * Global::pConfig->GetSamplesPerSec());
								break;
							case modulation_sequencer_ticks:
								if ( value < 1.0f) value = 0;
								else value = scale::Exponential(maximum, math::pi * 2 / 10000, math::pi * 2 * 2 * 3 * 4 * 5 * 7).apply_inverse(value * 3e-8 * Global::pPlayer->SamplesPerRow());
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
								else value = scale::Exponential(maximum, 0.0001 * math::pi * 2, 22050 * math::pi * 2).apply_inverse(value * 2.5e-3 * Global::pConfig->GetSamplesPerSec());
								break;
							case am_glide:
								if ( value < 1.0f) value = 0;
								else value = scale::Exponential(maximum, 0.0001 * math::pi * 2, 15 * 22050 * math::pi * 2).apply_inverse(value * 5e-6 * Global::pConfig->GetSamplesPerSec() * Global::pConfig->GetSamplesPerSec());
								break;
							case fm_radians_per_second:
								if ( value < 1.0f) value = 0;
								else value = scale::Exponential(maximum, 0.0001 * math::pi * 2, 100 * math::pi * 2).apply_inverse(value * 2.5e-5 * Global::pConfig->GetSamplesPerSec());
								break;
							case fm_bandwidth:
								if ( value < 1.0f) value = 0;
								else value = scale::Exponential(maximum, 0.0001 * math::pi * 2, 22050 * math::pi * 2).apply_inverse(value * 5e-4 * Global::pConfig->GetSamplesPerSec());
								break;
							}
						}
						break;
					}
					integral_value = std::floor(value + Real(0.5));
				}
			};
		}
	}
}
