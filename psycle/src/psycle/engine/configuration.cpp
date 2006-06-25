///\file
///\implementation psycle::host::Configuration.
#include <packageneric/pre-compiled.private.hpp>
#include PACKAGENERIC
#include "configuration.hpp"
#include <psycle/host/engine/registry.hpp>
#include <psycle/host/engine/MidiInput.hpp>
#include <psycle/host/engine/song.hpp>
#include <psycle/host/audiodrivers/WaveOut.hpp>
#include <psycle/host/audiodrivers/DirectSound.hpp>
#include <psycle/host/audiodrivers/ASIOInterface.hpp>
//#include <psycle/host/gui/NewMachine.hpp>

namespace psycle
{
	namespace host
	{
		Configuration::Configuration()
		{
			_initialized = false;
			autoStopMachines = false;

			SetSkinDefaults();
			_pOutputDriver = 0;

			// midi
			{
				_pMidiInput = 0;
				_midiHeadroom = 100;
				// enable velocity by default
				{
					midi().velocity().record()  = true;
					midi().velocity().type()    = 0; // 0 is cmd
					midi().velocity().command() = 0xc;
					midi().velocity().from()    = 0;
					midi().velocity().to()      = 0xff;
				}
			}
			// paths
			{
				{
					char c[1 << 10];
					c[0]='\0';
					::GetModuleFileName(0, c, sizeof c);
					program_executable_dir_ = c;
					program_executable_dir_ = program_executable_dir_.substr(0, program_executable_dir_.rfind('\\')) + '\\';
				}
				{
					SetInstrumentDir(appPath()+"instruments");
					SetCurrentInstrumentDir(GetInstrumentDir());
					SetSongDir(appPath()+"songs");
					SetCurrentSongDir(GetSongDir());
					SetSkinDir(appPath()+"skins");
					SetPluginDir
						(
							appPath()
							///\todo problem with manifests on msvc8 ; all plugins put along with the executable and other dll for now
							#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION__MAJOR < 8
								+"PsyclePlugins"
							#endif
						);
					SetVstDir(appPath()+"VstPlugins");
				}
			}
		}

		Configuration::~Configuration() throw()
		{
		}

		bool Configuration::Read()
		{
			_initialized = true;
			return true;
		}

		void Configuration::Write()
		{
		}

		void Configuration::SetInstrumentDir(std::string const & s)
		{
			instrument_dir_ = s;
		}

		void Configuration::SetCurrentInstrumentDir(std::string const & s)
		{
			current_instrument_dir_ = s;
		}

		void Configuration::SetSongDir(std::string const & s)
		{
			song_dir_ = s;
		}

		void Configuration::SetCurrentSongDir(std::string const & s)
		{
			current_song_dir_ = s;
		}

		void Configuration::SetSkinDir(std::string const & s)
		{
			skin_dir_ = s;
		}

		void Configuration::SetPluginDir(std::string const & s)
		{
			plugin_dir_ = s;
		}

		void Configuration::SetVstDir(std::string const & s)
		{
			vst_dir_ = s;
		}
	}
}
