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
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "traceGraph.h"
#include "image8880Graphics.h"
#include "rgb8880.h"
#include "trace.h"

//-------------------------------------------------------------------------

TraceGraph::
TraceGraph(
    int16_t width,
    int16_t traceHeight,
    int16_t traceScale,
    int16_t yPosition,
    int16_t gridHeight,
    int16_t traces,
    const std::string& title,
    const std::vector<std::string>& traceNames,
    const std::vector<fb32::RGB8880>& traceColours)
:
    Trace(
        width,
        traceHeight,
        traceScale,
        yPosition,
        gridHeight,
        traces,
        title,
        traceNames,
        traceColours)
{
}

//-------------------------------------------------------------------------

void
TraceGraph::
draw()
{
    boxFilled(
        getImage(),
        fb32::Image8880Point(0, 0),
        fb32::Image8880Point(getImage().getWidth() - 1, m_traceHeight),
        sc_background);

    //---------------------------------------------------------------------

    for (auto j = 0 ; j < m_traceHeight + 1 ; j+= m_gridHeight)
    {
        horizontalLine(
            getImage(),
            0,
            getImage().getWidth() - 1,
            j,
            sc_gridColour);
    }

    for (auto i = 0 ; i < m_columns ; ++i)
    {
        if (m_time[i] == 0)
        {
            verticalLine(
                getImage(),
                i,
                0,
                m_traceHeight,
                sc_gridColour);
        }
    }

    //---------------------------------------------------------------------

    for (auto& trace : m_traceData)
    {
        for (auto i2 = 1 ; i2 < m_columns ; ++i2)
        {
            auto i1 = i2 - 1;

            int16_t y1 = (trace.m_values[i1] * m_traceHeight)/m_traceScale;
            int16_t y2 = (trace.m_values[i2] * m_traceHeight)/m_traceScale;

            line(
                getImage(),
                fb32::Image8880Point(i1, m_traceHeight - y1),
                fb32::Image8880Point(i2, m_traceHeight - y2),
                trace.m_traceColour);
        }
    }
}

