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

#ifndef IMAGE8880_GRAPHICS_H
#define IMAGE8880_GRAPHICS_H

//-------------------------------------------------------------------------

#include <cstdint>

#include "image8880.h"
#include "point.h"
#include "rgb8880.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

void
box(
    Image8880& image,
    const Image8880Point& p1,
    const Image8880Point& p2,
    uint32_t rgb);

inline void
box(
    Image8880& image,
    const Image8880Point& p1,
    const Image8880Point& p2,
    const RGB8880& rgb)
{
    box(image, p1, p2, rgb.get8880());
}

//-------------------------------------------------------------------------

void
boxFilled(
    Image8880& image,
    const Image8880Point& p1,
    const Image8880Point& p2,
    uint32_t rgb);

inline void
boxFilled(
    Image8880& image,
    const Image8880Point& p1,
    const Image8880Point& p2,
    const RGB8880& rgb)
{
    boxFilled(image, p1, p2, rgb.get8880());
}

//-------------------------------------------------------------------------

void
line(
    Image8880& image,
    const Image8880Point& p1,
    const Image8880Point& p2,
    uint32_t rgb);

inline void
line(
    Image8880& image,
    const Image8880Point& p1,
    const Image8880Point& p2,
    const RGB8880& rgb)
{
    line(image, p1, p2, rgb.get8880());
}

//-------------------------------------------------------------------------

void
horizontalLine(
    Image8880& image,
    int16_t x1,
    int16_t x2,
    int16_t y,
    uint32_t rgb);

inline void
horizontalLine(
    Image8880& image,
    int16_t x1,
    int16_t x2,
    int16_t y,
    const RGB8880& rgb)
{
    horizontalLine(image, x1, x2, y, rgb.get8880());
}

//-------------------------------------------------------------------------

void
verticalLine(
    Image8880& image,
    int16_t x,
    int16_t y1,
    int16_t y2,
    uint32_t rgb);

inline void
verticalLine(
    Image8880& image,
    int16_t x,
    int16_t y1,
    int16_t y2,
    const RGB8880& rgb)
{
    verticalLine(image, x, y1, y2, rgb.get8880());
}

//-------------------------------------------------------------------------

} // namespace fb32

//-------------------------------------------------------------------------

#endif
