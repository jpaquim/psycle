#pragma once
/** @file
 *  @brief interface of the D3D class
 *  $Date: 2005/01/15 22:44:19 $
 *  $Revision: 1.3 $
 */

namespace SF {

	class D3D : public CD3DApplication 
	{
	public:
		/// D3D Exception
		class Exception : public std::exception
		{
		public:
			Exception(const HRESULT hResult){
				m_Reason = ::DXGetErrorString9(hResult);
				m_Reason += _T("\n");
				m_Reason += ::DXGetErrorDescription9(hResult);
			};
			
			Exception(const string& reason)
			{
				m_Reason = reason;
			};

			const TCHAR * what() {return m_Reason.c_str();};
			const string& what_str() { return m_Reason;};
			const HRESULT hResult() { return m_hResult;};
		private:
			string m_Reason;
			HRESULT m_hResult;
		};

		static D3D & GetInstance(CWindow* pWindow = NULL){
			if(m_pD3D.get() == NULL && pWindow != NULL){
				m_pD3D = std::auto_ptr<D3D>(new D3D(*pWindow));
			}
			return (*m_pD3D);
		}

		static D3D * GetInstancePtr(CWindow* pWindow = NULL){
			if(m_pD3D.get() == NULL && pWindow != NULL){
				m_pD3D = std::auto_ptr<D3D>(new D3D(*pWindow));
			}
			return m_pD3D.get();
		}

		static void Release () {
			m_pD3D.reset();
		}

		/// Destructor
		virtual ~D3D();
		/// Create 3D Device
		void Initialize();
		void Render(CWindow& window);

		LPDIRECT3DDEVICE9 const pD3DDevice(){ return m_pd3dDevice;};
		CWindow & Window(){ return m_Window;};	    
		virtual HRESULT RestoreDeviceObjects();
//	    virtual HRESULT InvalidateDeviceObjects();
	private:
		/// Constructor
		D3D(CWindow& window);
		CWindow & m_Window;
	   ID3DXMesh*  m_pD3DXMesh;            // D3DX mesh to store teapot
	   static std::auto_ptr<D3D> m_pD3D;

	};
}
