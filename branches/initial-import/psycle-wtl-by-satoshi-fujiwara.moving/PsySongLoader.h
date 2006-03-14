#pragma once

#include "isongloader.h"
/** @file
 *  @brief Psy2SongLoader Implement Class.
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */
namespace SF {
	class PsySongLoader : public ISongLoader
	{
	public:
		PsySongLoader(void);
		virtual ~PsySongLoader(void);
		/// RIFFファイルからソングをロードする
		virtual void Load(string& fileName,::Song& song,const bool fullopen = true);
	private:
		typedef std::map<std::string,SF::IPsySongLoader*> PsySongLoaderMap;
		PsySongLoaderMap m_Loader;///Psy ファイルローダー
	};
}
