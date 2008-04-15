// -*- mode:c++; indent-tabs-mode:t -*-
//#include <psycle/core/psycleCorePch.hpp>
#include "convert_internal_machines.private.hpp"
#include "machinefactory.h"
#include "plugin.h"
#include "helpers/scale.hpp"
#include "helpers/math/pi.hpp"
#include "player.h"
#include "song.h"
#include "fileio.h"

namespace psy {
	namespace core {
		namespace convert_internal_machines {
			namespace math = common::math;
			typedef common::Scale::Real Real;

			Converter::Converter()
			{}
			
			Converter::~Converter() throw()
			{
				for(std::map<Machine * const, const int *>::const_iterator i = machine_converted_from.begin() ; i != machine_converted_from.end() ; ++i) delete const_cast<int *>(i->second);
			}
			
			Machine & Converter::redirect(MachineFactory& factory, const int & index, const int& type, RiffFile & riff)
			{
				Machine * pointer_to_machine = factory.CreateMachine(MachineKey(Hosts::NATIVE,(plugin_names()(type).c_str()),0),index);
				if (!pointer_to_machine){
					pointer_to_machine = factory.CreateMachine(MachineKey::dummy(),index);
				}
				try {
					Machine & machine = *pointer_to_machine;
					machine_converted_from[&machine] = new int(type);
					machine.Init();
					//BOOST_STATIC_ASSERT(sizeof(int) == 4);
					{
						char c[16];
						riff.ReadArray(c, 16); c[15] = 0;
						machine.SetEditName(c);
					}
					riff.ReadArray(machine._inputMachines,MAX_CONNECTIONS);
					riff.ReadArray(machine._outputMachines,MAX_CONNECTIONS);
					riff.ReadArray(machine._inputConVol,MAX_CONNECTIONS);
					riff.ReadArray(machine._connection,MAX_CONNECTIONS);
					riff.ReadArray(machine._inputCon,MAX_CONNECTIONS);
					riff.Skip(96);  // ConnectionPoints, 12*8bytes
					riff.Read(machine._connectedInputs);
					riff.Read(machine._connectedOutputs);
					std::int32_t panning;
					riff.Read(panning);
					machine.SetPan(panning);
					riff.Skip(40); // skips shiatz
					switch(type) {
						case delay:
							{
								const int nparams = 2;
								std::int32_t parameters [nparams]; riff.ReadArray(parameters,nparams);
								retweak(machine, type, parameters, nparams, 5);
							}
							break;
						case flanger:
							{
								const int nparams = 2;
								std::int32_t parameters [nparams]; riff.ReadArray(parameters, nparams);
								retweak(machine, type, parameters, nparams, 7);
							}
							break;
						case gainer:
							riff.Skip(4);
							{
								const int nparams = 1;
								std::int32_t parameters [nparams]; riff.ReadArray(parameters, nparams);
								/*if(type == gainer)*/ retweak(machine, type, parameters, nparams);
							}
							break;
						default:
							riff.Skip(8);
					}
					switch(type) {
						case distortion:
						{
							const int nparams=4;
							std::int32_t parameters [nparams]; riff.ReadArray(parameters, nparams);
							retweak(machine, type, parameters, nparams);
							break;
						}
						default:
							riff.Skip(16);
					}
					switch(type) {
						case ring_modulator:
							{
								const int nparams=4;
								std::uint8_t parameters [nparams];
								riff.Read(parameters[0]);
								riff.Read(parameters[1]);
								riff.Skip(1);
								riff.Read(parameters[2]);
								riff.Read(parameters[3]);
								retweak(machine, type, parameters, nparams);
							}
							riff.Skip(40);
							break;
						case delay:
							riff.Skip(5);
							{
								const int nparams=4;
								std::int32_t parameters [nparams];
								riff.Read(parameters[0]);
								riff.Read(parameters[2]);
								riff.Read(parameters[1]);
								riff.Read(parameters[3]);
								retweak(machine, type, parameters, nparams);
							}
							riff.Skip(24);
							break;
						case flanger:
							riff.Skip(4);
							{
								const int nparams=1;
								unsigned char parameters [nparams]; riff.ReadArray(parameters, nparams);
								retweak(machine, type, parameters, nparams, 9);
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
								const int nparams=6;
								std::int32_t parameters [nparams];
								riff.ReadArray(&parameters[1], nparams-1);
								riff.Read(parameters[0]);
								retweak(machine, type, parameters, nparams);
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
///FIXME: This has to be ported to the new pattern class.
#if 0
				// Get the first category (there's only one with imported psy's) and...
				std::vector<PatternCategory*>::iterator cit  = song.patternSequence()->getPatternPool()->begin();
				// ... for all the patterns in this category...
				for (std::vector<Pattern*>::iterator pit  = (*cit)->begin() ; pit != (*cit)->end(); pit++)
				{
					// ... check all lines searching...
					for ( Pattern::iterator lit = (*pit)->begin() ; lit != (*pit)->end() ; lit++ ) {
						PatternEvent &  = lit->second;
						// ...tweaks to modify.
						for ( std::map<int, PatternEvent>::iterator tit = line.tweaks().begin(); tit != line.tweaks().end() ; tit++  )
						{
								// If this tweak is for a replaced machine, modify the values.
								std::map<Machine * const, const int *>::const_iterator i(machine_converted_from.find(song.machine(tit->second.machine())));
								if(i != machine_converted_from.end())
								{
									//Machine & machine(*i->first);
									const int & type(*i->second);
									int parameter(tit->second.instrument());
									int value((tit->second.command() << 8) + tit->second.parameter());
									retweak(type, parameter, value);
									tit->second.setInstrument(parameter);
									tit->second.setCommand(value>>8);
									tit->second.setParameter(value&0xff);
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
							///\todo: This "if" had to be added to avoid a crash when pattern contains erroneous data
							/// (which can happen, since the user can write any value in the pattern)
							/// More checks should be added where having values out of range can cause bad behaviours.
							if (parameter != 1) break;
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
									value *= Real(2 * 3 * 4 * 5 * 7) / Player::Instance()->timeInfo().samplesPerTick();
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
								else value = common::scale::Exponential(maximum, math::pi * 2 / 10000, math::pi * 2 * 2 * 3 * 4 * 5 * 7).apply_inverse(value * 3e-8 * Player::Instance()->timeInfo().samplesPerTick());
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
