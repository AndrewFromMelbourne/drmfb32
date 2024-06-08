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
#include <cstring>
#include <functional>

#include "life.h"

//-------------------------------------------------------------------------

using namespace fb32;
using namespace std::placeholders;

//-------------------------------------------------------------------------

Life::Life(int32_t size)
:
    m_size{size},
    m_cellColours{
        0x00000000,
        0x00FFFFFF
    },
    m_cells(size * size),
    m_cellsNext(size * size),
    m_image(size, size),
    m_threadPool()
{
}

//-------------------------------------------------------------------------

void
Life::updateCell(
    int col,
    int row,
    int value)
{
    const uint32_t left = (col == 0) ? m_size - 1 :  col - 1;
    const uint32_t right = (col == m_size - 1) ?  0 : col + 1;
    const uint32_t above = (row == 0) ? m_size - 1 : row - 1;
    const uint32_t below = (row == m_size - 1) ? 0 : row + 1;

    const uint32_t aboveOffset = above * m_size;
    const uint32_t rowOffset = row * m_size;
    const uint32_t belowOffset = below * m_size;

    m_cellsNext[left + aboveOffset] += value;
    m_cellsNext[col + aboveOffset] += value;
    m_cellsNext[right + aboveOffset] += value;

    m_cellsNext[left + rowOffset] += value;
    m_cellsNext[right + rowOffset] += value;

    m_cellsNext[left + belowOffset] += value;
    m_cellsNext[col + belowOffset] += value;
    m_cellsNext[right + belowOffset] += value;
}

//-------------------------------------------------------------------------

void
Life::setCell(
    int col,
    int row)
{
    updateCell(col, row, 1);
    m_cellsNext[col + (row * m_size)] |= aliveCellMask;

    fb32::Interface8880Point p{ col, row };
    m_image.setPixel(p, m_cellColours[1]);
}

//-------------------------------------------------------------------------

void
Life::clearCell(
    int col,
    int row)
{
    updateCell(col, row, -1);
    m_cellsNext[col + (row * m_size)] &= ~aliveCellMask;

    fb32::Interface8880Point p{ col, row };
    m_image.setPixel(p, m_cellColours[0]);
}

//-------------------------------------------------------------------------

void
Life::iterateUpperRows(
   int start,
   int end)
{
    auto diff = end - start;

    iterateRows(start, start + (diff / 2));
}

//-------------------------------------------------------------------------

void
Life::iterateLowerRows(
   int start,
   int end)
{
    auto diff = end - start;

    iterateRows(start + (diff / 2), end);
}

//-------------------------------------------------------------------------

void
Life::iterateRows(
   int start,
   int end)
{
    for (auto row = start ; row < end ; ++row)
    {
        for (auto col = 0 ; col < m_size ; ++col)
        {
            auto cell = m_cells[col + (row * m_size)];
            auto neighbours = cell & ~aliveCellMask;
            auto alive = cell & aliveCellMask;

            if (alive)
            {
                if ((neighbours != 2) && (neighbours != 3))
                {
                    clearCell(col, row);
                }
            }
            else
            {
                if (neighbours == 3)
                {
                    setCell(col, row);
                }
            }
        }
    }
}

//-------------------------------------------------------------------------

void
Life::iterate()
{
    {
        auto taskUpper = m_threadPool.parallelize_loop(m_size, std::bind(&Life::iterateUpperRows, this, _1, _2));
        taskUpper.wait();
    }

    {
        auto taskLower = m_threadPool.parallelize_loop(m_size, std::bind(&Life::iterateLowerRows, this, _1, _2));
        taskLower.wait();
    }

    m_cells = m_cellsNext;
}

//-------------------------------------------------------------------------

void
Life::init()
{
    std::fill(m_cells.begin(), m_cells.end(), 0);
    std::fill(m_cellsNext.begin(), m_cellsNext.end(), 0);
    m_image.clear(0);

    for (int row = 0 ; row < m_size ; ++row)
    {
        for (int col = 0 ; col < m_size ; ++col)
        {
            if (std::rand() > (RAND_MAX / 2))
            {
                setCell(col, row);
            }
        }
    }

    m_cells = m_cellsNext;
}

//-------------------------------------------------------------------------

void
Life::createGosperGliderGun()
{
    const int x = (m_size - 36) / 2;
    const int y = (m_size - 8) / 2;

    std::fill(m_cells.begin(), m_cells.end(), 0);
    std::fill(m_cellsNext.begin(), m_cellsNext.end(), 0);
    m_image.clear(0);

    setCell(x + 24, y + 0);

    setCell(x + 22, y + 1);
    setCell(x + 24, y + 1);

    setCell(x + 12, y + 2);
    setCell(x + 13, y + 2);
    setCell(x + 20, y + 2);
    setCell(x + 21, y + 2);
    setCell(x + 34, y + 2);
    setCell(x + 35, y + 2);

    setCell(x + 11, y + 3);
    setCell(x + 15, y + 3);
    setCell(x + 20, y + 3);
    setCell(x + 21, y + 3);
    setCell(x + 34, y + 3);
    setCell(x + 35, y + 3);

    setCell(x + 0, y + 4);
    setCell(x + 1, y + 4);
    setCell(x + 10, y + 4);
    setCell(x + 16, y + 4);
    setCell(x + 20, y + 4);
    setCell(x + 21, y + 4);

    setCell(x + 0, y + 5);
    setCell(x + 1, y + 5);
    setCell(x + 10, y + 5);
    setCell(x + 14, y + 5);
    setCell(x + 16, y + 5);
    setCell(x + 17, y + 5);
    setCell(x + 22, y + 5);
    setCell(x + 24, y + 5);

    setCell(x + 10, y + 6);
    setCell(x + 16, y + 6);
    setCell(x + 24, y + 6);

    setCell(x + 11, y + 7);
    setCell(x + 15, y + 7);

    setCell(x + 12, y + 8);
    setCell(x + 13, y + 8);

    m_cells = m_cellsNext;
}

//-------------------------------------------------------------------------

void
Life::createSimkinGliderGun()
{
    const int x = (m_size - 30) / 2;
    const int y = (m_size - 20 ) / 2;
    m_image.clear(0);

    std::fill(m_cells.begin(), m_cells.end(), 0);
    std::fill(m_cellsNext.begin(), m_cellsNext.end(), 0);

    setCell(x + 0, y + 0);
    setCell(x + 1, y + 0);
    setCell(x + 7, y + 0);
    setCell(x + 8, y + 0);

    setCell(x + 0, y + 1);
    setCell(x + 1, y + 1);
    setCell(x + 7, y + 1);
    setCell(x + 8, y + 1);

    setCell(x + 4, y + 3);
    setCell(x + 5, y + 3);

    setCell(x + 4, y + 4);
    setCell(x + 5, y + 4);

    setCell(x + 22, y + 9);
    setCell(x + 23, y + 9);
    setCell(x + 25, y + 9);
    setCell(x + 26, y + 9);

    setCell(x + 21, y + 10);
    setCell(x + 27, y + 10);

    setCell(x + 21, y + 11);
    setCell(x + 28, y + 11);
    setCell(x + 31, y + 11);
    setCell(x + 32, y + 11);

    setCell(x + 21, y + 12);
    setCell(x + 22, y + 12);
    setCell(x + 23, y + 12);
    setCell(x + 27, y + 12);
    setCell(x + 31, y + 12);
    setCell(x + 32, y + 12);

    setCell(x + 26, y + 13);

    setCell(x + 20, y + 17);
    setCell(x + 21, y + 17);

    setCell(x + 20, y + 18);

    setCell(x + 21, y + 19);
    setCell(x + 22, y + 19);
    setCell(x + 23, y + 19);

    setCell(x + 23, y + 20);

    m_cells = m_cellsNext;
}

//-------------------------------------------------------------------------

void
Life::update(
    fb32::Joystick& js)
{
    if (js.buttonPressed(Joystick::BUTTON_B))
    {
        init();
    }
    else if (js.buttonPressed(Joystick::BUTTON_X))
    {
        createGosperGliderGun();
    }
    else if (js.buttonPressed(Joystick::BUTTON_Y))
    {
        createSimkinGliderGun();
    }
    else
    {
        iterate();
    }
}

//-------------------------------------------------------------------------

void
Life::draw(
    fb32::FrameBuffer8880& fb)
{
    const int xOffset = (fb.getWidth() - m_image.getWidth()) / 2;
    const int yOffset = (fb.getHeight() - m_image.getHeight()) / 2;

    fb.putImage(fb32::Interface8880Point(xOffset, yOffset), m_image);
}

