// -*- mode:c++; indent-tabs-mode:t -*-

/**********************************************************************************************
	Copyright 2007-2008 members of the psycle project http://psycle.sourceforge.net

	This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
	This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
	You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**********************************************************************************************/

#include "configuration.hpp"
#include <psycle/core/file.h>

#include <psycle/audiodrivers/audiodriver.h>
#include <psycle/audiodrivers/wavefileout.h>

#if defined PSYCLE__SYDNEY_AVAILABLE
	#include <psycle/audiodrivers/sydney_out.hpp>
#endif
#if defined PSYCLE__GSTREAMER_AVAILABLE
	#include <psycle/audiodrivers/gstreamerout.h>
#endif
#if defined PSYCLE__JACK_AVAILABLE
	#include <psycle/audiodrivers/jackout.h>
#endif
#if defined PSYCLE__ALSA_AVAILABLE
	#include <psycle/audiodrivers/alsaout.h>
#endif
#if defined PSYCLE__ESOUND_AVAILABLE
	#include <psycle/audiodrivers/esoundout.h>
#endif
#if defined PSYCLE__NET_AUDIO_AVAILABLE
	#include <psycle/audiodrivers/netaudioout.h>
#endif
#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
	#include <psycle/audiodrivers/microsoftdirectsoundout.h>
#endif
#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
	#include <psycle/audiodrivers/microsoftmmewaveout.h>
#endif
#if defined PSYCLE__STEINBERG_ASIO_AVAILABLE
	#include <psycle/audiodrivers/steinberg_asio_out.hpp>
#endif

#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <exception>
#include <iostream>

#include <QtCore> // For getting key binding stuff.
#include <QtXml/QDomDocument> // for reading XML file
#include <QTextStream>

namespace qpsycle {

	Configuration::Configuration()
	:
		_RecordTweaks(),
		_RecordUnarmed()
	{
		AudioDriver* driver = new AudioDriver;
		_pSilentDriver = driver;
		addAudioDriver(driver);

		setDriverByName("silent");
		enableSound_ = false;

		addAudioDriver(new psy::core::WaveFileOut);

		#if defined PSYCLE__ALSA_AVAILABLE
			addAudioDriver(new psy::core::AlsaOut);
			// use alsa by default
			setDriverByName("alsa");
			enableSound_ = true;
		#endif
		#if defined PSYCLE__JACK_AVAILABLE
			addAudioDriver(new psy::core::JackOut);
		#endif
		#if defined PSYCLE__ESOUND_AVAILABLE
			addAudioDriver(new psy::core::ESoundOut);
		#endif
		#if defined PSYCLE__GSTREAMER_AVAILABLE
			addAudioDriver(new psy::core::GStreamerOut);
		#endif
		#if defined PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE
			addAudioDriver(new psy::core::MsDirectSound);
			// use dsound by default
			setDriverByName("dsound");
			enableSound_ = true;
		#endif
		#if defined PSYCLE__MICROSOFT_MME_AVAILABLE
			addAudioDriver(new psy::core::MsWaveOut);
		#endif
		#if defined PSYCLE__NET_AUDIO_AVAILABLE
			addAudioDriver(new psy::core::NetAudioOut);
		#endif

		#if 0 // where are these paths stored?
		{ char const * const env(std::getenv("PSYCLE_PATH"));
			if(env) plugin_path_ = env;
		}

		{ char const * const env(std::getenv("LADSPA_PATH"));
			if(env) ladspa_path_ = env;
		}
		#endif

		configureKeyBindings();
		loadConfig();
	}

	void Configuration::addAudioDriver(AudioDriver* driver) {
		std::cout << "psycle: configuration: audio driver registered: " <<  driver->info().name() << std::endl;
		driverMap_[ driver->info().name() ] = driver;
	}

	void Configuration::setDriverByName(const std::string & driverName) {
		std::map< std::string, AudioDriver*>::iterator it = driverMap_.begin();
		if((it = driverMap_.find(driverName)) != driverMap_.end()) {
			// driver found
			_pOutputDriver = it->second;
			std::cout << "psycle: configuration: audio driver set to: " << driverName << "\n";
		} else {
			std::cerr << "psycle: configuration: audio driver not found: " << driverName << ", setting fallback: " << _pOutputDriver->info().name() << "\n";
			// driver not found,  set silent default driver
			_pOutputDriver = _pSilentDriver;
		}
	}

	void Configuration::loadConfig() {
		std::string path = psy::core::File::replaceTilde("~" + psy::core::File::slash() + ".xpsycle.xml");
		if(path.length()) {
			try {
				loadConfig(psy::core::File::replaceTilde("~" + psy::core::File::slash() + ".xpsycle.xml"));
			} catch(std::exception const & e) {
				std::cerr << "psycle: configuration: error: " << e.what() << std::endl;
			}
		} else {
			#if 0
				#if defined PSYCLE__INSTALL_PATHS__CONFIGURATION
					path = PSYCLE__INSTALL_PATHS__CONFIGURATION "/xpsycle.xml";
				#endif
				if(path.length()) {
					try {
						loadConfig(path);
					} catch(std::exception const & e) {
						std::cerr << "psycle: configuration: error: " << e.what() << std::endl;
					}
				}
			#endif
		}
	}

	void Configuration::loadConfig(const std::string & path) {
		///\todo this implementation lacks some checks. it may access null pointer if the xml document tree isn't as expected.
		configFilePath_ = path;
		QFile *file = new QFile(QString::fromStdString(path));
		if(file->open(QIODevice::ReadOnly | QIODevice::Text)) {
			QDomDocument *doc = new QDomDocument();
			doc->setContent(file);
			QDomElement root = doc->firstChildElement();
			#if 0
			{ // paths
				QDomNodeList paths = root.elementsByTagName("path");
				for(int i = 0; i < paths.count(); ++i) {
					QDomElement path = paths.item(i).toElement();
					std::string id = path.attribute("id").toStdString();
					std::string src = path.attribute("src").toStdString();
					if(id == "plugindir") pluginPath_ = src;
					else if(id == "ladspadir") ladspaPath_ = src;
				}
			}
			#endif
			{ // audio
				{ // enable
					QDomElement audioElm = root.firstChildElement("audio");
					std::string enableStr = audioElm.attribute("enable").toStdString();
					int enable = 0;
					if(enableStr != "") enable = QString::fromStdString(enableStr).toInt();
					enableSound_ = enable;
					if(enable == 0) {
						setDriverByName("silent");
						doEnableSound = false;
					} else doEnableSound = true;
				}
				{ // driver
					QDomElement driverElm = root.firstChildElement("driver");
					if(doEnableSound) setDriverByName(driverElm.attribute("name").toStdString());
				}
				// alsa driver
				if(_pOutputDriver->info().name() == "alsa") {
					///\todo what if alsa settings are missing from xml document?
					QDomElement alsaElm = root.firstChildElement("alsa"); 
					std::string deviceName = alsaElm.attribute("device").toStdString();
					std::map< std::string, AudioDriver*>::iterator it = driverMap_.begin();
					if((it = driverMap_.find("alsa")) != driverMap_.end()) {
						psy::core::AudioDriverSettings settings = it->second->settings(); ///\todo why do we do a copy?
						if(deviceName.length()) {
							settings.setDeviceName(deviceName);
						} else {
							///\todo use the ALSA_CARD env var if present: char const * const device(std::getenv("ALSA_CARD"));
							settings.setDeviceName("default");
						}
						it->second->setSettings(settings); ///\todo why do we copy?
					}
				}
			}
		}
		doEnableSound = true;
	}

	void Configuration::configureKeyBindings() {
		int modifiers = Qt::NoModifier;
		inputHandler_.changeKeyCode(commands::key_stop, Key(modifiers, Qt::Key_1));
		inputHandler_.changeKeyCode(commands::key_C_0, Key(modifiers, Qt::Key_Z));
		inputHandler_.changeKeyCode(commands::key_CS0, Key(modifiers, Qt::Key_S));
		inputHandler_.changeKeyCode(commands::key_D_0, Key(modifiers, Qt::Key_X));
		inputHandler_.changeKeyCode(commands::key_DS0, Key(modifiers, Qt::Key_D));
		inputHandler_.changeKeyCode(commands::key_E_0, Key(modifiers, Qt::Key_C));
		inputHandler_.changeKeyCode(commands::key_F_0, Key(modifiers, Qt::Key_V));
		inputHandler_.changeKeyCode(commands::key_FS0, Key(modifiers, Qt::Key_G));
		inputHandler_.changeKeyCode(commands::key_G_0, Key(modifiers, Qt::Key_B));
		inputHandler_.changeKeyCode(commands::key_GS0, Key(modifiers, Qt::Key_H));
		inputHandler_.changeKeyCode(commands::key_A_0, Key(modifiers, Qt::Key_N));
		inputHandler_.changeKeyCode(commands::key_AS0, Key(modifiers, Qt::Key_J));
		inputHandler_.changeKeyCode(commands::key_B_0, Key(modifiers, Qt::Key_M));
		inputHandler_.changeKeyCode(commands::key_C_1, Key(modifiers, Qt::Key_Q));
		inputHandler_.changeKeyCode(commands::key_CS1, Key(modifiers, Qt::Key_2));
		inputHandler_.changeKeyCode(commands::key_D_1, Key(modifiers, Qt::Key_W));
		inputHandler_.changeKeyCode(commands::key_DS1, Key(modifiers, Qt::Key_3));
		inputHandler_.changeKeyCode(commands::key_E_1, Key(modifiers, Qt::Key_E));
		inputHandler_.changeKeyCode(commands::key_F_1, Key(modifiers, Qt::Key_R));
		inputHandler_.changeKeyCode(commands::key_FS1, Key(modifiers, Qt::Key_5));
		inputHandler_.changeKeyCode(commands::key_G_1, Key(modifiers, Qt::Key_T));
		inputHandler_.changeKeyCode(commands::key_GS1, Key(modifiers, Qt::Key_6));
		inputHandler_.changeKeyCode(commands::key_A_1, Key(modifiers, Qt::Key_Y));
		inputHandler_.changeKeyCode(commands::key_AS1, Key(modifiers, Qt::Key_7));
		inputHandler_.changeKeyCode(commands::key_B_1, Key(modifiers, Qt::Key_U));
		inputHandler_.changeKeyCode(commands::key_C_2, Key(modifiers, Qt::Key_I));
		inputHandler_.changeKeyCode(commands::key_CS2, Key(modifiers, Qt::Key_9));
		inputHandler_.changeKeyCode(commands::key_D_2, Key(modifiers, Qt::Key_O));
		inputHandler_.changeKeyCode(commands::key_DS2, Key(modifiers, Qt::Key_0));
		inputHandler_.changeKeyCode(commands::key_E_2, Key(modifiers, Qt::Key_P));

		#if 0 // no bindings for now.
			inputHandler_.changeKeyCode(commands::key_F_2, Key(modifiers, Qt::Key_X));
			inputHandler_.changeKeyCode(commands::key_FS2, Key(modifiers, Qt::Key_X));
			inputHandler_.changeKeyCode(commands::key_G_2, Key(modifiers, Qt::Key_X));
			inputHandler_.changeKeyCode(commands::key_GS2, Key(modifiers, Qt::Key_X));
			inputHandler_.changeKeyCode(commands::key_A_2, Key(modifiers, Qt::Key_X));
		#endif

		inputHandler_.changeKeyCode(commands::show_pattern_box,    Key(modifiers, Qt::Key_F1));
		inputHandler_.changeKeyCode(commands::show_machine_view,   Key(modifiers, Qt::Key_F2));
		inputHandler_.changeKeyCode(commands::show_pattern_view,   Key(modifiers, Qt::Key_F3));
		inputHandler_.changeKeyCode(commands::show_wave_editor,    Key(modifiers, Qt::Key_F4));
		inputHandler_.changeKeyCode(commands::show_sequencer_view, Key(modifiers, Qt::Key_F5));

		// Play control commands.
		inputHandler_.changeKeyCode(commands::play_from_position, Key(Qt::NoModifier,    Qt::Key_F6));
		inputHandler_.changeKeyCode(commands::play_start,         Key(Qt::ShiftModifier, Qt::Key_F6));
		inputHandler_.changeKeyCode(commands::play_loop_entry,    Key(Qt::NoModifier,    Qt::Key_F7));
		inputHandler_.changeKeyCode(commands::play_stop,          Key(Qt::NoModifier,    Qt::Key_F8));

		inputHandler_.changeKeyCode(commands::instrument_dec, Key(Qt::ControlModifier, Qt::Key_Minus));
		inputHandler_.changeKeyCode(commands::instrument_inc, Key(Qt::ControlModifier, Qt::Key_Equal));

		inputHandler_.changeKeyCode(commands::octave_down, Key(Qt::KeypadModifier, Qt::Key_Slash));
		inputHandler_.changeKeyCode(commands::octave_up,   Key(Qt::KeypadModifier, Qt::Key_Asterisk));

		// Machine View.
		inputHandler_.changeKeyCode(commands::mute_machine,   Key(Qt::ShiftModifier, Qt::Key_M));
		inputHandler_.changeKeyCode(commands::solo_machine,   Key(Qt::ShiftModifier, Qt::Key_S));
		inputHandler_.changeKeyCode(commands::bypass_machine, Key(Qt::ShiftModifier, Qt::Key_B));

		// Pattern View.
		inputHandler_.changeKeyCode(commands::navigate_up,        Key(Qt::NoModifier, Qt::Key_Up));
		inputHandler_.changeKeyCode(commands::navigate_down,      Key(Qt::NoModifier, Qt::Key_Down));
		inputHandler_.changeKeyCode(commands::navigate_left,      Key(Qt::NoModifier, Qt::Key_Left));
		inputHandler_.changeKeyCode(commands::navigate_right,     Key(Qt::NoModifier, Qt::Key_Right));
		inputHandler_.changeKeyCode(commands::navigate_page_down, Key(Qt::NoModifier, Qt::Key_PageDown));
		inputHandler_.changeKeyCode(commands::navigate_page_up,   Key(Qt::NoModifier, Qt::Key_PageUp));
		inputHandler_.changeKeyCode(commands::track_next,         Key(Qt::NoModifier, Qt::Key_Tab));
		inputHandler_.changeKeyCode(commands::track_prev,         Key(Qt::ShiftModifier, Qt::Key_Backtab));
		inputHandler_.changeKeyCode(commands::navigate_top,       Key(Qt::NoModifier, Qt::Key_Home));
		inputHandler_.changeKeyCode(commands::navigate_bottom,    Key(Qt::NoModifier, Qt::Key_End));

		inputHandler_.changeKeyCode(commands::pattern_step_dec, Key(Qt::NoModifier, Qt::Key_BracketLeft));
		inputHandler_.changeKeyCode(commands::pattern_step_inc, Key(Qt::NoModifier, Qt::Key_BracketRight));

		inputHandler_.changeKeyCode(commands::select_up,    Key(Qt::ShiftModifier, Qt::Key_Up));
		inputHandler_.changeKeyCode(commands::select_down,  Key(Qt::ShiftModifier, Qt::Key_Down));
		inputHandler_.changeKeyCode(commands::select_left,  Key(Qt::ShiftModifier, Qt::Key_Left));
		inputHandler_.changeKeyCode(commands::select_right, Key(Qt::ShiftModifier, Qt::Key_Right));
		inputHandler_.changeKeyCode(commands::select_track, Key(Qt::ControlModifier, Qt::Key_R));
		inputHandler_.changeKeyCode(commands::select_all,   Key(Qt::ControlModifier, Qt::Key_A));
		inputHandler_.changeKeyCode(commands::block_start,  Key(Qt::ControlModifier, Qt::Key_B));
		inputHandler_.changeKeyCode(commands::block_end,    Key(Qt::ControlModifier, Qt::Key_E));
		inputHandler_.changeKeyCode(commands::block_unmark, Key(Qt::ControlModifier, Qt::Key_U));

		inputHandler_.changeKeyCode(commands::block_copy,   Key(Qt::ControlModifier, Qt::Key_C));
		inputHandler_.changeKeyCode(commands::block_cut,    Key(Qt::ControlModifier, Qt::Key_X));
		inputHandler_.changeKeyCode(commands::block_paste,  Key(Qt::ControlModifier, Qt::Key_V));
		inputHandler_.changeKeyCode(commands::block_delete, Key(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_X));

		inputHandler_.changeKeyCode(commands::transpose_block_inc,   Key(Qt::ControlModifier, Qt::Key_F12));
		inputHandler_.changeKeyCode(commands::transpose_block_inc12, Key(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_F12));
		inputHandler_.changeKeyCode(commands::transpose_block_dec,   Key(Qt::ControlModifier, Qt::Key_F11));
		inputHandler_.changeKeyCode(commands::transpose_block_dec12, Key(Qt::ControlModifier | Qt::ShiftModifier, Qt::Key_F11));

		inputHandler_.changeKeyCode(commands::block_set_instrument, Key(Qt::ControlModifier, Qt::Key_T));
		inputHandler_.changeKeyCode(commands::block_set_machine,    Key(Qt::ControlModifier, Qt::Key_G));
		inputHandler_.changeKeyCode(commands::select_machine,       Key(Qt::NoModifier, Qt::Key_Return));

		inputHandler_.changeKeyCode(commands::row_clear,  Key(Qt::NoModifier, Qt::Key_Delete));
		inputHandler_.changeKeyCode(commands::row_insert, Key(Qt::NoModifier, Qt::Key_Insert));
		inputHandler_.changeKeyCode(commands::row_delete, Key(Qt::NoModifier, Qt::Key_Backspace));
	}

} // namespace qsycle
