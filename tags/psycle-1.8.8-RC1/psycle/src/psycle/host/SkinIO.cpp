///\file
///\brief implementation file for psycle::host::CSkinDlg.

#include <psycle/host/detail/project.private.hpp>
#include "SkinIO.hpp"
#include <sstream>
#include <boost/algorithm/string.hpp>
namespace psycle { namespace host {

		bool SkinIO::LocateSkinDir(CString findDir, CString findName, const char ext[], std::string& result)
		{
			CFileFind finder;
			bool found = false;
			int loop = finder.FindFile(findDir + "\\*");	// check for subfolders.
			while (loop) 
			{		
				loop = finder.FindNextFile();
				if (finder.IsDirectory() && !finder.IsDots())
				{
					found = LocateSkinDir(finder.GetFilePath(),findName, ext, result);
					if (found) {
						finder.Close();
						return found;
					}
				}
			}
			finder.Close();

			loop = finder.FindFile(findDir + "\\" + findName + ext); // check if the directory is empty
			while (loop)
			{
				loop = finder.FindNextFile();
				if (!finder.IsDirectory())
				{
					char szOpenName[MAX_PATH];
					std::sprintf(szOpenName,"%s\\%s.bmp",findDir,findName);
					FILE* file;
					file = std::fopen(szOpenName, "rb");
					if(file) {
						result = findDir;
						std::fclose(file);
						found = true;
						break;
					}
				}
			}
			finder.Close();
			return found;
		}

		void SkinIO::LocateSkins(CString findDir, std::list<std::string>& pattern_skins, std::list<std::string>& machine_skins)
		{
			CFileFind finder;
			int loop = finder.FindFile(findDir + "\\*");	// check for subfolders.
			while (loop) 
			{		
				loop = finder.FindNextFile();
				if (finder.IsDirectory() && !finder.IsDots())
				{
					LocateSkins(finder.GetFilePath(),pattern_skins, machine_skins);
				}
			}
			finder.Close();

			loop = finder.FindFile(findDir + "\\*.psh"); // check if the directory is empty
			while (loop)
			{
				loop = finder.FindNextFile();
				if (!finder.IsDirectory())
				{
					// ok so we have a .psh, does it have a valid matching .bmp?
					CString sName;
					char szOpenName[MAX_PATH];
					sName = finder.GetFileName();
					///\todo [bohan] const_cast for now, not worth fixing it imo without making something more portable anyway
					char* pExt = const_cast<char*>(strrchr(sName,46)); // last .
					pExt[0]=0;
					std::sprintf(szOpenName,"%s\\%s.bmp",findDir,sName);
					FILE* file;
					file = std::fopen(szOpenName, "rb");
					if(file) {
						std::sprintf(szOpenName,"%s\\%s",findDir,sName);
						pattern_skins.push_back(szOpenName);
						std::fclose(file);
					}
				}
			}
			finder.Close();
			loop = finder.FindFile(findDir + "\\*.psm"); // check if the directory is empty
			while (loop)
			{
				loop = finder.FindNextFile();
				if (!finder.IsDirectory())
				{
					// ok so we have a .psm, does it have a valid matching .bmp?
					CString sName;
					char szOpenName[MAX_PATH];
					sName = finder.GetFileName();
					///\todo [bohan] const_cast for now, not worth fixing it imo without making something more portable anyway
					char* pExt = const_cast<char*>(strrchr(sName,46)); // last .
					pExt[0]=0;
					std::sprintf(szOpenName,"%s\\%s.bmp",findDir,sName);
					FILE* file;
					file = std::fopen(szOpenName, "rb");
					if(file) {
						std::sprintf(szOpenName,"%s\\%s",findDir,sName);
						machine_skins.push_back(szOpenName);
						std::fclose(file);
					}
				}
			}
			finder.Close();
			pattern_skins.sort();
			machine_skins.sort();
		}
		void SkinIO::LoadTheme(const char* szFile, PsycleConfig::MachineView& macView,
			PsycleConfig::MachineParam& macParam, PsycleConfig::PatternView& patView)
		{
			std::FILE * hfile;
			if(!(hfile=std::fopen(szFile,"r")))
			{
				::MessageBox(0, "Couldn't open File for Reading. Operation Aborted",
					"File Open Error", MB_ICONERROR | MB_OK);
				return;
			}
			std::map<std::string,std::string> props;
			LoadProperties(hfile, props);

			std::string str1 = szFile;
			std::string skin_dir = str1.substr(0, str1.rfind('\\')).c_str();

			using helpers::hexstring_to_integer;
			hexstring_to_integer(props["pvc_separator"], patView.separator);
			hexstring_to_integer(props["pvc_separator2"], patView.separator2);
			hexstring_to_integer(props["pvc_background"], patView.background);
			hexstring_to_integer(props["pvc_background2"], patView.background2);
			hexstring_to_integer(props["pvc_row4beat"], patView.row4beat);
			hexstring_to_integer(props["pvc_row4beat2"], patView.row4beat2);
			hexstring_to_integer(props["pvc_rowbeat"], patView.rowbeat);
			hexstring_to_integer(props["pvc_rowbeat2"], patView.rowbeat2);
			hexstring_to_integer(props["pvc_row"], patView.row);
			hexstring_to_integer(props["pvc_row2"], patView.row2);
			hexstring_to_integer(props["pvc_font"], patView.font);
			hexstring_to_integer(props["pvc_font2"], patView.font2);
			hexstring_to_integer(props["pvc_fontPlay"], patView.fontPlay);
			hexstring_to_integer(props["pvc_fontPlay2"], patView.fontPlay2);
			hexstring_to_integer(props["pvc_fontCur"], patView.fontCur);
			hexstring_to_integer(props["pvc_fontCur2"], patView.fontCur2);
			hexstring_to_integer(props["pvc_fontSel"], patView.fontSel);
			hexstring_to_integer(props["pvc_fontSel2"], patView.fontSel2);
			hexstring_to_integer(props["pvc_selection"], patView.selection);
			hexstring_to_integer(props["pvc_selection2"], patView.selection2);
			hexstring_to_integer(props["pvc_playbar"], patView.playbar);
			hexstring_to_integer(props["pvc_playbar2"], patView.playbar2);
			hexstring_to_integer(props["pvc_cursor"], patView.cursor);
			hexstring_to_integer(props["pvc_cursor2"], patView.cursor2);

			patView.header_skin = props["pattern_header_skin"];
			if ( patView.header_skin.empty() || patView.header_skin == PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN)
			{
				patView.header_skin = "";
			}
			else if (patView.header_skin.rfind('\\') == -1)
			{
				patView.header_skin = skin_dir + '\\' + patView.header_skin;
				LoadPatternSkin((patView.header_skin + ".psh").c_str(),patView.PatHeaderCoords);
			}

			patView.font_name = props["pattern_fontface"];
			hexstring_to_integer(props["pattern_font_point"], patView.font_point);
			hexstring_to_integer(props["pattern_font_flags"], patView.font_flags);
			hexstring_to_integer(props["pattern_font_x"], patView.font_x);
			hexstring_to_integer(props["pattern_font_y"], patView.font_y);

			hexstring_to_integer(props["mv_colour"], macView.colour);
			hexstring_to_integer(props["mv_polycolour"], macView.polycolour);
			hexstring_to_integer(props["mv_triangle_size"], macView.triangle_size);
			hexstring_to_integer(props["mv_wirecolour"], macView.wirecolour);
			hexstring_to_integer(props["mv_wirewidth"], macView.wirewidth);
			hexstring_to_integer(props["mv_wireaa"], macView.wireaa);
			hexstring_to_integer(props["mv_generator_fontcolour"], macView.generator_fontcolour);
			hexstring_to_integer(props["mv_effect_fontcolour"], macView.effect_fontcolour);
			hexstring_to_integer(props["vu1"], macView.vu1);
			hexstring_to_integer(props["vu2"], macView.vu2);
			hexstring_to_integer(props["vu3"], macView.vu3);

			macView.machine_skin = props["machine_skin"];
			if (macView.machine_skin.empty() || macView.machine_skin == PSYCLE__PATH__DEFAULT_MACHINE_SKIN)
			{
				macView.machine_skin = "";
			}
			else if(macView.machine_skin.rfind('\\') == -1)
			{
				macView.machine_skin = skin_dir + '\\' + macView.machine_skin;
				LoadMachineSkin((macView.machine_skin + ".psm").c_str(),macView.MachineCoords);
			}
			
			std::map<std::string,std::string>::iterator it = props.find("machine_background");
			// This is set to true only when doing RefreshSettings
			macView.bBmpBkg = false;
			if(it != props.end() && !it->second.empty()) {
				macView.szBmpBkgFilename = it->second;
				if(macView.szBmpBkgFilename.rfind('\\') == -1)
				{
					macView.szBmpBkgFilename = skin_dir + '\\' + macView.szBmpBkgFilename;
				}
			}
			else {
				macView.szBmpBkgFilename = "";
			}

			macView.generator_fontface = props["generator_fontface"];
			hexstring_to_integer(props["generator_font_point"], macView.generator_font_point);
			hexstring_to_integer(props["generator_font_flags"], macView.generator_font_flags);
			macView.effect_fontface = props["effect_fontface"];
			hexstring_to_integer(props["effect_font_point"], macView.effect_font_point);
			hexstring_to_integer(props["effect_font_flags"], macView.effect_font_flags);

			hexstring_to_integer(props["machineGUITopColor"], macParam.topColor);
			hexstring_to_integer(props["machineGUIFontTopColor"], macParam.fontTopColor);
			hexstring_to_integer(props["machineGUIBottomColor"], macParam.bottomColor);
			hexstring_to_integer(props["machineGUIFontBottomColor"], macParam.fontBottomColor);
			hexstring_to_integer(props["machineGUIHTopColor"], macParam.hTopColor);
			hexstring_to_integer(props["machineGUIHFontTopColor"], macParam.fonthTopColor);
			hexstring_to_integer(props["machineGUIHBottomColor"], macParam.hBottomColor);
			hexstring_to_integer(props["machineGUIHFontBottomColor"], macParam.fonthBottomColor);
			hexstring_to_integer(props["machineGUITitleColor"], macParam.titleColor);
			hexstring_to_integer(props["machineGUITitleFontColor"], macParam.fonttitleColor);

			it = props.find("machine_GUI_bitmap");
			if(it != props.end() && !it->second.empty()) {
				macParam.szBmpControlsFilename = it->second;
				if(macParam.szBmpControlsFilename.rfind('\\') == -1)
				{
					macParam.szBmpControlsFilename = skin_dir + '\\' + macParam.szBmpControlsFilename;
				}
			}
			else {
				macParam.szBmpControlsFilename = "";
			}

			//
			//
			//
			// legacy...
			//
			//
			//
			it = props.find("machine_fontface");
			if(it != props.end()) {
				macView.generator_fontface = it->second;
				macView.effect_fontface = it->second;
			}
			it = props.find("machine_font_point");
			if(it != props.end()) {
				hexstring_to_integer(it->second, macView.generator_font_point);
				hexstring_to_integer(it->second, macView.effect_font_point);
			}
			it = props.find("mv_fontcolour");
			if(it != props.end()) {
				hexstring_to_integer(it->second, macView.generator_fontcolour);
				hexstring_to_integer(it->second, macView.effect_fontcolour);
			}
			std::fclose(hfile);
		}

		void SkinIO::SaveTheme(const char* szFile, PsycleConfig::MachineView& macView,
			PsycleConfig::MachineParam& macParam, PsycleConfig::PatternView& patView)
		{
			std::FILE * hfile;
			::CString str = szFile;
			::CString str2 = str.Right(4);
			if(str2.CompareNoCase(".psv")) str.Insert(str.GetLength(),".psv");
			if(!(hfile=std::fopen(str,"wb")))
			{
				::MessageBox(0, "Couldn't open File for Writing. Operation Aborted",
					"File Save Error", MB_ICONERROR | MB_OK);
				return;
			}
			std::fprintf(hfile,"[Psycle Display Presets v1.0]\n\n");
			std::fprintf(hfile,"\"pattern_fontface\"=\"%s\"\n", patView.font_name.c_str());
			std::fprintf(hfile,"\"pattern_font_point\"=dword:%.8X\n", patView.font_point);
			std::fprintf(hfile,"\"pattern_font_flags\"=dword:%.8X\n", patView.font_flags);
			std::fprintf(hfile,"\"pattern_font_x\"=dword:%.8X\n", patView.font_x);
			std::fprintf(hfile,"\"pattern_font_y\"=dword:%.8X\n", patView.font_y);
			if(patView.header_skin.empty())
			{
				std::fprintf(hfile,"\"pattern_header_skin\"=\"%s\"\n", PSYCLE__PATH__DEFAULT_PATTERN_HEADER_SKIN);
			}
			else
			{
				std::string str1 = patView.header_skin;
				std::string str2 = str1.substr(str1.rfind('\\'));
				str2 = str2.substr(0,str2.length()-4);
				std::fprintf(hfile,"\"pattern_header_skin\"=\"%s\"\n", str2.c_str());
			}
			std::fprintf(hfile,"\"pvc_separator\"=dword:%.8X\n", patView.separator);
			std::fprintf(hfile,"\"pvc_separator2\"=dword:%.8X\n", patView.separator2);
			std::fprintf(hfile,"\"pvc_background\"=dword:%.8X\n", patView.background);
			std::fprintf(hfile,"\"pvc_background2\"=dword:%.8X\n", patView.background2);
			std::fprintf(hfile,"\"pvc_font\"=dword:%.8X\n", patView.font);
			std::fprintf(hfile,"\"pvc_font2\"=dword:%.8X\n", patView.font2);
			std::fprintf(hfile,"\"pvc_fontCur\"=dword:%.8X\n", patView.fontCur);
			std::fprintf(hfile,"\"pvc_fontCur2\"=dword:%.8X\n", patView.fontCur2);
			std::fprintf(hfile,"\"pvc_fontSel\"=dword:%.8X\n", patView.fontSel);
			std::fprintf(hfile,"\"pvc_fontSel2\"=dword:%.8X\n", patView.fontSel2);
			std::fprintf(hfile,"\"pvc_fontPlay\"=dword:%.8X\n", patView.fontPlay);
			std::fprintf(hfile,"\"pvc_fontPlay2\"=dword:%.8X\n", patView.fontPlay2);
			std::fprintf(hfile,"\"pvc_row\"=dword:%.8X\n", patView.row);
			std::fprintf(hfile,"\"pvc_row2\"=dword:%.8X\n", patView.row2);
			std::fprintf(hfile,"\"pvc_rowbeat\"=dword:%.8X\n", patView.rowbeat);
			std::fprintf(hfile,"\"pvc_rowbeat2\"=dword:%.8X\n", patView.rowbeat2);
			std::fprintf(hfile,"\"pvc_row4beat\"=dword:%.8X\n", patView.row4beat);
			std::fprintf(hfile,"\"pvc_row4beat2\"=dword:%.8X\n", patView.row4beat2);
			std::fprintf(hfile,"\"pvc_selection\"=dword:%.8X\n", patView.selection);
			std::fprintf(hfile,"\"pvc_selection2\"=dword:%.8X\n", patView.selection2);
			std::fprintf(hfile,"\"pvc_playbar\"=dword:%.8X\n", patView.playbar);
			std::fprintf(hfile,"\"pvc_playbar2\"=dword:%.8X\n", patView.playbar2);
			std::fprintf(hfile,"\"pvc_cursor\"=dword:%.8X\n", patView.cursor);
			std::fprintf(hfile,"\"pvc_cursor2\"=dword:%.8X\n", patView.cursor2);
			std::fprintf(hfile,"\"vu1\"=dword:%.8X\n", macView.vu1);
			std::fprintf(hfile,"\"vu2\"=dword:%.8X\n", macView.vu2);
			std::fprintf(hfile,"\"vu3\"=dword:%.8X\n", macView.vu3);
			std::fprintf(hfile,"\"generator_fontface\"=\"%s\"\n", macView.generator_fontface.c_str());
			std::fprintf(hfile,"\"generator_font_point\"=dword:%.8X\n", macView.generator_font_point);
			std::fprintf(hfile,"\"generator_font_flags\"=dword:%.8X\n", macView.generator_font_flags);
			std::fprintf(hfile,"\"effect_fontface\"=\"%s\"\n", macView.effect_fontface.c_str());
			std::fprintf(hfile,"\"effect_font_point\"=dword:%.8X\n", macView.effect_font_point);
			std::fprintf(hfile,"\"effect_font_flags\"=dword:%.8X\n", macView.effect_font_flags);

			if(macView.machine_skin.empty())
			{
				std::fprintf(hfile,"\"machine_skin\"=\"%s\"\n", PSYCLE__PATH__DEFAULT_MACHINE_SKIN);
			}
			else
			{
				std::string str1 = macView.machine_skin;
				std::string str2 = str1.substr(str1.rfind('\\'));
				str2 = str2.substr(0,str2.length()-4);
				std::fprintf(hfile,"\"machine_skin\"=\"%s\"\n", str2.c_str());
			}

			std::fprintf(hfile,"\"mv_colour\"=dword:%.8X\n", macView.colour);
			std::fprintf(hfile,"\"mv_wirecolour\"=dword:%.8X\n", macView.wirecolour);
			std::fprintf(hfile,"\"mv_polycolour\"=dword:%.8X\n", macView.polycolour);
			std::fprintf(hfile,"\"mv_generator_fontcolour\"=dword:%.8X\n", macView.generator_fontcolour);
			std::fprintf(hfile,"\"mv_effect_fontcolour\"=dword:%.8X\n", macView.effect_fontcolour);
			std::fprintf(hfile,"\"mv_wirewidth\"=dword:%.8X\n", macView.wirewidth);
			std::fprintf(hfile,"\"mv_wireaa\"=hex:%.2X\n", macView.wireaa);
			std::fprintf(hfile,"\"machine_background\"=\"%s\"\n", macView.szBmpBkgFilename.c_str());
			std::fprintf(hfile,"\"machine_GUI_bitmap\"=\"%s\"\n", macParam.szBmpControlsFilename.c_str());
			std::fprintf(hfile,"\"mv_triangle_size\"=hex:%.2X\n", macView.triangle_size);
			std::fprintf(hfile,"\"machineGUITopColor\"=dword:%.8X\n", macParam.topColor);
			std::fprintf(hfile,"\"machineGUIFontTopColor\"=dword:%.8X\n", macParam.fontTopColor);
			std::fprintf(hfile,"\"machineGUIBottomColor\"=dword:%.8X\n", macParam.bottomColor);
			std::fprintf(hfile,"\"machineGUIFontBottomColor\"=dword:%.8X\n", macParam.fontBottomColor);
			std::fprintf(hfile,"\"machineGUIHTopColor\"=dword:%.8X\n", macParam.hTopColor);
			std::fprintf(hfile,"\"machineGUIHFontTopColor\"=dword:%.8X\n", macParam.fonthTopColor);
			std::fprintf(hfile,"\"machineGUIHBottomColor\"=dword:%.8X\n", macParam.hBottomColor);
			std::fprintf(hfile,"\"machineGUIHFontBottomColor\"=dword:%.8X\n", macParam.fonthBottomColor);
			std::fprintf(hfile,"\"machineGUITitleColor\"=dword:%.8X\n", macParam.titleColor);
			std::fprintf(hfile,"\"machineGUITitleFontColor\"=dword:%.8X\n", macParam.fonttitleColor);
			std::fclose(hfile);
		}

		void SkinIO::LoadMachineSkin(const char* szFile, SMachineCoords& coords)
		{
			std::FILE * hfile;
			if(!(hfile=std::fopen(szFile,"r")))
			{
				::MessageBox(0, "Couldn't open File for Reading. Operation Aborted",
					"File Open Error", MB_ICONERROR | MB_OK);
				return;
			}
			std::map<std::string,std::string> props;
			LoadProperties(hfile, props);

			memset(&coords,0,sizeof(SMachineCoords));
			SetSkinSource(props["master_source"], coords.sMaster);
			SetSkinSource(props["generator_source"], coords.sGenerator);
			SetSkinSource(props["generator_vu0_source"], coords.sGeneratorVu0);
			SetSkinSource(props["generator_vu_peak_source"], coords.sGeneratorVuPeak);
			SetSkinSource(props["generator_pan_source"], coords.sGeneratorPan);
			SetSkinSource(props["generator_mute_source"], coords.sGeneratorMute);
			SetSkinSource(props["generator_solo_source"], coords.sGeneratorSolo);
			SetSkinSource(props["effect_source"], coords.sEffect);
			SetSkinSource(props["effect_vu0_source"], coords.sEffectVu0);
			SetSkinSource(props["effect_vu_peak_source"], coords.sEffectVuPeak);
			SetSkinSource(props["effect_pan_source"], coords.sEffectPan);
			SetSkinSource(props["effect_mute_source"], coords.sEffectMute);
			SetSkinSource(props["effect_bypass_source"], coords.sEffectBypass);

			SetSkinSource(props["generator_vu_dest"], coords.dGeneratorVu);
			SetSkinSource(props["generator_pan_dest"], coords.dGeneratorPan);
			SetSkinDest(props["generator_mute_dest"], coords.dGeneratorMute);
			SetSkinDest(props["generator_solo_dest"], coords.dGeneratorSolo);
			SetSkinDest(props["generator_name_dest"], coords.dGeneratorName);
			SetSkinSource(props["effect_vu_dest"], coords.dEffectVu);
			SetSkinSource(props["effect_pan_dest"], coords.dEffectPan);
			SetSkinDest(props["effect_mute_dest"], coords.dEffectMute);
			SetSkinDest(props["effect_bypass_dest"], coords.dEffectBypass);
			SetSkinDest(props["effect_name_dest"], coords.dEffectName);
			
			std::map<std::string,std::string>::iterator it = props.find("transparency");
			if (it != props.end()) {
				int val;
				helpers::hexstring_to_integer(it->second, val);
				coords.cTransparency = val;
				coords.bHasTransparency = true;
			}
			std::fclose(hfile);
		}

		void SkinIO::LoadPatternSkin(const char* szFile, SPatternHeaderCoords& coords)
		{
			std::FILE * hfile;
			if(!(hfile=std::fopen(szFile,"r")))
			{
				::MessageBox(0, "Couldn't open File for Reading. Operation Aborted",
					"File Open Error", MB_ICONERROR | MB_OK);
				return;
			}
			std::map<std::string,std::string> props;
			LoadProperties(hfile, props);

			memset(&coords,0,sizeof(SPatternHeaderCoords));
			SetSkinSource(props["background_source"], coords.sBackground);
			SetSkinSource(props["number_0_source"], coords.sNumber0);
			SetSkinSource(props["record_on_source"], coords.sRecordOn);
			SetSkinSource(props["mute_on_source"], coords.sMuteOn);
			SetSkinSource(props["solo_on_source"], coords.sSoloOn);

			SetSkinDest(props["digit_x0_dest"], coords.dDigitX0);
			SetSkinDest(props["digit_0x_dest"], coords.dDigit0X);
			SetSkinDest(props["record_on_dest"], coords.dRecordOn);
			SetSkinDest(props["mute_on_dest"], coords.dMuteOn);
			SetSkinDest(props["solo_on_dest"], coords.dSoloOn);

			std::map<std::string,std::string>::iterator it = props.find("transparency");
			if (it != props.end()) {
				int val;
				helpers::hexstring_to_integer(it->second, val);
				coords.cTransparency = val;
				coords.bHasTransparency = true;
			}
			std::fclose(hfile);
		}

		void SkinIO::LoadProperties(std::FILE* hfile, std::map<std::string,std::string> & props)
		{
			char buf[1 << 10];
			bool loaded = false;
			while(std::fgets(buf, sizeof buf, hfile))
			{
				if(buf[0] == '#' || (buf[0] == '/' && buf[1] == '/') || buf[0] == '\n')
				{ 
					// Skip comments
					continue;
				} 
				char* equal = std::strchr(buf,'=');
				if (equal != NULL)
				{
					equal[0]='\0';
					//Skip the double quotes containing strings
					std::string key;
					if(buf[0] == '"')
					{
						char *tmp = equal-1;
						tmp[0]='\0';
						tmp = buf+1;
						key = tmp;
					} else { key = buf;	}

					char* value = &equal[1];
					int length = std::strlen(value);
					if(value[0] == '"')
					{
						length-=2;
						value++;
						value[length-1]='\0';
					}
					else {
						//skip the "dword:"  and "hex:" keywords
						char* twodots = std::strchr(value,':');
						if(twodots != NULL) {
							value =  &twodots[1];
						}
					}
					std::string strvalue = value;
					props[key] = strvalue;
					loaded = true;
				}
			}
			if(!loaded)
			{
				::MessageBox(0, "No settings found in the specified file",
					"File Load Error", MB_ICONERROR | MB_OK);
				return;
			}
		}

		void SkinIO::SetSkinSource(const std::string& buf, SSkinSource& skin)
		{
			std::vector<std::string> result(4);
			boost::split(result, buf, boost::is_any_of(","));
			std::istringstream istreamx(result[0]);
			istreamx >> skin.x;
			if (result.size() > 1) {
				std::istringstream istreamy(result[1]);
				istreamy >> skin.y;
			}
			if (result.size() > 2) {
				std::istringstream istreamw(result[2]);
				istreamw >> skin.width;
			}
			if (result.size() > 3) {
				std::istringstream istreamh(result[3]);
				istreamh >> skin.height;
			}
		}

		void SkinIO::SetSkinDest(const std::string& buf, SSkinDest& skin)
		{
			std::vector<std::string> result(2);
			boost::split(result, buf, boost::is_any_of(","));
			std::istringstream istreamx(result[0]);
			istreamx >> skin.x;
			if (result.size() > 1) {
				std::istringstream istreamy(result[1]);
				istreamy >> skin.y;
			}
		}


	}   // namespace
}   // namespace