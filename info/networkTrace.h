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
#include <vector>

#include "panel.h"
#include "traceGraph.h"

//-------------------------------------------------------------------------

namespace fb32
{
class FrameBuffer8880;
}

//-------------------------------------------------------------------------

class NetworkStats
{
public:

    NetworkStats();

    [[nodiscard]] int tx() const noexcept { return m_tx; }
    [[nodiscard]] int rx() const noexcept { return m_rx; }

    NetworkStats& operator-=(const NetworkStats& rhs) noexcept;

private:

    int m_tx;
    int m_rx;
};

NetworkStats operator-(const NetworkStats& lhs, const NetworkStats& rhs) noexcept;

//-------------------------------------------------------------------------

class NetworkTrace
:
    public TraceGraph
{
public:

    NetworkTrace(
        int width,
        int graphHeight,
        int fontHeight,
        int yPosition,
        int gridHeight = 20);

    void update(time_t now, fb32::Interface8880Font& font) override;

private:

    NetworkStats m_previousStats;
};

//-------------------------------------------------------------------------

