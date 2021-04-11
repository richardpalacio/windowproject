#pragma once

#include <windows.h>
#include <vector>
#include <random>
#include "entity.h"
#include "bear.h"
#include "blackbear.h"
#include "zombie.h"
#include "physics.h"

using namespace std;

class Game
{
public:
	Game(HWND*, RECT*);
	virtual ~Game();
	
	VOID GameInitialize();
	VOID GameRun(DOUBLE);
	VOID GameEnd();
private:
	// print the statistics to the screen
	TCHAR* GetStats();
protected:
	//the currently selected camera
	UINT m_nCurrentCamera;
	//frames per milliseconds
	DOUBLE m_nFPS;
	DOUBLE m_nCurrentTime;
	DOUBLE m_nCurrentTextDelay;
	DOUBLE m_nTimeUntilCharacterSelectable;
	//statistics
	TCHAR m_pStatisticsCharText[256];

	HRESULT result;
	RECT *m_pRFormatRect;
	HWND *m_pHWindow;
	Physics m_physicsWorldObject;
	std::vector<Entity*> m_pGameObjectVector;
	Entity* m_pCurrentCharacter;
};
