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
#include "interface8880.h"
#include "point.h"
#include "rgb8880.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

using Dimensions8880 = Dimensions<int>;
using Point8880 = Point<int>;

//-------------------------------------------------------------------------

class Interface8880Null
:
    public Interface8880
{
public:

    virtual ~Interface8880Null() = default;

    [[nodiscard]] Dimensions8880 getDimensions() const noexcept override
    {
        return m_dimensions;
    }

    virtual void clear(const RGB8880&) override {};
    virtual void clear([[maybe_unused]] uint32_t rgb = 0) override {};

    [[nodiscard]] virtual std::optional<RGB8880> getPixelRGB(Point8880 p) const override
    {
        if (validPixel(p))
        {
            return RGB8880{0};
        }

        return std::nullopt;
    }

    [[nodiscard]] std::optional<RGB8> getPixelRGB8(Point8880 p) const override
    {
        if (validPixel(p))
        {
            return RGB8{0, 0, 0};
        }

        return std::nullopt;
    }

    [[nodiscard]] std::optional<uint32_t> getPixel(Point8880 p) const override
    {
        if (validPixel(p))
        {
            return 0;
        }

        return std::nullopt;
    }

    bool setPixelRGB(Point8880 p, const RGB8880&) override { return validPixel(p); }
    bool setPixelRGB8(Point8880 p, RGB8)  override { return validPixel(p); }
    bool setPixel(Point8880 p, uint32_t)  override { return validPixel(p); }

    bool validPixel(Point8880 p) const noexcept override
    {
        const auto d = getDimensions();

        return ((p.x() >= 0) and
                (p.x() < d.width()) and
                (p.y() >= 0) and
                (p.y() < d.height()));
    }

private:

    Dimensions8880 m_dimensions;
};

//-------------------------------------------------------------------------

} // namespace fb32

