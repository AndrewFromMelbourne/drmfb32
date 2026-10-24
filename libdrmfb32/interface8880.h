//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2026 Andrew Duncan
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
#include <optional>
#include <span>

#include "dimensions.h"
#include "point.h"
#include "rgb8880.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

using Dimensions8880 = Dimensions<int>;
using Point8880 = Point<int>;

//-------------------------------------------------------------------------

class Interface8880
{
public:

    virtual ~Interface8880() = default;

    [[nodiscard]] virtual Dimensions8880 getDimensions() const noexcept = 0;

    virtual void clear(const RGB8880& rgb) = 0;
    virtual void clear(uint32_t rgb = 0) = 0;

    [[nodiscard]] virtual std::optional<RGB8880> getPixelRGB(Point8880 p) const = 0;
    [[nodiscard]] virtual std::optional<RGB8> getPixelRGB8(Point8880 p) const = 0;
    [[nodiscard]] virtual std::optional<uint32_t> getPixel(Point8880 p) const= 0;

    virtual bool setPixelRGB(Point8880 p, const RGB8880& rgb) = 0;
    virtual bool setPixelRGB8(Point8880 p, RGB8 rgb) = 0;
    virtual bool setPixel(Point8880 p, uint32_t rgb) = 0;

    virtual bool validPixel(Point8880 p) const noexcept = 0;
};

//-------------------------------------------------------------------------

inline Point8880
center(
    const Interface8880& frame,
    const Interface8880& image) noexcept
{
    const auto fd = frame.getDimensions();
    const auto id = image.getDimensions();

    return {(fd.getWidth() - id.getWidth()) / 2,
            (fd.getHeight() - id.getHeight()) / 2};
}

//-------------------------------------------------------------------------

} // namespace fb32

