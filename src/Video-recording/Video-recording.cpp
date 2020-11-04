// Video-recording.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include <Windowsx.h>
#include "Video-recording.h"
#include <Magick++.h>

#define N 1
#define TIMER_ID 1

RECT rcSize;
HDC hdcBackBuffer, hdcArea;
PAINTSTRUCT  ps;

std::vector<Magick::Image> frames;
int delay = 500;
int maxFrames = 50;
int resolution = 1;
bool flagRecording = false;
bool flagMouseDown = false;
POINT startPoint;
POINT endPoint;
Magick::Geometry selectedArea;
HDC secondHdc;
LONG width, height, offSetX, offSetY;

void ResizeWnd(HWND hWnd)
{
    HDC hdcWindow = GetDC(hWnd);

    GetClientRect(hWnd, &rcSize);
    
    width = endPoint.x - startPoint.x;
    height = endPoint.y - startPoint.y;
    offSetX = startPoint.x;
    offSetY = startPoint.y;
    
    if (width < 0)
    {
        width = -width;
        offSetX = endPoint.x;
    }

    if (height < 0)
    {
        height = -height;
        offSetY = endPoint.y;
    }

    if (hdcBackBuffer) DeleteDC(hdcBackBuffer);
    hdcBackBuffer = CreateCompatibleDC(hdcWindow);
    HBITMAP hbmBackBuffer = CreateCompatibleBitmap(hdcBackBuffer, rcSize.right - rcSize.left, rcSize.bottom - rcSize.top);
    SelectObject(hdcBackBuffer, hbmBackBuffer);
    DeleteObject(hbmBackBuffer);

    if (hdcArea) DeleteDC(hdcArea);
    hdcArea = CreateCompatibleDC(hdcWindow);
    HBITMAP hbmArea;
    hbmArea = CreateCompatibleBitmap(hdcArea, width, height);
    SelectObject(hdcArea, hbmArea);
    DeleteObject(hbmArea);
    RECT rcSprite;
    SetRect(&rcSprite, 0, 0, width, height);
    FillRect(hdcArea, &rcSprite, (HBRUSH)GetStockObject(BLACK_BRUSH));
    InvalidateRect(hWnd, &rcSize, true);

    ReleaseDC(hWnd, hdcWindow);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_TIMER:
    {
        //SetActiveWindow(hWnd);
        //InvalidateRect(hWnd, &rcSize, true);
        // Record logic
        if (frames.size() > 0 || flagRecording)
        {
            using namespace Magick;
            if (frames.size() < maxFrames && flagRecording)
            {
                Image img("screenshot:");
                img.crop(selectedArea);
                img.repage();
                img.animationDelay(delay / 10);
                //img.resize(Geometry(resolution));
                frames.push_back(img);
            }
            else
            {
                writeImages(frames.begin(), frames.end(), "D:/ig.gif");
                frames.clear();
                flagRecording = false;
            }
        }
        return 0;
    }
    case WM_CREATE:
    {
        SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 50, LWA_ALPHA);

        lpcs->style &= ~WS_CAPTION;
        SetWindowLong(hWnd, GWL_STYLE, lpcs->style);
        ResizeWnd(hWnd);
        SetTimer(hWnd, TIMER_ID, delay, NULL);
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        endPoint.x = GET_X_LPARAM(lParam);
        endPoint.y = GET_Y_LPARAM(lParam);
        if (flagMouseDown) ResizeWnd(hWnd);
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        flagMouseDown = true;
        startPoint.x = GET_X_LPARAM(lParam);
        startPoint.y = GET_Y_LPARAM(lParam);
        return 0;
    }
    case WM_LBUTTONUP:
    {
        flagMouseDown = false;
        endPoint.x = GET_X_LPARAM(lParam);
        endPoint.y = GET_Y_LPARAM(lParam);
        selectedArea = Magick::Geometry(width, height, offSetX, offSetY);
        ResizeWnd(hWnd);

        flagRecording = true;
        return 0;
    }
    case WM_SIZE:
    {     
        return 0;
    }
    case WM_KEYDOWN:
    {
        //KillTimer(hWnd, TIMER_ID);
        //PostQuitMessage(0);
    }
    case WM_PAINT:
    {
        BeginPaint(hWnd, &ps);
        FillRect(hdcBackBuffer, &rcSize, (HBRUSH)GetStockObject(GRAY_BRUSH));
        BitBlt(hdcBackBuffer, offSetX, offSetY, rcSize.right - rcSize.left, rcSize.bottom - rcSize.top, hdcArea, 0, 0, SRCCOPY);
        BitBlt(ps.hdc, 0, 0, rcSize.right - rcSize.left, rcSize.bottom - rcSize.top, hdcBackBuffer, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
        return 0;
    }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}
int WINAPI WinMain(HINSTANCE hPrevInstance, HINSTANCE hInstance, LPSTR lpCmdLine, int nShowCmd)
{
    static TCHAR className[] = TEXT("RecorderClass");
    static TCHAR windowName[] = TEXT("Recorder");

    WNDCLASSEX wcex;

    wcex.cbClsExtra = 0;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.cbWndExtra = 0;
    wcex.hbrBackground = NULL;
    wcex.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hIconSm = NULL;
    wcex.hInstance = hInstance;
    wcex.lpfnWndProc = WndProc;
    wcex.lpszClassName = className;
    wcex.lpszMenuName = NULL;
    wcex.style = 0;

    if (!RegisterClassEx(&wcex))
        return 0;

    RECT resolution;
    GetWindowRect(GetDesktopWindow(), &resolution);

    HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TOPMOST , className, NULL,
       SWP_NOMOVE | WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MAXIMIZE | WS_MAXIMIZEBOX, 0, 0, 1920, 1080, NULL, NULL, hInstance, NULL);
    if (!hWnd)
        return 0;

    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);

    MSG msg;

    while (true)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    return msg.wParam;
}