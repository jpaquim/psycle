///\file
///\implementation psycle::host::Configuration.

#include "PsycleConfig.hpp"
#include "Psycle.hpp"

#include "Registry.hpp"
#include "SkinIO.hpp"
#include "KeyPresetIO.hpp"

#include "WaveOut.hpp"
#include "DirectSound.hpp"
#include "ASIOInterface.hpp"
#include "WasapiDriver.hpp"

#include "MidiInput.hpp"

#include "NewMachine.hpp"
#include "Player.hpp"
#include "Song.hpp"
#include "NativeGui.hpp"
#include "NativeGraphics.hpp"

#include <universalis/os/fs.hpp>
#include <wingdi.h>

namespace psycle
{
	namespace host
	{
		extern CPsycleApp theApp;

		PsycleConfig::MachineParam::MachineParam()
		{
			CNativeGui::uiSetting = this;
			Knob::uiSetting = this;
			InfoLabel::uiSetting = this;
			GraphSlider::uiSetting = this;
			SwitchButton::uiSetting = this;
			CheckedButton::uiSetting = this;
			VuMeter::uiSetting = this;
			SetDefaultSettings();
		}
		PsycleConfig::MachineParam::~MachineParam()
		{
			font.DeleteObject();
			font_bold.DeleteObject();
			dial.DeleteObject();
			if (hbmMachineDial) DeleteObject(hbmMachineDial);
			hbmMachineDial=NULL;
			sliderBack.DeleteObject();
			sliderKnob.DeleteObject();
			vuOn.DeleteObject();
			vuOff.DeleteObject();
			switchOn.DeleteObject();
			switchOff.DeleteObject();
			CNativeGui::uiSetting = NULL;
			Knob::uiSetting = NULL;
			InfoLabel::uiSetting = NULL;
			GraphSlider::uiSetting = NULL;
			SwitchButton::uiSetting = NULL;
			CheckedButton::uiSetting = NULL;
			VuMeter::uiSetting = NULL;
		}
		void PsycleConfig::MachineParam::SetDefaultSettings(bool include_others)
		{
			toolbarOnMachineParams = true;
			hbmMachineDial = NULL;

			if(include_others) {
				SetDefaultColours();
				SetDefaultSkin();
			}
		}
		void PsycleConfig::MachineParam::SetDefaultColours()
		{
			topColor = 0x00D2C2BD;		
			fontTopColor = 0x00000000; 
			bottomColor = 0x009C796D;
			fontBottomColor = 0x00FFFFFF; 
			
			//highlighted param colours
			hTopColor = 0x00BC94A9;
			fonthTopColor = 0x00000000; 
			hBottomColor = 0x008B5A72;
			fonthBottomColor = 0x0044EEFF; 

			titleColor = 0x00000000;
			fonttitleColor = 0x00FFFFFF;
		}
		void PsycleConfig::MachineParam::SetDefaultSkin()
		{
			szBmpControlsFilename = "";

			dialwidth = 28;
			dialheight = 28;
			dialframes = 64;

			sliderheight = 182;
			sliderwidth = 28;
			sliderknobheight = 21;
			sliderknobwidth = 16;

			vuheight = 97;
			vuwidth = 16;

			switchheight = 28;
			switchwidth = 28;
		}

		void PsycleConfig::MachineParam::Load(ConfigStorage & store,std::string mainSkinDir, std::string machine_skin)
		{
			// Do not open group if loading version 1.8.6
			if(!store.GetVersion().empty()) {
				store.OpenGroup("MacParamVisual");
			}
			store.Read("toolbarOnVsts", toolbarOnMachineParams);
			store.Read("szBmpDialFilename", szBmpControlsFilename);
			store.Read("machineGUIFontTopColor", fontTopColor);
			store.Read("machineGUIFontBottomColor", fontBottomColor);
			store.Read("machineGUIHFontTopColor", fonthTopColor);
			store.Read("machineGUIHFontBottomColor", fonthBottomColor);
			store.Read("machineGUITitleFontColor", fonttitleColor);
			store.Read("machineGUITopColor", topColor);
			store.Read("machineGUIBottomColor", bottomColor);
			store.Read("machineGUIHTopColor", hTopColor);
			store.Read("machineGUIHBottomColor", hBottomColor);
			store.Read("machineGUITitleColor", titleColor);
			// Close group if loading version 1.8.8 and onwards
			if(!store.GetVersion().empty()) {
				store.CloseGroup();
			}
			if(szBmpControlsFilename.empty() || szBmpControlsFilename == PSYCLE__PATH__DEFAULT_DIAL_SKIN) {
				SetDefaultSkin();
			}
			else if(szBmpControlsFilename.rfind('\\') == -1) {
				if(machine_skin.empty()) {
					szBmpControlsFilename = mainSkinDir + "\\" + szBmpControlsFilename;
				}
				else {
					std::string skin_dir = machine_skin.substr(0,machine_skin.rfind('\\'));
					szBmpControlsFilename = skin_dir + "\\" + szBmpControlsFilename;
				}
			}
		}
		void PsycleConfig::MachineParam::Save(ConfigStorage & store)
		{
			store.CreateGroup("MacParamVisual");
			store.Write("toolbarOnVsts", toolbarOnMachineParams);
			store.Write("szBmpDialFilename", szBmpControlsFilename);
			store.Write("machineGUIFontTopColor", fontTopColor);
			store.Write("machineGUIFontBottomColor", fontBottomColor);
			store.Write("machineGUIHFontTopColor", fonthTopColor);
			store.Write("machineGUIHFontBottomColor", fonthBottomColor);
			store.Write("machineGUITitleFontColor", fonttitleColor);
			store.Write("machineGUITopColor", topColor);
			store.Write("machineGUIBottomColor", bottomColor);
			store.Write("machineGUIHTopColor", hTopColor);
			store.Write("machineGUIHBottomColor", hBottomColor);
			store.Write("machineGUITitleColor", titleColor);
			store.CloseGroup();
		}

		void PsycleConfig::MachineParam::RefreshSettings()
		{
			///\todo: Configurable font
			PsycleConfig::CreatePsyFont(font,"Tahoma",80,false,false);
			PsycleConfig::CreatePsyFont(font_bold,"Tahoma",80,true,false);
			HWND hwndDesk = ::GetDesktopWindow();
			::GetWindowRect(hwndDesk, &deskrect); 

			RefreshSkin();
			GraphSlider::xoffset = ((sliderwidth-sliderknobwidth)/2) +1;
		}
		void PsycleConfig::MachineParam::RefreshSkin()
		{
			dial.DeleteObject();
			if (hbmMachineDial) DeleteObject(hbmMachineDial);
			hbmMachineDial=NULL;
			sliderBack.DeleteObject();
			sliderKnob.DeleteObject();
			vuOn.DeleteObject();
			vuOff.DeleteObject();
			switchOn.DeleteObject();
			switchOff.DeleteObject();

			bool bBmpControls=false;
			if (!szBmpControlsFilename.empty())
			{
				hbmMachineDial = (HBITMAP)LoadImage(NULL, szBmpControlsFilename.c_str(), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
				if (hbmMachineDial && dial.Attach(hbmMachineDial))
				{	
					BITMAP bm;
					GetObject(hbmMachineDial,sizeof(BITMAP),&bm);

					if ((bm.bmWidth == 1792) && (bm.bmHeight == 28))
					{
						bBmpControls=true;
					}
				}
			}
			if(!bBmpControls) {
				szBmpControlsFilename="";
				dial.LoadBitmap(IDB_KNOB);
			}

			///\todo: We do not support changing these yet.
			sliderBack.LoadBitmap(IDB_SLIDERBACKV);
			sliderKnob.LoadBitmap(IDB_SLIDERKNOBV);
			vuOn.LoadBitmap(IDB_VUMETERON);
			vuOff.LoadBitmap(IDB_VUMETEROFF);
			switchOn.LoadBitmap(IDB_SWITCHON);
			switchOff.LoadBitmap(IDB_SWITCHOFF);
		}
		////////////////////////////////////
		PsycleConfig::MachineView::MachineView()
		{
			SetDefaultSettings();
		}
		PsycleConfig::MachineView::~MachineView()
		{
			generatorFont.DeleteObject();
			effectFont.DeleteObject();
			machineskin.DeleteObject();
			DeleteObject(hbmMachineSkin);
			hbmMachineSkin=NULL;
			machineskinmask.DeleteObject();
			machinebkg.DeleteObject();
			DeleteObject(hbmMachineBkg);
			hbmMachineBkg=NULL;
		}
		void PsycleConfig::MachineView::SetDefaultSettings(bool include_others)
		{
			draw_mac_index = true;
			draw_vus = true;
			hbmMachineSkin = NULL;
			hbmMachineBkg = NULL;

			if(include_others) {
				SetDefaultColours();
				SetDefaultSkin();
				SetDefaultBackground();
			}
		}
		void PsycleConfig::MachineView::SetDefaultColours()
		{
			colour =	0x0099766C;
			polycolour =	0x00ffffff;
			wirecolour =	0x00000000;
			triangle_size = 22;
			wirewidth = 1;
			wireaa = 1;

			vu1 = 0x0080FF80;
			vu2 = 0x00403731;
			vu3 = 0x00262bd7;

			generator_fontcolour = 0x00000000;
			generator_fontface = "Tahoma";
			generator_font_point = 80;
			generator_font_flags = 0;
			effect_fontcolour = 0x00000000;
			effect_fontface = "Tahoma";
			effect_font_point = 80;
			effect_font_flags = 0;
		}
		void PsycleConfig::MachineView::SetDefaultSkin()
		{
			machine_skin = "";

			MachineCoords.sMaster.x = 0;
			MachineCoords.sMaster.y = 0;
			MachineCoords.sMaster.width = 148;
			MachineCoords.sMaster.height = 47;

			MachineCoords.sGenerator.x = 0;
			MachineCoords.sGenerator.y = 47;
			MachineCoords.sGenerator.width = 148;
			MachineCoords.sGenerator.height = 47;
			MachineCoords.sGeneratorVu0.x = 0;
			MachineCoords.sGeneratorVu0.y = 141;
			MachineCoords.sGeneratorVu0.width = 7;
			MachineCoords.sGeneratorVu0.height = 4;
			MachineCoords.sGeneratorVuPeak.x = 128;
			MachineCoords.sGeneratorVuPeak.y = 141;
			MachineCoords.sGeneratorVuPeak.width = 2;
			MachineCoords.sGeneratorVuPeak.height = 4;
			MachineCoords.sGeneratorPan.x = 45;
			MachineCoords.sGeneratorPan.y = 145;
			MachineCoords.sGeneratorPan.width = 16;
			MachineCoords.sGeneratorPan.height = 5;
			MachineCoords.sGeneratorMute.x = 0;
			MachineCoords.sGeneratorMute.y = 145;
			MachineCoords.sGeneratorMute.width = 15;
			MachineCoords.sGeneratorMute.height = 14;
			MachineCoords.sGeneratorSolo.x = 15;
			MachineCoords.sGeneratorSolo.y = 145;
			MachineCoords.sGeneratorSolo.width = 15;
			MachineCoords.sGeneratorSolo.height = 14;

			MachineCoords.sEffect.x = 0;
			MachineCoords.sEffect.y = 94;
			MachineCoords.sEffect.width = 148;
			MachineCoords.sEffect.height = 47;
			MachineCoords.sEffectVu0.x = 0;
			MachineCoords.sEffectVu0.y = 141;
			MachineCoords.sEffectVu0.width = 7;
			MachineCoords.sEffectVu0.height = 4;
			MachineCoords.sEffectVuPeak.x = 128;
			MachineCoords.sEffectVuPeak.y = 141;
			MachineCoords.sEffectVuPeak.width = 2;
			MachineCoords.sEffectVuPeak.height = 4;
			MachineCoords.sEffectPan.x = 45;
			MachineCoords.sEffectPan.y = 145;
			MachineCoords.sEffectPan.width = 16;
			MachineCoords.sEffectPan.height = 5;
			MachineCoords.sEffectMute.x = 0;
			MachineCoords.sEffectMute.y = 145;
			MachineCoords.sEffectMute.width = 15;
			MachineCoords.sEffectMute.height = 14;
			MachineCoords.sEffectBypass.x = 30;
			MachineCoords.sEffectBypass.y = 145;
			MachineCoords.sEffectBypass.width = 15;
			MachineCoords.sEffectBypass.height = 14;

			MachineCoords.dGeneratorVu.x = 10;
			MachineCoords.dGeneratorVu.y = 35;
			MachineCoords.dGeneratorVu.width = 130;
			MachineCoords.dGeneratorVu.height = 0;
			MachineCoords.dGeneratorPan.x = 39;
			MachineCoords.dGeneratorPan.y = 26;
			MachineCoords.dGeneratorPan.width = 91;
			MachineCoords.dGeneratorPan.height = 0;
			MachineCoords.dGeneratorMute.x = 11;
			MachineCoords.dGeneratorMute.y = 5;
			MachineCoords.dGeneratorSolo.x = 26;
			MachineCoords.dGeneratorSolo.y = 5;
			MachineCoords.dGeneratorName.x = 49;
			MachineCoords.dGeneratorName.y = 7;

			MachineCoords.dEffectVu.x = 10;
			MachineCoords.dEffectVu.y = 35;
			MachineCoords.dEffectVu.width = 130;
			MachineCoords.dEffectVu.height = 0;
			MachineCoords.dEffectPan.x = 39;
			MachineCoords.dEffectPan.y = 26;
			MachineCoords.dEffectPan.width = 91;
			MachineCoords.dEffectPan.height = 0;
			MachineCoords.dEffectMute.x = 11;
			MachineCoords.dEffectMute.y = 5;
			MachineCoords.dEffectBypass.x = 26;
			MachineCoords.dEffectBypass.y = 5;
			MachineCoords.dEffectName.x = 49;
			MachineCoords.dEffectName.y = 7;
			MachineCoords.bHasTransparency = false;
		}
		void PsycleConfig::MachineView::SetDefaultBackground()
		{
			bBmpBkg = false;
			szBmpBkgFilename = "";
			bkgx = 0;
			bkgy = 0;
		}
		void PsycleConfig::MachineView::Load(ConfigStorage &store, std::string mainSkinDir)
		{
			// Do not open group if loading version 1.8.6
			if(!store.GetVersion().empty()) {
				store.OpenGroup("MachineVisual");
			}
			store.Read("mv_colour", colour);
			store.Read("mv_polycolour", polycolour);
			store.Read("mv_triangle_size", triangle_size);
			store.Read("mv_wirecolour", wirecolour);
			store.Read("mv_wirewidth", wirewidth);
			store.Read("mv_wireaa", wireaa);

			store.Read("vu1", vu1);
			store.Read("vu2", vu2);
			store.Read("vu3", vu3);
			store.Read("draw_vus", draw_vus);

			store.Read("szBmpBkgFilename", szBmpBkgFilename);
			store.Read("machine_skin", machine_skin);
			store.Read("draw_mac_index", draw_mac_index);

			store.Read("mv_generator_fontcolour", generator_fontcolour);
			store.Read("generator_fontface", generator_fontface);
			store.Read("generator_font_point", generator_font_point);
			store.Read("generator_font_flags", generator_font_flags);
			store.Read("mv_effect_fontcolour", effect_fontcolour);
			store.Read("effect_fontface", effect_fontface);
			store.Read("effect_font_point", effect_font_point);
			store.Read("effect_font_flags", effect_font_flags);
			// Close group if loading version 1.8.8 and onwards
			if(!store.GetVersion().empty()) {
				store.CloseGroup();
			}

			std::string skin_dir = mainSkinDir;
			if(machine_skin.empty() || machine_skin == PSYCLE__PATH__DEFAULT_MACHINE_SKIN)
			{
				SetDefaultSkin();
			}
			else {
				if(machine_skin.rfind('\\') == -1) {
					SkinIO::LocateSkinDir(mainSkinDir.c_str(), machine_skin.c_str(), ".psm", skin_dir);
					machine_skin = skin_dir + "\\" + machine_skin;
				}
				else {
					skin_dir = machine_skin.substr(0,machine_skin.rfind('\\'));
				}
				SkinIO::LoadMachineSkin((machine_skin + ".psm").c_str() ,MachineCoords);
			}
			if(szBmpBkgFilename.empty() || szBmpBkgFilename == PSYCLE__PATH__DEFAULT_BACKGROUND_SKIN)
			{
				SetDefaultBackground();
			}
			else if(szBmpBkgFilename.rfind('\\') == -1) {
				szBmpBkgFilename = skin_dir + "\\" + szBmpBkgFilename;
			}
		}
		void PsycleConfig::MachineView::Save(ConfigStorage &store)
		{
			store.CreateGroup("MachineVisual");
			store.Write("mv_colour", colour);
			store.Write("mv_polycolour", polycolour);
			store.Write("mv_triangle_size", triangle_size);
			store.Write("mv_wirecolour", wirecolour);
			store.Write("mv_wirewidth", wirewidth);
			store.Write("mv_wireaa", wireaa);

			store.Write("vu1", vu1);
			store.Write("vu2", vu2);
			store.Write("vu3", vu3);
			store.Write("draw_vus", draw_vus);

			store.Write("szBmpBkgFilename", szBmpBkgFilename);
			store.Write("machine_skin", machine_skin);
			store.Write("draw_mac_index", draw_mac_index);

			store.Write("mv_generator_fontcolour", generator_fontcolour);
			store.Write("generator_fontface", generator_fontface);
			store.Write("generator_font_point", generator_font_point);
			store.Write("generator_font_flags", generator_font_flags);
			store.Write("mv_effect_fontcolour", effect_fontcolour);
			store.Write("effect_fontface", effect_fontface);
			store.Write("effect_font_point", effect_font_point);
			store.Write("effect_font_flags", effect_font_flags);
			store.CloseGroup();
		}
		void PsycleConfig::MachineView::RefreshSettings()
		{
			wireaacolour =
				((((wirecolour&0x00ff0000) + ((colour&0x00ff0000)*4))/5)&0x00ff0000) +
				((((wirecolour&0x00ff00) + ((colour&0x00ff00)*4))/5)&0x00ff00) +
				((((wirecolour&0x00ff) + ((colour&0x00ff)*4))/5)&0x00ff);

			wireaacolour2 =
				(((((wirecolour&0x00ff0000)) + ((colour&0x00ff0000)))/2)&0x00ff0000) +
				(((((wirecolour&0x00ff00)) + ((colour&0x00ff00)))/2)&0x00ff00) +
				(((((wirecolour&0x00ff)) + ((colour&0x00ff)))/2)&0x00ff);

			bool bBold = generator_font_flags & 1;
			bool bItalic = generator_font_flags & 2;
			if(!PsycleConfig::CreatePsyFont(generatorFont,generator_fontface,generator_font_point,bBold,bItalic))
			{
				Error("Could not find this font! " + generator_fontface);
				if(!PsycleConfig::CreatePsyFont(generatorFont,"Tahoma",generator_font_point,bBold,bItalic))
					PsycleConfig::CreatePsyFont(generatorFont,"Arial",10,false,false);
			}
			bBold = effect_font_flags & 1;
			bItalic = effect_font_flags & 2;
			if(!PsycleConfig::CreatePsyFont(effectFont,effect_fontface,effect_font_point,bBold,bItalic))
			{
				Error("Could not find this font! " + effect_fontface);
				if(!PsycleConfig::CreatePsyFont(effectFont,"Tahoma",effect_font_point,bBold,bItalic))
					PsycleConfig::CreatePsyFont(effectFont,"Arial",10,false,false);
			}
			RefreshSkin();
		}
		void PsycleConfig::MachineView::RefreshSkin()
		{
			machineskin.DeleteObject();
			DeleteObject(hbmMachineSkin);
			hbmMachineSkin=NULL;
			machineskinmask.DeleteObject();
			if (machine_skin.empty())
			{
				machineskin.LoadBitmap(IDB_MACHINE_SKIN);
			}
			else if(!RefreshBitmaps())
			{
				PsycleConfig::Error("The machine skin specified cannot be loaded. Wrong name?");
				SetDefaultSkin();
				machineskin.LoadBitmap(IDB_MACHINE_SKIN);
			}
			if (MachineCoords.bHasTransparency)
			{
				PrepareMask(&machineskin,&machineskinmask,MachineCoords.cTransparency);
			}
			RefreshBackground();
		}
		bool PsycleConfig::MachineView::RefreshBitmaps()
		{
			hbmMachineSkin = (HBITMAP)LoadImage(NULL, (machine_skin+ ".bmp").c_str(), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
			if (hbmMachineSkin && machineskin.Attach(hbmMachineSkin))
			{	
				return true;
			}
			return false;
		}
		void PsycleConfig::MachineView::RefreshBackground()
		{
			machinebkg.DeleteObject();
			if ( hbmMachineBkg) DeleteObject(hbmMachineBkg);
			hbmMachineBkg=NULL;
			bBmpBkg=false;
			if (!szBmpBkgFilename.empty())
			{
				hbmMachineBkg = (HBITMAP)LoadImage(NULL, szBmpBkgFilename.c_str(), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
				if (hbmMachineBkg)
				{
					if (machinebkg.Attach(hbmMachineBkg))
					{	
						BITMAP bm;
						GetObject(hbmMachineBkg,sizeof(BITMAP),&bm);

						bkgx=bm.bmWidth;
						bkgy=bm.bmHeight;

						if ((bkgx > 0) && (bkgy > 0))
						{
							bBmpBkg=true;
						}
					}
				}
			}
			if(!bBmpBkg) {
				szBmpBkgFilename="";
			}
		}

		///////////////////////////////////////////////
		PsycleConfig::PatternView::PatternView()
		{
			SetDefaultSettings();
		}
		PsycleConfig::PatternView::~PatternView()
		{
			pattern_font.DeleteObject();
			patternheader.DeleteObject();
			DeleteObject(hbmPatHeader);
			hbmPatHeader=NULL;
			patternheadermask.DeleteObject();
		}
		void PsycleConfig::PatternView::SetDefaultSettings(bool include_others)
		{
			_centerCursor = false;
			draw_empty_data = false;
			timesig = 4;

			_linenumbers = true;
			_linenumbersHex = false;
			_linenumbersCursor = true;
			if(include_others) {
				SetDefaultColours();
				SetDefaultSkin();
			}
		}
		void PsycleConfig::PatternView::SetDefaultColours()
		{
			separator  = 0x00400000;
			separator2  = 0x00800000;
			background  = 0x0099766C;
			background2  = 0x00AA786C;
			row4beat  = 0x00AF958B;
			row4beat2 = 0x00AF958B;
			rowbeat  = 0x00C0ACA5;
			rowbeat2 = 0x00C0ACA5;
			row  = 0x00D0C4BF;
			row2 = 0x00D0C4BF;
			font  = 0x00000000;
			font2  = 0x00000000;
			fontPlay  = 0x00ffffff;
			fontPlay2  = 0x00ffffff;
			fontCur  = 0x00ffffff;
			fontCur2  = 0x00ffffff;
			fontSel  = 0x00ffffff;
			fontSel2  = 0x00ffffff;
			selection  = 0x00e00000;
			selection2 = 0x00ff5050;
			playbar  = 0x0080FF00;
			playbar2 = 0x0080FF00;
			cursor  = 0x000000e0;
			cursor2 = 0x000000e0;

			font_name = "Tahoma";
			font_flags = 1;
			font_point = 80;
			font_x = 9;
			font_y = 11;
		}
		void PsycleConfig::PatternView::SetDefaultSkin()
		{
			header_skin = "";
			hbmPatHeader = NULL;

			PatHeaderCoords.sBackground.x=0;
			PatHeaderCoords.sBackground.y=0;
			PatHeaderCoords.sBackground.width=109;
			PatHeaderCoords.sBackground.height=18;
			PatHeaderCoords.sNumber0.x = 0;
			PatHeaderCoords.sNumber0.y = 18;
			PatHeaderCoords.sNumber0.width = 7;
			PatHeaderCoords.sNumber0.height = 12;
			PatHeaderCoords.sRecordOn.x = 70;
			PatHeaderCoords.sRecordOn.y = 18;
			PatHeaderCoords.sRecordOn.width = 11;
			PatHeaderCoords.sRecordOn.height = 11;
			PatHeaderCoords.sMuteOn.x = 81;
			PatHeaderCoords.sMuteOn.y = 18;
			PatHeaderCoords.sMuteOn.width = 11;
			PatHeaderCoords.sMuteOn.height = 11;
			PatHeaderCoords.sSoloOn.x = 92;
			PatHeaderCoords.sSoloOn.y = 18;
			PatHeaderCoords.sSoloOn.width = 11;
			PatHeaderCoords.sSoloOn.height = 11;
			PatHeaderCoords.dDigitX0.x = 24;
			PatHeaderCoords.dDigitX0.y = 3;
			PatHeaderCoords.dDigit0X.x = 31;
			PatHeaderCoords.dDigit0X.y = 3;
			PatHeaderCoords.dRecordOn.x = 52;
			PatHeaderCoords.dRecordOn.y = 3;
			PatHeaderCoords.dMuteOn.x = 75;
			PatHeaderCoords.dMuteOn.y = 3;
			PatHeaderCoords.dSoloOn.x = 97;
			PatHeaderCoords.dSoloOn.y = 3;
			PatHeaderCoords.bHasTransparency = false;
		}
		void PsycleConfig::PatternView::Load(ConfigStorage &store, std::string mainSkinDir)
		{
			// Do not open group if loading version 1.8.6
			if(!store.GetVersion().empty()) {
				store.OpenGroup("PatternVisual");
			}
			store.Read("pvc_separator", separator);
			store.Read("pvc_separator2", separator2);
			store.Read("pvc_background", background);
			store.Read("pvc_background2", background2);
			store.Read("pvc_row4beat", row4beat);
			store.Read("pvc_row4beat2", row4beat2);
			store.Read("pvc_rowbeat", rowbeat);
			store.Read("pvc_rowbeat2", rowbeat2);
			store.Read("pvc_row", row);
			store.Read("pvc_row2", row2);
			store.Read("pvc_font", font);
			store.Read("pvc_font2", font2);
			store.Read("pvc_fontPlay", fontPlay);
			store.Read("pvc_fontPlay2", fontPlay2);
			store.Read("pvc_fontCur", fontCur);
			store.Read("pvc_fontCur2", fontCur2);
			store.Read("pvc_fontSel", fontSel);
			store.Read("pvc_fontSel2", fontSel2);
			store.Read("pvc_selection", selection);
			store.Read("pvc_selection2", selection2);
			store.Read("pvc_playbar", playbar);
			store.Read("pvc_playbar2", playbar2);
			store.Read("pvc_cursor", cursor);
			store.Read("pvc_cursor2", cursor2);

			store.Read("pattern_header_skin", header_skin);

			store.Read("pattern_fontface", font_name);
			store.Read("pattern_font_flags", font_flags);
			store.Read("pattern_font_point", font_point);
			store.Read("pattern_font_x", font_x);
			store.Read("pattern_font_y", font_y);

			store.Read("CenterCursor", _centerCursor);
			store.Read("pattern_draw_empty_data", draw_empty_data);
			store.Read("pv_timesig", timesig);
			store.Read("DisplayLineNumbers", _linenumbers);
			store.Read("DisplayLineNumbersHex", _linenumbersHex);
			store.Read("DisplayLineNumbersCursor", _linenumbersCursor);

			// Close group if loading version 1.8.8 and onwards
			if(!store.GetVersion().empty()) {
				store.CloseGroup();
			}

			if(header_skin.empty() || header_skin == PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN)
			{
				SetDefaultSkin();
			}
			else {
				if(header_skin.rfind('\\') == -1 && header_skin.length() > 0) {
					std::string skin_dir = mainSkinDir;
					SkinIO::LocateSkinDir(mainSkinDir.c_str(), header_skin.c_str(), ".psh", skin_dir);
					header_skin = skin_dir + "\\" +  header_skin;
				}
				SkinIO::LoadPatternSkin((header_skin + ".psh").c_str() ,PatHeaderCoords);
			}
		}
		void PsycleConfig::PatternView::Save(ConfigStorage &store)
		{
			store.CreateGroup("PatternVisual");
			store.Write("pvc_separator", separator);
			store.Write("pvc_separator2", separator2);
			store.Write("pvc_background", background);
			store.Write("pvc_background2", background2);
			store.Write("pvc_row4beat", row4beat);
			store.Write("pvc_row4beat2", row4beat2);
			store.Write("pvc_rowbeat", rowbeat);
			store.Write("pvc_rowbeat2", rowbeat2);
			store.Write("pvc_row", row);
			store.Write("pvc_row2", row2);
			store.Write("pvc_font", font);
			store.Write("pvc_font2", font2);
			store.Write("pvc_fontPlay", fontPlay);
			store.Write("pvc_fontPlay2", fontPlay2);
			store.Write("pvc_fontCur", fontCur);
			store.Write("pvc_fontCur2", fontCur2);
			store.Write("pvc_fontSel", fontSel);
			store.Write("pvc_fontSel2", fontSel2);
			store.Write("pvc_selection", selection);
			store.Write("pvc_selection2", selection2);
			store.Write("pvc_playbar", playbar);
			store.Write("pvc_playbar2", playbar2);
			store.Write("pvc_cursor", cursor);
			store.Write("pvc_cursor2", cursor2);

			store.Write("pattern_header_skin", header_skin);

			store.Write("pattern_fontface", font_name);
			store.Write("pattern_font_flags", font_flags);
			store.Write("pattern_font_point", font_point);
			store.Write("pattern_font_x", font_x);
			store.Write("pattern_font_y", font_y);

			store.Write("CenterCursor", _centerCursor);
			store.Write("pattern_draw_empty_data", draw_empty_data);
			store.Write("pv_timesig", timesig);
			store.Write("DisplayLineNumbers", _linenumbers);
			store.Write("DisplayLineNumbersHex", _linenumbersHex);
			store.Write("DisplayLineNumbersCursor", _linenumbersCursor);

			store.CloseGroup();
		}
		void PsycleConfig::PatternView::RefreshSettings()
		{
			bool bBold = font_flags & 1;
			bool bItalic = font_flags & 2;
			if(!CreatePsyFont(pattern_font,font_name,font_point,bBold,bItalic))
			{
				Error("Could not find this font! " + font_name);
				if(!CreatePsyFont(pattern_font,"Tahoma",font_point,bBold,bItalic))
					CreatePsyFont(pattern_font,"Arial",10,false,false);
			}
			RefreshSkin();
		}
		void PsycleConfig::PatternView::RefreshSkin()
		{
			patternheader.DeleteObject();
			DeleteObject(hbmPatHeader);
			hbmPatHeader=NULL;
			patternheadermask.DeleteObject();

			if (header_skin.empty())
			{
				patternheader.LoadBitmap(IDB_PATTERN_HEADER_SKIN);
			}
			else if(!RefreshBitmaps())
			{
				PsycleConfig::Error("The pattern header skin specified cannot be loaded. Wrong name?");
				SetDefaultSkin();
				patternheader.LoadBitmap(IDB_PATTERN_HEADER_SKIN);
			}
			if (PatHeaderCoords.bHasTransparency)
			{
				PrepareMask(&patternheader,&patternheadermask,PatHeaderCoords.cTransparency);
			}
		}
		bool PsycleConfig::PatternView::RefreshBitmaps()
		{
			hbmPatHeader = (HBITMAP)LoadImage(NULL, (header_skin+ ".bmp").c_str(), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
			if (hbmPatHeader && patternheader.Attach(hbmPatHeader))
			{	
				return true;
			}
			return false;
		}
		///////////////////////////////////////////////
		PsycleConfig::InputHandler::InputHandler()
		{
			SetDefaultSettings();
		}
		PsycleConfig::InputHandler::~InputHandler()
		{
		}
		void PsycleConfig::InputHandler::SetDefaultSettings(bool include_others)
		{
			bCtrlPlay = true;
			bFT2HomeBehaviour = true;
			bFT2DelBehaviour = true;
			_windowsBlocks = true;
			bShiftArrowsDoSelect = false;
			_wrapAround = true;
			_cursorAlwaysDown = false;
			_RecordMouseTweaksSmooth = false;
			_RecordUnarmed = true;
			_NavigationIgnoresStep = false;
			_pageUpSteps = 1;

			_RecordNoteoff = false;
			_RecordTweaks = false;
			bMultiKey = true;
			_notesToEffects = false;
			bMoveCursorPaste = true;

			if(include_others)
			{
				SetDefaultKeys();
			}
		}
		void PsycleConfig::InputHandler::SetDefaultKeys()
		{
			keyMap.clear();
			setMap.clear();
			// note keys

			// octave 0
			SetCmd(cdefKeyC_0,0,'Z',false);
			SetCmd(cdefKeyCS0,0,'S',false);
			SetCmd(cdefKeyD_0,0,'X',false);
			SetCmd(cdefKeyDS0,0,'D',false);
			SetCmd(cdefKeyE_0,0,'C',false);
			SetCmd(cdefKeyF_0,0,'V',false);
			SetCmd(cdefKeyFS0,0,'G',false);
			SetCmd(cdefKeyG_0,0,'B',false);
			SetCmd(cdefKeyGS0,0,'H',false);
			SetCmd(cdefKeyA_0,0,'N',false);
			SetCmd(cdefKeyAS0,0,'J',false);
			SetCmd(cdefKeyB_0,0,'M',false);

			// octave 1
			SetCmd(cdefKeyC_1,0,'Q',false);
			SetCmd(cdefKeyCS1,0,'2',false);
			SetCmd(cdefKeyD_1,0,'W',false);
			SetCmd(cdefKeyDS1,0,'3',false);
			SetCmd(cdefKeyE_1,0,'E',false);
			SetCmd(cdefKeyF_1,0,'R',false);
			SetCmd(cdefKeyFS1,0,'5',false);
			SetCmd(cdefKeyG_1,0,'T',false);
			SetCmd(cdefKeyGS1,0,'6',false);
			SetCmd(cdefKeyA_1,0,'Y',false);
			SetCmd(cdefKeyAS1,0,'7',false);
			SetCmd(cdefKeyB_1,0,'U',false);

			// octave 2
			SetCmd(cdefKeyC_2,0,'I',false);
			SetCmd(cdefKeyCS2,0,'9',false);
			SetCmd(cdefKeyD_2,0,'O',false);
			SetCmd(cdefKeyDS2,0,'0',false);
			SetCmd(cdefKeyE_2,0,'P',false);
			
			SetCmd(cdefKeyF_2,0,0,false);
			SetCmd(cdefKeyFS2,0,0,false);
			SetCmd(cdefKeyG_2,0,0,false);
			SetCmd(cdefKeyGS2,0,0,false);
			SetCmd(cdefKeyA_2,0,0,false);
			SetCmd(cdefKeyAS2,0,0,false);
			SetCmd(cdefKeyB_2,0,0,false);
			SetCmd(cdefKeyC_3,0,0,false);
			SetCmd(cdefKeyCS3,0,0,false);
			SetCmd(cdefKeyD_3,0,0,false);
			SetCmd(cdefKeyDS3,0,0,false);
			SetCmd(cdefKeyE_3,0,0,false);
			SetCmd(cdefKeyF_3,0,0,false);
			SetCmd(cdefKeyFS3,0,0,false);
			SetCmd(cdefKeyG_3,0,0,false);
			SetCmd(cdefKeyGS3,0,0,false);
			SetCmd(cdefKeyA_3,0,0,false);
			SetCmd(cdefKeyAS3,0,0,false);
			SetCmd(cdefKeyB_3,0,0,false);
			
			// special
			SetCmd(cdefKeyStop,0,'1',false);
			SetCmd(cdefKeyStopAny,MOD_C,'1',false);
			SetCmd(cdefTweakM,0,192,false);        // tweak machine (`)
			SetCmd(cdefMIDICC,MOD_S,192,false);    // Previously Tweak Effect. Now Mcm Command (~)
			SetCmd(cdefTweakS,MOD_C,192,false);        // tweak machine smooth (`)

			// immediate commands
			SetCmd(cdefEditToggle,0,' ',false);

			SetCmd(cdefOctaveUp,0,VK_MULTIPLY,false);
			SetCmd(cdefOctaveDn,MOD_E,VK_DIVIDE,false);

			SetCmd(cdefMachineDec,MOD_C|MOD_E,VK_LEFT,false);
			SetCmd(cdefMachineInc,MOD_C|MOD_E,VK_RIGHT,false);

			SetCmd(cdefInstrDec,MOD_C|MOD_E,VK_DOWN,false);
			SetCmd(cdefInstrInc,MOD_C|MOD_E,VK_UP,false);

			SetCmd(cdefPlayRowTrack,0,'4',false);
			SetCmd(cdefPlayRowPattern,0,'8',false);
			SetCmd(cdefPlayStart,MOD_S,VK_F5,false);
			SetCmd(cdefPlaySong,0,VK_F5,false);
			SetCmd(cdefPlayBlock,0,VK_F6,false);
			SetCmd(cdefPlayFromPos,0,VK_F7,false);
			SetCmd(cdefPlayStop,0,VK_F8,false);


			SetCmd(cdefInfoPattern,MOD_C,VK_RETURN,false);
			SetCmd(cdefInfoMachine,MOD_S,VK_RETURN,false);
			SetCmd(cdefEditMachine,0,VK_F2,false);
			SetCmd(cdefEditPattern,0,VK_F3,false);
			SetCmd(cdefEditInstr,0,VK_F10,false);
			SetCmd(cdefAddMachine,0,VK_F9,false);

			SetCmd(cdefPatternInc,MOD_S|MOD_E,VK_UP,false);
			SetCmd(cdefPatternDec,MOD_S|MOD_E,VK_DOWN,false);
			SetCmd(cdefSongPosInc,MOD_S|MOD_E,VK_RIGHT,false);
			SetCmd(cdefSongPosDec,MOD_S|MOD_E,VK_LEFT,false);

			SetCmd(cdefColumnNext,0,VK_TAB,false);
			SetCmd(cdefColumnPrev,MOD_S,VK_TAB,false);


			SetCmd(cdefNavUp,MOD_E,VK_UP,false);
			SetCmd(cdefNavDn,MOD_E,VK_DOWN,false);
			SetCmd(cdefNavLeft,MOD_E,VK_LEFT,false);
			SetCmd(cdefNavRight,MOD_E,VK_RIGHT,false);

			SetCmd(cdefNavPageUp,MOD_E,VK_PRIOR,false);
			SetCmd(cdefNavPageDn,MOD_E,VK_NEXT,false);
			SetCmd(cdefNavTop,MOD_E,VK_HOME,false);
			SetCmd(cdefNavBottom,MOD_E,VK_END,false);

			SetCmd(cdefSelectMachine,0,VK_RETURN,false);		
			SetCmd(cdefUndo,MOD_C,'Z',false);
			SetCmd(cdefRedo,MOD_C|MOD_S,'Z',false);
			SetCmd(cdefFollowSong,MOD_C,'F',false);
			SetCmd(cdefMaxPattern,MOD_C,VK_TAB,false);

			// editor commands

			SetCmd(cdefTransposeChannelInc,MOD_C,VK_F2,false);	
			SetCmd(cdefTransposeChannelDec,MOD_C,VK_F1,false);	
			SetCmd(cdefTransposeChannelInc12,MOD_C|MOD_S,VK_F2,false);
			SetCmd(cdefTransposeChannelDec12,MOD_C|MOD_S,VK_F1,false);
			SetCmd(cdefTransposeBlockInc,MOD_C,VK_F12,false);
			SetCmd(cdefTransposeBlockDec,MOD_C,VK_F11,false);
			SetCmd(cdefTransposeBlockInc12,MOD_C|MOD_S,VK_F12,false);	
			SetCmd(cdefTransposeBlockDec12,MOD_C|MOD_S,VK_F11,false);

			SetCmd(cdefPatternCut,MOD_C,VK_F3,false);
			SetCmd(cdefPatternCopy,MOD_C,VK_F4,false);
			SetCmd(cdefPatternPaste,MOD_C,VK_F5,false);
			SetCmd(cdefRowInsert,MOD_E,VK_INSERT,false);
			SetCmd(cdefRowDelete,0,VK_BACK,false);
			SetCmd(cdefRowClear,MOD_E,VK_DELETE,false);

			SetCmd(cdefBlockStart,MOD_C,'B',false);
			SetCmd(cdefBlockEnd,MOD_C,'E',false);
			SetCmd(cdefBlockUnMark,MOD_C,'U',false);
			SetCmd(cdefBlockDouble,MOD_C,'D',false);
			SetCmd(cdefBlockHalve,MOD_C,'H',false);
			SetCmd(cdefBlockCut,MOD_C,'X',false);
			SetCmd(cdefBlockCopy,MOD_C,'C',false);
			SetCmd(cdefBlockPaste,MOD_C,'V',false);
			SetCmd(cdefBlockMix,MOD_C,'M',false);
			SetCmd(cdefBlockInterpolate,MOD_C,'I',false);
			SetCmd(cdefBlockSetMachine,MOD_C,'G',false);
			SetCmd(cdefBlockSetInstr,MOD_C,'T',false);

			SetCmd(cdefSelectAll,MOD_C,'A',false);
			SetCmd(cdefSelectCol,MOD_C,'R',false);

			SetCmd(cdefEditQuantizeDec,0,219,false);    // lineskip - 1
			SetCmd(cdefEditQuantizeInc,0,221,false);    // lineskip + 1

			SetCmd(cdefPatternMixPaste,MOD_C|MOD_S,VK_F5,false);
			SetCmd(cdefPatternTrackMute,MOD_C,VK_F9,false);
			SetCmd(cdefPatternTrackSolo,MOD_C,VK_F8,false);
			SetCmd(cdefPatternTrackRecord,MOD_C,VK_F7,false);

			SetCmd(cdefPatternDelete,MOD_C|MOD_S,VK_F3,false);
			SetCmd(cdefBlockDelete,MOD_C|MOD_S,'X',false);

			SetCmd(cdefSelectBar,MOD_C,'K',false);
		}

		void PsycleConfig::InputHandler::Load(ConfigStorage &store)
		{
			// Load from private profile if loading version 1.8.6
			if(store.GetVersion().empty())
			{
				KeyPresetIO::LoadOldPrivateProfile(*this);
			}
			else {
				store.OpenGroup("InputHandling");
				store.Read("bCtrlPlay", bCtrlPlay);
				store.Read("bFT2HomeBehaviour", bFT2HomeBehaviour);
				store.Read("bFT2DelBehaviour", bFT2DelBehaviour);
				store.Read("bShiftArrowsDoSelect", bShiftArrowsDoSelect);
				store.Read("MoveCursorPaste", bMoveCursorPaste);
			}

			store.Read("windowsBlocks", _windowsBlocks);
			store.Read("WrapAround", _wrapAround);
			store.Read("CursorAlwaysDown", _cursorAlwaysDown);
			store.Read("RecordMouseTweaksSmooth", _RecordMouseTweaksSmooth);
			store.Read("RecordUnarmed", _RecordUnarmed);
			store.Read("NavigationIgnoresStep", _NavigationIgnoresStep);
			store.Read("pageupStepSize", _pageUpSteps);
			store.Read("RecordNoteoff", _RecordNoteoff);
			store.Read("RecordTweaks", _RecordTweaks);
			store.Read("bMultiKey", bMultiKey);
			store.Read("notesToEffects", _notesToEffects);

			// Finish loading if version 1.8.8 and onwards
			if(!store.GetVersion().empty()) {
				char buffer[64];
				std::map<CmdSet,std::pair<int,int>>::const_iterator it;
				for(it = setMap.begin(); it != setMap.end(); it++)
				{
					sprintf(buffer, "CmdSet:%04d", it->first);

					int value = 0;
					store.Read(buffer, value);

					SetCmd(it->first,value>>8,value&0xFF);
				}
				store.CloseGroup();
			}
		}

		void PsycleConfig::InputHandler::Save(ConfigStorage &store)
		{
			store.CreateGroup("InputHandling");
			store.Write("bCtrlPlay", bCtrlPlay);
			store.Write("bFT2HomeBehaviour", bFT2HomeBehaviour);
			store.Write("bFT2DelBehaviour", bFT2DelBehaviour);
			store.Write("bShiftArrowsDoSelect", bShiftArrowsDoSelect);
			store.Write("MoveCursorPaste", bMoveCursorPaste);
			store.Write("windowsBlocks", _windowsBlocks);
			store.Write("wrapAround", _wrapAround);
			store.Write("cursorAlwaysDown", _cursorAlwaysDown);
			store.Write("RecordMouseTweaksSmooth", _RecordMouseTweaksSmooth);
			store.Write("RecordUnarmed", _RecordUnarmed);
			store.Write("NavigationIgnoresStep", _NavigationIgnoresStep);
			store.Write("pageupStepSize", _pageUpSteps);
			store.Write("RecordNoteoff", _RecordNoteoff);
			store.Write("RecordTweaks", _RecordTweaks);
			store.Write("bMultiKey", bMultiKey);
			store.Write("notesToEffects", _notesToEffects);

			char buffer[64];
			std::map<CmdSet,std::pair<int,int>>::const_iterator it;
			for(it = setMap.begin(); it != setMap.end(); it++)
			{
				sprintf(buffer, "CmdSet:%04d", it->first);

				int value = (it->second.first<<8) | (it->second.second&0xFF);
				store.Write(buffer, value);
			}
			store.CloseGroup();
		}
		void PsycleConfig::InputHandler::RefreshSettings()
		{
			//Nothing to refresh for InputHandler.
		}

		// SetCmd
		// in: command def, key, modifiers
		// out: true if we had to remove another definition
		///\todo more warnings if we are changing existing defs
		bool PsycleConfig::InputHandler::SetCmd(CmdDef const &cmd, UINT modifiers, UINT key, bool checkforduplicates)
		{	
			std::map<CmdSet,std::pair<int,int>>::iterator itSet;
			std::map<std::pair<int,int>,CmdDef>::iterator itKey;
			std::pair<int,int> theKey(modifiers,key);
			bool modified = false;

			if(!cmd.IsValid())
			{
				return false;
			}

			if(!checkforduplicates)
			{
				//Do not map a 0,0 key. Just add it to the set.
				if(key != 0 || modifiers != 0) {
					keyMap[theKey]=cmd;
				}
				setMap[cmd.GetID()]=theKey;
				return false;
			}
			// Update the keyMap entry if this key was already being used.
			// Also, change the setMap entry if the keymap used a different CmdDef.
			itKey = keyMap.find(theKey);
			if(itKey != keyMap.end())
			{
				if(itKey->second.GetID() != cmd.GetID())
				{
					modified = true;
					std::map<CmdSet,std::pair<int,int>>::iterator itPrevSet;
					itPrevSet = setMap.find(itKey->second.GetID());
					itPrevSet->second.first = 0;
					itPrevSet->second.second = 0;
				}
				itKey->second=cmd;
			}
			//Do not map a 0,0 key. Just add it to the set.
			else if(key != 0 || modifiers != 0)
			{
				keyMap[theKey]=cmd;
			}

			// Update the setMap entry.
			// Also change the keyMap if this entry was already mapped
			itSet = setMap.find(cmd.GetID());
			assert(itSet != setMap.end());
			if(itSet->second.first != 0 || itSet->second.second != 0) {
				std::map<std::pair<int,int>,CmdDef>::iterator itPrevKey;
				itPrevKey = keyMap.find(itSet->second);
				if(itPrevKey != keyMap.end()) 
				{
					itPrevKey->second=cmd;
				} else {
					//Command mapped, but the mapped key not found? Well, whatever, let's fix it.
					keyMap[theKey]=cmd;
				}
			}
			itSet->second = theKey;

			return modified;
		}
		///////////////////////////////////////////////
		PsycleConfig::Midi::Midi() : groups_(16), velocity_(0x0c), pitch_(1), raw_()
		{
			SetDefaultSettings();
		}
		PsycleConfig::Midi::~Midi()
		{
		}
		void PsycleConfig::Midi::SetDefaultSettings()
		{
			for(std::size_t i(0) ; i < groups().size() ; ++i)
			{
				group(i).message() = group(i).command() = static_cast<int>(i + 1);
				group(i).record()  = false;
				group(i).type()    = 0;
				group(i).from()    = 0;
				group(i).to()      = 0xff;
			}
			// enable velocity, raw and the default gen and inst selection
			pitch_.record()     = false;
			pitch_.type()       = 0; // 0 is cmd
			pitch_.command()    = 0x1;
			pitch_.from()       = 0;
			pitch_.to()         = 0xff;
			velocity_.record()  = true;
			velocity_.type()    = 0; // 0 is cmd
			velocity_.command() = 0xC;
			velocity_.from()    = 0;
			velocity_.to()      = 0xff;
			raw_                = true;
			gen_select_with_     = Midi::MS_USE_SELECTED;
			inst_select_with_    = Midi::MS_USE_SELECTED;
			_midiHeadroom = 100;
			_midiMachineViewSeqMode = false;
		}
		void PsycleConfig::Midi::Load(ConfigStorage &store)
		{
			// Do not open group if loading version 1.8.6
			if(!store.GetVersion().empty()) {
				store.OpenGroup("devices\\midi");
			}

			store.Read("MidiMachineViewSeqMode", _midiMachineViewSeqMode);
			store.Read("MidiInputHeadroom", _midiHeadroom);

			// velocity
			{
				store.Read("MidiRecordVel" , velocity_.record() );
				store.Read("MidiTypeVel"   , velocity_.type()   );
				store.Read("MidiCommandVel", velocity_.command());
				store.Read("MidiFromVel"   , velocity_.from()   );
				store.Read("MidiToVel"     , velocity_.to()     );
			}
			// pitch
			{
				store.Read("MidiRecordPit" , pitch_   .record() );
				store.Read("MidiTypePit"   , pitch_   .type()   );
				store.Read("MidiCommandPit", pitch_   .command());
				store.Read("MidiFromPit"   , pitch_   .from()   );
				store.Read("MidiToPit"     , pitch_   .to()     );
			}
			for(std::size_t i(0) ; i < groups().size() ; ++i)
			{
				std::ostringstream oss;
				oss << i;
				std::string s(oss.str());
				store.Read("MidiMessage" + s, group(i).message());
				store.Read("MidiRecord"  + s, group(i).record() );
				store.Read("MidiType"    + s, group(i).type()   );
				store.Read("MidiCommand" + s, group(i).command());
				store.Read("MidiFrom"    + s, group(i).from()   );
				store.Read("MidiTo"      + s, group(i).to()     );
			}
			store.Read("MidiRawMcm", raw());

			int with = static_cast<int>(gen_select_with_);
			store.Read("MidiGenSelectorType", with);
			gen_select_with_ = static_cast<Midi::selector_t>(with);

			with = static_cast<int>(inst_select_with_);
			store.Read("MidiInstSelectorType", with);
			inst_select_with_ = static_cast<Midi::selector_t>(with);

			// Close group if loading version 1.8.8 and onwards
			if(!store.GetVersion().empty()) {
				store.CloseGroup();
			}
		}
		void PsycleConfig::Midi::Save(ConfigStorage &store)
		{
			store.CreateGroup("devices\\midi");
			store.Write("MidiMachineViewSeqMode", _midiMachineViewSeqMode);
			store.Write("MidiInputHeadroom", _midiHeadroom);
			store.Write("MidiRecordVel" , velocity_.record() );
			store.Write("MidiTypeVel"   , velocity_.type()   );
			store.Write("MidiCommandVel", velocity_.command());
			store.Write("MidiFromVel"   , velocity_.from()   );
			store.Write("MidiToVel"     , velocity_.to()     );
			store.Write("MidiRecordPit" , pitch_   .record() );
			store.Write("MidiTypePit"   , pitch_   .type()   );
			store.Write("MidiCommandPit", pitch_   .command());
			store.Write("MidiFromPit"   , pitch_   .from()   );
			store.Write("MidiToPit"     , pitch_   .to()     );
			for(std::size_t i(0) ; i < groups().size() ; ++i)
			{
				std::ostringstream oss;
				oss << i;
				std::string s(oss.str());
				store.Write("MidiMessage" + s, group(i).message());
				store.Write("MidiRecord"  + s, group(i).record() );
				store.Write("MidiType"    + s, group(i).type()   );
				store.Write("MidiCommand" + s, group(i).command());
				store.Write("MidiFrom"    + s, group(i).from()   );
				store.Write("MidiTo"      + s, group(i).to()     );
			}
			store.Write("MidiRawMcm", raw());
			store.Write("MidiGenSelectorType", static_cast<int>(gen_select_with_));
			store.Write("MidiInstSelectorType", static_cast<int>(inst_select_with_));

			store.CloseGroup();
		}
		void PsycleConfig::Midi::RefreshSettings()
		{
			Global::midi().GetConfigPtr()->midiHeadroom = _midiHeadroom;
		}


		/////////////////////////////////////////////
		PsycleConfig::PsycleConfig() : Configuration()
			, audioSettings(5)
		{
			// soundcard output device
			{
				bool vista = Is_Vista_or_Later();
				bool asio = ASIOInterface::SupportsAsio();
				_numOutputDrivers = vista ? 5 : 4;
				if(!asio) _numOutputDrivers--;

				int idx = 0;
				audioSettings[idx++] = new SilentSettings();
				audioSettings[idx++] = new WaveOutSettings();
				audioSettings[idx++] = new DirectSoundSettings();
				if(asio) {
					audioSettings[idx++] = new ASIODriverSettings();
				}
				if (vista) {
					audioSettings[idx++] = new WasapiSettings();
				}
			}
			//All the other constructors already do SetDefaultSettings.
			SetDefaultSettings(false);
		}

		PsycleConfig::~PsycleConfig() throw()
		{
			if(_numOutputDrivers)
			{
				for (int i(0);i<_numOutputDrivers;++i)
				{
					delete audioSettings[i];
				}
			}
			delete(_pOutputDriver);
		}

		void PsycleConfig::SetDefaultSettings(bool include_others)
		{
			if(include_others)
			{
				Configuration::SetDefaultSettings();
				patView_.SetDefaultSettings();
				macView_.SetDefaultSettings();
				macParam_.SetDefaultSettings();
				input_.SetDefaultSettings();
				midi_.SetDefaultSettings();
				for(int i=0; i < _numOutputDrivers; i++)
				{
					audioSettings[i]->SetDefaultSettings();
				}
			}
			// soundcard output device
			{
				_outputDriverIndex = 2;
			}

			_midiDriverIndex = 0;
			_syncDriverIndex = 0;

			store_place_ = STORE_REGEDIT;
			_allowMultipleInstances = false;
			_showAboutAtStart = true;
			bShowSongInfoOnLoad = true;
			bFileSaveReminders = true;
			autosaveSong = true;
			autosaveSongTime = 10;

			useDoubleBuffer = true;
			_bShowPatternNames = false;
			_followSong = false;


			// paths
			{
				SetSongDir((universalis::os::fs::home() / "Songs").native_file_string());
				SetWaveRecDir((universalis::os::fs::home() / "Songs").native_file_string());
				SetInstrumentDir((universalis::os::fs::home() / "Instruments").native_file_string());
				SetSkinDir(appPath()+"Skins");
				SetPresetsDir((universalis::os::fs::home() / "Presets").native_file_string());
			}
			recent_files_.clear();
			recent_files_.push_back("");
			recent_files_.push_back("");
			recent_files_.push_back("");
			recent_files_.push_back("");
		}

		bool PsycleConfig::LoadPsycleSettings()
		{
			///\todo: Implement the different Stores

			//Open Current user.
			Registry reg(Registry::HKCU, "");
			if(!reg.OpenLocation(PSYCLE__SETTINGS__REGISTRY__ROOT))
			{
				return false;
			}
			// Try opening version 1.8.8. Else, resort to 1.8.6. If neither, return.
			if(!reg.OpenGroup(PSYCLE__SETTINGS__CONFIGKEY))
			{
				if(!reg.OpenGroup(PSYCLE__PATH__REGISTRY__1_8_6KEY))
				{
					reg.CloseLocation();
					return false;
				}
			}
			Load(reg);
			reg.CloseLocation();
			return true;
		}
		bool PsycleConfig::SavePsycleSettings()
		{
			ConfigStorage* store;
			switch(store_place_)
			{
			case STORE_REGEDIT: store = new Registry(Registry::HKCU, PSYCLE__VERSION);
				break;
			case STORE_USER_DATA: return false;///\todo:
				break;
			case STORE_EXE_DIR: return false;///\todo:
				break;
			default:
				return false;
			}
			if(!store->CreateLocation(PSYCLE__SETTINGS__REGISTRY__ROOT))
			{
				return false;
			}
			if(!store->CreateGroup(PSYCLE__SETTINGS__CONFIGKEY))
			{
				store->CloseLocation();
				return false;
			}
			Save(*store);
			store->CloseLocation();
			delete store;
			return true;
		}

		void PsycleConfig::Load(ConfigStorage & store)
		{
			Configuration::Load(store);

			store.Read("OutputDriver", _outputDriverIndex);

			store.Read("MidiInputDriver", _midiDriverIndex);
			store.Read("MidiSyncDriver", _syncDriverIndex);

			store.Read("SongDir", song_dir_);
			store.Read("WaveRecDir", wave_rec_dir_);
			store.Read("InstrumentDir", instrument_dir_);
			store.Read("SkinDir", skin_dir_);
			store.Read("PresetsDir", presets_dir_);

			store.Read("AllowMultipleInstances", _allowMultipleInstances);
			store.Read("showAboutAtStart", _showAboutAtStart);
			store.Read("bShowSongInfoOnLoad", bShowSongInfoOnLoad);
			store.Read("bFileSaveReminders", bFileSaveReminders);
			store.Read("autosaveSong", autosaveSong);
			store.Read("autosaveSongTime", autosaveSongTime);
			int storepl = 0;
			store.Read("storePlace",storepl);
			if (storepl < 0 || storepl > STORE_TYPES) {
				store_place_ = STORE_REGEDIT;
			}
			else {
				store_place_ = static_cast<store_t>(storepl);
			}


			store.Read("useDoubleBuffer", useDoubleBuffer);
			store.Read("ShowPatternNames", _bShowPatternNames);
			store.Read("FollowSong", _followSong);

			///\todo: Check what to do with these three.
			bool cpu_measurements;
			store.Read("UseCPUMeasurements", cpu_measurements);
			Global::player().measure_cpu_usage_ = cpu_measurements;

			store.Read("NewMacDlgpluginOrder", CNewMachine::pluginOrder);
			store.Read("NewMacDlgpluginName", CNewMachine::pluginName);
///
// The other Load() calls can change the group, so do not add settings of PsycleConfig below here.
///
			patView_.Load(store, skin_dir_);
			macView_.Load(store, skin_dir_);
			macParam_.Load(store,skin_dir_, macView_.machine_skin);
			input_.Load(store);
			midi_.Load(store);
			for(int i=0; i < _numOutputDrivers; i++)
			{
				audioSettings[i]->Load(store);
			}

			recent_files_.clear();
			store.OpenGroup("recent-files");
			std::string read;
			store.Read("0",read);
			recent_files_.push_back(read);
			read="";
			store.Read("1",read);
			recent_files_.push_back(read);
			read="";
			store.Read("2",read);
			recent_files_.push_back(read);
			read="";
			store.Read("3",read);
			recent_files_.push_back(read);
			store.CloseGroup();
///
//Do not add settings here, see the comment above about groups
///
		}

		void PsycleConfig::Save(ConfigStorage & store)
		{
			Configuration::Save(store);

			store.Write("OutputDriver", _outputDriverIndex);

			store.Write("MidiInputDriver", _midiDriverIndex);
			store.Write("MidiSyncDriver", _syncDriverIndex);

			store.Write("SongDir", GetSongDir());
			store.Write("WaveRecDir", GetWaveRecDir());
			store.Write("InstrumentDir", GetInstrumentDir());
			store.Write("SkinDir", GetSkinDir());
			store.Write("PresetsDir", GetPresetsDir());

			store.Write("AllowMultipleInstances", _allowMultipleInstances);
			store.Write("showAboutAtStart", _showAboutAtStart);
			store.Write("bShowSongInfoOnLoad", bShowSongInfoOnLoad);
			store.Write("bFileSaveReminders", bFileSaveReminders);
			store.Write("autosaveSong", autosaveSong);
			store.Write("autosaveSongTime", autosaveSongTime);
			store.Write("storePlace",static_cast<int>(store_place_));
			store.Write("useDoubleBuffer", useDoubleBuffer);
			store.Write("FollowSong", _followSong);
			store.Write("ShowPatternNames", _bShowPatternNames);

			bool cpu_measurements = Global::player().measure_cpu_usage_;
			store.Write("UseCPUMeasurements", cpu_measurements);

			store.Write("NewMacDlgpluginOrder", CNewMachine::pluginOrder);
			store.Write("NewMacDlgpluginName", CNewMachine::pluginName);

			patView_.Save(store);
			macView_.Save(store);
			macParam_.Save(store);
			input_.Save(store);
			midi_.Save(store);
			for(int i=0; i < _numOutputDrivers; i++)
			{
				audioSettings[i]->Save(store);
			}

			store.CreateGroup("recent-files");
			store.Write("0",recent_files_[0]);
			store.Write("1",recent_files_[1]);
			store.Write("2",recent_files_[2]);
			store.Write("3",recent_files_[3]);
			store.CloseGroup();
		}


		void PsycleConfig::RefreshSettings()
		{
			Configuration::RefreshSettings();
			SetCurrentSongDir(GetSongDir());
			SetCurrentWaveRecDir(GetWaveRecDir());
			SetCurrentInstrumentDir(GetInstrumentDir());
			
			patView_.RefreshSettings();
			macView_.RefreshSettings();
			macParam_.RefreshSettings();
			input_.RefreshSettings();
			midi_.RefreshSettings();

			RefreshAudio();

			theApp.RestoreRecentFiles();
		}
		
		void PsycleConfig::RefreshAudio()
		{
			bool refreshAudio = (!_pOutputDriver || !_pOutputDriver->Enabled());
			bool refreshMidi = refreshAudio;
			if(0 > _outputDriverIndex || _outputDriverIndex >= _numOutputDrivers) {
				_outputDriverIndex = 0;
				refreshAudio=true;
				refreshMidi=true;
			}
			int _numMidiDrivers = CMidiInput::GetNumDevices();
			if (0 > _midiDriverIndex || _midiDriverIndex > _numMidiDrivers) {
				_midiDriverIndex = 0;
				refreshMidi=true;
			}
			if (0 > _syncDriverIndex || _syncDriverIndex > _numMidiDrivers) {
				_syncDriverIndex = 0;
				refreshMidi=true;
			}

			if(refreshAudio) {
				OutputChanged(_outputDriverIndex);
			}
			if(refreshMidi) {
				Global::midi().Close();
				Global::midi().SetDeviceId(DRIVER_MIDI, _midiDriverIndex - 1);
				Global::midi().SetDeviceId(DRIVER_SYNC, _syncDriverIndex - 1);
				Global::midi().Open();
			}
		}
		void PsycleConfig::OutputChanged(int newidx)
		{
			if(0 > newidx || newidx >= _numOutputDrivers) {
				_outputDriverIndex = 0;
			}
			else { _outputDriverIndex = newidx; }
			delete(_pOutputDriver);
			_pOutputDriver = audioSettings[_outputDriverIndex]->NewDriver();
			_pOutputDriver->Initialize(Global::pPlayer->Work, Global::pPlayer);
			Global::pPlayer->SetSampleRate(audioSettings[_outputDriverIndex]->samplesPerSec());
			_pOutputDriver->Enable(true);
		}
		void PsycleConfig::MidiChanged(int newidx)
		{
			if (0 > newidx || newidx > CMidiInput::GetNumDevices()) {
				_midiDriverIndex = 0;
			}
			else { _midiDriverIndex = newidx; }
			Global::midi().Close();
			Global::midi().SetDeviceId(DRIVER_MIDI, _midiDriverIndex - 1);
			Global::midi().Open();
		}
		void PsycleConfig::SyncChanged(int newidx)
		{
			if (0 > newidx || newidx > CMidiInput::GetNumDevices()) {
				_syncDriverIndex = 0;
			}
			else { _syncDriverIndex = newidx; }
			Global::midi().Close();
			Global::midi().SetDeviceId(DRIVER_SYNC, _syncDriverIndex - 1);
			Global::midi().Open();
		}

        void PsycleConfig::AddRecentFile(std::string const &f)
		{
			for(int i = recent_files_.size()-1; i > 0; --i) 
			{
				recent_files_[i]=recent_files_[i-1];
			}
			recent_files_[0]=f;
		}


		void PsycleConfig::Error(std::string const & what)
		{
			AfxMessageBox(what.c_str(), MB_ICONERROR | MB_OK);
		}


		bool PsycleConfig::CreatePsyFont(CFont & f, std::string const & sFontFace, int const & HeightPx, bool const & bBold, bool const & bItalic)
		{
			f.DeleteObject();
			CString sFace(sFontFace.c_str());
			LOGFONT lf = LOGFONT();
			if(bBold) lf.lfWeight = FW_BOLD;
			if(bItalic) lf.lfItalic = true;
			lf.lfHeight = HeightPx;
			lf.lfQuality = DEFAULT_QUALITY;
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

		void PsycleConfig::PrepareMask(CBitmap* pBmpSource, CBitmap* pBmpMask, COLORREF clrTrans)
		{
			BITMAP bm;
			// Get the dimensions of the source bitmap
			pBmpSource->GetObject(sizeof(BITMAP), &bm);
			// Create the mask bitmap
			pBmpMask->DeleteObject();
			pBmpMask->CreateBitmap( bm.bmWidth, bm.bmHeight, 1, 1, NULL);
			// We will need two DCs to work with. One to hold the Image
			// (the source), and one to hold the mask (destination).
			// When blitting onto a monochrome bitmap from a color, pixels
			// in the source color bitmap that are equal to the background
			// color are blitted as white. All the remaining pixels are
			// blitted as black.
			CDC hdcSrc, hdcDst;
			hdcSrc.CreateCompatibleDC(NULL);
			hdcDst.CreateCompatibleDC(NULL);
			// Load the bitmaps into memory DC
			CBitmap* hbmSrcT = (CBitmap*) hdcSrc.SelectObject(pBmpSource);
			CBitmap* hbmDstT = (CBitmap*) hdcDst.SelectObject(pBmpMask);
			// Change the background to trans color
			hdcSrc.SetBkColor(clrTrans);
			// This call sets up the mask bitmap.
			hdcDst.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcSrc,0,0,SRCCOPY);
			// Now, we need to paint onto the original image, making
			// sure that the "transparent" area is set to black. What
			// we do is AND the monochrome image onto the color Image
			// first. When blitting from mono to color, the monochrome
			// pixel is first transformed as follows:
			// if  1 (black) it is mapped to the color set by SetTextColor().
			// if  0 (white) is is mapped to the color set by SetBkColor().
			// Only then is the raster operation performed.
			hdcSrc.SetTextColor(RGB(255,255,255));
			hdcSrc.SetBkColor(RGB(0,0,0));
			hdcSrc.BitBlt(0,0,bm.bmWidth, bm.bmHeight, &hdcDst,0,0,SRCAND);
			// Clean up by deselecting any objects, and delete the
			// DC's.
			hdcSrc.SelectObject(hbmSrcT);
			hdcDst.SelectObject(hbmDstT);
			hdcSrc.DeleteDC();
			hdcDst.DeleteDC();
		}
	}
}
