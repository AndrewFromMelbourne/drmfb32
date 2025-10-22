//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2025 Andrew Duncan
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
#include <initializer_list>
#include <utility>
#include <span>
#include <vector>

#include "interface8880.h"
#include "rgb8880.h"
#include "point.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

class Image8880Frames
:
    public Interface8880
{
public:

    //---------------------------------------------------------------------
    // constructors, destructors and assignment

    Image8880Frames() = default;
    Image8880Frames(int width, int height, uint8_t numberOfFrames = 1);
    Image8880Frames(int width,
                    int height,
                    std::initializer_list<uint32_t> buffer,
                    uint8_t numberOfFrames = 1);
    Image8880Frames(int width,
                    int height,
                    std::span<const uint32_t> buffer,
                    uint8_t numberOfFrames = 1);

    ~Image8880Frames() override = default;

    Image8880Frames(const Image8880Frames&) = default;
    Image8880Frames& operator=(const Image8880Frames&) = default;

    Image8880Frames(Image8880Frames&& image) = default;
    Image8880Frames& operator=(Image8880Frames&& image) = default;

    //---------------------------------------------------------------------
    // getters and setters

    [[nodiscard]] int getWidth() const noexcept override { return m_width; }
    [[nodiscard]] int getHeight() const noexcept override  { return m_height; }

    [[nodiscard]] uint8_t getFrame() const { return m_frame; }
    [[nodiscard]] uint8_t getNumberOfFrames() const { return m_numberOfFrames; }

    [[nodiscard]] std::span<uint32_t> getBuffer() noexcept override { return m_buffer; };
    [[nodiscard]] std::span<const uint32_t> getBuffer() const noexcept override { return m_buffer; }

    std::optional<RGB8880> getFramePixelRGB(const Interface8880Point& p, uint8_t frame) const;
    std::optional<uint32_t> getFramePixel(const Interface8880Point& p, uint8_t frame) const;

    void setFrame(uint8_t frame);

    bool setFramePixel(const Interface8880Point& p, uint32_t rgb, uint8_t frame);

    bool
    setPixelRGB(
        const Interface8880Point& p,
        const RGB8880& rgb,
        uint8_t frame)
    {
        return setFramePixel(p, rgb.get8880(), frame);
    }

    std::size_t offset(const Interface8880Point& p) const noexcept override
    {
        return offset(p, m_frame);
    }

private:

    [[nodiscard]] std::size_t offset(const Interface8880Point& p, uint8_t frame) const noexcept;

    int m_width{};
    int m_height{};

    uint8_t m_frame;
    uint8_t m_numberOfFrames{};

    std::vector<uint32_t> m_buffer{};
};

//-------------------------------------------------------------------------

} // namespace fb32

