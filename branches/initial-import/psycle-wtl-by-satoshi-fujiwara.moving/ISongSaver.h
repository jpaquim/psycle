#if !defined(ISONGSAVER_H)
#define ISONGSAVER_H

/** @file
 *  @brief ISongSaver Interface.
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.3 $
 */

class Song;
namespace SF {
	/** ISongSaver Interface */
	struct ISongSaver {
		virtual ~ISongSaver(){;};
		/// RIFFファイルからソングをロードする
		virtual void Save(std::string& fileName,::Song& song,const bool fullopen = true) = 0;
		
		/// Exception
		class Exception : public std::exception
		{
		public:
			Exception(const std::string& reason)
			{
				m_Reason = reason;
			};
			const char * what() {return m_Reason.c_str();};
			const std::string& what_str() { return m_Reason;};
		private:
			std::string m_Reason;
		};
		///
	};
}
#endif