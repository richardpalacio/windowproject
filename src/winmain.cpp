//window creation
//d3d initialization
#pragma once

//Include Files
#include <windows.h>
#include <time.h>
#include <crtdbg.h>
#include <gdiplus.h>

#include "resource.h"
#include "graphicdevice.h"
#include "game.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace Gdiplus;

//macro defines
#define APPNAME TEXT("Direct3D_Windowed")
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define LOADING_IMAGE_WIDTH 1600
#define LOADING_IMAGE_HEIGHT 1200
#define WINDOWED true

//global variables
HWND g_hWindow;
HDC g_hDC;
HINSTANCE g_hInstance;
RECT g_rformatRect;
bool windowed = WINDOWED;
PAINTSTRUCT ps;
D3DGraphics *g_pD3DGraphics = new D3DGraphics();
sql::SQLString g_sqlStringDatabaseLocation = "tcp://52.205.189.103:3306";

//prototypes
BOOL CreateWindowInstance(HINSTANCE, int);

/*
message handler
LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam);
ignores or acts on the messages windows recieves
*/
LRESULT CALLBACK WndProc(HWND hWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int width;
	int height;
	HRESULT result;
	Graphics newGraphics(g_hDC);
	Pen newPen(Color(255, 255, 0, 0), 1);
	HDC bitmapHandle, canvasHandle;
	HBITMAP bitmapInstance;

	g_hWindow = hWindow;
	g_hDC = GetDC(hWindow);
	
	switch (msg)
	{
		case WM_PAINT:
			canvasHandle = BeginPaint(hWindow, &ps);

			// Load the bitmap from the resource
			bitmapInstance = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_BITMAP1));
			// Create a memory device compatible with the above DC variable
			bitmapHandle = CreateCompatibleDC(canvasHandle);
			// Select the new bitmap
			SelectObject(bitmapHandle, bitmapInstance);

			// Copy the bits from the memory DC into the current dc
			StretchBlt(canvasHandle, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, bitmapHandle, 0, 0, LOADING_IMAGE_WIDTH, LOADING_IMAGE_HEIGHT, SRCCOPY);

			// Restore the old bitmap
			DeleteDC(bitmapHandle);
			DeleteObject(bitmapInstance);
			EndPaint(hWindow, &ps);
			break;
		case WM_CLOSE:
			if (MessageBox(hWindow, _TEXT("do you want to close"), _TEXT("Game Window"), MB_YESNOCANCEL) == IDYES)
			{
				//calls d3d release method
				DestroyWindow(hWindow);
			}
			else
			{
				return 0;
			}
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_LBUTTONDOWN:
			//PostQuitMessage(0);
			return 0;
		case WM_RBUTTONDOWN:
			/*
			to make a window a fullscreen window just style: ws_ex_topmost
			to make a window, windowed, style: ws_overlapped

			change to fullscreen
			GetSystemMetrics
			change the presentation parameters: windowed...
			set SetWindowLongPtr, SetWindowPos
			hide the window and preform a device reset

			return from fullscreen
			rect struct
			AdjustWindowRect
			change presentation parameters: windowed...
			set SetWindowLongPtr, SetWindowPos
			preform a device reset
			*/
			//windowed; go fullscreen
			if (g_pD3DGraphics->GetDevice())
			{
				if (windowed)
				{
					width = GetSystemMetrics(SM_CXSCREEN);
					height = GetSystemMetrics(SM_CYSCREEN);

					g_pD3DGraphics->SetBackBufferSize(FALSE, width, height);

					SetWindowLongPtr( hWindow, GWL_STYLE, WS_POPUP);
					SetWindowPos(hWindow, HWND_TOP, 0, 0, width, height,
						SWP_NOZORDER | SWP_SHOWWINDOW);
					UpdateWindow(hWindow);
				}
				else
				{
					width = SCREEN_WIDTH;
					height = SCREEN_HEIGHT;

					RECT rect = {0, 0, width, height};
					AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);

					g_pD3DGraphics->SetBackBufferSize(TRUE, width, height);

					SetWindowLongPtr(hWindow, GWL_STYLE, WS_OVERLAPPED | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX
						| WS_SYSMENU | WS_CAPTION);
					SetWindowPos(hWindow, HWND_TOPMOST, 300, 100, rect.right, rect.bottom,
						SWP_SHOWWINDOW);
					UpdateWindow(hWindow);
				}

				// update current window status
				windowed = !windowed;

				g_pD3DGraphics->OnLostDevice();
				g_pD3DGraphics->GetDevice()->Reset(&g_pD3DGraphics->GetPresentationParams());
				g_pD3DGraphics->OnResetDevice();
			}
			return 0;
		//handle maximize/minimize of window
		case WM_SIZE:
			if (g_pD3DGraphics->GetDevice())
			{
				if (!windowed)
				{
					return 0;
				}
				else
				{
					GetClientRect(hWindow, &g_rformatRect);

					width = LOWORD(lParam);
					height = HIWORD(lParam);

					g_pD3DGraphics->SetBackBufferSize(windowed, width, height);

					result = g_pD3DGraphics->GetDevice()->TestCooperativeLevel();
					if (FAILED(result))
					{
						if (result == D3DERR_DEVICELOST)
						{
							Sleep(20);
							return 0;
						}
						else if (result == D3DERR_DRIVERINTERNALERROR)
						{
							PostQuitMessage(0);
							return 0;
						}
						else
						{
							if (result == D3DERR_DEVICENOTRESET)
							{
								g_pD3DGraphics->OnLostDevice();
								g_pD3DGraphics->GetDevice()->Reset(
									&g_pD3DGraphics->GetPresentationParams());
								g_pD3DGraphics->OnResetDevice();
							}
							else
							{
								Sleep(20);
								return 0;
							}
						}
					}

					if (!IsWindowVisible(hWindow))
					{
						g_pD3DGraphics->OnLostDevice();
						g_pD3DGraphics->GetDevice()->Reset(&g_pD3DGraphics->GetPresentationParams());
						g_pD3DGraphics->OnResetDevice();
					}
				}
			}
			return 0;
		//handle mouse resizing
		case WM_EXITSIZEMOVE:
			RECT rect;
			GetClientRect(hWindow, &rect);
			
			width = rect.right;
			height = rect.bottom;

			g_pD3DGraphics->SetBackBufferSize(windowed, width, height);
			
			if (g_pD3DGraphics->GetDevice())
			{
				g_pD3DGraphics->OnLostDevice();
				g_pD3DGraphics->GetDevice()->Reset(&g_pD3DGraphics->GetPresentationParams());
				g_pD3DGraphics->OnResetDevice();
			}
			return 0;
	}

	//all other messages, send to default window procedure
	return DefWindowProc(hWindow, msg, wParam, lParam);
}

/*
create and register a window class
*/
ATOM RegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wndclass;

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON2));
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = APPNAME;

	//register Window
	return RegisterClassEx(&wndclass);
}

/*
set the paramaters of the window
*/
BOOL CreateWindowInstance(HINSTANCE hInstance, int iCmdShow)
{
	HWND hWindow;
	
	hWindow = CreateWindow(
		APPNAME, //window class name
		APPNAME, //application name

		//window style overlappedwindow for	windowed
		WINDOWED ? (WS_OVERLAPPED | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX
		| WS_SYSMENU | WS_CAPTION) : (WS_EX_TOPMOST | WS_VISIBLE | WS_POPUP),

		CW_USEDEFAULT, //position of x
		CW_USEDEFAULT, //position of y
		SCREEN_WIDTH, //screen width
		SCREEN_HEIGHT, //screen height
		NULL, //parent window
		NULL, //menu
		hInstance, //application instance
		NULL //window parameters
	);

	if (!hWindow)
	{
		return false;
	}
	
	ShowWindow(hWindow, iCmdShow);
	UpdateWindow(hWindow);
	return true;
}

/*
main function
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	
	//declare variables
	BOOL runGame = TRUE;
	MSG msg;
	INT64 currentPerformanceCount;
	INT64 previousPerformanceCount;
	INT64 performanceCounterFrequency;
	//delta time in milliseconds
	DOUBLE deltaTime;
	Game g_pGameInstance(&g_hWindow, &g_rformatRect);

	// set the reference to the hInstance variable
	g_hInstance = hInstance;

	boost::property_tree::ptree tree;
	boost::property_tree::read_xml("./res/databaselocation.xml", tree);
	g_sqlStringDatabaseLocation = tree.get<std::string>("database.ip");

	//seed the timer
	srand(time_t(0));

	//create GDIplus objects and start
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	//create & register window class
	RegisterClass(hInstance);
	
	//create instance of window class
	if(!CreateWindowInstance(hInstance, iCmdShow))
	{
		return 0;
	}
	
	//initialize game/d3d
	if (!g_pD3DGraphics->Init_Direct3D(g_hWindow, SCREEN_WIDTH, SCREEN_HEIGHT, windowed, hInstance))
	{
		if (MessageBox(g_hWindow, _TEXT("Error Initializing DirectX"), _TEXT("Game Window"), MB_OK) == IDOK)
		{
			//calls d3d release method
			DestroyWindow(g_hWindow);

			return 0;
		}
	}

	//create d3d font object
	g_pD3DGraphics->MakeFont();
	//set vertex declaration
	g_pD3DGraphics->CreateVertexDeclaration();

	//initialize directInput
	g_pD3DGraphics->CreateDirectInput();
	// initialize game
	g_pGameInstance.GameInitialize();
	
	//number of counts
	QueryPerformanceCounter((LARGE_INTEGER*)&previousPerformanceCount);
	
	//counts per second
	QueryPerformanceFrequency((LARGE_INTEGER*)&performanceCounterFrequency);
	
	//message/game loop
	while (runGame)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			//look for quit message
			if(msg.message == WM_QUIT)
			{
				runGame = FALSE;
			}
			
			//decode and send messages to wndproc handler function
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//number of counts
			QueryPerformanceCounter((LARGE_INTEGER*)&currentPerformanceCount);
			
			//delta seconds = counts * (second / count)
			deltaTime = (DOUBLE)(currentPerformanceCount - previousPerformanceCount) * (1.0f / performanceCounterFrequency);
			//convert to milliseconds, delta milliseconds = seconds * (milliseconds / seconds)
			deltaTime = deltaTime * 1000 / 1;
			
			g_pGameInstance.GameRun(deltaTime);
			
			//store counts
			previousPerformanceCount = currentPerformanceCount;
		}
	}
	
	//shutdown GDI plus
	GdiplusShutdown(gdiplusToken);
	
	//shutdown the game
	g_pGameInstance.GameEnd();
	
	//delete allocated memory
	delete g_pD3DGraphics;
	
	return (int)msg.wParam;
}