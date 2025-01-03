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

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "interface8880.h"
#include "rgb8880.h"
#include "point.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

class Image8880
:
    public Interface8880
{
public:

    Image8880() = default;
    Image8880(int width, int height, uint8_t numberOfFrames = 1);
    Image8880(int width,
              int height,
              const std::vector<uint32_t>& buffer,
              uint8_t numberOfFrames = 1);

    ~Image8880() override = default;

    Image8880(const Image8880&) = default;
    Image8880& operator=(const Image8880&) = default;

    Image8880(Image8880&& image) = default;
    Image8880& operator=(Image8880&& image) = default;

    int getWidth() const override { return m_width; }
    int getHeight() const override { return m_height; }

    uint8_t getFrame() const { return m_frame; }
    uint8_t getNumberOfFrames() const { return m_numberOfFrames; }
    void setFrame(uint8_t frame);

    void clear(const RGB8880& rgb) override { clear(rgb.get8880()); }
    void clear(uint32_t rgb) override;

    uint32_t* getBuffer() override { return m_buffer.data(); };
    const uint32_t* getBuffer() const override { return m_buffer.data(); }

    std::optional<RGB8880> getPixelRGB(
        const Interface8880Point& p) const override
    {
        return getPixelRGB(p, m_frame);
    }

    std::optional<uint32_t> getPixel(const Interface8880Point& p) const override
    {
        return getPixel(p, m_frame);
    }

    std::optional<RGB8880> getPixelRGB(const Interface8880Point& p, uint8_t frame) const;
    std::optional<uint32_t> getPixel(const Interface8880Point& p, uint8_t frame) const;

    const uint32_t* getRow(int y) const;

    size_t offset(const Interface8880Point& p) const override
    {
        return offset(p, m_frame);
    }


    Image8880 resizeBilinearInterpolation(int width, int height) const;
    Image8880 resizeLanczos3Interpolation(int width, int height) const;
    Image8880 resizeNearestNeighbour(int width, int height) const;

    Image8880& resizeToBilinearInterpolation(Image8880& image) const;
    Image8880& resizeToLanczos3Interpolation(Image8880& image) const;
    Image8880& resizeToNearestNeighbour(Image8880& image) const;

    Image8880 scaleUp(uint8_t scale) const;

    bool
    setPixelRGB(
        const Interface8880Point& p,
        const RGB8880& rgb) override
    {
        return setPixel(p, rgb.get8880(), m_frame);
    }

    bool setPixel(
        const Interface8880Point& p,
        uint32_t rgb) override
    {
        return setPixel(p, rgb, m_frame);
    }

    bool setPixel(const Interface8880Point& p, uint32_t rgb, uint8_t frame);

    bool
    setPixelRGB(
        const Interface8880Point& p,
        const RGB8880& rgb,
        uint8_t frame)
    {
        return setPixel(p, rgb.get8880(), frame);
    }

private:

    size_t offset(const Interface8880Point& p, uint8_t frame) const;

    bool
    validPixel(const Interface8880Point& p) const
    {
        return ((p.x() >= 0) and (p.y() >= 0) and (p.x() < m_width) and (p.y() < m_height));
    }

    int m_width{};
    int m_height{};

    uint8_t m_frame;
    uint8_t m_numberOfFrames{};

    std::vector<uint32_t> m_buffer{};
};

//-------------------------------------------------------------------------

} // namespace fb32

