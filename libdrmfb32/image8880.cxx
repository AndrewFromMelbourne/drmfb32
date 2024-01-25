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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image8880.h"

//-------------------------------------------------------------------------

using size_type = std::vector<uint32_t>::size_type;

//-------------------------------------------------------------------------

fb32::Image8880::Image8880(
    int width,
    int height,
    uint8_t numberOfFrames)
:
    m_width{width},
    m_height{height},
    m_frame{0},
    m_numberOfFrames{numberOfFrames},
    m_buffer(width * height * numberOfFrames)
{
}

//-------------------------------------------------------------------------

fb32::Image8880::Image8880(
    int width,
    int height,
    const std::vector<uint32_t>& buffer,
    uint8_t numberOfFrames)
:
    m_width{width},
    m_height{height},
    m_frame{0},
    m_numberOfFrames{numberOfFrames},
    m_buffer(buffer)
{
    size_t minBufferSize = width * height * numberOfFrames;

    if (m_buffer.size() < minBufferSize)
    {
        m_buffer.resize(minBufferSize);
    }
}

//-------------------------------------------------------------------------

void
fb32::Image8880::setFrame(
    uint8_t frame)
{
    if (frame < m_numberOfFrames)
    {
        m_frame = frame;
    }
}

//-------------------------------------------------------------------------

void
fb32::Image8880::clear(
    uint32_t rgb)
{
    std::fill(m_buffer.begin(), m_buffer.end(), rgb);
}

//-------------------------------------------------------------------------

bool
fb32::Image8880::setPixel(
    const Interface8880Point& p,
    uint32_t rgb)
{
    bool isValid{validPixel(p)};

    if (isValid)
    {
        m_buffer[offset(p)] = rgb;
    }

    return isValid;
}

//-------------------------------------------------------------------------

std::optional<fb32::RGB8880>
fb32::Image8880::getPixelRGB(
    const Interface8880Point& p) const
{
    if (not validPixel(p))
    {
        return {};
    }

    return RGB8880(m_buffer[offset(p)]);
}

//-------------------------------------------------------------------------

std::optional<uint32_t>
fb32::Image8880::getPixel(
    const Interface8880Point& p) const
{
    if (not validPixel(p))
    {
        return {};
    }

    return m_buffer[offset(p)];
}

//-------------------------------------------------------------------------

const uint32_t*
fb32::Image8880::getRow(
    int y) const
{
    const Interface8880Point p{0, y};

    if (validPixel(p))
    {
        return  m_buffer.data() + offset(p);
    }
    else
    {
        return nullptr;
    }
}

//-------------------------------------------------------------------------

size_t
fb32::Image8880::offset(
    const Interface8880Point& p) const
{
    return p.x() + (p.y() * m_width) + (m_width * m_height * m_frame);
}

