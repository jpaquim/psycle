/** @file
 *  @brief implementation of the D3D class
 *  $Date: 2004/05/16 08:41:44 $
 *  $Revision: 1.2 $
 */
#include "stdafx.h"
#include ".\d3d.h"

namespace SF {
	
	std::auto_ptr<D3D> SF::D3D::m_pD3D;

	D3D::D3D(CWindow& window) : CD3DApplication(),m_Window(window)
	{
		m_d3dEnumeration.AppUsesDepthBuffer   = TRUE;

	}
	
	void D3D::Initialize()
	{
		m_hWnd = (HWND)m_Window;
		m_hWndFocus = m_hWnd;
		HRESULT _hres;

		if((_hres = Create(NULL)) != S_OK)
		{
			throw Exception(_hres);
		};

	    if( FAILED( _hres = D3DXCreateTeapot( m_pd3dDevice, &m_pD3DXMesh, NULL ) ) )
		{
			throw Exception(_hres);
		}

	}
	

	D3D::~D3D()
	{
		ATLTRACE2("~D3D()\n");
		SAFE_RELEASE( m_pD3DXMesh );
		SAFE_RELEASE( m_pd3dDevice);
	}

	void D3D::Render(CWindow& window){
		static float _fWorldRotY = 0.0f;
		static float _fWorldRotX = 0.0f;
	
		// Clear the viewport
		m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
							0x000000ff, 1.0f, 0L );
		//m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET,
		//					0x000000ff, 1.0f, 0L );
/*		D3DXMATRIX matWorld;
		D3DXMATRIX matRotY;
		D3DXMATRIX matRotX;

		_fWorldRotY += 1.5f;
		_fWorldRotX += 1.4f;
		
		D3DXMatrixRotationX( &matRotX, _fWorldRotX );
		D3DXMatrixRotationY( &matRotY, _fWorldRotY );

		D3DXMatrixMultiply( &matWorld, &matRotX, &matRotY );
		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matWorld );
*/
		D3DMATRIX matrix_trans=
		{
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
//			-1.0f,-1.0f,2.0f,1
			m_d3dsdBackBuffer.Width / 2,-1.0f *  (m_d3dsdBackBuffer.Height / 2),-900.0f,1

		};

		D3DXMATRIX _mscale;
//		D3DXMatrixScaling(&_mscale,50.0f,50.0f,50.0f);
		D3DXMatrixScaling(&_mscale,1.0f,1.0f,1.0f);

		m_pd3dDevice->SetTransform( D3DTS_WORLD, &matrix_trans);
		m_pd3dDevice->MultiplyTransform( D3DTS_WORLD, &_mscale);

		
		// Begin the scene
		if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
		{
			// TODO: render world
	        
			// Render the teapot mesh
			m_pD3DXMesh->DrawSubset(0);

			// Render stats and help text  
			//RenderText();

			// End the scene.
			m_pd3dDevice->EndScene();
		}

		m_pd3dDevice->Present(NULL,NULL,window,NULL);
	};

	//-----------------------------------------------------------------------------
	// Name: RestoreDeviceObjects()
	// Desc: Paired with InvalidateDeviceObjects()
	//       The device exists, but may have just been Reset().  Resources in
	//       D3DPOOL_DEFAULT and any other device state that persists during
	//       rendering should be set here.  Render states, matrices, textures,
	//       etc., that don't change during rendering can be set once here to
	//       avoid redundant state setting during Render() or FrameMove().
	//-----------------------------------------------------------------------------
	HRESULT D3D::RestoreDeviceObjects()
	{
		// TODO: setup render states
		HRESULT hr;

		// Setup a material
		D3DMATERIAL9 mtrl;
		D3DUtil_InitMaterial( mtrl, 1.0f, 0.0f, 0.0f );
		m_pd3dDevice->SetMaterial( &mtrl );

		// Set up the textures
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
		m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
		m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

		// Set miscellaneous render states
		m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
		m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,        TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,        0x000F0F0F );

		// Set the world matrix
		D3DXMATRIX matIdentity;
		D3DXMatrixIdentity( &matIdentity );
		m_pd3dDevice->SetTransform( D3DTS_WORLD,  &matIdentity );


		RECT _rect;
		GetClientRect(m_hWnd,&_rect);

		float _dwidth = m_d3dsdBackBuffer.Width;
		float _dheight = m_d3dsdBackBuffer.Height;

		//視野角度
		float _view_angle = 30.0f;
		//デバイスサイズの半分を各頂点に
		float _width = _dwidth / 2.0f;
		float _height = _dheight / 2.0f;
		//アスペクト比(高さを1としたときの幅)
		float _aspect = _dwidth / _dheight;
		//視野をZ=0でデバイスの幅と高さに合わせる
		float _fovy = _view_angle * D3DX_PI / 180.0f;                
		//奥行き
		float _depth = _height / (float)tan(_fovy / 2.0f);                   

		//ビューの設定
		//D3DXMatrixLookAtLH( &m_matView, 
		//	&D3DXVECTOR3( 0, 0,-fHeight),
		//	&D3DXVECTOR3( 0, fDepth,-fHeight),
		//	&D3DXVECTOR3( 0.0f, 0.0f, -1.0f ) );
		//m_matView *= m_matExt;

		//D3DXMATRIX matrix;
		//D3DXMatrixLookAtLH( &m_matViewBase, 
		//	&D3DXVECTOR3( fWidth,-fHeight,-fDepth),
		//	&D3DXVECTOR3( fWidth,-fHeight, 0.0f),
		//	&D3DXVECTOR3( 0.0f, 1.0f, 0.0f ) );
		////奥行きに対する比率を調整
		////近くの位置は0を越さないとZバッファが無効になる
		//D3DXMatrixPerspectiveFovLH( &m_matProjection,fFovy, fAspect,200.0f, 5000.0f );

		// Set up our view matrix. A view matrix can be defined given an eye point,
		// a point to lookat, and a direction for which way is up. Here, we set the
		// eye five units back along the z-axis and up three units, look at the
		// origin, and define "up" to be in the y-direction.
	///*	
		D3DXMATRIX matView;
		D3DXVECTOR3 vFromPt   = D3DXVECTOR3( _width, -_height,-_depth);
//		D3DXVECTOR3 vFromPt   = D3DXVECTOR3( _width, -_height,-100.0f);

		D3DXVECTOR3 vLookatPt = D3DXVECTOR3( _width, -_height, 0.0f );
		D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		D3DXMatrixLookAtLH( &matView, &vFromPt, &vLookatPt, &vUpVec );
		m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

		// Set the projection matrix
		D3DXMATRIX matProj;
//		FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
		D3DXMatrixPerspectiveFovLH( &matProj, _fovy, _aspect, 1.0f, 35000.0f );
//		D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 5000.0f );
		m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

		// Set up lighting states
		D3DLIGHT9 light;
		D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, -1.0f,-1.0f, 2.0f );
		
		light.Position.x = _width ;
		light.Position.y =  -_height;
		light.Position.z = -900.0f;
//		light.Range = 10000.0f;

		hr = m_pd3dDevice->SetLight( 0, &light );
		hr = m_pd3dDevice->LightEnable( 0, TRUE );
		hr = m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
		//*/
/*		
		D3DXMATRIX matView;
		D3DXVECTOR3 vFromPt   = D3DXVECTOR3( 0.0f, 0.0f, -5.0f );
		D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
		D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
		D3DXMatrixLookAtLH( &matView, &vFromPt, &vLookatPt, &vUpVec );
		m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

		// Set the projection matrix
		D3DXMATRIX matProj;
		FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
		D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f );
		m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

		// Set up lighting states
		D3DLIGHT9 light;
		D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, -1.0f, -1.0f, 1.0f );
		m_pd3dDevice->SetLight( 0, &light );
		m_pd3dDevice->LightEnable( 0, TRUE );
		m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
*/	
		// Restore the font
//		m_pFont->RestoreDeviceObjects();

		if( !m_bWindowed )
		{
			// Create a surface for configuring DInput devices
//			if( FAILED( hr = m_pd3dDevice->CreateOffscreenPlainSurface( 640, 480, 
//											m_d3dsdBackBuffer.Format, D3DPOOL_DEFAULT, 
//											&m_pDIConfigSurface, NULL ) ) ) 
//				return DXTRACE_ERR( "CreateOffscreenPlainSurface", hr );
		}

		return S_OK;
	}




}
