#pragma once

#include "ipsysongloader.h"
namespace SF {
	class PsyFSongLoader :
		public IPsySongLoader
	{
	public:
		PsyFSongLoader(void);
		virtual ~PsyFSongLoader(void);
		/// RIFFファイルからSongデータをロードする
		void Load(RiffFile& riffFile,Song& song,const bool fullopen = true);
	private:
		/// ファイル情報
		void LoadFileInfo(RiffFile& riffFile,Song& song,const bool fullopen = true);
		/// ソング情報
		void LoadSongInfo(RiffFile& riffFile,Song& song,const bool fullopen = true);
		/// シーケンスデータ
		void LoadSequenceData(RiffFile& riffFile,Song& song,const bool fullopen = true);
		/// パターンデータ
		void LoadPatternData(RiffFile& riffFile,Song& song,const bool fullopen = true);
		/// マシンデータの読み込み
		void LoadMachineData(RiffFile& riffFile,Song& song,const bool fullopen = true);
		/// インストルメントデータの読み込み
		void LoadInstrumentData(RiffFile& riffFile,Song& song,const bool fullopen = true);

		/// ソングロードメソッドへのポインタ
		typedef void (PsyFSongLoader::*pLoaderFunc)(RiffFile&,Song&,const bool);
		/// ローだ保持マップ
		typedef std::map<std::string,pLoaderFunc> LoadFuncMap;
		LoadFuncMap m_LoadFuncMap;

//		void Load(RiffFile& riffFile,Machine& machine,const bool fullopen = true);
	};
}
