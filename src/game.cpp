#include "game.h"

Game::Game(HWND *g_hWindow, RECT *g_rFormatRect)
:m_pHWindow(g_hWindow),m_pRFormatRect(g_rFormatRect),m_physicsWorldObject()
{
	m_nCurrentCamera = 0;
	m_nCurrentTime = 0;
	m_pCurrentCharacter = 0;
	m_nTimeUntilCharacterSelectable = 0;
	m_nCurrentTextDelay = 1000;
}

Game::~Game(void)
{
	/*
	free dynamically allocated memory
	*/
	for (std::vector<Entity*>::iterator iter = m_pGameObjectVector.begin(); iter != m_pGameObjectVector.end(); iter++)
	{
		delete *iter;
	}
}

VOID Game::GameInitialize()
{
	Entity* entityObject;
	default_random_engine generator;
	uniform_real_distribution<float> distribution(-150, 150);

	m_physicsWorldObject.Initialize();

	// create a set of entities at random locations
	for (size_t i = 0; i < 10; i++){
		entityObject = new Bear();
		entityObject->SetPosition(distribution(generator), 0, distribution(generator));

		m_pGameObjectVector.push_back(entityObject);
	}
	
	entityObject = new Zombie();
	entityObject->SetPosition(0, 0, 10);
	m_pGameObjectVector.push_back(entityObject);
	
	m_pCurrentCharacter = m_pGameObjectVector.back();
	m_nCurrentCamera = m_pGameObjectVector.size() - 1;
}

/*
each call to game run represents a frame of the game
deltatime in milliseconds
(1)f/(x)ms
In 1 second, 1s = 1000ms
we have: 1000ms / 1s * 1f / (x)ms = (y)f / 1s
*/
VOID Game::GameRun(DOUBLE deltatime)
{
	FLOAT c = static_cast<FLOAT>(g_pD3DGraphics->GetPresentationParams().BackBufferWidth / 2);
	FLOAT d = static_cast<FLOAT>(g_pD3DGraphics->GetPresentationParams().BackBufferHeight / 2);
	FLOAT nearPlane = m_pGameObjectVector.at(m_nCurrentCamera)->GetVisualSystem()->GetNearPlane();
	FLOAT farPlane = m_pGameObjectVector.at(m_nCurrentCamera)->GetVisualSystem()->GetFarPlane();
	D3DXVECTOR3 tempVec;
	D3DXVECTOR3 currentPosition;
	D3DXVECTOR3 transFormedVec;
	D3DXMATRIX tempMatrix;

	m_nCurrentTime += deltatime;
	m_nCurrentTextDelay += deltatime;

	// convert back to seconds to display the FPS
	// update fps text every second
	if (m_nCurrentTextDelay >= 1000)
	{
		m_nCurrentTextDelay = 0;

		m_nFPS = 1000 * (1 / deltatime);
	}

	result = g_pD3DGraphics->GetDevice()->TestCooperativeLevel();
	if (FAILED(result))
	{
		if (result == D3DERR_DEVICELOST)
		{
			Sleep(20);
			return;
		}
		else if (result == D3DERR_DRIVERINTERNALERROR)
		{
			PostQuitMessage(0);
		}
		else
		{
			if (result == D3DERR_DEVICENOTRESET)
			{
				////reset render states
				m_physicsWorldObject.OnLostDevice();
				g_pD3DGraphics->OnLostDevice();

				g_pD3DGraphics->GetDevice()->Reset(&g_pD3DGraphics->GetPresentationParams());
				
				m_physicsWorldObject.OnResetDevice();
				g_pD3DGraphics->OnResetDevice();
			}
			else
			{
				Sleep(20);
				return;
			}
		}
	}

	if (g_pD3DGraphics->GetDevice())
	{
		// erase previous frame by clearing backbuffer
		g_pD3DGraphics->GetDevice()->Clear(0, 0, D3DCLEAR_TARGET
			| D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

		// poll the keyboard
		g_pD3DGraphics->Poll();

		// once this button is pressed do not process it
		// again for 500ms
		// need: time it was pressed
		// the amount of time that has passed since the time it was pressed
		// check if the user has switched cameras, add a change character delay of 500ms
		if ((g_pD3DGraphics->GetKeyboardState()[DIK_TAB] & 0x80) && (m_nCurrentTime > m_nTimeUntilCharacterSelectable))
		{
			m_nCurrentCamera++;

			if (m_nCurrentCamera > (m_pGameObjectVector.size() - 1))
			{
				m_nCurrentCamera = 0;
			}

			m_nTimeUntilCharacterSelectable = m_nCurrentTime + 500;
		}
		
		// iterate through all entities and call their update function
		for (UINT i = 0; i < m_pGameObjectVector.size(); i++)
		{
			// if the camera is the current entity call update with true
			if (m_nCurrentCamera == i)
			{
				m_pCurrentCharacter = m_pGameObjectVector.at(i);

				m_pGameObjectVector.at(i)->Update(deltatime, TRUE);
			}

			m_pGameObjectVector.at(i)->Update(deltatime);
		}
		
		// update the world
		m_physicsWorldObject.Update(deltatime);

		// begin DirectX drawing
		g_pD3DGraphics->GetDevice()->BeginScene();

		// draw the world
		m_physicsWorldObject.Draw(m_pCurrentCharacter->GetVisualSystem()->GetViewMatrix(), m_pCurrentCharacter->GetVisualSystem()->GetProjectionMatrix());

		// draw the entity being followed by the camera
		m_pCurrentCharacter->Draw();
		currentPosition = m_pCurrentCharacter->GetPosition();

		// draw all the other entities
		for (vector<Entity*>::iterator iter = m_pGameObjectVector.begin(); iter != m_pGameObjectVector.end(); iter++)
		{
			//pass in the current camera to the other entities
			if (*iter != m_pCurrentCharacter)
			{
				// if the entity is completely outside of the frustrum then we can skip sending it to the shaders
				tempVec = (*iter)->GetPosition();
				tempMatrix = g_pD3DGraphics->GetWorldMatrix() * m_pGameObjectVector.at(m_nCurrentCamera)->GetVisualSystem()->GetViewMatrix();
				
				D3DXVec3TransformCoord(&transFormedVec, &tempVec, &tempMatrix);

				// clip in homogenous clip space
				if (transFormedVec.x < c && transFormedVec.x > -c
					&& transFormedVec.y < d && transFormedVec.y > -d
					&& transFormedVec.z >= nearPlane && transFormedVec.z <= farPlane) {
					(*iter)->Draw(m_pCurrentCharacter->GetVisualSystem());
				}
			}
		}

		//get current window size
		GetClientRect(*m_pHWindow, m_pRFormatRect);

		//draw stats text
		g_pD3DGraphics->GetFont()->DrawText(0, GetStats(), -1,
			m_pRFormatRect, 0, D3DCOLOR_XRGB(255, 255, 255));

		//end DirectX drawing
		g_pD3DGraphics->GetDevice()->EndScene();

		//present the new backbuffer
		g_pD3DGraphics->GetDevice()->Present(0, 0, 0, 0);
	}
}

VOID Game::GameEnd()
{
}

/*
return statistics as text to caller
*/
TCHAR* Game::GetStats()
{
	swprintf_s(m_pStatisticsCharText, 256, L"Press up,down,left,right, arrow keys\n to move the character, pageup,\n \
		pagedown to tilt up an down\n move the mouse wheel to zoom in and out\n tab switches characters\n FPS: %g", floor(m_nFPS));

	return m_pStatisticsCharText;
}