#include "ScreenUtils.h"

void CScreenUtils::ResizeBIBSection(s_offscreen_buffer *Buffer, int Width, int Height)
{
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biHeight = -Height;
    Buffer->Info.bmiHeader.biWidth = Width;

    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biPlanes = 1;

    Buffer->Height = Height;
    Buffer->Width = Width;

    Buffer->Pitch = Width * 4;
    Buffer->BytesPerPixel = 4;

    size_t BitmapMemorySize = (size_t)(4 * Width * Height);
    if (Buffer->Memory != NULL) VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void CScreenUtils::DisplayBuffer(s_offscreen_buffer *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    int  OffsetX = 0, OffsetY = 0;

    PatBlt(DeviceContext, 0, 0, WindowWidth, OffsetY, BLACKNESS);
    PatBlt(DeviceContext, 0, OffsetY + Buffer->Height, WindowWidth, WindowHeight, BLACKNESS);

    PatBlt(DeviceContext, 0, 0, OffsetX, WindowHeight, BLACKNESS);
    PatBlt(DeviceContext, OffsetX + Buffer->Width, 0, WindowWidth, WindowHeight, BLACKNESS);

    StretchDIBits
    (
        DeviceContext,

        OffsetX, OffsetY, Buffer->Width, Buffer->Height,
        0, 0, Buffer->Width, Buffer->Height,

        Buffer->Memory, &Buffer->Info,
        DIB_RGB_COLORS, SRCCOPY
    );
}

s_window_dimensions CScreenUtils::GetWindowDimension(HWND Window)
{
    RECT ClientRect = { 0 };
    GetClientRect(Window, &ClientRect);

    s_window_dimensions r = { 0 };
    r.Width = ClientRect.right - ClientRect.left;
    r.Height = ClientRect.bottom - ClientRect.top;

    return r;
}

void CScreenUtils::ClearBuffer(s_offscreen_buffer *Buffer, float r, float g, float b)
{
    unsigned int *Dest = ((unsigned int *)Buffer->Memory);
    unsigned int ClearColor = ((unsigned int)(r * 255.0f) << 16) | ((unsigned int)(g * 255.0f) << 8) | ((unsigned int)(b * 255.0f) << 0);

    for (int i = 0; i < Buffer->Height * Buffer->Width; ++i)
    {
        *Dest++ = ClearColor;
    }
}

void CScreenUtils::DrawRectangle(s_offscreen_buffer *Buffer, s_point start, s_point end, float r, float g, float b)
{
    int MinX = (int)(start.x + 0.5f);
    int MinY = (int)(start.y + 0.5f);

    int MaxX = (int)(end.x + 0.5f);
    int MaxY = (int)(end.y + 0.5f);

    if (MinX < 0) MinX = 0;
    if (MinY < 0) MinY = 0;

    if (MaxX > Buffer->Width) MaxX = Buffer->Width;
    if (MaxY > Buffer->Height) MaxY = Buffer->Height;

    unsigned int Color = ((unsigned int)(r * 255.0f)) << 16 | ((unsigned int)(g * 255.0f)) << 8 | ((unsigned int)(b * 255.0f)) << 0;
    unsigned char *Row = ((unsigned char *)Buffer->Memory + MinX * Buffer->BytesPerPixel + MinY * Buffer->Pitch);

    for (int Y = MinY; Y < MaxY; ++Y)
    {
        unsigned int *Pixel = (unsigned int *)Row;
        for (int X = MinX; X < MaxX; ++X)
        {
            *Pixel++ = Color;
        }

        Row += Buffer->Pitch;
    }
}

void CScreenUtils::DrawLine(s_offscreen_buffer *Buffer, s_point start, s_point end, float r, float g, float b)
{
    int MinX = (int)(start.x + 0.5f);
    int MinY = (int)(start.y + 0.5f);

    int MaxX = (int)(end.x + 0.5f);
    int MaxY = (int)(end.y + 0.5f);

    if (MinX < 0) MinX = 0;
    if (MinY < 0) MinY = 0;

    if (MaxX > Buffer->Width) MaxX = Buffer->Width;
    if (MaxY > Buffer->Height) MaxY = Buffer->Height;

    int dx, dy, p, x, y;

    dx = MaxX - MinX;
    dy = MaxY - MinY;

    x = MinX;
    y = MinY;

    p = 2 * dy - dx;

    while (x < MaxX)
    {
        if (p >= 0)
        {
            PutPixel(Buffer, (float) x, (float) y, r, g, b);
            y = y + 1;
            p = p + 2 * dy - 2 * dx;
        }
        else
        {
            PutPixel(Buffer, (float) x, (float) y, r, g, b);
            p = p + 2 * dy;
        }

        x = x + 1;
    }
}

void CScreenUtils::PutPixel(s_offscreen_buffer * Buffer, float x, float y, float r, float g, float b)
{
    int xx = (int)(x + 0.5f);
    int yy = (int)(y + 0.5f);

    if (xx < 0) return;
    if (yy < 0) return;

    if (xx >= Buffer->Width) return;
    if (yy >= Buffer->Height) return;

    unsigned int Color = ((unsigned int)(r * 255.0f)) << 16 | ((unsigned int)(g * 255.0f)) << 8 | ((unsigned int)(b * 255.0f)) << 0;
    unsigned char *Row = ((unsigned char *)Buffer->Memory + xx * Buffer->BytesPerPixel + yy * Buffer->Pitch);

    unsigned int *Pixel = (unsigned int *)Row;
    *Pixel = Color;
}
