#include "TrayIcon.h"

void CWin32TrayIcon::EnableIcon(NOTIFYICONDATA * pTrayIcon, HWND hWnd, int ID, const char * MouseOverMessage)
{
    pTrayIcon->hWnd = hWnd;
    pTrayIcon->uCallbackMessage = ID;

    pTrayIcon->cbSize = sizeof(NOTIFYICONDATA);
    pTrayIcon->uFlags = NIF_TIP | NIF_SHOWTIP | NIF_ICON | NIF_MESSAGE;

    if (MouseOverMessage)
    {
        strcpy_s(pTrayIcon->szTip, MouseOverMessage);
    }

    pTrayIcon->hIcon = LoadIcon(GetModuleHandle(0), MAKEINTRESOURCEA(100));
    Shell_NotifyIcon(NIM_ADD, pTrayIcon);
}

void CWin32TrayIcon::UpdateTip(NOTIFYICONDATA * pTrayIcon, const char * MouseOverMessage)
{
    strcpy_s(pTrayIcon->szTip, MouseOverMessage);
    Shell_NotifyIcon(NIM_MODIFY, pTrayIcon);
}

void CWin32TrayIcon::DisableIcon(NOTIFYICONDATA * pTrayIcon)
{
    Shell_NotifyIcon(NIM_DELETE, pTrayIcon);
}
