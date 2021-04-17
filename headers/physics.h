#pragma once

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>
#include <tchar.h>
#include <stdio.h>
#include "graphicdevice.h"

extern D3DGraphics *g_pD3DGraphics;

class Physics
{
public:
	Physics();
	~Physics();
	
	VOID Initialize();
	VOID Update(DOUBLE deltaTime);
	VOID Draw(D3DXMATRIX matView, D3DXMATRIX matProjection);
	VOID DrawPlane(D3DXMATRIX m_matView, D3DXMATRIX m_matProjection);
	VOID OnLostDevice();
	VOID OnResetDevice();
	//create terrain
	VOID CreateTerrain();
	//create boxes
	VOID CreateBoxes();
	VOID CreateSkyBox(); // create sphere
protected:
	DWORD m_nBoxNumVerts;
	DWORD m_nTriangleCount;
	// turbine vars
	DWORD m_nBatteryPower;
	DWORD m_nWindTurbineEnergyOutput;
	// plane height step size
	DWORD m_nPlaneHeightInterval;
	// plane width step size
	DWORD m_nPlaneWidthInterval;
	DWORD m_nPlaneNumRows;
	DWORD m_nPlaneNumColumns;
	DWORD m_nPlaneNumVerts;
	DWORD m_nCellRows;
	DWORD m_nCellColumns;
	DWORD m_nFloorTriangleCount;

	//plane indices
	DWORD *m_dwpPlaneIndices;

	Vertex *LPCubeVertexArray, *LPPlaneVertexArray;
	//plane vertex/index buffers
	IDirect3DVertexBuffer9 *m_pPlaneVertexbuffer;
	IDirect3DIndexBuffer9 *m_pPlaneIndexbuffer;

	//vertex/index variables
	IDirect3DVertexBuffer9 *m_pVertexbuffer;
	IDirect3DIndexBuffer9 *m_pIndexbuffer;
	
	//texture handles
	D3DXHANDLE m_hBoxTex;
	D3DXHANDLE m_hGroundTex;
	D3DXHANDLE m_hGrassTex;
	D3DXHANDLE m_hStoneTex;
	D3DXHANDLE m_hBlendTex;
	D3DXHANDLE m_hMeshCol;
	D3DXHANDLE m_hMeshTex;
	D3DXHANDLE m_hLightVecW; // sun handle
	
	//texture pointers
	IDirect3DTexture9 *m_pBoxTex;
	IDirect3DTexture9 *m_pPlaneTex;
	IDirect3DTexture9 *m_pGrassTex;
	IDirect3DTexture9 *m_pStoneTex;
	IDirect3DTexture9 *m_pBlendTex;
	
	//texture offset coordinates
	D3DXVECTOR2 textureOffsets;
	
	D3DXMATRIX m_matModelSpaceMatrix;
	D3DXMATRIX m_matTranslationMatrix;
	D3DXMATRIX m_matScalingMatrix;
	D3DXMATRIX m_matRotationMatrix;
	D3DXMATRIX m_matTransformMatrix;
};

