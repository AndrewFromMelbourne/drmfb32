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

#include <ft2build.h>
#include <freetype/freetype.h>

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

class Image8880FreeType
:
    public Interface8880Font
{
public:

    Image8880FreeType() = default;
    Image8880FreeType(const std::string& fontFile, int pixelSize);
    explicit Image8880FreeType(const FontConfig& fontConfig);
    ~Image8880FreeType() override;

    Image8880FreeType(const Image8880FreeType&) = delete;
    Image8880FreeType(Image8880FreeType&&) = delete;
    Image8880FreeType& operator=(const Image8880FreeType&) = delete;
    Image8880FreeType& operator=(Image8880FreeType&&) = delete;

    [[nodiscard]] std::string getFontFamilyName() const noexcept;
    [[nodiscard]] std::string getFontStyleName() const noexcept;

    [[nodiscard]] int getPixelHeight() const noexcept override;
    [[nodiscard]] int getPixelWidth() const noexcept override;

    [[nodiscard]] std::optional<char> getCharacterCode(CharacterCode code) const noexcept override;

    [[nodiscard]] int getPixelSize() const noexcept
    {
        return m_pixelSize;
    }

    bool setPixelSize(int pixelSize) noexcept;

    Interface8880Point
    drawChar(
        const Interface8880Point& p,
        uint8_t c,
        const RGB8880& rgb,
        Interface8880& image) override;

    Interface8880Point
    drawChar(
        const Interface8880Point& p,
        uint8_t c,
        uint32_t rgb,
        Interface8880& image) override;

    Interface8880Point
    drawWideChar(
        const Interface8880Point& p,
        uint32_t c,
        const RGB8880& rgb,
        Interface8880& image);

    Interface8880Point
    drawString(
        const Interface8880Point& p,
        std::string_view sv,
        const RGB8880& rgb,
        Interface8880& image) override;

    Interface8880Point
    drawString(
        const Interface8880Point& p,
        std::string_view sv,
        uint32_t rgb,
        Interface8880& image) override;

private:

    void
    drawChar(
        int xOffset,
        int yOffset,
        const FT_Bitmap& bitmap,
        const RGB8880& rgb,
        Interface8880& image);

    int m_pixelSize{};

    FT_Face m_face{};
    FT_Library m_library{};
};

//-------------------------------------------------------------------------

} // namespace fb32

//-------------------------------------------------------------------------

