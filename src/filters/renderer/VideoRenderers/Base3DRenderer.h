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
 * This class is added 9/18/2013 by Stefan de Bruijn, info@sdebruijn.nl.
 *
 * There are a bunch of different 3D panels on the market, and they work more or less like this:
 *
 * - An image is split into 2 different images; one for your left eye and one for your right eye.
 * - The simplest movie is a SBS movie, where the left image is on the left-end size of the movie and the
 *   right image is in the right-end of the movie. By simply splitting the movie in the middle, you get 2 images.
 * - Some monitors have eye tracking or other stuff. The libraries need to know where the window is located
 *   on the monitor, the size of it and the library itself will probably take care of the rendering using DirectX.
 * - We render both images to a texture, which is passed in a ID3DTexture9*[2] to this class. The libraries use
 *   these textures and use something like alpha-blending to render them on the correct pixels on your screen.
 * - Size of the monitor is required for the library to figure out which pixels to use.
 *
 * NOTE: This is not the most efficient way to do this: scaling is often supported by the libraries, which means
 * we don't need the scaling to be done before passing it to the libraries. That said, it's probably fast enough
 * if you can affort a 3D screen in the first place.
 *
 * This class is a base class for 3D monitors. Implementing it is probably as simple as forwarding the calls
 * to the library in the virtual calls; the base class and presenter take case of the rest. See the SeeFront3DRenderer
 * as an example.
 */

#pragma once

#include "AllocatorCommon.h"
#include "RenderersSettings.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <afxwin.h>

namespace DSObjects
{
    class Base3DRenderer
    {
    private:
        IDirect3DTexture9** textures;
        IDirect3DSurface9** surfaces;

        int textureWidth;
        int textureHeight;

    protected:
        IDirect3DDevice9* device;

    public:
        Base3DRenderer(IDirect3DDevice9* device, const CSize& monitorSize);
        void UpdateWindow(int left, int top, int width, int height);
        void Render();

        IDirect3DSurface9* GetLeftEye();
        IDirect3DSurface9* GetRightEye();

        virtual void Initialize3DDevice(IDirect3DDevice9* device, const CSize& monitorSize) = 0;
        virtual void UpdateTextureSize(int textureWidth, int textureHeight) = 0;
        virtual void SetWindowSize(int left, int top, int width, int height) = 0;
        virtual void RenderFrame(IDirect3DTexture9** textures, int firstTextureIndex) = 0;

        virtual ~Base3DRenderer();
    };
}
