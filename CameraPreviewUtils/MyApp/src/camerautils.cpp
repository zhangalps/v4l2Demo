/*
 * This file is part of libhybris
 *
 * Copyright (C) 2019 Beijing Yuan Xin Technology Co.,Ltd. All rights reserved.
 *
 * Authors:
 *       Chen Xin <chenxin@syberos.com>
 *
 * This software, including documentation, is protected by copyright controlled
 * by Beijing Yuan Xin Technology Co.,Ltd. All rights are reserved.
 */


#include "camerautils.h"


//YUY2视频转化RGB格式
void YUY2_RGB(BYTE *YUY2buff,BYTE *RGBbuff, size_t dwSize)
{
    //B = 1.164(Y - 16)         + 2.018(U - 128)
    //G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
    //R = 1.164(Y - 16) + 1.596(V - 128)
    for(size_t count = 0;count < dwSize; count += 4)
    {
        //Y0 U0 Y1 V0
        float Y0 = *YUY2buff;
        float U = *(++YUY2buff);
        float Y1 = *(++YUY2buff);
        float V = *(++YUY2buff);
        ++YUY2buff;
        *(RGBbuff) =   (BYTE)(Y0 + (1.370705 * (V-128)));
        *(++RGBbuff) = (BYTE)(Y0 - (0.698001 * (V-128)) - (0.337633 * (U-128)));
        *(++RGBbuff) = (BYTE)(Y0 + (1.732446 * (U-128)));
        *(++RGBbuff) = (BYTE)(Y1 + (1.370705 * (V-128)));
        *(++RGBbuff) = (BYTE)(Y1 - (0.698001 * (V-128)) - (0.337633 * (U-128)));
        *(++RGBbuff) = (BYTE)(Y1 + (1.732446 * (U-128)));
        ++RGBbuff;
    }
}
