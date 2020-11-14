#pragma once

#define MAIN_DISABLED 0
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <Windowsx.h>
#include <Magick++.h>
#include <string>
#include <fstream>

extern HINSTANCE hInstanceGlobal;
extern HBITMAP hBitmap;
extern HWND optionsHWND, hwndMaxFrames, hwndFramesDelay, 
			hwndResolutionCompression, hwndPathToCursor,
			hwndFlagCursor;
extern bool flagMainHWND, flagInit, flagCursorShow;
extern std::string buffStr;
extern std::wstring buffWStr;
extern wchar_t buffW[1024];
extern int maxFrames;
extern LONG delay;
extern double resolution;
extern std::string pathToCursor;
extern std::string prevPathToCursor;
extern Magick::Image cursorIco;

inline LRESULT CALLBACK OptionsWndProc(HWND, UINT, WPARAM, LPARAM);
inline void CreateMainHWND();
inline void ReadOptionsFile();
inline void WriteOptionsFile();