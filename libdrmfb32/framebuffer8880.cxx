//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2022 Andrew Duncan
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

#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_fourcc.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <system_error>

#include "drmMode.h"
#include "framebuffer8880.h"
#include "image8880.h"
#include "point.h"

//=========================================================================

namespace
{

//-------------------------------------------------------------------------

bool
findDrmResources(
    fb32::FileDescriptor& fd,
    uint32_t& crtcId,
    uint32_t& connectorId,
    drmModeModeInfo& mode)
{
    auto resources = drm::drmModeGetResources(fd);
    bool resourcesFound = false;

    for (int i = 0 ; (i < resources->count_connectors) and not resourcesFound ; ++i)
    {
        connectorId = resources->connectors[i];
        auto connector = drm::drmModeGetConnector(fd, connectorId);
        const bool connected = (connector->connection == DRM_MODE_CONNECTED);

        if (connected and (connector->count_modes > 0))
        {
            for (int j = 0 ; (j < resources->count_encoders) and not resourcesFound ; ++j)
            {
                uint32_t encoderId = resources->encoders[j];
                auto encoder = drm::drmModeGetEncoder(fd, encoderId);

                for (int k = 0 ; (k < resources->count_crtcs) and not resourcesFound ; ++k)
                {
                    uint32_t currentCrtc = 1 << k;

                    if (encoder->possible_crtcs & currentCrtc)
                    {
                        crtcId = resources->crtcs[k];
                        auto crtc = drm::drmModeGetCrtc(fd, crtcId);
                        if ((crtc->mode.hdisplay > 0) and (crtc->mode.vdisplay > 0))
                        {
                            mode = crtc->mode;
                            resourcesFound = true;
                        }
                    }
                }
            }
        }
    }

    return resourcesFound;
}

//-------------------------------------------------------------------------

}

//=========================================================================

fb32::FrameBuffer8880:: FrameBuffer8880(
    const std::string& device)
:
    m_width{0},
    m_height{0},
    m_length{0},
    m_lineLengthPixels{0},
    m_fd{::open(device.c_str(), O_RDWR)},
    m_fbp{nullptr},
    m_fbId{0},
    m_fbHandle{0},
    m_savedConnectorId{0},
    m_savedCrtc(nullptr, [](drmModeCrtc*){})
{
    if (m_fd.fd() == -1)
    {
        throw std::system_error{errno,
                                std::system_category(),
                                "cannot open dri device " + device};
    }

    //---------------------------------------------------------------------

    uint64_t hasDumb;
    if ((drmGetCap(m_fd.fd(), DRM_CAP_DUMB_BUFFER, &hasDumb) < 0) or not hasDumb)
    {
        throw std::system_error{errno,
                                std::system_category(),
                                "no DRM dumb buffer capability"};
    }

    //---------------------------------------------------------------------

    uint32_t crtcId = 0;
    uint32_t connectorId = 0;
    drmModeModeInfo mode;

    if (not findDrmResources(m_fd, crtcId, connectorId, mode))
    {
        throw std::logic_error("no connected CRTC found");
    }

    //---------------------------------------------------------------------

    m_width = mode.hdisplay;
    m_height = mode.vdisplay;

    struct drm_mode_create_dumb dmcb =
    {
        .height = mode.vdisplay,
        .width = mode.hdisplay,
        .bpp = 32,
        .flags = 0,
        .handle = 0,
        .pitch = 0,
        .size = 0
    };

    if (drmIoctl(m_fd.fd(), DRM_IOCTL_MODE_CREATE_DUMB, &dmcb) < 0)
    {
        throw std::system_error{errno,
                                std::system_category(),
                                "Cannot create a DRM dumb buffer"};
    }

    //---------------------------------------------------------------------

    m_length = dmcb.size;
    m_lineLengthPixels = dmcb.pitch / bytesPerPixel;
    m_fbHandle = dmcb.handle;

    uint32_t handles[4] = { dmcb.handle };
    uint32_t strides[4] = { dmcb.pitch };
    uint32_t offsets[4] = { 0 };

    if (drmModeAddFB2(
            m_fd.fd(),
            mode.hdisplay,
            mode.vdisplay,
            DRM_FORMAT_XRGB8888,
            handles,
            strides,
            offsets,
            &m_fbId,
            0) < 0)
    {
        throw std::system_error{errno,
                                std::system_category(),
                                "Cannot add frame buffer"};
    }

    //---------------------------------------------------------------------

    struct drm_mode_map_dumb dmmd =
    {
        .handle = m_fbHandle
    };

    if (drmIoctl(m_fd.fd(), DRM_IOCTL_MODE_MAP_DUMB, &dmmd) < 0)
    {
        throw std::system_error{errno,
                                std::system_category(),
                                "Cannot map dumb buffer"};
    }

    void* fbp = mmap(0, m_length, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd.fd(), dmmd.offset);

    if (fbp == MAP_FAILED)
    {
        throw std::system_error(errno,
                                std::system_category(),
                                "mapping framebuffer device to memory");
    }

    m_fbp = static_cast<uint32_t*>(fbp);

    //---------------------------------------------------------------------

    m_savedConnectorId = connectorId;
    m_savedCrtc = drm::drmModeGetCrtc(m_fd, crtcId);

    if (drmModeSetCrtc(m_fd.fd(), crtcId, m_fbId, 0, 0, &connectorId, 1, &mode) < 0)
    {
        throw std::system_error(errno,
                                std::system_category(),
                                "unable to set crtc with frame buffer");
    }
}

//-------------------------------------------------------------------------

fb32::FrameBuffer8880:: ~FrameBuffer8880()
{
    ::munmap(m_fbp, m_length);
    drmModeRmFB(m_fd.fd(), m_fbId);

    struct drm_mode_destroy_dumb dmdd =
    {
        .handle = m_fbHandle
    };

    drmIoctl(m_fd.fd(), DRM_IOCTL_MODE_DESTROY_DUMB, &dmdd);

    drmModeSetCrtc(m_fd.fd(),
                   m_savedCrtc->crtc_id,
                   m_savedCrtc->buffer_id,
                   m_savedCrtc->x,
                   m_savedCrtc->y,
                   &m_savedConnectorId,
                   1,
                   &(m_savedCrtc->mode));
}

//-------------------------------------------------------------------------

void
fb32::FrameBuffer8880:: clear(
    uint32_t rgb) const
{
    std::fill(m_fbp, m_fbp + (m_length / bytesPerPixel), rgb);
}

//-------------------------------------------------------------------------

bool
fb32::FrameBuffer8880:: setPixel(
    const FB8880Point& p,
    uint32_t rgb) const
{
    bool isValid{validPixel(p)};

    if (isValid)
    {
        m_fbp[offset(p)] = rgb;
    }

    return isValid;
}

//-------------------------------------------------------------------------

std::pair<bool, fb32::RGB8880>
fb32::FrameBuffer8880:: getPixelRGB(
    const FB8880Point& p) const
{
    bool isValid{validPixel(p)};
    RGB8880 rgb{0, 0, 0};

    if (isValid)
    {
        rgb.set8880(m_fbp[offset(p)]);
    }

    return std::make_pair(isValid, rgb);
}

//-------------------------------------------------------------------------

std::pair<bool, uint32_t>
fb32::FrameBuffer8880:: getPixel(
    const FB8880Point& p) const
{
    bool isValid{validPixel(p)};
    uint32_t rgb{0};

    if (isValid)
    {
        rgb = m_fbp[offset(p)];
    }

    return std::make_pair(isValid, rgb);
}

//-------------------------------------------------------------------------

bool
fb32::FrameBuffer8880:: putImage(
    const FB8880Point& p_left,
    const Image8880& image) const
{
    FB8880Point p{ p_left.x(), p_left.y() };

    if ((p.x() < 0) ||
        ((p.x() + image.getWidth()) > static_cast<int32_t>(m_width)))
    {
        return putImagePartial(p, image);
    }

    if ((p.y() < 0) ||
        ((p.y() + image.getHeight()) > static_cast<int32_t>(m_height)))
    {
        return putImagePartial(p, image);
    }

    for (int32_t j = 0 ; j < image.getHeight() ; ++j)
    {
        auto start = image.getRow(j);

        std::copy(start,
                  start + image.getWidth(),
                  m_fbp + ((j + p.y()) * m_lineLengthPixels) + p.x());
    }

    return true;
}

//-------------------------------------------------------------------------

bool
fb32::FrameBuffer8880:: putImagePartial(
    const FB8880Point& p,
    const Image8880& image) const
{
    auto x = p.x();
    auto xStart = 0;
    auto xEnd = image.getWidth() - 1;

    auto y = p.y();
    auto yStart = 0;
    auto yEnd = image.getHeight() - 1;

    if (x < 0)
    {
        xStart = -x;
        x = 0;
    }

    if ((x - xStart + image.getWidth()) >
        static_cast<int32_t>(m_width))
    {
        xEnd = m_height - 1 - (x - xStart);
    }

    if (y < 0)
    {
        yStart = -y;
        y = 0;
    }

    if ((y - yStart + image.getHeight()) >
        static_cast<int32_t>(m_height))
    {
        yEnd = m_height - 1 - (y - yStart);
    }

    if ((xEnd - xStart) <= 0)
    {
        return false;
    }

    if ((yEnd - yStart) <= 0)
    {
        return false;
    }

    for (auto j = yStart ; j <= yEnd ; ++j)
    {
        auto start = image.getRow(j) + xStart;

        std::copy(start,
                  start + (xEnd - xStart + 1),
                  m_fbp + ((j+y) * m_lineLengthPixels) + x);
    }

    return true;
}

//-------------------------------------------------------------------------

size_t
fb32::FrameBuffer8880:: offset(
    const FB8880Point& p) const
{
    return p.x() + p.y() * m_lineLengthPixels;
}
