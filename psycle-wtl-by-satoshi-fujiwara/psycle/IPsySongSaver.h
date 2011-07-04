#if !defined(ISONGSAVER_H)
#define ISONGSAVER_H

/** @file
 *  @brief IPsySongSaver Interface.
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

class Song;
class RiffFile;
namespace SF {
	/** IPsySongSaver Interface */
	struct IPsySongSaver {
		virtual ~IPsySongSaver(){;};
		/// RIFFファイルからソングをロードする
		virtual void Save(RiffFile& riffFile,::Song& song) = 0;
		
		/// Exception
		class Exception : public std::exception
		{
		public:
			Exception(const string& reason)
			{
				m_Reason = reason;
			};
			const TCHAR * what() {return m_Reason.c_str();};
			const string& what_str() { return m_Reason;};
		private:
			string m_Reason;
		};
		///
	};
}
#endif