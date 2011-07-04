/** @file
 *  @brief Volume UI 
 *  $Date: 2004/10/07 21:27:21 $
 *  $Revision: 1.3 $
 */
#include "stdafx.h"


#include "D3D.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "volume.h"
namespace SF {
	namespace UI {
	
		template <class ValueT > CComPtr<ID3DXSprite>  Volume<ValueT>::m_pVolumeSprite = NULL;
		template <class ValueT > CComPtr<IDirect3DTexture9>  Volume<ValueT>::m_pTexture = NULL;

		template <class ValueT > 
		Volume<ValueT>::Volume()
		{
			// GetWndClassInfo().Register(&m_pfnSuperWindowProc);
			
			::WNDCLASSEX _wc = GetWndClassInfo().m_wc;
			_wc.lpfnWndProc = &::DefWindowProc;
			::RegisterClassEx(&_wc);
			
			m_bCapture = false;
			m_Value = 0;
			m_Tick = 0;
			m_Min = 0;
			m_Max = 0;

			//GetWndClassInfo().Register(&m_pfnSuperWindowProc);
			//_AtlWinModule.AddCreateWndData(&m_thunk.cd, this);
		}

		template <class ValueT > 
		Volume<ValueT>::~Volume()
		{

		}

		template <class ValueT > 
		void Volume<ValueT>::Initialize()
		{

			LPDIRECT3DDEVICE9 _pdevice = SF::D3D::GetInstance().pD3DDevice();
		
			if(m_pVolumeSprite == NULL){
				HRESULT _hr = D3DXCreateSprite(_pdevice,&m_pVolumeSprite);
				if(_hr != D3D_OK){
					throw SF::D3D::Exception(_hr);
				}
			}
			
			GetWindowRect(&m_RectSize);

			if(m_pTexture == NULL)
			{
///*
//				D3DXIMAGE_INFO *pSrcInfo;
				HRESULT _hr = ::D3DXCreateTextureFromFileEx
					(_pdevice, // LPDIRECT3DDEVICE9 pDevice
					_T("c:\\test\\v01.png"),// LPCTSTR pSrcFile
					NULL,// UINT Width
					NULL,// UINT Height
					NULL,// MipLevels
					NULL,// DWORD Usage
					D3DFMT_UNKNOWN,// D3DFORMAT Format
					D3DPOOL_DEFAULT,// D3DPOOL Pool
					D3DX_FILTER_LINEAR,// DWORD Filter
					D3DX_FILTER_LINEAR,// DWORD MipFilter
					D3DCOLOR(0xffffffff),// color key 
					NULL,// D3DXIMAGE_INFO *pSrcInfo
					NULL,// PALETTEENTRY *pPalette,
					&m_pTexture// LPDIRECT3DTEXTURE9 *ppTexture
				);
//*/
//				HRESULT _hr = ::D3DXCreateTextureFromFile
//					(_pdevice, // LPDIRECT3DDEVICE9 pDevice
//					_T("c:\\test\\v01.png"),// LPCTSTR pSrcFile
//					&m_pTexture// LPDIRECT3DTEXTURE9 *ppTexture
//					);

				if(_hr != D3D_OK){
					throw SF::D3D::Exception(_hr);
				}

				// OnPaint(NULL);

				
			}
		}// Volume::Initialize()
		
		// 描画処理
//		DWORD Volume::OnItemPrePaint(int nID, LPNMCUSTOMDRAW lpnmcd)

		template <class ValueT > 
		void Volume<ValueT>::OnPaint(HDC hdc)
		{

			ATLTRACE2(_T("OnPaint\n"));
			WTL::CRect _rect;
			GetClientRect(&_rect);
			ATLTRACE2(_T("width:%d height:%d\n"),_rect.Width(),_rect.Height());
			LPDIRECT3DDEVICE9 pdevice = D3D::GetInstance().pD3DDevice();
			pdevice->BeginScene();
			m_pVolumeSprite->Begin(D3DXSPRITE_ALPHABLEND);
			m_pVolumeSprite->Draw(m_pTexture,NULL,NULL,NULL,0xFFFFFFFF);
			m_pVolumeSprite->Flush();
			m_pVolumeSprite->End();
			pdevice->EndScene();
			pdevice->Present(CRect(0,0,128,128),_rect,m_hWnd,NULL);

		
			SetMsgHandled(FALSE);
//			return CDRF_SKIPDEFAULT;
//			return CDRF_DODEFAULT;


		}// Volume::DrawItem()

		template <class ValueT > 
		HWND Volume<ValueT>::Create(HWND hWndParent, _U_RECT rect, LPCTSTR szWindowName,
			DWORD dwStyle, DWORD dwExStyle, _U_MENUorID MenuOrID, LPVOID lpCreateParam)
		{
			HWND hwnd = CWindowImpl<Volume>::Create(hWndParent,rect,szWindowName,dwStyle, dwExStyle, MenuOrID, lpCreateParam);
			Initialize();
			return hwnd;
		}

		// 明示的なインスタンス生成
		// 参照：http://www.fides.dti.ne.jp/~oka-t/cpplab-template-3.html

		template class Volume<float>;
		template class Volume<int>;
		template class Volume<double>;
		
	}
}
