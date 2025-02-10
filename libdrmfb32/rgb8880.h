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

//-------------------------------------------------------------------------

namespace fb32
{

//-------------------------------------------------------------------------

class RGB8880
{
public:

    RGB8880(uint8_t red, uint8_t green, uint8_t blue) noexcept;

    explicit RGB8880(uint32_t rgb) noexcept;

    [[nodiscard]] RGB8880 blend(uint8_t alpha, const RGB8880& background) const noexcept;

    [[nodiscard]] uint8_t getRed() const noexcept;
    [[nodiscard]] uint8_t getGreen() const noexcept;
    [[nodiscard]] uint8_t getBlue() const noexcept;

    [[nodiscard]] uint32_t get8880() const noexcept { return m_rgb; }

    [[nodiscard]] bool isGrey() const noexcept
    {
        return (getRed() == getGreen()) and (getGreen() == getBlue());
    }

    void setRGB(uint8_t red, uint8_t green, uint8_t blue) noexcept;

    void set8880(uint32_t rgb) noexcept { m_rgb = rgb; }
    void setGrey(uint8_t grey) noexcept { setRGB(grey, grey, grey); }

    [[nodiscard]] static RGB8880 blend(uint8_t alpha, const RGB8880& a, const RGB8880& b) noexcept;
    [[nodiscard]] static uint32_t rgbTo8880(uint8_t red, uint8_t green, uint8_t blue) noexcept
    {
        return (red << 16) | (green << 8) | blue;
    }

private:

    uint32_t m_rgb;
};

inline bool operator != (const RGB8880& lhs, const RGB8880& rhs) noexcept
{
    return lhs.get8880() != rhs.get8880();
}

inline bool operator == (const RGB8880& lhs, const RGB8880& rhs) noexcept
{
    return !(lhs != rhs);
}

//-------------------------------------------------------------------------

} // namespace fb32

