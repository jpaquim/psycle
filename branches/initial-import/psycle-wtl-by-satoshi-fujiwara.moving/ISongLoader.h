#if !defined(ISONGLOADER_H)
#define ISONGLOADER_H

/** @file
 *  @brief ISongLoader Interface.
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

class Song;
namespace SF {
	/** ISongLoader Interface */
	struct ISongLoader {
		virtual ~ISongLoader(){;};
		/// RIFFファイルからソングをロードする
		virtual void Load(string& fileName,::Song& song,const bool fullopen = true) = 0;
		
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