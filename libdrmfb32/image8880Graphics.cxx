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

#include <cmath>
#include <cstdint>

#include "interface8880.h"
#include "image8880Graphics.h"
#include "point.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

void
box(
    Interface8880& image,
    const Interface8880Point& p1,
    const Interface8880Point& p2,
    uint32_t rgb)
{
    verticalLine(image, p1.x(), p1.y(), p2.y(), rgb);
    horizontalLine(image, p1.x(), p2.x(), p1.y(), rgb);
    verticalLine(image, p2.x(), p1.y(), p2.y(), rgb);
    horizontalLine(image, p1.x(), p2.x(), p2.y(), rgb);
}

//-------------------------------------------------------------------------

void
boxFilled(
    Interface8880& image,
    const Interface8880Point& p1,
    const Interface8880Point& p2,
    uint32_t rgb)
{
    const auto sign_y = (p1.y() <= p2.y()) ? 1 : -1;
    auto y = p1.y();

    horizontalLine(image, p1.x(), p2.x(), y, rgb);

    while (y != p2.y())
    {
        y += sign_y;
        horizontalLine(image, p1.x(), p2.x(), y, rgb);
    }
}

//-------------------------------------------------------------------------

void
line(
    Interface8880& image,
    const Interface8880Point& p1,
    const Interface8880Point& p2,
    uint32_t rgb)
{
    if (p1.y() == p2.y())
    {
        horizontalLine(image, p1.x(), p2.x(), p1.y(), rgb);
    }
    else if (p1.x() == p2.x())
    {
        verticalLine(image, p1.x(), p1.y(), p2.y(), rgb);
    }
    else
    {
        const auto dx = std::abs(p2.x() - p1.x());
        const auto dy = std::abs(p2.y() - p1.y());

        const auto sign_x = (p1.x() <= p2.x()) ? 1 : -1;
        const auto sign_y = (p1.y() <= p2.y()) ? 1 : -1;

        auto x = p1.x();
        auto y = p1.y();

        image.setPixel(p1, rgb);

        if (dx > dy)
        {
            auto d = 2 * dy - dx;
            const auto incrE = 2 * dy;
            const auto incrNE = 2 * (dy - dx);

            while (x != p2.x())
            {
                x += sign_x;

                if (d <= 0)
                {
                    d += incrE;
                }
                else
                {
                    d += incrNE;
                    y += sign_y;
                }

                image.setPixel(Interface8880Point(x, y), rgb);
            }
        }
        else
        {
            auto d = 2 * dx - dy;
            const auto incrN = 2 * dx;
            const auto incrNE = 2 * (dx - dy);

            while (y != p2.y())
            {
                y += sign_y;

                if (d <= 0)
                {
                    d += incrN;
                }
                else
                {
                    d += incrNE;
                    x += sign_x;
                }

                image.setPixel(Interface8880Point(x, y), rgb);
            }
        }
    }
}

//-------------------------------------------------------------------------

void
horizontalLine(
    Interface8880& image,
    int x1,
    int x2,
    int y,
    uint32_t rgb)
{
    const auto sign_x = (x1 <= x2) ? 1 : -1;
    auto x = x1;

    image.setPixel(Interface8880Point(x, y), rgb);

    while (x != x2)
    {
        x += sign_x;
        image.setPixel(Interface8880Point(x, y), rgb);
    }
}

//-------------------------------------------------------------------------

void
verticalLine(
    Interface8880& image,
    int x,
    int y1,
    int y2,
    uint32_t rgb)
{
    const auto sign_y = (y1 <= y2) ? 1 : -1;
    auto y = y1;

    image.setPixel(Interface8880Point(x, y), rgb);

    while (y != y2)
    {
        y += sign_y;
        image.setPixel(Interface8880Point(x, y), rgb);
    }
}

//-------------------------------------------------------------------------

} // namespace fb32
