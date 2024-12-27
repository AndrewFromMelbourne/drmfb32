//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2024 Andrew Duncan
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

#include <algorithm>
#include <iostream>
#include <system_error>

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <turbojpeg.h>

#include "webcam.h"

//-------------------------------------------------------------------------

fb32::Webcam::Webcam(
    const std::string& device,
    bool fitToScreen,
    int requestedFPS,
    Interface8880& image)
:
    m_dimensions{ .width = 0, .height = 0 },
    m_fd{::open(device.c_str(), O_RDWR)},
    m_fitToScreen{fitToScreen},
    m_format{0},
    m_formatName{},
    m_image{},
    m_resizedImage{},
    m_videoBuffers{}
{
    if (m_fd.fd() == -1)
    {
        throw std::system_error{errno,
                                std::system_category(),
                                "cannot open video device " + device};
    }

    if (not hasVideoCapabilities())
    {
        throw std::invalid_argument("Device " +
                                    device +
                                    " does not have video capabilities");
    }

    if (not chooseFormat())
    {
        throw std::invalid_argument("Device " +
                                    device +
                                    " does not support YUYV or MJPEG");
    }

    chooseBestFit(image);

    if (m_fitToScreen)
    {
        initResizedImage(image);
    }

    if (not initVideo())
    {
        throw std::invalid_argument("Device " +
                                    device +
                                    " could not set requested video mode");
    }

    setFPS(requestedFPS);

    m_image = Image8880(m_dimensions.width, m_dimensions.height);

    if (not initBuffers())
    {
        throw std::invalid_argument("Device " +
                                    device +
                                    " could not create video buffers");
    }
}

//-------------------------------------------------------------------------

fb32::Webcam::~Webcam()
{
    stopStream();

    for (auto vb : m_videoBuffers)
    {
        ::munmap(vb.buffer, vb.length);
    }
}

//-------------------------------------------------------------------------

bool
fb32::Webcam::showFrame(
    Interface8880& image)
{
    v4l2_buffer buffer{
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .memory = V4L2_MEMORY_MMAP
    };

    if (::ioctl(m_fd.fd(), VIDIOC_DQBUF, &buffer) == -1)
    {
        return false;
    }

    auto vbuffer = m_videoBuffers[buffer.index].buffer;
    uint8_t* data = static_cast<uint8_t*>(vbuffer);

    //-----------------------------------------------------------------

    bool result = false;

    switch (m_format)
    {
        case V4L2_PIX_FMT_YUYV:

            result = convertYuyv(data, buffer.length);
            break;

        case V4L2_PIX_FMT_MJPEG:

            result = convertMjpeg(data, buffer.length);
            break;
    }

    if (result)
    {

        if (m_fitToScreen)
        {
            m_image.resizeToNearestNeighbour(m_resizedImage);

            const Interface8880Point p{
                (image.getWidth() - m_resizedImage.getWidth()) / 2,
                (image.getHeight() - m_resizedImage.getHeight()) / 2
            };

            image.putImage(p, m_resizedImage);
        }
        else
        {
            const Interface8880Point p{
                (image.getWidth() - m_image.getWidth()) / 2,
                (image.getHeight() - m_image.getHeight()) / 2
            };

            image.putImage(p, m_image);
        }
    }

    ::ioctl(m_fd.fd(), VIDIOC_QBUF, &buffer);

    return result;
}

//-------------------------------------------------------------------------

bool
fb32::Webcam::startStream() const
{
    enum v4l2_buf_type buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (::ioctl(m_fd.fd(), VIDIOC_STREAMON, &buf_type) == -1)
    {
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------

bool
fb32::Webcam::stopStream() const
{
    enum v4l2_buf_type buf_type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (::ioctl(m_fd.fd(), VIDIOC_STREAMOFF, &buf_type) == -1)
    {
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------

bool
fb32::Webcam::chooseBestFit(
    Interface8880& image)
{
    std::vector<Dimensions> dimensions;

    v4l2_frmsizeenum frmsize{ .index = 0, .pixel_format = m_format };

    while (::ioctl(m_fd.fd(), VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0)
    {
        if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
        {
            const int width = frmsize.discrete.width;
            const int height = frmsize.discrete.height;

            dimensions.emplace_back(width, height);
        }
        ++frmsize.index;
    }

    auto reverseSort = [](const Dimensions& a, const Dimensions& b) -> bool
    {
        return (a.height > b.height) or
                ((a.height == b.height) and (a.width > b.width));
    };

    std::sort(dimensions.begin(), dimensions.end(), reverseSort);

    m_dimensions = Dimensions{ 0, 0 };
    bool result = false;

    for (const auto d : dimensions)
    {
        if ((d.width <= image.getWidth()) and
            (d.height <= image.getHeight()) and
            (d.width > m_dimensions.width) and
            (d.height > m_dimensions.height))
        {
            m_dimensions = d;
            result = true;
            break;
        }
    }

    if (not result)
    {
        m_dimensions = dimensions.back();
        result = true;
    }

    return result;
}

//-------------------------------------------------------------------------

bool
fb32::Webcam::chooseFormat()
{
    bool result = false;
    v4l2_fmtdesc fmtdesc{ .type = V4L2_BUF_TYPE_VIDEO_CAPTURE };

    for (fmtdesc.index = 0 ;
         ::ioctl(m_fd.fd(), VIDIOC_ENUM_FMT, &fmtdesc) == 0 ;
         ++fmtdesc.index)
    {
        if (fmtdesc.pixelformat == V4L2_PIX_FMT_MJPEG)
        {
            m_format = fmtdesc.pixelformat;
            m_formatName = reinterpret_cast<char*>(fmtdesc.description);
            result = true;
            break;
        }
        if (fmtdesc.pixelformat == V4L2_PIX_FMT_YUYV)
        {
            m_format = fmtdesc.pixelformat;
            m_formatName = reinterpret_cast<char*>(fmtdesc.description);
            result = true;
        }
    }

    return result;
}

//-------------------------------------------------------------------------

bool
fb32::Webcam::convertMjpeg(
    const uint8_t* data,
    int length)
{
    bool result = false;
    tjhandle tjInstance = tjInitDecompress();

    if (tjInstance != NULL)
    {
        int jpegWidth = 0;
        int jpegHeight = 0;
        int jpegSubsamp = 0;
        int jpegColorspace = 0;

        if (tjDecompressHeader3(tjInstance,
                                data,
                                length,
                                &jpegWidth,
                                &jpegHeight,
                                &jpegSubsamp,
                                &jpegColorspace) >= 0)
        {
            if (tjDecompress2(tjInstance,
                              data,
                              length,
                              reinterpret_cast<unsigned char*>(m_image.getBuffer()),
                              m_image.getWidth(),
                              m_image.getWidth() * 4,
                              m_image.getHeight(),
                              TJPF_BGRX,
                              TJFLAG_FASTDCT) >= 0)
            {
                result = true;
            }
        }

        tjDestroy(tjInstance);
    }

    return result;
}

//-------------------------------------------------------------------------
//
// R = 1.164 (Y - 16) + 1.596 (V - 128)
// G = 1.164 (Y - 16) - 0.813 (V - 128) - 0.391 (U - 128)
// B = 1.164 (Y - 16) + 2.018 (U - 128)
//
// R = (1192 (Y - 16) + 1634 (V - 128)) / 1024
// G = (1192 (Y - 16) -  832 (V - 128) -  400 (U - 128)) / 1024
// B = (1992 (Y - 16) + 2066 (U - 128)) / 1024
//

bool
fb32::Webcam::convertYuyv(
    const uint8_t* data,
    int length)
{
    uint32_t* buffer = m_image.getBuffer();

    for (int i = 0 ; i < length ; i += 4)
    {
        const int y1 = data[0] - 16;
        const int u = data[1] - 128;
        const int y2 = data[2] - 16;
        const int v = data[3] - 128;

        const int y1Part = 1192 * y1;
        const int y2Part = 1192 * y2;

        const int rPart = 1634 * v;
        const int gPart = -832 * v - 400 * u;
        const int bPart = 2066 * u;

        const RGB8880 rgb1{
            static_cast<uint8_t>(std::clamp((y1Part + rPart) / 1024, 0, 255)),
            static_cast<uint8_t>(std::clamp((y1Part + gPart) / 1024, 0, 255)),
            static_cast<uint8_t>(std::clamp((y1Part + bPart) / 1024, 0, 255))};

        const RGB8880 rgb2{
            static_cast<uint8_t>(std::clamp((y2Part + rPart) / 1024, 0, 255)),
            static_cast<uint8_t>(std::clamp((y2Part + gPart) / 1024, 0, 255)),
            static_cast<uint8_t>(std::clamp((y2Part + bPart) / 1024, 0, 255))};

        buffer[0] = rgb1.get8880();
        buffer[1] = rgb2.get8880();

        data += 4;
        buffer += 2;
    }

    return true;
}

//-------------------------------------------------------------------------

bool
fb32::Webcam::hasVideoCapabilities() const
{
    v4l2_capability cap;

    if ((::ioctl(m_fd.fd(), VIDIOC_QUERYCAP, &cap) == -1) or
        ((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) or
        ((cap.capabilities & V4L2_CAP_STREAMING) == 0))
    {
        return false;
    }

    return true;
}

//-------------------------------------------------------------------------

bool fb32::Webcam::initBuffers()
{
    constexpr int NUMBER_OF_BUFFERS_TO_REQUEST{4};
    constexpr int MINIMUM_BUFFERS{2};

    struct v4l2_requestbuffers reqbuffers
    {
        .count = NUMBER_OF_BUFFERS_TO_REQUEST,
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .memory = V4L2_MEMORY_MMAP
    };

    if (::ioctl(m_fd.fd(), VIDIOC_REQBUFS, &reqbuffers) == -1)
    {
        return false;
    }

    if (reqbuffers.count < MINIMUM_BUFFERS)
    {
        return false;
    }

    const unsigned int numberOfVideoBuffers = reqbuffers.count;

    for (unsigned int i = 0 ; i < numberOfVideoBuffers ; ++i)
    {
        v4l2_buffer buffer
        {
            .index = i,
            .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
            .memory = V4L2_MEMORY_MMAP
        };

        if (::ioctl(m_fd.fd(), VIDIOC_QUERYBUF, &buffer) == -1)
        {
            return false;
        }

        VideoBuffer vb;

        vb.length = buffer.length;
        vb.buffer = ::mmap(NULL,
                           buffer.length,
                           PROT_READ | PROT_WRITE,
                           MAP_SHARED,
                           m_fd.fd(),
                           buffer.m.offset);

        if (vb.buffer == MAP_FAILED)
        {
            return false;
        }

        m_videoBuffers.push_back(vb);
    }

    //---------------------------------------------------------------------

    for (unsigned int i = 0 ; i < numberOfVideoBuffers ; ++i)
    {
        v4l2_buffer buffer
        {
            .index = i,
            .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
            .memory = V4L2_MEMORY_MMAP
        };

        if (::ioctl(m_fd.fd(), VIDIOC_QBUF, &buffer) == -1)
        {
            return false;
        }
    }

    return true;
}

//-------------------------------------------------------------------------

void
fb32::Webcam::initResizedImage(
    Interface8880& image)
{
    int width = (m_dimensions.width * image.getHeight()) /
                 m_dimensions.height;
    int height = image.getHeight();

    if (width > m_dimensions.width)
    {
        width = image.getWidth();
        height = (m_dimensions.height * image.getWidth()) /
                  m_dimensions.width;
    }

    m_resizedImage = Image8880{width, height};
}

//-------------------------------------------------------------------------

bool
fb32::Webcam::initVideo()
{
    v4l2_format fmt{};

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
    fmt.fmt.pix.width = m_dimensions.width,
    fmt.fmt.pix.height = m_dimensions.height,
    fmt.fmt.pix.field = V4L2_FIELD_ANY;
    fmt.fmt.pix.pixelformat = m_format;

    if (::ioctl(m_fd.fd(), VIDIOC_S_FMT, &fmt) == -1)
    {
        return false;
    }

    if (fmt.fmt.pix.pixelformat != m_format)
    {
        return false;
    }

    m_dimensions.width = fmt.fmt.pix.width;
    m_dimensions.height = fmt.fmt.pix.height;

    return true;
}

//-------------------------------------------------------------------------

bool
fb32::Webcam::setFPS(
    int fps) const
{
    if (fps <= 0)
    {
        return false;
    }

    struct v4l2_streamparm streamparm{ .type = V4L2_BUF_TYPE_VIDEO_CAPTURE };

    if (::ioctl(m_fd.fd(), VIDIOC_G_PARM, &streamparm) == -1)
    {
        return false;
    }

    if (not (streamparm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME))
    {
        return false;
    }

    streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamparm.parm.capture.timeperframe.numerator = 1;
    streamparm.parm.capture.timeperframe.denominator = fps;

    if (::ioctl(m_fd.fd(), VIDIOC_S_PARM, &streamparm) == -1)
    {
        return false;
    }

    return true;
}

