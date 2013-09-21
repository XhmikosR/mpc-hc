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
#include "Base3DRenderer.h"

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#define DLL2_EXPORTS
#include <stdio.h>

namespace DSObjects
{
    Base3DRenderer::Base3DRenderer(IDirect3DDevice9* device, const CSize& monitorSize)
    {
        this->textures = 0;
        this->surfaces = 0;
        this->textureHeight = this->textureWidth = -1;
        this->device = device;
    }

    void Base3DRenderer::Render()
    {
        // NOTE: UpdateWindow MUST be called first!
        this->RenderFrame(textures, 0);
    }

    void Base3DRenderer::UpdateWindow(int left, int top, int width, int height)
    {
        this->SetWindowSize(left, top, width, height);

        // We create textures and surfaces for each eye that's the same size of the texture we render.
        // This is not done every time - only when we resize the window should this be done.
        if (this->textureWidth != width || this->textureHeight != height) {
            this->textureHeight = height;
            this->textureWidth = width;

            if (textures) {
                // Remove textures first
                for (int i = 0; i < 2; ++i) {
                    textures[i]->Release();
                    surfaces[i]->Release();
                }
            } else {
                // Allocate texture and surface buffer
                textures = new IDirect3DTexture9*[2];
                surfaces = new IDirect3DSurface9*[2];
            }

            // Generate a couple of surfaces
            for (int i = 0; i < 2; ++i) {
                device->CreateTexture(
                    width,
                    height, 1,
                    D3DUSAGE_RENDERTARGET,
                    D3DFMT_A32B32G32R32F,
                    D3DPOOL_DEFAULT,
                    &textures[i],
                    nullptr);
                textures[i]->GetSurfaceLevel(0, &surfaces[i]);
            }

            // Sets the texture size
            this->UpdateTextureSize(textureWidth, textureHeight);
        }
    }

    IDirect3DSurface9* Base3DRenderer::GetLeftEye()
    {
        // Returns the surface for the left eye. We don't check the ptr; it should simply exist. :-)
        return surfaces[0];
    }

    IDirect3DSurface9* Base3DRenderer::GetRightEye()
    {
        // Returns the surface for the right eye. We don't check the ptr; it should simply exist. :-)
        return surfaces[1];
    }

    Base3DRenderer::~Base3DRenderer()
    {
        // Remove textures and surfaces; the other cleanup is done in the destructor of the library
        // implementation (e.g. clean up your own code in your destructor please)
        if (textures) {
            // Remove textures first
            for (int i = 0; i < 2; ++i) {
                textures[i]->Release();
                surfaces[i]->Release();
            }

            // Delete arrays
            delete[] textures;
            delete[] surfaces;
        }
    }
}
