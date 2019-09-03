/****************************************************

  ksMain.cpp
   Copyright 2019.09.01 konoar

 ****************************************************/

#include "ksExport.h"

#define KS_SCREEN_W  300
#define KS_SCREEN_H  300
#define KS_PAPER_W   500
#define KS_PAPER_H   500
#define KS_ID_BT    1000

void ksRender(HDC hdc, ksRenderData *d)
{

    HPEN    hpen, hpenbak;
    HBRUSH  hbrushbak;

    hpen        = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
    hpenbak     = (HPEN)    SelectObject(hdc, hpen);
    hbrushbak   = (HBRUSH)  SelectObject(hdc, GetStockObject(NULL_BRUSH));

    for (int idx = 0; idx < KS_RENDER_MAX; idx++) {
        Ellipse(
            hdc,
            d->record[idx].pos.x - d->record[idx].rad,
            d->record[idx].pos.y - d->record[idx].rad,
            d->record[idx].pos.x + d->record[idx].rad,
            d->record[idx].pos.y + d->record[idx].rad);
    }

    SelectObject(hdc, hpenbak);
    SelectObject(hdc, hbrushbak);
    DeleteObject(hpen);

}

LRESULT __stdcall WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{

    static ksRenderData d;
    static long c = 0;

    switch (msg) {

    case WM_CREATE:

        d.screenW = KS_SCREEN_W;
        d.screenH = KS_SCREEN_H;

        d.paperW  = KS_PAPER_W;
        d.paperH  = KS_PAPER_H;

        srand(GetTickCount());

        for (int idx = 0; idx < KS_RENDER_MAX; idx++) {
            d.record[idx].pos.x = (long)((double)KS_SCREEN_W * ((double)rand() / RAND_MAX));
            d.record[idx].pos.y = (long)((double)KS_SCREEN_H * ((double)rand() / RAND_MAX));
            d.record[idx].rad   = (long)(50.0 * ((double)rand() / RAND_MAX)) + 10;
        }

        CreateWindowEx(
            0,
            TEXT("BUTTON"),
            TEXT("エクスポート..."),
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            10,
            10,
            150,
            40,
            hwnd,
            (HMENU)KS_ID_BT,
#ifdef _WIN64
            (HINSTANCE)GetWindowLong(hwnd, GWLP_HINSTANCE),
#else
            (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
#endif
            NULL
        );

        return 0;

    case WM_LBUTTONDOWN:

        d.record[c].pos.x   = lp & 0x0000FFFF;
        d.record[c].pos.y   = lp >> 16;
        d.record[c].rad     = (long)(50.0 * ((double)rand() / RAND_MAX)) + 10;

        if (++c >= KS_RENDER_MAX) {
            c = 0;
        }

        InvalidateRect(hwnd, NULL, TRUE);

        return 0;

    case WM_COMMAND:

        switch (LOWORD(wp)) {

        case KS_ID_BT:
        {
            OPENFILENAME f  = { 0 };
            TCHAR p[MAX_PATH] = L"\0";

            f.lStructSize   = sizeof(OPENFILENAME);
            f.hwndOwner     = hwnd;
            f.lpstrFile     = p;
            f.nMaxFile      = MAX_PATH;
            f.Flags         = OFN_EXPLORER;
            f.lpstrFilter   =
                TEXT("EMF files {*.emf}\0*.emf\0")
                TEXT("All files {*.*}\0.*\0\0");
            f.nFilterIndex  = 0;

            if (GetSaveFileName(&f)) {
                ksExport(&d, ksRender, p);
            }

            break;
        }

        }

        return 0;

    case WM_PAINT:
    {

        PAINTSTRUCT ps;
        HDC         hdc;

        hdc = BeginPaint(hwnd, &ps); {

            ksRender(hdc, &d);

        } EndPaint(hwnd, &ps);

        return 0;

    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
        
    }

    return DefWindowProc(hwnd, msg, wp, lp);

}

int __stdcall WinMain(HINSTANCE hinst, HINSTANCE hprev, LPSTR cmd, int show)
{

    TCHAR appname[] = TEXT("TestResize");

    WNDCLASSEX app;

    app.cbSize          = sizeof(WNDCLASSEX);
    app.style           = CS_HREDRAW | CS_VREDRAW;
    app.lpfnWndProc     = WndProc;
    app.cbClsExtra      = 0;
    app.cbWndExtra      = 0;
    app.hInstance       = hinst;
    app.hIcon           = LoadIcon(NULL, IDI_APPLICATION);
    app.hCursor         = LoadCursor(NULL, IDC_ARROW);
    app.hbrBackground   = (HBRUSH)(COLOR_WINDOW + 1);
    app.lpszMenuName    = NULL;
    app.lpszClassName   = appname;
    app.hIconSm         = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassEx(&app);

    HWND hwnd;

    hwnd = CreateWindowEx(
        WS_EX_TOPMOST,
        appname,
        appname,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        KS_SCREEN_W,
        KS_SCREEN_H,
        NULL,
        NULL,
        hinst,
        NULL
    );

    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage (&msg);
    }

    return (int)msg.wParam;

}
