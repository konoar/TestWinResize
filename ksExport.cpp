/****************************************************

  ksExport.cpp
   Copyright 2019.09.01 konoar

 ****************************************************/

#include "ksExport.h"

void ksExport(ksRenderData* data, ksRenderFunc func, const wchar_t* filename)
{
    //
    // 実際の画面表示はタイトルバーの高さ分短いです
    //

    HDC hMetaDC, hScreenDC;

    RECT r;

    //
    // 0.01ミリ単位の出力サイズ
    //
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
        TEXT("OneUp\\0\0"));

    ReleaseDC(NULL, hScreenDC);

    if (!hMetaDC) {
        return;
    }

    SetMapMode(hMetaDC, MM_ANISOTROPIC);

    SetWindowExtEx(
        hMetaDC,
        data->screenW,
        data->screenH,
        NULL);

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

}
