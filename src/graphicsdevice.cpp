/*
DX Graphics definitions
include D3DGraphics library files in project dependencies
*/
#include "graphicdevice.h"

/*
default constructor
*/
D3DGraphics::D3DGraphics()
{
	//variable initializations
	m_lpD3dInterface = NULL;
	m_lpD3dDev = NULL;
	m_lpFont = 0;
	
	m_lpVertexDeclaration = 0;
	m_pInputInterface = 0;
	m_pKeyboardDeviceInterface = 0;
	m_pMouseDeviceInterface = 0;

	m_hFX = 0;
	
	//presentation parameter variables
	m_dwVertexProcessing = 0;
	m_dwFillMode = D3DFILL_WIREFRAME;
	
	//directX display mode structure
	m_d3dDisplayMode;
}

/*
copy constructor
*/
D3DGraphics::D3DGraphics(const D3DGraphics &obj)
{
	m_bWindowed = obj.m_bWindowed;
	m_hWindow = obj.m_hWindow;
	m_hInstance = obj.m_hInstance;
	m_lpD3dInterface = obj.m_lpD3dInterface;
	m_lpD3dDev = obj.m_lpD3dDev;
	m_d3dCapability = obj.m_d3dCapability;
	m_d3dPP = obj.m_d3dPP;
	m_d3dDisplayMode = obj.m_d3dDisplayMode;
	m_dwVertexProcessing = obj.m_dwVertexProcessing;
}

/*
destructor
*/
D3DGraphics::~D3DGraphics()
{
	Release();
}

/*
specify and initialize d3d device
build FX file
*/
BOOL D3DGraphics::Init_Direct3D(HWND g_hWindow, DWORD width, DWORD height, BOOL Windowed, HINSTANCE hInstance)
{
	m_hWindow = g_hWindow;
	m_bWindowed = Windowed;
	m_nWindowWidth = width;
	m_nWindowHeight = height;
	m_hInstance = hInstance;
	
	/************************************************************************/
	/* create the interface then the device interface						*/
	/************************************************************************/
	
	//initialize direct3d
	m_lpD3dInterface = Direct3DCreate9(D3D_SDK_VERSION);
	if (!m_lpD3dInterface)
	{
		MessageBox(m_hWindow, TEXT("error creating direct3d interface"),
			TEXT("error"), MB_OK);
		Release();
		return 0;
	}
	//build presentation params
	if (!BuildPresentationParams())
	{
		MessageBox(m_hWindow, TEXT("error setting direct3d parameters"),
			TEXT("error"), MB_OK);
		Release();
		return 0;
	}
	//create direct3d device
	HR(m_lpD3dInterface->CreateDevice(
		D3DADAPTER_DEFAULT, //video adapter to use
		D3DDEVTYPE_HAL, //hardware abstraction layer
		m_hWindow, //window handle
		m_dwVertexProcessing, //vertex processing
		&m_d3dPP, //pointer to presentation parameters
		&m_lpD3dDev)); //pointer to m_lpD3dInterface device
	
	if (!m_lpD3dDev)
	{
		MessageBox(m_hWindow, TEXT("error creating direct3d device"),
			TEXT("error"), MB_OK);
		Release();
		return 0;
	}

	//Set the transform matrices to identity
	D3DXMatrixIdentity(&m_matWorld);
	
	// setup fx file, get techniques, and handles
	SetupFX();
	
	return TRUE;
}

/*
load an image from a file onto a surface
*/
LPDIRECT3DSURFACE9 D3DGraphics::LoadSurface(LPCSTR filename, D3DCOLOR transcolor)
{
	string stdStringFileName = string(filename);
	wstring wFileName = wstring(stdStringFileName.begin(), stdStringFileName.end());
	
	//initialize bitmap surface struct
	LPDIRECT3DSURFACE9 image = 0;
	//initialize image info struct
	D3DXIMAGE_INFO info;
	//get image info
	HR(D3DXGetImageInfoFromFile(wFileName.c_str(), &info));

	//create surface
	HR(m_lpD3dDev->CreateOffscreenPlainSurface(
		info.Width, //width of surface
		info.Height, //height of surface
		D3DFMT_X8R8G8B8, //color
		D3DPOOL_DEFAULT, //memory pool to use
		&image, //pointer to surface
		NULL)); //reserved(always NULL)

	//load surface from file into new surface
	HR(D3DXLoadSurfaceFromFile(
		image, //destination surface
		NULL, //destination palette
		NULL, //destination rectangle
		wFileName.c_str(), //source filename
		NULL, //source rectangle
		D3DX_DEFAULT, //controls how image is filtered
		transcolor, //for transparency (0 for none)
		NULL)); //source image info (usually NULL)

	//return image surface
	return image;
}

/*
load texture from a file
without mip-map chains
*/
LPDIRECT3DTEXTURE9 D3DGraphics::LoadTexture(LPCSTR filename, D3DCOLOR transcolor)
{
	string stdStringFileName = string(filename);
	wstring wFileName = wstring(stdStringFileName.begin(), stdStringFileName.end());
	
	//the texture pointer
	LPDIRECT3DTEXTURE9 texture = 0;
	//the struct for reading bitmap info
	D3DXIMAGE_INFO info;
	//get width and height of bitmap file
	HR(D3DXGetImageInfoFromFile(wFileName.c_str(), &info));

	//create new texture by loading bitmap image file
	HR(D3DXCreateTextureFromFileEx(
		m_lpD3dDev, //m_lpD3dInterface device object
		wFileName.c_str(), //bitmap filename
		info.Width, //width
		info.Height, //height
		1, //mip-map levels (1 for no chain)
		D3DPOOL_DEFAULT, //the type of surface
		D3DFMT_UNKNOWN, //surface format (default)
		D3DPOOL_DEFAULT, //memory class for the texture
		D3DX_DEFAULT, //image filter
		D3DX_DEFAULT, //mip filter
		transcolor, //transparent color
		&info, //file info (loaded from file)
		NULL, //color palette
		&texture)); //destination texture

	//return texture
	return texture;
}

/*
get capabilities and setup presentation parameters
*/
BOOL D3DGraphics::BuildPresentationParams()
{
	//query system hardware capabilities
	//get display mode
	HR(m_lpD3dInterface->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_d3dDisplayMode));
	// Check for hardware transform and lighting
	HR(m_lpD3dInterface->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &m_d3dCapability));
	if (m_d3dCapability.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) //check if HTL bit is 1 in caps
	{
		m_dwVertexProcessing = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		//check for pure device 
		if (m_d3dCapability.DevCaps & D3DDEVCAPS_PUREDEVICE) //check if PD bit is 1 in caps
		{
			m_dwVertexProcessing |= D3DCREATE_PUREDEVICE;
		} 
	}
	//if capabilities don't meet requirements use software processing
	else
	{
		m_dwVertexProcessing = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	
	//set direct3d presentation parameters
	ZeroMemory(&m_d3dPP, sizeof(m_d3dPP)); //clear old parameters
	m_d3dPP.Windowed = m_bWindowed; //true - windowed or false - fullscreen
	m_d3dPP.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_d3dPP.MultiSampleQuality = 0;
	m_d3dPP.SwapEffect = D3DSWAPEFFECT_COPY;
	m_d3dPP.BackBufferFormat = m_bWindowed ? D3DFMT_UNKNOWN : D3DFMT_X8R8G8B8;
	m_d3dPP.BackBufferCount = 1;
	m_d3dPP.BackBufferWidth = m_bWindowed ? 0 : m_d3dDisplayMode.Width;
	m_d3dPP.BackBufferHeight = m_bWindowed ? 0 : m_d3dDisplayMode.Height;
	m_d3dPP.hDeviceWindow = m_hWindow;
	m_d3dPP.EnableAutoDepthStencil = TRUE;
	m_d3dPP.FullScreen_RefreshRateInHz = m_bWindowed ? D3DPRESENT_RATE_DEFAULT : m_d3dDisplayMode.RefreshRate;
	m_d3dPP.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	//check for a useable depth/stencil format
	D3DFORMAT adapterFormat = m_bWindowed ? m_d3dDisplayMode.Format : D3DFMT_X8R8G8B8;
	if (SUCCEEDED(m_lpD3dInterface->CheckDeviceFormat(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE,
		D3DFMT_D24S8)))
	{
		m_d3dPP.AutoDepthStencilFormat = D3DFMT_D24S8;
	}	
	else if (SUCCEEDED(m_lpD3dInterface->CheckDeviceFormat(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE,
		D3DFMT_D24X8)))
	{
		m_d3dPP.AutoDepthStencilFormat = D3DFMT_D24X8;
	}
	else if (SUCCEEDED(m_lpD3dInterface->CheckDeviceFormat(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL, adapterFormat, D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE,
		D3DFMT_D16)))
	{
		m_d3dPP.AutoDepthStencilFormat = D3DFMT_D16;
	}
	else //depth/stencil formt not available, quit
	{
		return false;
	}
	
	return true;
}

/*
setup mouse and keyboard for input
*/
VOID D3DGraphics::CreateDirectInput()
{
	HRESULT hr;
	
	//create input interface
	HR(DirectInput8Create(m_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pInputInterface, 0));
	
	//create keyboard device interface
	HR(m_pInputInterface->CreateDevice(GUID_SysKeyboard, &m_pKeyboardDeviceInterface, 0));
	
	//set keyboard data format
	HR(m_pKeyboardDeviceInterface->SetDataFormat(&c_dfDIKeyboard));
	
	//set keyboard cooperative level
	HR(m_pKeyboardDeviceInterface->SetCooperativeLevel(m_hWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
	
	//acquire the keyboard
	if (m_pKeyboardDeviceInterface)
	{
		hr = m_pKeyboardDeviceInterface->Acquire();
	}
	
	//loop until we have acquired the keyboard
	while (FAILED(hr))
	{
		ZeroMemory(m_pCKeyboardState, sizeof(m_pCKeyboardState));
		hr = m_pKeyboardDeviceInterface->Acquire();
		
		//if somthing went wrong but all we need to do is reacquire the device then call acquire
		while(hr == DIERR_INPUTLOST) 
		{          
			hr = m_pKeyboardDeviceInterface->Acquire();
		}
		
		/*
		if we cannot get access to the input yet, i.e. access denied
		then loop and try again
		*/
		if (FAILED(hr))
		{
			continue;
		}
		
		//everything went well reread the state
		m_pKeyboardDeviceInterface->GetDeviceState(sizeof(m_pCKeyboardState), (void**)&m_pCKeyboardState);
	}
	
	//create mouse device interface
	HR(m_pInputInterface->CreateDevice(GUID_SysMouse, &m_pMouseDeviceInterface, 0));
	
	//create mouse data format
	HR(m_pMouseDeviceInterface->SetDataFormat(&c_dfDIMouse2));
	
	//create mouse cooperative level
	HR(m_pMouseDeviceInterface->SetCooperativeLevel(m_hWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
	
	//acquire mouse
	if (m_pMouseDeviceInterface)
	{
		hr = m_pMouseDeviceInterface->Acquire();
	}
	
	//loop until we can acquire mouse
	while (FAILED(hr))
	{
		ZeroMemory(&m_dimMouseState, sizeof(m_dimMouseState));
		hr = m_pMouseDeviceInterface->Acquire();
		
		//if somthing went wrong but all we need to do is reacquire the device then call acquire
		while(hr == DIERR_INPUTLOST) 
		{
			hr = m_pMouseDeviceInterface->Acquire();
		}
		
		/*
		if we cannot get access to the input yet, i.e. access denied
		then loop and try again
		*/
		if (FAILED(hr))
		{
			continue;
		}
		
		//everything went well reread the state
		m_pMouseDeviceInterface->GetDeviceState(sizeof(DIMOUSESTATE2), (void**)&m_dimMouseState);
	}
	
	ZeroMemory(m_pCKeyboardState, sizeof(m_pCKeyboardState));
	ZeroMemory(&m_dimMouseState, sizeof(m_dimMouseState));
}

/*
Set the backbuffer width and height if windowed, else go full screen
*/
VOID D3DGraphics::SetBackBufferSize(BOOL windowed, DWORD width, DWORD height)
{
	this->GetPresentationParams().BackBufferWidth = width;
	this->GetPresentationParams().BackBufferHeight = height;
	
	if (windowed)
	{
		this->GetPresentationParams().BackBufferFormat = D3DFMT_UNKNOWN;
		this->GetPresentationParams().Windowed = true;
	}
	else
	{
		this->GetPresentationParams().BackBufferFormat = D3DFMT_X8R8G8B8;
		this->GetPresentationParams().Windowed = false;
	}
}

/*
specify parameters and create font
*/
VOID D3DGraphics::MakeFont()
{
	D3DXFONT_DESC fontDesc;
	fontDesc.Height          = 20;
	fontDesc.Width           = 12;
	fontDesc.Weight          = FW_BOLD;
	fontDesc.MipLevels       = 0;
	fontDesc.Italic          = true;
	fontDesc.CharSet         = DEFAULT_CHARSET;
	fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	fontDesc.Quality         = DEFAULT_QUALITY;
	fontDesc.PitchAndFamily  = DEFAULT_PITCH | FF_DONTCARE;
	wcscpy_s(fontDesc.FaceName, LF_FACESIZE, _TEXT("Times New Roman"));
	
	HR(D3DXCreateFontIndirect(m_lpD3dDev, &fontDesc, &m_lpFont));
}

/*
free memory
free COM interfaces
*/
VOID D3DGraphics::Release()
{
	//release keyboard, mouse and input COM objects
	if (m_pKeyboardDeviceInterface)
	{
		HR(m_pKeyboardDeviceInterface->Unacquire());
		m_pKeyboardDeviceInterface->Release();
		m_pKeyboardDeviceInterface = 0;
	}
	if (m_pMouseDeviceInterface)
	{
		HR(m_pMouseDeviceInterface->Unacquire());
		m_pMouseDeviceInterface->Release();
		m_pMouseDeviceInterface = 0;
	}
	if (m_pInputInterface)
	{
		m_pInputInterface->Release();
		m_pInputInterface = 0;
	}

	if (m_lpVertexDeclaration)
	{
		m_lpVertexDeclaration->Release();
		m_lpVertexDeclaration = 0;
	}
	
	//release effect interface
	if (m_hFX)
	{
		m_hFX->Release();
		m_hFX = 0;
	}
	
	//release font interface
	if (m_lpFont)
	{
		m_lpFont->Release();
		m_lpFont = 0;
	}
	
	//release direct3d device
	if (m_lpD3dDev)
	{
		m_lpD3dDev->Release();
		m_lpD3dDev = 0;
	}
	
	//release direct3d interface
	if (m_lpD3dInterface)
	{
		m_lpD3dInterface->Release();
		m_lpD3dInterface = 0;
	}
}

/*
do on lost device
*/
VOID D3DGraphics::OnLostDevice()
{
	if (m_lpFont)
	{
		HR(m_lpFont->OnLostDevice());
	}
	
	if (m_hFX)
	{
		HR(m_hFX->OnLostDevice());
	}
}

/*
do on reset device
*/
VOID D3DGraphics::OnResetDevice()
{
	if (m_lpFont)
	{
		HR(m_lpFont->OnResetDevice());
	}
	
	if (m_hFX)
	{
		HR(m_hFX->OnResetDevice());
	}
}

/*
create D3DVERTEXELEMENT9 vertex declaration
*/
VOID D3DGraphics::CreateVertexDeclaration()
{
	//create array of D3DVertexElements
	D3DVERTEXELEMENT9 vertexElements[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
	//create declaration
	HR(m_lpD3dDev->CreateVertexDeclaration(vertexElements, &m_lpVertexDeclaration));
}

/*
poll the keyboard and mouse for changes
*/
VOID D3DGraphics::Poll()
{
	HRESULT hr;
	
	hr = m_pKeyboardDeviceInterface->GetDeviceState(sizeof(m_pCKeyboardState), (void**)&m_pCKeyboardState);
	if (FAILED(hr))
	{
		ZeroMemory(m_pCKeyboardState, sizeof(m_pCKeyboardState));
		hr = m_pKeyboardDeviceInterface->Acquire();
		
		//if somthing went wrong but all we need to do is reacquire the device then call acquire
		while(hr == DIERR_INPUTLOST) 
		{          
			hr = m_pKeyboardDeviceInterface->Acquire();
		}
		
		/*
		if we cannot get access to the input yet, i.e. access denied
		then return and try again next cycle
		*/
		if (FAILED(hr))
		{
			return;
		}
		
		//everything went well reread the state
		m_pKeyboardDeviceInterface->GetDeviceState(sizeof(m_pCKeyboardState), (void**)&m_pCKeyboardState);
	}
	
	hr = m_pMouseDeviceInterface->GetDeviceState(sizeof(DIMOUSESTATE2), (void**)&m_dimMouseState);
	if (FAILED(hr))
	{
		ZeroMemory(&m_dimMouseState, sizeof(m_dimMouseState));
		hr = m_pMouseDeviceInterface->Acquire();
		
		//if somthing went wrong but all we need to do is reacquire the device then call acquire
		while(hr == DIERR_INPUTLOST) 
		{          
			hr = m_pMouseDeviceInterface->Acquire();
		}
		
		/*
		if we cannot get access to the input yet, i.e. access denied
		then return and try again next cycle
		*/
		if (FAILED(hr))
		{
			return;
		}
		
		//everything went well reread the state
		m_pMouseDeviceInterface->GetDeviceState(sizeof(DIMOUSESTATE2), (void**)&m_dimMouseState);
	}
}

/*
check for shader support
create effect from file
get technique handle
get effect parameter handles
*/
VOID D3DGraphics::SetupFX()
{
	D3DCAPS9 caps;
	HR(m_lpD3dDev->GetDeviceCaps(&caps));
	
	// Check for vertex shader version 2.0 support.
	if(caps.VertexShaderVersion < D3DVS_VERSION(2, 0))
	{
		Release();
		exit(0);
	}
	
	// Check for pixel shader version 3.0 support.
	if(caps.PixelShaderVersion < D3DPS_VERSION(3, 0))
	{
		Release();
		exit(0);
	}
	
	ID3DXBuffer *error = 0;
	HR(D3DXCreateEffectFromFile(m_lpD3dDev, L"res/transformShading.fx", 0, 0,
		D3DXSHADER_DEBUG & D3DXSHADER_SKIPOPTIMIZATION, 0, &m_hFX, &error));
	if (error)
	{
		MessageBox(0, (TCHAR*)error->GetBufferPointer(), 0, 0);
		Release();
		exit(0);
	}
	
	m_hTech = m_hFX->GetTechniqueByName("transformTech");
	if (!m_hTech)
	{
		MessageBox(0, _TEXT("Invalid Technique name transformTech"), 0, 0);
		Release();
		exit(0);
	}
	
	/*
	get handles to variables defined in the FX file
	*/
	
	// fx get handle to world/view/projection variable
	m_hWorldViewProjection = m_hFX->GetParameterByName(0, "gWorldViewProjection");
	if (!m_hWorldViewProjection)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gWorldViewProjection"), 0, 0);
		Release();
		exit(0);
	}
	
	// fx get handle to the world inverse transpose matrix for transforming vertex normals
	m_hWorldInverseTranspose = m_hFX->GetParameterByName(0, "gWorldInverseTranspose");
	if (!m_hWorldInverseTranspose)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gWorldInverseTranspose"), 0, 0);
		Release();
		exit(0);
	}
	
	// fx get handle to mutable selector in FX file
	m_hChooser = m_hFX->GetParameterByName(0, "gIsFloor");
	if (!m_hChooser)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gIsFloor"), 0, 0);
		Release();
		exit(0);
	}
	
	// fx get handle to offset
	m_hOffsetXY = m_hFX->GetParameterByName(0, "gOffsetXY");
	if (!m_hOffsetXY)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gOffsetXY"), 0, 0);
		Release();
		exit(0);
	}
	
	// fx get handle to mesh
	// fx get handle to mutable selector in FX file
	m_hIsMesh = m_hFX->GetParameterByName(0, "gIsMesh");
	if (!m_hIsMesh)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gIsMesh"), 0, 0);
		Release();
		exit(0);
	}

	// fx get handle to the bounding box boolean
	// fx get handle to mutable selector in FX file
	m_hIsBoundingBox = m_hFX->GetParameterByName(0, "gIsBoundingBox");
	if (!m_hIsBoundingBox)
	{
		MessageBox(0, _TEXT("Invalid Parameter name m_hIsBoundingBox"), 0, 0);
		Release();
		exit(0);
	}
}