#define _CRT_SECURE_NO_WARNINGS
#define TIMER_ID 1
#define WORK_AREA_TRANSPARENCY_ACTIVE 20
#define WORK_AREA_TRANSPARENCY_DISABLED 0

#include "framework.h"
#include <Windowsx.h>
#include "Video-recording.h"
#include <Magick++.h>
#include <ctime>

HWND mainHWND;
RECT rcSize;
HDC hdcBackBuffer, hdcArea;
PAINTSTRUCT ps;

std::vector<Magick::Image> frames;
int maxFrames = 1000;
bool flagRecording = false;
bool flagMouseDown = false;
bool flagCursorShow = true;
bool areaIsReady = false;
POINT startPoint;
POINT endPoint;
Magick::Geometry selectedArea;
HDC secondHdc;
LONG delay = 250;
LONG width, height, offSetX, offSetY;
LONG resolution = 1;

POINT cursorPos;
Magick::Image cursorIco;

void ResizeWnd(HWND hWnd);
void HideMainHWND()
{
    endPoint.x = 0;
    endPoint.y = 0;
    startPoint.x = 0;
    startPoint.y = 0;
    ResizeWnd(mainHWND);
    SetLayeredWindowAttributes(mainHWND, NULL, WORK_AREA_TRANSPARENCY_DISABLED, LWA_ALPHA);
    SetWindowLong(mainHWND, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST);
    areaIsReady = false;
}

HHOOK _hook;
KBDLLHOOKSTRUCT kbdStruct;

LRESULT __stdcall HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        if (wParam == WM_KEYDOWN)
        {
            kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);
            if (kbdStruct.vkCode == 67 && GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_LWIN))
            {
                SetLayeredWindowAttributes(mainHWND, NULL, WORK_AREA_TRANSPARENCY_ACTIVE, LWA_ALPHA);
                SetWindowLong(mainHWND, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST);
            } 
            else if (kbdStruct.vkCode == VK_ESCAPE)
            {
                flagRecording = false;
                flagMouseDown = false;
                HideMainHWND();
            }
            else if (kbdStruct.vkCode ==  82)
            {
                if (!flagMouseDown && areaIsReady) flagRecording = !flagRecording;
            }
        }
    }

    return CallNextHookEx(_hook, nCode, wParam, lParam);
}

void SetHook()
{
    _hook = SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, NULL, 0);
}

void UnHook()
{
    UnhookWindowsHookEx(_hook);
}

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
                img.compressType(MagickCore::LZWCompression);
                if (flagCursorShow)
                {
                    GetCursorPos(&cursorPos);
                    img.composite(cursorIco, (cursorPos.x - offSetX), (cursorPos.y - offSetY), MagickCore::PlusCompositeOp);
                }
                if (resolution > 1)
                {
                    img.resize(Geometry(std::to_string(width / resolution)));
                }
                img.animationDelay(delay / 10);
                frames.push_back(img);
            }
            else
            {
                UnHook();
                areaIsReady = false;
                if (flagRecording) HideMainHWND();
                std::time_t time = std::time(0);  
                std::tm* now = std::localtime(&time);
                std::string pathToFile = "GIFs/" + std::to_string(now->tm_mday) + "." + std::to_string(now->tm_mon) + 
                    "." + std::to_string(now->tm_year + 1900) + " (" + std::to_string(now->tm_hour) + "-" + 
                    std::to_string(now->tm_min) + "-" + std::to_string(now->tm_sec) + ").gif";

                writeImages(frames.begin(), frames.end(), pathToFile);
                frames.clear();
                if (!flagRecording) areaIsReady = true;
                flagRecording = false;
                SetHook();
            }
        }
        return 0;
    }
    case WM_CREATE:
    {
        LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
        lpcs->style &= ~WS_CAPTION;
        SetWindowLong(hWnd, GWL_STYLE, lpcs->style);

        cursorIco = Magick::Image("cursor/cur1.png");
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
        SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST);
        areaIsReady = true;
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
        FillRect(hdcBackBuffer, &rcSize, (HBRUSH)GetStockObject(WHITE_BRUSH));
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

    mainHWND = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST , className, NULL,
       SWP_NOMOVE | WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MAXIMIZE | WS_MAXIMIZEBOX, 0, 0, resolution.right, resolution.bottom, NULL, NULL, hInstance, NULL);
    SetLayeredWindowAttributes(mainHWND, NULL, WORK_AREA_TRANSPARENCY_DISABLED, LWA_ALPHA);

    if (!mainHWND) return 0;

    ShowWindow(mainHWND, nShowCmd);
    UpdateWindow(mainHWND);
    SetHook();

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