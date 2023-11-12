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

#pragma once

//-------------------------------------------------------------------------

#include <cstdint>
#include <string>
#include <utility>

#include "drmMode.h"
#include "point.h"
#include "fileDescriptor.h"
#include "rgb8880.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

using FB8880Point = Point<int32_t>;

//-------------------------------------------------------------------------

class Image8880;

//-------------------------------------------------------------------------

class FrameBuffer8880
{
public:

    static constexpr size_t bytesPerPixel{4};

    explicit FrameBuffer8880(const std::string& device);

    ~FrameBuffer8880();

    FrameBuffer8880(const FrameBuffer8880& fb) = delete;
    FrameBuffer8880& operator=(const FrameBuffer8880& fb) = delete;

    FrameBuffer8880(FrameBuffer8880&& fb) = delete;
    FrameBuffer8880& operator=(FrameBuffer8880&& fb) = delete;

    int32_t getWidth() const { return m_width; }
    int32_t getHeight() const { return m_height; }

    void clear(const RGB8880& rgb) const { clear(rgb.get8880()); }
    void clear(uint32_t rgb = 0) const;

    bool
    setPixelRGB(
        const FB8880Point& p,
        const RGB8880& rgb) const
    {
        return setPixel(p, rgb.get8880());
    }

    bool setPixel(const FB8880Point& p, uint32_t rgb) const;

    std::pair<bool, RGB8880> getPixelRGB(const FB8880Point& p) const;
    std::pair<bool, uint32_t> getPixel(const FB8880Point& p) const;

    bool putImage(const FB8880Point& p, const Image8880& image) const;

private:

    bool
    putImagePartial(
        const FB8880Point& p,
        const Image8880& image) const;

    bool
    validPixel(const FB8880Point& p) const
    {
        return (p.x() >= 0) &&
               (p.y() >= 0) &&
               (p.x() < static_cast<int32_t>(m_width)) &&
               (p.y() < static_cast<int32_t>(m_height));
    }

    size_t offset(const FB8880Point& p) const;

    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_length;
    int32_t m_lineLengthPixels;

    FileDescriptor m_fd;
    uint32_t* m_fbp;
    uint32_t m_fbId;
    uint32_t m_fbHandle;

    uint32_t m_savedConnectorId;
    drm::drmModeCrtc_ptr m_savedCrtc;
};

//-------------------------------------------------------------------------

} // namespace fb32

