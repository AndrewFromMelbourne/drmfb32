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

#ifndef DYNAMIC_INFO_H
#define DYNAMIC_INFO_H

//-------------------------------------------------------------------------

#include <cstdint>
#include <string>

#include "image8880Font8x16.h"
#include "panel.h"
#include "rgb8880.h"

//-------------------------------------------------------------------------

namespace fb32
{
class FrameBuffer8880;
}

//-------------------------------------------------------------------------

class DynamicInfo
:
    public Panel
{
public:

    DynamicInfo(int width,
                int fontHeight,
                int yPosition);

    void init(fb32::Interface8880Font& font) override;
    void update(time_t now, fb32::Interface8880Font& font) override;

private:

    fb32::RGB8880 m_heading;
    fb32::RGB8880 m_foreground;
    fb32::RGB8880 m_warning;
    fb32::RGB8880 m_background;

    static std::string getIpAddress(char& interface);
    void drawIpAddress(fb32::Interface8880Point& position,
                       fb32::Interface8880Font& font);

    static std::string getTemperature();
    void drawTemperature(fb32::Interface8880Point& position,
                         fb32::Interface8880Font& font);

    static std::string getTime(time_t now);
    void drawTime(fb32::Interface8880Point& position,
                  fb32::Interface8880Font& font,
                  time_t now);
};

//-------------------------------------------------------------------------

#endif

