/****************************************************

  ksExport.h
   Copyright 2019.09.01 konoar

 ****************************************************/

#ifndef __KS_EXPORT_H__
#define __KS_EXPORT_H__

#include <Windows.h>

struct ksRenderData
{

#define KS_RENDER_MAX 5

    long screenW;   // px
    long screenH;   // px

    long paperW;    // mm
    long paperH;    // mm

    struct
    {

        POINT pos;
        long  rad;

    } record[KS_RENDER_MAX];

};

typedef void (*ksRenderFunc)(HDC, ksRenderData*);

BOOL ksExport(
    ksRenderData* data, ksRenderFunc func, const wchar_t* filename);

#endif // __KS_EXPORT_H__
