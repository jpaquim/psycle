#pragma once
/** @file
 *  @brief Drawer Interface
 *  $Date: 2004/11/09 12:40:35 $
 *  $Revision: 1.1 $
 */
namespace SF {
	class IDrawer
	{
	public:
		class Exception : public std::exception
		{

		};

		IDrawer(void){};
		~IDrawer(void){};
		
		void LoadBitmap(const string & path) = 0;
		void LoadBitmap(const TCHAR *) = 0;
		void Load() = 0;
		void Initialize() = 0;
		void Begin() = 0;

		/// Draw Method 
		void DrawBitMap(
			const CRect* pSourceRect = NULL,
			const CWindow* pDestWnd = NULL,
			const CRect* pDestRect = NULL,
			const bool bPresent = true) = 0;

		void End() = 0;
		void Present() = 0;
	};
}
