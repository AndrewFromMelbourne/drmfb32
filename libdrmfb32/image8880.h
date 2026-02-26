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

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <utility>
#include <span>
#include <vector>

#include "interface8880.h"
#include "rgb8880.h"

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

class Image8880
:
    public Interface8880
{
public:

    //---------------------------------------------------------------------
    // constructors, destructors and assignment

    Image8880() = default;
    Image8880(Dimensions8880 d);
    Image8880(Dimensions8880 d, std::initializer_list<uint32_t> buffer);
    Image8880(Dimensions8880 d, std::span<const uint32_t> buffer);

    ~Image8880() override = default;

    Image8880(const Image8880&) = default;
    Image8880& operator=(const Image8880&) = default;

    Image8880(Image8880&& image) = default;
    Image8880& operator=(Image8880&& image) = default;

    Image8880(const Interface8880& i)
    :
        Image8880(i.getDimensions(), i.getBuffer())
    {
    }

    Image8880& operator=(const Interface8880& i);

    //---------------------------------------------------------------------
    // getters and setters

    [[nodiscard]] Dimensions8880 getDimensions() const noexcept override { return m_dimensions; }

    [[nodiscard]] std::span<uint32_t> getBuffer() noexcept override { return m_buffer; };
    [[nodiscard]] std::span<const uint32_t> getBuffer() const noexcept override { return m_buffer; }

    std::size_t offset(Point8880 p) const noexcept override;

private:

    Dimensions8880 m_dimensions;
    std::vector<uint32_t> m_buffer{};
};

//-------------------------------------------------------------------------

} // namespace fb32

