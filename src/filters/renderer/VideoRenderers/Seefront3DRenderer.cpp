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
 */

#include "stdafx.h"
#include "Seefront3DRenderer.h"

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#define DLL2_EXPORTS
#include <stdio.h>

namespace DSObjects
{
    void TrackingCallback(double eyeXpos, double eyeYpos, double eyeZpos, int hasReliableResult, void* thisptr)
    {
        // For now we ignore this tracking data; it doesn't seem to be needed by the API. It's fun to watch though ;-)
    }

    Seefront3DRenderer::Seefront3DRenderer(IDirect3DDevice9* device, const CSize& monitorSize)
        : Base3DRenderer(device, monitorSize)
    {}

    void Seefront3DRenderer::Initialize3DDevice(IDirect3DDevice9* device, const CSize& monitorSize)
    {
        // Get the DLL from the registry.

        CRegKey registryKey;
        if (ERROR_SUCCESS != registryKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\seefront\\seefront_runtime_dx"))) {
            // TODO FIXME: Show a popup message box or something like that: seefront is not installed.
            TRACE(_T("Cannot find seefront runtime DX in registry; should be in HKLM\\SOFTWARE\\seefront\\seefront_runtime_dx"));
            registryKey.Close();

            return; // SESE violation... yeah...
        }

        WCHAR libraryFullPath[2048];
        ULONG fullPathBufferSize = 2048;
        if (ERROR_SUCCESS != registryKey.QueryStringValue(_T("location"), libraryFullPath, &fullPathBufferSize)) {
            // TODO FIXME: Show a popup message box or something like that: seefront is not installed.
            TRACE(_T("Cannot find seefront runtime DX location in registry; should be in HKLM\\SOFTWARE\\seefront\\seefront_runtime_dx"));
            registryKey.Close();

            return; // SESE violation... yeah...
        }

        // Load the DLL
        hGetProcIDDLL = ::LoadLibrary(libraryFullPath);

        // Define the Function in the DLL for reuse. This is just prototyping the dll's function.
        getMajorVersionNumber = getMajorVersionNumberFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "getMajorVersionNumber"));
        getMinorVersionNumber = getMinorVersionNumberFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "getMinorVersionNumber"));
        getBuildNumber = getBuildNumberFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "getBuildNumber"));
        isCalibrated = isCalibratedFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_isCalibrated"));

        createInstance = createInstanceFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_createInstance"));
        destroyInstance = destroyFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_destroy"));
        checkInstance = checkInstanceFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_checkInstance"));

        showSetupDialog = showSetupDialogFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_showSetupDialog"));

        startTrackerUpdate = startTrackerUpdateFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_startTrackerUpdate"));
        stopTrackerUpdate = stopTrackerUpdateFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_stopTrackerUpdate"));

        setTrackerCallback = setTrackerCallbackExFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_setTrackerCallbackEx"));

        move = moveFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_move"));
        release = releaseFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_release"));
        render = renderFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_render"));
        render_without_sync = render_without_syncFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_render_without_sync"));
        resize = resizeFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_resize"));
        setRepaintCallback = setRepaintCallbackFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_setRepaintCallback"));
        setScreen = setScreenFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_setScreen"));
        setTextures = setTexturesFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_setTextures"));
        setTextureSize = setTextureSizeFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_setTextureSize"));
        sync = syncFunc(GetProcAddress(HMODULE(hGetProcIDDLL), "sfdx_sync"));

        // Initializes the seefront class
        this->seefront_data = createInstance(device);

        // Initialize tracker; the last 0 is for 'user data', which we don't really need.
        setTrackerCallback(seefront_data, TrackingCallback, this);

        // Start the eye tracker
        startTrackerUpdate(seefront_data);

        // Set the screen size
        setScreen(this->seefront_data, 0, 0, monitorSize.cx, monitorSize.cy);
    }

    void Seefront3DRenderer::SetWindowSize(int left, int top, int width, int height)
    {
        move(this->seefront_data, left, top);
        resize(this->seefront_data, width, height);
    }

    void Seefront3DRenderer::RenderFrame(IDirect3DTexture9** textures, int firstTextureIndex)
    {
        setTextures(this->seefront_data, textures, firstTextureIndex, 0, 0, 1, 1);
        render_without_sync(this->seefront_data);
    }

    void Seefront3DRenderer::UpdateTextureSize(int textureWidth, int textureHeight)
    {
        setTextureSize(this->seefront_data, textureWidth, textureHeight);
    }

    Seefront3DRenderer::~Seefront3DRenderer()
    {
        // Destruct
        if (this->seefront_data) {
            // Stop the tracker
            this->stopTrackerUpdate(this->seefront_data);

            // Destroy the thing
            this->destroyInstance(this->seefront_data);

            // Release
            this->release(this->seefront_data);

            // Done, ensure it's gone for good.
            this->seefront_data = 0;
        }

        // Release the DLL
        FreeLibrary(hGetProcIDDLL);
    }
}
