#pragma once

//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2024 Andrew Duncan
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

#include <array>
#include <cstdint>

#include "image8880.h"
#include "interface8880.h"
#include "joystick.h"

#include "images.h"

//-------------------------------------------------------------------------

class Puzzle
{
public:

    struct Location
    {
        int x;
        int y;
    };

    Puzzle();

    void init();
    bool update(fb32::Joystick& js);
    void draw(fb32::Interface8880& fb);

private:

    int getInversionCount() const;
    bool isSolvable() const;
    bool isSolved() const;

    static constexpr int puzzleWidth = 4;
    static constexpr int puzzleHeight = 4;
    static constexpr int boardSize = puzzleWidth * puzzleHeight;

    std::array<uint8_t, boardSize> m_board;
    std::array<fb32::Image8880, tileCount> m_tileBuffers;
    Location m_blankLocation;
};
