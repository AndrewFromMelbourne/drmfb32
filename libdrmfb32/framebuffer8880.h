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

#include "drmMode.h"
#include "point.h"
#include "fileDescriptor.h"
#include "interface8880.h"
#include "rgb8880.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

using Interface8880Point = Point<int32_t>;

//-------------------------------------------------------------------------

class Image8880;

//-------------------------------------------------------------------------

class FrameBuffer8880
:
    public Interface8880
{
public:

    static constexpr size_t bytesPerPixel{4};

    explicit FrameBuffer8880(
        const std::string& device = "",
        uint32_t connectorId = 0);

    ~FrameBuffer8880() override;

    FrameBuffer8880(const FrameBuffer8880& fb) = delete;
    FrameBuffer8880& operator=(const FrameBuffer8880& fb) = delete;

    FrameBuffer8880(FrameBuffer8880&& fb) = delete;
    FrameBuffer8880& operator=(FrameBuffer8880&& fb) = delete;

    void clear(const RGB8880& rgb) override { clear(rgb.get8880()); }
    void clear(uint32_t rgb = 0) override;

    std::span<uint32_t> getBuffer() noexcept override { return {m_fbp, getBufferSize()}; };
    std::span<const uint32_t> getBuffer() const noexcept override { return {m_fbp, getBufferSize()}; }

    size_t getBufferSize() const noexcept { return m_lineLengthPixels * m_height;}

    std::optional<RGB8880> getPixelRGB(const Interface8880Point& p) const override;
    std::optional<uint32_t> getPixel(const Interface8880Point& p) const override;

    int getWidth() const noexcept override { return m_width; }
    int getHeight() const noexcept override { return m_height; }

    size_t offset(const Interface8880Point& p) const noexcept override;

    bool
    setPixelRGB(
        const Interface8880Point& p,
        const RGB8880& rgb) override
    {
        return setPixel(p, rgb.get8880());
    }

    bool setPixel(const Interface8880Point& p, uint32_t rgb) override;

    void update();

private:

    bool
    validPixel(const Interface8880Point& p) const
    {
        return (p.x() >= 0) and (p.y() >= 0) and (p.x() < m_width) and (p.y() < m_height);
    }

    int m_width;
    int m_height;
    int m_length;
    int m_lineLengthPixels;

    FileDescriptor m_fd;
    uint32_t* m_fbp;
    uint32_t m_fbId;
    uint32_t m_fbHandle;

    uint32_t m_connectorId;
    uint32_t m_crtcId;
    drmModeModeInfo m_mode;
    drm::drmModeCrtc_ptr m_originalCrtc;
};

//-------------------------------------------------------------------------

} // namespace fb32

