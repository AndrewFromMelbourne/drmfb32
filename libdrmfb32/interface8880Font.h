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

#include <optional>
#include <string_view>

#include "interface8880.h"
#include "point.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

class RGB8880;

//-------------------------------------------------------------------------

class Interface8880Font
{
public:

    enum class CharacterCode
    {
        DEGREE_SYMBOL
    };

    Interface8880Font();
    virtual ~Interface8880Font() = 0;

    Interface8880Font(const Interface8880Font&) = delete;
    Interface8880Font(Interface8880Font&&) = delete;
    Interface8880Font& operator=(const Interface8880Font&) = delete;
    Interface8880Font& operator=(Interface8880Font&&) = delete;

    [[nodiscard]] virtual int getPixelHeight() const noexcept = 0;
    [[nodiscard]] virtual int getPixelWidth() const noexcept = 0;

    [[nodiscard]] virtual std::optional<char> getCharacterCode(CharacterCode code) const noexcept = 0;

    virtual Interface8880Point
    drawChar(
        const Interface8880Point& p,
        uint8_t c,
        const RGB8880& rgb,
        Interface8880& image) = 0;

    virtual Interface8880Point
    drawChar(
        const Interface8880Point& p,
        uint8_t c,
        uint32_t rgb,
        Interface8880& image) = 0;

    virtual Interface8880Point
    drawString(
        const Interface8880Point& p,
        std::string_view sv,
        const RGB8880& rgb,
        Interface8880& image) = 0;

    virtual Interface8880Point
    drawString(
        const Interface8880Point& p,
        std::string_view sv,
        uint32_t rgb,
        Interface8880& image) = 0;
};

//-------------------------------------------------------------------------

} // namespace fb32

//-------------------------------------------------------------------------

