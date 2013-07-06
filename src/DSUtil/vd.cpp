//  VirtualDub - Video processing and capture application
//  Copyright (C) 1998-2001 Avery Lee
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  Notes:
//  - BitBltFromI420ToRGB is from VirtualDub
//  - The core assembly function of CCpuID is from DVD2AVI
//  - sse2 yv12 to yuy2 conversion by Haali
//  (- vd.cpp/h should be renamed to something more sensible already :)


#include "stdafx.h"
#include "vd.h"
#include "vd_asm.h"
#include <intrin.h>

#pragma warning(disable : 4799) // no emms... blahblahblah

CCpuID g_cpuid;

CCpuID::CCpuID()
{
    int CPUInfo[4] = {-1};
    __cpuid(CPUInfo, 1);
    int t = CPUInfo[3];

    int mflags = 0;
    mflags |= ((t&0x00800000)!=0) ? mmx : 0;            // STD MMX
    mflags |= ((t&0x02000000)!=0) ? ssemmx+ssefpu : 0;  // STD SSE
    mflags |= ((t&0x04000000)!=0) ? sse2 : 0;           // SSE2

    t = CPUInfo[2];
    mflags |= ((t&0x00000001)!=0) ? sse3 : 0;           // SSE3

    // 3dnow
    __cpuid(CPUInfo, 0x80000001);
    t = CPUInfo[3];
    mflags |= ((t&0x80000000)!=0) ? _3dnow : 0;         // 3D NOW
    mflags |= ((t&0x00400000)!=0) ? ssemmx : 0;         // SSE MMX

    // result
    m_flags = (flag_t)mflags;
}

void memcpy_accel(void* dst, const void* src, size_t len)
{
#ifndef _WIN64
    if((g_cpuid.m_flags & CCpuID::ssefpu) && len >= 128
        && !((DWORD)src&15) && !((DWORD)dst&15))
    {
        __asm
        {
            mov     esi, dword ptr [src]
            mov     edi, dword ptr [dst]
            mov     ecx, len
            shr     ecx, 7
    memcpy_accel_sse_loop:
            prefetchnta [esi+16*8]
            movaps      xmm0, [esi]
            movaps      xmm1, [esi+16*1]
            movaps      xmm2, [esi+16*2]
            movaps      xmm3, [esi+16*3]
            movaps      xmm4, [esi+16*4]
            movaps      xmm5, [esi+16*5]
            movaps      xmm6, [esi+16*6]
            movaps      xmm7, [esi+16*7]
            movntps     [edi], xmm0
            movntps     [edi+16*1], xmm1
            movntps     [edi+16*2], xmm2
            movntps     [edi+16*3], xmm3
            movntps     [edi+16*4], xmm4
            movntps     [edi+16*5], xmm5
            movntps     [edi+16*6], xmm6
            movntps     [edi+16*7], xmm7
            add         esi, 128
            add         edi, 128
            dec         ecx
            jne         memcpy_accel_sse_loop
            mov     ecx, len
            and     ecx, 127
            cmp     ecx, 0
            je      memcpy_accel_sse_end
    memcpy_accel_sse_loop2:
            mov     dl, byte ptr[esi]
            mov     byte ptr[edi], dl
            inc     esi
            inc     edi
            dec     ecx
            jne     memcpy_accel_sse_loop2
    memcpy_accel_sse_end:
            emms
            sfence
        }
    }
    else if((g_cpuid.m_flags & CCpuID::mmx) && len >= 64
        && !((DWORD)src&7) && !((DWORD)dst&7))
    {
        __asm
        {
            mov     esi, dword ptr [src]
            mov     edi, dword ptr [dst]
            mov     ecx, len
            shr     ecx, 6
    memcpy_accel_mmx_loop:
            movq    mm0, qword ptr [esi]
            movq    mm1, qword ptr [esi+8*1]
            movq    mm2, qword ptr [esi+8*2]
            movq    mm3, qword ptr [esi+8*3]
            movq    mm4, qword ptr [esi+8*4]
            movq    mm5, qword ptr [esi+8*5]
            movq    mm6, qword ptr [esi+8*6]
            movq    mm7, qword ptr [esi+8*7]
            movq    qword ptr [edi], mm0
            movq    qword ptr [edi+8*1], mm1
            movq    qword ptr [edi+8*2], mm2
            movq    qword ptr [edi+8*3], mm3
            movq    qword ptr [edi+8*4], mm4
            movq    qword ptr [edi+8*5], mm5
            movq    qword ptr [edi+8*6], mm6
            movq    qword ptr [edi+8*7], mm7
            add     esi, 64
            add     edi, 64
            loop    memcpy_accel_mmx_loop
            mov     ecx, len
            and     ecx, 63
            cmp     ecx, 0
            je      memcpy_accel_mmx_end
    memcpy_accel_mmx_loop2:
            mov     dl, byte ptr [esi]
            mov     byte ptr [edi], dl
            inc     esi
            inc     edi
            dec     ecx
            jne     memcpy_accel_mmx_loop2
    memcpy_accel_mmx_end:
            emms
        }
    }
    else
#endif
    {
        memcpy(dst, src, len);
    }
}

static void yuvtoyuy2row_c(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width)
{
    WORD* dstw = (WORD*)dst;
    for(; width > 1; width -= 2)
    {
        *dstw++ = (*srcu++<<8)|*srcy++;
        *dstw++ = (*srcv++<<8)|*srcy++;
    }
}

static void yuvtoyuy2row_avg_c(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv)
{
    WORD* dstw = (WORD*)dst;
    for(; width > 1; width -= 2, srcu++, srcv++)
    {
        *dstw++ = (((srcu[0]+srcu[pitchuv])>>1)<<8)|*srcy++;
        *dstw++ = (((srcv[0]+srcv[pitchuv])>>1)<<8)|*srcy++;
    }
}

static void asm_blend_row_clipped_c(BYTE* dst, BYTE* src, DWORD w, DWORD srcpitch)
{
    BYTE* src2 = src + srcpitch;
    do
    {
        *dst++ = (*src++ + *src2++ + 1) >> 1;
    } while(w--);
}

static void asm_blend_row_c(BYTE* dst, BYTE* src, DWORD w, DWORD srcpitch)
{
    BYTE* src2 = src + srcpitch;
    BYTE* src3 = src2 + srcpitch;
    do
    {
        *dst++ = (*src++ + (*src2++ << 1) + *src3++ + 2) >> 2;
    } while(w--);
}

bool BitBltFromI420ToI420(int w, int h, BYTE* dsty, BYTE* dstu, BYTE* dstv, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if((w&1))
        return(false);

    if(w > 0 && w == srcpitch && w == dstpitch)
    {
        memcpy_accel(dsty, srcy, h*srcpitch);
        memcpy_accel(dstu, srcu, h/2*srcpitch/2);
        memcpy_accel(dstv, srcv, h/2*srcpitch/2);
    }
    else
    {
        int pitch = min(abs(srcpitch), abs(dstpitch));

        for(ptrdiff_t y = 0; y < h; y++, srcy += srcpitch, dsty += dstpitch)
            memcpy_accel(dsty, srcy, pitch);

        srcpitch >>= 1;
        dstpitch >>= 1;

        pitch = min(abs(srcpitch), abs(dstpitch));

        for(ptrdiff_t y = 0; y < h; y+=2, srcu += srcpitch, dstu += dstpitch)
            memcpy_accel(dstu, srcu, pitch);

        for(ptrdiff_t y = 0; y < h; y+=2, srcv += srcpitch, dstv += dstpitch)
            memcpy_accel(dstv, srcv, pitch);
    }

    return(true);
}

bool BitBltFromYUY2ToYUY2(int w, int h, BYTE* dst, int dstpitch, BYTE* src, int srcpitch)
{
    if(w > 0 && w == srcpitch && w == dstpitch)
    {
        memcpy_accel(dst, src, h*srcpitch);
    }
    else
    {
        int pitch = min(abs(srcpitch), abs(dstpitch));

        for(ptrdiff_t y = 0; y < h; y++, src += srcpitch, dst += dstpitch)
            memcpy_accel(dst, src, pitch);
    }

    return(true);
}

#ifndef _WIN64
extern "C" void asm_YUVtoRGB32_row(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB24_row(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB16_row(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB32_row_MMX(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB24_row_MMX(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB16_row_MMX(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB32_row_ISSE(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB24_row_ISSE(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
extern "C" void asm_YUVtoRGB16_row_ISSE(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width);
#endif

bool BitBltFromI420ToRGB(int w, int h, BYTE* dst, int dstpitch, int dbpp, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch)
{
    if(w<=0 || h<=0 || (w&1) || (h&1))
        return(false);

#ifndef _WIN64
    void (*asm_YUVtoRGB_row)(void* ARGB1, void* ARGB2, BYTE* Y1, BYTE* Y2, BYTE* U, BYTE* V, long width) = NULL;;

    if((g_cpuid.m_flags & CCpuID::ssefpu) && !(w&7))
    {
        switch(dbpp)
        {
        case 16: asm_YUVtoRGB_row = asm_YUVtoRGB16_row/*_ISSE*/; break; // TODO: fix _ISSE (555->565)
        case 24: asm_YUVtoRGB_row = asm_YUVtoRGB24_row_ISSE; break;
        case 32: asm_YUVtoRGB_row = asm_YUVtoRGB32_row_ISSE; break;
        }
    }
    else if((g_cpuid.m_flags & CCpuID::mmx) && !(w&7))
    {
        switch(dbpp)
        {
        case 16: asm_YUVtoRGB_row = asm_YUVtoRGB16_row/*_MMX*/; break; // TODO: fix _MMX (555->565)
        case 24: asm_YUVtoRGB_row = asm_YUVtoRGB24_row_MMX; break;
        case 32: asm_YUVtoRGB_row = asm_YUVtoRGB32_row_MMX; break;
        }
    }
    else
    {
        switch(dbpp)
        {
        case 16: asm_YUVtoRGB_row = asm_YUVtoRGB16_row; break;
        case 24: asm_YUVtoRGB_row = asm_YUVtoRGB24_row; break;
        case 32: asm_YUVtoRGB_row = asm_YUVtoRGB32_row; break;
        }
    }

    if(!asm_YUVtoRGB_row)
        return(false);

    do
    {
        asm_YUVtoRGB_row(dst + dstpitch, dst, srcy + srcpitch, srcy, srcu, srcv, w/2);

        dst += 2*dstpitch;
        srcy += srcpitch*2;
        srcu += srcpitch/2;
        srcv += srcpitch/2;
    }
    while(h -= 2);

    if(g_cpuid.m_flags & CCpuID::mmx)
        __asm emms

    if(g_cpuid.m_flags & CCpuID::ssefpu)
        __asm sfence

    return(true);
#else
    ASSERT(FALSE);
    return(false);
#endif
}

bool BitBltFromI420ToYUY2(int w, int h, BYTE* dst, int dstpitch, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch, bool fInterlaced)
{
    if(w<=0 || h<=0 || (w&1) || (h&1))
        return(false);

    if(srcpitch == 0) srcpitch = w;

    void (*yuvtoyuy2row)(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width) = NULL;
    void (*yuvtoyuy2row_avg)(BYTE* dst, BYTE* srcy, BYTE* srcu, BYTE* srcv, DWORD width, DWORD pitchuv) = NULL;

#ifndef _WIN64
    if((g_cpuid.m_flags & CCpuID::sse2)
        && !((DWORD_PTR)srcy&15) && !((DWORD_PTR)srcu&15) && !((DWORD_PTR)srcv&15) && !(srcpitch&31)
        && !((DWORD_PTR)dst&15) && !(dstpitch&15))
    {
        if(!fInterlaced) yv12_yuy2_sse2(srcy, srcu, srcv, srcpitch/2, w/2, h, dst, dstpitch);
        else yv12_yuy2_sse2_interlaced(srcy, srcu, srcv, srcpitch/2, w/2, h, dst, dstpitch);
        return(true);
    }
    else
    {
        ASSERT(!fInterlaced);
    }

    if((g_cpuid.m_flags & CCpuID::mmx) && !(w&7))
    {
        yuvtoyuy2row = yuvtoyuy2row_MMX;
        yuvtoyuy2row_avg = yuvtoyuy2row_avg_MMX;
    }
    else
#endif
    {
        yuvtoyuy2row = yuvtoyuy2row_c;
        yuvtoyuy2row_avg = yuvtoyuy2row_avg_c;
    }

    if(!yuvtoyuy2row)
        return(false);

    do
    {
        yuvtoyuy2row(dst, srcy, srcu, srcv, w);
        yuvtoyuy2row_avg(dst + dstpitch, srcy + srcpitch, srcu, srcv, w, srcpitch/2);

        dst += 2*dstpitch;
        srcy += srcpitch*2;
        srcu += srcpitch/2;
        srcv += srcpitch/2;
    }
    while((h -= 2) > 2);

    yuvtoyuy2row(dst, srcy, srcu, srcv, w);
    yuvtoyuy2row(dst + dstpitch, srcy + srcpitch, srcu, srcv, w);

#ifndef _WIN64
    if(g_cpuid.m_flags & CCpuID::mmx)
        __asm emms
#endif

    return(true);
}

bool BitBltFromRGBToRGB(int w, int h, BYTE* dst, int dstpitch, int dbpp, BYTE* src, int srcpitch, int sbpp)
{
    if(dbpp == sbpp)
    {
        int rowbytes = w*dbpp>>3;

        if(rowbytes > 0 && rowbytes == srcpitch && rowbytes == dstpitch)
        {
            memcpy_accel(dst, src, h*rowbytes);
        }
        else
        {
            for(ptrdiff_t y = 0; y < h; y++, src += srcpitch, dst += dstpitch)
                memcpy_accel(dst, src, rowbytes);
        }

        return(true);
    }

    if(sbpp != 16 && sbpp != 24 && sbpp != 32
    || dbpp != 16 && dbpp != 24 && dbpp != 32)
        return(false);

    if(dbpp == 16)
    {
        for(ptrdiff_t y = 0; y < h; y++, src += srcpitch, dst += dstpitch)
        {
            if(sbpp == 24)
            {
                BYTE* s = (BYTE*)src;
                WORD* d = (WORD*)dst;
                for(ptrdiff_t x = 0; x < w; x++, s+=3, d++)
                    *d = (WORD)(((*((DWORD*)s)>>8)&0xf800)|((*((DWORD*)s)>>5)&0x07e0)|((*((DWORD*)s)>>3)&0x1f));
            }
            else if(sbpp == 32)
            {
                DWORD* s = (DWORD*)src;
                WORD* d = (WORD*)dst;
                for(ptrdiff_t x = 0; x < w; x++, s++, d++)
                    *d = (WORD)(((*s>>8)&0xf800)|((*s>>5)&0x07e0)|((*s>>3)&0x1f));
            }
        }
    }
    else if(dbpp == 24)
    {
        for(ptrdiff_t y = 0; y < h; y++, src += srcpitch, dst += dstpitch)
        {
            if(sbpp == 16)
            {
                WORD* s = (WORD*)src;
                BYTE* d = (BYTE*)dst;
                for(ptrdiff_t x = 0; x < w; x++, s++, d+=3)
                {   // not tested, r-g-b might be in reverse
                    d[0] = (*s&0x001f)<<3;
                    d[1] = (*s&0x07e0)<<5;
                    d[2] = (*s&0xf800)<<8;
                }
            }
            else if(sbpp == 32)
            {
                BYTE* s = (BYTE*)src;
                BYTE* d = (BYTE*)dst;
                for(ptrdiff_t x = 0; x < w; x++, s+=4, d+=3)
                    {d[0] = s[0]; d[1] = s[1]; d[2] = s[2];}
            }
        }
    }
    else if(dbpp == 32)
    {
        for(ptrdiff_t y = 0; y < h; y++, src += srcpitch, dst += dstpitch)
        {
            if(sbpp == 16)
            {
                WORD* s = (WORD*)src;
                DWORD* d = (DWORD*)dst;
                for(ptrdiff_t x = 0; x < w; x++, s++, d++)
                    *d = ((*s&0xf800)<<8)|((*s&0x07e0)<<5)|((*s&0x001f)<<3);
            }
            else if(sbpp == 24)
            {
                BYTE* s = (BYTE*)src;
                DWORD* d = (DWORD*)dst;
                for(ptrdiff_t x = 0; x < w; x++, s+=3, d++)
                    *d = *((DWORD*)s)&0xffffff;
            }
        }
    }

    return(true);
}

void DeinterlaceBlend(BYTE* dst, BYTE* src, DWORD rowbytes, DWORD h, DWORD dstpitch, DWORD srcpitch)
{
    void (*blend_row_clipped)(BYTE* dst, BYTE* src, DWORD w, DWORD srcpitch) = NULL;
    void (*blend_row)(BYTE* dst, BYTE* src, DWORD w, DWORD srcpitch) = NULL;

#ifndef _WIN64
    if((g_cpuid.m_flags & CCpuID::sse2) && !((DWORD)src&0xf) && !((DWORD)dst&0xf) && !(srcpitch&0xf))
    {
        blend_row_clipped = asm_blend_row_clipped_SSE2;
        blend_row = asm_blend_row_SSE2;
    }
    else if(g_cpuid.m_flags & CCpuID::mmx)
    {
        blend_row_clipped = asm_blend_row_clipped_MMX;
        blend_row = asm_blend_row_MMX;
    }
    else
#endif
    {
        blend_row_clipped = asm_blend_row_clipped_c;
        blend_row = asm_blend_row_c;
    }

    if(!blend_row_clipped)
        return;

    blend_row_clipped(dst, src, rowbytes, srcpitch);

    if((h -= 2) > 0) do
    {
        dst += dstpitch;
        blend_row(dst, src, rowbytes, srcpitch);
        src += srcpitch;
    }
    while(--h);

    blend_row_clipped(dst + dstpitch, src, rowbytes, srcpitch);

#ifndef _WIN64
    if(g_cpuid.m_flags & CCpuID::mmx)
        __asm emms
#endif
}

#ifndef _WIN64
extern "C" void mmx_YUY2toRGB24(const BYTE* src, BYTE* dst, const BYTE* src_end, int src_pitch, int row_size, bool rec709);
extern "C" void mmx_YUY2toRGB32(const BYTE* src, BYTE* dst, const BYTE* src_end, int src_pitch, int row_size, bool rec709);
#endif

bool BitBltFromYUY2ToRGB(int w, int h, BYTE* dst, int dstpitch, int dbpp, BYTE* src, int srcpitch)
{
    void (* YUY2toRGB)(const BYTE* src, BYTE* dst, const BYTE* src_end, int src_pitch, int row_size, bool rec709) = NULL;

#ifndef _WIN64
    if(g_cpuid.m_flags & CCpuID::mmx)
    {
        YUY2toRGB =
            dbpp == 32 ? mmx_YUY2toRGB32 :
            dbpp == 24 ? mmx_YUY2toRGB24 :
            // dbpp == 16 ? mmx_YUY2toRGB16 : // TODO
            NULL;
    }
    else
#endif
    {
        ASSERT(FALSE);
        // TODO
    }

    if(!YUY2toRGB)
        return(false);

    YUY2toRGB(src, dst, src + h*srcpitch, srcpitch, w, false);

    return(true);
}
