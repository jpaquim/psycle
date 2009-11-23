/***************************************************************************
*   Copyright (C) 2006 by  Stefan   *
*   natti@linux   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#include "configuration.h"
#include <psycore/song.h>
#include "defaultbitmaps.h"
#include <ngrs/file.h>
#include <ngrs/keyevent.h>
#include <sys/stat.h>
#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <exception>

#if defined XPSYCLE__CONFIGURATION
#include <xpsycle/install_paths.h>
#endif

namespace psy {
	namespace host {

		Configuration::Configuration()
		{
			#if !defined XPSYCLE__CONFIGURATION
			std::cout << "xpsycle: warning: built without configuration" << std::endl;
			#endif
			setXmlDefaults();
			setSkinDefaults();
			loadConfig();			
			_RecordTweaks = false;
			_RecordUnarmed = true;
		}


		Configuration::~Configuration()
		{			
		}

		const std::string & Configuration::iconPath() const {
			return iconPath_;
		}

		const std::string & Configuration::pluginPath() const {
			return pluginPath_;
		}

		const std::string & Configuration::prsPath() const {
			return prsPath_;
		}

		const std::string & Configuration::hlpPath() const {
			return hlpPath_;
		}

		const std::string & Configuration::ladspaPath() const {
			return ladspaPath_;
		}

		bool Configuration::enableSound() const {
			return enableSound_;
		}

		void Configuration::setXmlDefaults() {     
			std::string xml_mem_;   

			xml_mem_ =  " <!-- xpsycle configuration file --> ";
			xml_mem_ += " <xpsycle> ";
			xml_mem_ += " <path id='plugindir'";
			xml_mem_ += " src='c:\\Programme\\Psycle\\PsyclePlugins' />";

			//			xml_mem_ += " <audio enable='1' />";
			//			xml_mem_ += " <driver name='mswaveout' />"; 

			/*  xml_mem_ += " <!-- help configuration -->";
			//  xml_mem_ += " <path id='hlpdir' src='~/xpsycle/doc/' />";

			xml_mem_ += " <!-- audio configuration -->";


			xml_mem_ += " <!-- gui configuration -->";

			//  xml_mem_ += " <path id='icondir' src='~/xpsycle/icons/' />";

			//  xml_mem_ += " <path id='prsdir' src='~/.xpsycle/prs/'></path>";

			xml_mem_ += " <!-- keyHandler configuration -->";
			xml_mem_ += " <!-- general keys -->";*/

			xml_mem_ += " <key id='current_machine-1' mod='ctrl' keychar='NK_Left' />";
			xml_mem_ += " <key id='current_machine+1' mod='ctrl' keychar='NK_Right' />";
			xml_mem_ += " <key id='current_instrument-1' mod='ctrl' keychar='NK_Up' />";
			xml_mem_ += " <key id='current_instrument+1' mod='ctrl' keychar='NK_Down' />";

			xml_mem_ += " <key id='screen_machines' keychar='F2' />";
			xml_mem_ += " <key id='screen_patterns' keychar='F3' />";
			xml_mem_ += " <key id='edit_instrument' keychar='F4' />";
			xml_mem_ += " <key id='screen_sequencer' keychar='F5' />";
			xml_mem_ += " <key id='play_song_start' keychar='F6' />";
			xml_mem_ += " <key id='play_song_normal' keychar='F7' />";
			xml_mem_ += " <key id='stop_playback' keychar='F8' />";
			xml_mem_ += " <key id='add_new_machine' keychar='F9' />";

			xml_mem_ += " <!-- patternview octave 0  -->";

			xml_mem_ += " <key id='oct_C_0' keychar='Y'/>";
			xml_mem_ += " <key id='oct_CS0' keychar='S'/>";
			xml_mem_ += " <key id='oct_D_0' keychar='X'/>";
			xml_mem_ += " <key id='oct_DS0' keychar='D'/>";
			xml_mem_ += " <key id='oct_E_0' keychar='C'/>";
			xml_mem_ += " <key id='oct_F_0' keychar='V'/>";
			xml_mem_ += " <key id='oct_FS0' keychar='G'/>";
			xml_mem_ += " <key id='oct_G_0' keychar='B'/>";
			xml_mem_ += " <key id='oct_GS0' keychar='H'/>";
			xml_mem_ += " <key id='oct_A_0' keychar='N'/>";
			xml_mem_ += " <key id='oct_AS0' keychar='J'/>";
			xml_mem_ += " <key id='oct_B_0' keychar='M'/>";

			xml_mem_ += " <!-- patternview octave 1  -->";

			xml_mem_ += " <key id='oct_C_1' keychar='Q'/>";
			xml_mem_ += " <key id='oct_CS1' keychar='2'/>";
			xml_mem_ += " <key id='oct_D_1' keychar='W'/>";
			xml_mem_ += " <key id='oct_DS1' keychar='3'/>";
			xml_mem_ += " <key id='oct_E_1' keychar='E'/>";
			xml_mem_ += " <key id='oct_F_1' keychar='R'/>";
			xml_mem_ += " <key id='oct_FS1' keychar='5'/>";
			xml_mem_ += " <key id='oct_G_1' keychar='T'/>";
			xml_mem_ += " <key id='oct_GS1' keychar='6'/>";
			xml_mem_ += " <key id='oct_A_1' keychar='Z'/>";
			xml_mem_ += " <key id='oct_AS1' keychar='7'/>";
			xml_mem_ += " <key id='oct_B_1' keychar='U'/>";

			xml_mem_ += " <!-- patternview octave 2  -->";

			xml_mem_ += " <key id='oct_C_2' keychar='I' />";
			xml_mem_ += " <key id='oct_CS2' keychar='9' />";
			xml_mem_ += " <key id='oct_D_2' keychar='O' />";
			xml_mem_ += " <key id='oct_DS2' keychar='0' />";

			xml_mem_ += " <!-- patternview misc key options -->";

			xml_mem_ += " <key id='edit_toggle' mod='ctrl' keychar=' ' />";
			xml_mem_ += " <key id='key_stop' keychar='1' />";
			xml_mem_ += " <key id='current_octave-1' mod='ctrl' keychar='+' />";
			xml_mem_ += " <key id='current_octave+1' mod='ctrl' keychar='#' />";
			xml_mem_ += " <key id='patternstep_dec' keychar='[' />";
			xml_mem_ += " <key id='patternstep_inc' keychar=']' />";

			xml_mem_ += " <!-- patternview navigation key options -->";
			xml_mem_ += " <key id='nav_left' keychar='NK_Left' />";
			xml_mem_ += " <key id='nav_right' keychar='NK_Right' />";
			xml_mem_ += " <key id='nav_up' keychar='NK_Up' />";
			xml_mem_ += " <key id='nav_down' keychar='NK_Down' />";
			xml_mem_ += " <key id='nav_up_16' keychar='NK_Page_Up' />";
			xml_mem_ += " <key id='nav_down_16' keychar='NK_Page_Down' />";
			xml_mem_ += " <key id='nav_top' keychar='NK_Home' />";
			xml_mem_ += " <key id='nav_bottom' keychar='NK_End' />";
			xml_mem_ += " <key id='next_column' keychar='NK_Tab' />";
			xml_mem_ += " <key id='prev_column' keychar='XK_ISO_Left_Tab' />";
			xml_mem_ += " <key id='delete_row' keychar='NK_Delete' />";
			xml_mem_ += " <key id='insert_row' keychar='NK_Insert' />";
			xml_mem_ += " <key id='clear_row' keychar='NK_BackSpace' />";

			xml_mem_ += " <!-- patternview block key options -->";

			xml_mem_ += " <key id='block_copy' mod='ctrl' keychar='C' />";
			xml_mem_ += " <key id='block_cut' mod='ctrl' keychar='X' />";
			xml_mem_ += " <key id='block_delete' mod='ctrl' keychar='Y' />";
			xml_mem_ += " <key id='block_double' mod='ctrl' keychar='D' />";
			xml_mem_ += " <key id='block_halve' mod='ctrl' keychar='H' />";
			xml_mem_ += " <key id='block_interpolate' mod='ctrl' keychar='I' />";
			xml_mem_ += " <key id='block_mix' mod='ctrl' keychar='M' />";
			xml_mem_ += " <key id='block_paste' mod='ctrl' keychar='V' />";
			xml_mem_ += " <key id='block_select_all' mod='ctrl' keychar='A' />";
			xml_mem_ += " <key id='block_select_bar' mod='ctrl' keychar='E' />";
			xml_mem_ += " <key id='block_select_column' mod='ctrl' keychar='R' />";
			xml_mem_ += " <key id='block_select_up' mod='shift' keychar='NK_Up' />";
			xml_mem_ += " <key id='block_select_down' mod='shift' keychar='NK_Down' />";
			xml_mem_ += " <key id='block_select_left' mod='shift' keychar='NK_Left' />";
			xml_mem_ += " <key id='block_select_right' mod='shift' keychar='NK_Right' />";
			xml_mem_ += " <key id='block_select_top' mod='shift' keychar='NK_Home' />";
			xml_mem_ += " <key id='block_select_bottom' mod='shift' keychar='NK_End' />";
			xml_mem_ += " <key id='block_set_instrument' mod='ctrl' keychar='T' />";
			xml_mem_ += " <key id='block_set_machine' mod='ctrl' keychar='G' />";
			xml_mem_ += " <key id='block_start' mod='ctrl' keychar='B' />";
			xml_mem_ += " <key id='block_start' mod='ctrl' keychar='K' />";
			xml_mem_ += " <key id='block_unmark' mod='ctrl' keychar='U' />";

			xml_mem_ += " </xpsycle> ";

			const char* pcLADSPAPath = std::getenv("LADSPA_PATH");
			if ( !pcLADSPAPath) {
				#ifdef __unix__
				pcLADSPAPath = "/usr/lib/ladspa/";
				ladspaPath_ = pcLADSPAPath;
				#else
				ladspaPath_ = std::getenv("ProgramFiles");
				ladspaPath_ += "\\Audacity\\Plug-Ins";
				#endif
			}
			else 
				ladspaPath_ = pcLADSPAPath;

			ngrs::XmlParser parser;
			parser.tagParse.connect(this,&Configuration::onConfigTagParse);
			parser.parseString ( xml_mem_ );

		}     

		void Configuration::setSkinDefaults( )
		{
			setDriverByName("silent");
			enableSound_ = false;

#if defined XPSYCLE__CONFIGURATION
#include <xpsycle/install_paths.h>
			hlpPath_ = XPSYCLE__INSTALL_PATHS__DOC "/";
			iconPath_ = XPSYCLE__INSTALL_PATHS__PIXMAPS "/";
			pluginPath_ = XPSYCLE__INSTALL_PATHS__PLUGINS "/";
			prsPath_ = XPSYCLE__INSTALL_PATHS__PRESETS "/";
#else
			// we don't have any information about the installation paths,
			// so, we can only assume everything is at a fixed place, like under the user home dir
			hlpPath_ = ngrs::File::replaceTilde("~/xpsycle/doc/");
			iconPath_ = ngrs::File::replaceTilde("~/xpsycle/pixmaps/");
#ifdef __unix__
			pluginPath_ = ngrs::File::replaceTilde("~/xpsycle/plugins/");
#else
			pluginPath_ = std::getenv("ProgramFiles");
			pluginPath_ += "\\Psycle\\PsyclePlugins\\";
#endif


			prsPath_ =  ngrs::File::replaceTilde("~" + ngrs::File::slash() + "xpsycle" + ngrs::File::slash() +"prs" + ngrs::File::slash() );
#endif

			#if !defined NDEBUG
			std::cout
				<< "xpsycle: configuration: initial defaults:\n"
				<< "xpsycle: configuration: pixmap dir: " << iconPath_ << "\n"
				<< "xpsycle: configuration: plugin dir: " << pluginPath_ << "\n"
				<< "xpsycle: configuration: preset dir: " << prsPath_ << "\n"
				<< "xpsycle: configuration: doc    dir: " << hlpPath_ << "\n"
				<< "xpsycle: configuration: ladspa dir: " << ladspaPath_ << "\n";
			#endif

		}

		void Configuration::setDriverByName( const std::string & driverName )
		{
		}


		void Configuration::loadConfig()
		{
			// system-wide

			// environment
			// this is most useful for developpers.
			// you can test xpsycle directly from within the build dir,
			// pointing various paths to the source or build dir.
			char const * const xpsycleenv(std::getenv("XPSYCLE__CONFIGURATION"));
			std::string path= (xpsycleenv?xpsycleenv:"");
			if(path.length()!=0) {
				try {
					loadConfig(path);
				}
				catch(std::exception const & e)
				{
					std::cerr << "xpsycle: configuration: error: " << e.what() << std::endl;
				}
			} else {
							path = ngrs::File::replaceTilde("~" + ngrs::File::slash() + ".xpsycle.xml");
			  if (path.length()!=0) {
					try {
											loadConfig( ngrs::File::replaceTilde( "~" + ngrs::File::slash() + ".xpsycle.xml") );
					}
					catch( std::exception const & e ) {
						std::cerr << "xpsycle: configuration: error: " << e.what() << std::endl;
					}
				} else {

					/*  	path=XPSYCLE__INSTALL_PATHS__CONFIGURATION "/xpsycle.xml";
					if (path.length()!=0){
					try {
					loadConfig(path);
					}
					catch(std::exception const & e)
					{
					std::cerr << "xpsycle: configuration: error: " << e.what() << std::endl;
					}
					}*/
				}
			}
		}

		void Configuration::loadConfig( const std::string & path )
		{
#if !defined NDEBUG
			std::cout << "xpsycle: configuration: attempting to load file: " << path << std::endl;
#endif

						ngrs::XmlParser parser;   
			parser.tagParse.connect(this,&Configuration::onConfigTagParse);

			// check whether the file is readable
						if( !ngrs::File::fileIsReadable( path ) )
			{
				std::ostringstream s;
				s << "cannot read file: " << path;
				throw std::runtime_error(s.str());
			}

			// parse the file
			try
			{
				parser.parseFile( path );
			}
			catch( std::exception const & e )
			{
				std::ostringstream s;
				s <<
					"an error occured while parsing xml configuration file " << path << " ; "
					"the settings might have been only partially loaded ; "
					"message: " << e.what();
				throw std::runtime_error(s.str());
			} catch(...)
			{
				std::ostringstream s;
				s <<
					"an unidentified error occured while parsing xml configuration file " << path << " ; "
					"the settings might have been only partially loaded";
				throw std::runtime_error(s.str());
			}

#if !defined NDEBUG
			std::cout
				<< "xpsycle: configuration: after loading file: " << path << "\n"
				<< "xpsycle: configuration: pixmap dir: " << iconPath_ << "\n"
				<< "xpsycle: configuration: plugin dir: " << pluginPath_ << "\n"
				<< "xpsycle: configuration: preset dir: " << prsPath_ << "\n"
				<< "xpsycle: configuration: doc    dir: " << hlpPath_ << "\n"
				<< "xpsycle: configuration: ladspa dir: " << ladspaPath_ << "\n";
#endif

			doEnableSound = true;
		}

		void Configuration::onConfigTagParse( const ngrs::XmlParser & parser, const std::string & tagName )
		{
			if ( tagName == "path" ) {
				std::string id  = parser.getAttribValue("id"); 
				std::string src = parser.getAttribValue("src");

				if ( id == "icondir" )   iconPath_   = src;
									else  
				if ( id == "plugindir" ) pluginPath_ = src;
									else
				if ( id == "prsdir" )    prsPath_    = src;
									else
				if ( id == "hlpdir" )    hlpPath_    = src; 
									else
				if ( id == "ladspadir" ) ladspaPath_ = src;
			} else
				if (tagName == "driver" && doEnableSound) {		
					setDriverByName( parser.getAttribValue("name"));
				} else
					if (tagName == "alsa") {
						/*std::string deviceName = parser.getAttribValue("device");
						std::map< std::string, AudioDriver*>::iterator it = driverMap_.begin();
						if ( ( it = driverMap_.find( "alsa" ) ) != driverMap_.end() ) {
							AudioDriverSettings settings = it->second->settings();
							settings.setDeviceName( deviceName );
							it->second->setSettings( settings );
						}		*/
											} else
						if (tagName == "audio") {
							std::string enableStr = parser.getAttribValue("enable");
							int enable = 0;
														if ( enableStr != "" ) enable = ngrs::str<int>(enableStr);
							enableSound_ = enable;
							if (enable == 0) {
								setDriverByName( "silent" );
								doEnableSound = false;
							} else doEnableSound = true;
						} else
							if (tagName == "key") {

								// the keycode id 
								std::string id         = parser.getAttribValue("id");

								// define special chars pressed in addition to this key function
								std::string modInput    = parser.getAttribValue("mod");
																int shift = ngrs::nsNone;
								if (modInput == "ctrl")  {
																	shift = ngrs::nsCtrl;   
								} else
								if (modInput == "shift") {
																	shift = ngrs::nsShift;      
								}

								// the keycode      
								int keyCode = 0;
								std::string keyCodeStr = parser.getAttribValue("keycode");
								if (keyCodeStr!="") keyCode = ngrs::str<int>(keyCodeStr);
								std::string keyCharStr = parser.getAttribValue("keychar");

								if (keyCharStr!="") {
									keyCode = keyCharStr[0];
									if ( keyCharStr == "NK_Left" )    keyCode = ngrs::NK_Left; else
									if ( keyCharStr == "NK_Right")    keyCode = ngrs::NK_Right; else
									if ( keyCharStr == "NK_Up" )       keyCode = ngrs::NK_Up; else
									if ( keyCharStr == "NK_Down" )     keyCode = ngrs::NK_Down; else
									if ( keyCharStr == "NK_Page_Up" )  keyCode = ngrs::NK_Page_Up; else
									if ( keyCharStr == "NK_Page_Down" ) keyCode = ngrs::NK_Page_Down; else
									if ( keyCharStr == "NK_Home" )      keyCode = ngrs::NK_Home; else
									if ( keyCharStr == "NK_End" )       keyCode = ngrs::NK_End; else
									if ( keyCharStr == "NK_Tab" )       keyCode = ngrs::NK_Tab; else
#ifdef __unix__
									if ( keyCharStr == "XK_ISO_Left_Tab" ) keyCode = XK_ISO_Left_Tab; else
#endif
									if ( keyCharStr == "NK_Insert" ) keyCode = ngrs::NK_Insert; else
									if ( keyCharStr == "NK_BackSpace" ) keyCode = ngrs::NK_BackSpace; else
									if ( keyCharStr == "NK_Delete" ) keyCode = ngrs::NK_Delete; else
									if ( keyCharStr == "F1") keyCode = ngrs::NK_F1; else
									if ( keyCharStr == "F2") keyCode = ngrs::NK_F2; else
									if ( keyCharStr == "F3") keyCode = ngrs::NK_F3; else
									if ( keyCharStr == "F4") keyCode = ngrs::NK_F4; else
									if ( keyCharStr == "F5") keyCode = ngrs::NK_F5; else
									if ( keyCharStr == "F6") keyCode = ngrs::NK_F6; else
									if ( keyCharStr == "F7") keyCode = ngrs::NK_F7; else
									if (keyCharStr == "F8") keyCode = ngrs::NK_F8; else
									if (keyCharStr == "F9") keyCode = ngrs::NK_F9;
																}
	if (id == "add_new_machine") {
	inputHandler_.changeKeyCode(cdefAddMachine,Key(shift,keyCode));
	} else
if (id == "block_copy") {
											inputHandler_.changeKeyCode(cdefBlockCopy,Key(shift,keyCode));
										} else
											if (id == "block_cut") {
												inputHandler_.changeKeyCode(cdefBlockCut,Key(shift,keyCode));
											} else
												if (id == "block_delete") {
													inputHandler_.changeKeyCode(cdefBlockDelete,Key(shift,keyCode));
												} else
													if (id == "block_double") {
														inputHandler_.changeKeyCode(cdefBlockDouble,Key(shift,keyCode));
													} else
														if (id == "block_end") {
															inputHandler_.changeKeyCode(cdefBlockEnd,Key(shift,keyCode));
														} else
															if (id == "block_halve") {
																inputHandler_.changeKeyCode(cdefBlockHalve,Key(shift,keyCode));
															} else
																if (id == "block_interpolate") {
																	inputHandler_.changeKeyCode(cdefBlockInterpolate,Key(shift,keyCode));
																} else
																	if (id == "block_mix") {
																		inputHandler_.changeKeyCode(cdefBlockMix,Key(shift,keyCode));
																	} else
																		if (id == "block_paste") {
																			inputHandler_.changeKeyCode(cdefBlockPaste,Key(shift,keyCode));
																		} else
																			if (id == "block_select_all") {
																				inputHandler_.changeKeyCode(cdefSelectAll,Key(shift,keyCode));
																			} else
																				if (id == "block_select_bar") {
																					inputHandler_.changeKeyCode(cdefSelectBar,Key(shift,keyCode));
																				} else
																					if (id == "block_select_column") {
																						inputHandler_.changeKeyCode(cdefSelectCol,Key(shift,keyCode));
																					} else
																						if (id == "block_select_up") {
																							inputHandler_.changeKeyCode(cdefSelectUp,Key(shift,keyCode));
																						} else
																							if (id == "block_select_down") {
																								inputHandler_.changeKeyCode(cdefSelectDn,Key(shift,keyCode));
																							} else
																								if (id == "block_select_left") {
																									inputHandler_.changeKeyCode(cdefSelectLeft,Key(shift,keyCode));
																								} else
																									if (id == "block_select_right") {
																										inputHandler_.changeKeyCode(cdefSelectRight,Key(shift,keyCode));
																									} else
																										if (id == "block_select_top") {
																											inputHandler_.changeKeyCode(cdefSelectTop,Key(shift,keyCode));
																										} else
																											if (id == "block_select_bottom") {
																												inputHandler_.changeKeyCode(cdefSelectBottom,Key(shift,keyCode));
																											} else
																												if (id == "block_set_instrument") {
																													inputHandler_.changeKeyCode(cdefBlockSetInstr,Key(shift,keyCode));
																												} else
																													if (id == "block_set_machine") {
																														inputHandler_.changeKeyCode(cdefBlockSetMachine,Key(shift,keyCode));
																													} else
																														if (id == "block_start") {
																															inputHandler_.changeKeyCode(cdefBlockStart,Key(shift,keyCode));
																														} else
																															if (id == "block_unmark") {
																																inputHandler_.changeKeyCode(cdefBlockUnMark,Key(shift,keyCode));
																															} else
																																if (id == "clear_row") {
																																	inputHandler_.changeKeyCode(cdefRowClear,Key(shift,keyCode));
																																} else
																																	if (id == "current_instrument+1") {
																																		inputHandler_.changeKeyCode(cdefInstrInc,Key(shift,keyCode));
																																	} else
																																		if (id == "current_instrument-1") {
																																			inputHandler_.changeKeyCode(cdefInstrDec,Key(shift,keyCode));
																																		} else
																																			if (id == "current_machine+1") {
																																				inputHandler_.changeKeyCode(cdefMachineInc,Key(shift,keyCode));
																																			} else
																																				if (id == "current_machine-1") {
																																					inputHandler_.changeKeyCode(cdefMachineDec,Key(shift,keyCode));
																																				} else
																																					if (id == "current_octave+1") {
																																						inputHandler_.changeKeyCode(cdefOctaveUp,Key(shift,keyCode));
																																					} else
																																						if (id == "current_octave-1") {
																																							inputHandler_.changeKeyCode(cdefOctaveDn,Key(shift,keyCode));
																																						} else
																																							if (id == "current_pattern+1") {
																																								inputHandler_.changeKeyCode(cdefPatternInc,Key(shift,keyCode));
																																							} else
																																								if (id == "current_pattern-1") {
																																									inputHandler_.changeKeyCode(cdefPatternDec,Key(shift,keyCode));
																																								} else
																																									if (id == "delete_row") {
																																										inputHandler_.changeKeyCode(cdefRowDelete,Key(shift,keyCode));
																																									} else
																																										if (id == "edit_instrument") {
																																											inputHandler_.changeKeyCode(cdefEditInstr,Key(shift,keyCode));
																																										} else
																																											if (id == "edit_redo") {
																																												inputHandler_.changeKeyCode(cdefRedo,Key(shift,keyCode));
																																											} else
																																												if (id == "edit_undo") {
																																													inputHandler_.changeKeyCode(cdefUndo,Key(shift,keyCode));
																																												} else
																																													if (id == "edit_toggle") {
																																														inputHandler_.changeKeyCode(cdefEditToggle,Key(shift,keyCode));
																																													} else
																																														if (id.find("oct")!=std::string::npos) {
																																															if (id == "oct_C_0") {
																																																inputHandler_.changeKeyCode(cdefKeyC_0,Key(shift,keyCode));
																																															} else
																																																if (id == "oct_CS0") {
																																																	inputHandler_.changeKeyCode(cdefKeyCS0,Key(shift,keyCode));
																																																} else
																																																	if (id == "oct_D_0") {
																																																		inputHandler_.changeKeyCode(cdefKeyD_0,Key(shift,keyCode));
																																																	} else
																																																		if (id == "oct_DS0") {
																																																			inputHandler_.changeKeyCode(cdefKeyDS0,Key(shift,keyCode));
																																																		} else
																																																			if (id == "oct_E_0") {
																																																				inputHandler_.changeKeyCode(cdefKeyE_0,Key(shift,keyCode));
																																																			} else
																																																				if (id == "oct_F_0") {
																																																					inputHandler_.changeKeyCode(cdefKeyF_0,Key(shift,keyCode));
																																																				} else
																																																					if (id == "oct_FS0") {
																																																						inputHandler_.changeKeyCode(cdefKeyFS0,Key(shift,keyCode));
																																																					} else
																																																						if (id == "oct_G_0") {
																																																							inputHandler_.changeKeyCode(cdefKeyG_0,Key(shift,keyCode));
																																																						} else
																																																							if (id == "oct_GS0") {
																																																								inputHandler_.changeKeyCode(cdefKeyGS0,Key(shift,keyCode));
																																																							} else
																																																								if (id == "oct_A_0") {
																																																									inputHandler_.changeKeyCode(cdefKeyA_0,Key(shift,keyCode));
																																																								} else
																																																									if (id == "oct_AS0") {
																																																										inputHandler_.changeKeyCode(cdefKeyAS0,Key(shift,keyCode));
																																																									} else
																																																										if (id == "oct_B_0") {
																																																											inputHandler_.changeKeyCode(cdefKeyB_0,Key(shift,keyCode));
																																																										}  else // and now the all again for octave 2
																																																											if (id == "oct_C_1") {
																																																												inputHandler_.changeKeyCode(cdefKeyC_1,Key(shift,keyCode));
																																																											} else
																																																												if (id == "oct_CS1") {
																																																													inputHandler_.changeKeyCode(cdefKeyCS1,Key(shift,keyCode));
																																																												} else
																																																													if (id == "oct_D_1") {
																																																														inputHandler_.changeKeyCode(cdefKeyD_1,Key(shift,keyCode));
																																																													} else
																																																														if (id == "oct_DS1") {
																																																															inputHandler_.changeKeyCode(cdefKeyDS1,Key(shift,keyCode));
																																																														} else
																																																															if (id == "oct_E_1") {
																																																																inputHandler_.changeKeyCode(cdefKeyE_1,Key(shift,keyCode));
																																																															} else
																																																																if (id == "oct_F_1") {
																																																																	inputHandler_.changeKeyCode(cdefKeyF_1,Key(shift,keyCode));
																																																																} else
																																																																	if (id == "oct_FS1") {
																																																																		inputHandler_.changeKeyCode(cdefKeyFS1,Key(shift,keyCode));
																																																																	} else
																																																																		if (id == "oct_G_1") {
																																																																			inputHandler_.changeKeyCode(cdefKeyG_1,Key(shift,keyCode));
																																																																		} else
																																																																			if (id == "oct_GS1") {
																																																																				inputHandler_.changeKeyCode(cdefKeyGS1,Key(shift,keyCode));
																																																																			} else
																																																																				if (id == "oct_A_1") {
																																																																					inputHandler_.changeKeyCode(cdefKeyA_1,Key(shift,keyCode));
																																																																				} else
																																																																					if (id == "oct_AS1") {
																																																																						inputHandler_.changeKeyCode(cdefKeyAS1,Key(shift,keyCode));
																																																																					} else
																																																																						if (id == "oct_B_1") {
																																																																							inputHandler_.changeKeyCode(cdefKeyB_1,Key(shift,keyCode));
																																																																						} else // and now again for octave 2
																																																																							if (id == "oct_C_2") {
																																																																								inputHandler_.changeKeyCode(cdefKeyC_2,Key(shift,keyCode));
																																																																							} else
																																																																								if (id == "oct_CS2") {
																																																																									inputHandler_.changeKeyCode(cdefKeyCS2,Key(shift,keyCode));
																																																																								} else
																																																																									if (id == "oct_D_2") {
																																																																										inputHandler_.changeKeyCode(cdefKeyD_2,Key(shift,keyCode));
																																																																									} else
																																																																										if (id == "oct_DS2") {
																																																																											inputHandler_.changeKeyCode(cdefKeyDS2,Key(shift,keyCode));
																																																																										} else
																																																																											if (id == "oct_E_2") {
																																																																												inputHandler_.changeKeyCode(cdefKeyE_2,Key(shift,keyCode));
																																																																											} else
																																																																												if (id == "oct_F_2") {
																																																																													inputHandler_.changeKeyCode(cdefKeyF_2,Key(shift,keyCode));
																																																																												} else
																																																																													if (id == "oct_FS2") {
																																																																														inputHandler_.changeKeyCode(cdefKeyFS2,Key(shift,keyCode));
																																																																													} else
																																																																														if (id == "oct_G_2") {
																																																																															inputHandler_.changeKeyCode(cdefKeyG_2,Key(shift,keyCode));
																																																																														} else
																																																																															if (id == "oct_GS2") {
																																																																																inputHandler_.changeKeyCode(cdefKeyGS2,Key(shift,keyCode));
																																																																															} else
																																																																																if (id == "oct_A_2") {
																																																																																	inputHandler_.changeKeyCode(cdefKeyA_2,Key(shift,keyCode));
																																																																																}
																																														} else
																																															if (id == "key_stop") {
																																																inputHandler_.changeKeyCode(cdefKeyStop,Key(shift,keyCode));
																																															} else
																																																if (id == "key_stop_current") {
																																																	inputHandler_.changeKeyCode(cdefKeyStopAny,Key(shift,keyCode)); // not sure or key_stop ?
																																																}if (id == "machine_info") {
																																																	inputHandler_.changeKeyCode(cdefInfoMachine,Key(shift,keyCode));
																																																} else
																																																	if (id == "maximise_pattern_view") {
																																																		inputHandler_.changeKeyCode(cdefMaxPattern,Key(shift,keyCode));
																																																	} else
																																																		if (id == "mcm_midi_cc") {
																																																			inputHandler_.changeKeyCode(cdefMIDICC,Key(shift,keyCode));
																																																		} else
																																																			if (id == "nav_bottom") {
																																																				inputHandler_.changeKeyCode(cdefNavBottom,Key(shift,keyCode));
																																																			} else
																																																				if (id == "nav_down") {
																																																					inputHandler_.changeKeyCode(cdefNavDn,Key(shift,keyCode));
																																																				} else
																																																					if (id == "nav_down_16") {
																																																						inputHandler_.changeKeyCode(cdefNavPageDn,Key(shift,keyCode));
																																																					} else
																																																						if (id == "nav_left") {
																																																							inputHandler_.changeKeyCode(cdefNavLeft,Key(shift,keyCode));
																																																						} else
																																																							if (id == "nav_right") {
																																																								inputHandler_.changeKeyCode(cdefNavRight,Key(shift,keyCode));
																																																							} else
																																																								if (id == "nav_top") {
																																																									inputHandler_.changeKeyCode(cdefNavTop,Key(shift,keyCode));
																																																								} else
																																																									if (id == "nav_up") {
																																																										inputHandler_.changeKeyCode(cdefNavUp,Key(shift,keyCode));
																																																									} else
																																																										if (id == "nav_up_16") {
																																																											inputHandler_.changeKeyCode(cdefNavPageUp,Key(shift,keyCode));
																																																										} else
																																																											if (id == "nav_firsttrack") {
																																																												inputHandler_.changeKeyCode(cdefNavFirstTrack,Key(shift,keyCode));
																																																											} else
																																																												if (id == "nav_lasttrack") {
																																																													inputHandler_.changeKeyCode(cdefNavLastTrack,Key(shift,keyCode));
																																																												} else
																																																													if (id == "next_column") {
																																																														inputHandler_.changeKeyCode(cdefColumnNext,Key(shift,keyCode));
																																																													} else
																																																														if (id == "pattern_copy") {
																																																															inputHandler_.changeKeyCode(cdefPatternCopy,Key(shift,keyCode));
																																																														} else
																																																															if (id == "pattern_cut") {
																																																																inputHandler_.changeKeyCode(cdefPatternCut,Key(shift,keyCode));
																																																															} else
																																																																if (id == "pattern_delete") {
																																																																	inputHandler_.changeKeyCode(cdefPatternDelete,Key(shift,keyCode));
																																																																} else
																																																																	if (id == "pattern_info") {
																																																																		inputHandler_.changeKeyCode(cdefInfoPattern,Key(shift,keyCode));
																																																																	} else
																																																																		if (id == "pattern_mix_paste") {
																																																																			inputHandler_.changeKeyCode(cdefPatternMixPaste,Key(shift,keyCode));
																																																																		} else
																																																																			if (id == "pattern_paste") {
																																																																				inputHandler_.changeKeyCode(cdefPatternPaste,Key(shift,keyCode));
																																																																			} else
																																																																				if (id == "pattern_track_mute") {
																																																																					inputHandler_.changeKeyCode(cdefPatternTrackMute,Key(shift,keyCode));
																																																																				} else
																																																																					if (id == "pattern_track_record") {
																																																																						inputHandler_.changeKeyCode(cdefPatternTrackRecord,Key(shift,keyCode));
																																																																					} else
																																																																						if (id == "pattern_track_solo") {
																																																																							inputHandler_.changeKeyCode(cdefPatternTrackSolo,Key(shift,keyCode));
																																																																						} else
																																																																							if (id == "patternstep_dec") {
																																																																								inputHandler_.changeKeyCode(cdefPatternstepDec,Key(shift,keyCode));
																																																																							} else
																																																																								if (id == "patternstep_inc") {
																																																																									inputHandler_.changeKeyCode(cdefPatternstepInc,Key(shift,keyCode));
																																																																								} else
																																																																									if (id == "play_current_note") {
																																																																										inputHandler_.changeKeyCode(cdefPlayFromPos,Key(shift,keyCode));
																																																																									} else
																																																																										if (id == "play_current_row") {
																																																																											inputHandler_.changeKeyCode(cdefPlayRowTrack,Key(shift,keyCode));
																																																																										} else
																																																																											if (id == "play_sel_pattern_looped") {
																																																																												inputHandler_.changeKeyCode(cdefKeyStopAny,Key(shift,keyCode));
																																																																											} else
																																																																												if (id == "play_song_current") {
																																																																													inputHandler_.changeKeyCode(cdefPlayFromPos,Key(shift,keyCode));
																																																																												} else
																																																																													if (id == "play_song_start") {
																																																																														inputHandler_.changeKeyCode(cdefPlayStart,Key(shift,keyCode));
																																																																													} else
																																																																														if (id == "play_song_normal") {
																																																																															inputHandler_.changeKeyCode(cdefPlaySong,Key(shift,keyCode));
																																																																														} else
																																																																															if (id == "position+1") {
																																																																																inputHandler_.changeKeyCode(cdefSongPosInc,Key(shift,keyCode));
																																																																															} else
																																																																																if (id == "position-1") {
																																																																																	inputHandler_.changeKeyCode(cdefSongPosDec,Key(shift,keyCode));
																																																																																} else
																																																																																	if (id == "prev_column") {
																																																																																		inputHandler_.changeKeyCode(cdefColumnPrev,Key(shift,keyCode));
																																																																																	} else
																																																																																		if (id == "row_skip+1") {
																																																																																			//inputHandler_.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
																																																																																		} else
																																																																																			if (id == "row_skip-1") {
																																																																																				//inputHandler_.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
																																																																																			} else
																																																																																				if (id == "screen_machines") {
																																																																																					inputHandler_.changeKeyCode(cdefEditMachine,Key(shift,keyCode));
																																																																																				} else
																																																																																					if (id == "screen_patterns") {
																																																																																						inputHandler_.changeKeyCode(cdefEditPattern,Key(shift,keyCode));
																																																																																					} else
																																																																																						if (id == "screen_sequencer") {
																																																																																							inputHandler_.changeKeyCode(cdefEditSequence,Key(shift,keyCode));
																																																																																						} else
																																																																																							if (id == "select_mac_in_cursor") {
																																																																																								//inputHandler_.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
																																																																																							} else
																																																																																								if (id == "show_error_log") {
																																																																																									//inputHandler_.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
																																																																																								} else
																																																																																									if (id == "stop_playback") {
																																																																																										inputHandler_.changeKeyCode(cdefPlayStop,Key(shift,keyCode));
																																																																																									} else
																																																																																										if (id == "toggle_edit_mode") {
																																																																																											//inputHandler_.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
																																																																																										} else
																																																																																											if (id == "toggle_follow_song") {
																																																																																												//inputHandler_.changeKeyCode(cdefKeyStopAny,Key(mod,keyCode));
																																																																																											} else
																																																																																												if (id == "transpose_block+1") {
																																																																																													inputHandler_.changeKeyCode(cdefTransposeBlockInc,Key(shift,keyCode));
																																																																																												} else
																																																																																													if (id == "transpose_block+12") {
																																																																																														inputHandler_.changeKeyCode(cdefTransposeBlockInc12,Key(shift,keyCode));
																																																																																													} else
																																																																																														if (id == "transpose_block-1") {
																																																																																															inputHandler_.changeKeyCode(cdefTransposeBlockDec,Key(shift,keyCode));
																																																																																														} else
																																																																																															if (id == "transpose_block-12") {
																																																																																																inputHandler_.changeKeyCode(cdefTransposeBlockDec12,Key(shift,keyCode));
																																																																																															} else
																																																																																																if (id == "transpose_channel+1") {
																																																																																																	inputHandler_.changeKeyCode(cdefTransposeChannelInc,Key(shift,keyCode));
																																																																																																} else
																																																																																																	if (id == "transpose_channel+12") {
																																																																																																		inputHandler_.changeKeyCode(cdefTransposeChannelInc12,Key(shift,keyCode));
																																																																																																	} else
																																																																																																		if (id == "transpose_channel-1") {
																																																																																																			inputHandler_.changeKeyCode(cdefTransposeChannelDec,Key(shift,keyCode));
																																																																																																		} else
																																																																																																			if (id == "transpose_channel-12") {
																																																																																																				inputHandler_.changeKeyCode(cdefTransposeChannelDec12,Key(shift,keyCode));
																																																																																																			} else
																																																																																																				if (id == "tweak_parameter") {
																																																																																																					inputHandler_.changeKeyCode(cdefTweakM,Key(shift,keyCode));
																																																																																																				} else
																																																																																																					if (id == "tweak_smooth_paramater") {
																																																																																																						inputHandler_.changeKeyCode(cdefTweakS,Key(shift,keyCode));
																																																																																																					}
							}
		}
		
		InputHandler & Configuration::inputHandler() {
			return inputHandler_;
		}

	}
}