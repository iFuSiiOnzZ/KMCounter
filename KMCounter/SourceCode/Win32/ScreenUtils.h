#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

typedef struct s_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;

    int Width;
    int Height;

    int Pitch;
    int BytesPerPixel;
} s_offscreen_buffer;

typedef struct  s_window_dimensions
{
    int Width;
    int Height;
} s_window_dimensions;

typedef struct s_point
{
    float x, y;
} s_point;

typedef struct s_color
{
    float r, g, b;
} s_color;

struct CScreenUtils
{
    static s_window_dimensions GetWindowDimension(HWND Window);

    static void ClearBuffer(s_offscreen_buffer *Buffer, float r, float g, float b);
    static void PutPixel(s_offscreen_buffer *Buffer, float x, float y, float r, float g, float b);

    static void DrawRectangle(s_offscreen_buffer *Buffer, s_point start, s_point end, float r, float g, float b);
    static void DrawLine(s_offscreen_buffer *Buffer, s_point start, s_point end, float r, float g, float b);

    static void ResizeBIBSection(s_offscreen_buffer *Buffer, int Width, int Height);
    static void DisplayBuffer(s_offscreen_buffer *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight);
};
