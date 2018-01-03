#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

struct CWindowUtils
{
    static void DisableCloseButton(HWND hwnd);
    static void EnableCloseButton(HWND hwnd);

    static void DisableMinimizeButton(HWND hwnd);
    static void EnableMinimizeButton(HWND hwnd);

    static void DisableMaximizeButton(HWND hwnd);
    static void EnableMaximizeButton(HWND hwnd);
};

