///\file
///\implementation psycle::host::Configuration.
#include <psycle/project.private.hpp>
#include "Global.hpp"
#include "Configuration.hpp"
#include "Registry.hpp"
#if !defined WINAMP_PLUGIN
	#include "WaveOut.hpp"
	#include "DirectSound.hpp"
	#include "ASIOInterface.hpp"
	#include "MidiInput.hpp"
	#include "NewMachine.hpp"
#endif // !defined WINAMP_PLUGIN
#include "Song.hpp"
namespace psycle
{
	namespace host
	{
		Configuration::Configuration()
		{
			_initialized = false;
			autoStopMachines = false;
#if !defined WINAMP_PLUGIN
			_allowMultipleInstances = false;
			_toolbarOnVsts = true;
			_bShowPatternNames = false;
			_windowsBlocks = true;
			_wrapAround = true;
			_centerCursor = false;
			_cursorAlwaysDown = false;
			_RecordNoteoff = false;
			_RecordTweaks = false;
			_notesToEffects = false;
			_RecordMouseTweaksSmooth = FALSE;
			_RecordUnarmed = true;
			_NavigationIgnoresStep = false;
			_midiMachineViewSeqMode = false;
			useDoubleBuffer = true;
			_showAboutAtStart = true;
			_followSong = false;
			bShowSongInfoOnLoad = true;
			bFileSaveReminders = true;
			autosaveSong = true;
			autosaveSongTime = 10;
			_pageUpSteps = 1;

			SetSkinDefaults();
			// soundcard output device
			{
				_numOutputDrivers = 4;
				_ppOutputDrivers = new AudioDriver*[_numOutputDrivers];
				_ppOutputDrivers[0] = new AudioDriver;
				_ppOutputDrivers[1] = new WaveOut;
				_ppOutputDrivers[2] = new DirectSound;
				_ppOutputDrivers[3] = new ASIOInterface;
				if(((ASIOInterface*)(_ppOutputDrivers[3]))->_drivEnum.size() <= 0)
				{
					_numOutputDrivers--;
					delete _ppOutputDrivers[3]; _ppOutputDrivers[3] = 0;
				}
				_outputDriverIndex = 1;
				_pOutputDriver = _ppOutputDrivers[_outputDriverIndex];
			}
			// midi
			{
				_pMidiInput = new CMidiInput;
				_numMidiDrivers = CMidiInput::Instance()->GetNumDevices();
				_midiDriverIndex = 0;
				_syncDriverIndex = 0;
				CMidiInput::Instance()->SetDeviceId(DRIVER_MIDI, _midiDriverIndex - 1);
				CMidiInput::Instance()->SetDeviceId(DRIVER_SYNC, _syncDriverIndex - 1);
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
#endif // !defined WINAMP_PLUGIN
			// pattern height
			{
				defaultPatLines = 64;
				for(int c(0) ; c < MAX_PATTERNS; ++c)
				{
					// All pattern reset
					Global::_pSong->patternLines[c] = defaultPatLines;
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
					SetPluginDir(appPath()+"PsyclePlugins");
					SetVstDir(appPath()+"VstPlugins");
				}
			}
		}

		Configuration::~Configuration() throw()
		{
#if !defined WINAMP_PLUGIN
			seqFont.DeleteObject();
			generatorFont.DeleteObject();
			effectFont.DeleteObject();
			if(_numOutputDrivers)
			{
				for (int i(0);i<_numOutputDrivers;++i)
				{
					delete _ppOutputDrivers[i];
				}
				delete [] _ppOutputDrivers;
			}
			delete _pMidiInput;
#endif // !defined WINAMP_PLUGIN
		}

		bool Configuration::Read()
		{
			_initialized = true;
			Registry reg;
			Registry::result result;
			if(reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT) == ERROR_SUCCESS) // general case
			{
				result = reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY);
				if(result != ERROR_SUCCESS)
				{
					reg.CloseRootKey();
					if (reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT "--1.7") == ERROR_SUCCESS)  // case for 1.7 alphas
					{
						result = reg.OpenKey("configuration");
						if(result != ERROR_SUCCESS)
						{
							reg.CloseRootKey();
							return ReadVersion17();		// Case for 1.7.6 and older. If it doesn't exist, it will return false.
						}
					}
					else return ReadVersion17();		// Case for 1.7.6 and older. If it doesn't exist, it will return false.
				}
			}
			else if (reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT "--1.7") == ERROR_SUCCESS)  // case for 1.7 alphas
			{
				result = reg.OpenKey("configuration");
				if(result != ERROR_SUCCESS)
				{
					reg.CloseRootKey();
					return false;
				}
			}
			else return ReadVersion17();		// Case for 1.7.6 and older. If it doesn't exist, it will return false.


#if !defined WINAMP_PLUGIN
			reg.QueryValue("NewMacDlgpluginOrder", CNewMachine::pluginOrder);
			reg.QueryValue("NewMacDlgpluginName", CNewMachine::pluginName);
			reg.QueryValue("WrapAround", _wrapAround);
			reg.QueryValue("AllowMultipleInstances", _allowMultipleInstances);
			reg.QueryValue("windowsBlocks", _windowsBlocks);
			reg.QueryValue("CenterCursor", _centerCursor);
			reg.QueryValue("FollowSong", _followSong);
			reg.QueryValue("CursorAlwaysDown", _cursorAlwaysDown);
			reg.QueryValue("useDoubleBuffer", useDoubleBuffer);
			reg.QueryValue("DisplayLineNumbers", _linenumbers);
			reg.QueryValue("DisplayLineNumbersHex", _linenumbersHex);
			reg.QueryValue("DisplayLineNumbersCursor", _linenumbersCursor);
			reg.QueryValue("showAboutAtStart", _showAboutAtStart);
			reg.QueryValue("RecordNoteoff", _RecordNoteoff);
			reg.QueryValue("RecordTweaks", _RecordTweaks);
			reg.QueryValue("RecordMouseTweaksSmooth", _RecordMouseTweaksSmooth);
			reg.QueryValue("notesToEffects", _notesToEffects);
			reg.QueryValue("RecordUnarmed", _RecordUnarmed);
			reg.QueryValue("ShowPatternNames", _bShowPatternNames);
			reg.QueryValue("toolbarOnVsts", _toolbarOnVsts);
			//reg.QueryValue("MoveCursorPaste", _MoveCursorPaste);
			reg.QueryValue("NavigationIgnoresStep", _NavigationIgnoresStep);
			// midi
			{
				reg.QueryValue("MidiMachineViewSeqMode", _midiMachineViewSeqMode);
				// velocity
				{
					reg.QueryValue("MidiRecordVel" , midi().velocity().record() );
					reg.QueryValue("MidiTypeVel"   , midi().velocity().type()   );
					reg.QueryValue("MidiCommandVel", midi().velocity().command());
					reg.QueryValue("MidiFromVel"   , midi().velocity().from()   );
					reg.QueryValue("MidiToVel"     , midi().velocity().to()     );
				}
				// pitch
				{
					reg.QueryValue("MidiRecordPit" , midi().pitch()   .record() );
					reg.QueryValue("MidiTypePit"   , midi().pitch()   .type()   );
					reg.QueryValue("MidiCommandPit", midi().pitch()   .command());
					reg.QueryValue("MidiFromPit"   , midi().pitch()   .from()   );
					reg.QueryValue("MidiToPit"     , midi().pitch()   .to()     );
				}
				for(std::size_t i(0) ; i < midi().groups().size() ; ++i)
				{
					std::ostringstream oss;
					oss << i;
					std::string s(oss.str());
					reg.QueryValue("MidiMessage" + s, midi().group(i).message());
					reg.QueryValue("MidiRecord"  + s, midi().group(i).record() );
					reg.QueryValue("MidiType"    + s, midi().group(i).type()   );
					reg.QueryValue("MidiCommand" + s, midi().group(i).command());
					reg.QueryValue("MidiFrom"    + s, midi().group(i).from()   );
					reg.QueryValue("MidiTo"      + s, midi().group(i).to()     );
				}
				reg.QueryValue("MidiRawMcm", midi().raw());
			}
#endif // !defined WINAMP_PLUGIN

			reg.QueryValue("defaultPatLines", defaultPatLines);
			for(int c(0) ; c < MAX_PATTERNS; ++c)
			{
				// All pattern reset
				Global::_pSong->patternLines[c] = defaultPatLines;
			}
#if !defined WINAMP_PLUGIN
			reg.QueryValue("bShowSongInfoOnLoad", bShowSongInfoOnLoad);
			reg.QueryValue("bFileSaveReminders", bFileSaveReminders);
			reg.QueryValue("autosaveSong", autosaveSong);
			reg.QueryValue("autosaveSongTime", autosaveSongTime);
			reg.QueryValue("pageupStepSize",_pageUpSteps);
			reg.QueryValue("pv_timesig",pv_timesig);
			reg.QueryValue("mv_colour", mv_colour);
			reg.QueryValue("mv_wirecolour", mv_wirecolour);
			reg.QueryValue("mv_polycolour", mv_polycolour);
			reg.QueryValue("mv_wireaa", mv_wireaa);
			reg.QueryValue("mv_wirewidth", mv_wirewidth);
			reg.QueryValue("mv_generator_fontcolour", mv_generator_fontcolour);
			reg.QueryValue("mv_effect_fontcolour", mv_effect_fontcolour);
			reg.QueryValue("mv_triangle_size", mv_triangle_size);
			reg.QueryValue("bBmpBkg", bBmpBkg);
			reg.QueryValue("bBmpDial", bBmpDial);
			reg.QueryValue("szBmpBkgFilename", szBmpBkgFilename);
			reg.QueryValue("szBmpDialFilename", szBmpDialFilename);
			reg.QueryValue("pvc_background", pvc_background);
			reg.QueryValue("pvc_background2", pvc_background2);
			reg.QueryValue("pvc_separator", pvc_separator);
			reg.QueryValue("pvc_separator2", pvc_separator2);
			reg.QueryValue("pvc_row4beat", pvc_row4beat);
			reg.QueryValue("pvc_row4beat2", pvc_row4beat2);
			reg.QueryValue("pvc_rowbeat", pvc_rowbeat);
			reg.QueryValue("pvc_rowbeat2", pvc_rowbeat2);
			reg.QueryValue("pvc_row", pvc_row);
			reg.QueryValue("pvc_row2", pvc_row2);
			reg.QueryValue("pvc_font", pvc_font);
			reg.QueryValue("pvc_font2", pvc_font2);
			reg.QueryValue("pvc_fontPlay", pvc_fontPlay);
			reg.QueryValue("pvc_fontPlay2", pvc_fontPlay2);
			reg.QueryValue("pvc_fontCur", pvc_fontCur);
			reg.QueryValue("pvc_fontCur2", pvc_fontCur2);
			reg.QueryValue("pvc_fontSel", pvc_fontSel);
			reg.QueryValue("pvc_fontSel2", pvc_fontSel2);
			reg.QueryValue("pvc_selection", pvc_selection);
			reg.QueryValue("pvc_selection2", pvc_selection2);
			reg.QueryValue("pvc_playbar", pvc_playbar);
			reg.QueryValue("pvc_playbar2", pvc_playbar2);
			reg.QueryValue("pvc_cursor", pvc_cursor);
			reg.QueryValue("pvc_cursor2", pvc_cursor2);
			reg.QueryValue("vu1", vu1);
			reg.QueryValue("vu2", vu2);
			reg.QueryValue("vu3", vu3);
			reg.QueryValue("machineGUITopColor", machineGUITopColor);
			reg.QueryValue("machineGUIFontTopColor", machineGUIFontTopColor);
			reg.QueryValue("machineGUIBottomColor", machineGUIBottomColor);
			reg.QueryValue("machineGUIFontBottomColor", machineGUIFontBottomColor);
			reg.QueryValue("machineGUIHTopColor", machineGUIHTopColor);
			reg.QueryValue("machineGUIHFontTopColor", machineGUIHFontTopColor);
			reg.QueryValue("machineGUIHBottomColor", machineGUIHBottomColor);
			reg.QueryValue("machineGUIHFontBottomColor", machineGUIHFontBottomColor);
			reg.QueryValue("machineGUITitleColor", machineGUITitleColor);
			reg.QueryValue("machineGUITitleFontColor", machineGUITitleFontColor);
			mv_wireaacolour =
				((((mv_wirecolour&0x00ff0000) + ((mv_colour&0x00ff0000)*4))/5)&0x00ff0000) +
				((((mv_wirecolour&0x00ff00) + ((mv_colour&0x00ff00)*4))/5)&0x00ff00) +
				((((mv_wirecolour&0x00ff) + ((mv_colour&0x00ff)*4))/5)&0x00ff);
			mv_wireaacolour2 =
				(((((mv_wirecolour&0x00ff0000)) + ((mv_colour&0x00ff0000)))/2)&0x00ff0000) +
				(((((mv_wirecolour&0x00ff00)) + ((mv_colour&0x00ff00)))/2)&0x00ff00) +
				(((((mv_wirecolour&0x00ff)) + ((mv_colour&0x00ff)))/2)&0x00ff);
			reg.QueryValue("pattern_fontface", pattern_fontface);
			reg.QueryValue("pattern_header_skin", pattern_header_skin);
			reg.QueryValue("pattern_font_point", pattern_font_point);
			reg.QueryValue("pattern_font_x", pattern_font_x);
			reg.QueryValue("pattern_font_y", pattern_font_y);
			reg.QueryValue("pattern_font_flags", pattern_font_flags);
			reg.QueryValue("pattern_draw_empty_data", pattern_draw_empty_data);
			reg.QueryValue("draw_mac_index", draw_mac_index);
			reg.QueryValue("draw_vus", draw_vus);
			reg.QueryValue("generator_fontface", generator_fontface);
			reg.QueryValue("generator_font_point", generator_font_point);
			reg.QueryValue("generator_font_flags", generator_font_flags);
			reg.QueryValue("effect_fontface", effect_fontface);
			reg.QueryValue("effect_font_point", effect_font_point);
			reg.QueryValue("effect_font_flags", effect_font_flags);
			reg.QueryValue("machine_skin", machine_skin);
			CreateFonts();
			// soundcard output device
			{
				reg.QueryValue("OutputDriver", _outputDriverIndex);
				if(0 > _outputDriverIndex || _outputDriverIndex >= _numOutputDrivers) _outputDriverIndex = 1;
				_pOutputDriver = _ppOutputDrivers[_outputDriverIndex];
			}
			// midi
			{
				{
					reg.QueryValue("MidiInputDriver", _midiDriverIndex);
					if(0 > _midiDriverIndex || _midiDriverIndex > _numMidiDrivers) 
					{
					CMidiInput::Instance()->SetDeviceId(DRIVER_MIDI, -1);
					}
					else CMidiInput::Instance()->SetDeviceId(DRIVER_MIDI, _midiDriverIndex - 1);
				}
				{
					reg.QueryValue("MidiSyncDriver", _syncDriverIndex);
					if(0 > _syncDriverIndex || _syncDriverIndex > _numMidiDrivers)
					{
						CMidiInput::Instance()->SetDeviceId(DRIVER_SYNC, -1);
					}
					else CMidiInput::Instance()->SetDeviceId(DRIVER_SYNC, _syncDriverIndex - 1);
				}
				{
					reg.QueryValue("MidiInputHeadroom", _midiHeadroom);
					CMidiInput::Instance()->GetConfigPtr()->midiHeadroom = _midiHeadroom;
				}
			}
#endif // !defined WINAMP_PLUGIN

			// paths
			{
				reg.QueryValue("InstrumentDir", instrument_dir_);
				SetCurrentInstrumentDir(GetInstrumentDir());
				reg.QueryValue("SongDir", song_dir_);
				SetCurrentSongDir(GetSongDir());
				reg.QueryValue("SkinDir", skin_dir_);
				reg.QueryValue("PluginDir", plugin_dir_);
				reg.QueryValue("VstDir", vst_dir_);
			}
			reg.CloseKey();
			reg.CloseRootKey();
			return true;
		}

		void Configuration::Write()
		{
#if !defined WINAMP_PLUGIN
			Registry reg;
			if(reg.OpenRootKey(HKEY_CURRENT_USER, PSYCLE__PATH__REGISTRY__ROOT) != ERROR_SUCCESS)
			{
				Error("Unable to write configuration to the registry");
				return;
			}
			if(reg.OpenKey(PSYCLE__PATH__REGISTRY__CONFIGKEY) != ERROR_SUCCESS)
			{
				if(reg.CreateKey(PSYCLE__PATH__REGISTRY__CONFIGKEY) != ERROR_SUCCESS)
				{
					Error("Unable to write configuration to the registry");
					return;
				}
			}
			reg.SetValue("NewMacDlgpluginOrder", CNewMachine::pluginOrder);
			reg.SetValue("NewMacDlgpluginName", CNewMachine::pluginName);
			reg.SetValue("WrapAround", _wrapAround);
			reg.SetValue("AllowMultipleInstances", _allowMultipleInstances);
			reg.SetValue("CenterCursor", _centerCursor);
			reg.SetValue("FollowSong", _followSong);
			reg.SetValue("windowsBlocks",_windowsBlocks);
			reg.SetValue("CursorAlwaysDown", _cursorAlwaysDown);
			reg.SetValue("useDoubleBuffer", useDoubleBuffer);
			reg.SetValue("DisplayLineNumbers", _linenumbers);
			reg.SetValue("DisplayLineNumbersHex", _linenumbersHex);
			reg.SetValue("DisplayLineNumbersCursor", _linenumbersCursor);
			reg.SetValue("showAboutAtStart", _showAboutAtStart);
			reg.SetValue("RecordNoteoff", _RecordNoteoff);
			reg.SetValue("RecordTweaks", _RecordTweaks);
			reg.SetValue("RecordMouseTweaksSmooth", _RecordMouseTweaksSmooth);
			reg.SetValue("notesToEffects", _notesToEffects);
			reg.SetValue("RecordUnarmed", _RecordUnarmed);
			reg.SetValue("ShowPatternNames", _bShowPatternNames);
			reg.SetValue("toolbarOnVsts", _toolbarOnVsts);
			//reg.SetValue("MoveCursorPaste", _MoveCursorPaste);
			reg.SetValue("NavigationIgnoresStep", _NavigationIgnoresStep);
			reg.SetValue("MidiMachineViewSeqMode", _midiMachineViewSeqMode);
			reg.SetValue("OutputDriver", _outputDriverIndex);
			reg.SetValue("MidiInputDriver", _midiDriverIndex);
			reg.SetValue("MidiSyncDriver", _syncDriverIndex);
			reg.SetValue("MidiInputHeadroom", _midiHeadroom);
			reg.SetValue("MidiRecordVel" , midi().velocity().record() );
			reg.SetValue("MidiTypeVel"   , midi().velocity().type()   );
			reg.SetValue("MidiCommandVel", midi().velocity().command());
			reg.SetValue("MidiFromVel"   , midi().velocity().from()   );
			reg.SetValue("MidiToVel"     , midi().velocity().to()     );
			reg.SetValue("MidiRecordPit" , midi().pitch()   .record() );
			reg.SetValue("MidiTypePit"   , midi().pitch()   .type()   );
			reg.SetValue("MidiCommandPit", midi().pitch()   .command());
			reg.SetValue("MidiFromPit"   , midi().pitch()   .from()   );
			reg.SetValue("MidiToPit"     , midi().pitch()   .to()     );
			for(std::size_t i(0) ; i < midi().groups().size() ; ++i)
			{
				std::ostringstream oss;
				oss << i;
				std::string s(oss.str());
				reg.SetValue("MidiMessage" + s, midi().group(i).message());
				reg.SetValue("MidiRecord"  + s, midi().group(i).record() );
				reg.SetValue("MidiType"    + s, midi().group(i).type()   );
				reg.SetValue("MidiCommand" + s, midi().group(i).command());
				reg.SetValue("MidiFrom"    + s, midi().group(i).from()   );
				reg.SetValue("MidiTo"      + s, midi().group(i).to()     );
			}
			reg.SetValue("MidiRawMcm", midi().raw());
			reg.SetValue("defaultPatLines", defaultPatLines);
			reg.SetValue("bShowSongInfoOnLoad", bShowSongInfoOnLoad);
			reg.SetValue("bFileSaveReminders", bFileSaveReminders);
			reg.SetValue("autosaveSong", autosaveSong);
			reg.SetValue("autosaveSongTime", autosaveSongTime);
			reg.SetValue("pageupStepSize",_pageUpSteps);
			reg.SetValue("pv_timesig",pv_timesig);
			reg.SetValue("mv_colour", mv_colour);
			reg.SetValue("mv_wirecolour", mv_wirecolour);
			reg.SetValue("mv_polycolour", mv_polycolour);
			reg.SetValue("mv_wireaa", mv_wireaa);
			reg.SetValue("mv_wirewidth", mv_wirewidth);
			reg.SetValue("mv_generator_fontcolour", mv_generator_fontcolour);
			reg.SetValue("mv_effect_fontcolour", mv_effect_fontcolour);
			reg.SetValue("mv_triangle_size", mv_triangle_size);
			reg.SetValue("bBmpBkg", bBmpBkg);
			reg.SetValue("bBmpDial", bBmpDial);
			reg.SetValue("szBmpBkgFilename", szBmpBkgFilename);
			reg.SetValue("szBmpDialFilename", szBmpDialFilename);
			reg.SetValue("pvc_separator", pvc_separator);
			reg.SetValue("pvc_separator2", pvc_separator2);
			reg.SetValue("pvc_background", pvc_background);
			reg.SetValue("pvc_background2", pvc_background2);
			reg.SetValue("pvc_row4beat", pvc_row4beat);
			reg.SetValue("pvc_row4beat2", pvc_row4beat2);
			reg.SetValue("pvc_rowbeat", pvc_rowbeat);
			reg.SetValue("pvc_rowbeat2", pvc_rowbeat2);
			reg.SetValue("pvc_row", pvc_row);
			reg.SetValue("pvc_row2", pvc_row2);
			reg.SetValue("pvc_font", pvc_font);
			reg.SetValue("pvc_font2", pvc_font2);
			reg.SetValue("pvc_fontPlay", pvc_fontPlay);
			reg.SetValue("pvc_fontPlay2", pvc_fontPlay2);
			reg.SetValue("pvc_fontCur", pvc_fontCur);
			reg.SetValue("pvc_fontCur2", pvc_fontCur2);
			reg.SetValue("pvc_fontSel", pvc_fontSel);
			reg.SetValue("pvc_fontSel2", pvc_fontSel2);
			reg.SetValue("pvc_selection", pvc_selection);
			reg.SetValue("pvc_selection2", pvc_selection2);
			reg.SetValue("pvc_playbar", pvc_playbar);
			reg.SetValue("pvc_playbar2", pvc_playbar2);
			reg.SetValue("pvc_cursor", pvc_cursor);
			reg.SetValue("pvc_cursor2", pvc_cursor2);
			reg.SetValue("vu1", vu1);
			reg.SetValue("vu2", vu2);
			reg.SetValue("vu3", vu3);
			reg.SetValue("machineGUITopColor", machineGUITopColor);
			reg.SetValue("machineGUIFontTopColor", machineGUIFontTopColor);
			reg.SetValue("machineGUIBottomColor", machineGUIBottomColor);
			reg.SetValue("machineGUIFontBottomColor", machineGUIFontBottomColor);
			reg.SetValue("machineGUIHTopColor", machineGUIHTopColor);
			reg.SetValue("machineGUIHFontTopColor", machineGUIHFontTopColor);
			reg.SetValue("machineGUIHBottomColor", machineGUIHBottomColor);
			reg.SetValue("machineGUIHFontBottomColor", machineGUIHFontBottomColor);
			reg.SetValue("machineGUITitleColor", machineGUITitleColor);
			reg.SetValue("machineGUITitleFontColor", machineGUITitleFontColor);
			reg.SetValue("pattern_fontface", pattern_fontface);
			reg.SetValue("pattern_font_point", pattern_font_point);
			reg.SetValue("pattern_font_flags", pattern_font_flags);
			reg.SetValue("pattern_font_x", pattern_font_x);
			reg.SetValue("pattern_font_y", pattern_font_y);
			reg.SetValue("pattern_draw_empty_data", pattern_draw_empty_data);
			reg.SetValue("draw_mac_index", draw_mac_index);
			reg.SetValue("draw_vus", draw_vus);
			reg.SetValue("pattern_header_skin", pattern_header_skin);
			reg.SetValue("generator_fontface", generator_fontface);
			reg.SetValue("generator_font_point", generator_font_point);
			reg.SetValue("generator_font_flags", generator_font_flags);
			reg.SetValue("effect_fontface", effect_fontface);
			reg.SetValue("effect_font_point", effect_font_point);
			reg.SetValue("effect_font_flags", effect_font_flags);
			reg.SetValue("machine_skin", machine_skin);
			reg.SetValue("InstrumentDir", GetInstrumentDir());
			reg.SetValue("SongDir", GetSongDir());
			reg.SetValue("PluginDir", GetPluginDir());
			reg.SetValue("VstDir", GetVstDir());
			reg.SetValue("SkinDir", GetSkinDir());
			reg.CloseKey();
			reg.CloseRootKey();
#endif // !defined WINAMP_PLUGIN

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

		void Configuration::Error(std::string const & what)
		{
			MessageBox(0, what.c_str(), "Psycle", MB_ICONERROR | MB_OK);
		}
#if !defined WINAMP_PLUGIN

		bool Configuration::CreatePsyFont(CFont & f, std::string const & sFontFace, int const & HeightPx, bool const & bBold, bool const & bItalic)
		{
			f.DeleteObject();
			CString sFace(sFontFace.c_str());
			LOGFONT lf = LOGFONT();
			if(bBold) lf.lfWeight = FW_BOLD;
			if(bItalic) lf.lfItalic = true;
			lf.lfHeight = HeightPx;
			lf.lfQuality = NONANTIALIASED_QUALITY;
			std::strncpy(lf.lfFaceName,(LPCTSTR)sFace,32);
			if(!f.CreatePointFontIndirect(&lf))
			{			
				CString sFaceLowerCase = sFace;
				sFaceLowerCase.MakeLower();
				strncpy(lf.lfFaceName,(LPCTSTR)sFaceLowerCase,32);
				if(!f.CreatePointFontIndirect(&lf)) return false;
			}
			return true;
		}

		void Configuration::CreateFonts()
		{	
			bool bBold = pattern_font_flags & 1;
			bool bItalic = pattern_font_flags & 2;
			if(!CreatePsyFont(seqFont,pattern_fontface,pattern_font_point,bBold,bItalic))
			{
				MessageBox(0,pattern_fontface.c_str(),"Could not find this font!",0);
				if(!CreatePsyFont(seqFont,"Tahoma",pattern_font_point,bBold,bItalic))
					if(!CreatePsyFont(seqFont,"Verdana",pattern_font_point,bBold,bItalic))
						CreatePsyFont(seqFont,"Arial",pattern_font_point,bBold,bItalic);
			}
			bBold = generator_font_flags & 1;
			bItalic = generator_font_flags & 2;
			if(!CreatePsyFont(generatorFont,generator_fontface,generator_font_point,bBold,bItalic))
			{
				MessageBox(0,generator_fontface.c_str(),"Could not find this font!",0);
				if(!CreatePsyFont(seqFont,"Tahoma",generator_font_point,bBold,bItalic))
					if(!CreatePsyFont(seqFont,"Verdana",generator_font_point,bBold,bItalic))
						CreatePsyFont(seqFont,"Arial",generator_font_point,bBold,bItalic);
			}
			bBold = effect_font_flags & 1;
			bItalic = effect_font_flags & 2;
			if(!CreatePsyFont(effectFont,effect_fontface,effect_font_point,bBold,bItalic))
			{
				MessageBox(0,effect_fontface.c_str(),"Could not find this font!",0);
				if(!CreatePsyFont(seqFont,"Tahoma",effect_font_point,bBold,bItalic))
					if(!CreatePsyFont(seqFont,"Verdana",effect_font_point,bBold,bItalic))
						CreatePsyFont(seqFont,"Arial",effect_font_point,bBold,bItalic);
			}
		}

		void Configuration::SetSkinDefaults()
		{
			_linenumbers = true;
			_linenumbersHex = false;
			_linenumbersCursor = true;
			pv_timesig = 4;
			pattern_fontface = "Tahoma";
			pattern_header_skin = PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN;
			pattern_font_point = 80;
			pattern_font_x = 9;
			pattern_font_y = 11;
			pattern_draw_empty_data = FALSE;
			draw_mac_index = TRUE;
			draw_vus = TRUE;
			generator_fontface = "Tahoma";
			generator_font_point = 80;
			effect_fontface = "Tahoma";
			effect_font_point = 80;
			pattern_font_flags = 1;
			generator_font_flags = 0;
			effect_font_flags = 0;
			machine_skin = PSYCLE__PATH__DEFAULT_MACHINE_SKIN;
			mv_colour =	0x0099766C;
			mv_wirecolour =	0x00000000;
			mv_polycolour =	0x00ffffff;
			mv_generator_fontcolour = 0x00000000;
			mv_effect_fontcolour = 0x00000000;
			mv_wireaa = 1;
			mv_triangle_size = 22;
			mv_wirewidth = 1;
			mv_wireaacolour =
				((((mv_wirecolour&0x00ff0000) + ((mv_colour&0x00ff0000)*4))/5)&0x00ff0000) +
				((((mv_wirecolour&0x00ff00) + ((mv_colour&0x00ff00)*4))/5)&0x00ff00) +
				((((mv_wirecolour&0x00ff) + ((mv_colour&0x00ff)*4))/5)&0x00ff);

			mv_wireaacolour2 =
				(((((mv_wirecolour&0x00ff0000)) + ((mv_colour&0x00ff0000)))/2)&0x00ff0000) +
				(((((mv_wirecolour&0x00ff00)) + ((mv_colour&0x00ff00)))/2)&0x00ff00) +
				(((((mv_wirecolour&0x00ff)) + ((mv_colour&0x00ff)))/2)&0x00ff);
			pvc_separator  = 0x00400000;
			pvc_separator2  = 0x00800000;
			pvc_background  = 0x0099766C;
			pvc_background2  = 0x00AA786C;
			pvc_row4beat  = 0x00AF958B;
			pvc_row4beat2 = 0x00AF958B;
			pvc_rowbeat  = 0x00C0ACA5;
			pvc_rowbeat2 = 0x00C0ACA5;
			pvc_row  = 0x00D0C4BF;
			pvc_row2 = 0x00D0C4BF;
			pvc_font  = 0x00000000;
			pvc_font2  = 0x00000000;
			pvc_fontPlay  = 0x00ffffff;
			pvc_fontPlay2  = 0x00ffffff;
			pvc_fontCur  = 0x00ffffff;
			pvc_fontCur2  = 0x00ffffff;
			pvc_fontSel  = 0x00ffffff;
			pvc_fontSel2  = 0x00ffffff;
			pvc_selection  = 0x00e00000;
			pvc_selection2 = 0x00ff5050;
			pvc_playbar  = 0x0080FF00;
			pvc_playbar2 = 0x0080FF00;
			pvc_cursor  = 0x000000e0;
			pvc_cursor2 = 0x000000e0;

			machineGUITopColor = 0x00D2C2BD;		
			machineGUIFontTopColor = 0x00000000; 
			machineGUIBottomColor = 0x009C796D;
			machineGUIFontBottomColor = 0x00FFFFFF; 

			machineGUIHTopColor = 0x00BC94A9;	//highlighted param colours
			machineGUIHFontTopColor = 0x00000000; 
			machineGUIHBottomColor = 0x008B5A72;
			machineGUIHFontBottomColor = 0x0044EEFF; 

			machineGUITitleColor = 0x00000000;
			machineGUITitleFontColor = 0x00FFFFFF;

			vu1 = 0x0080FF80;
			vu2 = 0x00403731;
			vu3 = 0x00262bd7;

			bBmpBkg = false;
			bBmpDial = false;
			szBmpBkgFilename = "No Background Bitmap";
			szBmpDialFilename = "No Dial Bitmap";

		}

#endif //!defined WINAMP_PLUGIN


		bool Configuration::ReadVersion17()
		{
			// Code to import the devices (waveout/DX/Asio) is in their loaders.

			Registry reg;
			Registry::result result;

			if (reg.OpenRootKey(HKEY_CURRENT_USER, "Software\\AAS\\Psycle") != ERROR_SUCCESS)
			{
				return false;
			}
			result = reg.OpenKey("CurrentVersion");
			if ( result != ERROR_SUCCESS) return false;

#if !defined WINAMP_PLUGIN
			reg.QueryValue("NewMacDlgpluginOrder", CNewMachine::pluginOrder);
			reg.QueryValue("NewMacDlgpluginName", CNewMachine::pluginName);
			reg.QueryValue("WrapAround", _wrapAround);
			reg.QueryValue("CenterCursor", _centerCursor);
			reg.QueryValue("FollowSong", _followSong);
			reg.QueryValue("CursorAlwaysDown", _cursorAlwaysDown);
			reg.QueryValue("useDoubleBuffer", useDoubleBuffer);
			reg.QueryValue("DisplayLineNumbers", _linenumbers);
			reg.QueryValue("DisplayLineNumbersHex", _linenumbersHex);
			reg.QueryValue("DisplayLineNumbersCursor", _linenumbersCursor);
			reg.QueryValue("showAboutAtStart", _showAboutAtStart);
			reg.QueryValue("RecordNoteoff", _RecordNoteoff);
			reg.QueryValue("RecordTweaks", _RecordTweaks);
			reg.QueryValue("RecordMouseTweaksSmooth", _RecordMouseTweaksSmooth);
			reg.QueryValue("notesToEffects", _notesToEffects);
			reg.QueryValue("RecordUnarmed", _RecordUnarmed);
			//reg.QueryValue("MoveCursorPaste", _MoveCursorPaste);
			reg.QueryValue("NavigationIgnoresStep", _NavigationIgnoresStep);
			// midi
			{
				reg.QueryValue("MidiMachineViewSeqMode", _midiMachineViewSeqMode);
				// velocity
				{
					reg.QueryValue("MidiRecordVel" , midi().velocity().record() );
					reg.QueryValue("MidiTypeVel"   , midi().velocity().type()   );
					reg.QueryValue("MidiCommandVel", midi().velocity().command());
					reg.QueryValue("MidiFromVel"   , midi().velocity().from()   );
					reg.QueryValue("MidiToVel"     , midi().velocity().to()     );
				}
				// pitch
				{
					reg.QueryValue("MidiRecordPit" , midi().pitch()   .record() );
					reg.QueryValue("MidiTypePit"   , midi().pitch()   .type()   );
					reg.QueryValue("MidiCommandPit", midi().pitch()   .command());
					reg.QueryValue("MidiFromPit"   , midi().pitch()   .from()   );
					reg.QueryValue("MidiToPit"     , midi().pitch()   .to()     );
				}
				for(std::size_t i(0) ; i < midi().groups().size() ; ++i)
				{
					std::ostringstream oss;
					oss << i;
					std::string s(oss.str());
					reg.QueryValue("MidiMessage" + s, midi().group(i).message());
					reg.QueryValue("MidiRecord"  + s, midi().group(i).record() );
					reg.QueryValue("MidiType"    + s, midi().group(i).type()   );
					reg.QueryValue("MidiCommand" + s, midi().group(i).command());
					reg.QueryValue("MidiFrom"    + s, midi().group(i).from()   );
					reg.QueryValue("MidiTo"      + s, midi().group(i).to()     );
				}
				reg.QueryValue("MidiRawMcm", midi().raw());
			}
#endif // !defined WINAMP_PLUGIN
			reg.QueryValue("defaultPatLines", defaultPatLines);
			for(int c(0) ; c < MAX_PATTERNS; ++c)
			{
				// All pattern reset
				Global::_pSong->patternLines[c] = defaultPatLines;
			}
#if !defined WINAMP_PLUGIN

			char savedbyte;
			int savedint;
			reg.QueryValue("bShowSongInfoOnLoad", bShowSongInfoOnLoad);
			reg.QueryValue("bFileSaveReminders", bFileSaveReminders);
			reg.QueryValue("autosaveSong", autosaveSong);
			reg.QueryValue("autosaveSongTime", savedbyte);
			autosaveSongTime = savedbyte;
			reg.QueryValue("mv_colour", mv_colour);
			reg.QueryValue("mv_wirecolour", mv_wirecolour);
			reg.QueryValue("mv_polycolour", mv_polycolour);
			reg.QueryValue("mv_wireaa", savedint);
			mv_wireaa = savedint;
			reg.QueryValue("mv_wirewidth", mv_wirewidth);
			reg.QueryValue("mv_generator_fontcolour", mv_generator_fontcolour);
			reg.QueryValue("mv_effect_fontcolour", mv_effect_fontcolour);
			reg.QueryValue("mv_triangle_size", mv_triangle_size);
			reg.QueryValue("bBmpBkg", bBmpBkg);
			reg.QueryValue("bBmpDial", bBmpDial);
			reg.QueryValue("szBmpBkgFilename", szBmpBkgFilename);
			reg.QueryValue("szBmpDialFilename", szBmpDialFilename);
			reg.QueryValue("pvc_background", pvc_background);
			reg.QueryValue("pvc_background2", pvc_background2);
			reg.QueryValue("pvc_separator", pvc_separator);
			reg.QueryValue("pvc_separator2", pvc_separator2);
			reg.QueryValue("pvc_row4beat", pvc_row4beat);
			reg.QueryValue("pvc_row4beat2", pvc_row4beat2);
			reg.QueryValue("pvc_rowbeat", pvc_rowbeat);
			reg.QueryValue("pvc_rowbeat2", pvc_rowbeat2);
			reg.QueryValue("pvc_row", pvc_row);
			reg.QueryValue("pvc_row2", pvc_row2);
			reg.QueryValue("pvc_font", pvc_font);
			reg.QueryValue("pvc_font2", pvc_font2);
			reg.QueryValue("pvc_fontPlay", pvc_fontPlay);
			reg.QueryValue("pvc_fontPlay2", pvc_fontPlay2);
			reg.QueryValue("pvc_fontCur", pvc_fontCur);
			reg.QueryValue("pvc_fontCur2", pvc_fontCur2);
			reg.QueryValue("pvc_fontSel", pvc_fontSel);
			reg.QueryValue("pvc_fontSel2", pvc_fontSel2);
			reg.QueryValue("pvc_selection", pvc_selection);
			reg.QueryValue("pvc_selection2", pvc_selection2);
			reg.QueryValue("pvc_playbar", pvc_playbar);
			reg.QueryValue("pvc_playbar2", pvc_playbar2);
			reg.QueryValue("pvc_cursor", pvc_cursor);
			reg.QueryValue("pvc_cursor2", pvc_cursor2);
			reg.QueryValue("vu1", vu1);
			reg.QueryValue("vu2", vu2);
			reg.QueryValue("vu3", vu3);
			reg.QueryValue("machineGUITopColor", machineGUITopColor);
			reg.QueryValue("machineGUIFontTopColor", machineGUIFontTopColor);
			reg.QueryValue("machineGUIBottomColor", machineGUIBottomColor);
			reg.QueryValue("machineGUIFontBottomColor", machineGUIFontBottomColor);
			reg.QueryValue("machineGUIHTopColor", machineGUIHTopColor);
			reg.QueryValue("machineGUIHFontTopColor", machineGUIHFontTopColor);
			reg.QueryValue("machineGUIHBottomColor", machineGUIHBottomColor);
			reg.QueryValue("machineGUIHFontBottomColor", machineGUIHFontBottomColor);
			reg.QueryValue("machineGUITitleColor", machineGUITitleColor);
			reg.QueryValue("machineGUITitleFontColor", machineGUITitleFontColor);
			mv_wireaacolour =
				((((mv_wirecolour&0x00ff0000) + ((mv_colour&0x00ff0000)*4))/5)&0x00ff0000) +
				((((mv_wirecolour&0x00ff00) + ((mv_colour&0x00ff00)*4))/5)&0x00ff00) +
				((((mv_wirecolour&0x00ff) + ((mv_colour&0x00ff)*4))/5)&0x00ff);
			mv_wireaacolour2 =
				(((((mv_wirecolour&0x00ff0000)) + ((mv_colour&0x00ff0000)))/2)&0x00ff0000) +
				(((((mv_wirecolour&0x00ff00)) + ((mv_colour&0x00ff00)))/2)&0x00ff00) +
				(((((mv_wirecolour&0x00ff)) + ((mv_colour&0x00ff)))/2)&0x00ff);
			reg.QueryValue("pattern_fontface", pattern_fontface);
			reg.QueryValue("pattern_header_skin", pattern_header_skin);
			reg.QueryValue("pattern_font_point", pattern_font_point);
			reg.QueryValue("pattern_font_x", pattern_font_x);
			reg.QueryValue("pattern_font_y", pattern_font_y);
			reg.QueryValue("pattern_font_flags", pattern_font_flags);
			reg.QueryValue("pattern_draw_empty_data", pattern_draw_empty_data);
			reg.QueryValue("draw_mac_index", draw_mac_index);
			reg.QueryValue("draw_vus", draw_vus);
			reg.QueryValue("generator_fontface", generator_fontface);
			reg.QueryValue("generator_font_point", generator_font_point);
			reg.QueryValue("generator_font_flags", generator_font_flags);
			reg.QueryValue("effect_fontface", effect_fontface);
			reg.QueryValue("effect_font_point", effect_font_point);
			reg.QueryValue("effect_font_flags", effect_font_flags);
			reg.QueryValue("machine_skin", machine_skin);
			CreateFonts();
			// soundcard output device
			{
				reg.QueryValue("OutputDriver", _outputDriverIndex);
				if(0 > _outputDriverIndex || _outputDriverIndex >= _numOutputDrivers) _outputDriverIndex = 1;
				_pOutputDriver = _ppOutputDrivers[_outputDriverIndex];

			}
			// midi
			{
				{
					reg.QueryValue("MidiInputDriver", _midiDriverIndex);
					if(0 > _midiDriverIndex || _midiDriverIndex > _numMidiDrivers) _midiDriverIndex = 0;
					CMidiInput::Instance()->SetDeviceId(DRIVER_MIDI, _midiDriverIndex - 1);
				}
				{
					reg.QueryValue("MidiSyncDriver", _syncDriverIndex);
					if(0 > _syncDriverIndex || _syncDriverIndex > _numMidiDrivers) _syncDriverIndex = 0;
					CMidiInput::Instance()->SetDeviceId(DRIVER_SYNC, _syncDriverIndex - 1);
				}
				{
					reg.QueryValue("MidiInputHeadroom", _midiHeadroom);
					CMidiInput::Instance()->GetConfigPtr()->midiHeadroom = _midiHeadroom;
				}
			}
#endif // !defined WINAMP_PLUGIN
			// paths
			{
				reg.QueryValue("InstrumentDir", instrument_dir_);
				SetCurrentInstrumentDir(GetInstrumentDir());
				reg.QueryValue("SongDir", song_dir_);
				SetCurrentSongDir(GetSongDir());
				reg.QueryValue("SkinDir", skin_dir_);
				reg.QueryValue("PluginDir", plugin_dir_);
				reg.QueryValue("VstDir", vst_dir_);
			}
			reg.CloseKey();
			reg.CloseRootKey();
			return true;
		}
	}
}
