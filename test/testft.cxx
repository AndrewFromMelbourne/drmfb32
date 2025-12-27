//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2023 Andrew Duncan
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

#include <getopt.h>
#include <libgen.h>

#include <chrono>
#include <iostream>
#include <print>
#include <system_error>
#include <thread>

#include "framebuffer8880.h"
#include "image8880.h"
#include "image8880FreeType.h"
#include "point.h"

//-------------------------------------------------------------------------

using namespace fb32;
using namespace std::chrono_literals;

//-------------------------------------------------------------------------

void
printUsage(
    std::ostream& stream,
    const std::string& name)
{
    std::println(stream, "");
    std::println(stream, "Usage: {} <options>", name);
    std::println(stream, "");
    std::println(stream, "    --connector,-c - dri connector to use");
    std::println(stream, "    --device,-d - dri device to use");
    std::println(stream, "    --font,-f - font file to use[:pixel height]");
    std::println(stream, "    --help,-h - print usage and exit");
    std::println(stream, "");
}

//-------------------------------------------------------------------------

int
main(
    int argc,
    char *argv[])
{
    uint32_t connector{0};
    std::string device{};
    const std::string program = basename(argv[0]);
    FontConfig fontConfig;

    //---------------------------------------------------------------------

    static const char* sopts = "c:d:f:h";
    static option lopts[] =
    {
        { "connector", required_argument, nullptr, 'c' },
        { "device", required_argument, nullptr, 'd' },
        { "font", required_argument, nullptr, 'f' },
        { "help", no_argument, nullptr, 'h' },
        { nullptr, no_argument, nullptr, 0 }
    };

    int opt{};

    while ((opt = ::getopt_long(argc, argv, sopts, lopts, nullptr)) != -1)
    {
        switch (opt)
        {
        case 'c':

            connector = std::stol(optarg);

            break;

        case 'd':

            device = optarg;

            break;

        case 'f':

            fontConfig = fb32::parseFontConfig(optarg, 32);

            break;

        case 'h':

            printUsage(std::cout, program);
            ::exit(EXIT_SUCCESS);

            break;

        default:

            printUsage(std::cerr, program);
            ::exit(EXIT_FAILURE);

            break;
        }
    }

    //---------------------------------------------------------------------

    if (fontConfig.m_fontFile.empty())
    {
        std::println(std::cerr, "Error: Font file must be specfied");
        exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    try
    {
        constexpr RGB8880 black{0, 0, 0};
        constexpr RGB8880 white{255, 255, 255};
        FrameBuffer8880 fb{device, connector};

        Image8880 image{fb.getWidth(), fb.getHeight()};
        image.clear(black);

        //-----------------------------------------------------------------

        Image8880FreeType ft{fontConfig};

        Interface8880Point p{0, 0};

        p = ft.drawString(p, "abcdefghijklmnopqrstuvwxyz ", white, image);
        p = ft.drawString(p, "0123456789", white, image);

        p.setX(0);
        p.incrY(ft.getPixelHeight());

        p = ft.drawString(p, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", white, image);

        p.setX(0);
        p.incrY(ft.getPixelHeight());

        p = ft.drawChar(p, '@', white, image);

        p.setX(0);
        p.incrY(ft.getPixelHeight());

        for (int j = 0 ; j < 16 ; ++j)
        {
            for (int i = 0 ; i < 16 ; ++i)
            {
                const uint8_t c = static_cast<uint8_t>(i + (j * 16));
                p.setX(i * ft.getPixelWidth());
                ft.drawChar(p, c, white, image);
            }

            p.incrY(ft.getPixelHeight());
        }

        //-----------------------------------------------------------------

        fb.putImage(Interface8880Point{0, 0}, image);
        fb.update();

        //-----------------------------------------------------------------

        std::this_thread::sleep_for(10s);
    }
    catch (std::exception& error)
    {
        std::println(std::cerr, "Error: {}", error.what());
        exit(EXIT_FAILURE);
    }

    //---------------------------------------------------------------------

    return 0 ;
}

