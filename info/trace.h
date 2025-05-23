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
#include <string>
#include <vector>

#include <sys/time.h>

#include "interface8880Font.h"
#include "panel.h"
#include "rgb8880.h"

//-------------------------------------------------------------------------

namespace fb32
{
class FrameBuffer8880;
}

//-------------------------------------------------------------------------

class TraceData
{
public:

    TraceData(
        const std::string& name,
        fb32::RGB8880 traceColour,
        fb32::RGB8880 gridColour,
        int width)
    :
        m_name{name},
        m_traceColour{traceColour},
        m_gridColour{gridColour},
        m_values{},
        m_width{width}
    {
        m_values.reserve(width);
    }

    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    [[nodiscard]] fb32::RGB8880 traceColour() const noexcept { return m_traceColour; }
    [[nodiscard]] fb32::RGB8880 gridColour() const noexcept { return m_gridColour; }

    void addData(int value);
    [[nodiscard]] int max() const;
    [[nodiscard]] int value(int i) const { return m_values[i]; }

private:

    std::string m_name;
    fb32::RGB8880 m_traceColour;
    fb32::RGB8880 m_gridColour;
    std::vector<int> m_values;
    int m_width;
};

//-------------------------------------------------------------------------

struct TraceConfiguration
{
    std::string m_name;
    fb32::RGB8880 m_traceColour;
};

//-------------------------------------------------------------------------

class Trace
:
    public Panel
{
public:

    Trace(
        int width,
        int traceHeight,
        int fontHeight,
        int traceScale,
        int yPosition,
        int gridHeight,
        const std::string& title,
        const std::vector<TraceConfiguration>& traces);

    void init(fb32::Interface8880Font& font) override;
    void update(time_t now, fb32::Interface8880Font& font) override = 0;

protected:

    void addData(const std::vector<int>& data, time_t now);
    virtual void draw() = 0;

    int m_traceHeight;
    int m_fontHeight;
    int m_traceScale;
    int m_gridHeight;
    int m_columns;

    std::string m_title;

    bool m_autoScale;

    std::vector<TraceData> m_traceData;
    std::vector<time_t> m_time;

    static const fb32::RGB8880 sc_foreground;
    static const fb32::RGB8880 sc_background;
    static const fb32::RGB8880 sc_gridColour;

private:

    void addDataPoint(const std::vector<int>& data, time_t now);
};

//-------------------------------------------------------------------------

