#pragma once

//ENABLE DirectX debug information
#if defined (DEBUG) | defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

//define direct input version
#define DIRECTINPUT_VERSION 0x0800

#define WHITE D3DCOLOR_XRGB(255, 255, 255)
#define BLACK D3DCOLOR_XRGB(0, 0, 0)
#define RED D3DCOLOR_XRGB(255, 0, 0)
#define GREEN D3DCOLOR_XRGB(0, 255, 0)
#define BLUE D3DCOLOR_XRGB(0, 0, 255)
#define YELLOW D3DCOLOR_XRGB(255, 255, 0)
#define CYAN D3DCOLOR_XRGB(0, 255, 255)
#define MAGENTA D3DCOLOR_XRGB(255, 0, 255)

#include <string>

//include DirectX libraries
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>
#include <tchar.h>
#include <dinput.h>
#include <stdio.h>

//ENABLE DirectX to output debug information, with file, line, error
#if defined(DEBUG) | defined(_DEBUG)
	#ifndef HR
	#define HR(x)										\
	{													\
		HRESULT hr = x;									\
		if(FAILED(hr))									\
		{												\
			DXTrace(__FILE__, __LINE__, hr, L#x, true);	\
		}												\
	}
	#endif

#else
	#ifndef HR
	#define HR(x) x;
	#endif
#endif

//vertex structure
struct Vertex
{
	//struct constructors
	Vertex():position(0,0,0),normal(0,0,0),texture(0,0){};
	Vertex(FLOAT x, FLOAT y, FLOAT z, FLOAT nx, FLOAT ny, FLOAT nz, FLOAT u, FLOAT v):position(x,y,z),normal(nx,ny,nz),texture(u,v){};
	Vertex(const D3DXVECTOR3 &v):position(v),normal(0,0,0),texture(0,0){};
	Vertex(const D3DXVECTOR3 &v, const D3DXVECTOR2 &uv):position(v),texture(uv){};
	Vertex(const D3DXVECTOR3 &v, const D3DXVECTOR3 &n, const D3DXVECTOR2 &uv):position(v),normal(n),texture(uv){};

	D3DXVECTOR3& operator += ( CONST D3DXVECTOR3& v){position = position + v; return position;};

	D3DXVECTOR3 position; //position
	D3DXVECTOR3 normal; //normal
	D3DXVECTOR2 texture; //texture
};

using namespace std;

class D3DGraphics
{
public:
	//default constructor
	D3DGraphics();
	//copy constructor
	D3DGraphics(const D3DGraphics &obj);
	//destructor
	~D3DGraphics();

	//build presentation parameters
	BOOL BuildPresentationParams();
	//create directInput
	VOID CreateDirectInput();
	//set vertex declarations
	VOID CreateVertexDeclaration();
	// initialize direct 3d
	BOOL Init_Direct3D(HWND, DWORD width, DWORD height, BOOL windowed, HINSTANCE hInstance);
	//load a surface
	LPDIRECT3DSURFACE9 LoadSurface(LPCSTR, D3DCOLOR);
	//load a texture
	LPDIRECT3DTEXTURE9 LoadTexture(LPCSTR, D3DCOLOR);
	//make font
	VOID MakeFont();
	//on lost and reset device 
	VOID OnLostDevice();
	VOID OnResetDevice();
	//poll keyboard and mouse
	VOID Poll();
	//build FX file
	VOID SetupFX();
	// set the size of the back buffer
	VOID SetBackBufferSize(BOOL windowed, DWORD width, DWORD height);
	// set window or full screen mode
	VOID SetWindowed(BOOL x){m_bWindowed = x;};
	
	//getters and setters
	LPD3DXFONT& GetFont(){return m_lpFont;}
	LPDIRECT3D9& GetInterface() {return m_lpD3dInterface;}
	LPDIRECT3DDEVICE9& GetDevice() {return m_lpD3dDev;}
	D3DPRESENT_PARAMETERS& GetPresentationParams() {return m_d3dPP;}
	BYTE* GetKeyboardState() {return m_pCKeyboardState;}
	DIMOUSESTATE2& GetMouseState() {return m_dimMouseState;}
	
	//to be moved
	ID3DXEffect* GetFXInterface() {return m_hFX;}
	D3DXHANDLE& GetTechniqueHandle() {return m_hTech;}
	D3DXHANDLE& GetChooserHandle() {return m_hChooser;}
	D3DXHANDLE& GetIsMeshHandle() {return m_hIsMesh;}
	D3DXHANDLE& GetIsBoundingBoxHandle() { return m_hIsBoundingBox; }
	D3DXHANDLE& GetOffsetHandle() {return m_hOffsetXY;}
	D3DXHANDLE& GetWorldViewProjectionHandle() {return m_hWorldViewProjection;}
	D3DXHANDLE& GetWorldInverseTransposeHandler() {return m_hWorldInverseTranspose;}
	LPDIRECT3DDEVICE9 GetD3DDevice() {return m_lpD3dDev;}
	LPDIRECT3DVERTEXDECLARATION9 GetD3DVertexDecl() {return m_lpVertexDeclaration;}
	D3DXMATRIX& GetWorldMatrix() {return m_matWorld;}
private:
	VOID Release();
	
	//prefix 'I' means COM
	BOOL m_bWindowed;
	// 256 byte keyboard character state
	BYTE m_pCKeyboardState[256];
	
	DWORD m_nWindowHeight;
	DWORD m_nWindowWidth;
	
	DWORD m_dwFillMode;
	DWORD m_dwVertexProcessing;
	DWORD m_dwMaterialsCount;
	
	DIMOUSESTATE2 m_dimMouseState;
	
	HWND m_hWindow;
	HINSTANCE m_hInstance;
	
	//presentation param variables
	D3DPRESENT_PARAMETERS m_d3dPP;
	D3DCAPS9 m_d3dCapability;
	D3DDISPLAYMODE m_d3dDisplayMode;
	//mesh diffuse
	D3DCOLORVALUE m_d3dcvDiffuseMesh;
	
	//world matrix
	D3DXMATRIX m_matWorld;
	
	//technique handle
	D3DXHANDLE m_hTech;
	//effect parameter handles
	D3DXHANDLE m_hWorldViewProjection;
	D3DXHANDLE m_hWorldInverseTranspose;
	//mutable selector
	D3DXHANDLE m_hChooser;
	//offset handles
	D3DXHANDLE m_hOffsetXY;
	//mesh tex selector
	D3DXHANDLE m_hIsMesh;
	//bounding box handle
	D3DXHANDLE m_hIsBoundingBox;
	
	//direct3d variables
	LPDIRECT3D9 m_lpD3dInterface;
	LPDIRECT3DDEVICE9 m_lpD3dDev;
	// vertex declaration
	LPDIRECT3DVERTEXDECLARATION9 m_lpVertexDeclaration;
	//font
	LPD3DXFONT m_lpFont;
	
	//keyboard, mouse input
	IDirectInput8 *m_pInputInterface;
	IDirectInputDevice8 *m_pKeyboardDeviceInterface;
	IDirectInputDevice8 *m_pMouseDeviceInterface;
	
	ID3DXEffect *m_hFX;
};