#pragma once


#include "ipsysongsaver.h"
namespace SF {
	class PsyFSongSaver :
		public IPsySongSaver
	{
	public:
		PsyFSongSaver(void);
		virtual ~PsyFSongSaver(void);
		/// RIFFファイルからSongデータをロードする
		void Save(RiffFile& riffFile,::Song& song);
	private:
		/// ファイル情報
		void SaveFileInfo(RiffFile& riffFile,::Song& song);
		/// ソング情報
		void SaveSongInfo(RiffFile& riffFile,::Song& song);
		/// シーケンスデータ
		void SaveSequenceData(RiffFile& riffFile,::Song& song);
		/// パターンデータ
		void SavePatternData(RiffFile& riffFile,::Song& song);
		/// マシンデータ
		void SaveMachineData(RiffFile& riffFile,::Song& song);
		/// インストルメントデータ
		void SaveInstrumentData(RiffFile& riffFile,::Song& song);

		CProgressDialog m_Progress;

	};
}