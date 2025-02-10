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
#include <cmath>
#include <functional>
#include <numbers>
#include <stdexcept>

#include "image8880.h"

//-------------------------------------------------------------------------

using size_type = std::vector<uint32_t>::size_type;
using Point = fb32::Interface8880Point;

//-------------------------------------------------------------------------

fb32::Image8880::Image8880(
    int width,
    int height)
:
    m_width{width},
    m_height{height},
    m_buffer(width * height)
{
}

//-------------------------------------------------------------------------

fb32::Image8880::Image8880(
    int width,
    int height,
    std::initializer_list<uint32_t> buffer)
:
    m_width{width},
    m_height{height},
    m_buffer{buffer}
{
    size_t minBufferSize = width * height;

    if (m_buffer.size() < minBufferSize)
    {
        m_buffer.resize(minBufferSize);
    }
}

//-------------------------------------------------------------------------

fb32::Image8880::Image8880(
    int width,
    int height,
    std::span<const uint32_t> buffer)
:
    m_width{width},
    m_height{height},
    m_buffer{}
{
    m_buffer.assign(buffer.begin(), buffer.end());

    size_t minBufferSize = width * height;

    if (m_buffer.size() < minBufferSize)
    {
        m_buffer.resize(minBufferSize);
    }
}

//-------------------------------------------------------------------------

size_t
fb32::Image8880::offset(
    const Interface8880Point& p) const noexcept
{
    return p.x() + (p.y() * m_width);
}

