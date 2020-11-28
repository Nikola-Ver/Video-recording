#include "options.h"

inline LRESULT CALLBACK OptionsWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
        SetWindowPos(optionsHWND, NULL, 0, 0, 413, 673, SWP_HIDEWINDOW);
        flagMainHWND = false;
        return 0;
    }
    case WM_CTLCOLOREDIT:
    {
        HDC hdcEdit = (HDC)wParam;
        SetTextColor(hdcEdit, RGB(255, 255, 255));
        SetBkColor(hdcEdit, RGB(5, 24, 16));
        return (LONG)GetStockObject(BLACK_BRUSH);
    }
    break;
    case WM_COMMAND:
    {
        if (!flagInit) break;

        GetWindowText(hwndMaxFrames, buffW, 1024);
        buffWStr = std::wstring(buffW);
        buffStr = std::string(buffWStr.begin(), buffWStr.end());
        try
        {
            maxFrames = std::stoi(buffStr);
        }
        catch (...) {}

        GetWindowText(hwndFramesDelay, buffW, 1024);
        buffWStr = std::wstring(buffW);
        buffStr = std::string(buffWStr.begin(), buffWStr.end());
        try
        {
            delay = std::stol(buffStr);
        }
        catch (...) {}

        GetWindowText(hwndResolutionCompression, buffW, 1024);
        buffWStr = std::wstring(buffW);
        buffStr = std::string(buffWStr.begin(), buffWStr.end());
        try
        {
            resolution = std::stol(buffStr);
        }
        catch (...) {}

        GetWindowText(hwndPathToCursor, buffW, 1024);
        buffWStr = std::wstring(buffW);
        pathToCursor = std::string(buffWStr.begin(), buffWStr.end());

        ComboBox_GetLBText(hwndFlagCursor, ComboBox_GetCurSel(hwndFlagCursor), buffW);
        buffWStr = std::wstring(buffW);

        if (buffWStr == L"Enabled")
        {
            flagCursorShow = true;
        }
        else
        {
            flagCursorShow = false;
        }

        if (prevPathToCursor != pathToCursor)
        {
            prevPathToCursor = pathToCursor;
            try
            {
                cursorIco = Magick::Image(pathToCursor);
            }
            catch (...) {}
        }

        return 0;
    }
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

inline void CreateMainHWND()
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

    hwndMaxFrames = CreateWindow(L"EDIT", L"", WS_VISIBLE | ES_NUMBER | WS_TABSTOP | ES_AUTOHSCROLL | WS_CHILD,
        167, 202, 180, 20, optionsHWND, NULL, hInstanceGlobal, NULL);
    SendMessage(hwndMaxFrames, WM_SETFONT, (WPARAM)hfont, (LPARAM)0);

    hwndFramesDelay = CreateWindow(L"EDIT", L"", WS_VISIBLE | ES_NUMBER | WS_TABSTOP | ES_AUTOHSCROLL | WS_CHILD,
        180, 263, 142, 20, optionsHWND, NULL, hInstanceGlobal, NULL);
    SendMessage(hwndFramesDelay, WM_SETFONT, (WPARAM)hfont, (LPARAM)0);

    hwndResolutionCompression = CreateWindow(L"EDIT", L"", WS_VISIBLE | ES_NUMBER | WS_TABSTOP | ES_AUTOHSCROLL | WS_CHILD,
        277, 329, 70, 20, optionsHWND, NULL, hInstanceGlobal, NULL);
    SendMessage(hwndResolutionCompression, WM_SETFONT, (WPARAM)hfont, (LPARAM)0);

    hwndFlagCursor = CreateWindow(L"COMBOBOX", L"", WS_VISIBLE | WS_TABSTOP | CBS_HASSTRINGS | CBS_DROPDOWNLIST | WS_CHILD,
        167, 389, 186, 100, optionsHWND, NULL, hInstanceGlobal, NULL);
    SendMessage(hwndFlagCursor, WM_SETFONT, (WPARAM)hfont, (LPARAM)0);

    SendMessage(hwndFlagCursor, CB_ADDSTRING, 0, (LPARAM)TEXT("Enabled"));
    SendMessage(hwndFlagCursor, CB_ADDSTRING, 0, (LPARAM)TEXT("Disabled"));

    hwndPathToCursor = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | WS_CHILD,
        120, 454, 227, 20, optionsHWND, NULL, hInstanceGlobal, NULL);
    SendMessage(hwndPathToCursor, WM_SETFONT, (WPARAM)hfont, (LPARAM)0);
}

inline void ReadOptionsFile()
{
    std::ifstream file;
    try
    {
        cursorIco = Magick::Image(pathToCursor);
    }
    catch (...)
    {
        MessageBox(NULL, L"Cursor not found", L"ERROR", MB_ICONERROR | MB_TOPMOST);
        flagCursorShow = false;
    }

    try
    {
        file.open("options.dat");
        file >> maxFrames >> delay >> resolution >> flagCursorShow >> pathToCursor;
    }
    catch (...) {}

    file.close();
    SetWindowText(hwndMaxFrames, (LPCWSTR)std::to_wstring(maxFrames).c_str());
    SetWindowText(hwndFramesDelay, (LPCWSTR)std::to_wstring(delay).c_str());
    SetWindowText(hwndResolutionCompression, (LPCWSTR)std::to_wstring(resolution).c_str());
    SetWindowText(hwndPathToCursor, (LPCWSTR)(std::wstring(pathToCursor.begin(), pathToCursor.end())).c_str());
    if (flagCursorShow)
    {
        SendMessage(hwndFlagCursor, CB_SETCURSEL, 0, 0);
    }
    else
    {
        SendMessage(hwndFlagCursor, CB_SETCURSEL, 1, 0);
    }

    flagInit = true;
    prevPathToCursor = pathToCursor;
}

inline void WriteOptionsFile()
{
    std::ofstream myfile;
    myfile.open("options.dat");
    myfile << maxFrames << " " << delay << " " << resolution << " " << flagCursorShow << " " << pathToCursor << " ";
    myfile.close();
}