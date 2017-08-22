#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shellapi.h>

struct CWin32TrayIcon
{
    static void EnableIcon(NOTIFYICONDATA *pTrayIcon, HWND hWnd, int ID, const char *MouseOverMessage = nullptr);
    static void UpdateTip(NOTIFYICONDATA *pTrayIcon, const char *MouseOverMessage);
    static void DisableIcon(NOTIFYICONDATA *pTrayIcon);
};