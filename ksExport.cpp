/****************************************************

  ksExport.cpp
   Copyright 2019.09.01 konoar

 ****************************************************/

#include "ksExport.h"

void ksExport(ksRenderData* data, ksRenderFunc func, const wchar_t* filename)
{
    RECT r = { 0, 0, 0, 0 };

    TCHAR desc[] =
        TEXT("TestWinResize\0")
        TEXT("OneUp\\0\0");

    HDC hMetaDC, hScreenDC;
    float pxX, pxY, mmX, mmY;

    // 30cm
    SetRect(&r, 0, 0, data->width * 100, data->height * 100);

    hScreenDC = GetDC(NULL);

    pxX = (float)GetDeviceCaps(hScreenDC, HORZRES);
    pxY = (float)GetDeviceCaps(hScreenDC, VERTRES);
    mmX = (float)GetDeviceCaps(hScreenDC, HORZSIZE);
    mmY = (float)GetDeviceCaps(hScreenDC, VERTSIZE);

    hMetaDC = CreateEnhMetaFile(hScreenDC, filename, &r, desc);

    ReleaseDC(NULL, hScreenDC);

    if (!hMetaDC) {
        return;
    }

    SetMapMode(hMetaDC, MM_ANISOTROPIC);

    SetWindowExtEx(
        hMetaDC,
        data->width,
        data->height,
        NULL);

    SetViewportExtEx(
        hMetaDC,
        data->width  * (pxX / mmX),
        data->height * (pxY / mmY),
        NULL
    );

    func(hMetaDC, data);

    CloseEnhMetaFile(hMetaDC);

}
