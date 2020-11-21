#define _CRT_SECURE_NO_WARNINGS
#define TIMER_ID 1
#define WORK_AREA_TRANSPARENCY_ACTIVE 20
#define WORK_AREA_TRANSPARENCY_DISABLED 0
#define MAIN_DISABLED 0
#define MAIN_ACTIVE 255
#define MAX_NUMBER_OF_PIXELS 250000000

#include "framework.h"
#include <Windowsx.h>
#include "Video-recording.h"
#include "windowFunctions.cpp"
#include "options.cpp"
#include <Magick++.h>
#include <ctime>
#include <fstream>

HWND areaHWND;
RECT resolutionWH;
RECT rcSize;
HDC hdcBackBuffer, hdcArea;
PAINTSTRUCT ps;

std::vector<Magick::Image> frames;

int maxFrames = 1000;
LONG delay = 250;
double resolution = 1;
bool flagCursorShow = true;
std::string pathToCursor = "cursors/cur0.png";
std::string prevPathToCursor;
bool flagRecording = false;
bool flagMouseDown = false;
bool areaIsReady = false;
bool flagMainHWND = false;
bool flagInit = false;
bool flagEscKey = false;

POINT startPoint;
POINT endPoint;
Magick::Geometry selectedArea;
HDC secondHdc;
LONG width, height, offSetX, offSetY;
LONG numberOfPixelsPerFrame;
POINT cursorPos;

Magick::Image cursorIco;

HWND optionsHWND;
HINSTANCE hInstanceGlobal;
HBITMAP hBitmap;
HWND hwndMaxFrames;
HWND hwndFramesDelay;
HWND hwndResolutionCompression;
HWND hwndFlagCursor;
HWND hwndPathToCursor;

std::string buffStr;
std::wstring buffWStr;
wchar_t buffW[1024];

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
                endPoint.x = 0;
                endPoint.y = 0;
                startPoint.x = 0;
                startPoint.y = 0;
                ResizeWnd(areaHWND);
                flagRecording = false;
                SetLayeredWindowAttributes(areaHWND, NULL, WORK_AREA_TRANSPARENCY_ACTIVE, LWA_ALPHA);
                SetWindowLong(areaHWND, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST);
                SetWindowPos(areaHWND, NULL, 0, 0, resolutionWH.right, resolutionWH.bottom, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE);
            } 
            else if (kbdStruct.vkCode == VK_ESCAPE && GetAsyncKeyState(VK_LWIN))
            {
                flagRecording = false;
                flagMouseDown = false;
                flagEscKey = true;
                HideAreaHWND();
            }
            else if (kbdStruct.vkCode ==  VK_F2)
            {
                if (!flagMouseDown && areaIsReady)
                {
                    flagRecording = !flagRecording;
                    if (flagRecording) SetTimer(areaHWND, TIMER_ID, delay, NULL);
                }
            }
            else if (kbdStruct.vkCode == 86 && GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_LWIN))
            {
                DestroyWindow(areaHWND);
            }
            else if (kbdStruct.vkCode == 79 && GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_LWIN))
            {
                if (!flagMainHWND)
                {
                    SetLayeredWindowAttributes(optionsHWND, NULL, MAIN_ACTIVE, LWA_ALPHA);
                    SetWindowPos(optionsHWND, NULL, 0, 0, 413, 673, SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOMOVE);
                    SetWindowLong(optionsHWND, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST);
                }
                else
                {
                    WriteOptionsFile();
                    SetLayeredWindowAttributes(optionsHWND, NULL, MAIN_DISABLED, LWA_ALPHA);
                    SetWindowPos(optionsHWND, NULL, 0, 0, 413, 673, SWP_HIDEWINDOW);
                    SetWindowLong(optionsHWND, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST);
                }
                flagMainHWND = !flagMainHWND;
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

LRESULT CALLBACK AreaWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_TIMER:
    {
        if (frames.size() > 0 || flagRecording)
        {
            using namespace Magick;
            if (frames.size() < maxFrames && frames.size() * numberOfPixelsPerFrame / (resolution * resolution) < MAX_NUMBER_OF_PIXELS && flagRecording)
            {
                Image img;
                try
                {
                    img = Image("screenshot:");
                }
                catch (...)
                {
                    flagRecording = false;
                    KillTimer(areaHWND, TIMER_ID);
                    MessageBox(NULL, L"Can't start recording", L"ERROR", MB_ICONERROR);
                    return 0;
                }
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
                    img.resize(Geometry(std::to_string(((double) width) / resolution)));
                }
                img.animationDelay(delay / 10);
                frames.push_back(img);
            }
            else
            {
                KillTimer(areaHWND, TIMER_ID);
                MessageBox(NULL, L"Recording is over", L"Notification", MB_ICONINFORMATION);
                UnHook();
                areaIsReady = false;

                std::time_t time = std::time(0);  
                std::tm* now = std::localtime(&time);
                std::string pathToFile = "GIFs/" + std::to_string(now->tm_mday) + "." + std::to_string(now->tm_mon) + 
                    "." + std::to_string(now->tm_year + 1900) + " (" + std::to_string(now->tm_hour) + "-" + 
                    std::to_string(now->tm_min) + "-" + std::to_string(now->tm_sec) + ").gif";

                try 
                {
                    writeImages(frames.begin(), frames.end(), pathToFile);
                }
                catch (...)
                {
                    MessageBox(NULL, L"Make a GIFs folder to store GIF files", L"ERROR", MB_ICONERROR);
                    pathToFile = "" + std::to_string(now->tm_mday) + "." + std::to_string(now->tm_mon) +
                        "." + std::to_string(now->tm_year + 1900) + " (" + std::to_string(now->tm_hour) + "-" +
                        std::to_string(now->tm_min) + "-" + std::to_string(now->tm_sec) + ").gif";
                    writeImages(frames.begin(), frames.end(), pathToFile);
                }

                frames.clear();
                if (!flagRecording && !flagEscKey) areaIsReady = true;
                flagRecording = false;
                SetHook();
                flagEscKey = false;
                MessageBox(NULL, L"GIF is ready", L"Notification", MB_ICONINFORMATION);
            }
        }
        return 0;
    }
    case WM_CREATE:
    {
        ReadOptionsFile();
        ResizeWnd(hWnd);
        HideAreaHWND();
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
        numberOfPixelsPerFrame = width * height;
        return 0;
    }
    case WM_SIZE:
    {     
        return 0;
    }
    case WM_CLOSE:
    {
        return 0;
    }
    case WM_DESTROY:
    {
        UnHook();
        WriteOptionsFile();
        PostQuitMessage(0);  
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
    static TCHAR className[] = TEXT("RecorderAreaClass");
    static TCHAR windowName[] = TEXT("AreaRecorder");

    WNDCLASSEX wcex;

    wcex.cbClsExtra = 0;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.cbWndExtra = 0;
    wcex.hbrBackground = NULL;
    wcex.hCursor = LoadCursor(hInstance, IDC_CROSS);
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hIconSm = NULL;
    wcex.hInstance = hInstance;
    wcex.lpfnWndProc = AreaWndProc;
    wcex.lpszClassName = className;
    wcex.lpszMenuName = NULL;
    wcex.style = 0;

    if (!RegisterClassEx(&wcex))
        return 0;

    hInstanceGlobal = hInstance;
    CreateMainHWND();
    GetWindowRect(GetDesktopWindow(), &resolutionWH);

    areaHWND = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST, className, NULL,
       WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MAXIMIZE | WS_MAXIMIZEBOX & ~WS_CAPTION, 0, 0, 
        resolutionWH.right, resolutionWH.bottom, NULL, NULL, hInstance, NULL);
    SetLayeredWindowAttributes(areaHWND, NULL, WORK_AREA_TRANSPARENCY_DISABLED, LWA_ALPHA);

    if (!areaHWND) return 0;

    ShowWindow(areaHWND, nShowCmd);
    UpdateWindow(areaHWND);
    SetHook();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
