#include "Win32\TrayIcon.h"
#include "Win32\FileUtils.h"

///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////

#define  CLASS_NAME "KMCounter"
#define WINDOW_NAME "KMCounter"

///////////////////////////////////////////////////////////////////////////////

#define TRY_ICON_ID (WM_USER + 1)
#define UNUSED(x) (void)(x)

///////////////////////////////////////////////////////////////////////////////

typedef enum tray_menu_options_e
{
    Menu_Exit,
    Menu_Close,
    Menu_Statistics,

    /*
        NOTE(Andrei): All the options have to go
        above this comment.
    */
    Menu_Counter
} tray_menu_options_e;

///////////////////////////////////////////////////////////////////////////////

typedef struct keyboard_t
{
    size_t Hits;
} keyboard_t;

typedef struct mouse_t
{
    size_t HitsLeftButton;
    size_t HitsRightButton;
    size_t HitsCentralButton;
} mouse_t;

typedef struct kmcounter_t
{
    keyboard_t Kyeboard;
    mouse_t Mouse;
} kmcounter_t;

///////////////////////////////////////////////////////////////////////////////

std::vector<kmcounter_t> g_KMCounterStatistics; /* Usage statistics of the user input */
static kmcounter_t g_KMCounter = { 0 };         /* Count the keyboard and mouse hits */

static int g_AtomianGuardExit  =     0;         /* Exit app control */
static HMENU g_hTrayWnd        =     0;         /* Tray menu window */

///////////////////////////////////////////////////////////////////////////////

std::string GetInfoAsString()
{
    char s[1024] = { 0 };
    char Fmt[] =
        "Mouse hits:    %zd\n"
        "Keyboard hits: %zd\n"
        "------------------\n"
        "Total hits: %zd";

    size_t TotalMouseHists = g_KMCounter.Mouse.HitsLeftButton + g_KMCounter.Mouse.HitsRightButton + g_KMCounter.Mouse.HitsCentralButton;
    size_t TotalHists = TotalMouseHists + g_KMCounter.Kyeboard.Hits;

    sprintf_s(s, Fmt, TotalMouseHists, g_KMCounter.Kyeboard.Hits, TotalHists);
    return s;
}

///////////////////////////////////////////////////////////////////////////////

void AddMenu(HMENU hMenu, int Id, char *WndText)
{
    MENUITEMINFO MenuItem = { 0 };
    MenuItem.cbSize = sizeof(MenuItem);

    MenuItem.fMask = MIIM_ID | MIIM_STATE | MIIM_DATA | MIIM_TYPE;
    MenuItem.fState = MFS_UNCHECKED | MFS_ENABLED;
    MenuItem.fType = MFT_STRING;

    MenuItem.dwTypeData = (char *)WndText;
    MenuItem.wID = Id;

    InsertMenuItem(hMenu, GetMenuItemCount(hMenu), true, &MenuItem);
}

void AddMenuSeparator(HMENU hMenu)
{
    InsertMenu(hMenu, GetMenuItemCount(hMenu), MF_SEPARATOR | MF_BYPOSITION, 0, NULL);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wp, LPARAM lp)
{
    if (nCode == HC_ACTION && (wp == WM_SYSKEYUP || wp == WM_KEYUP))
    {
        ++g_KMCounter.Kyeboard.Hits;
    }

    return CallNextHookEx(NULL, nCode, wp, lp);
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wp, LPARAM lp)
{
    if (nCode == HC_ACTION && wp == WM_LBUTTONUP)
    {
        ++g_KMCounter.Mouse.HitsLeftButton;
    }
    else if(nCode == HC_ACTION && wp == WM_RBUTTONUP)
    {
        ++g_KMCounter.Mouse.HitsRightButton;
    }
    else if (nCode == HC_ACTION && wp == WM_MBUTTONUP)
    {
        ++g_KMCounter.Mouse.HitsCentralButton;
    }

    return CallNextHookEx(NULL, nCode, wp, lp);
}

///////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT Msg, WPARAM wp, LPARAM lp)
{
    if (Msg == WM_CREATE)
    {
        g_hTrayWnd = CreatePopupMenu();
        ShowWindow(GetConsoleWindow(), SW_HIDE);

        AddMenu(g_hTrayWnd, tray_menu_options_e::Menu_Close, "Close this menu");
        AddMenu(g_hTrayWnd, tray_menu_options_e::Menu_Exit, "Exit KMCounter");

        AddMenuSeparator(g_hTrayWnd);
        AddMenu(g_hTrayWnd, tray_menu_options_e::Menu_Statistics, "Statistics");

        return 0;
    }
    else if (Msg == WM_CLOSE || Msg == WM_DESTROY)
    {
        ShowWindow(GetConsoleWindow(), SW_SHOW);
        DestroyMenu(g_hTrayWnd);

        g_AtomianGuardExit = 1;
        return 0;
    }
    else if (Msg == TRY_ICON_ID && lp == WM_RBUTTONDOWN)
    {
        POINT CursorPos = { 0 };
        GetCursorPos(&CursorPos);

        switch (TrackPopupMenu(g_hTrayWnd, TPM_RETURNCMD | TPM_NONOTIFY, CursorPos.x, CursorPos.y, 0, hWnd, NULL))
        {
            case tray_menu_options_e::Menu_Exit:
            {
                DestroyMenu(g_hTrayWnd);
                g_AtomianGuardExit = 1;
            } break;

            case tray_menu_options_e::Menu_Close:
            {
                // NOTE(Andrei): Don't do nothing
            } break;

            //////////////////////////////////////////////////////////////////////////

            case tray_menu_options_e::Menu_Statistics:
            {
                MessageBoxA(NULL, "Show a graph with the user statistics, or anything else", "NOT IMPLEMENTED", MB_OK);
            } break;
        }

        return 0;
    }
    else if (Msg == TRY_ICON_ID)
    {

    }

    return DefWindowProcA(hWnd, Msg, wp, lp);
}

int main(int argc, char *argv[])
{
    WNDCLASSEXA WndCls = { 0 };

    WndCls.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    WndCls.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndCls.hCursor = LoadCursor(NULL, IDC_ARROW);

    WndCls.lpfnWndProc = WndProc;
    WndCls.lpszClassName = CLASS_NAME;

    WndCls.cbSize = sizeof(WNDCLASSEXA);
    WndCls.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (RegisterClassExA(&WndCls) == 0)
    {
        return EXIT_FAILURE;
    }

    HWND hWnd = CreateWindowExA
    (
        WS_EX_CLIENTEDGE, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, 0, 0,
        50, 50, HWND_DESKTOP, NULL, GetModuleHandle(0), NULL
    );

    if (hWnd == NULL)
    {
        return EXIT_FAILURE;
    }

    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    MSG hMsg = { 0 };
    s_file_content statisticsData = CFileUtils::LoadFileIntoMemory("statistics.dat");

    if (statisticsData.Size)
    {
        size_t numElements = statisticsData.Size / sizeof(kmcounter_t);
        g_KMCounterStatistics.resize(numElements);

        for (size_t i = 0; i < numElements; ++i)
        {
            g_KMCounterStatistics[i] = ((kmcounter_t *)statisticsData.Memory)[i];
        }
    }

    NOTIFYICONDATA TrayIcon = { 0 };
    CWin32TrayIcon::EnableIcon(&TrayIcon, hWnd, TRY_ICON_ID, "KMCounter");

    HHOOK KbdHook = SetWindowsHookExA(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, GetModuleHandle(NULL), NULL);
    HHOOK MouseHook = SetWindowsHookExA(WH_MOUSE_LL , (HOOKPROC)LowLevelMouseProc   , GetModuleHandle(NULL), NULL);

    while (!g_AtomianGuardExit)
    {
        while (PeekMessage(&hMsg, NULL, 0U, 0U, PM_REMOVE) > 0)
        {
            TranslateMessage(&hMsg);
            DispatchMessageA(&hMsg);
        }

        const std::string &s = GetInfoAsString();
        CWin32TrayIcon::UpdateTip(&TrayIcon, s.c_str());

        // NOTE(Andrei): Don't waste CPU time
        Sleep(10);
    }

    DestroyWindow(hWnd);
    UnhookWindowsHookEx(KbdHook);
    UnhookWindowsHookEx(MouseHook);

    CWin32TrayIcon::DisableIcon(&TrayIcon);
    UnregisterClass(CLASS_NAME, GetModuleHandle(NULL));

    s_file_content fileContent =
    {
        &g_KMCounter,
        sizeof(g_KMCounter)
    };

    CFileUtils::WriteFileContentIntoDisk(fileContent, "statistics.dat", CFileUtils::Append);
    return EXIT_SUCCESS;
}