#include "windowFunctions.h"

inline void ResizeWnd(HWND hWnd)
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

inline void HideAreaHWND()
{
    endPoint.x = 0;
    endPoint.y = 0;
    startPoint.x = 0;
    startPoint.y = 0;
    ResizeWnd(areaHWND);
    SetLayeredWindowAttributes(areaHWND, NULL, WORK_AREA_TRANSPARENCY_DISABLED, LWA_ALPHA);
    SetWindowLong(areaHWND, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED);
    SetWindowPos(areaHWND, NULL, 0, 0, resolutionWH.right, resolutionWH.bottom, SWP_HIDEWINDOW);
    areaIsReady = false;
}