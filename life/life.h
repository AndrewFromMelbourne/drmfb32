#pragma once

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

#include <array>
#include <cstdint>
#include <vector>

#include "framebuffer8880.h"
#include "image8880.h"
#include "joystick.h"

#include "BS_thread_pool.hpp"

//-------------------------------------------------------------------------

class Life
{
public:

    static constexpr size_t aliveCellShift{4};
    static constexpr uint8_t aliveCellMask{1 << aliveCellShift};

    enum CellState
    {
        CELL_DEAD,
        CELL_ALIVE
    };

    explicit Life(int32_t size);

    void init();
    void update(fb32::Joystick& js);
    void draw(fb32::FrameBuffer8880& fb);

private:

    void updateCell(int16_t col, int16_t row, int16_t value);
    void setCell( int16_t col, int16_t row);
    void clearCell(int16_t col, int16_t row);
    void createGosperGliderGun();
    void createSimkinGliderGun();
    void iterateUpperRows(int16_t start, int16_t end);
    void iterateLowerRows(int16_t start, int16_t end);
    void iterateRows(int16_t start, int16_t end);
    void iterate();

    int32_t m_size;
    std::array<uint32_t, 2> m_cellColours;
    std::vector<uint8_t> m_cells;
    std::vector<uint8_t> m_cellsNext;
    fb32::Image8880 m_image;
    BS::thread_pool m_threadPool;
};

