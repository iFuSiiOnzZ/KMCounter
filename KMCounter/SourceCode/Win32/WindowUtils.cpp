#include "WindowUtils.h"

void CWindowUtils::DisableCloseButton(HWND hwnd)
{
    EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
}

void CWindowUtils::EnableCloseButton(HWND hwnd)
{
    EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
}

void CWindowUtils::DisableMinimizeButton(HWND hwnd)
{
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_MINIMIZEBOX);
}

void CWindowUtils::EnableMinimizeButton(HWND hwnd)
{
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_MINIMIZEBOX);
}

void CWindowUtils::DisableMaximizeButton(HWND hwnd)
{
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
}

void CWindowUtils::EnableMaximizeButton(HWND hwnd)
{
    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | WS_MAXIMIZEBOX);
}