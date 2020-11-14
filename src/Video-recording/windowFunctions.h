#pragma once

#define WORK_AREA_TRANSPARENCY_DISABLED 0

#include <windows.h>

extern RECT resolutionWH, rcSize;
extern POINT startPoint, endPoint;
extern LONG width, height, offSetX, offSetY;
extern bool areaIsReady;
extern HDC hdcBackBuffer, hdcArea;
extern HWND areaHWND;

inline void ResizeWnd(HWND);
inline void HideAreaHWND();