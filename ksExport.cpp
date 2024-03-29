/****************************************************

  ksExport.cpp
   Copyright 2019.09.01 konoar

 ****************************************************/

#include "ksExport.h"
#include "wchar.h"

static BOOL ksGetTempFilename(wchar_t* filename, int max)
{

    DWORD len;
    wchar_t buff1[MAX_PATH], buff2[MAX_PATH];

    if (0 == GetTempPath(MAX_PATH, buff1)) {
        return FALSE;
    }

    if (0 == GetTempFileName(buff1, L"KS=", 0, buff2)) {
        return FALSE;
    }

    if ((len = wcslen(buff2)) > (max - 1)) {
        return FALSE;
    }

    memcpy(filename, buff2, sizeof(wchar_t) * (len + 1));

    return TRUE;

}

static BOOL ksMakeEMF(
    ksRenderData* data, ksRenderFunc func, const wchar_t* filename)
{

    HDC hMetaDC, hScreenDC;

    //
    // 0.01ミリ単位の出力サイズ
    //
    // 実際の画面表示はタイトルバーの高さ分短いです
    //

    RECT r;

    SetRect(&r, 0, 0, data->paperW * 100, data->paperH * 100);

    hScreenDC = GetDC(NULL);

    double pxX, pxY, mmX, mmY;

    pxX = (double)GetDeviceCaps(hScreenDC, HORZRES);
    pxY = (double)GetDeviceCaps(hScreenDC, VERTRES);
    mmX = (double)GetDeviceCaps(hScreenDC, HORZSIZE);
    mmY = (double)GetDeviceCaps(hScreenDC, VERTSIZE);

    hMetaDC = CreateEnhMetaFile(
        NULL,
        filename,
        &r,
        TEXT("TestWinResize.exe\0")
        TEXT("OneUp\0\0")
    );

    ReleaseDC(NULL, hScreenDC);

    if (!hMetaDC) {
        return FALSE;
    }

    SetMapMode(hMetaDC, MM_ANISOTROPIC);

    SetWindowExtEx(
        hMetaDC,
        data->screenW,
        data->screenH,
        NULL
    );

    SetViewportExtEx(
        hMetaDC,
        (int)((double)data->paperW * (pxX / mmX)),
        (int)((double)data->paperH * (pxY / mmY)),
        NULL
    );

    func(hMetaDC, data);

    DeleteEnhMetaFile(
        CloseEnhMetaFile(hMetaDC)
    );

    return TRUE;

}

static BOOL ksGetEMFRect(const wchar_t* filename, RECT* r)
{

    HENHMETAFILE fh;
    DWORD   sz      = 0;
    LPVOID  dt      = NULL;
    BOOL    retval  = TRUE;

    if (NULL == (fh = GetEnhMetaFile(filename))) {
        return FALSE;
    }

    do {

        if (0 == (sz = GetEnhMetaFileHeader(fh, 0, NULL))) {
            retval = FALSE;
            break;
        }

        dt = malloc(sz);

        if (0 == (sz = GetEnhMetaFileHeader(fh, sz, (LPENHMETAHEADER)dt))) {
            retval = FALSE;
            break;
        }

        memcpy(r, &((LPENHMETAHEADER)dt)->rclFrame, sizeof(RECT));

    } while (0);

    if (dt) {
        free(dt);
        dt = NULL;
    }

    DeleteEnhMetaFile(fh);

    return retval;

}

static BOOL ksDrawEMF(
    HDC hdc, const wchar_t* filename, int x, int y, int width, int height)
{

    HENHMETAFILE fh;
    HRGN hrgn;
    BOOL retval = TRUE;
    double pxX, pxY, mmX, mmY;

    pxX = (double)GetDeviceCaps(hdc, HORZRES);
    pxY = (double)GetDeviceCaps(hdc, VERTRES);
    mmX = (double)GetDeviceCaps(hdc, HORZSIZE);
    mmY = (double)GetDeviceCaps(hdc, VERTSIZE);

    SetMapMode(hdc, MM_ANISOTROPIC);

    SetWindowExtEx(hdc, width, height, NULL);

    SetViewportExtEx(
        hdc,
        (int)((double)width  * (pxX / mmX) * 0.01),
        (int)((double)height * (pxY / mmY) * 0.01),
        NULL
    );

    if (NULL == (fh = GetEnhMetaFile(filename))) {
        return FALSE;
    }

    do {

        RECT r;
        SetRect(&r, x, y, (x + width), (y + height));

        if (NULL == (hrgn = CreateRectRgn(
            (int)((double)(x         ) * (pxX / mmX) * 0.01),
            (int)((double)(y         ) * (pxY / mmY) * 0.01),
            (int)((double)(x + width ) * (pxX / mmX) * 0.01),
            (int)((double)(y + height) * (pxY / mmY) * 0.01)))) {
            retval = FALSE;
            break;
        }

        SelectClipRgn(hdc, hrgn); {

            PlayEnhMetaFile(hdc, fh, &r);

        } SelectClipRgn(hdc, NULL);

    } while (0);

    if (hrgn) {
        DeleteObject(hrgn);
    }

    DeleteEnhMetaFile(fh);

    return retval;

}

static BOOL ksDrawFrame(HDC hdc, int x, int y, int width, int height)
{

    HPEN    hpen, hpenbak;
    HBRUSH  hbrushbak;

    hpen        = CreatePen(PS_SOLID, 50, RGB(0, 0, 0));
    hpenbak     =   (HPEN)SelectObject(hdc, hpen);
    hbrushbak   = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

    Rectangle(
        hdc,
        x - 50,
        y - 50,
        x + width  + 100,
        y + height + 100
    );

    SelectObject(hdc, hpenbak);
    SelectObject(hdc, hbrushbak);
    DeleteObject(hpen);

    return TRUE;

}

static BOOL ksDrawPageNo(HDC hdc, int x, int y, int no)
{

    HFONT hfnt, hfntbak;
    wchar_t buff[16];

    hfnt = CreateFont(
        300, 0, 0, 0,
        FW_LIGHT, 0, 0, 0,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH,
        L"ＭＳ ゴシック"
    );

    if (0 == hfnt) {
        return FALSE;
    }

    SetBkMode   (hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));

    hfntbak = (HFONT)SelectObject(hdc, hfnt); {

        if (0 < swprintf(buff, 16, L"ページ %d", no)) {

            RECT r;

            DrawText(
                hdc,
                buff,
                wcslen(buff),
                &r,
                DT_CALCRECT
            );

            TextOut(
                hdc,
                x - ((r.right  - r.left) / 2),
                y - ((r.bottom - r.top ) / 2),
                buff,
                wcslen(buff)
            );

        }

    } SelectObject(hdc, hfntbak);

    DeleteObject(hfnt);

    return TRUE;

}

static BOOL ksMake4Up(
    const wchar_t* filenamein, const wchar_t* filenameout)
{

    const int margin    = 10 * 100;
    const int margin_tp = margin / 10 *  6;
    const int margin_bt = margin / 10 * 14;

    RECT r;
    HDC hdc;

    if (!ksGetEMFRect(filenamein, &r)) {
        return FALSE;
    }

    RECT w[4] = {
        // ページ１ 左上
        {
            margin,
            margin_tp,
            r.right  / 2 - margin,
            r.bottom / 2 - margin_bt
        },
        // ページ２ 右上
        {
            r.right  / 2 + margin,
            margin_tp,
            r.right      - margin,
            r.bottom / 2 - margin_bt
        },
        // ページ３ 左下
        {
            margin,
            r.bottom / 2 + margin_tp,
            r.right  / 2 - margin,
            r.bottom     - margin_bt
        },
        // ページ４ 右下
        {
            r.right  / 2 + margin,
            r.bottom / 2 + margin_tp,
            r.right      - margin,
            r.bottom     - margin_bt
        }
    };

    hdc = CreateEnhMetaFile(
        NULL,
        filenameout,
        &r,
        TEXT("TestWinResize.exe\0")
        TEXT("FourUp\0\0")
    );

    if (!hdc) {
        return FALSE;
    }

    for (int idx = 0; idx < 4; idx++) {

        ksDrawEMF(
            hdc,
            filenamein,
            w[idx].left,
            w[idx].top,
            w[idx].right  - w[idx].left,
            w[idx].bottom - w[idx].top
        );

        ksDrawFrame(
            hdc,
            w[idx].left,
            w[idx].top,
            w[idx].right  - w[idx].left,
            w[idx].bottom - w[idx].top
        );

        ksDrawPageNo(
            hdc,
            w[idx].left + (w[idx].right - w[idx].left) / 2,
            w[idx].bottom + margin_bt / 2,
            idx + 1
        );

    }

    DeleteEnhMetaFile(
        CloseEnhMetaFile(hdc)
    );

    return TRUE;

}

BOOL ksExport(
    ksRenderData* data, ksRenderFunc func, const wchar_t* filename)
{

    wchar_t tempfile[MAX_PATH];

    if (!ksGetTempFilename(tempfile, MAX_PATH)) {
        return FALSE;
    }

    if (!ksMakeEMF(data, func, tempfile)) {
        return FALSE;
    }

    if (!ksMake4Up(tempfile, filename)) {
        return FALSE;
    }

    if (!DeleteFile(tempfile)) {
        return FALSE;
    }

    return TRUE;

}
