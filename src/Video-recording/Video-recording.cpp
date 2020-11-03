// Video-recording.cpp : Определяет точку входа для приложения.
//

#include "framework.h"
#include "Video-recording.h"
#include <Magick++.h>

#define N 1
#define TIMER_ID 1

RECT rcSize;
HDC hdcBackBuffer, hdcTable;
PAINTSTRUCT  ps;

std::vector<Magick::Image> frames;
int delay = 100;
int maxFrames = 10;
std::string resolution = "1920";
bool flagRecording = true;
HHOOK _hook;

LRESULT CALLBACK KeyDownProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    MessageBox(NULL, L"F1 is pressed!", L"key pressed", MB_ICONINFORMATION);
    return CallNextHookEx(_hook, nCode, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HDC          hdc;

    switch (msg)
    {
    case WM_TIMER:
    {
        InvalidateRect(hWnd, &rcSize, true);
        return 0;
    }
    case WM_SIZE:
    {
        GetClientRect(hWnd, &rcSize);
        return 0;
    }
    case WM_CREATE:
    {
        SetTimer(hWnd, TIMER_ID, delay, NULL);
        return 0;
    }
    case WM_KEYDOWN:
    {
        KillTimer(hWnd, TIMER_ID);
        PostQuitMessage(0);
        return 0;
    }
    case WM_PAINT:
    {
        BeginPaint(hWnd, &ps);

        // Record logic
        if (frames.size() > 0 || flagRecording)
        {
            using namespace Magick;
            if (frames.size() < maxFrames && flagRecording)
            {
                Image img("screenshot:");
                img.crop(Geometry(500, 500, 200, 200));
                //img.crop(Geometry("500x500+200+200"));
                img.repage();
                //img.trim();
                //img.chop(Geometry("700x700"));
                img.animationDelay(delay / 10);
                //img.resize(Geometry(resolution));
                frames.push_back(img);
            }
            else
            {
                writeImages(frames.begin(), frames.end(), "D:/das.gif");
                frames.clear();
                frames.clear();
            }
        }

        EndPaint(hWnd, &ps);

        return 0;
    }
    }
}

int WINAPI WinMain(HINSTANCE hPrevInstance, HINSTANCE hInstance, LPSTR lpCmdLine, int nShowCmd)
{
    static TCHAR className[] = TEXT("GameClass");
    static TCHAR windowName[] = TEXT("WinApi");

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

    HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, className, windowName, NULL, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
    if (!hWnd)
        return 0;

    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);

    SetWindowsHookEx(WH_KEYBOARD, KeyDownProc, NULL, 0);

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