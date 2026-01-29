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

#include <algorithm>
#include <cstdint>
#include <ranges>
#include <regex>
#include <string>


#include "rgb8880.h"

// ========================================================================

namespace
{

// ------------------------------------------------------------------------

std::string
    toupper(
        std::string_view s)
{
    std::string result;
    std::ranges::copy(std::views::transform(s, ::toupper), std::back_inserter(result));
    return result;
}

// ------------------------------------------------------------------------

} // namespace

// ========================================================================

fb32::RGB8880::RGB8880(
    RGB8 rgb) noexcept
:
    m_rgb{rgbTo8880(rgb.red, rgb.green, rgb.blue)}
{
}

//-------------------------------------------------------------------------

fb32::RGB8880
fb32::RGB8880::blend(
    uint8_t alpha,
    const RGB8880& background) const noexcept
{
    return blend(alpha, *this, background);
}

//-------------------------------------------------------------------------

fb32::RGB8
fb32::RGB8880::getRGB8() const noexcept
{
    return RGB8{ static_cast<uint8_t>((m_rgb >> 16) & 0xFF),
                 static_cast<uint8_t>((m_rgb >> 8) & 0xFF),
                 static_cast<uint8_t>(m_rgb & 0xFF) };
}

//-------------------------------------------------------------------------

void
fb32::RGB8880::setRGB8(
    RGB8 rgb8) noexcept
{
    m_rgb = rgbTo8880(rgb8.red, rgb8.green, rgb8.blue);
}

//-------------------------------------------------------------------------

fb32::RGB8880
fb32::RGB8880::blend(
    uint8_t alpha,
    const RGB8880& a,
    const RGB8880& b) noexcept
{
    auto blendChannel = [](uint8_t alpha, int a, int b) -> int
    {
        return ((a * alpha) + (b * (255 - alpha))) / 255;
    };

    //---------------------------------------------------------------------

    const auto red = blendChannel(alpha, a.getRed(), b.getRed());
    const auto green = blendChannel(alpha, a.getGreen(), b.getGreen());
    const auto blue = blendChannel(alpha, a.getBlue(), b.getBlue());

    return RGB8880(red, green, blue);
}

//-------------------------------------------------------------------------

std::optional<fb32::RGB8880>
fb32::parseRGB8880(
    std::string_view str)
{
    auto s{toupper(str)};
    std::smatch match;

    //---------------------------------------------------------------------
    // Match #RRGGBB or RRGGBB

    const std::regex hexRegex(R"(#?([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2}))");

    if (std::regex_match(s, match, hexRegex))
    {
        return fb32::RGB8880{
            static_cast<uint8_t>(std::stoi(match[1].str(), nullptr, 16)),
            static_cast<uint8_t>(std::stoi(match[2].str(), nullptr, 16)),
            static_cast<uint8_t>(std::stoi(match[3].str(), nullptr, 16))};
    }

    //---------------------------------------------------------------------
    // Match #RGB or RGB

    const std::regex shortHexRegex(R"(#?([0-9A-F])([0-9A-F])([0-9A-F]))");

    if (std::regex_match(s, match, shortHexRegex))
    {
        auto red = std::stoi(match[1].str(), nullptr, 16);
        auto green = std::stoi(match[2].str(), nullptr, 16);
        auto blue = std::stoi(match[3].str(), nullptr, 16);

        red += (red << 4);
        green += (green << 4);
        blue += (blue << 4);

        return fb32::RGB8880{
            static_cast<uint8_t>(red),
            static_cast<uint8_t>(green),
            static_cast<uint8_t>(blue)};
    }

    //---------------------------------------------------------------------
    // Match RGB(r,g,b)

    const std::regex rgbFuncRegex(R"(RGB\(\s*(\d{1,3})\s*,\s*(\d{1,3})\s*,\s*(\d{1,3})\s*\))");
    if (std::regex_match(s, match, rgbFuncRegex))
    {
        return fb32::RGB8880{
            static_cast<uint8_t>(std::stoi(match[1].str())),
            static_cast<uint8_t>(std::stoi(match[2].str())),
            static_cast<uint8_t>(std::stoi(match[3].str()))};
    }

    //---------------------------------------------------------------------
    // No match

    return std::nullopt;
}

