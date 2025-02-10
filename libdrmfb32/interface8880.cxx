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

#include "image8880.h"
#include "interface8880.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

Interface8880::~Interface8880()
{
}

//-------------------------------------------------------------------------

void
fb32::Interface8880::clear(
    uint32_t rgb)
{
    auto buffer = getBuffer();
    std::fill(buffer.begin(), buffer.end(), rgb);
}

//-------------------------------------------------------------------------

std::optional<fb32::RGB8880>
fb32::Interface8880::getPixelRGB(
    const Interface8880Point& p) const
{
    if (not validPixel(p))
    {
        return {};
    }

    auto buffer = getBuffer();
    return RGB8880(buffer[offset(p)]);
}

//-------------------------------------------------------------------------

std::optional<uint32_t>
fb32::Interface8880::getPixel(
    const Interface8880Point& p) const
{
    if (not validPixel(p))
    {
        return {};
    }

    auto buffer = getBuffer();
    return buffer[offset(p)];
}

//-------------------------------------------------------------------------

std::span<uint32_t>
fb32::Interface8880::getRow(
    int y)
{
    const Interface8880Point p{0, y};

    if (validPixel(p))
    {
        return  getBuffer().subspan(offset(p), getWidth());
    }
    else
    {
        return {};
    }
}

//-------------------------------------------------------------------------

std::span<const uint32_t>
fb32::Interface8880::getRow(
    int y) const
{
    const Interface8880Point p{0, y};

    if (validPixel(p))
    {
        return  getBuffer().subspan(offset(p), getWidth());
    }
    else
    {
        return {};
    }
}

//-------------------------------------------------------------------------

bool
fb32::Interface8880::putImage(
    const Interface8880Point& p_left,
    const Interface8880& image)
{
    Interface8880Point p{ p_left.x(), p_left.y() };

    if ((p.x() < 0) or
        ((p.x() + image.getWidth()) > getWidth()))
    {
        return putImagePartial(p, image);
    }

    if ((p.y() < 0) or
        ((p.y() + image.getHeight()) > getHeight()))
    {
        return putImagePartial(p, image);
    }

    for (int j = 0 ; j < image.getHeight() ; ++j)
    {
        auto row = image.getRow(j);
        const auto ost = offset(Interface8880Point{p.x(), j + p.y()});

        std::copy(row.begin(), row.end(), getBuffer().subspan(ost).begin());
    }

    return true;
}

//-------------------------------------------------------------------------

bool
fb32::Interface8880::putImagePartial(
    const Interface8880Point& p,
    const Interface8880& image)
{
    auto x = p.x();
    auto xStart = 0;
    auto xEnd = image.getWidth() - 1;

    auto y = p.y();
    auto yStart = 0;
    auto yEnd = image.getHeight() - 1;

    if (x < 0)
    {
        xStart = -x;
        x = 0;
    }

    if ((x - xStart + image.getWidth()) > getWidth())
    {
        xEnd = getWidth() - 1 - (x - xStart);
    }

    if (y < 0)
    {
        yStart = -y;
        y = 0;
    }

    if ((y - yStart + image.getHeight()) > getHeight())
    {
        yEnd = getHeight() - 1 - (y - yStart);
    }

    if ((xEnd - xStart) <= 0)
    {
        return false;
    }

    if ((yEnd - yStart) <= 0)
    {
        return false;
    }

    const auto xLength = xEnd - xStart + 1;

    for (auto j = yStart ; j <= yEnd ; ++j)
    {
        auto row = image.getRow(j).subspan(xStart, xLength);
        const auto ost = offset(Interface8880Point{x, j - yStart + y});

        std::copy(row.begin(), row.end(), getBuffer().subspan(ost).begin());
    }

    return true;
}

//-------------------------------------------------------------------------

bool
fb32::Interface8880::setPixel(
    const Interface8880Point& p,
    uint32_t rgb)
{
    bool isValid{validPixel(p)};

    if (isValid)
    {
        auto buffer = getBuffer();
        buffer[offset(p)] = rgb;
    }

    return isValid;
}

//-------------------------------------------------------------------------

Interface8880Point
center(
    const Interface8880& frame,
    const Interface8880& image) noexcept
{
    return {(frame.getWidth() - image.getWidth()) / 2,
            (frame.getHeight() - image.getHeight()) / 2};
}

//-------------------------------------------------------------------------

} // namespace fb32

