//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2023 Andrew Duncan
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------

#include "drmMode.h"

#include <iostream>

//-------------------------------------------------------------------------

int
main()
{
    drm::DrmDevices devices;

    if (devices.getDeviceCount() < 0)
    {
        return 1;
    }

    for (auto i = 0 ; i < devices.getDeviceCount() ; ++i)
    {
        auto device = devices.getDevice(i);

        std::cout << i;

        if (device->available_nodes & (1 << DRM_NODE_PRIMARY))
        {
            std::cout << " Primary(" << device->nodes[DRM_NODE_PRIMARY] << ')';
        }
        if (device->available_nodes & (1 << DRM_NODE_CONTROL))
        {
            std::cout << " Control(" << device->nodes[DRM_NODE_CONTROL] << ')';
        }
        if (device->available_nodes & (1 << DRM_NODE_RENDER))
        {
            std::cout << " Render(" << device->nodes[DRM_NODE_RENDER] << ')';
        }

        std::cout << '\n';
    }

    return 0;
}
