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

#include "rgb8880.h"

//-------------------------------------------------------------------------

fb32::RGB8880::RGB8880(
    uint8_t red,
    uint8_t green,
    uint8_t blue) noexcept
:
    m_rgb{0}
{
    setRGB(red, green, blue);
}

//-------------------------------------------------------------------------

fb32::RGB8880::RGB8880(
    RGB8 rgb) noexcept
:
    m_rgb{0}
{
    setRGB8(rgb);
}

//-------------------------------------------------------------------------

fb32::RGB8880::RGB8880(
    uint32_t rgb) noexcept
:
    m_rgb{rgb}
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

uint8_t
fb32::RGB8880::getRed() const noexcept
{
    return (m_rgb >> 16) & 0xFF;
}

//-------------------------------------------------------------------------

uint8_t
fb32::RGB8880::getGreen() const noexcept
{
    return (m_rgb >> 8) & 0xFF;
}

//-------------------------------------------------------------------------

uint8_t
fb32::RGB8880::getBlue() const noexcept
{
    return m_rgb & 0xFF;
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
fb32::RGB8880::setRGB(
    uint8_t red,
    uint8_t green,
    uint8_t blue) noexcept
{
    m_rgb = rgbTo8880(red, green, blue);
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

