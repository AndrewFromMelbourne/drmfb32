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

class Interface8880Base
:
    public Interface8880
{
public:

    static constexpr auto c_bytesPerPixel{4};

    ~Interface8880Base() override = default;

    [[nodiscard]] virtual std::span<uint32_t> getBuffer() noexcept = 0;
    [[nodiscard]] virtual std::span<const uint32_t> getBuffer() const  noexcept = 0;

    [[nodiscard]] virtual Dimensions8880 getDimensions() const noexcept = 0;

    void clear(const RGB8880& rgb) override { clear(rgb.get8880()); }
    void clear(uint32_t rgb = 0) override;

    [[nodiscard]] std::optional<RGB8880> getPixelRGB(Point8880 p) const override;
    [[nodiscard]] std::optional<RGB8> getPixelRGB8(Point8880 p) const override;
    [[nodiscard]] std::optional<uint32_t> getPixel(Point8880 p) const override;

    [[nodiscard]] std::span<uint32_t> getRow(int y);
    [[nodiscard]] std::span<const uint32_t> getRow(int y) const;

    [[nodiscard]] virtual std::size_t offset(Point8880 p) const noexcept = 0;

    bool
    setPixelRGB(
        Point8880 p,
        const RGB8880& rgb) override
    {
        return setPixel(p, rgb.get8880());
    }

    bool
    setPixelRGB8(
        Point8880 p,
        RGB8 rgb) override
    {
        return setPixel(p, RGB8880(rgb).get8880());
    }

    bool setPixel(Point8880 p, uint32_t rgb) override;

    bool putImage(Point8880 p, const Interface8880Base& image);

    [[nodiscard]] bool
    validPixel(Point8880 p) const noexcept override
    {
        const auto d = getDimensions();

        return ((p.x() >= 0) and
                (p.x() < d.width()) and
                (p.y() >= 0) and
                (p.y() < d.height()));
    }

private:

    bool putImagePartial(Point8880 p, const Interface8880Base& image);
};

//-------------------------------------------------------------------------

} // namespace fb32

