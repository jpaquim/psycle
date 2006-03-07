#if !defined(IPSYSONGLOADER_H)
#define IPSYSONGLOADER_H

/** @file
 *  @brief IPsySongLoader Interface.
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.4 $
 */

class RiffFile;
namespace SF {
	/** IPsySongLoader Interface */
	struct IPsySongLoader {
		virtual ~IPsySongLoader(){;};
		/// RIFFファイルからソングをロードする
		virtual void Load(RiffFile& riffFile,::Song& song,const bool fullopen = true) = 0;
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