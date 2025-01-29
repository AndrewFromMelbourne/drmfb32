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

#include "point.h"
#include "rgb8880.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

using Interface8880Point = Point<int>;

class Image8880;

//-------------------------------------------------------------------------

class Interface8880
{
public:

    static constexpr auto c_bytesPerPixel{4};

    virtual ~Interface8880() = 0;

    [[nodiscard]] virtual std::span<uint32_t> getBuffer() noexcept = 0;
    [[nodiscard]] virtual std::span<const uint32_t> getBuffer() const  noexcept = 0;

    [[nodiscard]] virtual int getWidth() const noexcept = 0;
    [[nodiscard]] virtual int getHeight() const noexcept = 0;

    virtual void clear(const RGB8880& rgb) = 0;
    virtual void clear(uint32_t rgb = 0) = 0;

    [[nodiscard]] virtual std::optional<RGB8880> getPixelRGB(const Interface8880Point& p) const = 0;
    [[nodiscard]] virtual std::optional<uint32_t> getPixel(const Interface8880Point& p) const = 0;

    [[nodiscard]] virtual size_t offset(const Interface8880Point& p) const noexcept = 0;

    virtual bool
    setPixelRGB(
        const Interface8880Point& p,
        const RGB8880& rgb) = 0;

    virtual bool setPixel(const Interface8880Point& p, uint32_t rgb) = 0;

    virtual bool putImage(const Interface8880Point& p, const Image8880& image);

private:

    bool putImagePartial(const Interface8880Point& p, const Image8880& image);
};

//-------------------------------------------------------------------------

Interface8880Point
center(
    const Interface8880& frame,
    const Interface8880& image) noexcept;

//-------------------------------------------------------------------------

} // namespace fb32

