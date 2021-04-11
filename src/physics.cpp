#include "physics.h"

Physics::Physics()
:textureOffsets(0,0), m_pBoxTex(0)
{
	m_pPlaneTex = 0;
	m_pGrassTex = 0;
	m_pStoneTex = 0;
	m_pBlendTex = 0;
	
	m_nBoxNumVerts = 24;
	m_nTriangleCount = 12;
	LPCubeVertexArray = 0;

	// plane dy
	m_nPlaneHeightInterval = 5;
	// plane dx
	m_nPlaneWidthInterval = 5;

	m_pVertexbuffer = 0;
	m_pPlaneVertexbuffer = 0;
	m_pIndexbuffer = 0;
	m_pPlaneIndexbuffer = 0;

	m_nPlaneNumRows = 5;
	m_nPlaneNumColumns = 5;
	m_nPlaneNumVerts = m_nPlaneNumRows * m_nPlaneNumColumns;
	m_nCellRows = m_nPlaneNumRows - 1;
	m_nCellColumns = m_nPlaneNumColumns - 1;
	m_nFloorTriangleCount = m_nCellRows * m_nCellColumns * 2;

	LPPlaneVertexArray = new Vertex[m_nPlaneNumVerts];
	m_dwpPlaneIndices = new DWORD[m_nFloorTriangleCount * 3];
	
	D3DXMatrixIdentity(&m_matTransformMatrix);
	D3DXMatrixIdentity(&m_matTranslationMatrix);
	D3DXMatrixIdentity(&m_matRotationMatrix);
	D3DXMatrixIdentity(&m_matScalingMatrix);
}


Physics::~Physics()
{
	//release textures
	if (m_pBoxTex)
	{
		m_pBoxTex->Release();
		m_pBoxTex = 0;
	}
	if (m_pPlaneTex)
	{
		m_pPlaneTex->Release();
		m_pPlaneTex = 0;
	}
	if (m_pGrassTex)
	{
		m_pGrassTex->Release();
		m_pGrassTex = 0;
	}
	if (m_pStoneTex)
	{
		m_pStoneTex->Release();
		m_pStoneTex = 0;
	}
	if (m_pBlendTex)
	{
		m_pBlendTex->Release();
		m_pBlendTex = 0;
	}
	
	if (m_pPlaneVertexbuffer)
	{
		m_pPlaneVertexbuffer->Release();
		m_pPlaneVertexbuffer = 0;
	}
	if (m_pPlaneIndexbuffer)
	{
		m_pPlaneIndexbuffer->Release();
		m_pPlaneIndexbuffer = 0;
	}

	if (LPPlaneVertexArray)
	{
		free(LPPlaneVertexArray);
		LPPlaneVertexArray = 0;
	}
	if (m_dwpPlaneIndices)
	{
		free(m_dwpPlaneIndices);
		m_dwpPlaneIndices = 0;
	}

	//release vertex/index buffers and vertex declaration
	if (m_pVertexbuffer)
	{
		m_pVertexbuffer->Release();
		m_pVertexbuffer = 0;
	}
	if (m_pIndexbuffer)
	{
		m_pIndexbuffer->Release();
		m_pIndexbuffer = 0;
	}
}

VOID Physics::Initialize()
{
	CreateTerrain();
	CreateBoxes();

	// load textures from files
	HR(D3DXCreateTextureFromFile(g_pD3DGraphics->GetD3DDevice(), L"res/crate.jpg", &m_pBoxTex));
	HR(D3DXCreateTextureFromFile(g_pD3DGraphics->GetD3DDevice(), L"res/ground0.dds", &m_pPlaneTex));
	HR(D3DXCreateTextureFromFile(g_pD3DGraphics->GetD3DDevice(), L"res/grass0.dds", &m_pGrassTex));
	HR(D3DXCreateTextureFromFile(g_pD3DGraphics->GetD3DDevice(), L"res/stone2.dds", &m_pStoneTex));
	HR(D3DXCreateTextureFromFile(g_pD3DGraphics->GetD3DDevice(), L"res/blendmap.jpg", &m_pBlendTex));

	//get handle to texture variable
	m_hBoxTex = g_pD3DGraphics->GetFXInterface()->GetParameterByName(0, "gBoxTex");
	if (!m_hBoxTex)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gBoxTex"), 0, 0);
		exit(0);
	}

	//get handle to texture variable
	m_hGroundTex = g_pD3DGraphics->GetFXInterface()->GetParameterByName(0, "gGroundTex");
	if (!m_hGroundTex)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gGroundTex"), 0, 0);
		exit(0);
	}

	//get handle to texture variable
	m_hGrassTex = g_pD3DGraphics->GetFXInterface()->GetParameterByName(0, "gGrassTex");
	if (!m_hGrassTex)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gGrassTex"), 0, 0);
		exit(0);
	}

	//get handle to texture variable
	m_hStoneTex = g_pD3DGraphics->GetFXInterface()->GetParameterByName(0, "gStoneTex");
	if (!m_hStoneTex)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gStoneTex"), 0, 0);
		exit(0);
	}

	//get handle to texture variable
	m_hBlendTex = g_pD3DGraphics->GetFXInterface()->GetParameterByName(0, "gBlendTex");
	if (!m_hBlendTex)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gBlendTex"), 0, 0);
		exit(0);
	}

	m_hMeshCol = g_pD3DGraphics->GetFXInterface()->GetParameterByName(0, "gMeshColor");
	if (!m_hMeshCol)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gMeshColor"), 0, 0);
		exit(0);
	}

	m_hMeshTex = g_pD3DGraphics->GetFXInterface()->GetParameterByName(0, "gMeshTex");
	if (!m_hMeshTex)
	{
		MessageBox(0, _TEXT("Invalid Parameter name gMeshTex"), 0, 0);
		exit(0);
	}
}
VOID Physics::Update(DOUBLE deltaTime)
{

}

VOID Physics::Draw(D3DXMATRIX m_matView, D3DXMATRIX m_matProjection)
{
	// old FX value holders
	BOOL isFloorOld = FALSE;
	BOOL isMeshOld = FALSE;
	BOOL isBoundingBoxOld = FALSE;

	// new FX file booleans
	BOOL isFloor = FALSE;
	BOOL isMesh = FALSE;
	BOOL isBoundingBox = FALSE;
	FLOAT planeWidth;
	FLOAT planeHeight;
	D3DXMATRIX tempMatrix;
	unsigned int numpasses = 0;

	// save the old FX values
	HR(g_pD3DGraphics->GetFXInterface()->GetValue(g_pD3DGraphics->GetChooserHandle(), &isFloorOld, sizeof(BOOL)));
	HR(g_pD3DGraphics->GetFXInterface()->GetValue(g_pD3DGraphics->GetIsMeshHandle(), &isMeshOld, sizeof(BOOL)));
	HR(g_pD3DGraphics->GetFXInterface()->GetValue(g_pD3DGraphics->GetIsBoundingBoxHandle(), &isBoundingBoxOld, sizeof(BOOL)));
	
	/*textureOffsets += D3DXVECTOR2(.25f, .1f);

	if (textureOffsets.x > 1.0f)
	{
		textureOffsets.x = 0;
	}

	if (textureOffsets.y > 1.0f)
	{
		textureOffsets.y = 0;
	}*/
	
	//set the FVF (flexible vertex format) code
	g_pD3DGraphics->GetD3DDevice()->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);
	
	//set technique / parameters
	HR(g_pD3DGraphics->GetFXInterface()->SetTechnique(g_pD3DGraphics->GetTechniqueHandle()));

	//set textures
	HR(g_pD3DGraphics->GetFXInterface()->SetTexture(m_hBoxTex, m_pBoxTex));
	HR(g_pD3DGraphics->GetFXInterface()->SetTexture(m_hGroundTex, m_pPlaneTex));
	HR(g_pD3DGraphics->GetFXInterface()->SetTexture(m_hGrassTex, m_pGrassTex));
	HR(g_pD3DGraphics->GetFXInterface()->SetTexture(m_hStoneTex, m_pStoneTex));
	HR(g_pD3DGraphics->GetFXInterface()->SetTexture(m_hBlendTex, m_pBlendTex));

	//set texture animation value
	//HR(g_pD3DGraphics->GetFXInterface()->SetValue(g_pD3DGraphics->GetOffsetHandle(), &textureOffsets, sizeof(D3DXVECTOR2)));

	/*
	start programmable shader pipeline
	because of row vector use, right multiply matrices
	*/
	HR(g_pD3DGraphics->GetFXInterface()->Begin(&numpasses, 0));
	for (int i = 0; i < (int)numpasses; i++)
	{
		// begin pass
		HR(g_pD3DGraphics->GetFXInterface()->BeginPass(i));

		// set vertex/index streams
		HR(g_pD3DGraphics->GetD3DDevice()->SetStreamSource(0, m_pVertexbuffer, 0, sizeof(Vertex)));
		HR(g_pD3DGraphics->GetD3DDevice()->SetIndices(m_pIndexbuffer));

		// set FX state booleans
		isFloor = false;
		HR(g_pD3DGraphics->GetFXInterface()->SetValue(g_pD3DGraphics->GetChooserHandle(), &isFloor, sizeof(BOOL)));
		HR(g_pD3DGraphics->GetFXInterface()->SetValue(g_pD3DGraphics->GetIsMeshHandle(), &isMesh, sizeof(BOOL)));
		HR(g_pD3DGraphics->GetFXInterface()->SetValue(g_pD3DGraphics->GetIsBoundingBoxHandle(), &isBoundingBox, sizeof(BOOL)));

		/*
		DRAW BOX 1
		battery box is a cube centered at the origin
		with a coordinate range of [1,-1]
		so we translate it up to sit on the ground tile
		*/
		//position the the model in local space
		D3DXMatrixTranslation(&m_matTranslationMatrix, 0, 1, 0);
		m_matModelSpaceMatrix = m_matTranslationMatrix;
		// position the model in world space
		D3DXMatrixTranslation(&m_matTranslationMatrix, 0, 0, 0);
		tempMatrix = g_pD3DGraphics->GetWorldMatrix() * m_matTranslationMatrix;
		// concatenate model and world matrices
		tempMatrix = m_matModelSpaceMatrix * tempMatrix;
		// set world/view/projection matrix in fx file
		HR(g_pD3DGraphics->GetFXInterface()->SetMatrix(
			g_pD3DGraphics->GetWorldViewProjectionHandle(),
			&(tempMatrix * m_matView * m_matProjection))
		);
		D3DXMATRIX worldInverseTranspose;
		D3DXMatrixInverse(&worldInverseTranspose, 0, &(tempMatrix * m_matView * m_matProjection));
		D3DXMatrixTranspose(&worldInverseTranspose, &worldInverseTranspose);
		HR(g_pD3DGraphics->GetFXInterface()->SetMatrix(
			g_pD3DGraphics->GetWorldInverseTransposeHandler(),
			&worldInverseTranspose)
		);
		HR(g_pD3DGraphics->GetFXInterface()->CommitChanges());
		HR(g_pD3DGraphics->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_nBoxNumVerts, 0, m_nTriangleCount));

		//DRAW BOX 2
		//position the the model in local space
		D3DXMatrixTranslation(&m_matTranslationMatrix, 0, 1, 0);
		m_matModelSpaceMatrix = m_matTranslationMatrix;
		// position the model in world space
		D3DXMatrixTranslation(&m_matTranslationMatrix, 0, 0, 5);
		tempMatrix = g_pD3DGraphics->GetWorldMatrix() * m_matTranslationMatrix;
		// concatenate model and world matrices
		tempMatrix = m_matModelSpaceMatrix * tempMatrix;
		// set world/view/projection matrix in fx file
		HR(g_pD3DGraphics->GetFXInterface()->SetMatrix(
			g_pD3DGraphics->GetWorldViewProjectionHandle(),
			&(tempMatrix * m_matView * m_matProjection))
		);
		D3DXMatrixInverse(&worldInverseTranspose, 0, &(tempMatrix * m_matView * m_matProjection));
		D3DXMatrixTranspose(&worldInverseTranspose, &worldInverseTranspose);
		HR(g_pD3DGraphics->GetFXInterface()->SetMatrix(
			g_pD3DGraphics->GetWorldInverseTransposeHandler(),
			&worldInverseTranspose)
		);
		HR(g_pD3DGraphics->GetFXInterface()->CommitChanges());
		HR(g_pD3DGraphics->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_nBoxNumVerts, 0, m_nTriangleCount));

		//DRAW BOX 3
		//position the the model in local space
		D3DXMatrixTranslation(&m_matTranslationMatrix, 0, 1, 0);
		m_matModelSpaceMatrix = m_matTranslationMatrix;
		// position the model in world space
		D3DXMatrixTranslation(&m_matTranslationMatrix, -10, 0, 5);
		tempMatrix = g_pD3DGraphics->GetWorldMatrix() * m_matTranslationMatrix;
		// concatenate model and world matrices
		tempMatrix = m_matModelSpaceMatrix * tempMatrix;
		HR(g_pD3DGraphics->GetFXInterface()->SetMatrix(
			g_pD3DGraphics->GetWorldViewProjectionHandle(),
			&(tempMatrix * m_matView * m_matProjection))
		);
		D3DXMatrixInverse(&worldInverseTranspose, 0, &(tempMatrix * m_matView * m_matProjection));
		D3DXMatrixTranspose(&worldInverseTranspose, &worldInverseTranspose);
		HR(g_pD3DGraphics->GetFXInterface()->SetMatrix(
			g_pD3DGraphics->GetWorldInverseTransposeHandler(),
			&worldInverseTranspose)
		);
		HR(g_pD3DGraphics->GetFXInterface()->CommitChanges());
		HR(g_pD3DGraphics->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_nBoxNumVerts, 0, m_nTriangleCount));

		//DRAW FLOOR
		//set vertex/index streams, still using the same vertex declaration
		HR(g_pD3DGraphics->GetD3DDevice()->SetStreamSource(0, m_pPlaneVertexbuffer, 0, sizeof(Vertex)));
		HR(g_pD3DGraphics->GetD3DDevice()->SetIndices(m_pPlaneIndexbuffer));
		
		// set FX floor texture variable
		isFloor = true;
		HR(g_pD3DGraphics->GetFXInterface()->SetValue(g_pD3DGraphics->GetChooserHandle(), &isFloor, sizeof(BOOL)));

		// position the the model in local space
		D3DXMatrixIdentity(&m_matModelSpaceMatrix);
		D3DXMatrixScaling(&m_matScalingMatrix, 20, 1, 20);
		m_matModelSpaceMatrix = m_matScalingMatrix;
		// position the model in world space
		planeWidth = m_nPlaneWidthInterval * m_nCellRows * m_matScalingMatrix._11;
		planeHeight = m_nPlaneHeightInterval * m_nCellColumns * m_matScalingMatrix._33;
		D3DXMatrixTranslation(&m_matTranslationMatrix, -1 * (planeWidth / 2.0f), 0, (planeHeight / 2.0f));
		tempMatrix = g_pD3DGraphics->GetWorldMatrix() * m_matTranslationMatrix;
		// concatenate model and world matrices
		tempMatrix = m_matModelSpaceMatrix * tempMatrix;
		HR(g_pD3DGraphics->GetFXInterface()->SetMatrix(
			g_pD3DGraphics->GetWorldViewProjectionHandle(),
			&(tempMatrix * m_matView * m_matProjection))
		);
		D3DXMatrixInverse(&worldInverseTranspose, 0, &(tempMatrix * m_matView * m_matProjection));
		D3DXMatrixTranspose(&worldInverseTranspose, &worldInverseTranspose);
		HR(g_pD3DGraphics->GetFXInterface()->SetMatrix(
			g_pD3DGraphics->GetWorldInverseTransposeHandler(),
			&worldInverseTranspose)
		);
		HR(g_pD3DGraphics->GetFXInterface()->CommitChanges());
		HR(g_pD3DGraphics->GetD3DDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_nPlaneNumVerts, 0, m_nFloorTriangleCount));

		//done with this pass
		HR(g_pD3DGraphics->GetFXInterface()->EndPass());
	}

	// restore the old FX values
	HR(g_pD3DGraphics->GetFXInterface()->SetValue(g_pD3DGraphics->GetChooserHandle(), &isFloorOld, sizeof(BOOL)));
	HR(g_pD3DGraphics->GetFXInterface()->SetValue(g_pD3DGraphics->GetIsMeshHandle(), &isMeshOld, sizeof(BOOL)));
	HR(g_pD3DGraphics->GetFXInterface()->SetValue(g_pD3DGraphics->GetIsBoundingBoxHandle(), &isBoundingBoxOld, sizeof(BOOL)));

	//make sure begin actually got called
	if (numpasses != 0)
	{
		HR(g_pD3DGraphics->GetFXInterface()->End());
	}
}

/*
describe the floor geometry in local space
*/
VOID Physics::CreateTerrain()
{
	int k = 0;

	for (DWORD i = 0; i < (DWORD)m_nPlaneNumRows; i++)
	for (DWORD j = 0; j < (DWORD)m_nPlaneNumColumns; j++)
	{
		LPPlaneVertexArray[k].position.x = 1.0f * (j * m_nPlaneWidthInterval);
		LPPlaneVertexArray[k].position.y = 0;
		LPPlaneVertexArray[k].position.z = -1.0f * (i * m_nPlaneHeightInterval);
		LPPlaneVertexArray[k].normal = D3DXVECTOR3(0,1,0);
		LPPlaneVertexArray[k].texture = D3DXVECTOR2((float)j, (float)i) / 4.0f;

		k++;
	}

	k = 0;
	for (DWORD i = 0; i < (DWORD)m_nCellRows; i++)
	{
		for (DWORD j = 0; j < (DWORD)m_nCellColumns; j++)
		{
			m_dwpPlaneIndices[k] = i * m_nPlaneNumColumns + j;
			m_dwpPlaneIndices[k + 1] = i * m_nPlaneNumColumns + j + 1;
			m_dwpPlaneIndices[k + 2] = (i + 1) * m_nPlaneNumColumns + j;

			m_dwpPlaneIndices[k + 3] = (i + 1) * m_nPlaneNumColumns + j;
			m_dwpPlaneIndices[k + 4] = i * m_nPlaneNumColumns + j + 1;
			m_dwpPlaneIndices[k + 5] = (i + 1) * m_nPlaneNumColumns + j + 1;

			//next quad
			k += 6;
		}
	}

	HR(g_pD3DGraphics->GetD3DDevice()->CreateVertexBuffer
		(m_nPlaneNumVerts * sizeof(Vertex), D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &m_pPlaneVertexbuffer, 0));

	Vertex *x = 0;
	HR(m_pPlaneVertexbuffer->Lock(0, 0, (void**)&x, 0));
	for (DWORD i = 0; i < (DWORD)m_nPlaneNumVerts; i++)
	{
		x[i] = LPPlaneVertexArray[i];
	}
	HR(m_pPlaneVertexbuffer->Unlock());

	HR(g_pD3DGraphics->GetD3DDevice()->CreateIndexBuffer
		(m_nFloorTriangleCount * 3 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16,
		D3DPOOL_MANAGED, &m_pPlaneIndexbuffer, 0));

	WORD *r = 0;
	HR(m_pPlaneIndexbuffer->Lock(0, 0, (void**)&r, 0));
	for (DWORD i = 0; i < (DWORD)m_nFloorTriangleCount * 3; i++)
	{
		r[i] = (WORD)m_dwpPlaneIndices[i];
	}
	HR(m_pPlaneIndexbuffer->Unlock());
}

/*
create battery boxes
create the vertex buffer and the index buffer
describe the object in local space
*/
VOID Physics::CreateBoxes()
{
	//create vertex buffer
	HR(g_pD3DGraphics->GetD3DDevice()->CreateVertexBuffer
		(24 * sizeof(Vertex), 0, 0, D3DPOOL_MANAGED, &m_pVertexbuffer, 0));

	//lock buffer so you can write to it
	HR(m_pVertexbuffer->Lock(0, 0, (void**)&LPCubeVertexArray, 0));

	/*
	look down face's normal vector and list vertices
	in clockwise order
	*/

	//front face
	LPCubeVertexArray[0] = Vertex(-1.0f, -1.0f, -1.0f, 0, 0, -1.0f, 0, 1.0f);
	LPCubeVertexArray[1] = Vertex(-1.0f, 1.0f, -1.0f, 0, 0, -1.0f, 0, 0);
	LPCubeVertexArray[2] = Vertex(1.0f, 1.0f, -1.0f, 0, 0, -1.0f, 1.0f, 0);
	LPCubeVertexArray[3] = Vertex(1.0f, -1.0f, -1.0f, 0, 0, -1.0f, 1.0f, 1.0f);

	//back face
	LPCubeVertexArray[4] = Vertex(-1.0f, -1.0f, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f);
	LPCubeVertexArray[5] = Vertex(-1.0f, 1.0f, 1.0f, 0, 0, 1.0f, 1.0f, 0);
	LPCubeVertexArray[6] = Vertex(1.0f, 1.0f, 1.0f, 0, 0, 1.0f, 0, 0);
	LPCubeVertexArray[7] = Vertex(1.0f, -1.0f, 1.0f, 0, 0, 1.0f, 0, 1.0f);

	//left face
	LPCubeVertexArray[8] = Vertex(-1.0f, -1.0f, 1.0f, -1.0f, 0, 0, 0, 1.0f);
	LPCubeVertexArray[9] = Vertex(-1.0f, 1.0f, 1.0f, -1.0f, 0, 0, 0, 0);
	LPCubeVertexArray[10] = Vertex(-1.0f, 1.0f, -1.0f, -1.0f, 0, 0, 1.0f, 0);
	LPCubeVertexArray[11] = Vertex(-1.0f, -1.0f, -1.0f, -1.0f, 0, 0, 1.0f, 1.0f);

	//right face
	LPCubeVertexArray[12] = Vertex(1.0f, -1.0f, -1.0f, 1.0f, 0, 0, 0, 1.0f);
	LPCubeVertexArray[13] = Vertex(1.0f, 1.0f, -1.0f, 1.0f, 0, 0, 0, 0);
	LPCubeVertexArray[14] = Vertex(1.0f, 1.0f, 1.0f, 1.0f, 0, 0, 1.0f, 0);
	LPCubeVertexArray[15] = Vertex(1.0f, -1.0f, 1.0f, 1.0f, 0, 0, 1.0f, 1.0f);

	//top face
	LPCubeVertexArray[16] = Vertex(-1.0f, 1.0f, 1.0f, 0, 1.0f, 0, 0, 0);
	LPCubeVertexArray[17] = Vertex(1.0f, 1.0f, 1.0f, 0, 1.0f, 0, 1.0f, 0);
	LPCubeVertexArray[18] = Vertex(1.0f, 1.0f, -1.0f, 0, 1.0f, 0, 1.0f, 1.0f);
	LPCubeVertexArray[19] = Vertex(-1.0f, 1.0f, -1.0f, 0, 1.0f, 0, 0, 1.0f);

	//bottom face
	LPCubeVertexArray[20] = Vertex(-1.0f, -1.0f, -1.0f, 0, -1.0f, 0, 0, 0);
	LPCubeVertexArray[21] = Vertex(1.0f, -1.0f, -1.0f, 0, -1.0f, 0, 1.0f, 0);
	LPCubeVertexArray[22] = Vertex(1.0f, -1.0f, 1.0f, 0, -1.0f, 0, 1.0f, 1.0f);
	LPCubeVertexArray[23] = Vertex(-1.0f, -1.0f, 1.0f, 0, -1.0f, 0, 0, 1.0f);

	//unlock
	HR(m_pVertexbuffer->Unlock());

	//create index buffer
	//number of faces * 2 triangles per face
	//number of triangles * 3 vertices per triangle
	HR(g_pD3DGraphics->GetD3DDevice()->CreateIndexBuffer
		(36 * sizeof(WORD), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &m_pIndexbuffer, 0));
	WORD *k = 0;
	HR(m_pIndexbuffer->Lock(0, 0, (void**)&k, 0));

	// Front face.
	k[0] = 0; k[1] = 1; k[2] = 2;
	k[3] = 0; k[4] = 2; k[5] = 3;

	// Back face.
	k[6] = 4; k[7] = 6; k[8] = 5;
	k[9] = 4; k[10] = 7; k[11] = 6;

	// Left face.
	k[12] = 8; k[13] = 9; k[14] = 10;
	k[15] = 8; k[16] = 10; k[17] = 11;

	// Right face.
	k[18] = 12; k[19] = 13; k[20] = 14;
	k[21] = 12; k[22] = 14; k[23] = 15;

	// Top face.
	k[24] = 19; k[25] = 16; k[26] = 17;
	k[27] = 19; k[28] = 17; k[29] = 18;

	// Bottom face.
	k[30] = 23; k[31] = 20; k[32] = 21;
	k[33] = 23; k[34] = 21; k[35] = 22;

	//unlock
	HR(m_pIndexbuffer->Unlock());
}

VOID Physics::OnLostDevice()
{
	//destroy vertex and index buffers if they are in dynamic memory locations
	if (m_pVertexbuffer)
	{
		m_pVertexbuffer->Release();
	}
	if (m_pIndexbuffer)
	{
		m_pIndexbuffer->Release();
	}
}

VOID Physics::OnResetDevice()
{
	//create the vertex and index buffers if they are in dynamic memory locations
	CreateTerrain();
	CreateBoxes();
}