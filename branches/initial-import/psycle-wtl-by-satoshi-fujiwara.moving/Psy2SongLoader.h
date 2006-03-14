#pragma once


/** @file
 *  @brief Psy2SongLoader Implement Class.
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
#include "ipsysongloader.h"
namespace SF {
	class Psy2SongLoader :
		public IPsySongLoader
	{
	public:
		static const int NAME_LENGTH = 32;
		static const int AUTHOR_LENGTH = 32;
		static const int COMMENT_LENGTH = 128;

		Psy2SongLoader(void);
		virtual ~Psy2SongLoader(void);
			/// RIFFファイルからSongデータをロードする
		void Load(RiffFile& riffFile,::Song& song,const bool fullopen = true);
	};
}
