// GDI_CapturingAnImage.cpp : Defines the entry point for the application.
//

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#include <windows.h>
#include <commctrl.h>
// Файлы заголовков среды выполнения C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "GDI_CapturingAnImage.h"

#include <functional>
#include <libhackrfpp/hackrf_device.h>
#include <secam_framebuffer/secam_framebuffer.h>
#include <secam_framebuffer/progressive_fb.h>
#include <circular_buffer/circular_buffer.h>

volatile bool Exiting = false;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

HWND hWnd;

HANDLE ScreenShotThreadHandle;
HANDLE ScreenShotThreadSemaphore;

using TFb = ProgressiveFB;

class FbListener : public FrameBufferBase::IListener {
public:
    void onFrameRequest() override {
        // PostMessage(hWnd, WM_USER_MAKE_SCREENSHOT, 0, 0);
        ReleaseSemaphore(ScreenShotThreadSemaphore, 1, 0);
    }
};

std::shared_ptr<FrameBufferBase> fb;
std::shared_ptr<HackRFdevice> hackrf;
std::shared_ptr<CircularBuffer> circularBuf;
std::shared_ptr<FbListener> fbListener;

template <typename T>
void setFB() {
    hackrf->set_sample_rate(T::SAMPLE_RATE);

    fb = std::make_shared<T>();

    circularBuf->setListener(fb);

    fb->setListener(fbListener);
}

int CaptureAnImage(HWND hWnd);


DWORD WINAPI ScreenShotThreadLoop(LPVOID lpParam)
{
    while (!Exiting)
    {
        WaitForSingleObject(
            ScreenShotThreadSemaphore,
            1000L);

        CaptureAnImage(hWnd);
    }
    return 0;
}

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

//
//   FUNCTION: CaptureAnImage(HWND hWnd)
//
//   PURPOSE: Captures a screenshot
//
//   COMMENTS: 
//
//      Note: This function attempts to create a file called captureqwsx.bmp 
//        

char* lpbitmap = NULL;
int lpbitmapLength = 0;

const int screenShotWidth = 720;
const int screenShotHeight = 570;

int CaptureAnImage(HWND hWnd)
{
    auto frameBuffer = fb;

    int screenX = (GetSystemMetrics(SM_CXSCREEN) / 2 - screenShotWidth / 2) - 32;
    int screenY = (GetSystemMetrics(SM_CYSCREEN) / 2 - screenShotHeight / 2);

    HDC hdcScreen;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    DWORD dwBmpSize = 0;

    // Retrieve the handle to a display device context for the client 
    // area of the window. 
    hdcScreen = GetDC(NULL);

    // Create a compatible bitmap from the Window DC.
    hbmScreen = CreateCompatibleBitmap(hdcScreen, screenShotWidth, screenShotHeight); // hbmScreen = CreateCompatibleBitmap(hdcWindow, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

    if (!hbmScreen)
    {
        MessageBox(hWnd, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
        goto done;
    }


    // Create a compatible DC, which is used in a BitBlt from the window DC.
    hdcMemDC = CreateCompatibleDC(hdcScreen);

    if (!hdcMemDC)
    {
        MessageBox(hWnd, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
        goto done;
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC, hbmScreen);

    // Bit block transfer into our compatible memory DC.
    if (!BitBlt(hdcMemDC,
        0, 0,
		screenShotWidth, screenShotHeight,//rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
        hdcScreen,//hdcWindow,
		screenX, screenY,
        SRCCOPY))
    {
        MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    BITMAP bmpScreen;
    // Get the BITMAP from the HBITMAP.
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

    BITMAPINFOHEADER   bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.
    
    if (lpbitmap == NULL)
    {
        lpbitmap = (char*)malloc(dwBmpSize);
        lpbitmapLength = dwBmpSize;
    }
    else if (dwBmpSize != lpbitmapLength)
    {
        free(lpbitmap);
        lpbitmap = (char*)malloc(dwBmpSize);
        lpbitmapLength = dwBmpSize;
    }

    // Gets the "bits" from the bitmap, and copies them into a buffer 
    // that's pointed to by lpbitmap.
    GetDIBits(hdcScreen, hbmScreen, 0,
        (UINT)bmpScreen.bmHeight,
        lpbitmap,
        (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    frameBuffer->LoadBitMap32BppMirrorV(bmpScreen.bmWidth, bmpScreen.bmHeight, lpbitmap);

    // Clean up.
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);

    return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    ScreenShotThreadSemaphore = CreateSemaphore(
        NULL,           // default security attributes
        0,  // initial count
        1,  // maximum count
        NULL);          // unnamed semaphore

    hackrf = std::make_shared<HackRFdevice>();
    hackrf->openAny();
    hackrf->set_freq(770000000);
    hackrf->amp_enable(false);
    hackrf->set_txvga_gain(10);

    
    circularBuf = std::make_shared<CircularBuffer>();
    fbListener = std::make_shared<FbListener>();

    setFB<SECAM_FrameBuffer>();

    hackrf->setCallbackListener(circularBuf);
    hackrf->startTransmit();


    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GDICAPTURINGANIMAGE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GDICAPTURINGANIMAGE));

    ScreenShotThreadHandle = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size  
        ScreenShotThreadLoop,       // thread function name
        0,          // argument to thread function 
        0,                      // use default creation flags 
        0);   // returns the thread identifier 

    MSG msg;
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT)
            break;
    }

    Exiting = true;

    WaitForSingleObject(
        ScreenShotThreadHandle,
        INFINITE
    );

    hackrf->transferAbort();
    hackrf->close();

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = { 0 };

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GDICAPTURINGANIMAGE);
    wcex.lpszClassName = szWindowClass;

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

HWND hwndTrack;

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 320, 80, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    hwndTrack = CreateWindow(L"msctls_trackbar32", L"",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_TRANSPARENTBKGND | TBS_AUTOTICKS | TBS_TOOLTIPS,
        20, 20,
        90, 30,
        hWnd,
        (HMENU)IDM_SLIDER,
        hInstance, NULL);

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
    static int gain = 10;
    switch (message)
    {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_INTEROFFSET_P:
            //hackrf->set_txvga_gain(++gain);
            //fb->interOffset+= 10;
            //fb->fillBuffer(fb->buffer);
            break;
        case IDM_INTEROFFSET_M:
            //hackrf->set_txvga_gain(--gain);
            //fb->interOffset-= 10;
           // fb->fillBuffer(fb->buffer);
            break;
        case IDM_PROGRESSIVE:
            setFB<ProgressiveFB>();
            break;
        case IDM_INTERLEACED:
            setFB<SECAM_FrameBuffer>();
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    //case WM_PAINT:
    //{
        //PAINTSTRUCT ps;
        //HDC hdc = BeginPaint(hWnd, &ps);
        
        //EndPaint(hWnd, &ps);
    //}
    //break;
    //case WM_USER_MAKE_SCREENSHOT:
    //{
        //CaptureAnImage(hWnd);
    //}
    //break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_HSCROLL:
        switch (LOWORD(wParam)) {

        case TB_ENDTRACK:
        {
            DWORD dwPos = SendMessage(hwndTrack, TBM_GETPOS, 0, 0);

           //fb->interOffset = fb->rowLength * dwPos / 100;
           //fb->fillBuffer(fb->buffer);
        }



            break;

        default:

            break;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}