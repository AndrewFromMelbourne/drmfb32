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

#include "image8880.h"
#include "point.h"
#include "rgb8880.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

void
box(
    Interface8880& image,
    const Interface8880Point& p1,
    const Interface8880Point& p2,
    uint32_t rgb);

inline void
box(
    Interface8880& image,
    const Interface8880Point& p1,
    const Interface8880Point& p2,
    const RGB8880& rgb)
{
    box(image, p1, p2, rgb.get8880());
}

//-------------------------------------------------------------------------

void
boxFilled(
    Interface8880& image,
    const Interface8880Point& p1,
    const Interface8880Point& p2,
    uint32_t rgb);

inline void
boxFilled(
    Interface8880& image,
    const Interface8880Point& p1,
    const Interface8880Point& p2,
    const RGB8880& rgb)
{
    boxFilled(image, p1, p2, rgb.get8880());
}

void
boxFilled(
    Interface8880& image,
    const Interface8880Point& p1,
    const Interface8880Point& p2,
    const RGB8880& rgb,
    uint8_t alpha);

inline void
boxFilled(
        Interface8880& image,
        const Interface8880Point& p1,
        const Interface8880Point& p2,
        uint32_t rgb,
        uint8_t alpha)
{
    boxFilled(image, p1, p2, RGB8880{rgb}, alpha);
}

//-------------------------------------------------------------------------

void
line(
    Interface8880& image,
    const Interface8880Point& p1,
    const Interface8880Point& p2,
    uint32_t rgb);

inline void
line(
    Interface8880& image,
    const Interface8880Point& p1,
    const Interface8880Point& p2,
    const RGB8880& rgb)
{
    line(image, p1, p2, rgb.get8880());
}

//-------------------------------------------------------------------------

void
horizontalLine(
    Interface8880& image,
    int x1,
    int x2,
    int y,
    uint32_t rgb);

inline void
horizontalLine(
    Interface8880& image,
    int x1,
    int x2,
    int y,
    const RGB8880& rgb)
{
    horizontalLine(image, x1, x2, y, rgb.get8880());
}

//-------------------------------------------------------------------------

void
verticalLine(
    Interface8880& image,
    int x,
    int y1,
    int y2,
    uint32_t rgb);

inline void
verticalLine(
    Interface8880& image,
    int x,
    int y1,
    int y2,
    const RGB8880& rgb)
{
    verticalLine(image, x, y1, y2, rgb.get8880());
}

//-------------------------------------------------------------------------

void
circle(
    Interface8880& image,
    const Interface8880Point& p,
    int r,
    uint32_t rgb);

inline void
circle(
    Interface8880& image,
    const Interface8880Point& p,
    int r,
    const RGB8880& rgb)
{
    circle(image, p, r, rgb.get8880());
}

//-------------------------------------------------------------------------

void
circleFilled(
    Interface8880& image,
    const Interface8880Point& p,
    int r,
    uint32_t rgb);

inline void
circleFilled(
    Interface8880& image,
    const Interface8880Point& p,
    int r,
    const RGB8880& rgb)
{
    circleFilled(image, p, r, rgb.get8880());
}

//-------------------------------------------------------------------------

} // namespace fb32

