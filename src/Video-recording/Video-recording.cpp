#define _CRT_SECURE_NO_WARNINGS
#define TIMER_ID 1
#define WORK_AREA_TRANSPARENCY_ACTIVE 20
#define WORK_AREA_TRANSPARENCY_DISABLED 0
#define MAIN_DISABLED 0
#define MAIN_ACTIVE 255
#define MAX_FRAMES 10000

#include "framework.h"
#include <Windowsx.h>
#include "Video-recording.h"
#include <Magick++.h>
#include <ctime>

HWND areaHWND;
RECT rcSize;
HDC hdcBackBuffer, hdcArea;
PAINTSTRUCT ps;

std::vector<Magick::Image> frames;

int maxFrames = 1000;
LONG delay = 250;
double resolution = 1;
bool flagCursorShow = true;
std::string pathToCursor = "cursors/cur0.png";

bool flagRecording = false;
bool flagMouseDown = false;
bool areaIsReady = false;
bool flagMainHWND = false;
bool flagWrite = false;

POINT startPoint;
POINT endPoint;
Magick::Geometry selectedArea;
HDC secondHdc;
LONG width, height, offSetX, offSetY;
POINT cursorPos;

Magick::Image cursorIco;

HWND optionsHWND;
HINSTANCE hInstanceGlobal;
HBITMAP hBitmap;

void CreateMainHWND();
void ResizeWnd(HWND);

void HideMainHWND()
{
    endPoint.x = 0;
    endPoint.y = 0;
    startPoint.x = 0;
    startPoint.y = 0;
    ResizeWnd(areaHWND);
    SetLayeredWindowAttributes(areaHWND, NULL, WORK_AREA_TRANSPARENCY_DISABLED, LWA_ALPHA);
    SetWindowLong(areaHWND, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST);
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
                endPoint.x = 0;
                endPoint.y = 0;
                startPoint.x = 0;
                startPoint.y = 0;
                ResizeWnd(areaHWND);
                SetLayeredWindowAttributes(areaHWND, NULL, WORK_AREA_TRANSPARENCY_ACTIVE, LWA_ALPHA);
                SetWindowLong(areaHWND, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST);
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
            else if (kbdStruct.vkCode == 86 && GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_LWIN))
            {
                DestroyWindow(areaHWND);
            }
            else if (kbdStruct.vkCode == 79 && GetAsyncKeyState(VK_SHIFT) && GetAsyncKeyState(VK_LWIN))
            {
                if (!flagMainHWND)
                {
                    SetLayeredWindowAttributes(optionsHWND, NULL, MAIN_ACTIVE, LWA_ALPHA);
                    SetWindowLong(optionsHWND, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TOPMOST);
                }
                else
                {
                    SetLayeredWindowAttributes(optionsHWND, NULL, MAIN_DISABLED, LWA_ALPHA);
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

LRESULT CALLBACK AreaWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_TIMER:
    {
        if (frames.size() > 0 || flagRecording)
        {
            using namespace Magick;
            if (frames.size() < maxFrames && frames.size() < MAX_FRAMES && flagRecording)
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
                    img.resize(Geometry(std::to_string(((double) width) / resolution)));
                }
                img.animationDelay(delay / 10);
                frames.push_back(img);
            }
            else if (!flagWrite)
            {
                flagWrite = true;
                UnHook();
                areaIsReady = false;
                if (flagRecording) HideMainHWND();

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
                    MessageBox(NULL, L"Make a GIFs folder to store GIF files", L"ERROR", MB_ICONWARNING);
                    pathToFile = "" + std::to_string(now->tm_mday) + "." + std::to_string(now->tm_mon) +
                        "." + std::to_string(now->tm_year + 1900) + " (" + std::to_string(now->tm_hour) + "-" +
                        std::to_string(now->tm_min) + "-" + std::to_string(now->tm_sec) + ").gif";
                    writeImages(frames.begin(), frames.end(), pathToFile);
                }

                frames.clear();
                if (!flagRecording) areaIsReady = true;
                flagRecording = false;
                SetHook();
                flagWrite = false;
            }
        }
        return 0;
    }
    case WM_CREATE:
    {
        try
        {
            cursorIco = Magick::Image(pathToCursor);
        }
        catch(...)
        {
            MessageBox(NULL, L"Cursor not found", L"ERROR", MB_ICONERROR);
            flagCursorShow = false;
        }
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
    case WM_CLOSE:
    {
        return 0;
    }
    case WM_DESTROY:
    {
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

    RECT resolution;
    GetWindowRect(GetDesktopWindow(), &resolution);

    areaHWND = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST, className, NULL,
       SWP_NOMOVE | WS_POPUP | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MAXIMIZE | WS_MAXIMIZEBOX & ~WS_CAPTION, 0, 0, 
       resolution.right, resolution.bottom, NULL, NULL, hInstance, NULL);
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

LRESULT CALLBACK OptionsWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        hBitmap = (HBITMAP)LoadImage(hInstanceGlobal, L"img/background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT     ps;
        HDC             hdc;
        BITMAP          bitmap;
        HDC             hdcMem;
        HGDIOBJ         oldBitmap;

        hdc = BeginPaint(hWnd, &ps);

        hdcMem = CreateCompatibleDC(hdc);
        oldBitmap = SelectObject(hdcMem, hBitmap);

        GetObject(hBitmap, sizeof(bitmap), &bitmap);
        BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_CLOSE:
    {
        SetLayeredWindowAttributes(optionsHWND, NULL, MAIN_DISABLED, LWA_ALPHA);
        SetWindowLong(optionsHWND, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST);
        flagMainHWND = false;
        return 0;
    }
    case WM_CTLCOLOREDIT:
    {
        HDC hdcEdit = (HDC)wParam;
        SetTextColor(hdcEdit, RGB(255, 255, 255));
        SetBkColor(hdcEdit, RGB(5, 24, 16));
        return (LONG) GetStockObject(BLACK_BRUSH);
    }
    break;
    case WM_COMMAND:
    {
        return 0;
    }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CreateMainHWND()
{
    WNDCLASSEX wcex;

    wcex.cbClsExtra = 0;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.cbWndExtra = 0;
    wcex.hbrBackground = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hIcon = NULL;
    wcex.hIconSm = NULL;
    wcex.hInstance = hInstanceGlobal;
    wcex.lpfnWndProc = OptionsWndProc;
    wcex.lpszClassName = L"Video-Recoding";
    wcex.lpszMenuName = NULL;
    wcex.style = 0;

    RegisterClassEx(&wcex);
    optionsHWND = CreateWindowEx(WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOPMOST,
        L"Video-Recoding", NULL, WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU & ~WS_CAPTION, 0, 0, 413, 673, NULL, NULL, NULL, NULL);
    SetLayeredWindowAttributes(optionsHWND, NULL, MAIN_DISABLED, LWA_ALPHA);

    LOGFONT logFont;
    logFont.lfHeight = -16;
    strcpy((char*)logFont.lfFaceName, "Aria");
    auto hfont = CreateFontIndirect(&logFont);

    HWND hwndMaxFrames = CreateWindow(L"EDIT", L"", WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL | WS_CHILD,
        167, 202, 180, 20, optionsHWND, NULL, hInstanceGlobal, NULL);
    SendMessage(hwndMaxFrames, WM_SETFONT, (WPARAM)hfont, (LPARAM)0);

    HWND hwndFramesDelay = CreateWindow(L"EDIT", L"", WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL | WS_CHILD,
        180, 263, 142, 20, optionsHWND, NULL, hInstanceGlobal, NULL);
    SendMessage(hwndFramesDelay, WM_SETFONT, (WPARAM)hfont, (LPARAM)0);

    HWND hwndResolutionCompression = CreateWindow(L"EDIT", L"", WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL | WS_CHILD,
        277, 329, 70, 20, optionsHWND, NULL, hInstanceGlobal, NULL);
    SendMessage(hwndResolutionCompression, WM_SETFONT, (WPARAM)hfont, (LPARAM)0);

    HWND hwndFlagCursor = CreateWindow(L"COMBOBOX", L"", WS_VISIBLE | CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_CHILD,
        167, 389, 186, 100, optionsHWND, NULL, hInstanceGlobal, NULL);
    SendMessage(hwndFlagCursor, WM_SETFONT, (WPARAM)hfont, (LPARAM)0);

    SendMessage(hwndFlagCursor, CB_ADDSTRING, 0, (LPARAM)TEXT("Enabled"));
    SendMessage(hwndFlagCursor, CB_ADDSTRING, 0, (LPARAM)TEXT("Disabled"));
    SendMessage(hwndFlagCursor, CB_SETCURSEL, 1, 0);

    HWND hwndPathToCursor = CreateWindow(L"EDIT", L"", WS_VISIBLE | ES_AUTOHSCROLL | WS_CHILD,
        120, 454, 227, 20, optionsHWND, NULL, hInstanceGlobal, NULL);
    SendMessage(hwndPathToCursor, WM_SETFONT, (WPARAM)hfont, (LPARAM)0);
}
