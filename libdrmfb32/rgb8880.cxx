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

fb32::RGB8880:: RGB8880(
    uint8_t red,
    uint8_t green,
    uint8_t blue)
:
    m_rgb{0}
{
    setRGB(red, green, blue);
}

//-------------------------------------------------------------------------

fb32::RGB8880:: RGB8880(
    uint32_t rgb)
:
    m_rgb{rgb}
{
}

//-------------------------------------------------------------------------

uint8_t
fb32::RGB8880:: getRed() const
{
    return (m_rgb >> 16) & 0xFF;
}

//-------------------------------------------------------------------------

uint8_t
fb32::RGB8880:: getGreen() const
{
    return (m_rgb >> 8) & 0xFF;
}

//-------------------------------------------------------------------------

uint8_t
fb32::RGB8880:: getBlue() const
{
    return m_rgb & 0xFF;
}

//-------------------------------------------------------------------------

void
fb32::RGB8880:: setRGB(
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
    m_rgb = (red << 16) | (green << 8) | blue;
}

//-------------------------------------------------------------------------

fb32::RGB8880
fb32::RGB8880:: blend(
    uint8_t alpha,
    const RGB8880& a,
    const RGB8880& b)
{
    auto red = (((int)(a.getRed()) * alpha)
             + ((int)(b.getRed()) * (255 - alpha)))
             / 255;

    auto green = (((int)(a.getGreen()) * alpha)
               + ((int)(b.getGreen()) * (255 - alpha)))
               / 255;

    auto blue = (((int)(a.getBlue()) * alpha)
              + ((int)(b.getBlue()) * (255 - alpha)))
              / 255;

    return RGB8880(red, green, blue);
}

