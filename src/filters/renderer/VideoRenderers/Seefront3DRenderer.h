/*
 * (C) 2013, 2017 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * This class is added 9/18/2013 by Stefan de Bruijn, info@sdebruijn.nl .
 *
 * Seefront 3D uses face tracking to interlace textures on the screen.
 */

#pragma once

#include "Base3DRenderer.h"

namespace DSObjects
{
    class Seefront3DRenderer : public Base3DRenderer
    {

    private:
        // All the seefront calls that are bound to the seefront DLL.
        typedef int (__cdecl* getMinorVersionNumberFunc)();
        typedef int (__cdecl* getMajorVersionNumberFunc)();
        typedef int (__cdecl* getBuildNumberFunc)();
        typedef int (__cdecl* checkInstanceFunc)(void*);
        typedef int (__cdecl* isCalibratedFunc)(void);
        typedef int (__cdecl* showSetupDialogFunc)(void*, void*);
        typedef void* (__cdecl* createInstanceFunc)(struct IDirect3DDevice9*);
        typedef void (__cdecl* destroyFunc)(void*);

        typedef void (__cdecl* setTrackerCallbackFunc)(void*, void (__cdecl*)(double, double, double, void*), void*, char const*, void*);
        typedef void (__cdecl* setTrackerCallbackExFunc)(void*, void (__cdecl*)(double, double, double, int, void*), void*);
        typedef void (__cdecl* startTrackerUpdateFunc)(void*);
        typedef void (__cdecl* stopTrackerUpdateFunc)(void*);

        typedef void (__cdecl* moveFunc)(void*, int, int);
        typedef void (__cdecl* releaseFunc)(void*);
        typedef void (__cdecl* renderFunc)(void*);
        typedef void (__cdecl* render_without_syncFunc)(void*);
        typedef void (__cdecl* resizeFunc)(void*, int, int);
        typedef void (__cdecl* setRepaintCallbackFunc)(void*, void (__cdecl*)(void*), void*, char const*, void*);
        typedef void (__cdecl* setScreenFunc)(void*, int, int, int, int); //x, y, width, height
        typedef void (__cdecl* setTexturesFunc)(void*, struct IDirect3DTexture9**, unsigned int, float, float, float, float);
        typedef void (__cdecl* setTextureSizeFunc)(void*, int, int);
        typedef void (__cdecl* syncFunc)(void*);

        getMinorVersionNumberFunc getMinorVersionNumber;
        getMajorVersionNumberFunc getMajorVersionNumber;
        getBuildNumberFunc getBuildNumber;
        checkInstanceFunc checkInstance;
        isCalibratedFunc isCalibrated;
        showSetupDialogFunc showSetupDialog;
        createInstanceFunc createInstance;
        destroyFunc destroyInstance;
        moveFunc move;
        releaseFunc release;
        renderFunc render;
        render_without_syncFunc render_without_sync;
        resizeFunc resize;
        setRepaintCallbackFunc setRepaintCallback;
        setScreenFunc setScreen;
        setTexturesFunc setTextures;
        setTextureSizeFunc setTextureSize;
        setTrackerCallbackExFunc setTrackerCallback;
        startTrackerUpdateFunc startTrackerUpdate;
        stopTrackerUpdateFunc stopTrackerUpdate;
        syncFunc sync;

        HINSTANCE hGetProcIDDLL;
        void* seefront_data;

    public:
        Seefront3DRenderer(IDirect3DDevice9* device, const CSize& monitorSize);
        virtual void Initialize3DDevice(IDirect3DDevice9* device, const CSize& monitorSize);
        virtual void SetWindowSize(int left, int top, int width, int height);
        virtual void RenderFrame(IDirect3DTexture9** textures, int firstTextureIndex);
        virtual void UpdateTextureSize(int textureWidth, int textureHeight);

        virtual ~Seefront3DRenderer();
    };
}
