// Multi-Thread.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "GraphicsLab.h"
//#include "cds_job-master/cds_job.h"
#include <thread>
#include "JobSystem.h"
#include "GraphSystem.h"
#include "World.h"
#include "UI/UISystem.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


#define kNumWorkers 12
#define kTotalJobCount (64*1024)

#define WND_SIZE_WIDTH 800
#define WND_SIZE_HEIGHT 600


#include <chrono>
#include <thread>

std::shared_ptr<GraphSystem> GGraphSystem;
std::shared_ptr<World> GWorld;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	JobSystem::Init(kNumWorkers, kNumWorkers-2, 4096); // TODO(cort): touchy touchy!

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MULTITHREAD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }



    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MULTITHREAD));

	MSG msg = {0};

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			//开始渲染上一帧数据
			GGraphSystem->BeginRender();

			UISystem::GetInstance()->OnUpdate(0.0f);
			
			//更新当前帧逻辑数据
			JobSystem::Update();

			//等待渲染完成
			GGraphSystem->WaitforFinishRender();

			UISystem::GetInstance()->OnPostRender();

			Sleep(1);

		}
	}

	JobSystem::UnInit();

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MULTITHREAD));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;// MAKEINTRESOURCEW(IDC_MULTITHREAD);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT,WND_SIZE_WIDTH, WND_SIZE_HEIGHT, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   GWorld = std::shared_ptr<World>(new World);

   GGraphSystem = std::shared_ptr<GraphSystem>(new GraphSystem(GWorld.get()));


   if (!GGraphSystem->InitDirect3D(hWnd, WND_SIZE_WIDTH, WND_SIZE_HEIGHT))return FALSE;

   GWorld->Init(GGraphSystem->GetD3dDevice());

   GWorld->CreateBox(GGraphSystem->GetD3dDevice(), { 
	   glm::vec3(0.0f),
	   glm::angleAxis(glm::radians(0.0f), glm::vec3(0.f, 1.f, 0.f)),
	   glm::vec3(1.0f) });

   UISystem::GetInstance()->Init();

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_SIZE:
		GGraphSystem->OnResize();
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
