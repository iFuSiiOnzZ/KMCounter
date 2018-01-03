#include "Win32\TrayIcon.h"
#include "Win32\FileUtils.h"

#include "Win32\ScreenUtils.h"
#include "Win32\WindowUtils.h"

///////////////////////////////////////////////////////////////////////////////

#include <string>
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////

#define  CLASS_NAME "KMCounter"
#define WINDOW_NAME "KMCounter"

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 360


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
    FILETIME StartTime;
    FILETIME EndTime;

    keyboard_t Kyeboard;
    mouse_t Mouse;
} kmcounter_t;

///////////////////////////////////////////////////////////////////////////////
static s_offscreen_buffer g_BackBuffer = { 0 };
static bool g_ShowStatistics = false;
static int g_StatisticsPos = -1;


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

size_t GetTotalHits(const kmcounter_t &KMCounter)
{
    size_t TotalMouseHists = KMCounter.Mouse.HitsLeftButton + KMCounter.Mouse.HitsRightButton + KMCounter.Mouse.HitsCentralButton;
    size_t TotalHists = TotalMouseHists + KMCounter.Kyeboard.Hits;

    return TotalHists;
}

///////////////////////////////////////////////////////////////////////////////

int AddMenu(HMENU hMenu, int Id, char *WndText)
{
    MENUITEMINFO MenuItem = { 0 };
    MenuItem.cbSize = sizeof(MenuItem);

    MenuItem.fMask = MIIM_ID | MIIM_STATE | MIIM_DATA | MIIM_TYPE;
    MenuItem.fState = MFS_UNCHECKED | MFS_ENABLED;
    MenuItem.fType = MFT_STRING;

    MenuItem.dwTypeData = (char *)WndText;
    MenuItem.wID = Id;

    int MenuPos = GetMenuItemCount(hMenu);
    InsertMenuItemA(hMenu, MenuPos, true, &MenuItem);

    return MenuPos;
}

int AddMenuSeparator(HMENU hMenu)
{
    int MenuPos = GetMenuItemCount(hMenu);
    InsertMenuA(hMenu, MenuPos, MF_SEPARATOR | MF_BYPOSITION, 0, NULL);

    return MenuPos;
}

void UpdateMenu(HMENU hMenu, int MenuPos, int Id, char *WndText)
{
    ModifyMenu(hMenu, MenuPos, MF_BYPOSITION | MF_STRING, Id, WndText);
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
        g_StatisticsPos = AddMenu(g_hTrayWnd, tray_menu_options_e::Menu_Statistics, "Show statistics");

        return 0;
    }
    else if (Msg == WM_CLOSE || Msg == WM_DESTROY)
    {
        ShowWindow(GetConsoleWindow(), SW_SHOW);
        DestroyMenu(g_hTrayWnd);

        g_AtomianGuardExit = 1;
        return 0;
    }
    else if (Msg == WM_PAINT)
    {
        PAINTSTRUCT Paint = { 0 };
        HDC DeviceContext = BeginPaint(hWnd, &Paint);

        s_window_dimensions d = CScreenUtils::GetWindowDimension(hWnd);
        CScreenUtils::ResizeBIBSection(&g_BackBuffer, d.Width, d.Height);
        CScreenUtils::DisplayBuffer(&g_BackBuffer, DeviceContext, d.Width, d.Height);

        EndPaint(hWnd, &Paint);
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
                g_ShowStatistics = !g_ShowStatistics;
                ShowWindow(hWnd, g_ShowStatistics ? SW_SHOW : SW_HIDE);

                char *TryMenuText = g_ShowStatistics ? "Hide statistics" : "Show statistics";
                UpdateMenu(g_hTrayWnd, g_StatisticsPos, tray_menu_options_e::Menu_Statistics, TryMenuText);
            } break;
        }

        return 0;
    }

    return DefWindowProcA(hWnd, Msg, wp, lp);
}

int main(int argc, char *argv[])
{
    GetSystemTimeAsFileTime(&g_KMCounter.StartTime);
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

    int StartX = GetSystemMetrics(SM_CXSCREEN) / 2 - WINDOW_WIDTH  / 2;
    int StartY = GetSystemMetrics(SM_CYSCREEN) / 2 - WINDOW_HEIGHT / 2;

    HWND hWnd = CreateWindowExA
    (
        WS_EX_CLIENTEDGE, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, StartX, StartY,
        WINDOW_WIDTH, WINDOW_HEIGHT, HWND_DESKTOP, NULL, GetModuleHandle(0), NULL
    );

    if (hWnd == NULL)
    {
        return EXIT_FAILURE;
    }

    CWindowUtils::DisableCloseButton(hWnd);
    CWindowUtils::DisableMaximizeButton(hWnd);
    CWindowUtils::DisableMinimizeButton(hWnd);

    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    MSG hMsg = { 0 };
    HDC DeviceContext = GetDC(hWnd);
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

    g_KMCounterStatistics.push_back(g_KMCounter);
    std::vector<s_color> statisticsColors(g_KMCounterStatistics.size(), s_color{ 1.0f, 1.0f, 1.0f });

    for (size_t i = 0; i < statisticsColors.size() - 1; ++i)
    {
        statisticsColors[i].r = (float)rand() / (RAND_MAX + 1.0f);
        statisticsColors[i].g = (float)rand() / (RAND_MAX + 1.0f);
        statisticsColors[i].b = (float)rand() / (RAND_MAX + 1.0f);
    }

    NOTIFYICONDATA TrayIcon = { 0 };
    CWin32TrayIcon::EnableIcon(&TrayIcon, hWnd, TRY_ICON_ID, "KMCounter");

    HHOOK KbdHook = SetWindowsHookExA(WH_KEYBOARD_LL, (HOOKPROC)LowLevelKeyboardProc, GetModuleHandle(NULL), NULL);
    HHOOK MouseHook = SetWindowsHookExA(WH_MOUSE_LL, (HOOKPROC)LowLevelMouseProc, GetModuleHandle(NULL), NULL);

    while (!g_AtomianGuardExit)
    {
        while (PeekMessage(&hMsg, NULL, 0U, 0U, PM_REMOVE) > 0)
        {
            TranslateMessage(&hMsg);
            DispatchMessageA(&hMsg);
        }

        const std::string &s = GetInfoAsString();
        CWin32TrayIcon::UpdateTip(&TrayIcon, s.c_str());

        if (g_ShowStatistics)
        {
            CScreenUtils::ClearBuffer(&g_BackBuffer, 0.0f, 0.0f, 0.0f);
            s_window_dimensions d = CScreenUtils::GetWindowDimension(hWnd);

            g_KMCounterStatistics[g_KMCounterStatistics.size() - 1] = g_KMCounter;
            float MaxHits = 0.0f, StartX = 0.0f, space = (float)d.Width / (float)g_KMCounterStatistics.size();

            for (size_t i = 0; i < g_KMCounterStatistics.size(); ++i)
            {
                float f = (float)GetTotalHits(g_KMCounterStatistics[i]);
                MaxHits = f < MaxHits ? MaxHits : f;
            }

            for (size_t i = 0; i < g_KMCounterStatistics.size(); ++i)
            {
                float CurrentHeight = (float)GetTotalHits(g_KMCounterStatistics[i]) / MaxHits - 0.01f;

                s_point StartPoint{ StartX, d.Height - CurrentHeight * d.Height };
                s_point EndPoint{ StartX + space, (float) d.Height };

                float r = statisticsColors[i].r;
                float g = statisticsColors[i].g;
                float b = statisticsColors[i].b;

                CScreenUtils::DrawRectangle(&g_BackBuffer, StartPoint, EndPoint, r, g, b);
                StartX += space;
            }

            CScreenUtils::DisplayBuffer(&g_BackBuffer, DeviceContext, d.Width, d.Height);
        }

        // NOTE(Andrei): Don't waste CPU time
        Sleep(10);
    }

    DestroyWindow(hWnd);
    UnregisterClass(CLASS_NAME, GetModuleHandle(NULL));

    UnhookWindowsHookEx(KbdHook);
    UnhookWindowsHookEx(MouseHook);

    CWin32TrayIcon::DisableIcon(&TrayIcon);
    GetSystemTimeAsFileTime(&g_KMCounter.EndTime);

    s_file_content fileContent =
    {
        &g_KMCounter,
        sizeof(g_KMCounter)
    };

    CFileUtils::WriteFileContentIntoDisk(fileContent, "statistics.dat", CFileUtils::Append);
    return EXIT_SUCCESS;
}
