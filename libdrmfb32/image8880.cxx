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
#include <ranges>
#include <stdexcept>

#include "image8880.h"

//-------------------------------------------------------------------------

using size_type = std::vector<uint32_t>::size_type;
using Point = fb32::Point8880;

//-------------------------------------------------------------------------

fb32::Image8880::Image8880(
    Dimensions8880 d)
:
    m_dimensions{d},
    m_buffer(d.area())
{
}

//-------------------------------------------------------------------------

fb32::Image8880::Image8880(
    Dimensions8880 d,
    std::initializer_list<uint32_t> buffer)
:
    m_dimensions{d},
    m_buffer{buffer}
{
    const std::size_t minBufferSize = d.area();

    if (m_buffer.size() < minBufferSize)
    {
        m_buffer.reserve(minBufferSize);
    }
}

//-------------------------------------------------------------------------

fb32::Image8880::Image8880(
    Dimensions8880 d,
    std::span<const uint32_t> buffer)
:
    m_dimensions{d},
    m_buffer{}
{
    m_buffer.assign(cbegin(buffer), cend(buffer));

    const std::size_t minBufferSize = d.area();

    if (m_buffer.size() < minBufferSize)
    {
        m_buffer.reserve(minBufferSize);
    }
}

//-------------------------------------------------------------------------

fb32::Image8880::Image8880(
    const fb32::Interface8880& i)
:
    m_dimensions{},
    m_buffer{}
{
    copy(i);
}

//-------------------------------------------------------------------------

fb32::Image8880&
fb32::Image8880::operator=(
    const fb32::Interface8880& i)
{
    if (&i != this)
    {
        copy(i);
    }
    return *this;
}


//-------------------------------------------------------------------------

void
fb32::Image8880::copy(
    const fb32::Interface8880& i)
{
    m_dimensions = i.getDimensions();
    m_buffer.reserve(m_dimensions.area());

    for (auto y = 0 ; y < m_dimensions.height() ; ++y)
    {
        auto destination = getRow(y);
        const auto source = i.getRow(y).subspan(0, m_dimensions.width());
        std::ranges::copy(source, begin(destination));
    }
}

//-------------------------------------------------------------------------

std::size_t
fb32::Image8880::offset(
    Point8880 p) const noexcept
{
    return p.x() + (p.y() * m_dimensions.width());
}

