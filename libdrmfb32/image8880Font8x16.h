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

#include "interface8880.h"
#include "interface8880Font.h"
#include "point.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

using Interface8880Point = Point<int>;

//-------------------------------------------------------------------------

class RGB8880;

//-------------------------------------------------------------------------

class Image8880Font8x16
:
    public Interface8880Font
{
public:

    Image8880Font8x16() = default;
    ~Image8880Font8x16() override = default;

    Image8880Font8x16(const Image8880Font8x16&) = delete;
    Image8880Font8x16(Image8880Font8x16&&) = delete;
    Image8880Font8x16& operator=(const Image8880Font8x16&) = delete;
    Image8880Font8x16& operator=(Image8880Font8x16&&) = delete;

    [[nodiscard]] int getPixelHeight() const noexcept override;
    [[nodiscard]] int getPixelWidth() const noexcept override;

    [[nodiscard]] std::optional<char> getCharacterCode(CharacterCode code) const noexcept override;

    Interface8880Point
    drawChar(
        Interface8880Point p,
        uint8_t c,
        const RGB8880& rgb,
        Interface8880& image) override;

    Interface8880Point
    drawChar(
        Interface8880Point p,
        uint8_t c,
        uint32_t rgb,
        Interface8880& image) override;

    Interface8880Point
    drawString(
        Interface8880Point p,
        std::string_view cv,
        const RGB8880& rgb,
        Interface8880& image) override;

    Interface8880Point
    drawString(
        Interface8880Point p,
        std::string_view sv,
        uint32_t rgb,
        Interface8880& image) override;
};

//-------------------------------------------------------------------------

} // namespace fb32

