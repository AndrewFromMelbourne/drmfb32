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
#include <sys/stat.h>

#include "drmMode.h"
#include "fileDescriptor.h"

//-------------------------------------------------------------------------

bool
drm::drmDeviceHasDumbBuffer(
    const std::string& device)
{
    fb32::FileDescriptor fd{::open(device.c_str(), O_RDWR)};

    if (fd.fd() == -1)
    {
        return false;
    }

    //---------------------------------------------------------------------

    uint64_t hasDumb{};
    return not drmGetCap(fd.fd(), DRM_CAP_DUMB_BUFFER, &hasDumb) and hasDumb;
}

//-------------------------------------------------------------------------

drm::DrmDevices::DrmDevices()
:
    m_deviceCount{},
    m_devices{}
{
    m_deviceCount = drmGetDevices2(0, m_devices.data(), m_devices.size());
}

//-------------------------------------------------------------------------

drm::DrmDevices::~DrmDevices()
{
    if (m_deviceCount > 0)
    {
        drmFreeDevices(m_devices.data(), m_deviceCount);
    }
}

//-------------------------------------------------------------------------

drmDevicePtr
drm::DrmDevices::getDevice(int index) const
{
    if (index < 0 or index >= m_deviceCount)
    {
        return nullptr;
    }

    return m_devices[index];
}

//-------------------------------------------------------------------------

drm::drmModeConnector_ptr
drm::drmModeGetConnector(
    fb32::FileDescriptor& fd,
    uint32_t connId) noexcept
{
    return drmModeConnector_ptr(::drmModeGetConnector(fd.fd(), connId),
                                &drmModeFreeConnector);
}

//-------------------------------------------------------------------------

drm::drmModeCrtc_ptr
drm::drmModeGetCrtc(
    fb32::FileDescriptor& fd,
    uint32_t crtcId) noexcept
{
    return drmModeCrtc_ptr(::drmModeGetCrtc(fd.fd(), crtcId),
                           &drmModeFreeCrtc);
}

//-------------------------------------------------------------------------

drm::drmModeEncoder_ptr
drm::drmModeGetEncoder(
    fb32::FileDescriptor& fd,
    uint32_t encoderId) noexcept
{
    return drmModeEncoder_ptr(::drmModeGetEncoder(fd.fd(), encoderId),
                              &drmModeFreeEncoder);
}

//-------------------------------------------------------------------------

drm::drmModeObjectProperties_ptr
drm::drmModeObjectGetProperties(
    fb32::FileDescriptor& fd,
    uint32_t objectId,
    uint32_t objectType) noexcept
{
    return drmModeObjectProperties_ptr(::drmModeObjectGetProperties(fd.fd(),
                                                                    objectId,
                                                                    objectType),
                                       &drmModeFreeObjectProperties);
}

//-------------------------------------------------------------------------

drm::drmModePlane_ptr
drm::drmModeGetPlane(
    fb32::FileDescriptor& fd,
    uint32_t planeId) noexcept
{
    return drmModePlane_ptr(::drmModeGetPlane(fd.fd(), planeId),
                            &drmModeFreePlane);
}

//-------------------------------------------------------------------------

drm::drmModePlaneRes_ptr
drm::drmModeGetPlaneResources(
    fb32::FileDescriptor& fd) noexcept
{
    return drmModePlaneRes_ptr(::drmModeGetPlaneResources(fd.fd()),
                               &drmModeFreePlaneResources);
}

//-------------------------------------------------------------------------

drm::drmModePropertyRes_ptr
drm::drmModeGetProperty(
    fb32::FileDescriptor& fd,
    uint32_t propertyId) noexcept
{
    return drmModePropertyRes_ptr(::drmModeGetProperty(fd.fd(), propertyId),
                                  &drmModeFreeProperty);
}

//-------------------------------------------------------------------------

drm::drmModeRes_ptr
drm::drmModeGetResources(
    fb32::FileDescriptor& fd) noexcept
{
    return drmModeRes_ptr(::drmModeGetResources(fd.fd()),
                          &drmModeFreeResources);
}

//-------------------------------------------------------------------------

uint64_t
drm::drmGetPropertyValue(
    fb32::FileDescriptor& fd,
    uint32_t objectId,
    uint32_t objectType,
    const std::string& name) noexcept
{
    auto properties{drmModeObjectGetProperties(fd, objectId, objectType)};

    for (auto i = 0; i < properties->count_props; ++i)
    {
        auto property{drmModeGetProperty(fd, properties->props[i])};

        if (name == property->name)
        {
            return properties->prop_values[i];
        }
    }

    return ~0;
}
