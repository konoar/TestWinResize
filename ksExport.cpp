/****************************************************

  ksExport.cpp
   Copyright 2019.09.01 konoar

 ****************************************************/

#include "ksExport.h"

void ksExport(ksRenderData* data, ksRenderFunc func, const wchar_t* filename)
{

    HDC hMetaDC, hScreenDC;

    RECT r;

    // 0.01ミリ単位の出力サイズ
    SetRect(&r, 0, 0, data->paperW * 100, data->paperH * 100);

    hScreenDC = GetDC(NULL);

    float pxX, pxY, mmX, mmY;

    pxX = (float)GetDeviceCaps(hScreenDC, HORZRES);
    pxY = (float)GetDeviceCaps(hScreenDC, VERTRES);
    mmX = (float)GetDeviceCaps(hScreenDC, HORZSIZE);
    mmY = (float)GetDeviceCaps(hScreenDC, VERTSIZE);

    hMetaDC = CreateEnhMetaFile(
        NULL,
        filename,
        &r,
        TEXT("TestWinResize.exe\0")
        TEXT("OneUp\\0\0"));

    ReleaseDC(NULL, hScreenDC);

    if (!hMetaDC) {
        return;
    }

    SetMapMode(hMetaDC, MM_ANISOTROPIC);

    SetWindowExtEx(
        hMetaDC,
        data->screenW,
        data->screenW,
        NULL);

    SetViewportExtEx(
        hMetaDC,
        (float)data->paperW * (pxX / mmX),
        (float)data->paperH * (pxY / mmY),
        NULL
    );

    func(hMetaDC, data);

    DeleteEnhMetaFile(
        CloseEnhMetaFile(hMetaDC)
    );

}
